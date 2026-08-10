# Data, Cryptography, and Secrets - Typing

The examples assume TanStack Start, Vercel, Supabase Auth, PostgreSQL,
Storage, Edge Functions, and npm or Bun.

## Generate a cryptographically secure token

```typescript
const token = crypto.randomUUID();
```

For a random byte sequence:

```typescript
const bytes = crypto.getRandomValues(new Uint8Array(32));

const token = Buffer.from(bytes).toString("base64url");
```

Do not replace these APIs with `Math.random()`.

## Keep secrets server-side

```typescript
// Server-side code only
const serviceRoleKey = process.env.SUPABASE_SERVICE_ROLE_KEY;

if (!serviceRoleKey) {
  throw new Error("Missing server secret");
}
```

Do not import this value into a browser component.

## Use Supabase Auth for passwords

The application should use Supabase Auth rather than creating its own
password storage system.

```typescript
const { data, error } = await supabase.auth.signUp({
  email,
  password,
});
```

The application does not need a custom `password_hash` column in its own
profile table.

## Select only required data

```typescript
const { data, error } = await supabase
  .from("customers")
  .select("id, display_name, status")
  .eq("id", customerId)
  .single();
```

Avoid:

```typescript
.select("*")
```

when the operation does not require every field.

## Store secrets in environment configuration

A local `.env` file may contain:

```text
SUPABASE_URL=...
SUPABASE_PUBLISHABLE_KEY=...
SUPABASE_SERVICE_ROLE_KEY=...
WEBHOOK_SECRET=...
ENCRYPTION_KEY=...
```

Never commit the actual values.

```gitignore
.env
.env.local
.env.*.local
```

Configure production secrets through Vercel and Supabase's appropriate secret
configuration mechanisms rather than committing them to the repository.

## Simple application-level encryption boundary

Do not implement production cryptography from scratch. Use a vetted library
and a documented key-management strategy.

The application boundary should conceptually look like:

```typescript
const ciphertext = await encryptSensitiveValue(
  plaintext,
  encryptionKey,
);

const plaintext = await decryptSensitiveValue(
  ciphertext,
  encryptionKey,
);
```

The encryption key must come from protected server-side configuration.

## Avoid logging secrets

Bad:

```typescript
console.log({
  authorization: request.headers.get("authorization"),
  webhookSecret,
});
```

Better:

```typescript
console.log({
  event: "webhook_received",
  requestId,
});
```

Logs should describe security-relevant events without reproducing credentials.

## Test secret exposure

```typescript
test("browser configuration contains no server secret", () => {
  expect(import.meta.env.VITE_SUPABASE_SERVICE_ROLE_KEY).toBeUndefined();
});
```

The exact build environment may differ, but the invariant is important:
privileged credentials must never become browser configuration.

## Useful commands

```bash
npm audit
npm test
```

or:

```bash
bun audit
bun test
```

Use the package manager and dependency tooling supported by the project.
