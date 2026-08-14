# Shader hot reload - concepts

A shader IDE earns its name when the preview updates without any explicit
action. The user types, pauses, and the preview reflects the new shader. This
is hot reload: detect that the source changed, wait for the user to pause,
recompile, and swap the running pipeline.

The previous lessons built the pieces. This lesson assembles them into a
loop that watches the editor, debounces edits, and rebuilds the preview.

## The edit-to-preview loop

Hot reload is the automation of the golden loop:

```text
source changes
    |
    v
wait for pause
    |
    v
compile
    |
    v
success? -> rebuild pipeline
     |        |
     +-> keep old pipeline
    (and show diagnostics)
    |
    v
next frame renders the result
```

Two decisions make this reliable: when to compile, and what to do when
compilation fails.

## Detecting a change

The editor marks the source as dirty whenever an edit happens. Inserting a
character, deleting one, or pasting text sets a flag. The reload loop checks
that flag each frame.

```cpp
void insertChar(TextBuffer& buffer, char c) {
    buffer.text.insert(buffer.text.begin() + buffer.cursor, c);
    buffer.cursor++;
    buffer.dirty = true;
}
```

Dirty is a property of the buffer, because every mutation sets it. The reload
loop only needs to know whether anything changed since the last compile.

## Debouncing

Compiling on every keystroke is wasteful. A user typing a word triggers one
compile per character, and most of those compiles are thrown away before the
shader is valid.

The standard answer is a debounce: start a timer when the source becomes
dirty, and compile only when the source has been stable for a short interval,
such as a quarter of a second.

```text
t0  edit -> timer restarts
t1  edit -> timer restarts
t2  edit -> timer restarts
...
tN  no edits -> timer expires -> compile
```

The debounce turns a burst of keystrokes into one compile after the burst.
The interval is a tuning choice: too short compiles mid-typing, too long
makes the preview feel slow.

## The reload state

The reload loop keeps a small amount of state:

```text
dirty      -> source changed since the last compile
stableTime -> time when the source last changed
compiling  -> a compile is in progress
```

A compile is a synchronous, fast operation, so the IDE can run it directly in
the frame loop. No threads are needed for shader-sized sources.

## The compile transaction

A compile must not corrupt the running preview. The safe pattern is to
compile and build everything new before destroying anything old.

```text
compile new source
    |
    v
create new fragment module
    |
    v
create new pipeline
    |
    v
destroy old module and pipeline
    |
    v
store the new pipeline
```

Building the new pipeline while the old one is still used by the last frame
guarantees that a failure anywhere in the sequence leaves the preview intact.

## Keep last good

The most important rule of hot reload is that a bad edit must not blank the
preview. If the user types a syntax error, the compile fails, and the renderer
must continue using the last successful pipeline.

```text
compile fails
    |
    v
diagnostics -> shown to the user
    |
    v
pipeline unchanged -> preview keeps running
```

This keeps the preview alive during the normal back-and-forth of editing.
The user sees the error and the last working image side by side, which is the
feedback loop that makes shader development fast.

## Replacing the fragment stage

The pipeline depends on the fragment shader, which is the part that changes.
Rebuilding the pipeline requires:

```text
destroy the old fragment module
    |
    v
create the new fragment module from fresh SPIR-V
    |
    v
create a new pipeline with the new module
    |
    v
destroy the old pipeline
```

The vertex module never changes, so it is created once and reused.

## GPU is busy

The old pipeline is still in flight while the new one is being built. The
renderer must not destroy a pipeline that the GPU is still using. Before
swapping, the IDE waits for the graphics queue to drain:

```cpp
vkDeviceWaitIdle(device);
```

This is a deliberate cost. The IDE pauses briefly at each successful reload
to guarantee that old objects are free to destroy. For an interactive editor
this is acceptable, and it removes an entire class of lifetime bugs.

## Feedback to the user

The user needs to know what happened without staring at the preview. The IDE
keeps a compile status:

```text
status = compiling
status = ok (SPIR-V size, elapsed time)
status = error (count, first message)
```

The status is shown in the interface. When a reload succeeds, the preview
changes; when it fails, the status explains why. The diagnostics lesson turns
this into a full panel.

## The reload check

Each frame the reload loop runs one small check:

```cpp
if (sourceDirty && now - stableTime > debounceInterval) {
    recompile();
}
```

This single condition is the entire trigger mechanism. Everything else is
the transaction that follows.

## What this lesson establishes

The preview now updates itself. Edit, pause, and the shader recompiles and
re-renders without any button press. The keep-last-good rule guarantees that
failed edits never destroy the running preview. The next lessons surface the
compiler's diagnostics in the editor and arrange the whole interface.

## Next step

Now type the code version of this lesson.
