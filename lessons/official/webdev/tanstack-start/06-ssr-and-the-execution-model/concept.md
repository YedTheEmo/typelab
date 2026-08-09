# TanStack Start SSR and the execution model - concepts

TanStack Start can render a React application on the server before sending
the resulting HTML to the browser.

This creates an execution model that is more complicated than a purely
client-rendered React application.

The same application can participate in several stages:

```text
server request
    |
    v
route matching
    |
    v
server rendering
    |
    v
HTML
    |
    v
browser
    |
    v
hydration
    |
    v
client navigation
```

Understanding these stages is important because code does not necessarily
execute in the same environment at every stage.

## Client-side rendering

In a purely client-rendered application, the browser receives JavaScript
and constructs the interface there.

A simplified model is:

```text
browser
    |
    v
download JavaScript
    |
    v
React renders
    |
    v
HTML appears
```

The server may provide the static files, but the browser performs the
application rendering.

This model is straightforward because there is only one application
execution environment: the browser.

TanStack Start adds another environment.

## Server-side rendering

With server-side rendering, the server participates in producing the initial
HTML.

The browser requests a URL:

```text
GET /
```

The Start server can then process that request, match the route tree, load
the required route data, and render the React application.

Conceptually:

```text
browser
   |
   | HTTP request
   v
Start server
   |
   +-- match routes
   |
   +-- load route data
   |
   +-- render React
   |
   v
HTML response
```

The browser receives HTML that already represents the rendered application.

This is the first major server-side capability added by Start around the
router model.

## Why render on the server?

Server rendering means the browser does not have to start with an empty
application and construct the initial UI entirely from JavaScript.

The server can produce meaningful HTML as part of the initial response.

This can improve the initial rendering experience and allows the application
to participate in a traditional request-response lifecycle while still
becoming a client-side application afterward.

The important point is that server rendering does not mean the application
stays server-rendered forever.

It only describes how the initial document is produced.

## Hydration

After receiving server-rendered HTML, the browser still needs JavaScript
to make the application interactive.

React performs hydration.

Hydration takes the HTML that already exists and connects it to the
browser-side React application.

The simplified sequence is:

```text
server renders
    |
    v
HTML arrives
    |
    v
React hydrates
    |
    v
application becomes interactive
```

Hydration is therefore different from rendering the application from
scratch.

The server has already produced the markup.

The browser-side React runtime needs to attach itself to that markup and
establish the interactive application.

## Server rendering versus hydration

These two operations happen in different environments.

Server rendering:

```text
server -> React -> HTML
```

Hydration:

```text
browser -> React -> existing HTML
```

This distinction matters when debugging.

If something fails during server rendering, the problem exists in the
server execution stage.

If something fails during hydration, the browser has received HTML but the
browser-side application cannot correctly attach to it.

The two stages can therefore produce different classes of problems.

## Client-side navigation after hydration

Once hydration has completed, navigating between routes does not normally
require the browser to request another complete HTML document.

The router can handle the transition.

For example:

```text
/current
    |
    | click Link
    v
/router
    |
    v
/new-route
```

The router matches the new route and obtains whatever data or code is
required for that transition.

The existing application remains active while the relevant route hierarchy
changes.

This produces the hybrid behavior associated with a full-stack application:

```text
initial navigation -> server
later navigation   -> browser router
```

The distinction is conceptual rather than absolute because applications can
also perform explicit document navigations and other server interactions.

## Isomorphic execution

Full-stack React applications contain code that may be shared between
server and browser environments.

This is sometimes called isomorphic or universal code.

For example, a component can be rendered on the server and later hydrated
in the browser.

The component therefore needs to be valid in both environments.

A component that accesses the DOM during rendering can cause problems because
the server does not provide the browser's DOM APIs.

For example:

```tsx
function Component() {
    return <p>{document.title}</p>
}
```

The browser has `document`, but server-side rendering does not provide the
same browser environment.

The important rule is:

```text
shared rendering code must work where it executes
```

Do not assume that browser APIs exist during server rendering.

## Server-only code

Some operations must never execute in browser code.

Examples include:

```text
database credentials
private API keys
filesystem access
server-only environment variables
```

A server can safely access these resources because they remain outside the
browser.

Sending such values to the browser would expose them to the user.

The execution boundary therefore becomes a security boundary.

A useful model is:

```text
server
    |
    +-- secrets
    +-- database
    +-- private services
    |
    v
browser
    |
    +-- public UI
    +-- user interaction
    +-- browser APIs
```

The server can send the results of private operations to the browser, but
the private implementation and credentials must remain server-side.

## Route code is not automatically server-only

A route file can contain multiple kinds of logic.

For example:

```tsx
export const Route = createFileRoute("/users")({
    loader: loadUsers,
    component: Users,
})
```

The route participates in routing, while its loader and component have their
own execution characteristics.

A component that is rendered during SSR participates in server execution.
The same component is later used by the browser after hydration.

A loader can also participate in different stages depending on whether the
route is being loaded as part of server rendering or client-side navigation.

Therefore, the correct question is not:

```text
"Is this file a route file?"
```

The useful question is:

```text
"Where does this particular operation execute?"
```

## The server boundary

TanStack Start provides explicit mechanisms for keeping server operations
on the server.

Server functions are one example.

A server function lets application code request server-side work without
putting the implementation itself into browser code.

Conceptually:

```text
browser code
    |
    | invoke
    v
server function
    |
    v
server-only operation
```

This is important because simply importing a database library into a
component is not an acceptable substitute for establishing a server
boundary.

The framework needs to know which code belongs to the server.

## Data during SSR

Route loaders are especially important during server rendering.

A route may require data before its UI can be rendered.

The server can therefore perform the route's loading work and use the result
while producing HTML.

Conceptually:

```text
request
    |
    v
match route
    |
    v
loader
    |
    v
route data
    |
    v
React render
    |
    v
HTML
```

The browser receives the resulting document.

This is one reason route loaders are integrated into the router rather than
being treated as arbitrary component-side effects.

The router knows that the route has a data requirement as part of the
rendering process.

## Data during navigation

After hydration, a navigation can instead occur entirely through the
client-side router.

The sequence becomes:

```text
click Link
    |
    v
match destination
    |
    v
load required route data
    |
    v
render destination
```

The browser can therefore perform route transitions without repeating the
entire initial document request.

This is the second half of Start's hybrid execution model.

## Server and browser have different capabilities

The server and browser are not interchangeable environments.

The server can generally access:

```text
database
filesystem
private credentials
server environment
```

The browser can access:

```text
DOM
local browser storage
user input
browser APIs
```

Neither environment should be treated as if it were the other.

A full-stack application is successful when code is deliberately placed
according to the capabilities and security requirements of the environment
where it executes.

## The complete execution model

The entire application lifecycle can be summarized as:

```text
                 initial request
                       |
                       v
                Start server
                       |
                +------+
                |
                v
             route match
                |
                v
            load route data
                |
                v
            render React
                |
                v
                HTML
                |
                v
              browser
                |
                v
             hydration
                |
                v
         client-side router
                |
                v
          later navigation
```

The initial document is produced through server-side rendering.

The browser then hydrates that document.

Afterward, the router can manage client-side navigation while still
coordinating route data and other application behavior.

## Why this matters for the next lessons

Server functions, server routes, middleware, authentication, database access,
and environment variables all depend on understanding the execution boundary.

Without that model, it is easy to confuse:

```text
"this code is used by a route"
```

with:

```text
"this code is guaranteed to execute only on the server"
```

Those statements are not equivalent.

The next lesson will introduce server functions as an explicit mechanism
for crossing the browser-server boundary.

## Next step

Now type the code version of this lesson.
