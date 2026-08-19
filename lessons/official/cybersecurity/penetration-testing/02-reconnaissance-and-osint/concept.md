# Reconnaissance and OSINT

Reconnaissance is the first phase of an engagement. It is the process of
gathering information about a target before touching it. Done well, recon
turns a blind search into a list of concrete attack surfaces. Done poorly, it
sends the tester into enumeration with nothing to look for.

Open-source intelligence, or OSINT, is the branch of recon that uses publicly
available sources. In a properly scoped engagement, OSINT is a powerful head
start because it is information the target has already published or leaked,
sometimes without realizing it.

On the Arch Linux attacker machine, recon is a collection of small tools and
one consistent habit: record everything, because every result becomes a target
for the next phase.

## Passive versus active recon

The most important split in reconnaissance is between passive and active
methods.

Passive recon never contacts the target. It reads DNS records, certificate
transparency logs, search engines, registries, and cached content. Because the
target is not contacted, passive recon is quiet and low-risk.

Active recon contacts the target or its infrastructure directly. It queries
DNS servers, loads web pages, or probes services. Active recon generates
log entries on the target side and can alert defenders.

The order matters:

```text
passive recon
    ->
active recon
    ->
enumeration
```

Exhaust passive sources before becoming active. The passive results tell the
tester which active queries are worth making.

## The goal of reconnaissance

Reconnaissance produces an inventory. The inventory should answer:

```text
What domain names belong to the target?
What hosts are associated with those domains?
What services do those hosts likely expose?
What technologies do those services use?
What people and roles are visible?
What credentials or documents have leaked?
```

Every answer is a potential entry point. A domain name that resolves nowhere is
a dead end. A subdomain running a forgotten admin panel is a finding.

A useful recon output is a table:

```text
domain        host                 purpose            tech hints
target.lab    app.target.lab       main application   web server
target.lab    git.target.lab       internal git       git service
target.lab    vpn.target.lab       remote access      vpn software
```

## DNS reconnaissance

Domain name system records are a public map of the target's naming. Common
record types are:

```text
A     IPv4 address of a host
AAAA  IPv6 address of a host
NS    authoritative name servers
MX    mail exchange servers
TXT   arbitrary text, including SPF and DKIM
CNAME alias to another name
SOA   start of authority for the zone
```

The same domain can appear in many record types, and each type reveals
infrastructure. MX records expose mail servers. NS records expose DNS
providers. TXT records can reveal cloud service ownership strings and email
authentication details.

A tester starts by resolving the base domain, then expands into related
records and subdomains.

## Subdomain enumeration

A target rarely runs its entire attack surface on one host. Subdomains
distribute services, and some of those services are less maintained than the
main application.

Subdomains are discovered three ways:

```text
brute force: try a wordlist against the domain
passive: query certificate transparency and search engines
cross-reference: use DNS records to find related names
```

Brute force asks DNS for each name in a wordlist. Passive methods read the
logs of the public certificate authority system: every certificate issued for
a domain must be logged, and those logs list the subdomains the certificate
covered.

The valuable find is the subdomain that exists but was never meant to be
public. A test environment, a staging app, or a dashboard named after an
internal project is a common finding in this phase.

## Search engines and dorking

Search engines index more than web pages. Files, configuration snippets, and
error messages become searchable if they are public.

A dork is a carefully constructed search query that targets a specific type of
content. The syntax varies by engine, but the idea is consistent:

```text
site: restrict results to a domain
filetype: restrict results to a file type
intitle: match a phrase in the title
inurl: match a phrase in the URL
```

Examples of the pattern:

```text
site:target.lab filetype:pdf
site:target.lab inurl:admin
site:target.lab intitle:error
```

Public documents can contain names, email addresses, internal hostnames, and
config snippets. Each of those is a thread the tester can pull.

## Certificate transparency

Every public TLS certificate is logged by certificate transparency, and the
logs are public. Asking the logs for a domain returns every certificate that
has ever been issued for it.

The value is the subdomains. A wildcard certificate covers many names, but
specific certificates are issued per host, and the issuance records reveal the
hosts.

A convenient way to query certificate logs without building a client is:

```text
crtsh: query the certificate transparency database by domain
```

The result is a list of hostnames with certificate metadata. This is a
passive, exhaustive subdomain source that brute force can never match.

## Internet-wide scanning services

Services such as Shodan continuously scan the internet and store the banners,
ports, and service metadata they receive. Querying these databases is passive
recon: the information was collected by someone else.

A query can filter by:

```text
hostname
IP address
open port
service banner text
operating system hints
```

The practical use is answering questions such as "which of the target's
public IPs expose an FTP service?" without scanning them directly.

## People and metadata

Files published by the target often contain more than their visible content.

A PDF or office document can embed:

```text
the author's name
the organization's internal name
the software version that created the file
internal server paths
previous editor names
```

This is metadata, and it is routinely left in documents by accident. It is
also routinely the source of usernames that a later credential attack can use.

Social sources add the human dimension: job postings name the technology stack,
and professional profiles name the people responsible for it. A report can
include this context as supporting information, always respecting the scope
and the target's privacy expectations.

## Organizing recon results

Recon produces a large amount of small facts. Unorganized, they are noise.
Organized, they become a map.

The working rule from the previous lesson applies:

```text
save every command
save every output
label every result with its source
```

A simple structure for this course:

```text
recon/
    domains.txt
    hosts.txt
    subdomains.txt
    documents/
    screenshots/
```

As new hosts are discovered, they move forward into enumeration. Recon is not
a separate activity that ends; it feeds every later phase whenever a new name
appears.

## A reconnaissance checklist

Before an engagement leaves the recon phase, the tester should be able to
answer the following with a source behind each answer:

```text
What domains are in scope?
Which of those domains resolve to hosts?
Which subdomains exist beyond the obvious ones?
Which records hint at mail, DNS, and cloud infrastructure?
Which hosts expose web, file, or remote-access services?
Which published documents contain metadata?
Which people, roles, and technology names are visible?
```

A checkmark without a source is not an answer. The checklist is only complete
when every line points to a saved command, output, or screenshot.

Recon stops when the next question cannot be answered passively. At that
point the tester is ready to become active, which the next lessons cover.

## Next step

Now type the code version of this lesson: passive and active DNS recon,
subdomain discovery, and a search pattern that produces an attack surface
list.