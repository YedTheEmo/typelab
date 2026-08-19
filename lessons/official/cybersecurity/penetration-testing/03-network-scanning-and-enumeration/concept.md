# Network Scanning and Enumeration

Reconnaissance produces a list of hosts and names. Enumeration is where that
list becomes a map of running services, versions, and configuration details.
Scanning finds the open doors; enumeration opens each door and reads what is
behind it.

This is the phase where the tester's Arch Linux box becomes active. Every
probe from here on generates log entries on the target side, so the scope and
timing rules from the first lesson apply with extra force.

## What enumeration answers

By the end of enumeration the tester should know, for every in-scope host:

```text
which hosts are alive
which ports are open
which service runs on each open port
which version that service reports
what the service configuration allows
```

A port scan answers the first three. Service detection answers the version.
Banner grabbing and protocol interaction answer the configuration.

The output of this phase is the attack surface:

```text
host 10.10.20.5
    ssh open, version OpenSSH 8.9
    http open, nginx 1.22
    http content: login page
```

## Host discovery

Before scanning ports, a tester finds which hosts respond at all.

A single host can be scanned directly, but a whole subnet is usually swept
first. Host discovery methods include:

```text
ping sweep (ICMP echo)
ARP scan (local link only)
TCP connect to common ports
no host discovery at all (scan everything)
```

A ping sweep is fast and quiet, but many hosts ignore ICMP. A TCP probe to a
few well-known ports is more reliable but noisier. On a local lab network an
ARP scan is definitive because every device answers ARP for its own address.

Host discovery is a triage step. It narrows the target list so the slower port
scans are spent on hosts that exist.

## Port scanning fundamentals

A port scan asks, for each of 65,535 TCP ports and 65,535 UDP ports, whether
the target is listening.

The two most common TCP scans are:

```text
connect scan: complete the TCP handshake
SYN scan: send SYN, read the SYN-ACK or RST, never finish
```

A connect scan uses the operating system's normal socket calls, so it appears
in the target's logs as an ordinary connection. A SYN scan is stealthier
because the handshake is never completed, and it is faster. A SYN scan needs
root privileges, which the attacker box has.

UDP scanning is slower and less reliable because UDP has no handshake. A
tester usually scans the small set of UDP ports that host common services.

## Scan timing and scope

Scanning generates traffic. The amount is controlled by timing templates and
by the target list.

Nmap timing templates range from paranoid to insane:

```text
paranoid     serial, minutes between probes
sneaky       serial, 15 seconds between probes
polite       parallel, under 10 packets per second
normal       the default
aggressive   fast, extra service detection
insane       very fast, likely to drop packets
```

The right template depends on the engagement. A defensive scan on a production
network uses polite or normal. A lab engagement can use aggressive without
consequence.

The scope rules still hold: scan only in-scope hosts, only during the allowed
window, and never scan out-of-scope ranges even when they appear next to
in-scope hosts.

## Service version detection

An open port is a door. The service behind it is the room behind the door, and
the version is the exact model of lock.

Version detection interrogates the service and reports:

```text
the service name
the application name and version
the operating system hints
any extra protocol data
```

The version matters because vulnerabilities are version-specific. A
vulnerability in OpenSSH 8.9 is not the same as one in OpenSSH 9.2. The
reconnaissance output, the vulnerability research, and the eventual exploit
all key off the exact version string.

## Operating system fingerprinting

The same version of a service can behave differently across operating systems.
TCP/IP behavior differs subtly, and fingerprinting reads those differences.

Operating system fingerprinting is an estimate, not a guarantee. Firewalls,
proxies, and load balancers can obscure the true host. Treat the result as a
hint that service enumeration confirms.

## Banner grabbing and manual checks

Automated detection is a start, but a tester also reads the service directly.

Banner grabbing connects to the service and reads its opening message. Many
services announce themselves:

```text
FTP announces its version on connect
SSH announces its software and version
HTTP announces its server header on request
SMTP announces its mail server
```

The manual step reveals details a scanner may miss, including login banners,
allowed commands, and configuration quirks. In a lab, the banner is often the
shortcut to the correct exploit.

## Protocol-specific enumeration

Each service family has its own enumeration patterns. The common ones on a
typical network are:

```text
HTTP(S): web root, directories, headers, application fingerprints
SSH: version, supported authentication methods
SMB: shares, users, anonymous access
NFS: exported filesystems, permissions
FTP: anonymous login, writable directories
SNMP: community strings, device information
SMTP: users, relay behavior, banner
```

The list is not exhaustive. The principle is that a service advertises its
own weaknesses: an SMB share lists names, a web server lists paths, an NFS
export lists directories.

## Scripts and automation

Banner and protocol checks are automatable. Scan frameworks bundle them into
libraries of small scripts that run against a service after the port is
found.

A typical automation flow:

```text
find open ports
    ->
run service detection
    ->
run scripts relevant to that service
    ->
collect results into a file
```

Scripts are valuable because they are consistent. They encode the checks a
tester would otherwise retype for every host, and they produce output in a
uniform format that feeds the report.

The risk is trusting script output without reading the underlying response.
A script saying a service is vulnerable is a starting point, not proof. The
manual check confirms it.

## Common ports as shortcuts

Familiarity with common ports makes scan output readable at a glance.

```text
21    FTP
22    SSH
23    Telnet
25    SMTP
53    DNS
80    HTTP
110   POP3
139   SMB (NetBIOS session)
143   IMAP
443  HTTPS
445   SMB (direct)
993   IMAPS
3306  MySQL
3389  RDP
5432  PostgreSQL
5985  WinRM
6379  Redis
8080  HTTP alternate
```

A non-standard port hosting a standard service is itself a finding. An
admin dashboard on port 8443, or a database reachable on the internet, is
exactly the kind of discovery enumeration is meant to surface.

## Organizing scan results

Scan output accumulates quickly and needs a home per host.

The working structure:

```text
enum/
    10.10.20.5/
        ports.txt
        services.txt
        smb.txt
    10.10.20.6/
        ports.txt
        services.txt
```

Each file is saved with the command that produced it. The next phase selects
targets from these files, so their accuracy decides the quality of everything
that follows.

## Next step

Now type the code version of this lesson: host discovery, port scanning,
service detection, and service-specific enumeration against the lab
targets.