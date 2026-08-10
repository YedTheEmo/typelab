# Dependencies, Infrastructure, and Deployment Security - Typing

The examples assume TanStack Start, Vercel, Supabase, npm or Bun.

## Install dependencies reproducibly

npm:

```bash
npm ci
```

Bun:

```bash
bun install --frozen-lockfile
```

Use the command appropriate to the repository's lockfile.

## Audit dependencies

```bash
npm audit
```

or:

```bash
bun audit
```

Review the result instead of blindly upgrading every package.

## Inspect the dependency tree

npm:

```bash
npm ls --depth=0
```

Bun:

```bash
bun pm ls
```

Use this to identify dependencies that may no longer be intentional.

## Keep secrets out of source control

```gitignore
.env
.env.local
.env.*.local
```

Check the repository before committing:

```bash
git status
git diff --cached
```

Do not rely on `.gitignore` after a secret has already entered Git history.
A committed secret should be considered exposed and rotated.

## Validate production configuration

```typescript
const requiredServerEnv = [
  "SUPABASE_URL",
  "SUPABASE_SERVICE_ROLE_KEY",
];

for (const name of requiredServerEnv) {
  if (!process.env[name]) {
    throw new Error(`Missing required server environment variable: ${name}`);
  }
}
```

Do this on the server, never by exposing the complete environment to the
browser.

## Keep privileged Supabase credentials server-side

```typescript
const serviceRoleKey = process.env.SUPABASE_SERVICE_ROLE_KEY;

if (!serviceRoleKey) {
  throw new Error("Missing service role key");
}
```

Never place this value in a `VITE_*` browser variable.

## Enable RLS

A representative Supabase migration:

```sql
alter table public.orders enable row level security;

create policy "users can read their own orders"
on public.orders
for select
to authenticated
using (user_id = auth.uid());
```

Policies must reflect the application's actual authorization model.

## Protect Storage independently

Use private buckets for objects that are not public.

```sql
-- Example policy concept:
-- allow authenticated users to access only objects
-- belonging to their authorized scope.
```

Storage authorization must be designed separately from database-row
authorization.

## Run security checks in CI

A minimal npm pipeline can include:

```bash
npm ci
npm audit --audit-level=high
npm test
npm run build
```

A Bun-based project can use its corresponding install, audit, test, and build
commands.

## Deployment invariant

A useful deployment test is:

```typescript
test("production configuration does not expose privileged secrets", () => {
  expect(import.meta.env.VITE_SUPABASE_SERVICE_ROLE_KEY).toBeUndefined();
});
```

The exact build setup may differ, but privileged credentials must never be
part of client configuration.

## Review before deployment

```bash
git diff origin/main...HEAD
```

Review configuration changes, dependency changes, authentication changes,
database migrations, and deployment configuration before production release.
