# API Security, Webhooks, and Abuse Resistance - Typing

This file is the implementation companion to the concept lesson. The examples
assume TanStack Start, Supabase, Vercel, and npm or Bun.

## Validate an API request

```typescript
import { z } from "zod";

const createOrderSchema = z.object({
  productId: z.string().uuid(),
  quantity: z.number().int().min(1).max(20),
});

const parsed = createOrderSchema.safeParse(await request.json());

if (!parsed.success) {
  return Response.json({ error: "invalid_request" }, { status: 400 });
}

const input = parsed.data;
```

## Bound pagination

```typescript
const requestedLimit = Number(url.searchParams.get("limit") ?? "20");

if (!Number.isFinite(requestedLimit)) {
  return Response.json({ error: "invalid_limit" }, { status: 400 });
}

const limit = Math.min(Math.max(Math.trunc(requestedLimit), 1), 100);
```

## Allowlist sorting

```typescript
const sortableFields = {
  createdAt: "created_at",
  name: "name",
} as const;

const requestedSort = url.searchParams.get("sort") ?? "createdAt";

if (!(requestedSort in sortableFields)) {
  return Response.json({ error: "invalid_sort" }, { status: 400 });
}

const column =
  sortableFields[requestedSort as keyof typeof sortableFields];
```

Do not concatenate an arbitrary request parameter into SQL.

## Authenticate with Supabase

```typescript
const {
  data: { user },
} = await supabase.auth.getUser();

if (!user) {
  return Response.json({ error: "unauthorized" }, { status: 401 });
}
```

Authentication must be followed by authorization for the requested resource.

## Enforce ownership in the database

Prefer a query whose authorization boundary is part of the data access operation.

```typescript
const { data, error } = await supabase
  .from("orders")
  .select("*")
  .eq("id", orderId)
  .eq("user_id", user.id)
  .maybeSingle();
```

For multi-tenant applications, Supabase Row Level Security should provide a
database-level enforcement layer in addition to application checks.

## Signed webhook verification

A provider-specific implementation should verify the raw body.

```typescript
const rawBody = await request.text();
const signature = request.headers.get("x-webhook-signature");

if (!signature) {
  return new Response("Missing signature", { status: 401 });
}

// Replace this function with the provider's documented signing algorithm.
const expectedSignature = await computeWebhookSignature(
  rawBody,
  process.env.WEBHOOK_SECRET!,
);

if (!timingSafeEqual(signature, expectedSignature)) {
  return new Response("Invalid signature", { status: 401 });
}
```

Do not invent a signing algorithm for a real provider. Use its documented
scheme exactly.

## Idempotent webhook processing

```sql
create table webhook_events (
  event_id text primary key,
  received_at timestamptz not null default now(),
  processed_at timestamptz
);
```

The primary key prevents the same event identifier from being inserted twice.

Application logic can then distinguish a new event from a duplicate delivery.

## Timeout an outbound request

```typescript
const controller = new AbortController();
const timeout = setTimeout(() => controller.abort(), 10_000);

try {
  const response = await fetch(targetUrl, {
    method: "POST",
    body: JSON.stringify(payload),
    headers: {
      "content-type": "application/json",
    },
    signal: controller.signal,
  });

  if (!response.ok) {
    throw new Error(`Upstream returned ${response.status}`);
  }
} finally {
  clearTimeout(timeout);
}
```

## Test the security invariants

```typescript
test("rejects an excessive page size", async () => {
  const response = await request("/api/orders?limit=1000000");

  expect(response.status).toBe(400);
});
```

The exact testing framework may vary. The security property should not.

## Useful commands

```bash
npm install zod
npm run test
```

or:

```bash
bun add zod
bun test
```

For a production application, add API abuse tests to the normal test suite and
run them in CI before deployment.
