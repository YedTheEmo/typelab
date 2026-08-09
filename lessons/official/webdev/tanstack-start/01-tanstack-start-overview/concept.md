# TanStack Start overview - concepts

TanStack Start is a full-stack React framework built around TanStack Router.

The important mental model is that Start does not replace the router with a
separate application model. The route tree remains the structure of the
application, while Start adds the server capabilities around that structure.

A useful simplified flow is:

request -> server -> route -> render -> browser

After the first document reaches the browser, the router can handle later
navigation without requesting another complete HTML document.

## The route tree

A TanStack Start application is organized around a route tree.

Each route describes a location in the application and can also contain
behavior associated with that location. This can include UI, parameters,
data loading, error handling, and navigation behavior.

A simplified route definition looks like this:

```tsx
const Route = createFileRoute("/")({
    component: Home,
})
```

The route is therefore more than a URL-to-component mapping. It is a unit
that participates in the application's routing and execution model.

Routes can also be nested. A parent route can provide structure that remains
present while a child route changes, which is why the route tree is central
to the way a Start application is composed.

## File-based routing

TanStack Start commonly uses file-based routing.

A file such as:

```text
src/routes/index.tsx
```

represents the root route, while another file can represent a different URL:

```text
src/routes/about.tsx
```

The filesystem therefore becomes a convenient representation of the route
tree.

A route file is not simply a page containing a React component. It is a
route module that can define the behavior associated with that part of the
application.

This becomes increasingly important as routes begin to load data, define
pending and error behavior, and participate in server rendering.

## Route data

A route can define a loader that provides data required by its UI.

For example:

```tsx
export const Route = createFileRoute("/")({
    loader: () => {
        return {
            message: "hello",
        }
    },
})
```

The loader produces data associated with the route. The route component can
then retrieve that data:

```tsx
function Home() {
    const data = Route.useLoaderData()

    return <h1>{data.message}</h1>
}
```

The important relationship is:

route -> loader -> data -> component

The loader describes what data the route needs, while the component describes
how that data should be displayed.

This separation becomes particularly useful when the application starts
loading data during server rendering or client-side navigation.

## Server rendering

Start can render the initial application on the server.

When the browser requests a URL, the server can determine the matching route
tree, execute the necessary server-side work, and render the React
application into HTML.

The browser therefore receives an already-rendered document:

```text
browser -> request -> Start server
browser <- HTML <- rendered route tree
```

The actual rendering pipeline contains additional stages, but this model
captures the important distinction from a purely client-rendered application.

With purely client-side rendering, the browser receives an application shell
and JavaScript constructs the interface there. With server rendering, the
server participates in producing the initial HTML before the browser takes
over interactive behavior.

## Hydration

Server-rendered HTML is only the first stage.

After the browser loads the application's JavaScript, React hydrates the
HTML. Hydration connects the browser-side React application to the markup
that the server already produced.

After hydration, the application can respond to interaction and perform
client-side navigation.

The simplified lifecycle is:

```text
server render -> HTML -> browser -> hydration -> client navigation
```

This explains why a Start application can behave like an interactive
client-side application while still using server rendering for its initial
document.

## Server and browser execution

A full-stack framework introduces an important boundary: some operations
belong on the server, while others belong in the browser.

Browser code can access things such as the DOM and browser APIs. Server code
can access resources that should never be exposed to the browser, such as
private credentials, database connections, and server-side environment
secrets.

The location of a file does not by itself determine where every piece of
its code executes.

For example, route-related code can participate in both server rendering
and later client-side navigation. You therefore need to understand the
execution context of an operation rather than assuming that everything
associated with a route is server-only.

A useful distinction is:

```text
browser code -> executes in the browser
server code  -> executes on the server
```

Start provides explicit mechanisms for performing server-only operations
from application code. Server functions are one of those mechanisms.

## Server functions

A server function represents an operation whose implementation runs on the
server.

Conceptually, application code can invoke an operation while the actual
work remains on the server:

```text
application -> server function -> server
```

This is useful for operations such as database access, authentication,
private API calls, and other work that should not be placed directly into
browser code.

The important idea is that a server function is not simply a normal
JavaScript function containing a database call. The framework provides the
mechanism for crossing the client-server boundary while keeping the
implementation on the server.

Server functions will be explored separately later in the track.

## Server routes

Start also provides server routes.

A server route participates directly in the HTTP layer. It can receive an
HTTP request and produce an HTTP response.

Conceptually:

```text
HTTP request -> server route -> HTTP response
```

This makes server routes appropriate when the application needs an actual
HTTP endpoint.

The distinction between the two server mechanisms is therefore useful:

```text
server function -> application-level server operation
server route   -> HTTP-level endpoint
```

Both execute on the server, but they represent different boundaries and
are intended for different kinds of communication.

## The complete mental model

The major pieces fit together like this:

```text
                    TanStack Start
                          |
              +-----------+-----------+
              |                       |
        TanStack Router          server runtime
              |                       |
         route tree             server functions
              |                 server routes
       +------+------+             middleware
       |             |
      UI          route data
       |
    browser
```

TanStack Router provides the navigable structure of the application.

TanStack Start surrounds that structure with server capabilities such as
server rendering and server-side operations.

The browser receives the initial rendered application, hydrates it, and can
then use the router for subsequent navigation.

## Why the mental model matters

The later lessons introduce routing, nested layouts, loaders, server
rendering, server functions, server routes, middleware, and deployment.

Those features are easier to understand when their boundaries are clear.

A route is not merely a URL.

A loader is not merely a fetch call.

Server rendering is not merely React running somewhere else.

A server function is not merely a normal function with a special name.

Each feature exists because a full-stack application has to coordinate
routing, data, HTTP, server execution, and browser execution.

Understanding those relationships first makes the individual APIs much
easier to reason about.

## Next step

Now type the code version of this lesson.
