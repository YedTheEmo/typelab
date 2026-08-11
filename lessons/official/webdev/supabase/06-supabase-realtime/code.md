# Supabase realtime - typing

This lesson types a realtime channel that carries chat messages, presence
tracking, and Postgres Changes subscriptions inside a TanStack Start
component.

## Subscribe to a channel

A channel is created from the Supabase client.

```tsx
// import the browser-side Supabase client
import { createClient } from "@/lib/supabase/client"

// create a client for browser-side realtime access
const supabase = createClient()

// join a named channel
const channel = supabase
    .channel("room-42")
    // receive messages broadcast by other clients
    .on("broadcast", { event: "chat" }, (payload) => {
        // add the incoming message to the UI state
        console.log("received:", payload.payload)
    })
    // receive database changes for the messages table
    .on(
        "postgres_changes",
        { event: "INSERT", schema: "public", table: "messages" },
        (payload) => {
            // handle the newly inserted row
            console.log("new message row:", payload.new)
        },
    )
    // establish the connection
    .subscribe()
```

The channel now receives both application events and database changes.

## Broadcast a message

Any client on the channel can send a broadcast.

```tsx
// send a chat message to the channel
await channel.send({
    // identify the broadcast type
    type: "broadcast",

    // identify the event name the receiver is listening for
    event: "chat",

    // provide the payload
    payload: {
        message: "Hello from another client",
        user: "user-1",
    },
})
```

Every other client subscribed to `room-42` receives the event.

## Track presence

Presence keeps a synchronized set of online clients.

```tsx
// join the channel with presence tracking
const presenceChannel = supabase
    .channel("room-42")
    // listen for presence state changes
    .on("presence", { event: "sync" }, () => {
        // read the current set of online users
        const users = presenceChannel.presenceState()
    })
    // establish the connection
    .subscribe()
```

After subscribing, the client reports itself as present.

```tsx
// announce this client's presence
await presenceChannel.track({
    // identify the client
    user_id: "user-1",

    // provide a display name
    user_name: "Ada",
})
```

All subscribers see the updated presence state.

## Use the channel in a component

The subscription belongs to the component's lifecycle.

```tsx
// import the React hook for running effects
import { useEffect } from "react"

// import the browser-side Supabase client
import { createClient } from "@/lib/supabase/client"

// define a component that receives live messages
function LiveMessages() {
    // run the subscription setup when the component mounts
    useEffect(() => {
        // create the browser client
        const supabase = createClient()

        // join the channel
        const channel = supabase
            .channel("room-42")
            .on("postgres_changes", {
                // listen for inserts
                event: "INSERT",

                // watch the public schema
                schema: "public",

                // watch the messages table
                table: "messages",
            }, (payload) => {
                // handle the new message row
                console.log(payload.new)
            })
            // establish the connection
            .subscribe()

        // remove the subscription when the component unmounts
        return () => {
            // close the channel
            channel.unsubscribe()
        }
    }, [])

    // render the live message area
    return <div>Live messages</div>
}
```

The cleanup function prevents connection leaks.

## Receive a single row change

Realtime delivers the changed row in the event payload.

```tsx
// handle a single table change event
channel.on("postgres_changes", {
    // react to inserts only
    event: "INSERT",
    schema: "public",
    table: "messages",
}, (payload) => {
    // access the new row
    const message = payload.new

    // render the message text
    console.log(message.text)
})
```

The payload carries the row data as it now exists in the database.

## Practice

Type the core subscription shape:

```tsx
const channel = supabase
    .channel("room-42")
    .on("broadcast", { event: "chat" }, (payload) => {
        console.log(payload.payload)
    })
    .subscribe()
```

Then type the broadcast:

```tsx
await channel.send({
    type: "broadcast",
    event: "chat",
    payload: { message: "Hello", user: "user-1" },
})
```

And the presence tracking:

```tsx
await channel.track({ user_id: "user-1", user_name: "Ada" })
```

The central pattern is:

```text
channel
  -> Broadcast    (client-to-client events)
  -> Presence     (who is online)
  -> Postgres Changes (database-to-client)
  -> component subscribe / unsubscribe
```

Realtime pushes events to the clients that are listening.
