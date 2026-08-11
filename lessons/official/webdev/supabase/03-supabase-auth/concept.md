# Supabase auth - concepts

Authentication answers one question:

```text
Who is making this request?
```

Authorization answers a different question:

```text
What is that user allowed to do?
```

Supabase Auth handles the first. Row Level Security and application code
handle the second.

The two concerns are connected, but they are not the same operation.

## Auth is backed by the database

Supabase Auth is not a separate identity service floating above the
database.

Its identities live in a PostgreSQL table:

```text
auth.users
```

Each registered user is a row in that table.

```text
auth.users
    id
    email
    created_at
    ...
```

This is why authentication and database access integrate so naturally.
A signed-in user becomes an identity that PostgreSQL itself can understand.

## Signing up

A common flow starts with registration.

```text
browser
    |
    | email + password
    v
Supabase Auth
    |
    v
auth.users (new row)
```

After sign-up, the user can sign in with the same credentials.

Supabase supports several identity mechanisms:

```text
email + password
passwordless email
passwordless SMS
OAuth / social providers
```

Each mechanism eventually produces the same thing: an identity in
`auth.users` and a session for the client.

## OAuth and passwordless flows

OAuth lets users sign in through an external provider.

```text
browser
    |
    | redirect
    v
provider
    |
    | consent
    v
back to Supabase
    |
    v
auth.users identity
```

The user never gives the application their provider password.

Passwordless flows work differently. Supabase sends a one-time link or code,
and the user signs in by following it.

```text
email or SMS
    |
    v
one-time link / code
    |
    v
Supabase Auth
    |
    v
auth.users identity
```

From the application's point of view, the result is the same: an identity
and a session.

## Profile tables and triggers

`auth.users` is managed by Supabase. Application-specific user data should
not be stuffed into it.

A common pattern is a separate application table.

```text
auth.users          application_profiles
    id     <------    user_id
    email             display_name
                      ...
```

Many Supabase projects create a profile row automatically when a user signs
up, using a database trigger.

```text
insert into auth.users
        |
        v
     trigger
        |
        v
insert into application_profiles
```

The profile table then belongs to the application schema and can participate
in the application's own data model.

## Sessions and tokens

Signing in produces a session.

A session is represented by tokens:

```text
access token  ->  proves the identity for a limited time
refresh token  ->  obtains a new access token when it expires
```

The access token is typically a JWT that carries the user's identity.

```text
JWT
    |
    +-- subject (user id)
    +-- issued at
    +-- expires
```

The client sends the access token with requests, and Supabase and PostgreSQL
can use it to determine who is making the request.

## Token refresh

Access tokens expire.

When an access token expires, the client can use the refresh token to obtain
a new one.

```text
access token expired
        |
        v
refresh token
        |
        v
new access token
```

In a web application this refresh typically happens automatically while a
session is still valid.

For application code, the useful consequence is that a signed-in user's
identity should be checked when it matters, rather than assuming an old token
is still valid.

## Session storage in a web application

A TanStack Start application needs the session to survive between the
browser and the server.

The session is stored in cookies.

The `@supabase/ssr` package connects Supabase's session handling to
TanStack Start's cookie helpers:

```text
browser
    |
    | session cookies
    v
TanStack Start
    |
    v
server Supabase client
```

The browser client and the server client must agree on the same session, or
the server will not recognize the signed-in user during server-side
rendering.

## getSession versus getUser

Supabase client methods that inspect the session fall into two groups.

`getSession` reads the tokens stored on the client.

`getUser` asks Supabase to validate the token and return the user.

For verifying identity in application code, `getUser` is generally the safer
choice, because it actually validates the token rather than trusting
whatever is stored locally.

```text
getSession  ->  what does this client hold?
getUser     ->  is this token valid, and for whom?
```

## Browser-side auth state

The browser also needs to know whether a user is signed in, for rendering
purposes such as showing a sign-in button or a user menu.

The browser client can hold the session and expose the current user.

```text
browser client
    |
    v
getSession / onAuthStateChange
    |
    v
current user in the UI
```

A common distinction is:

```text
UI state      ->  browser client session
authorization ->  server client getUser + database policies
```

The browser can use its session for presentation.

Security decisions should not rely only on client-side state.

## The database knows the user too

Because auth identities live in PostgreSQL, SQL can refer to the current
user.

Supabase exposes the current user to the database through helper functions:

```text
auth.uid()  ->  the current user's id, or null
auth.jwt()  ->  the current request's token claims
```

An unauthenticated request has no `auth.uid()`.

A signed-in request does.

This is the bridge between authentication and Row Level Security:

```text
signed-in user
    |
    v
auth.uid()
    |
    v
policy
    |
    v
allowed rows
```

RLS policies can therefore restrict rows to the current user without the
application passing the user id around.

## Protecting routes

Not every route should be visible to everyone.

A TanStack Start route can check whether a user is signed in before
allowing access.

```text
request
    |
    v
protected route
    |
    +-- user signed in  ->  render
    |
    +-- not signed in   ->  redirect to sign-in
```

The check belongs on the server as well as in the client.

Server-side rendering makes this especially important: a route that is
protected should not render private data into HTML for a user who is not
authenticated.

## Auth and server-side rendering

During server-side rendering, the server is rendering a page for a specific
incoming request.

That request carries the session cookies.

```text
request with cookies
    |
    v
TanStack Start server
    |
    v
server Supabase client
    |
    | forwards cookies to Supabase
    v
Supabase Auth
    |
    v
authenticated identity
```

This is exactly why the `@supabase/ssr` integration exists.

The server client from the overview lesson passes the request's cookies to
Supabase and writes any updated cookies back to the response.

Without that link, the server would render the page as if no user were
signed in, even when the browser holds a valid session.

## Auth and client navigation

After hydration, navigation happens in the browser.

A route loader may run again, and it needs the same identity the server
would have.

```text
client navigation
    |
    v
route loader
    |
    v
server-side auth check
    |
    v
fresh identity
```

The session cookies travel with the request in both cases, so the identity
check works consistently during initial rendering and during client
navigation.

## Signing out

Signing out removes the session.

The client tells Supabase to end the session, and the stored cookies are
cleared.

```text
signed-in browser
    |
    v
sign out
    |
    v
session removed
    |
    v
auth.uid() -> null on the next request
```

After sign-out, requests are anonymous again.

## Common pitfalls

Authentication looks simple until it is done incorrectly.

A few patterns to avoid:

```text
trusting getSession for security decisions
storing a secret in browser-readable code
forgetting that the browser can always change its own UI state
writing an RLS policy that does not use auth.uid()
```

A token from the browser proves only that someone holds a token.

The application and the database must still validate it and enforce rules
for the identified user.

## Auth and RLS together

Authentication and Row Level Security form the security model of a Supabase
application.

```text
browser
    |
    | access token
    v
Supabase Auth
    |
    v
authenticated identity
    |
    v
auth.uid()
    |
    v
RLS policy
    |
    v
allowed rows
```

Auth establishes who the user is.

RLS uses that identity to decide what the user may access.

This division appears throughout the rest of the track.

## The complete auth model

A signed-in TanStack Start request can be viewed as:

```text
browser
    |
    | sign-in form
    v
server Supabase client
    |
    v
Supabase Auth
    |
    v
session cookies
    |
    v
subsequent requests
    |
    v
auth.uid() in the database
    |
    v
protected routes and policies
```

The important mental model is that a session is not just a browser detail.

It becomes a database-level identity that other Supabase services, especially
Row Level Security, depend on.

## Next step

The next lesson covers Supabase Storage, which stores files and uses the same
authenticated identity to control access to objects.
