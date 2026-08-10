# Security Testing and Operations

Security controls that are never tested eventually become assumptions. A secure web application needs tests that verify security invariants, logs that support investigation, monitoring that exposes abnormal behavior, and operational procedures for responding to failures.

For demonstrations, assume TanStack Start on Vercel with Supabase Auth, PostgreSQL, Storage, Edge Functions, and Realtime.

## Security testing is about properties

A security test should answer a question such as:

```text
Can an unauthenticated user access this endpoint?
Can user A read user B's record?
Can a webhook be accepted without a valid signature?
Can an attacker submit an oversized request?
Can a private file be downloaded without authorization?
Can the same event be processed twice?
```

These are more useful than simply asking whether a function returns the expected output.

## Authentication tests

Verify that protected resources reject unauthenticated requests.

```text
no session → 401
valid session → continue
```

Do not only test the happy path.

## Authorization tests

Authorization tests should explicitly attempt cross-user and cross-tenant access.

For example:

```text
User A requests User B's order
→ denied
```

This is one of the most important classes of tests for applications with user-owned or tenant-owned data.

## Negative testing

Security testing deliberately supplies values that normal UI flows should not produce.

Examples:

- missing fields;
- wrong types;
- excessive lengths;
- invalid identifiers;
- unexpected enum values;
- oversized pagination;
- malformed JSON;
- duplicate events;
- expired credentials.

## Injection testing

Test inputs containing syntax that could be interpreted by downstream systems.

The goal is not merely to search for famous payload strings. Test whether untrusted data remains data when passed through:

- SQL;
- HTML;
- URLs;
- shell commands;
- templates;
- JSON;
- regular expressions.

## Security headers and browser controls

Tests can verify that production responses include expected headers such as:

- Content-Security-Policy where appropriate;
- Strict-Transport-Security;
- X-Content-Type-Options;
- Referrer-Policy;
- appropriate framing restrictions.

Do not blindly copy a header configuration. Headers should match the application's actual browser behavior.

## Dependency and build testing

Automate:

```text
dependency audit
→ unit tests
→ integration tests
→ build
→ deployment verification
```

For high-risk systems, add static analysis, secret scanning, and dynamic testing as appropriate.

## Logging

Logs should support questions such as:

- what happened?
- when did it happen?
- which request or event was involved?
- which account or tenant was affected?
- did the operation succeed?
- what error occurred?

Use correlation or request IDs.

Avoid logging secrets and unnecessary sensitive data.

## Monitoring

Monitoring turns logs into operational signals.

Useful signals include:

- authentication failures;
- authorization failures;
- unusual request rates;
- repeated webhook failures;
- database errors;
- dependency or deployment failures;
- abnormal error rates;
- storage access anomalies.

The objective is not to record everything. It is to make meaningful security events observable.

## Incident response

A response plan should define:

1. detection;
2. containment;
3. investigation;
4. remediation;
5. recovery;
6. post-incident review.

For a leaked API credential, containment may mean immediate revocation and replacement rather than merely deleting the value from source code.

## Secret compromise

Assume that a secret appearing in:

- Git history;
- logs;
- browser bundles;
- screenshots;
- tickets;
- chat;
- public build artifacts

may be compromised.

The response is rotation and revocation, not merely deletion.

## Backup and recovery

Security operations include recovery.

Know:

- what data is backed up;
- how long backups are retained;
- who can restore them;
- whether restoration has been tested;
- how access controls behave after restoration.

An untested backup is an assumption.

## Security review cadence

Security review should occur:

- before production;
- when authentication changes;
- when authorization changes;
- when handling of sensitive data changes;
- when introducing major dependencies;
- when adding integrations;
- when changing deployment infrastructure.

Security is continuous because the application's attack surface changes continuously.

## Operational security checklist

Before release, verify:

- authentication tests pass;
- authorization tests include negative cases;
- input validation is tested;
- injection boundaries are tested;
- security headers are verified;
- dependencies are reviewed;
- secrets are scanned;
- logs avoid sensitive data;
- important security events are observable;
- backups and recovery are understood;
- incident response actions are known;
- credential rotation is possible.

The purpose of security testing is not to prove that an application can never be compromised. It is to systematically reduce unverified assumptions and detect failures before attackers do.
