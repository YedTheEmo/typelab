# Editor text buffer - typing

This lesson types the text buffer: a contiguous source string, a line index,
a cursor offset, editing operations, cursor movement, and the offset-to-line
conversions that connect the editor to compiler diagnostics.

## Define the buffer

The buffer stores the source and its line index.

```cpp
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>

// a text buffer with a line index and a cursor
struct TextBuffer {
    // the whole source text, newlines included
    std::string text;

    // byte offset where each line begins
    std::vector<uint32_t> lineOffsets;

    // the current cursor byte offset
    uint32_t cursor = 0;
};
```

## Rebuild the line index

The index is rebuilt from the text whenever the structure changes.

```cpp
// rebuild the line index from the text
void rebuildLines(TextBuffer& buffer) {
    // start with a fresh index
    buffer.lineOffsets.clear();

    // the first line always begins at offset zero
    buffer.lineOffsets.push_back(0);

    // scan the whole text for newlines
    for (uint32_t i = 0; i < buffer.text.size(); i++) {
        // record the line start after each newline
        if (buffer.text[i] == '\n') {
            buffer.lineOffsets.push_back(i + 1);
        }
    }
}
```

## Find the line for an offset

A binary search finds the last line start at or before an offset.

```cpp
// return the line number that contains an offset
uint32_t lineOf(const TextBuffer& buffer, uint32_t offset) {
    // find the last line start that is not after the offset
    auto it = std::upper_bound(
        buffer.lineOffsets.begin(),
        buffer.lineOffsets.end(),
        offset
    );

    // compute the line number from the search position
    return static_cast<uint32_t>(it - buffer.lineOffsets.begin()) - 1;
}
```

## Convert offset to column

The column is the offset minus the line start.

```cpp
// return the column of an offset within its line
uint32_t columnOf(const TextBuffer& buffer, uint32_t offset) {
    // find the containing line number
    uint32_t line = lineOf(buffer, offset);

    // subtract the line start to get the column
    return offset - buffer.lineOffsets[line];
}
```

## Convert line and column to offset

Diagnostics arrive as line and column and must map back to an offset.

```cpp
// convert a line and column back into an offset
uint32_t offsetOf(
    const TextBuffer& buffer,
    uint32_t line,
    uint32_t column
) {
    // clamp the line into the buffer
    uint32_t clampedLine = std::min(
        line,
        static_cast<uint32_t>(buffer.lineOffsets.size() - 1)
    );

    // start from the line start
    uint32_t offset = buffer.lineOffsets[clampedLine];

    // advance by the requested column, clamped to the line
    uint32_t lineLength = lineLengthOf(buffer, clampedLine);
    return offset + std::min(column, lineLength);
}

// return the length of one line without its newline
uint32_t lineLengthOf(const TextBuffer& buffer, uint32_t line) {
    // compute the start of the next line
    uint32_t next = (line + 1 < buffer.lineOffsets.size())
        ? buffer.lineOffsets[line + 1]
        : static_cast<uint32_t>(buffer.text.size());

    // subtract the line start and drop the newline
    return next - buffer.lineOffsets[line] - 1;
}
```

## Insert a character

Inserting a character places it at the cursor and advances.

```cpp
// insert one character at the cursor
void insertChar(TextBuffer& buffer, char c) {
    // insert the character at the cursor offset
    buffer.text.insert(buffer.text.begin() + buffer.cursor, c);

    // advance the cursor past the character
    buffer.cursor++;

    // rebuild the index when a newline was inserted
    if (c == '\n') {
        rebuildLines(buffer);
    }
}
```

## Insert a newline

A newline splits the current line and the index is rebuilt.

```cpp
// insert a newline at the cursor
void insertNewline(TextBuffer& buffer) {
    // insert the newline at the cursor offset
    buffer.text.insert(buffer.text.begin() + buffer.cursor, '\n');

    // advance the cursor past the newline
    buffer.cursor++;

    // rebuild the line index
    rebuildLines(buffer);
}
```

## Delete the character before the cursor

Deleting removes the character before the cursor.

```cpp
// delete the character before the cursor
void deleteBack(TextBuffer& buffer) {
    // reject deletion at the start of the buffer
    if (buffer.cursor == 0) {
        return;
    }

    // remember whether a newline was removed
    bool removedNewline =
        buffer.text[buffer.cursor - 1] == '\n';

    // remove the character before the cursor
    buffer.text.erase(buffer.cursor - 1, 1);

    // move the cursor back
    buffer.cursor--;

    // rebuild the index when a newline was removed
    if (removedNewline) {
        rebuildLines(buffer);
    }
}
```

## Move the cursor horizontally

Left and right movement are simple offset changes.

```cpp
// move the cursor one character left
void moveLeft(TextBuffer& buffer) {
    // clamp to the start of the buffer
    if (buffer.cursor > 0) {
        buffer.cursor--;
    }
}

// move the cursor one character right
void moveRight(TextBuffer& buffer) {
    // clamp to the end of the buffer
    if (buffer.cursor < buffer.text.size()) {
        buffer.cursor++;
    }
}
```

## Move the cursor vertically

Vertical movement works in line and column space.

```cpp
// move the cursor one line up
void moveUp(TextBuffer& buffer) {
    // find the current position
    uint32_t line = lineOf(buffer, buffer.cursor);
    uint32_t column = columnOf(buffer, buffer.cursor);

    // reject movement from the first line
    if (line == 0) {
        return;
    }

    // clamp the column to the target line
    uint32_t targetColumn = std::min(
        column,
        lineLengthOf(buffer, line - 1)
    );

    // place the cursor on the previous line
    buffer.cursor =
        buffer.lineOffsets[line - 1] + targetColumn;
}

// move the cursor one line down
void moveDown(TextBuffer& buffer) {
    // find the current position
    uint32_t line = lineOf(buffer, buffer.cursor);
    uint32_t column = columnOf(buffer, buffer.cursor);

    // reject movement from the last line
    if (line + 1 >= buffer.lineOffsets.size()) {
        return;
    }

    // clamp the column to the target line
    uint32_t targetColumn = std::min(
        column,
        lineLengthOf(buffer, line + 1)
    );

    // place the cursor on the next line
    buffer.cursor =
        buffer.lineOffsets[line + 1] + targetColumn;
}
```

## Now type it again

Reconstruct the core edit operations.

```cpp
// insert one character at the cursor
void insertChar(TextBuffer& buffer, char c) {
    buffer.text.insert(buffer.text.begin() + buffer.cursor, c);
    buffer.cursor++;
}

// delete the character before the cursor
void deleteBack(TextBuffer& buffer) {
    if (buffer.cursor == 0) {
        return;
    }
    buffer.text.erase(buffer.cursor - 1, 1);
    buffer.cursor--;
}
```

Then reconstruct the coordinate conversions.

```cpp
// return the line number that contains an offset
uint32_t lineOf(const TextBuffer& buffer, uint32_t offset);

// return the column of an offset within its line
uint32_t columnOf(const TextBuffer& buffer, uint32_t offset);

// convert a line and column back into an offset
uint32_t offsetOf(
    const TextBuffer& buffer,
    uint32_t line,
    uint32_t column
);
```

## Wrap up

The flow:

```text
text + line index + cursor
    -> edit operations
    -> offset <-> line/column conversions
    -> diagnostics bridge
```

The buffer is the shared coordinate system for editing and error display.
