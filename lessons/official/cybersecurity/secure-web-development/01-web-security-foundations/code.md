````markdown
# Web security foundations - typing

This lesson types a secure request boundary: authenticate a user, authorize
a resource, validate input, and persist only allowed data.

## Install the client

The project uses the Supabase JavaScript client for server-side operations.

```bash
# install the Supabase client
bun add @supabase/supabase-js
````

## Create the server client

The server needs a Supabase client that can inspect the authenticated user.

```typescript
// import the Supabase client factory
import { createClient } from "@supabase/supabase-js";

// read the public Supabase project URL
const supabaseUrl = process.env.SUPABASE_URL!;

// read the server-side Supabase key
const supabaseKey = process.env.SUPABASE_ANON_KEY!;

// create a client for the current application boundary
const supabase = createClient(supabaseUrl, supabaseKey);
```

## Define the request

The request contains data controlled by the browser.

```typescript
// describe the fields accepted from the browser
type UpdateProfileRequest = {
    userId: string;
    displayName: string;
};
```

The user identifier is treated as input rather than proof of identity.

```typescript
// validate the request shape before using its values
function validateRequest(input: UpdateProfileRequest) {
    // reject an empty user identifier
    if (!input.userId.trim()) {
        throw new Error("invalid user id");
    }

    // reject an empty display name
    if (!input.displayName.trim()) {
        throw new Error("invalid display name");
    }

    // reject an excessively large display name
    if (input.displayName.length > 80) {
        throw new Error("display name is too long");
    }
}
```

## Authenticate the actor

The authenticated identity comes from the server-side session.

```typescript
// retrieve the identity associated with the current session
async function requireUser() {
    // ask Supabase Auth for the current user
    const result = await supabase.auth.getUser();

    // reject the request when no authenticated user exists
    if (result.error || !result.data.user) {
        throw new Error("authentication required");
    }

    // return the authenticated identity
    return result.data.user;
}
```

## Authorize the resource

Authentication tells us who is acting. Authorization checks whether the
requested resource belongs to that identity.

```typescript
// authorize an update against the authenticated identity
function authorizeUser(
    authenticatedUserId: string,
    requestedUserId: string,
) {
    // reject requests for another user's profile
    if (authenticatedUserId !== requestedUserId) {
        throw new Error("forbidden");
    }
}
```

## Persist the change

Only authorized data reaches the database operation.

```typescript
// update a profile after all security checks have passed
async function updateProfile(input: UpdateProfileRequest) {
    // validate data supplied by the browser
    validateRequest(input);

    // establish the authenticated identity
    const user = await requireUser();

    // enforce ownership before changing the record
    authorizeUser(user.id, input.userId);

    // persist only the validated profile field
    const result = await supabase
        .from("profiles")
        .update({ display_name: input.displayName })
        .eq("id", input.userId);

    // stop when the database operation failed
    if (result.error) {
        throw new Error("profile update failed");
    }

    // return a minimal success response
    return { ok: true };
}
```

## Connect the operation

The application boundary receives the untrusted request and invokes the
security sequence.

```typescript
// expose the application operation to a request handler
export async function handleProfileUpdate(
    input: UpdateProfileRequest,
) {
    // process the request through the protected operation
    return updateProfile(input);
}
```

The important order is authentication, authorization, validation, and then
the protected operation.

## Add a database boundary

Application checks should be reinforced by database-level authorization.

```sql
-- create a policy that protects profile ownership
create policy "users update own profile"
on profiles
for update
using (auth.uid() = id);
```

The policy makes the ownership rule part of the database boundary instead of
depending entirely on application code.

## Test the security boundary

The important test is not only that the valid request works. The important
test is that changing the requested identity does not grant access.

```typescript
// describe the authorization boundary
async function testAuthorization() {
    // represent the authenticated user
    const authenticatedUserId = "user-a";

    // represent a different requested resource
    const requestedUserId = "user-b";

    // reject access across the ownership boundary
    authorizeUser(authenticatedUserId, requestedUserId);
}
```

The call above should throw because authentication does not grant ownership
of another user's resource.

## Now type it again

Type the security sequence again without changing its order.

```typescript
// validate data supplied by the browser
validateRequest(input);

// establish the authenticated identity
const user = await requireUser();

// enforce ownership before changing the record
authorizeUser(user.id, input.userId);

// persist only the validated profile field
const result = await supabase
    .from("profiles")
    .update({ display_name: input.displayName })
    .eq("id", input.userId);
```

The key distinction is between identity and authority.

```typescript
// authenticate the actor
const user = await requireUser();

// authorize the requested resource
authorizeUser(user.id, input.userId);
```

Then reinforce the same boundary at the database.

```sql
-- protect the same ownership rule inside Postgres
create policy "users update own profile"
on profiles
for update
using (auth.uid() = id);
```

## Wrap up

The flow: input -> authenticate -> authorize -> validate -> persist -> enforce.

```
```

