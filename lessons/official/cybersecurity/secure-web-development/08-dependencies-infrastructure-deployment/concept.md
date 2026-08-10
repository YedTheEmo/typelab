# Dependencies, Infrastructure, and Deployment Security

Application security extends beyond application source code. A vulnerable dependency, leaked environment variable, permissive deployment configuration, compromised build process, or incorrectly exposed backend can defeat otherwise careful application controls.

For demonstrations, assume TanStack Start on Vercel with Supabase Auth, PostgreSQL, Storage, Edge Functions, and Realtime.

## The dependency supply chain is part of the attack surface

Modern web applications execute large amounts of third-party code.

Dependencies can introduce:

- known vulnerabilities;
- malicious packages;
- compromised maintainers;
- transitive dependencies;
- unsafe post-install behavior;
- unexpected runtime behavior.

A package should not be trusted merely because it is popular.

## Keep dependencies intentional

Every dependency should have a reason to exist.

Before adding a package, consider:

- whether the platform already provides the capability;
- package maintenance activity;
- security history;
- dependency count;
- permissions and runtime behavior;
- whether the package is needed in production or only development.

Reducing unnecessary dependencies reduces attack surface.

## Lock dependency versions

A lockfile makes dependency resolution reproducible.

Use the project's package manager consistently.

For npm:

```bash
npm ci
```

For Bun:

```bash
bun install --frozen-lockfile
```

The exact command should match the repository's package manager and lockfile.

## Audit dependencies

Run the package manager's audit functionality regularly.

```bash
npm audit
```

or:

```bash
bun audit
```

An audit result is not an automatic instruction to upgrade every package. Investigate whether a vulnerability affects the application's actual dependency path and deployment.

## Avoid blind upgrades

Security updates matter, but changing major versions without review can introduce new defects.

Use a controlled process:

```text
identify
→ assess
→ update
→ test
→ deploy
→ monitor
```

## Protect the build environment

The build pipeline may have access to:

- source code;
- deployment credentials;
- package registries;
- environment variables;
- production services.

Therefore:

- minimize CI permissions;
- protect deployment tokens;
- review workflow changes;
- avoid executing untrusted pull-request code with production secrets;
- keep secrets unavailable to jobs that do not require them.

## Environment separation

Development, preview, and production environments should not casually share credentials or databases.

A useful model is:

```text
development → development resources
preview     → preview/test resources
production  → production resources
```

A preview deployment should not automatically receive production service-role credentials.

## Vercel deployment security

Vercel provides the deployment boundary for the web application.

Important controls include:

- environment-specific variables;
- restricted deployment permissions;
- protected production branches;
- review of build configuration;
- avoiding secrets in client-exposed variables.

Remember that anything included in browser JavaScript is public to the browser user.

## Supabase infrastructure security

Supabase services are separate security boundaries.

Consider independently:

- Auth configuration;
- PostgreSQL RLS;
- Storage policies;
- Edge Function authorization;
- Realtime publication and access;
- service-role credentials.

Enabling a service does not automatically authorize every user to access its data.

## RLS is an infrastructure security boundary

Supabase Row Level Security should be enabled for application tables where it is required.

The application should not rely entirely on frontend filtering.

The database should enforce the tenant or user boundary.

## Storage is an independent boundary

A database row describing a file does not automatically protect the file itself.

Supabase Storage buckets and object policies need their own access rules.

A private object should not become public merely because an application table references it.

## Production configuration

Configuration should be explicit.

Avoid production defaults such as:

```text
DEBUG=true
ALLOW_ALL_ORIGINS=true
DISABLE_AUTH=true
```

Configuration should fail closed where practical.

Missing a critical secret should normally prevent the service from starting rather than silently selecting an insecure fallback.

## Secure deployment workflow

A practical workflow is:

```text
commit
→ automated tests
→ dependency/security checks
→ preview deployment
→ review
→ production deployment
→ monitoring
```

Do not treat deployment as the point where security begins. The deployment process is itself part of the security architecture.

## Infrastructure failure assumptions

Plan for:

- expired credentials;
- unavailable upstream services;
- failed deployments;
- database outages;
- dependency vulnerabilities;
- leaked secrets;
- compromised accounts.

Security includes the ability to revoke, redeploy, restore, and recover.

## Supply-chain checklist

Before production:

- dependencies are intentional;
- lockfiles are committed;
- dependency vulnerabilities are reviewed;
- build permissions are minimized;
- production secrets are protected;
- environments are separated;
- Supabase RLS is configured;
- Storage policies are configured;
- Edge Functions enforce authorization;
- browser bundles contain no privileged secrets;
- deployment configuration has been reviewed;
- rollback and secret rotation procedures exist.
