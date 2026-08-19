# Pivoting and Tunneling

A shell on one host is rarely the end of an engagement. Networks are layered,
and the valuable systems usually sit behind the one the tester just broke
into. Pivoting is the art of using a compromised host as a stepping stone to
reach the hosts behind it.

This lesson covers the two ideas that make pivoting work: tunneling, which
carries the attacker's traffic through the compromised host, and proxying,
which lets the attacker's tools speak through that tunnel.

## The network layout

Consider a typical lab layout:

```text
attacker 10.10.20.25
    |
    v
compromised host 10.10.20.5
    |
    v
internal network 172.16.30.0/24
    |
    +--> internal web 172.16.30.10
    |
    +--> database 172.16.30.15
```

The attacker can reach the compromised host but cannot reach the internal
network directly. The compromised host can reach both. The pivot uses the
compromised host as a relay between the two worlds.

## What a tunnel is

A tunnel is an encrypted connection through which other traffic flows.

The classic example is SSH: the attacker connects to the compromised host
over SSH and carries further traffic inside that connection. From the
compromised host's perspective, all the relayed traffic looks like SSH
traffic, which is usually allowed and rarely inspected.

A tunnel has two ends:

```text
the local end: where the attacker's traffic enters
the remote end: where the traffic exits into the internal network
```

The tunnel hides the relayed traffic inside a permitted protocol. That is the
whole point of tunneling for pivoting.

## SSH local forwarding

SSH local forwarding exposes a remote service as a local port.

```text
attacker connects to the compromised host with -L
attacker's local port forwards to an internal address:port
```

Example:

```text
ssh -L 8080:172.16.30.10:80 user@10.10.20.5
```

After this, the attacker's browser at localhost:8080 reaches the internal web
server's port 80 through the tunnel. The SSH client on the attacker's side
listens on port 8080, forwards the bytes through the encrypted connection to
the compromised host, and the compromised host delivers them to the internal
target.

Local forwarding is for reaching a specific internal service.

## SSH remote forwarding

SSH remote forwarding works in the opposite direction: it exposes the
attacker's machine to a service on the compromised side.

```text
ssh -R 9000:localhost:4444 user@10.10.20.5
```

Now anything on the compromised host that connects to its localhost:9000 is
forwarded back through the tunnel to the attacker's port 4444. This is the
classic way to receive a reverse shell that would otherwise be blocked: the
compromised host dials a local port, and the tunnel delivers the connection
to the attacker's listener.

## SSH dynamic forwarding

Dynamic forwarding turns the SSH connection into a general-purpose proxy.

```text
ssh -D 1080 user@10.10.20.5
```

The attacker's machine listens on port 1080 as a SOCKS proxy. Any tool that
supports SOCKS can send its traffic through that proxy, and the proxy forwards
each connection out of the compromised host into its network.

Dynamic forwarding is the flexible pivot: instead of mapping one service at a
time, it gives every SOCKS-capable tool access to the entire internal network.

## Proxy chains

Not every tool speaks SOCKS. A proxy chain layer routes arbitrary connections
through the SOCKS proxy.

The common setup:

```text
application -> proxychains -> SOCKS proxy -> compromised host -> internal
```

The application believes it is talking directly to its target. Proxychains
receives the connection, wraps it, and sends it through the proxy. A command
run under proxychains behaves as if it originated from the compromised host.

Proxychains is the glue that lets nmap, curl, and other tools use the pivot
without rewriting their networking.

## Chisel for flexible pivots

SSH is not always available on the compromised host. Chisel provides the same
tunneling through a single binary that works over raw TCP.

Chisel has two roles:

```text
server: runs on the attacker side
client: runs on the compromised host
```

The client connects out to the server, and the server exposes forwarding or
SOCKS endpoints. Because the connection is outbound, chisel works even when
the target allows no inbound connections. It is the tool of choice when SSH
is blocked or unavailable.

## Finding the internal network

A tunnel needs a destination. The compromised host reveals its own network
view first.

```text
ip addr: the host's interfaces and addresses
ip route: the networks the host can reach
arp cache: neighbors the host has already spoken to
DNS and hosts files: internal names and addresses
```

The host's own routes are the map of what exists behind it. An interface on
172.16.30.0/24, or a route toward that range, tells the tester which network
to enumerate through the pivot. The discovery is passive and safe: it reads
the host's own state.

## Choosing the tunnel type

The right tunnel depends on what the pivot must carry.

```text
one specific service  -> SSH local forwarding
reverse connection    -> SSH remote forwarding
many services         -> SSH dynamic forwarding (SOCKS)
no SSH available      -> chisel
```

A tester chooses the smallest tunnel that does the job. A single internal web
service needs a local forward, not a SOCKS proxy for the whole network.

## Scanning through the pivot

Once a tunnel or proxy exists, enumeration continues from inside.

```text
nmap through the proxy against the internal subnet
curl through the proxy against internal web services
credential tools through the proxy against internal services
```

The results build a second enumeration table, this time for the internal
network. The same methodology from the earlier lessons applies; only the
network path changed.

## Lateral movement

Lateral movement is the act of hopping from the compromised host to another
host in the network.

Typical lateral moves:

```text
reuse a recovered credential on an internal service
exploit a service that only the internal network can reach
use the tunnel to reach an internal management interface
```

Each hop extends the engagement deeper. The evidence for a hop is the same as
the evidence for the original breach: the command, the output, and the
resulting access.

## Safety through the pivot

Pivoting multiplies the reach of every action, so the scope rules matter more,
not less.

```text
scan only in-scope internal ranges
do not pivot into systems outside the engagement
stop at the agreed depth
document each hop as evidence
```

The compromised host is the client's system, and every action through it is an
action on the client's network. The discipline of confirming each step applies
exactly as it did at the perimeter.

## Next step

Now type the code version of this lesson: SSH local, remote, and dynamic
tunnels, proxy chains, chisel, and internal enumeration through the pivot.