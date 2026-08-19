# Reconnaissance and OSINT - Typing

This lesson types a reconnaissance workflow: passive DNS lookups, subdomain
discovery, certificate transparency queries, and organizing the results into
an attack surface list. The example domain is target.lab, a stand-in for an
authorized in-scope target.

## Start with the base domain

Resolve the primary domain before expanding outward.

```bash
# resolve the A record for the base domain
dig +short target.lab A

# resolve the name servers for the domain
dig +short target.lab NS

# resolve the mail servers for the domain
dig +short target.lab MX
```

DNS records name the first set of hosts to enumerate.

## Ask for more record types

TXT records often reveal infrastructure choices.

```bash
# show TXT records for the domain
dig target.lab TXT +short

# show the start of authority record
dig target.lab SOA +short

# resolve the IPv6 address, if one exists
dig +short target.lab AAAA
```

Every record type is a data point about the target's layout.

## Verify hosts resolve

A domain can exist without resolving. Filter for names that actually answer.

```bash
# check each name in a list for a resolvable address
while read -r host; do
    ip=$(dig +short "$host" A | head -n 1)
    if [ -n "$ip" ]; then
        echo "$host -> $ip"
    fi
done < <(printf '%s\n' "app.target.lab" "vpn.target.lab" "git.target.lab")
```

Resolvable names become enumeration targets.

## Brute force subdomains

Use a wordlist to ask DNS about common names.

```bash
# create a small wordlist
cat > recon/wordlist.txt <<'EOF'
admin
api
app
blog
dev
git
mail
staging
test
vpn
www
EOF

# brute force the domain with the wordlist
while read -r word; do
    host="$word.target.lab"
    ip=$(dig +short "$host" A | head -n 1)
    if [ -n "$ip" ]; then
        echo "found: $host -> $ip"
    fi
done < recon/wordlist.txt
```

Longer wordlists exist, but the technique is the same.

## Query certificate transparency

The certificate transparency logs reveal every certificate ever issued for
the domain.

```bash
# query the certificate transparency database for the domain
curl -s "https://crt.sh/?q=%25.target.lab&output=json" \
    | jq -r '.[].name_value' \
    | sort -u
```

The result is a passive, exhaustive subdomain list.

## Clean and deduplicate results

Combine brute force and certificate results into one list.

```bash
# merge the certificate results into a file
curl -s "https://crt.sh/?q=%25.target.lab&output=json" \
    | jq -r '.[].name_value' \
    | tr 'A-Z' 'a-z' \
    | sort -u > recon/subdomains.txt

# add brute force results and deduplicate
while read -r word; do
    echo "$word.target.lab"
done < recon/wordlist.txt >> recon/subdomains.txt

# sort and remove duplicates
sort -u recon/subdomains.txt -o recon/subdomains.txt

# show the clean list
cat recon/subdomains.txt
```

A single deduplicated list feeds the next phase.

## Filter to live hosts

Not every subdomain in the list is alive. Filter for hosts that resolve.

```bash
# keep only subdomains that resolve to an address
while read -r sub; do
    ip=$(dig +short "$sub" A | head -n 1)
    if [ -n "$ip" ]; then
        echo "$sub $ip" >> recon/live-hosts.txt
    fi
done < recon/subdomains.txt

# display the resolved hosts
cat recon/live-hosts.txt
```

Live hosts are the enumeration targets for the next lesson.

## Search engines with dork patterns

Public indexes can reveal files and pages the target forgot.

```bash
# document the dork patterns to try in a browser
cat > recon/dorks.txt <<'EOF'
site:target.lab filetype:pdf
site:target.lab inurl:admin
site:target.lab intitle:error
site:target.lab filetype:env
site:target.lab inurl:backup
EOF

# show the dork list
cat recon/dorks.txt
```

Each promising result is saved with its URL and source.

## Try a zone transfer

A misconfigured name server can return its entire zone to anyone who asks.

```bash
# attempt a zone transfer from each name server
for ns in $(dig +short target.lab NS); do
    echo "trying $ns"
    dig @$ns target.lab AXFR +short
done
```

A successful transfer returns the full zone in one request.

## Whois and registrant data

Registries hold public records about the domain and its owner.

```bash
# query the whois service for the domain
whois target.lab | head -n 30

# query the owner of an IP address
whois 10.10.20.5 | grep -iE 'netname|org-name|descr'
```

Registrant records can expose names, emails, and the hosting provider.

## Record output with timestamps

Recon is only useful if the results survive to the next phase.

```bash
# timestamp every saved result
ts=$(date -u +%Y%m%dT%H%M%SZ)

# save the live host list with a timestamp header
{
    echo "# live hosts $ts"
    cat recon/live-hosts.txt
} > "recon/live-hosts-$ts.txt"

# keep the newest copy referenced by the phase folder
cp "recon/live-hosts-$ts.txt" recon/live-hosts.txt
```

A timestamped note is evidence the report can cite.

## Check documents for metadata

Files the target publishes often carry internal details in their metadata.

```bash
# install a metadata extraction tool
sudo pacman -S exiftool

# extract metadata from a downloaded document
exiftool recon/sample-report.pdf

# keep only the interesting fields
exiftool recon/sample-report.pdf \
    | grep -iE 'author|creator|producer|software'
```

Author names and producer software hint at usernames and the stack.

## Review the organized output

Confirm the recon structure before moving on.

```bash
# list everything collected so far
find recon -type f | sort

# summarize the live hosts
awk '{ print $2 }' recon/live-hosts.txt | sort | uniq -c
```

Recon output becomes enumeration input.

## Wrap up

Recon sequence: passive records -> subdomains -> certificate logs -> live
host list -> dorks -> organized notes.
