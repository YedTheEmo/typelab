# Security Testing and Operations - Typing

The examples assume TanStack Start, Vercel, Supabase, npm or Bun.

## Test unauthenticated access

```typescript
test("protected endpoint rejects anonymous requests", async () => {
  const response = await request("/api/orders");

  expect(response.status).toBe(401);
});
```

## Test object-level authorization

```typescript
test("user cannot read another user's order", async () => {
  const response = await requestAs(userA, `/api/orders/${userBOrderId}`);

  expect(response.status).toBe(404);
});
```

Returning `404` instead of `403` can sometimes avoid revealing whether a
resource exists. The correct status should follow the application's threat
model and API contract.

## Test invalid input

```typescript
test("rejects an excessive quantity", async () => {
  const response = await createOrder({
    productId: validProductId,
    quantity: 999999,
  });

  expect(response.status).toBe(400);
});
```

## Test webhook authenticity

```typescript
test("rejects a webhook with an invalid signature", async () => {
  const response = await sendWebhook({
    body: validPayload,
    signature: "invalid",
  });

  expect(response.status).toBe(401);
});
```

## Test webhook idempotency

```typescript
test("does not process the same event twice", async () => {
  await sendWebhook(validEvent);
  await sendWebhook(validEvent);

  expect(await countBusinessEffects(validEvent.id)).toBe(1);
});
```

The database should enforce event uniqueness where appropriate.

## Test private Storage access

```typescript
test("anonymous users cannot access private files", async () => {
  const response = await downloadPrivateFileWithoutSession(filePath);

  expect(response.status).toBe(401);
});
```

The exact implementation depends on the Supabase Storage policy.

## Add security headers to integration tests

```typescript
test("production response includes HSTS", async () => {
  const response = await request("/");

  expect(response.headers.get("strict-transport-security")).toBeTruthy();
});
```

Only assert headers that are actually required by the deployment architecture.

## Avoid sensitive logging

```typescript
function logSecurityEvent(event: {
  requestId: string;
  action: string;
  userId?: string;
}) {
  console.info({
    type: "security_event",
    ...event,
  });
}
```

Do not pass passwords, access tokens, service keys, or raw request bodies to
the logger.

## Correlation IDs

```typescript
const requestId =
  request.headers.get("x-request-id") ?? crypto.randomUUID();

console.info({
  requestId,
  event: "order_request",
});
```

A production system should define how trusted upstream request IDs are
handled rather than blindly accepting arbitrary identifiers.

## Useful security commands

```bash
npm audit
npm test
npm run build
```

or:

```bash
bun audit
bun test
bun run build
```

Additional tooling such as secret scanning, static analysis, and dependency
scanning can be added to CI.

## CI security gate

A conceptual sequence is:

```text
install
→ audit
→ test
→ build
→ deploy preview
→ verify
→ production
```

The security suite should include negative authorization tests rather than only
happy-path functional tests.
