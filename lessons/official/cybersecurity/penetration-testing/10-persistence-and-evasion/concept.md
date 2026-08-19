# Persistence and Evasion

A shell that vanishes on reboot is a temporary advantage. Persistence keeps
access after the original exploit is patched, the service restarts, or the
host reboots. Evasion keeps that access from being detected while it exists.

For a penetration tester these are not offensive play. They are the last
technical phases of the methodology: demonstrating how far access survives,
and proving that the controls intended to catch it can be defeated. Both are
findings when they succeed.

## What persistence is

Persistence is a mechanism that restores access without repeating the original
attack.

```text
original access: an exploit that runs once
persistent access: a backdoor that runs on its own later
```

A persistence mechanism survives at least one of:

```text
a reboot
a service restart
a process crash
a user logging out
```

The mechanisms all follow one pattern: the host already runs something
automatically, and the tester arranges for that something to run the tester's
code as well.

## Persistence mechanisms on Linux

The common Linux mechanisms are:

```text
systemd units: services that start at boot or on an event
cron jobs: commands that run on a schedule
shell profiles: commands that run at login
SSH keys: permanent remote access without a password
init scripts and startup hooks: run during boot
```

Each mechanism runs with the privileges of its configured user. A systemd
unit installed as root runs as root at boot. A cron job for a user runs as
that user. The tester chooses the mechanism that matches the access already
held and the availability the client wants demonstrated.

## SSH keys as persistence

An SSH key is the most direct persistence on a Linux host.

```text
generate a key pair on the attacker
append the public key to the target's authorized_keys
connect to the target whenever needed
```

The authorized_keys file controls which keys may log in. Appending to it is
the classic persistence move: quiet, effective, and independent of exploits,
services, or reboots. In an engagement, the key is registered as evidence and
removed at the agreed cleanup step.

## Hiding through legitimate mechanisms

Evasion starts with not standing out. A mechanism that uses the system's own
facilities is harder to spot than a novel binary.

```text
a systemd unit named like an existing service
a cron entry that looks like routine maintenance
a process name that matches common processes
```

The rule is that the host should look the same to a defender as it did before.
Behavior that matches the baseline is behavior that is not investigated.

## Logs and the audit trail

Every action writes to the host's logs. The tester must know what the action
leaves behind.

```text
authentication logs: who logged in and how
command history: what commands the shell recorded
process accounting: what executed
file access: what was read or changed
```

Evasion means managing the trail deliberately, not deleting everything.
Deleting logs wholesale is itself a giant alert; editing selectively is both
more effective and more dangerous to do wrong. In the lab the actions are
practiced so the tester understands what a defender would see.

## Payload encoding

Detection often keys on known payload signatures. Encoding changes the bytes
without changing the behavior.

```text
base64: hide the payload in a text blob
variable renaming: defeat string matching
staged delivery: keep the payload off disk
```

Encoding is not the same as encryption. A determined analyst decodes base64
quickly. The point of encoding in a lab is to learn what a defender's
signatures actually look for, and to practice the tradecraft around payload
delivery.

## Living off the land

The least detectable code is code the host already runs.

```text
use bash, python, perl already present on the host
use system tools for the post-exploitation actions
avoid uploading new binaries
```

If the target has python, the tester uses python. If it has a package manager,
the tester uses it. Uploading a foreign binary is the one thing every defender
is looking for, so the smallest footprint is the footprint built from what is
already installed.

## Defender awareness

Evasion is measured against the target's actual defenses.

```text
what does the host log?
what does the network inspect?
what does the endpoint agent monitor?
```

A lab target with no monitoring teaches the mechanics but not the art. The
discipline is to practice as if a defender will review every artifact, because
on a real engagement one will.

## Ethics and scope of evasion

Evasion and persistence are the most sensitive phases in the methodology.

The rules are explicit:

```text
only against in-scope lab targets
only as far as the engagement permits
remove all persistence at cleanup
document every mechanism as a finding
```

Persistence that is left behind after the engagement is no longer a test; it
is an intrusion that the client did not authorize. The engagement contract
states when and how everything must be removed, and the tester's cleanup list
is part of the deliverable.

## Choosing what to persist

Persistence should match the demonstration the engagement requires.

```text
demonstrate access after reboot -> a boot-time mechanism
demonstrate remote re-entry     -> an SSH key or listener
demonstrate scheduled access    -> a cron or timer job
demonstrate stealth             -> the most legitimate-looking mechanism
```

Installing every possible backdoor is noisy and pointless. The tester selects
the one mechanism that proves the point, records it, and removes it later.
Relevance beats variety.

## The detection timeline

Evasion is a race against the defender's timeline.

```text
immediate: what the host logs during the action
short-term: what an agent or SIEM correlates in minutes
long-term: what an analyst finds during a deep review
```

A payload that evades a signature check today may be correlated by a log
review tomorrow. The lab teaches both sides of the race: how to reduce the
immediate footprint, and how to reason about what the audit trail will show.

## Cleanup

The engagement ends with cleanup, and cleanup is practiced in the lab.

```text
remove installed keys
disable and delete backdoor units
revert changed files
clear the changed log entries
confirm the host returns to its baseline
```

A clean host is a host the client can keep. The cleanup list is written when
the persistence is installed, so nothing is forgotten at the end.

## Next step

Now type the code version of this lesson: install persistence through several
mechanisms, practice encoding and living off the land, and execute a cleanup
pass against the lab target.