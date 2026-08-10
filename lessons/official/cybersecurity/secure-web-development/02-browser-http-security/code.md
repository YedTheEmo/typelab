# Browser and HTTP security - typing

This lesson types browser security controls: HTTPS-aware cookies, origin
validation, CSRF protection, secure headers, and a restrictive CSP.

## Install the server package

The demonstration uses the Node-compatible standard library available to the
TanStack Start server runtime.

```bash
# install the Supabase client used by the server boundary
bun add @supabase/supabase-js
````

## Define trusted origins

The server should know which browser origins are allowed to perform
cross-origin operations.

```typescript
// define the application's trusted browser origin
const trustedOrigin = "https://app.example.com";

// check whether a request came from the trusted origin
function isTrustedOrigin(origin: string | null) {
    // reject requests without an origin
    if (!origin) {
        return false;
    }

    // accept only the configured application origin
    return origin === trustedOrigin;
}
```

## Validate a state-changing request

Origin validation can provide an additional CSRF defense for requests that
should only originate from the application's browser context.

```typescript
// validate the browser origin before a state-changing operation
function requireTrustedOrigin(origin: string | null) {
    // reject an unexpected browser origin
    if (!isTrustedOrigin(origin)) {
        throw new Error("untrusted origin");
    }
}
```

## Generate a CSRF token

A CSRF token must be unpredictable and difficult for another site to
manufacture.

```typescript
// import the cryptographic random generator
import { randomBytes } from "node:crypto";

// create an unpredictable CSRF token
function createCsrfToken() {
    // generate thirty-two random bytes
    return randomBytes(32).toString("hex");
}
```

## Compare a submitted token

The server should compare the browser-supplied token against the expected
token using a safe comparison.

```typescript
// import the constant-time comparison helper
import { timingSafeEqual } from "node:crypto";

// compare two token values safely
function tokensMatch(expected: string, supplied: string) {
    // convert the expected token into bytes
    const expectedBytes = Buffer.from(expected);

    // convert the supplied token into bytes
    const suppliedBytes = Buffer.from(supplied);

    // reject values with different lengths
    if (expectedBytes.length !== suppliedBytes.length) {
        return false;
    }

    // compare the token bytes without ordinary string comparison
    return timingSafeEqual(expectedBytes, suppliedBytes);
}
```

## Set secure cookie attributes

Cookie attributes should reflect the security requirements of the session.

```typescript
// describe the attributes for a protected session cookie
const sessionCookie = [
    "session=opaque-session-value",
    "Path=/",
    "Secure",
    "HttpOnly",
    "SameSite=Lax",
].join("; ");
```

Secure prevents ordinary HTTP transmission. HttpOnly prevents direct
JavaScript access. SameSite limits cross-site cookie sending.

## Set security headers

The server can send browser-enforced security policies with the response.

```typescript
// define security headers for the application response
const securityHeaders = {
    // require secure connections after the browser accepts the policy
    "Strict-Transport-Security": "max-age=31536000; includeSubDomains",

    // prevent MIME-type sniffing
    "X-Content-Type-Options": "nosniff",

    // restrict framing to the application itself
    "Content-Security-Policy":
        "default-src 'self'; object-src 'none'; frame-ancestors 'self'",

    // reduce unnecessary referrer information
    "Referrer-Policy": "strict-origin-when-cross-origin",

    // restrict selected browser capabilities
    "Permissions-Policy": "camera=(), microphone=(), geolocation=()",
};
```

## Build the protected handler

A state-changing endpoint can combine the browser security controls.

```typescript
// describe the protected request data
type ProtectedRequest = {
    origin: string | null;
    csrfToken: string;
    expectedCsrfToken: string;
};
```

The server checks the request before performing the protected operation.

```typescript
// protect a state-changing browser request
function protectRequest(input: ProtectedRequest) {
    // validate the browser origin
    requireTrustedOrigin(input.origin);

    // reject a request with an invalid CSRF token
    if (!tokensMatch(input.expectedCsrfToken, input.csrfToken)) {
        throw new Error("invalid csrf token");
    }

    // allow the application operation to continue
    return { ok: true };
}
```

## Add CORS deliberately

A cross-origin API should allow only the origins that actually need access.

```typescript
// build a CORS response for a trusted browser origin
function buildCorsHeaders(origin: string | null) {
    // reject an origin that is not explicitly trusted
    if (!isTrustedOrigin(origin)) {
        return {};
    }

    // allow the approved origin to read the response
    return {
        "Access-Control-Allow-Origin": origin,
        "Access-Control-Allow-Credentials": "true",
        Vary: "Origin",
    };
}
```

The server still needs authentication and authorization. CORS only controls
browser cross-origin behavior.

## Test the boundary

The security test should reject an untrusted origin.

```typescript
// demonstrate rejection of an untrusted browser origin
function testUntrustedOrigin() {
    // represent a request from another website
    const origin = "https://evil.example";

    // verify that the request is rejected
    requireTrustedOrigin(origin);
}
```

The same principle applies to the CSRF token.

```typescript
// demonstrate rejection of an incorrect CSRF token
function testInvalidToken() {
    // represent the expected request token
    const expected = createCsrfToken();

    // represent an attacker-controlled token
    const supplied = "attacker-controlled-token";

    // verify that the tokens do not match
    if (tokensMatch(expected, supplied)) {
        throw new Error("invalid token accepted");
    }
}
```

## Now type it again

Type the browser security sequence again.

```typescript
// validate the browser origin
requireTrustedOrigin(origin);

// reject a request with an invalid CSRF token
if (!tokensMatch(expectedCsrfToken, csrfToken)) {
    throw new Error("invalid csrf token");
}
```

Then reconstruct the session cookie.

```typescript
// define the attributes for a protected session cookie
const sessionCookie = [
    "session=opaque-session-value",
    "Path=/",
    "Secure",
    "HttpOnly",
    "SameSite=Lax",
].join("; ");
```

Finally, type the browser policy headers.

```typescript
// prevent MIME-type sniffing
"X-Content-Type-Options": "nosniff",

// restrict framing to the application itself
"Content-Security-Policy":
    "default-src 'self'; object-src 'none'; frame-ancestors 'self'",

// reduce unnecessary referrer information
"Referrer-Policy": "strict-origin-when-cross-origin",
```

## Wrap up

The flow: HTTPS -> origin -> cookie -> CSRF -> CORS -> security headers.

```
