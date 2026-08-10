# Data, Cryptography, and Secrets

Cryptography is a set of mechanisms, not a substitute for access control. A secure application must decide what information needs protection, where it exists, who can access it, how long it is retained, and which cryptographic operation is appropriate.

For demonstrations, assume TanStack Start on Vercel with Supabase Auth, PostgreSQL, Storage, Edge Functions, and Realtime.

## Classify data before protecting it

Different data has different security requirements.

Consider:

- public data;
- ordinary application data;
- confidential business data;
- credentials and authentication material;
- highly sensitive personal data;
- cryptographic keys and secrets.

Do not automatically encrypt everything without considering key management, searchability, performance, backups, and operational recovery.

## Hashing is not encryption

Hashing is a one-way transformation intended for verification and integrity-oriented use cases.

Password storage should use a password hashing algorithm designed for the job, such as Argon2id or another modern password hashing scheme supported by the application's authentication system.

Encryption is reversible with a key.

Use encryption when the application must recover the original plaintext.

## Do not encrypt passwords

Passwords should normally be stored as password hashes by the authentication system.

With Supabase Auth, application developers should not create a parallel password table. Let the authentication service manage password authentication.

## Encryption at rest and application-level encryption

Supabase and its underlying infrastructure provide platform-level protections for stored data. That does not mean every application-level confidentiality requirement is automatically satisfied.

Application-level encryption can be appropriate when a specific field requires additional protection from database readers or backups.

The trade-off is that encrypted values are generally harder to query and index.

## Key management matters more than the cipher choice

A strong encryption algorithm is ineffective if its key is exposed.

Keys should:

- never be committed to Git;
- never be embedded in client JavaScript;
- never be logged;
- be stored in server-side environment configuration or a dedicated secret manager;
- be rotated according to operational requirements;
- have restricted access.

A Vercel environment variable containing a secret must only be consumed by server-side code.

A Supabase Edge Function secret must not be returned to the browser.

## Public and secret environment variables

Client-exposed variables are not secrets.

In a Vite/TanStack application, variables intentionally exposed to browser code should be treated as public.

Examples of data that may be public include:

```text
SUPABASE_URL
SUPABASE_PUBLISHABLE_KEY
```

Examples that must remain server-side include:

```text
SUPABASE_SERVICE_ROLE_KEY
ENCRYPTION_KEY
WEBHOOK_SECRET
THIRD_PARTY_API_SECRET
```

The exact variable names are project-specific.

## Service-role credentials bypass normal database protections

Supabase service-role credentials are privileged server credentials.

They must never be shipped to the browser.

A common architecture is:

```text
Browser
  |
  | authenticated request
  v
TanStack Start / Edge Function
  |
  | privileged server operation when justified
  v
Supabase
```

The privileged credential belongs only on the server side.

## Cryptographic randomness

Do not use predictable random values for security tokens.

Use the platform's cryptographically secure random generator.

```typescript
const token = crypto.randomUUID();
```

For secrets requiring raw random bytes:

```typescript
const bytes = crypto.getRandomValues(new Uint8Array(32));
```

Do not use `Math.random()` for password reset tokens, API credentials, session identifiers, or other security-sensitive values.

## Token design

A token should be:

- unpredictable;
- sufficiently long;
- scoped to its purpose;
- short-lived when practical;
- revocable where required;
- stored safely.

If a token grants access to a sensitive operation, possession of the token may effectively represent authorization.

Treat it accordingly.

## Secret rotation

Rotation requires more than changing a string.

A workable design must account for:

1. issuing the new secret;
2. deploying consumers;
3. allowing a controlled transition if necessary;
4. revoking the old secret;
5. verifying that old credentials no longer work.

Design secret-dependent integrations so that rotation is operationally possible.

## Data minimization

Encryption does not eliminate the consequences of unnecessary collection.

If the application does not need a field, do not collect or retain it.

Queries should also select only the fields needed by the operation rather than returning an entire sensitive record to the application.

## Logging sensitive data

Do not log:

- passwords;
- access tokens;
- API keys;
- encryption keys;
- raw authentication headers;
- unnecessary personal identifiers;
- sensitive request bodies.

A debugging statement can become a permanent security incident when logs are copied into monitoring systems, support tools, or backups.

## Cryptographic failure

If decryption fails, do not silently substitute empty or fabricated plaintext.

Treat cryptographic failures as errors that require controlled handling.

Likewise, do not invent fallback encryption algorithms. Use established libraries and documented protocols.

## Practical rules

1. Hash passwords; encrypt recoverable secrets.
2. Generate security tokens with cryptographic randomness.
3. Keep keys server-side.
4. Never expose Supabase service-role credentials.
5. Minimize sensitive data collection.
6. Restrict data returned by queries.
7. Keep secrets out of logs and source control.
8. Plan key and secret rotation before production.
9. Use established cryptographic primitives and libraries.
10. Treat cryptography as one layer of a broader access-control architecture.
