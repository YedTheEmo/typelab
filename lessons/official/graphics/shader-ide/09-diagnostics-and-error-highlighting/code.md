# Diagnostics and error highlighting - typing

This lesson types the diagnostics system: parse compiler text into structured
messages, collect error lines, draw the diagnostics panel, highlight error
lines in the editor, and jump to a location when a diagnostic is clicked.

## Define the diagnostic struct

The parsed form of a compiler message.

```cpp
#include <string>
#include <vector>

// a single compile message
struct Diagnostic {
    // the line the message refers to
    int line = 0;

    // the column the message refers to
    int column = 0;

    // true when the message is an error
    bool isError = false;

    // the human-readable message
    std::string message;
};

// the full result of one compile
struct CompileResult {
    // all messages from the compile
    std::vector<Diagnostic> diagnostics;

    // the error line numbers, for highlighting
    std::vector<int> errorLines;

    // true when the compile produced valid SPIR-V
    bool ok = false;
};
```

## Parse a diagnostic line

Compiler text is turned into a structured message.

```cpp
#include <sstream>

// parse one diagnostic line
Diagnostic parseDiagnosticLine(const std::string& line) {
    // start with an empty diagnostic
    Diagnostic diagnostic;
    diagnostic.message = line;

    // find the location parenthesis
    size_t open = line.find('(');

    // return early when no location is present
    if (open == std::string::npos) {
        return diagnostic;
    }

    // find the comma after the line number
    size_t comma = line.find(',', open);
    size_t close = line.find(')', comma);

    // return early when the location is malformed
    if (comma == std::string::npos || close == std::string::npos) {
        return diagnostic;
    }

    // parse the line number
    diagnostic.line = std::stoi(line.substr(open + 1, comma - open - 1));

    // parse the column number
    diagnostic.column = std::stoi(
        line.substr(comma + 1, close - comma - 1)
    );

    // detect the severity keyword
    diagnostic.isError =
        line.find("error") != std::string::npos;

    return diagnostic;
}
```

## Collect the error lines

The editor needs a fast set of lines to highlight.

```cpp
// build the error line list from the diagnostics
void collectErrorLines(
    const std::vector<Diagnostic>& diagnostics,
    std::vector<int>& errorLines
) {
    // start with an empty list
    errorLines.clear();

    // inspect every diagnostic
    for (const Diagnostic& diagnostic : diagnostics) {
        // keep only the errors
        if (!diagnostic.isError) {
            continue;
        }

        // add the error line
        errorLines.push_back(diagnostic.line);
    }
}
```

## Draw the diagnostics panel

The panel lists every message and reports clicks.

```cpp
#include <imgui.h>

// draw the diagnostics panel
void drawDiagnosticsPanel(
    const CompileResult& result,
    int& selectedLine
) {
    // open the diagnostics window
    ImGui::Begin("Diagnostics");

    // show the compile status summary
    if (result.ok) {
        ImGui::Text("compile ok");
    } else {
        ImGui::Text("compile failed");
    }

    // list each diagnostic
    for (const Diagnostic& diagnostic : result.diagnostics) {
        // format the location prefix
        std::string label =
            std::to_string(diagnostic.line)
            + ":"
            + std::to_string(diagnostic.column)
            + "  "
            + diagnostic.message;

        // color errors differently
        if (diagnostic.isError) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s",
                label.c_str());
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.4f, 1.0f), "%s",
                label.c_str());
        }

        // detect a click on this diagnostic
        if (ImGui::IsItemClicked()) {
            // jump to the diagnostic location
            selectedLine = diagnostic.line;
        }
    }

    // close the diagnostics window
    ImGui::End();
}
```

## Draw the editor with highlighting

The editor paints each line, highlighting error lines.

```cpp
#include "../buffer/TextBuffer.h"

// draw the editor panel with error highlighting
void drawEditorPanel(
    TextBuffer& buffer,
    const std::vector<int>& errorLines
) {
    // open the editor window
    ImGui::Begin("Editor");

    // start at the first line
    int lineIndex = 1;

    // iterate over the buffered lines
    for (const std::string& line : buffer.lines) {
        // check whether this line has an error
        bool isError = contains(errorLines, lineIndex);

        // draw the error highlight behind the line
        if (isError) {
            ImGui::TextColored(
                ImVec4(1.0f, 0.6f, 0.6f, 1.0f),
                "%s",
                line.c_str()
            );
        } else {
            ImGui::Text("%s", line.c_str());
        }

        // advance to the next line
        lineIndex++;
    }

    // close the editor window
    ImGui::End();
}

// return true when a list contains a value
bool contains(const std::vector<int>& list, int value) {
    // scan the list for the value
    for (int item : list) {
        if (item == value) {
            return true;
        }
    }

    return false;
}
```

## Jump the cursor to a diagnostic

The buffer converts line and column back to an offset.

```cpp
// move the editor cursor to a diagnostic location
void jumpToDiagnostic(
    TextBuffer& buffer,
    const Diagnostic& diagnostic
) {
    // convert the location into a buffer offset
    buffer.cursor = offsetOf(
        buffer,
        diagnostic.line,
        diagnostic.column
    );
}
```

## Apply a new compile result

The editor highlights reflect the newest diagnostics.

```cpp
// apply a compile result to the interface state
void applyCompileResult(
    CompileResult& result,
    const std::vector<Diagnostic>& diagnostics,
    bool ok
) {
    // store the compile status
    result.ok = ok;

    // store the raw diagnostics
    result.diagnostics = diagnostics;

    // rebuild the error line list
    collectErrorLines(diagnostics, result.errorLines);
}
```

## Now type it again

Reconstruct the diagnostic parse.

```cpp
// find the location parenthesis
size_t open = line.find('(');

// find the comma after the line number
size_t comma = line.find(',', open);

// find the closing parenthesis
size_t close = line.find(')', comma);

// parse the line and column
diagnostic.line = std::stoi(line.substr(open + 1, comma - open - 1));
diagnostic.column = std::stoi(line.substr(comma + 1, close - comma - 1));

// detect the severity keyword
diagnostic.isError = line.find("error") != std::string::npos;
```

Then reconstruct the editor highlight loop.

```cpp
// iterate over the buffered lines
for (const std::string& line : buffer.lines) {
    // check whether this line has an error
    bool isError = contains(errorLines, lineIndex);

    // draw the error highlight behind the line
    if (isError) {
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.6f, 1.0f), "%s",
            line.c_str());
    } else {
        ImGui::Text("%s", line.c_str());
    }

    lineIndex++;
}
```

## Wrap up

The flow:

```text
compiler text -> Diagnostic -> diagnostics panel + error lines
    -> error line highlight
    -> click -> offsetOf -> cursor jump
```

Diagnostics now reach the exact line in the editor.
