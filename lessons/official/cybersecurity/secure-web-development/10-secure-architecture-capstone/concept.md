# Secure Architecture, Testing, and Operations

A secure web application is not created by adding a few security libraries. Security emerges from the architecture, implementation, deployment, monitoring, testing, and response process.

This final lesson combines the earlier controls into an operational security model for a TanStack Start application deployed on Vercel with Supabase.

## Security is a system property

The major boundaries in the demonstration architecture are:

```text
Browser
  |
  v
TanStack Start / Vercel
  |
  +--> Supabase Auth
  |
  +--> Supabase PostgreSQL + RLS
  |
  +--> Supabase Storage + policies
  |
  +--> Supabase Edge Functions
  |
  +--> Supabase Realtime
  |
  +--> external services
```

Each boundary has a different security responsibility.

The browser is an untrusted environment.

The application server establishes application behavior.

Supabase Auth establishes identity.

PostgreSQL RLS provides a database-level authorization boundary.

Storage policies protect objects.

Edge Functions provide server-side execution for operations that should not occur in the browser.

Vercel provides the application deployment environment.

## Define security invariants

A security invariant is a property that must remain true.

Examples:

```text
Anonymous users cannot read private orders.

Users cannot read another tenant's records.

Privileged service-role credentials never reach browser code.

Private Storage objects cannot be downloaded without authorization.

Webhook events require valid signatures.

The same webhook event cannot produce duplicate business effects.

Sensitive values never appear in application logs.

Production deployments cannot start without required secrets.
```

These statements can become tests, database policies, code assertions, deployment checks, and review criteria.

## Defense in depth

Do not depend on one layer.

For example, tenant isolation can involve:

```text
application authorization
+
Supabase RLS
+
careful query construction
+
negative authorization tests
```

If one layer contains a defect, another layer may prevent the defect from becoming a cross-tenant data exposure.

Defense in depth does not mean duplicating every control everywhere. It means placing independent controls at meaningful boundaries.

## Fail closed

When a security decision cannot be established, the default should normally deny the sensitive operation.

Examples:

```text
missing identity → deny
missing authorization context → deny
invalid signature → deny
unknown resource scope → deny
missing required secret → deployment failure
```

Avoid fallback behavior that silently converts a security failure into broader access.

## Design the trust boundaries

For every operation, identify:

1. where untrusted data enters;
2. where identity is established;
3. where authorization is decided;
4. where data is persisted;
5. where external systems are contacted;
6. where privileged credentials are used;
7. where security events are recorded.

This makes security review concrete.

## Secure request lifecycle

A protected operation should conceptually follow:

```text
request
→ establish identity
→ validate input
→ authorize resource
→ apply business rules
→ perform bounded operation
→ persist result
→ emit safe audit information
→ return minimal response
```

Do not perform sensitive side effects before authorization has succeeded.

## Secure deployment lifecycle

A secure delivery process can be represented as:

```text
development
→ code review
→ dependency review
→ security tests
→ build
→ preview
→ verification
→ production
→ monitoring
→ incident response
```

Security therefore continues after deployment.

## What the developer must know

A web developer should be able to reason about:

- HTTP and browser security;
- authentication and sessions;
- authorization and object ownership;
- input validation;
- injection boundaries;
- XSS and CSRF;
- CORS and security headers;
- cryptography and secret management;
- API and webhook security;
- dependency and supply-chain risk;
- deployment configuration;
- database and Storage authorization;
- logging and monitoring;
- security testing;
- incident response.

The objective is not memorization of every vulnerability name.

The objective is recognizing trust boundaries and asking:

```text
What is untrusted here?
Who is allowed to do this?
What prevents unauthorized access?
What happens if this input is malicious?
What happens if this dependency fails?
What happens if this request is replayed?
What happens if this secret leaks?
What happens if this component is unavailable?
```

## Final architecture review

Before calling an application secure enough for deployment, review the entire path:

```text
Browser
  ↓
HTTP security
  ↓
Authentication
  ↓
Authorization
  ↓
Input validation
  ↓
Business logic
  ↓
Database / Storage
  ↓
External integrations
  ↓
Logging / monitoring
  ↓
Deployment / recovery
```

A control missing at any important boundary becomes a candidate failure mode.

## Capstone principle

There is no single secure-web-development switch.

Security is a collection of explicit constraints maintained throughout the application's lifecycle.

A developer who can identify trust boundaries, enforce authorization, constrain untrusted input, protect secrets, isolate data, test negative cases, and operate the system securely has the foundation required to build secure web applications.
