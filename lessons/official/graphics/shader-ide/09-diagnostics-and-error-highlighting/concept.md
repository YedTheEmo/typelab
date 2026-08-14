# Diagnostics and error highlighting - concepts

The compiler produces diagnostics, and the IDE's job is to make them visible
in the right place. A compile error is only useful if the user can see which
line caused it. This lesson connects three systems: the compiler's diagnostic
text, the text buffer's line mapping, and the ImGui interface that displays
both a diagnostics list and highlighted lines.

## Diagnostics have a shape

The Slang compiler reports problems with a location and a severity. The
location is a line and column. The severity separates errors from warnings.

A typical diagnostic line:

```text
shader.slang(12, 5): error: unknown identifier 'flaot4'
```

The IDE parses this into a structured form:

```cpp
struct Diagnostic {
    int line;
    int column;
    bool isError;
    std::string message;
};
```

Structured diagnostics let the interface do useful things: sort by line,
group errors, highlight the offending line, and jump to the location on
click.

## From compiler to buffer

The compiler speaks line and column. The text buffer speaks offsets. The
buffer lesson already provided the conversion:

```text
line 12, column 5
    -> offsetOf(buffer, 12, 5)
    -> the caret can jump there
```

This one conversion is the bridge that turns a diagnostic into a cursor
position and a highlighted line.

## The diagnostics panel

The diagnostics panel is an ImGui window that lists every diagnostic from the
last compile.

```text
Diagnostics
  error  shader.slang(12,5): unknown identifier 'flaot4'
  error  shader.slang(14,9): 'color' is not declared
  warning shader.slang(3,1): unused variable 'seed'
```

Each row shows the severity and the message. Clicking a row moves the editor's
cursor to that location. This is a simple, high-value interaction: the user
sees the list, clicks, and lands on the problem.

## Error line highlighting

The editor paints a background highlight behind lines that have an error. The
renderer needs the set of error line numbers, which it derives from the
diagnostic list.

```text
line 3:  ...                              normal
line 12: float4 c = flaot4;               <-- red highlight
line 14: color += c;                      <-- red highlight
```

The editor is drawn as a sequence of lines, so highlighting is a per-line
decision based on the set of error lines. The highlight survives edits as
long as the line numbers still match the current buffer, which is true until
the next compile replaces the diagnostic list.

## Editor interaction

The editor panel supports the basics: clicking to move the cursor, typing to
insert, and scrolling to view long shaders. The diagnostics system adds two
behaviors on top:

```text
click a diagnostic  -> move the editor cursor there
hover / select      -> the highlighted line stays visible
```

The cursor jump is implemented with the buffer's offsetOf conversion. This
is the same conversion the highlighting uses, so the three features share one
piece of code.

## Keeping diagnostics after edits

Diagnostics are valid for the source that produced them. When the user edits
the source, the old diagnostics may point at lines that no longer exist. The
IDE has two choices: keep the old list until the next compile, or clear it on
the first edit.

Keeping the old list is more useful. The user can fix the error, and the
highlight stays visible until the next compile confirms the fix. When the
reload loop compiles successfully, the list is cleared and the highlights
disappear.

```text
edit -> old diagnostics stay
compile succeeds -> clear diagnostics
compile fails    -> replace with new diagnostics
```

## Severity drives presentation

Errors and warnings are styled differently. Errors use a red tint and block
the pipeline swap. Warnings use a yellow tint and do not block the preview.

```text
error   -> red, pipeline not rebuilt
warning -> yellow, pipeline rebuilt normally
```

This distinction is cheap to implement and makes the panel legible at a
glance.

## The status line

The IDE also keeps a compact status line that summarizes the last compile:

```text
compiling...
ok (3421 bytes)
3 errors
```

The status line is always visible, so the user gets feedback even when the
diagnostics panel is closed. The reload lesson introduced the status; this
lesson gives it the same source of truth as the panel.

## What this lesson establishes

Compiler output is now visible where it matters. The diagnostics panel lists
every message, the editor highlights error lines, and clicking a diagnostic
jumps to the location. The loop that used to end at "compile" now continues
to "explain the failure at the exact line."

## Next step

Now type the code version of this lesson.
