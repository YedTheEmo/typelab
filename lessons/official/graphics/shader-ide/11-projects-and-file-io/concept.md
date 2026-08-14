# Projects and file I/O - concepts

A shader IDE becomes a real tool when its work survives a restart. This
lesson adds the file layer: loading shader source from disk, saving it back,
and organizing multiple shaders into one project. The file layer is thin
because the text buffer already stores source as a plain string, and the
compiler already accepts that string.

## The project is a folder

A project is a directory containing shader source files. The IDE treats the
project folder as the working directory for shader names.

```text
my-project/
  gradient.slang
  plasma.slang
  mouse-paint.slang
```

Keeping shaders as plain files means the user can edit them with any tool and
share them with version control. The IDE adds no hidden project format for
this course; the files are the project.

## Loading a shader

Loading a shader file is reading its text into the editor buffer.

```text
file path
    |
    v
read file -> string
    |
    v
buffer.text = string
    |
    v
rebuild line index
    |
    v
mark dirty -> triggers recompile
```

The buffer's rebuildLines operation from the buffer lesson is exactly what
loading needs. After the source is in the buffer, the normal reload loop
compiles it and the preview updates. Loading is therefore just an edit that
replaces the whole buffer.

## Saving a shader

Saving writes the buffer text back to disk. Because the buffer stores the
whole source as one string, saving is a single write call.

```cpp
writeFile(path, buffer.text);
```

The saved file is the exact source the user sees. There is no separate serial
form, which keeps save and load trivially symmetric.

## Multiple shaders

The IDE can hold several shaders at once and switch between them. Each
shader is a slot with its own source, name, entry point, and compile state.

```text
Shader slots
    |
    +--> gradient.slang   (active)
    +--> plasma.slang
    +--> mouse-paint.slang
```

A tab bar shows the open shaders. Clicking a tab switches the active shader,
which swaps the buffer contents, triggers a compile, and updates the preview.

## The tab bar

The tab bar is an ImGui widget that lists the shader names and tracks the
active one.

```text
[ gradient.slang | plasma.slang | + ]
```

A plus button opens a new empty shader. Each tab carries the file path, so
switching is a lookup, not a reload.

## Entry point selection

A shader module can contain more than one entry point, or the user may want
to switch between fragment and compute modes. The IDE exposes the entry point
as a per-shader setting.

A combo box lists the available entry points:

```text
entry point: [main  v]
```

The compiler already finds the entry point by name. The IDE stores the
selected name in the shader slot and passes it to the compile call. Changing
the entry point recompiles with the new selection.

## Marking changes

A shader is dirty when its buffer differs from the saved file. The IDE shows
a marker on the tab and warns before switching or closing a shader with
unsaved changes.

```text
gradient.slang *   <- modified
```

The marker is derived from comparing the buffer text to the last saved text.
This is cheap and needs no extra state beyond the saved copy.

## Autosave

Because shaders are plain text, the IDE can autosave after every successful
compile. This protects against crashes and makes the files always reflect the
latest working shader.

```text
compile succeeds
    |
    v
write buffer to disk (if the file is already known)
```

Autosave is optional but cheap, and it makes the IDE feel safer. The save
happens only for shaders that were loaded from a file, so a new unsaved
shader is never written over an existing file.

## The File menu

The menu bar from the layout lesson hosts the file commands:

```text
File
    New Shader
    Open Shader...
    Save
    Save As...
```

The commands mutate the slot list and the active buffer. Open and save use a
file dialog; for this course a simple path prompt or a fixed project folder
is sufficient.

## The new shader template

A new shader starts from a small template so the user is never staring at an
empty buffer:

```slang
[shader("fragment")]
float4 main(float4 position : SV_Position) : SV_Target
{
    return float4(1.0, 0.0, 0.0, 1.0);
}
```

The template is the same ShaderToy-style shape the pipeline expects, so the
first compile immediately produces a visible preview.

## What this lesson establishes

Shaders now live on disk and can be loaded, saved, switched, and autosaved.
The editor holds multiple shader slots with per-shader entry points. The
remaining work is the compute execution path and the final integration that
ties the whole application together.

## Next step

Now type the code version of this lesson.
