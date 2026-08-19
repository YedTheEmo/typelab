# Password and Credential Attacks

Passwords are the weakest link on most networks. Every account, service, and
administrative interface is guarded by a secret, and secrets are chosen by
people. This lesson covers the two sides of credential attacks: trying
passwords against a live service, and cracking hashes that have already been
captured.

Both sides run against the Arch Linux attacker box, and both are governed by
the same scope rules as everything else.

## Where credentials come from

A credential attack rarely starts from nothing. The tester collects candidate
material first:

```text
usernames from recon and enumeration
default credentials for the discovered services
passwords found in documents and configuration files
password hashes captured from services or files
reused passwords across exposed services
```

The attack that works is usually the one built on evidence, not the one that
guesses blindly. A username discovered on the web service and a default
password for the same product is a finding waiting to happen.

## Online attacks

An online attack tries passwords against a live authentication service.

```text
service: ssh, web login, ftp, smb
input: a target service, a username, and a password list
output: which password, if any, authenticated
```

The attacker box runs a tool that connects, submits the candidates, and reads
the result. Each attempt is a real request to the service, which means online
attacks are:

```text
slow (network round trips)
observable (service logs every attempt)
risky (lockout policies can disable the account)
```

Because every attempt is a request, online attacks are bounded by the target's
behavior and by the scope rules, not by the attacker's speed.

## Lockouts and rate limits

Many services lock an account after repeated failures, and most rate-limit
connection attempts.

A lockout changes the engagement in two ways:

```text
the target account becomes unavailable to everyone
the finding becomes denial-of-service, not access
```

The tester therefore spaces attempts, watches for lockout responses, and
prefers a small, targeted list over a massive one. When a lockout policy is
suspected, the scope document should already say whether a lockout is
acceptable. In the lab, the rule is simply: do not hammer the target into a
state where legitimate use fails.

## Wordlists

A password list is only as good as its contents. Common sources:

```text
rockyou: the classic breached-password wordlist
SecLists: curated lists by category and language
custom lists built from the target's own data
```

The most effective list for an engagement is often a small custom one: the
company name, the product name, the current season and year, and common
variations. A targeted list of a few hundred candidates frequently outperforms
a multi-gigabyte generic list, because it matches the way humans choose
passwords.

## Offline cracking

Offline cracking works on hashes that have already been captured. The attacker
tries candidates, hashes each one, and compares against the captured hash.

```text
captured hash:  the password's digest
candidate:      a guessed password
hash(candidate): computed digest
match:          candidate is the password
```

Offline cracking has one decisive advantage over online attacks: there is no
network, no service, no lockout, and no log entry per attempt. The only limit
is the attacker's hardware and the hash's design.

## Hash identification

Not every hash is cracked the same way. The algorithm decides the speed.

A tester first identifies the format:

```text
$6$...     SHA-512 crypt (Linux shadow)
$2y$...    bcrypt (web frameworks)
$1$...     MD5 crypt
NT hash    Windows NTLM
```

The identifier prefixes reveal the algorithm. The algorithm sets the
cracking speed: MD5 and NT hashes are fast, SHA-512 is slower, and bcrypt is
deliberately slow. The correct mode for the tool must match the algorithm, so
identification comes before any cracking attempt.

## Cracking methodology

The order of attempts follows probability, not effort:

```text
try the obvious first: names, defaults, target-derived words
apply a ruleset to generate variations
use a targeted wordlist
use a large generic wordlist
escalate to brute force only if needed
```

A ruleset transforms base words into typical password patterns: appending a
digit, capitalizing, doubling, appending the year. The value of rules is that
they multiply a small wordlist into the forms people actually use without
filling the disk.

## Pass-the-hash

Some authentication protocols let a tester authenticate with the hash alone,
without ever learning the password.

Windows NTLM is the classic case: the domain accepts the NT hash as proof of
identity. A captured hash becomes a working credential:

```text
capture an NT hash from memory or a file
present the hash directly to the service
authenticate as the account without the password
```

Pass-the-hash turns a stolen digest into an actionable credential. The same
technique exists for some other protocols, so the concept generalizes:
knowing the authentication material, even in hashed form, can be enough to
log in.

## Where hashes are captured

Hash capture is a post-exploitation activity, but the attack starts here.

```text
memory: credentials cached by running processes
files: /etc/shadow, configuration files, backup archives
databases: password columns and user tables
logs: scripts that embed credentials
```

Each capture is a credential that feeds the offline cracker. This is why the
credential phase connects the earlier stages: enumeration found the services,
exploitation got the shell, and the shell exposes the hashes.

## Scope and safety

Credential attacks are active and noisy. The rules:

```text
test only accounts and services in scope
avoid destructive lockouts
do not use cracked credentials outside the engagement
protect captured hashes as sensitive evidence
```

Captured credentials are client data. They are recorded only as long as the
engagement needs them and handled with the same care as any other finding.

## Next step

Now type the code version of this lesson: online attacks with hydra, hash
identification, offline cracking with john and hashcat, and pass-the-hash
against the lab targets.