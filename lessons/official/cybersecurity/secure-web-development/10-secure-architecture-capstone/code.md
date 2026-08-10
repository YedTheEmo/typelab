# Secure Architecture, Testing, and Operations - Typing

This capstone combines the course controls using TanStack Start, Vercel,
Supabase Auth, PostgreSQL, Storage, Edge Functions, Realtime, npm, or Bun.

## Security invariant: authenticated access

```typescript
const {
  data: { user },
} = await supabase.auth.getUser();

if (!user) {
  return Response.json({ error: "unauthorized" }, { status: 401 });
}
```

## Security invariant: resource ownership

```typescript
const { data: order } = await supabase
  .from("orders")
  .select("id, status")
  .eq("id", orderId)
  .eq("user_id", user.id)
  .maybeSingle();

if (!order) {
  return Response.json({ error: "not_found" }, { status: 404 });
}
```

The database should also enforce the appropriate ownership or tenant policy
through Supabase RLS.

## Security invariant: bounded input

```typescript
const schema = z.object({
  name: z.string().trim().min(1).max(100),
});

const result = schema.safeParse(await request.json());

if (!result.success) {
  return Response.json({ error: "invalid_request" }, { status: 400 });
}
```

## Security invariant: no privileged browser secret

```typescript
// Server-only:
const serviceRoleKey = process.env.SUPABASE_SERVICE_ROLE_KEY;
```

Never expose the value through client-side environment configuration.

## Security invariant: private file access

```typescript
const { data: object } = await supabase
  .from("documents")
  .select("storage_path")
  .eq("id", documentId)
  .eq("owner_id", user.id)
  .maybeSingle();

if (!object) {
  return Response.json({ error: "not_found" }, { status: 404 });
}
```

The Storage policy must independently enforce the application's object-access
boundary.

## Security invariant: webhook authenticity

```typescript
const rawBody = await request.text();
const signature = request.headers.get("x-webhook-signature");

if (!signature) {
  return new Response("Unauthorized", { status: 401 });
}

const valid = await verifyProviderSignature(
  rawBody,
  signature,
  process.env.WEBHOOK_SECRET!,
);

if (!valid) {
  return new Response("Unauthorized", { status: 401 });
}
```

Use the external provider's documented verification algorithm.

## Security invariant: webhook idempotency

```sql
create table webhook_events (
  event_id text primary key,
  received_at timestamptz not null default now()
);
```

A unique event identifier provides a database-enforced deduplication boundary.

## Security invariant: required configuration

```typescript
function requireServerEnv(name: string): string {
  const value = process.env[name];

  if (!value) {
    throw new Error(`Missing required environment variable: ${name}`);
  }

  return value;
}

const supabaseUrl = requireServerEnv("SUPABASE_URL");
const serviceRoleKey = requireServerEnv("SUPABASE_SERVICE_ROLE_KEY");
```

Missing critical configuration should fail explicitly rather than silently
falling back to insecure behavior.

## Security tests

```typescript
test("anonymous users cannot access private data", async () => {
  const response = await request("/api/private-data");

  expect(response.status).toBe(401);
});

test("users cannot access another user's resource", async () => {
  const response = await requestAs(userA, `/api/orders/${userBOrderId}`);

  expect(response.status).toBe(404);
});

test("duplicate webhook events have one effect", async () => {
  await deliverWebhook(event);
  await deliverWebhook(event);

  expect(await countEffects(event.id)).toBe(1);
});
```

## Final CI sequence

npm:

```bash
npm ci
npm audit --audit-level=high
npm test
npm run build
```

Bun:

```bash
bun install --frozen-lockfile
bun audit
bun test
bun run build
```

The exact CI commands should follow the project's package manager and scripts.

## Final review checklist

```text
[ ] Browser is treated as untrusted.
[ ] Authentication is server-enforced.
[ ] Authorization is resource/tenant scoped.
[ ] Supabase RLS protects application data.
[ ] Storage policies protect private objects.
[ ] Inputs are validated at trust boundaries.
[ ] Output is safely encoded.
[ ] Secrets remain server-side.
[ ] API requests are bounded.
[ ] Webhooks are authenticated and idempotent.
[ ] External calls have timeouts.
[ ] Dependencies are reviewed.
[ ] Security tests include negative cases.
[ ] Logs avoid credentials and unnecessary sensitive data.
[ ] Production configuration fails safely.
[ ] Monitoring and incident response exist.
```

The capstone is complete when these are treated as architecture invariants
rather than optional implementation details.
