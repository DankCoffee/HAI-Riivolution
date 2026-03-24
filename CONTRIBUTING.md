# Contributing to HAI-Riivolution

## Commit Message Format

All commit messages MUST follow this format:

```
[COMPONENT/TYPE] Brief description in useful English

Optional detailed explanation of what changed and why.
Can span multiple lines.
```

### Component Tags

Use the component that best represents where the changes were made:

- `LAUNCHER` - Riivolution launcher application
- `RAWKSD` - RawkSD application
- `DIPMODULE` - DIP (Disc Interface) IOS module
- `FILEMODULE` - File system IOS module
- `MEGAMODULE` - Debugger IOS module
- `LIBIOS` - Core IOS library
- `LIBWIIDRC` - Wii U GamePad input library
- `RIIFS` - RiiFS server (PC tool)
- `BUILD` - Build system, Makefiles, dependencies
- `DOCS` - Documentation files
- `TOOLS` - Other tools (stripios, dollz3, etc.)

### Type Tags

- `FEAT` - New feature or functionality
- `FIX` - Bug fix
- `REFACTOR` - Code refactoring (no functional changes)
- `PERF` - Performance improvement
- `DOCS` - Documentation changes
- `TEST` - Adding or updating tests
- `CHORE` - Maintenance tasks (dependencies, formatting, etc.)
- `HAXX` - IOS exploit/kernel patching changes

### Multiple Components

If changes span multiple components, use the primary component or `BUILD`:

```
[BUILD/FEAT] Add support for IOS 58
```

Or make separate commits per component when possible.

## Examples

### Good Commit Messages

```
[LAUNCHER/FIX] Fix memory leak in XML parser

The riivolution_config parser was not freeing xmlDoc objects
after parsing, causing memory exhaustion on complex patches.
```

```
[DIPMODULE/FEAT] Add USB keyboard support to HID driver

Implements HID keyboard protocol in usbhid.cpp to support
USB keyboards for text input in games.
```

```
[LIBWIIDRC/FIX] Correct analog stick deadzone calculation

Previous deadzone was too aggressive, causing drift issues.
Changed threshold from 0x200 to 0x100.
```

```
[DOCS/CHORE] Update build instructions for Ubuntu 24.04
```

```
[HAXX/REFACTOR] Simplify IOS module loading logic
```

**Note**: Do not include AI tool attributions or generation notices in commit messages. Focus on the technical changes and their impact.

### Bad Commit Messages

```
fix bug
```
*Missing tag and too vague*

```
[launcher] various updates
```
*Wrong tag format (lowercase), not descriptive*

```
Update haxx.cpp
```
*Missing tag, doesn't explain what changed*

```
[LAUNCHER] more stuff
```
*Not useful English, too vague*

## General Guidelines

1. **First line**: Keep under 72 characters
2. **Tag**: Always use `[COMPONENT/TYPE]` format in UPPERCASE
3. **Description**: Use imperative mood ("Add feature" not "Added feature")
4. **Be specific**: Describe what changed and why, not just what file
5. **Body**: Add detailed explanation if the change isn't obvious
6. **References**: Include issue numbers if applicable

## Build Requirements

**IMPORTANT**: Building this project requires the devkitPPC Docker container.

- Do NOT build directly on host system
- Use the devkitpro/devkitppc Docker image
- See README.md for Docker setup instructions
- All dependencies must be installed in container

## Setting Up Commit Template (Optional)

You can configure git to use a commit message template:

```bash
git config commit.template .gitmessage
```

This will pre-fill your commit messages with the correct format.
