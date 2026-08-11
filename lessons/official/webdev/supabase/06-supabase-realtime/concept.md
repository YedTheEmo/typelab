# Supabase realtime - concepts

The previous lessons all followed the same direction:

```text
client
   |
   v
request
   |
   v
Supabase
   |
   v
response
```

The client asks, and the server answers.

Realtime adds the opposite direction.

```text
event
   |
   v
Supabase Realtime
   |
   v
connected clients
```

Instead of the client polling for changes, Supabase pushes events to
connected clients as they happen.

## Polling versus push

Without realtime, a client can approximate live data by polling.

```text
client          Supabase
   |                |
   | request        |
   |--------------->|
   | response       |
   |<---------------|
   |                |
   | request        |
   |--------------->|
   | response       |
   |<---------------|
```

The client repeatedly asks whether anything changed.

With Realtime, the connection stays open, and events arrive when they occur.

```text
client          Supabase
   |                |
   | change in DB   |
   |<---------------|
   | change in DB   |
   |<---------------|
```

Realtime is therefore not simply database polling. It is an event-delivery
mechanism.

## Channels

Realtime communication happens over channels.

A channel is a named connection point that clients join.

```text
channel "room-42"
    |
    +-- client A
    +-- client B
    +-- client C
```

Clients subscribe to a channel to receive its events.

Channels let an application separate different live features:

```text
room-42        ->  chat for a room
presence-42    ->  who is in the room
album-9        ->  changes to one album
```

## Channel lifecycle

A channel has a lifecycle.

A client subscribes, receives events, and eventually unsubscribes.

```text
subscribe
    |
    v
connected / receiving events
    |
    v
unsubscribe
```

Subscriptions hold a persistent connection.

If a component is destroyed while its channel stays subscribed, the
connection leaks and keeps consuming resources.

Every subscription therefore needs a matching cleanup, typically tied to the
component's unmount.

## Connection states

A channel passes through states as it connects.

```text
subscribing
    |
    v
subscribed / receiving events
    |
    v
unsubscribed (cleanup)
```

If a connection drops, the client may reconnect.

```text
connected
    |
    v
disconnected
    |
    v
reconnecting
    |
    v
connected again
```

Application code should not assume a single uninterrupted connection.

Events may be missed during a disconnect, so a feature that needs complete
history should load it from the database and use Realtime for the changes
after that point.

## Broadcast

Broadcast sends a message to every client subscribed to a channel.

```text
client A
    |
    | broadcast "new message"
    v
channel
    |
    +-- client A (sender)
    +-- client B
    +-- client C
```

Broadcast is useful for application events such as chat messages,
collaborative cursor positions, or any event that does not necessarily come
from the database.

Messages are delivered to whoever is listening on the channel.

## Presence

Presence tracks which clients are currently on a channel.

```text
channel "room-42"
    |
    +-- user 1  (online)
    +-- user 2  (online)
    +-- user 3  (online)
```

Clients report their presence, and Realtime keeps the set of online clients
synchronized across all subscribers.

Presence is useful for:

```text
who is online
who is typing
live member lists
```

Each connected client receives the state of everyone else on the channel.

## Postgres Changes

Postgres Changes delivers database changes to subscribers.

```text
insert / update / delete
        |
        v
PostgreSQL
        |
        v
Supabase Realtime
        |
        v
subscribed clients
```

A client subscribes to changes on a table.

```text
change on albums
        |
        v
event with the new row
        |
        v
clients subscribed to albums
```

When the application or any client changes a row, subscribers receive an
event describing the change.

## Choosing what to watch

A Postgres Changes subscription can be scoped.

A client can watch:

```text
a specific table
a specific schema
a specific operation (insert / update / delete)
```

```text
table: "messages"
    +-- insert
    +-- update
    +-- delete
```

Watching only what the feature needs keeps events targeted and avoids
delivering unrelated traffic to every listener.

## Events carry the changed data

A Postgres Changes event includes the row data.

An insert event carries the new row.

```text
INSERT  ->  new row
```

An update event can carry both the old and the new row.

```text
UPDATE  ->  old row + new row
```

A delete event carries the row that was removed.

```text
DELETE  ->  removed row
```

The client receives enough information to update its own state without
re-querying the whole table.

## Authorization still applies

Realtime does not bypass the security model.

The same identity from the auth lesson applies to subscriptions.

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
can this client receive this event?
```

A client should not receive changes it would not be allowed to read.

The details of how policies gate Realtime connect to the Row Level Security
lesson that comes next in this track.

## Use cases

Realtime becomes valuable when multiple clients care about the same changing
data.

```text
chat
collaborative editing
live dashboards
activity feeds
notifications
```

Each use case shares one shape:

```text
change or event
    |
    v
Realtime
    |
    v
all interested clients
```

## Realtime in TanStack Start

A TanStack Start component can subscribe to a channel and update its state
when events arrive.

```text
component mounts
    |
    v
subscribe to channel
    |
    v
handle events -> update UI
```

Because subscriptions hold a connection, they need a lifecycle.

When a component unmounts, the subscription should be removed so the
connection does not leak.

```text
mount   ->  subscribe
unmount ->  unsubscribe
```

This is the same subscribe/cleanup pattern used for any resource tied to a
component's lifetime.

## Broadcast, Presence, and Postgres Changes together

The three mechanisms cover different needs:

```text
Broadcast          ->  send events between clients
Presence           ->  track who is connected
Postgres Changes   ->  follow database changes
```

An application can use all three at once.

For example, a collaborative room might use Broadcast for messages,
Presence for the member list, and Postgres Changes to follow the room's
stored records.

## A combined chat feature

The three mechanisms come together naturally in a chat feature.

A message sent by one client can take two paths.

```text
message sent
    |
    +-- Broadcast -> everyone online in the room
    |
    +-- INSERT row -> Postgres Changes -> everyone subscribed
```

Broadcast delivers to currently connected clients.

The database row is the durable record that any client can load later.

Presence answers who is in the room.

```text
room-42
    |
    +-- Broadcast     -> new messages while connected
    +-- Presence      -> online member list
    +-- Postgres Changes -> the stored message history
```

This is the common realtime shape: durable state in PostgreSQL, live
delivery through Realtime, and presence on top.

## When realtime is not the answer

Realtime adds value when clients must react promptly to changes.

It is not needed for data that changes rarely or is only read on request.

```text
request/response data      ->  loaders, not Realtime
frequently changing data   ->  consider Realtime
one-shot notifications     ->  loaders or polling may suffice
```

Realtime subscriptions also hold connections and consume resources.

Opening channels for everything without a reason is wasteful.

The right default is to use regular queries first and add Realtime where
live updates actually matter.

## The complete realtime model

```text
database change / client event
        |
        v
     channel
        |
        +-- Broadcast
        +-- Presence
        +-- Postgres Changes
        |
        v
connected clients
        |
        v
TanStack Start components
```

Realtime is the piece of Supabase that lets data flow toward the client
instead of only answering client requests.

## Next step

The next lesson covers Row Level Security, the mechanism that controls what
each identity can access, including the events Realtime delivers.
