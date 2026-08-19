# Network Scanning and Enumeration - Typing

This lesson types a complete enumeration pass against the lab targets
10.10.20.5 and 10.10.20.6 from the Arch Linux attacker box. Every result is
saved so the next phase can use it.

## Prepare a target file

Scan input should come from the recon output.

```bash
# record the in-scope targets
cat > enum/targets.txt <<'EOF'
10.10.20.5
10.10.20.6
EOF

# make a folder for each target
for host in $(cat enum/targets.txt); do
    mkdir -p "enum/$host"
done
```

Each host gets its own evidence folder.

## Host discovery

Find which hosts in the lab subnet respond.

```bash
# ICMP sweep of the lab subnet
nmap -sn 10.10.20.0/24

# TCP probe to common ports as a fallback
nmap -sn -PS22,80,443,445 10.10.20.0/24
```

Alive hosts continue to the port scan.

## Port scan with version detection

Scan the known targets for open TCP ports.

```bash
# SYN scan with service version detection on the first host
sudo nmap -sS -sV 10.10.20.5 -oA enum/10.10.20.5/ports

# scan the second host
sudo nmap -sS -sV 10.10.20.6 -oA enum/10.10.20.6/ports
```

The -oA flag writes normal, grepable, and XML output.

## Check the open ports

Read the scan results before choosing next steps.

```bash
# show the open ports and versions
cat enum/10.10.20.5/ports.nmap

# list just the service versions
grep -E '^[0-9]+/(tcp|udp)' enum/10.10.20.5/ports.nmap
```

The version strings become the research input for later phases.

## Scan specific ports across hosts

A focused scan on a port that matters can be run across all targets at once.

```bash
# scan HTTP on both hosts
sudo nmap -p 80,443 -sV -sC 10.10.20.5 10.10.20.6 -oA enum/http-scan
```

The -sC flag runs the default set of enumeration scripts.

## Operating system fingerprinting

Estimate the operating system behind the services.

```bash
# OS detection against the first host
sudo nmap -O 10.10.20.5 -oN enum/10.10.20.5/os.txt

# show the guessed operating system
grep -A 2 'OS details' enum/10.10.20.5/os.txt
```

OS detection is a hint, not a fact.

## Banner grabbing

Read what the services announce when they answer.

```bash
# grab the SSH banner
nc -w 3 10.10.20.5 22

# grab the HTTP server header
curl -sI http://10.10.20.5 | grep -i '^server:'

# grab an FTP banner, if port 21 is open
nc -w 3 10.10.20.5 21
```

Banners often confirm the version a scanner reported.

## Enumerate the web service

The web service is a full application and gets its own attention.

```bash
# fetch the root page and save it
curl -s http://10.10.20.5 -o enum/10.10.20.5/index.html

# show the page title and any links
grep -oE '<title>[^<]+' enum/10.10.20.5/index.html

# list common paths using a small wordlist
cat > enum/10.10.20.5/paths.txt <<'EOF'
/
/admin
/login
/robots.txt
/sitemap.xml
/backup
/.git/HEAD
EOF

# probe each path
for path in $(cat enum/10.10.20.5/paths.txt); do
    code=$(curl -s -o /dev/null -w '%{http_code}' "http://10.10.20.5$path")
    echo "$code $path"
done
```

A directory listing or a backup file is an immediate target.

## Enumerate SMB

If port 445 is open, enumerate shares and users.

```bash
# list the SMB shares
smbclient -L //10.10.20.5 -N

# connect to an anonymous share and list its contents
smbclient //10.10.20.5/share -N -c 'ls'
```

Anonymous SMB access is a common lab finding.

## Enumerate NFS

If port 2049 is open, list the exported filesystems.

```bash
# show the exported NFS filesystems
showmount -e 10.10.20.5

# try to mount a writable export
sudo mkdir -p /mnt/lab-nfs
sudo mount -t nfs 10.10.20.5:/srv/data /mnt/lab-nfs

# list the mounted files
ls -la /mnt/lab-nfs
```

A world-writable NFS export is frequently the path to a shell.

## Enumerate SNMP

If UDP port 161 is open, read the device's public community string.

```bash
# walk the SNMP tree with the default community
snmpwalk -v2c -c public 10.10.20.5

# extract the system information only
snmpwalk -v2c -c public 10.10.20.5 1.3.6.1.2.1.1
```

SNMP can disclose users, processes, and network configuration.

## Scan all TCP ports

A default scan covers the most common 1000 ports. A thorough engagement scans
every port on the important targets.

```bash
# scan all 65535 TCP ports with version detection
sudo nmap -p- -sV 10.10.20.5 -oA enum/10.10.20.5/ports-all

# compare against the first scan for new ports
grep -E '^[0-9]+/(tcp|udp)' enum/10.10.20.5/ports-all.nmap
```

A service on a high port is easy to miss with default scans.

## Targeted UDP scan

UDP services do not answer with a handshake, so they need a dedicated scan.

```bash
# scan common UDP ports on the target
sudo nmap -sU --top-ports 50 10.10.20.5 -oA enum/10.10.20.5/ports-udp

# list any open UDP ports
grep -E '^[0-9]+/udp' enum/10.10.20.5/ports-udp.nmap
```

SNMP, DNS, and TFTP are the usual finds on UDP.

## Control scan timing

Timing keeps the scan respectful of the network and the scope rules.

```bash
# polite timing for a careful first pass
sudo nmap -T2 -sV 10.10.20.5

# aggressive timing for a lab engagement
sudo nmap -T4 -sV 10.10.20.5
```

Choose timing by the engagement, not by habit.

## Run targeted scripts

The default script set is a good start, but specific services deserve specific
scripts.

```bash
# run the HTTP title and server-header scripts
nmap -p 80,443 --script http-title,http-server-header 10.10.20.5

# enumerate SMB shares with the dedicated script
nmap -p 445 --script smb-enum-shares 10.10.20.5

# check the SSH host key and authentication methods
nmap -p 22 --script ssh2-enum-algos 10.10.20.5
```

Script output is a starting point for manual verification.

## Filter scan output

Convert raw scan output into a clean list for the report.

```bash
# extract host, port, and service into a table
awk '/^[0-9]+\/(tcp|udp)/ { print $1, $3, $4 }' \
    enum/10.10.20.5/ports.nmap

# save the table
awk '/^[0-9]+\/(tcp|udp)/ { print $1, $3, $4 }' \
    enum/10.10.20.5/ports.nmap > enum/10.10.20.5/services.txt

# show the resulting table
cat enum/10.10.20.5/services.txt
```

A clean table is the attack surface the next phase works from.

## Enumerate FTP

An FTP server that allows anonymous access hands over its files freely.

```bash
# test anonymous login on the FTP service
ftp -n 10.10.20.5 <<'EOF'
user anonymous anonymous
ls
quit
EOF

# if the service accepts connections, list the writable state
echo 'user anonymous anonymous' | timeout 5 openssl s_client \
    -connect 10.10.20.5:990 -quiet 2>/dev/null | head
```

Anonymous FTP is a quick win when the enumeration shows port 21.

## Review the enumeration output

Confirm every target has evidence before moving on.

```bash
# show the complete enumeration tree
find enum -type f | sort

# summarize open services per host
for host in $(cat enum/targets.txt); do
    echo "== $host =="
    grep -E '^[0-9]+/(tcp|udp)' "enum/$host/ports.nmap"
done
```

The attack surface is now concrete.

## Wrap up

Enumeration sequence: targets -> host discovery -> port scan -> versions ->
banners -> service-specific checks -> organized evidence.
