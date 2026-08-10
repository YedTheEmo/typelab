# Input validation and output encoding - concepts

Web applications continuously move data across trust boundaries. Browser
input enters the server, server data enters templates, records are written
to databases, files are uploaded to storage, and generated content is sent
back to browsers.

The central security rule is:

```text
all externally controlled data is untrusted
until validated for its intended use
```

Validation and encoding solve different problems. Validation determines
whether data is acceptable for an operation. Encoding transforms data so that
it is interpreted safely in a particular output context.

For this course, assume TanStack Start on Vercel with Supabase Auth,
PostgreSQL, Storage, Edge Functions, and Realtime.

## Never trust input

Input can originate from:

```text
browser forms
URL parameters
query strings
HTTP headers
cookies
uploaded files
webhooks
Realtime messages
third-party APIs
database records
environment configuration
```

A browser-controlled value remains untrusted even when it came from an
interface your application generated.

For example:

```json
{
  "role": "admin"
}
```

is still attacker-controlled if submitted by the browser.

Security decisions must therefore be based on server-side validation and
trusted state.

## Validation has a purpose

Validation should answer:

```text
Is this value acceptable for this operation?
```

Different operations require different validation.

For example, an email address may need:

```text
required
reasonable length
valid application format
```

A page number may need:

```text
integer
minimum value
maximum value
```

An uploaded avatar may need:

```text
allowed file type
maximum size
acceptable dimensions
```

Validation should be defined by the business operation rather than by a
generic rule applied everywhere.

## Validate at the trust boundary

The most important validation point is where untrusted data enters a trusted
operation.

A useful model is:

```text
browser
    |
    | untrusted input
    v
TanStack Start
    |
    | validation
    v
application logic
    |
    v
Supabase
```

Client-side validation can improve usability. It cannot replace server-side
validation because attackers can bypass the client entirely.

The same principle applies to Supabase Edge Functions and other backend
entry points.

## Use allowlists where practical

An allowlist defines what is accepted.

For example:

```text
status:
    pending
    processing
    completed
    failed
```

is easier to reason about than attempting to identify every possible invalid
string.

For constrained values, prefer explicit allowed values.

```text
unknown value -> reject
```

rather than:

```text
unknown value -> attempt to interpret
```

## Validate type, length, range, and structure

A value should be validated according to how the application will use it.

Typical dimensions include:

```text
type
presence
length
range
format
enumeration
relationship
```

For example, a quantity may require:

```text
integer
>= 1
<= 100
```

A string field may require:

```text
required
1-100 characters
```

A date may require a valid representation and an acceptable business range.

Validation should happen before dangerous operations are performed.

## Do not confuse validation with sanitization

Validation asks whether data is acceptable.

Sanitization modifies data.

These are not interchangeable.

For example, an application accepting a username might reject invalid
characters rather than silently transforming arbitrary input into another
username.

Sanitization can be appropriate for some workflows, but blindly "cleaning"
attacker input can create ambiguity and security gaps.

Prefer explicit validation when the application has a clear expected format.

## Parse instead of guessing

Do not rely on implicit type coercion for security-sensitive values.

For example:

```text
"10"
```

may be a string received from an HTTP request even if the application
expects an integer.

The server should explicitly parse and validate it.

The desired flow is:

```text
raw input
   |
   v
parse
   |
   v
validate
   |
   v
typed application value
```

This makes the boundary explicit.

## Schema validation

Schema validation provides a structured description of acceptable input.

In a TypeScript application, Zod is a practical example.

A conceptual schema might define:

```text
displayName -> string, 1-80 characters
age         -> integer, 0-130
status      -> approved set
```

The schema should represent the application's accepted input, not merely the
frontend form.

A shared schema can reduce duplicated assumptions, but the server remains the
security authority.

## Validate before database operations

A database query should not receive arbitrary values when the application
can reject invalid input earlier.

The sequence should be:

```text
request
  |
  v
parse
  |
  v
validate
  |
  v
authorize
  |
  v
database operation
```

Validation and authorization solve different problems.

A valid input can still be unauthorized.

An authorized user can still submit invalid input.

## SQL injection

SQL injection occurs when attacker-controlled data changes the structure of
a SQL statement.

Conceptually unsafe construction looks like:

```text
"SELECT * FROM users WHERE email = '" + input + "'"
```

The input becomes part of SQL syntax.

The correct principle is parameterization:

```text
SQL structure
    +
parameter value
```

rather than string concatenation.

Supabase's query builder normally handles parameterization for ordinary
queries. Developers should still avoid constructing raw SQL from untrusted
strings.

## Parameterization is not authorization

A parameterized query prevents input from being interpreted as SQL syntax.

It does not determine whether the user may access the selected records.

These are separate controls:

```text
parameterization
    -> protects query structure

authorization
    -> protects resource access
```

A perfectly parameterized query can still expose every row if the application
forgot to apply an ownership or tenant restriction.

## Output is a different trust boundary

Input validation does not automatically make data safe for every output
context.

Consider a string:

```text
<profile name>
```

The same value can have different security consequences when placed into:

```text
HTML
JavaScript
CSS
URL
SQL
shell command
HTTP header
```

Encoding must therefore be selected according to the output context.

The rule is:

```text
encode for the context where the value is used
```

## HTML output encoding

If untrusted text is inserted into HTML, it must remain text rather than
becoming markup.

For example:

```text
<script>alert(1)</script>
```

should be displayed as text when it is intended to be a user's name.

Frameworks such as React escape ordinary text interpolation by default.

That protection should not be bypassed casually.

## Avoid dangerous HTML injection

React provides safe escaping for normal JSX text:

```tsx
<div>{user.displayName}</div>
```

A dangerous escape hatch is raw HTML rendering:

```tsx
<div dangerouslySetInnerHTML={{ __html: content }} />
```

Raw HTML should be treated as a security-sensitive operation.

If user-controlled content must support HTML, use a carefully designed
sanitization pipeline appropriate for the exact HTML subset.

Do not assume that removing a few obvious tags is sufficient.

## JavaScript context is different

HTML encoding does not automatically make a value safe inside JavaScript.

For example:

```html
<script>
    const name = "...";
</script>
```

is a different output context from:

```html
<div>...</div>
```

Avoid injecting untrusted data into executable JavaScript whenever possible.

Prefer structured data passed through framework mechanisms rather than
constructing JavaScript source from strings.

## URL context is different

URLs have their own parsing and security concerns.

An application accepting a redirect destination should not blindly redirect
to arbitrary external destinations.

A common problem is:

```text
/login?next=https://evil.example
```

If the server redirects there after authentication, the attacker can abuse
the application as an open redirect.

Validate redirect targets against an explicit allowlist or constrain them to
trusted relative paths.

## CSS context is different

Do not assume a value safe in HTML is automatically safe in CSS.

Avoid constructing CSS source from untrusted strings.

Prefer framework-supported properties and controlled values.

The safest approach is often to keep attacker-controlled data out of CSS
source entirely.

## HTTP header context

Untrusted data placed into HTTP headers can create response-splitting or
header-injection problems in poorly designed systems.

Frameworks and runtimes usually protect against illegal header characters,
but application code should still avoid constructing security-sensitive
headers from arbitrary input.

Redirect locations, content-disposition values, and custom headers deserve
particular attention.

## Command injection

If a web application invokes an operating-system process, untrusted input
must not be concatenated into a shell command.

Conceptually unsafe:

```text
"tool " + userInput
```

The input may become executable shell syntax.

Prefer APIs that pass arguments as structured values without invoking a shell
when possible.

The strongest design is often to avoid shell execution entirely for
user-controlled workflows.

## Path traversal

File paths are another trust boundary.

An attacker may submit:

```text
../../secret.txt
```

or an equivalent encoded representation.

If the application concatenates this into a filesystem path, the attacker may
escape the intended directory.

The application should avoid treating user input as a trusted filesystem
path.

Use identifiers mapped to known resources, normalize paths where appropriate,
and enforce the intended storage boundary.

## File upload validation

File uploads require multiple controls.

Do not trust:

```text
filename
extension
Content-Type header
```

as the only evidence of file type.

A safer upload workflow considers:

```text
authentication
authorization
maximum size
allowed media types
actual file content
storage location
generated filename
post-upload processing
```

Uploaded files should not automatically become executable application
content.

## File names are data

Original filenames can contain unusual characters, long values, path-like
segments, or misleading extensions.

Applications should generally generate storage identifiers rather than using
untrusted filenames directly as filesystem paths.

For Supabase Storage, object paths should be derived from trusted identifiers
and authorization context.

## Revalidate after upload

An upload endpoint should not assume that successful transfer means the file
is safe.

Depending on the application, further processing may include:

```text
type detection
metadata inspection
virus scanning
image decoding
thumbnail generation
content policy checks
```

The required controls depend on the file type and application.

## Validate third-party data too

Developers sometimes trust external APIs because the data did not originate
in their own browser.

That is an incorrect trust assumption.

External services can return unexpected, malformed, stale, or compromised
data.

The boundary should be:

```text
third-party response
        |
        v
parse
        |
        v
validate
        |
        v
application state
```

This is especially important for webhooks and provider integrations.

## Webhook payloads are input

A webhook request is not trusted merely because it claims to come from a
known provider.

A robust webhook flow typically includes:

```text
receive request
    |
    v
verify authenticity
    |
    v
parse payload
    |
    v
validate schema
    |
    v
process event
```

Signature verification establishes authenticity. Schema validation establishes
that the payload has the expected structure.

They solve different problems.

## Error messages

Validation errors should be useful to legitimate clients without exposing
unnecessary implementation details.

Avoid returning:

```text
SQL syntax error near ...
database hostname ...
filesystem path ...
internal stack trace ...
```

to arbitrary users.

A safer API response might identify the invalid field and expected format
while keeping internal diagnostics in server logs.

## Logging rejected input

Rejected input can be useful for security monitoring, but logs are another
data store.

Do not blindly record:

```text
password
access token
session cookie
full request body
SSN
private document content
```

when investigating validation failures.

Log the minimum information needed for diagnosis and security analysis.

## Validation and authorization work together

Consider:

```text
POST /documents
{
    "organizationId": "A",
    "title": "Report"
}
```

Validation can establish:

```text
organizationId is a valid UUID
title is 1-200 characters
```

Authorization must establish:

```text
the authenticated user may create documents in organization A
```

Both are necessary.

```text
input
  |
  v
validation
  |
  v
authorization
  |
  v
operation
```

## Validation is not a security filter for everything

A common mistake is trying to solve every injection vulnerability by
blacklisting suspicious characters.

For example:

```text
reject "<script>"
```

does not constitute an XSS defense.

Different output contexts require different encoding and safe APIs.

Likewise:

```text
reject "'"
```

does not constitute SQL injection protection.

Use parameterized queries.

The strongest security approach uses the correct mechanism for each
interpreter.

## Defense in depth

A secure application can have several independent controls:

```text
input schema
    +
authorization
    +
parameterized query
    +
database RLS
    +
output encoding
    +
CSP
```

Each addresses a different failure mode.

Do not remove a necessary control because another layer happens to exist.

## Review every data flow

For each piece of external data, trace:

```text
source
  ->
parser
  ->
validator
  ->
authorization
  ->
storage
  ->
transformation
  ->
output context
```

Ask at every stage:

```text
Can the attacker control this?
What type is it?
What constraints apply?
Who is allowed to use it?
Which interpreter receives it next?
How is it encoded?
```

This method scales from a simple form field to complex provider
integrations.

## Security review

For every input, identify:

```text
source
type
maximum size
allowed values
validation point
authorization requirement
storage destination
output context
encoding mechanism
```

For every dangerous interpreter, identify the correct defense:

```text
SQL        -> parameterized queries
HTML       -> contextual output encoding / safe framework rendering
JavaScript -> avoid dynamic code and script interpolation
URL        -> validate destination and encode components
CSS        -> controlled values
shell      -> structured arguments or avoid shell execution
filesystem -> safe resource mapping and path controls
```

The security objective is not to make input "clean."

It is to ensure that untrusted data cannot change the meaning of an operation
or escape its intended context.

## Next step

Now type the code version of this lesson.
