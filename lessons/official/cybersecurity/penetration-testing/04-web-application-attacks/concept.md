# Web Application Attacks

Web applications are the largest attack surface on most networks. Every
organization runs a web server, and every web server runs code that accepts
input from strangers. The previous lesson mapped the services; this lesson
attacks the web application itself, from the perspective of a tester who has
been handed nothing but a URL.

The browser is never part of the security boundary. The tester's requests are
crafted directly with curl and scripted tools, exactly as an attacker would
send them.

## Think like the application

A web application is a collection of endpoints that accept input and return
output. The tester's job is to find input that the application handles
incorrectly.

Before any exploit, map the application:

```text
what endpoints exist?
what parameters does each endpoint accept?
what methods does each endpoint allow?
what data flows into the endpoint?
what does the endpoint do with that data?
```

The OWASP Top 10 names the most common failure classes, but the tester thinks
in terms of the data flow, not the list. An endpoint that takes an identifier
and returns a record is interesting. An endpoint that takes a filename and
fetches it is interesting. An endpoint that takes text and echoes it back is
interesting.

## Mapping the application

The first active step is content discovery: find every path the application
serves, including paths the UI never links to.

Directories, backup files, configuration files, and admin panels appear when a
wordlist is pushed at the server. Each response is a clue:

```text
200: the path exists
301/302: the path redirects, often to a login
401/403: the path exists but is protected
404: the path does not exist
```

A 403 is as interesting as a 200. It means something is there, and the tester
now asks why it is protected and how to get past the protection.

## Finding parameters

Endpoints are only as interesting as their parameters. Parameters arrive in:

```text
the URL query string
the request body
headers and cookies
hidden form fields
file uploads
```

Every parameter is candidate input. A tester collects them from page source,
network traffic, JavaScript, and application behavior, then tests each one for
misbehavior.

## SQL injection

SQL injection happens when user input becomes part of a SQL query without
being separated from it.

The classic case:

```text
query built by concatenation:
    SELECT * FROM users WHERE id = '<input>'

attacker input:
    ' OR 1=1 --

result:
    SELECT * FROM users WHERE id = '' OR 1=1 --'
```

The injected fragment changes the meaning of the query. The tester proves the
weakness by making the query behave observably differently:

```text
an input that causes an error
an input that changes the number of results
an input that changes timing
```

Blind injection produces no visible data but still changes behavior, so timing
and boolean responses become the evidence. The goal in an engagement is proof,
not full database dump: demonstrate that the boundary is broken and document
how.

## Cross-site scripting

Cross-site scripting, or XSS, is the injection of script into a page that
other users load. The victim is not the server; it is the browser of whoever
views the page.

The three classes:

```text
reflected: the input appears in the response immediately
stored: the input is saved and shown to later visitors
DOM-based: the script runs in the browser without touching the server
```

The tester looks for input that the page echoes back, then checks whether the
echo is encoded or raw. A harmless proof, such as a marker string, establishes
the injection point before any script payload is considered. XSS matters
because it is the delivery mechanism for session theft and authenticated
actions performed as the victim.

## Server-side request forgery

Server-side request forgery, or SSRF, happens when the server fetches a URL
that the attacker supplies.

A feature that accepts a URL, an image, a webhook, or a file reference is a
candidate. The server performs the request with its own privileges and its own
network position:

```text
server fetches: http://attacker-supplied/path
attacker asks the server to fetch internal addresses
```

The result is that a request that should never leave the application server
can reach internal services, cloud metadata, or the server itself. The tester
proves SSRF by pointing the feature at a target the tester controls and
observing the server's request.

## Command injection

Command injection is the execution of operating-system commands through
application input.

It appears when the application builds a shell command from user input:

```text
application runs: ping -c 1 <input>

attacker input:   10.10.20.5; whoami

command executed: ping -c 1 10.10.20.5; whoami
```

The separator characters of the shell (semicolon, ampersand, pipe, command
substitution) become the injection surface. Command injection is usually the
fastest path to a full shell, which is why it is tested early on any endpoint
that touches files, processes, or system utilities.

## Broken access control

Broken access control is the failure to enforce who may do what.

The attacker-side view:

```text
request a resource by its identifier
change the identifier to another object
check whether access follows
```

Changing an order id, a user id, or a document id and receiving someone else's
data is the same class of flaw the web-security course calls IDOR. From the
tester's side, it is often the first finding of an engagement because it needs
nothing but a logged-in account and a modified request.

## Path traversal and file disclosure

When an application serves files by name, the name may include path segments
the developer did not intend.

```text
requested:  /download?file=report.pdf
injected:   /download?file=../../../../etc/passwd
```

The dots climb out of the intended directory. If the application concatenates
the input into a filesystem path and reads it, the tester reads files the
application should never expose. The evidence is a file whose presence proves
the read: on Linux, /etc/passwd is the classic target because its format is
unmistakable.

## Authentication and session attacks

Login systems are tested as application logic, not as a black box.

The tester asks:

```text
Can credentials be enumerated by the error messages?
Can the session cookie be predicted or fixed?
Can another user's session be replayed?
Can a reset flow hand over another account?
```

Error messages that distinguish "user not found" from "wrong password" are
themselves a finding because they let an attacker enumerate accounts. A
session cookie without randomness or without expiration becomes a theft target.
Each behavior is tested with the same input-and-observe pattern as every other
endpoint.

## Testing without breaking

A web application test is active and can damage the target. The rules of
engagement govern how far to go:

```text
use proof payloads before destructive ones
prefer read-only evidence
stop at the agreed depth
restore the lab target after the test
```

In the lab, every target is disposable, so payloads can be exercised fully.
On a real engagement, the tester proves the weakness with the least invasive
action that demonstrates it, then records the finding.

## Next step

Now type the code version of this lesson: content discovery, parameter
manipulation, and proof-of-concept requests for the common web flaws against
the lab application.