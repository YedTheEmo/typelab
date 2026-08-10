# Authentication sessions and identity - concepts

Authentication establishes which identity is making a request. A web
application then needs a reliable way to maintain that identity across
requests and distinguish authentication from authorization.

For this course, assume a TanStack Start application deployed on Vercel.
Supabase Auth provides identity services, while Supabase PostgreSQL, Storage,
Edge Functions, and Realtime remain separate service boundaries. The browser
is untrusted.

## Authentication is identity

Authentication answers:

```text
Who is making this request?
```

Authorization answers:

```text
What may that identity do?
```

The distinction is fundamental. Supabase Auth can establish an authenticated
user identity. That identity does not automatically establish permission to
read another user's records, administer an organization, download a private
document, or change billing data.

The security flow is:

```text
credentials
    ->
authentication
    ->
identity
    ->
authorization
    ->
resource
```

A failure at any stage should prevent the protected operation.

## Use a managed identity system

Authentication contains many security-sensitive concerns: credential
verification, session lifecycle, recovery, email verification, token
handling, and account protection.

Using Supabase Auth delegates much of this identity infrastructure to a
specialized service.

The application still owns important decisions:

```text
application
    |
    +-- which routes require authentication
    +-- which resources require authorization
    +-- which identities can perform actions
    +-- which data is exposed
    +-- how recovery affects application state
```

A managed authentication service reduces implementation burden. It does not
remove application security responsibility.

## Understand credentials

Credentials are evidence presented to establish identity. Passwords are one
example. Other mechanisms include one-time codes, passkeys, and external
identity providers.

The application should not need to inspect or store a user's plaintext
password.

The desired relationship is:

```text
password
    |
    v
authentication service
    |
    v
authenticated identity
```

For a managed Supabase Auth deployment, password verification belongs to the
authentication system rather than ordinary application tables.

## Passwords are not secrets to encrypt

Encryption and password hashing solve different problems.

Encryption is designed so authorized parties can recover plaintext. Password
storage should instead use a one-way password hashing scheme with an
appropriate work factor.

The desired relationship is:

```text
password
    ->
password hash
```

not:

```text
password
    ->
encrypted password
```

If an attacker obtains a password database, reversible encryption creates a
different recovery problem from a properly configured password hash.

A developer using Supabase Auth should generally avoid implementing password
hashing manually. The important principle is understanding why plaintext
passwords and reversible password storage are unsafe.

## Credential attacks

Authentication endpoints are exposed to automated attacks.

Common examples include password guessing, credential stuffing, password
spraying, and account enumeration.

Credential stuffing is especially important because attackers can reuse
username and password combinations leaked from another service.

Authentication is therefore an abuse-sensitive boundary.

Useful controls include:

```text
rate limiting
authentication monitoring
strong recovery flows
MFA where appropriate
breach-aware password policies
generic failure responses
```

A strong password does not solve credential reuse elsewhere.

## Avoid account enumeration

An authentication or recovery endpoint can accidentally reveal whether an
account exists.

For example, these responses expose different information:

```text
"no account exists"
```

and:

```text
"invalid password"
```

An attacker can use that distinction to build a list of valid accounts.

Where enumeration matters, responses should avoid unnecessarily revealing
whether a particular account exists.

The exact behavior depends on the authentication flow and product
requirements. The principle is to disclose only what the requester needs to
continue.

## Understand sessions

After authentication, users should not need to submit credentials for every
request. A session associates later requests with an authenticated identity.

Conceptually:

```text
login
  |
  v
session established
  |
  +--> request
  +--> request
  +--> request
  |
  v
logout or expiration
```

The session is therefore a bearer of authority.

If an attacker obtains a usable session credential, the attacker may be able
to act as the victim until the credential expires or is invalidated.

Session security is consequently as important as password security.

## Protect session credentials

Session credentials should not be exposed unnecessarily to browser
JavaScript.

A common protection for cookie-based sessions is:

```text
Secure
HttpOnly
SameSite
```

Secure restricts transmission to HTTPS. HttpOnly prevents ordinary JavaScript
access. SameSite controls cross-site cookie behavior.

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

An XSS vulnerability can still cause a browser to perform actions as the
authenticated user even when the session cookie itself cannot be read.

## Understand session expiration

Sessions need a lifecycle:

```text
created
  ->
active
  ->
expired
  ->
revoked
```

Expiration limits the useful lifetime of a stolen credential. Revocation
allows the application or identity system to terminate access before normal
expiration.

The appropriate lifetime depends on the application. Administrative
sessions, ordinary sessions, refresh tokens, and recovery credentials may
have different requirements.

Avoid treating "logged in" as a permanent security state.

## Rotate sensitive credentials

Some credentials should change when the security context changes.

Examples include session identifiers after authentication, recovery tokens
after use, and credentials after compromise.

Rotation limits the usefulness of credentials captured before the security
state changed.

The general principle is:

```text
security boundary changes
        |
        v
old credential may no longer remain valid
```

This is particularly important for session fixation.

## Understand session fixation

Session fixation occurs when an attacker can cause a victim to authenticate
using a session identifier that the attacker already knows.

The dangerous sequence is:

```text
attacker knows session
        |
        v
victim authenticates
        |
        v
same session becomes authenticated
        |
        v
attacker reuses session
```

A secure authentication system should establish or rotate authenticated
session state so that a pre-authentication identifier does not become a
long-lived authenticated credential.

Managed authentication systems can handle much of this lifecycle. The
application should avoid inventing a parallel session mechanism that
undermines the provider's protections.

## Avoid duplicate identity systems

A common architectural mistake is creating a second user identity system
beside Supabase Auth without a clear reason.

For example:

```text
Supabase Auth user
        +
custom login table
        +
custom session token
```

This creates multiple sources of truth.

A stronger design is:

```text
Supabase Auth
      |
      v
authenticated identity
      |
      v
application profile
      |
      v
authorization data
```

The application database can store profile and business information while
Supabase Auth remains responsible for authentication identity.

## Link application data to identity

Authentication becomes useful when application records can be associated with
the authenticated identity.

A profile may conceptually contain:

```text
id
display_name
created_at
```

The `id` corresponds to the identity established by the authentication
system.

The application can then reason about ownership:

```text
authenticated user
        |
        v
profile
        |
        v
owned resources
```

This relationship should be enforced by the data model and authorization
policies rather than merely assumed by frontend code.

## Handle authentication state on the server

Protected operations should establish authentication on the server.

The browser can request:

```text
GET /account
```

but the server must determine whether the request is associated with an
authenticated identity.

A useful boundary is:

```text
browser request
      |
      v
TanStack Start server
      |
      v
Supabase Auth
      |
      v
authenticated identity
```

The client should not be able to choose an arbitrary user identifier and have
the server accept it as the authenticated identity.

## Do not trust client identity fields

Suppose a browser submits:

```text
user_id=administrator
```

That value is input. It is not authentication.

The server must derive identity from the authenticated session rather than
from a request body, query parameter, hidden form field, or frontend state.

The secure relationship is:

```text
session
   |
   v
authenticated user
   |
   v
server-side identity
```

rather than:

```text
request.user_id
   |
   v
trusted identity
```

This distinction prevents identity and access-control mistakes.

## Understand MFA

Multi-factor authentication adds another category of evidence to the
authentication process.

The basic idea is:

```text
something you know
        +
something you have
        |
        v
stronger authentication
```

A password alone can be compromised through phishing, credential reuse,
malware, or database breaches. An additional factor can reduce the impact of
a stolen password.

MFA is especially useful for administrative and high-impact accounts.

MFA does not authorize arbitrary application operations. A user who
successfully completes MFA is still subject to authorization rules.

## Secure account recovery

Recovery is another authentication path.

A system with strong login security but weak password recovery is still
vulnerable.

A conceptual recovery flow is:

```text
recovery request
    ->
temporary credential
    ->
identity verification
    ->
credential replacement
    ->
old recovery credential invalidated
```

Recovery tokens should be difficult to guess, short-lived where appropriate,
and invalidated after use.

Recovery endpoints should also avoid unnecessary account enumeration.

## Treat email verification as identity state

Email verification can establish that a user controls a particular email
address.

It does not automatically establish that the user is entitled to every
application privilege associated with that account.

For example:

```text
verified email
        !=
administrator
```

Application roles and privileges remain authorization concerns.

## Treat tokens according to purpose

Applications may encounter access tokens, refresh tokens, recovery tokens,
email verification tokens, CSRF tokens, and webhook signatures.

These are not interchangeable.

Each token has a purpose, lifetime, audience, and trust boundary.

Ask:

```text
what authority does this token represent?
```

That answer determines where it can be stored, who can receive it, how long
it should remain valid, and what must happen if it is compromised.

Do not pass a privileged server credential to a browser simply because the
browser needs some authenticated functionality.

## Never expose privileged keys

Supabase provides credentials with different capabilities.

A privileged server-side key must remain on the server. It must never be
embedded in browser JavaScript, shipped in a client bundle, or stored in
public source code.

The deployment boundary should look like:

```text
browser
    |
    | public client configuration
    v
application

server
    |
    | privileged secrets
    v
Supabase services
```

Vercel environment variables and server-side configuration provide mechanisms
for keeping deployment credentials outside source code.

The exact credential available to a client must match the provider's security
model.

## Log authentication events carefully

Authentication events are useful security signals.

Examples include:

```text
login succeeded
login failed
password recovery requested
session revoked
MFA challenge failed
privileged account authenticated
```

Logs should not contain passwords, session tokens, recovery tokens, or other
credentials.

A useful event records security-relevant facts without recording the secret
that proves them.

For example:

```text
user_id
event_type
timestamp
request metadata
```

can be useful, while a raw bearer token is generally dangerous to retain.

## Build authentication around failure

Authentication code should fail closed.

Examples include:

```text
missing identity -> reject
invalid credential -> reject
expired session -> reject
invalid recovery token -> reject
missing authentication configuration -> fail safely
```

Do not interpret an authentication error as anonymous success.

A protected route should not become public because the identity lookup failed.

Keep these states distinct:

```text
authenticated
unauthenticated
authentication failed
```

They should not collapse into one permissive fallback.

## Authentication architecture

The baseline architecture for this course is:

```text
browser
    |
    v
TanStack Start
    |
    v
Supabase Auth
    |
    v
authenticated identity
    |
    +--> application profile
    |
    +--> authorization rules
    |
    +--> Postgres resources
    |
    +--> Storage resources
    |
    +--> Realtime resources
```

Supabase Auth establishes identity. The application maps that identity to
business data and permissions. PostgreSQL, Storage, and Realtime remain
security boundaries that must not assume authentication alone grants
unrestricted access.

## Authentication review

When reviewing authentication, ask:

```text
where are credentials verified?
where does identity come from?
where is the session maintained?
can the browser read the credential?
how long does the credential remain valid?
can it be revoked?
what happens after authentication?
how does recovery work?
what happens when authentication fails?
```

Then inspect privileged boundaries:

```text
which secrets exist?
which credentials reach the browser?
which routes require identity?
which actions require additional authorization?
```

This prevents authentication from becoming an isolated login feature rather
than part of the application's security architecture.

## Next step

Now type the code version of this lesson.
