#include <network.h>
#include <stdio.h>
#include <sys/iosupport.h>
#include <ogc/machine/processor.h>

#include <string.h>
#include <unistd.h>
#include <errno.h>

extern const devoptab_t *devoptab_list[];

static int socket = -1;
static int connection_failed = 0;

static void InitializeNetwork(const char *ip_str, const int port)
{
	int init;
	if (socket>=0)
		return;

	connection_failed = 0;

	// Retry net_init (can return -EAGAIN initially)
	int retries = 100;
	while (retries-- > 0 && (init = net_init()) < 0) {
		if (init == -EAGAIN) {
			usleep(10000); // 10ms delay
			continue;
		}
		break;
	}
	if (init < 0) {
		connection_failed = 1;
		return; // Network init failed
	}

	// DEBUG: Connect to me
	socket = net_socket(PF_INET, SOCK_STREAM, 0);
	if (socket < 0) {
		connection_failed = 1;
		return;
	}

	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));

	address.sin_family = PF_INET;
	address.sin_port = htons(port);
	int ret = inet_aton(ip_str, &address.sin_addr);
	if (ret == 0) {
		net_close(socket);
		socket = -1;
		connection_failed = 1;
		return;
	}
	if (net_connect(socket, (struct sockaddr*)&address, sizeof(address)) < 0)
	{
		net_close(socket);
		socket = -1;
		connection_failed = 1;
		return;
	}
}

static ssize_t DebugPrint(struct _reent *r, void *fd, const char *ptr, size_t len)
{
	if (socket >= 0) {
		size_t total_sent = 0;
		while (total_sent < len) {
			ssize_t res = net_send(socket, ptr + total_sent, len - total_sent, 0);
			if (res < 0) {
				socket = -1; // Connection lost
				break;
			}
			if (res == 0) {
				continue; // Retry
			}
			total_sent += res;
		}
		return total_sent;
	}
	return 0;
}

static const devoptab_t dotab_netout = {
	"netout",
	0,
	NULL,
	NULL,
	DebugPrint, // device write
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};


void Init_DebugConsole(const char *ip_str, int port)
{
	unsigned int level;

	_CPU_ISR_Disable(level);

	if (ip_str) {
		InitializeNetwork(ip_str, port);
		// Print connection status BEFORE redirecting stdout
		if (socket >= 0) {
			printf("DEBUG: Network connected to %s:%d\n", ip_str, port);
		} else {
			printf("DEBUG: Network connection FAILED to %s:%d (check IP, run nc -l -p %d)\n", ip_str, port, port);
		}
	}

	devoptab_list[STD_OUT] = (const devoptab_t *)&dotab_netout;
	devoptab_list[STD_ERR] = (const devoptab_t *)&dotab_netout;

	_CPU_ISR_Restore(level);

	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	printf("Debug Console Connected\n");
}

void Init_DebugConsole_Shutdown() {
	net_deinit();
	socket = -1;
	connection_failed = 0;
}

int DebugConsole_GetStatus() {
	return socket >= 0 ? 1 : (connection_failed ? -1 : 0);
}
