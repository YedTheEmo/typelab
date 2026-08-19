# Privilege Escalation

Most exploits land on the target as an unprivileged user: the web application's
service account, or a low-privilege login. The valuable data on a Linux host is
almost always owned by root. Privilege escalation is the phase that closes that
gap: turning an unprivileged foothold into root, or into a more privileged
account.

This lesson covers the Linux privilege-escalation methodology that a tester
runs from a shell on the target.

## Vertical and horizontal escalation

Privilege escalation comes in two directions.

Vertical escalation raises the privilege level:

```text
www-data -> root
```

Horizontal escalation moves to a different account at the same level:

```text
www-data -> a more interesting user account
```

Horizontal moves matter because users can hold data and credentials that a
service account does not. The end goal is usually vertical: root, because root
can read everything.

## The escalation mindset

The shell on the target is a starting point for investigation, not a finished
product. The tester asks:

```text
what can this user read?
what can this user write?
what runs with higher privileges?
what did the administrator leave misconfigured?
```

The answers come from the host's own configuration: file permissions, sudo
rules, scheduled jobs, and running services. Privilege escalation is almost
always a configuration problem before it is a kernel problem.

## Automated enumeration

The first pass on a fresh shell is automated enumeration.

A script such as linpeas collects the interesting facts in one run:

```text
SUID and SGID binaries
sudo privileges
writable files and directories
cron jobs and their scripts
services running as root
kernel and distribution version
cached credentials
```

Automation is a time-saver, not a substitute for thinking. The script's
output is scanned for lines that match known escalation patterns. Each
candidate is then verified by hand, exactly like a scanner finding in the
vulnerability phase.

## Manual enumeration

The manual checks confirm and extend the automated output.

The core manual commands ask the same questions the script asked, but with
the tester reading the raw answers:

```text
whoami, id: the current identity and groups
sudo -l: what this user may run as root
find SUID binaries: binaries that run with their owner's privileges
crontab -l: the user's scheduled jobs
```

Manual reading matters because a script can miss context. A sudo rule that
permits one command is only interesting if that command can be abused.

## SUID binaries

A SUID binary runs with the privileges of its owner. When the owner is root,
the binary runs as root no matter who launches it.

```text
-rwsr-xr-x root root /usr/bin/somebinary
```

The setuid bit (the s in the owner permissions) is the marker. A SUID binary
that can execute commands, read files, or write anywhere is an escalation
path. Even a SUID binary that is not dangerous itself can be dangerous when
it is older than a known vulnerability.

## Sudo configuration

The sudo rule lists what a user may run as root.

A rule like:

```text
lab-user ALL=(ALL) NOPASSWD: /usr/bin/vim
```

lets lab-user run vim as root without a password. A text editor can open and
edit any file, so the rule is effectively root. The tester reads the rule,
finds the command it permits, and checks whether that command can escape into
a shell or file access.

## Capabilities

Linux capabilities grant specific privileges to binaries without full root.

```text
cap_setuid: a binary can change its effective user id
cap_net_raw: a binary can open raw sockets
cap_dac_read_search: a binary can bypass file read permissions
```

A binary with cap_setuid can be abused to run a shell as root. Capabilities
are checked with the same find-based approach used for SUID.

## Cron jobs

Scheduled jobs run as their configured user, often root.

A cron job is interesting when the tester can influence what it runs:

```text
the script file is writable by the tester
the script references a command from a writable directory
the script's directory is writable and in the PATH it uses
```

When a root cron job executes a file the tester can modify, the modified
content runs as root on the next schedule.

## Writable paths and services

A service configured to run as root inherits root for whatever it executes.

```text
systemd units with an executable that is writable
PATH entries that a root process searches before the real command
world-writable files used by a root process
```

The pattern is the same as cron: find a higher-privileged process that reads
or runs something the tester can control, then control it.

## Kernel exploits

Kernel exploits are the last resort, not the first tool.

A kernel vulnerability is a single CVE against the running kernel version,
and its exploit runs code with the kernel's privileges. The risks are real:

```text
kernel exploits can crash the host
the exact version and build must match
mitigations can make the exploit fail
```

The tester uses a kernel exploit only when the configuration paths are
exhausted, the target is a disposable lab host, and the version matches
exactly.

## Confirming the result

Escalation is confirmed the same way every other step is confirmed.

```text
id: the new identity
whoami: the current user
cat /root/proof.txt: access to root-only data
```

The command output is the evidence. The report records the escalation path:
the misconfiguration found, the command used, and the result.

## Credentials on the host

A host holds its own credentials, and a low-privilege shell can often read
them.

```text
/home/user/.ssh: private keys for other accounts
history files: passwords typed into commands
configuration files: embedded passwords and API keys
processes: environment variables of running services
database files: application user hashes
```

Reading these is not separate from privilege escalation. A stolen SSH key or
a service password can move the tester horizontally, and a service password
is often the same as the administrator's password.

## An escalation checklist

A working order of checks keeps the process disciplined:

```text
who am I and what groups do I have?
what can I run with sudo?
which binaries are setuid or carry capabilities?
what cron jobs run and who owns their scripts?
which services run as root and what do they execute?
what credentials are readable from my current account?
what kernel is running and does a matching exploit exist?
```

Each answer is either a dead end or the next step. Checking the list in
order catches the easy wins before the risky ones.

## Documenting the path

The escalation is not complete until it is written down.

```text
the user the shell started as
the misconfiguration discovered
the exact commands used
the resulting identity
the evidence files captured
```

A repeatable escalation path is a finding the client can fix. A vague note
that the tester "became root" cannot be remediated or reproduced.

## Next step

Now type the code version of this lesson: run automated and manual enumeration
from the foothold, find the misconfiguration, and escalate to root with
evidence.