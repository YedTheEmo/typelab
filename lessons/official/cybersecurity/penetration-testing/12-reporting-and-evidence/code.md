# Reporting and Evidence - Typing

This lesson types the reporting workflow: gather the engagement's evidence,
build the findings table, assemble the report document, and package the
deliverable.

## Gather the evidence

The report is assembled from the engagement folders.

```bash
# create the report workspace
mkdir -p report/evidence

# copy every evidence file into the report workspace
find exploit enum recon -type f \
    -exec cp --parents {} report/evidence/ \; 2>/dev/null

# show the collected evidence tree
find report/evidence -type f | sort
```

Every file the report cites is present in the deliverable.

## Build the findings table

The findings table is the spine of the report.

```bash
# create the findings table
cat > report/findings.tsv <<'EOF'
id	severity	asset	title	status
F-01	high	10.10.20.5	command injection in /ping	confirmed
F-02	medium	10.10.20.5	recovered weak SSH credentials	confirmed
F-03	medium	10.10.20.6	anonymous SMB share access	confirmed
F-04	high	172.16.30.10	SQL injection in item endpoint	confirmed
EOF

# display the table
column -t -s $'\t' report/findings.tsv
```

Each row maps to a full finding section.

## Create the report document

Assemble the report structure in markdown.

```bash
# create the report file
cat > report/report.md <<'EOF'
# Penetration Test Report - lab-01

## Executive summary

The assessment found four confirmed findings across the lab environment.
The most serious allow an unauthenticated attacker to execute commands
and retrieve database contents. Credentials recovered during the test
granted access to internal services.

## Scope and methodology

Targets: 10.10.20.5, 10.10.20.6, and the internal 172.16.30.0/24 network.
Methodology: reconnaissance, enumeration, exploitation, post-exploitation,
and reporting.

## Findings

| id | severity | asset | title |
|----|----------|-------|-------|
| F-01 | high | 10.10.20.5 | command injection in /ping |
| F-02 | medium | 10.10.20.5 | recovered weak SSH credentials |
| F-03 | medium | 10.10.20.6 | anonymous SMB share access |
| F-04 | high | 172.16.30.10 | SQL injection in item endpoint |
EOF

# show the report so far
cat report/report.md
```

The structure carries the executive view and the table of findings.

## Write one finding detail

A confirmed finding gets its full detail section.

```bash
# append the first finding's detail
cat >> report/report.md <<'EOF'

## F-01 - Command injection in /ping

### Severity: high

The endpoint is reachable over the network without authentication.

### Attack scenario

An attacker sends the command separator followed by a proof command.
The application executes the input as a shell command and returns
the output in the response.

### Evidence

The saved request and response reproduce the finding.

### Remediation

Validate the host parameter against an allowlist and do not
construct shell commands from user input.
EOF

# show the completed finding section
tail -n 20 report/report.md
```

The detail tells the engineer exactly what to fix.

## Add a finding without a fix list

A non-finding is recorded for transparency.

```bash
# append a non-finding to the report
cat >> report/report.md <<'EOF'

## Non-findings

The following scanner results were checked and not confirmed:

- nginx version mismatch: banner shows a patched build.
- default path on 10.10.20.6 returned 404 on direct request.
EOF

# show the appended section
tail -n 10 report/report.md
```

Discarded candidates are documented so the review is auditable.

## Verify every evidence reference

The report cites files that must exist.

```bash
# check that the cited evidence files exist
for f in \
    "report/evidence/exploit/web/cmd-inject.txt" \
    "report/evidence/exploit/creds.txt" \
    "report/evidence/exploit/web/sqli-error-full.txt"; do
    if [ -f "$f" ]; then
        echo "ok: $f"
    else
        echo "missing: $f"
    fi
done
```

A missing reference fails the final review.

## Package the deliverable

The report ships as a protected archive.

```bash
# create the deliverable archive
tar -czf report/lab-01-report.tar.gz \
    report/report.md report/findings.tsv report/evidence

# generate a checksum for delivery verification
sha256sum report/lab-01-report.tar.gz > report/lab-01-report.sha256

# restrict the deliverable's permissions
chmod 600 report/lab-01-report.tar.gz report/lab-01-report.sha256

# show the final deliverable
ls -la report/lab-01-report*
```

The archive and its checksum are the handoff to the client.

## Wrap up

Reporting sequence: gather evidence -> findings table -> report document ->
finding details -> verify references -> package deliverable.
