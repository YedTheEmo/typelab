# Authentication sessions and identity - typing

This lesson types authentication with Supabase Auth: create a server client,
derive identity from the session, protect a route, and keep privileged
credentials on the server.

## Install Supabase

The application uses the Supabase JavaScript client.

```bash
# install the Supabase JavaScript client
bun add @supabase/supabase-js
```

## Configure the server client

The server reads its Supabase configuration from the environment.

```typescript
// import the Supabase client factory
import { createClient } from "@supabase/supabase-js";

// read the Supabase project URL
const supabaseUrl = process.env.SUPABASE_URL!;

// read the server-side Supabase key
const supabaseKey = process.env.SUPABASE_ANON_KEY!;

// create the server-side Supabase client
const supabase = createClient(supabaseUrl, supabaseKey);
```

The credential belongs to the server environment rather than browser code.

## Require an authenticated user

The server derives identity from Supabase Auth instead of trusting request
data.

```typescript
// retrieve the identity associated with the current session
async function requireUser() {
    // ask Supabase Auth for the current authenticated user
    const result = await supabase.auth.getUser();

    // reject requests without a valid authenticated identity
    if (result.error || !result.data.user) {
        throw new Error("authentication required");
    }

    // return the identity established by authentication
    return result.data.user;
}
```

The returned user is the server's identity source.

## Define a protected operation

The protected operation receives data from the browser, but it does not use
browser data to establish identity.

```typescript
// describe data supplied by the browser
type UpdateProfileInput = {
    displayName: string;
};
```

The authenticated identity comes from the session.

```typescript
// update the profile belonging to the authenticated user
async function updateProfile(input: UpdateProfileInput) {
    // establish the authenticated identity
    const user = await requireUser();

    // reject an empty display name
    if (!input.displayName.trim()) {
        throw new Error("display name required");
    }

    // reject an excessively long display name
    if (input.displayName.length > 80) {
        throw new Error("display name too long");
    }

    // update only the authenticated user's profile
    const result = await supabase
        .from("profiles")
        .update({ display_name: input.displayName })
        .eq("id", user.id);

    // reject database failures
    if (result.error) {
        throw new Error("profile update failed");
    }

    // return a minimal success result
    return { ok: true };
}
```

Notice that there is no `userId` supplied by the browser.

## Expose the protected operation

A TanStack Start server function can call the protected operation.

```typescript
// expose the authenticated profile operation
export async function saveProfile(input: UpdateProfileInput) {
    // process the request through the protected operation
    return updateProfile(input);
}
```

The server function becomes the application boundary between the browser and
the authenticated operation.

## Store application identity data

Authentication identity can be associated with an application profile.

```typescript
// describe application data linked to an authenticated identity
type Profile = {
    id: string;
    displayName: string;
};
```

The profile identifier should correspond to the authenticated user identity.

```sql
-- create a profile table keyed by the authentication identity
create table profiles (
    id uuid primary key references auth.users(id),
    display_name text not null
);
```

The database now expresses the identity relationship directly.

## Protect the profile with RLS

The database can reinforce the application's ownership rule.

```sql
-- enable row level security for profile records
alter table profiles enable row level security;

-- allow users to read only their own profile
create policy "users read own profile"
on profiles
for select
using (auth.uid() = id);

-- allow users to update only their own profile
create policy "users update own profile"
on profiles
for update
using (auth.uid() = id);
```

The database boundary uses the authenticated identity rather than a
client-supplied user identifier.

## Handle logout

Authentication state needs a lifecycle.

```typescript
// end the current authenticated session
async function logout() {
    // ask Supabase Auth to sign out the current session
    const result = await supabase.auth.signOut();

    // reject an unsuccessful logout operation
    if (result.error) {
        throw new Error("logout failed");
    }
}
```

The application should treat logout as the end of the current authenticated
state rather than simply navigating to another page.

## Reject missing authentication

A protected route must fail when no authenticated identity exists.

```typescript
// demonstrate the protected identity boundary
async function protectedRoute() {
    // retrieve the authenticated identity
    const user = await requireUser();

    // return only identity information needed by this operation
    return {
        userId: user.id,
    };
}
```

The `requireUser` function throws before the protected operation when the
session is missing or invalid.

## Keep privileged credentials server-side

A privileged key must not be placed in browser code.

```typescript
// read a privileged secret only from the server environment
const serviceRoleKey = process.env.SUPABASE_SERVICE_ROLE_KEY!;
```

The secret can then be used by controlled server operations when its
privileges are actually required.

```typescript
// create a privileged server-side Supabase client
const adminClient = createClient(
    supabaseUrl,
    serviceRoleKey,
);
```

This client must never be imported into client-side application code.

## Test identity separation

The security boundary should use the authenticated identity rather than a
request-supplied identity.

```typescript
// describe an operation that receives only mutable profile data
type ProfileInput = {
    displayName: string;
};

// protect the profile operation with the authenticated identity
async function secureProfileUpdate(input: ProfileInput) {
    // derive identity from the authenticated session
    const user = await requireUser();

    // update the profile using the trusted identity
    return supabase
        .from("profiles")
        .update({ display_name: input.displayName })
        .eq("id", user.id);
}
```

There is no user identifier for an attacker to replace in the request.

## Now type it again

Type the authentication boundary again, then reconstruct the ownership
operation and its database policy.

```typescript
// retrieve the identity associated with the current session
const result = await supabase.auth.getUser();

// reject requests without a valid authenticated identity
if (result.error || !result.data.user) {
    throw new Error("authentication required");
}

// return the identity established by authentication
return result.data.user;
```

```typescript
// establish the authenticated identity
const user = await requireUser();

// update only the authenticated user's profile
const result = await supabase
    .from("profiles")
    .update({ display_name: input.displayName })
    .eq("id", user.id);
```

```sql
-- allow users to update only their own profile
create policy "users update own profile"
on profiles
for update
using (auth.uid() = id);
```

## Wrap up

credentials -> Supabase Auth -> identity -> protected operation -> database
