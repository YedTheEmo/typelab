# Wireless and WPA Attacks

Wireless is the network that is literally in the air. A wired network requires
physical access or a foothold; a wireless network broadcasts its presence and
invites proximity. This makes wireless a distinct attack surface with its own
methodology, tools, and risks.

This lesson covers wireless testing from the Arch Linux attacker box: putting
a wireless interface into monitor mode, finding networks, capturing the
material needed to test the password, and attacking the cryptographic
exchange.

## The wireless rule of scope

Wireless testing has an extra scope problem: radio does not respect network
boundaries.

A wireless scan sees every network within range, not just the client's.
Testing a network that is not in the engagement is unauthorized regardless of
proximity. The rule is absolute:

```text
test only the access points named in the scope
ignore every other network the radio sees
```

The lab for this course uses an access point the tester controls, which makes
every attack legal and every packet a practice packet.

## The wireless stack

A wireless interface is a network interface with two relevant modes:

```text
managed mode: the normal client mode, associated with an access point
monitor mode: captures all nearby frames without associating
```

Attacks start in monitor mode. In managed mode the card only receives its own
traffic; in monitor mode it receives every frame on the channel. Most attacks
need monitor mode, and some cards or drivers cannot enter it at all, so the
first check is whether the hardware cooperates.

## Discovering networks

The first wireless step is a survey: what networks are present, on which
channels, with which security settings.

A survey reports per network:

```text
the SSID (the network name)
the BSSID (the access point's address)
the channel
the encryption type
the signal strength
```

The encryption type is the most important line. It decides which attack
applies: WEP is broken by design, WPA2 uses a handshake that can be captured
and cracked, and WPA3 changes the exchange enough to resist the classic
handshake attack.

## The WPA2 handshake

WPA2-Personal protects the network with a passphrase. The security rests on
the four-way handshake that a client performs when it joins.

The handshake's key property for the tester:

```text
the passphrase never travels over the air
instead, the parties exchange data derived from it
the exchange includes a value that proves knowledge of the passphrase
```

The tester captures the handshake frames. The captured exchange can then be
tested offline against candidate passphrases: derive the expected values from
each candidate and compare them to the captured data. A match reveals the
passphrase.

## Capturing the handshake

The handshake occurs when a client joins the network. The tester must be
listening at the right moment on the right channel.

Two ways to obtain a handshake:

```text
passive: wait for a legitimate client to connect
active: send deauthentication frames to force a reconnect
```

Passive capture is clean but slow. Deauthentication is fast but loud: the
access point is told to drop a client, the client reconnects, and the
reconnect produces a fresh handshake in front of the listener. Deauthentication
is an active denial action, so it is practiced only against the tester's own
lab access point.

## Cracking the passphrase

The captured handshake is cracked like any other captured secret.

```text
wordlist: candidate passphrases
compute the derived key material for each candidate
compare against the captured handshake
match: the passphrase
```

WPA2 passphrase cracking is offline, so there is no service to lock and no
network traffic per attempt. The limiting factors are the wordlist and the
hardware. The dedicated tool for the handshake format is aircrack-ng, and
hashcat supports the format for GPU cracking.

## The PMKID attack

The PMKID attack captures a different value and works without a client.

```text
a value derived from the passphrase and the access point
sent by some access points in the first message of the handshake
testable offline without waiting for a client
```

When the access point leaks the PMKID, the tester does not need a
deauthentication and does not need to wait. The capture is a single frame, and
the cracking proceeds against the PMKID value instead of the handshake.

## WPS attacks

Wi-Fi Protected Setup offers a shortcut for joining a network, and shortcuts
are attackable.

```text
WPS PIN: an eight-digit pin, brute-forceable in halves
WPS push-button: relies on physical presence
```

The eight-digit PIN is split into two halves of four and three digits plus a
checksum, which reduces the search space dramatically. A PIN attack can
recover the passphrase without any client and without any handshake. WPS has
been disabled on many modern access points, so its presence is itself a
finding.

## Enterprise networks

Wireless security splits into two families:

```text
personal: one shared passphrase (WPA2-Personal, WPA3-Personal)
enterprise: per-user credentials through a RADIUS server (802.1X)
```

Enterprise networks are tested differently. The weakness is rarely the
wireless crypto; it is the identity layer: unauthenticated probing, credential
validation against the internal RADIUS, and the fallback behavior of the
supplicant. An enterprise test in the lab validates the tester's own RADIUS
setup before any real-world application.

## What wireless testing proves

A wireless finding in a report says something concrete:

```text
the network's passphrase was recovered
the network accepts a predictable passphrase
the network leaks a value that enables offline attack
the network uses a broken configuration
```

The impact is that anyone within radio range can join the network. The
remediation is a stronger passphrase, a current protocol, and disabled
legacy features. The tester demonstrates the weakness against the lab access
point and documents the exact attack used.

## Next step

Now type the code version of this lesson: put the wireless interface into
monitor mode, survey the lab network, capture a handshake, and crack the
passphrase against the lab access point.