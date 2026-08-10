# API Security, Webhooks, and Abuse Resistance

An API is an exposed security boundary. A browser interface is not a security boundary: an attacker can construct requests directly, omit client-side validation, replay requests, alter parameters, and send requests at a volume the normal UI never produces.

This lesson focuses on the application-layer controls that make APIs resistant to malformed input, unauthorized access, replay, resource exhaustion, webhook forgery, and operational failure.

For demonstrations, assume a TanStack Start application deployed on Vercel using Supabase Auth, PostgreSQL, Storage, Edge Functions, and Realtime.

## Treat every API request as hostile input

The server must independently establish:

1. who is making the request;
2. whether that identity may perform the operation;
3. whether the input satisfies the API contract;
4. whether the requested resource belongs to the caller's permitted scope;
5. whether the operation is within acceptable resource and rate limits.

Client-side validation improves usability. It does not establish security.

A secure endpoint therefore validates on the server even when the same form is already validated in React.

## Authentication is only one API control

A valid session answers an identity question. It does not answer authorization.

An endpoint such as:

```text
GET /api/orders/123
```

must not assume that possession of a valid session permits access to order `123`.

The application must establish that the authenticated principal has access to that particular resource.

This is the same object-level authorization problem covered earlier in the course, applied at the API boundary.

## Validate request schemas

Use explicit schemas rather than accepting arbitrary objects.

```typescript
import { z } from "zod";

const createOrderSchema = z.object({
  productId: z.string().uuid(),
  quantity: z.number().int().min(1).max(20),
});
```

The schema establishes an expected shape and useful limits. It should be applied before business logic executes.

Do not treat a TypeScript type as runtime validation. TypeScript disappears at runtime; attackers send runtime values.

## Bound resource consumption

Security includes availability.

Examples of useful limits include:

- maximum request body size;
- maximum pagination size;
- maximum uploaded file size;
- maximum search length;
- maximum batch size;
- maximum execution time;
- maximum retry count.

A pagination API should not allow an arbitrary client-controlled limit such as:

```text
?limit=100000000
```

Prefer a bounded value:

```typescript
const limit = Math.min(Math.max(requestedLimit, 1), 100);
```

The exact limit should be chosen according to the application's workload rather than copied blindly.

## Allowlist dynamic behavior

Dynamic sorting, filtering, and field selection can become injection or authorization problems when arbitrary database fields are accepted.

Prefer:

```typescript
const sortableFields = {
  createdAt: "created_at",
  name: "name",
} as const;
```

The client supplies a logical identifier. The server maps that identifier to an approved operation.

Do not concatenate arbitrary client strings into SQL, shell commands, URLs, or other interpreters.

## Rate limiting and abuse controls

Rate limiting protects endpoints from excessive use.

Useful dimensions can include:

- authenticated user;
- organization or tenant;
- IP address;
- API key;
- endpoint;
- expensive operation.

A single global limit is often insufficient. An authenticated tenant may legitimately perform more operations than an unauthenticated caller, while an expensive export endpoint may require a much lower limit than a simple read.

On Vercel and Supabase, rate limiting can be implemented around the application's server and Edge boundaries using an appropriate external or persistent counter mechanism. Do not assume that an in-memory counter is a reliable distributed rate limiter.

## Webhook verification

A webhook endpoint receives requests from another system. The endpoint must not trust a request merely because it uses a special URL.

A typical signed webhook flow is:

1. receive the raw request body;
2. obtain the signature header;
3. compute the expected signature using a shared secret;
4. compare signatures using a timing-safe comparison;
5. reject invalid signatures;
6. only then parse and process the event.

The exact signing scheme depends on the provider.

Do not reconstruct the body into JSON and then verify a signature over a differently serialized representation. Signature verification normally depends on the exact bytes that were signed.

## Webhook idempotency

Delivery systems commonly retry events.

Therefore:

```text
event A
event A
event A
```

must not necessarily produce three business operations.

Persist an event identifier or another provider-defined idempotency key and make processing idempotent.

The database should enforce uniqueness where appropriate rather than relying solely on application code.

## Replay resistance

A valid signed message can still be replayed if the protocol permits it.

Where the provider supplies timestamps, nonces, or unique event identifiers, use them according to the provider's protocol.

A typical policy is:

```text
valid signature
+ acceptable timestamp
+ previously unseen event ID
= process
```

The exact policy must match the external provider.

## Timeouts and failure boundaries

External requests should have explicit timeouts.

A request that waits indefinitely can consume server resources and create cascading failures.

Use bounded retries with backoff where retries are safe.

Do not automatically retry non-idempotent operations merely because a network request failed. A timeout does not prove that the remote system did not receive the request.

## Error handling

External clients should receive useful status categories without receiving internal implementation details.

Avoid returning:

```text
Postgres error: relation "private_customer_ssns" does not exist
```

Prefer a stable application error such as:

```json
{
  "error": "request_failed",
  "message": "The request could not be completed."
}
```

Detailed diagnostic information belongs in protected server logs.

## API security checklist

Before considering an endpoint complete, verify:

- authentication is required where appropriate;
- authorization is checked for the requested resource;
- request schemas are validated server-side;
- dynamic behavior is allowlisted;
- pagination and resource usage are bounded;
- sensitive operations have appropriate rate limits;
- webhook signatures are verified;
- webhook processing is idempotent;
- replay behavior is considered;
- external calls have timeouts;
- retries are bounded and safe;
- errors do not disclose internal details;
- security-relevant events are logged without leaking secrets or sensitive data.

API security is therefore not a single middleware. It is the collection of controls surrounding an externally reachable operation.
