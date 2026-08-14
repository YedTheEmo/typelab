# Editor text buffer - concepts

The editor is the subsystem the user touches most. It stores shader source,
accepts edits, and reports the cursor position. Its data structure must
support three operations well: editing anywhere in the buffer, moving the
cursor, and mapping between byte offsets and line-and-column positions.

The third operation matters more than it first appears. The Slang compiler
reports errors by line and column. The IDE displays those errors by pointing
at a line in the editor. A text buffer that cannot answer "which line is this
offset on?" cannot show compile errors, which makes it useless for a shader
IDE.

## Text is the whole problem

A shader source file is text. Everything the editor does is an operation on
that text: insert a character, delete a character, split a line, move the
cursor. There is no hidden structure. The editor does not need to understand
Slang syntax.

```text
characters
    |
    +--> lines (split by newline)
    |
    +--> cursor (a position in the text)
    |
    +--> offsets (byte positions)
```

Because the text is small by GPU standards, the editor can use a simple
representation: one contiguous string plus an index of line start positions.

## The buffer representation

The buffer is stored as a single string containing the whole source, with
newline characters separating lines. A separate index stores the byte offset
where each line begins.

```text
text:        float4 color = float4(1.0);\nvoid main() {}\n
lineOffsets: 0, 28
```

The first line starts at offset zero. The second line starts at offset
twenty-eight, immediately after the first newline.

This representation is easy to build and easy to debug. The index is rebuilt
from the text whenever the structure changes, which is cheap for shader-sized
files.

## The cursor is an offset

The cursor is a byte offset into the text. The cursor can sit anywhere from
zero to the length of the text.

```cpp
uint32_t cursorOffset;
```

Keeping the cursor as a single offset makes edits simple: inserting a
character at the cursor shifts every later offset by one. The line index does
not need to change for most edits because the offset values stay correct.

The editor derives the visible line and column from the offset when it needs
them.

## Line and column from offset

Given the line index, converting an offset to a line number is a search: find
the last line start that is not after the offset.

```text
offset 40 in { 0, 28, 41 }
    -> line 2 (starts at 41)
```

The column is the offset minus the line start.

```text
offset 43 - line start 41 = column 2
```

The compiler reports diagnostics as line and column. This conversion is the
bridge between the compiler's coordinate system and the editor's.

## Editing operations

Every edit is a small, well-defined mutation of the text and the cursor.

Inserting a character places the character at the cursor and advances the
cursor by one.

```text
before:   "float c|or"
after:    "float co|r"
```

Deleting a character removes the character before the cursor and moves the
cursor back by one.

```text
before:   "float co|r"
after:    "float c|or"
```

Inserting a newline splits the current line at the cursor and advances the
cursor past the newline.

```text
before:   "float c|olor"
after:    "float c\n|olor"
```

These are the primitive operations. Higher-level behavior such as tab
insertion and undo is built from them.

## Cursor movement

Cursor movement changes the offset without changing the text. The direction
moves are defined relative to lines and columns.

```text
left    -> offset - 1 (clamped)
right   -> offset + 1 (clamped)
up      -> move to previous line, same column (clamped to line)
down    -> move to next line, same column (clamped to line)
```

Up and down movement is defined in line-and-column space, so the editor
converts the current offset to line and column, moves the line number, and
converts back.

```text
offset
    -> line, column
    -> line + 1, column
    -> offset (clamped to the new line)
```

This is why the line index exists: moving vertically needs the line
boundaries.

## Why an index instead of a list of lines

An alternative representation stores each line as its own string in a list.
Editing is then easy on the current line, but joining and splitting lines
requires moving data between list entries.

The index representation keeps the text in one place. Any edit is a change
to the string plus a possible rebuild of the index. For shader-sized sources
this rebuild is negligible, and the single-string form is trivial to pass to
the Slang compiler, which wants the whole source as one contiguous buffer.

## The buffer feeds the compiler

The editor owns the source string. The compiler consumes it. The interface
between them is exactly one function:

```cpp
bool compile(const char* source, size_t length, ...);
```

Because the buffer stores text as one contiguous string, handing it to the
compiler is a pointer and a length, not a reassembly of lines.

```text
editor.text
    |
    v
Slang compiler
    |
    v
SPIR-V or diagnostics
```

The editor does not care which of the two results arrives. It only needs to
display diagnostics, which requires the line mapping the editor already
provides.

## Diagnostics come back to the buffer

A compile error is reported with a location:

```text
error 30059: 'foo' : no member 'bar'
shader.slang(7, 5): error ...
```

The IDE extracts the line number and column, then asks the editor to convert
them back to an offset so it can highlight the offending line.

```text
line 7, column 5
    -> line start for line 7
    -> offset = lineStart + 4
    -> highlight that range
```

The text buffer is therefore not just a place to type. It is the coordinate
system that both editing and diagnostics share.

## The buffer is not a widget

The buffer is pure data and pure operations. It has no knowledge of ImGui,
windows, or pixels. The drawing layer converts the buffer's offsets and lines
into ImGui text rendering, but that conversion lives outside the buffer.

This separation keeps the buffer testable and reusable. If the IDE later
gains syntax highlighting, the highlight engine works on the same buffer
structure without changing its interface.

## What this lesson establishes

The text buffer is the substrate of the editor. It stores one contiguous
string, keeps a line index, tracks a cursor offset, and answers two kinds of
queries: edit operations and offset-to-line conversions. Later lessons wire
this structure to ImGui text input, to the Slang compiler, and to error
highlighting.

## Next step

Now type the code version of this lesson.
