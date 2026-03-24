---
description: Apply commit message rules and configure git commit template
---

Configure git to use the commit message template and verify the rules are set up correctly.

Run the following command to enable the commit template:

```bash
git config commit.template .gitmessage
```

Then verify the configuration:

```bash
git config commit.template
```

The commit message rules require the format: `[COMPONENT/TYPE] Brief description`

For full details, see CONTRIBUTING.md.
