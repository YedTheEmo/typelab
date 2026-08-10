# Browser and HTTP security - concepts

Web applications are built on top of HTTP and executed inside a browser
security model. A developer who understands only application code can still
create vulnerabilities by misunderstanding origins, cookies, cross-origin
requests, or browser-enforced restrictions.

For this lesson, assume a TanStack Start application deployed on Vercel with
Supabase Auth, PostgreSQL, Storage, Edge Functions, and Realtime. The browser
is an untrusted client. HTTPS protects transport, but it does not decide
whether a request is authorized.

## Understand HTTP as a boundary

HTTP carries requests from a client to a server and responses back to the
client. A request can contain a method, URL, headers, cookies, and a body.
Every one of those values can affect application behavior.

A simplified request looks like this:

```text
POST /account/profile
Host: app.example.com
Cookie: session=...
Content-Type: application/json

{"displayName":"Alice"}
````

The browser may construct this request, but the server must not assume that
the browser interface was followed. A user can reproduce the same request
with another client and change its values.

HTTP therefore provides transport for security decisions rather than
providing authorization itself.

A useful mental model is:

```text
HTTP
  |
  +-- identity information
  +-- application data
  +-- browser context
  +-- security metadata
  |
  v
server-side security decisions
```

The server decides whether a request is valid and authorized.

## Use HTTPS for transport

HTTP sends application traffic without transport encryption. HTTPS uses TLS
to protect communication between the client and the server.

For a production application, the normal flow is:

```text
browser
    |
    | HTTPS
    v
Vercel
    |
    | HTTPS
    v
application services
```

TLS provides confidentiality and integrity for the protected connection. It
also allows the client to authenticate the server through certificates.

TLS does not make malicious application requests safe. An attacker who
controls a user's browser can still send a valid HTTPS request containing
malicious input. HTTPS protects the channel, not the application logic.

The distinction matters because developers sometimes treat "we use HTTPS"
as a complete security answer. It is only one layer.

## Understand origins

An origin is defined by scheme, host, and port.

For example:

```text
https://app.example.com
```

and:

```text
https://api.example.com
```

have different origins because their hosts differ.

The browser uses origins to isolate documents and control which resources and
requests can interact across boundaries.

This is the basis of the same-origin policy. Without origin isolation, a
malicious website could freely inspect information belonging to another
website that the user is logged into.

The same-origin policy is therefore a browser security boundary rather than
an application authorization system.

## Same-origin policy

A browser page normally has broad access to resources belonging to its own
origin and restricted access to resources belonging to another origin.

Consider:

```text
https://app.example.com
https://evil.example
```

The second site should not automatically be able to read private responses
from the first site.

The browser applies origin restrictions to many operations, including
scripts, documents, and cross-origin network requests. The exact behavior
depends on the browser API being used.

This distinction is important:

```text
cross-origin request
```

does not necessarily mean:

```text
cross-origin response can be read
```

A browser may send a request while still preventing the initiating page from
reading the response.

That distinction is central to understanding CORS and CSRF.

## Understand CORS

CORS means Cross-Origin Resource Sharing. It allows a server to tell browsers
which other origins may read certain responses.

Suppose the frontend is:

```text
https://app.example.com
```

and an API is:

```text
https://api.example.com
```

The API can return a response such as:

```text
Access-Control-Allow-Origin: https://app.example.com
```

The browser can then allow JavaScript from the approved origin to read the
response.

CORS is a browser enforcement mechanism. It is not authentication and it is
not authorization.

A server must still authenticate and authorize the request.

A dangerous misconception is:

```text
CORS blocks attackers from calling my API
```

That is not generally true. Non-browser clients do not have to obey browser
CORS enforcement. An API still needs its own authentication, authorization,
input validation, and abuse controls.

## Avoid permissive CORS with credentials

Credentials can include cookies and other browser-managed authentication
state. Cross-origin credentialed requests therefore require more careful
configuration.

A server should not blindly combine permissive origins with credentialed
access.

The security question is:

```text
which exact origins are allowed to make authenticated browser requests?
```

An explicit allowlist is easier to reason about than an unrestricted policy.

CORS configuration should match the actual application architecture. If the
TanStack Start application and API are same-origin, introducing broad
cross-origin access may provide no benefit and can create unnecessary
complexity.

## Understand cookies

Cookies are browser-managed pieces of state associated with a domain and
path. They are commonly used for sessions and authentication.

A cookie might be configured conceptually as:

```text
session=...
Secure
HttpOnly
SameSite=Lax
```

The Secure attribute tells the browser to send the cookie only over secure
connections. HttpOnly prevents JavaScript from directly reading the cookie.
SameSite controls when the browser sends the cookie in cross-site contexts.

These attributes address different threats.

```text
Secure
    protects transport usage

HttpOnly
    reduces direct JavaScript access

SameSite
    controls cross-site sending behavior
```

None of these attributes replaces server-side authorization.

## Understand HttpOnly

An HttpOnly cookie cannot normally be read through JavaScript APIs such as
document.cookie.

This limits the usefulness of some attacks that execute JavaScript in the
application's origin. For example, an XSS vulnerability may still allow an
attacker to perform actions as the victim, but an HttpOnly session cookie is
not directly exposed to JavaScript.

HttpOnly therefore reduces one consequence of client-side script compromise.
It does not make XSS harmless.

The server still needs to validate requests and enforce authorization.

## Understand SameSite

SameSite controls whether a cookie is sent with cross-site requests.

The main modes are:

```text
Strict
Lax
None
```

Strict applies the strongest cross-site restriction. Lax permits some
top-level navigation cases. None allows cross-site cookie sending and
requires Secure.

SameSite can significantly reduce CSRF exposure, but developers should
understand the exact behavior of their authentication and navigation flows
rather than treating it as a universal CSRF solution.

Applications with complex cross-site integrations may need explicit CSRF
protection as well.

## Understand CSRF

Cross-Site Request Forgery abuses a browser's existing authority.

Suppose a user is authenticated to:

```text
https://bank.example
```

A malicious site can attempt to cause the victim's browser to send a request
to that site. If authentication credentials are automatically attached, the
target may receive a request associated with the victim's session.

The attack does not require stealing the password.

The core problem is:

```text
browser automatically supplies authority
        +
attacker controls another website
        +
target accepts the request
```

CSRF primarily matters when authentication is automatically attached to
requests, especially with cookies.

## Defend against CSRF

A strong CSRF defense ensures that a cross-site attacker cannot manufacture a
valid state-changing request.

Common controls include SameSite cookies, CSRF tokens, and validating request
origins where appropriate.

A token-based pattern looks like:

```text
authenticated session
        +
unpredictable request token
        |
        v
state-changing operation
```

The token must be associated with the intended security context and checked
server-side.

Applications should not rely on a value that an attacker can freely predict
or obtain from the cross-site context being defended.

## Distinguish CSRF from CORS

CORS and CSRF address different problems.

CORS controls whether browser JavaScript can read cross-origin responses.

CSRF concerns unwanted requests made with the victim's existing authority.

A server can therefore have:

```text
strict CORS
```

and still be vulnerable to:

```text
CSRF
```

Conversely, an API can have broad CORS access while remaining secure if it
does not use ambient browser credentials and has strong authentication and
authorization.

The correct defense depends on the authentication architecture.

## Choose browser storage carefully

Web applications can store values in cookies, localStorage, sessionStorage,
IndexedDB, and application memory. These mechanisms have different security
properties.

JavaScript-readable storage is accessible to scripts running in the
application's origin. If an attacker achieves script execution through XSS,
sensitive values stored there may become accessible.

Cookies can provide HttpOnly protection, but they introduce browser-managed
credential behavior that must be considered for CSRF.

There is no universal rule that one storage mechanism is always secure.
Choose storage based on what the value represents, who needs access to it,
and which browser threats apply.

For sensitive authentication state, avoid exposing secrets to client
JavaScript when the architecture does not require it.

## Control framing

A malicious site can sometimes attempt to display another site inside a
frame. If users interact with the framed application, attackers may attempt
to manipulate the presentation or trick users into clicking something.

This is commonly called clickjacking.

Applications can restrict framing through response headers such as
Content-Security-Policy frame-ancestors.

The security goal is:

```text
trusted framing origins
        |
        v
application
```

rather than allowing arbitrary sites to embed the application.

## Use security headers

Security headers communicate security policies to browsers.

Important examples include:

```text
Content-Security-Policy
Strict-Transport-Security
X-Content-Type-Options
Referrer-Policy
Permissions-Policy
```

Each serves a different purpose.

HSTS tells browsers to use HTTPS for a site. Content-Security-Policy
restricts which resources and scripts may execute. X-Content-Type-Options
reduces MIME-sniffing behavior. Referrer Policy controls referrer
information. Permissions Policy controls access to selected browser
capabilities.

Headers are defense layers. They do not replace authentication,
authorization, or secure application logic.

## Content Security Policy

CSP restricts what a browser may load or execute for a document.

A policy can constrain scripts, styles, images, connections, frames, and
other resource classes.

A conceptual policy might look like:

```text
default-src 'self';
script-src 'self';
object-src 'none';
```

The exact policy must match the application. An overly broad policy provides
less protection, while an overly restrictive policy can break legitimate
features.

CSP is especially valuable as a defense layer against XSS. It can reduce the
impact of injected scripts when the policy prevents arbitrary script
execution.

CSP should therefore complement safe output handling rather than replace it.

## Avoid mixed content

Mixed content occurs when a secure HTTPS page loads certain resources through
insecure HTTP.

For example:

```text
HTTPS page
    |
    +-- HTTP script
```

The insecure resource weakens the security assumptions of the page.

Production applications should serve resources securely and avoid insecure
resource dependencies.

A modern deployment through Vercel should normally use HTTPS for the
application and its externally loaded resources.

## Control response data

Browser security is not only about preventing requests. The data returned by
the server also matters.

An endpoint that returns excessive information can expose sensitive records
even when authentication succeeds.

For example, a profile endpoint should not automatically return unrelated
organization records merely because the authenticated user is valid.

A secure response follows the principle:

```text
authorized request
        |
        v
minimum necessary data
```

This reduces the amount of sensitive information exposed to the browser.

## Treat browser APIs as security-sensitive

Modern applications use many browser capabilities: fetch, WebSockets,
WebRTC, geolocation, notifications, storage, workers, and service workers.

Every capability introduces assumptions about origins, permissions, or
untrusted input.

The developer should ask:

```text
what origin owns this data?
who can invoke this API?
what does the browser automatically attach?
what can JavaScript read?
what can an embedded page access?
```

These questions reveal security boundaries before implementation details
obscure them.

## Secure the whole request path

A useful model for a TanStack Start application is:

```text
browser
    |
    | HTTPS
    v
Vercel
    |
    v
TanStack Start
    |
    +--> Supabase Auth
    |
    +--> Supabase Postgres
    |
    +--> Supabase Storage
    |
    +--> Supabase Edge Functions
    |
    +--> Supabase Realtime
```

Each layer has different responsibilities.

The browser enforces browser security rules. HTTPS protects transport.
TanStack Start handles application behavior. Supabase Auth establishes
identity. Postgres and Storage can enforce data boundaries. Edge Functions
provide server-side operations. Realtime introduces persistent browser
connections that must also respect authorization.

No single layer can safely be assumed to solve all of these problems.

## Security review questions

When reviewing a web feature, ask where the request originates and which
origin receives it. Determine whether credentials are automatically attached.
Check whether the browser is allowed to read the response. Identify whether
the operation changes state.

Then inspect the server:

```text
who is authenticated?
what are they authorized to do?
what input do they control?
what data is returned?
what browser policy applies?
```

Finally inspect the underlying service:

```text
does Postgres enforce the boundary?
does Storage enforce the boundary?
does Realtime expose only permitted data?
does an Edge Function trust client input?
```

This approach connects browser behavior to application and data-layer
security.

## Next step

Now type the code version of this lesson.

```
