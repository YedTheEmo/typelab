# Shader IDE overview - typing

This lesson types the application skeleton: the module boundaries, the shader
program data, and the golden loop that edits, compiles, renders, and previews.

## Describe the shader program

The central piece of data is the shader program being edited.

```cpp
#include <string>
#include <vector>
#include <cstdint>

// describe a shader program under edit
struct ShaderProgram {
    // the source text being edited
    std::string source;

    // the file name shown to the user
    std::string name;

    // the entry point selected for compilation
    std::string entryPoint;

    // the compiled SPIR-V from the last success
    std::vector<uint32_t> spirv;

    // true when the last compile succeeded
    bool compiled = false;
};
```

The program stores both the source and the last compiled result. This lets
the preview keep working after a failed edit.

## Declare the subsystems

Each subsystem is a small struct with one responsibility.

```cpp
// the text editor state
struct Editor {
    // the shader source buffer
    ShaderProgram program;
};

// the Slang compiler wrapper
struct Compiler {
    // compile source into SPIR-V
    bool compile(ShaderProgram& program);
};

// the Vulkan preview renderer
struct PreviewRenderer {
    // render the current program to the preview
    void render(const ShaderProgram& program);
};
```

The shell connects these pieces and owns the window.

```cpp
// the application shell
struct App {
    // the text editing subsystem
    Editor editor;

    // the shader compilation subsystem
    Compiler compiler;

    // the Vulkan preview subsystem
    PreviewRenderer renderer;
};
```

## The golden loop

The frame loop calls each subsystem in a fixed order.

```cpp
// run one frame of the application
void runFrame(App& app) {
    // handle window and input events
    pollEvents();

    // draw the editor user interface
    drawEditor(app.editor);

    // check whether the source changed
    if (sourceChanged(app.editor)) {
        // compile the edited source
        app.compiler.compile(app.editor.program);
    }

    // render the current program
    app.renderer.render(app.editor.program);

    // present the rendered preview
    present();
}
```

## Handle compile failure

A failed compile is a data result, not a crash.

```cpp
// run one complete edit cycle
void processEdit(Editor& editor, Compiler& compiler) {
    // ask the compiler to produce SPIR-V
    bool ok = compiler.compile(editor.program);

    // keep the last good shader when compilation failed
    if (!ok) {
        return;
    }
}
```

The program object keeps its previous SPIR-V on failure, so the preview
continues to show the last working image.

## Map the data flow

The whole application moves data along one path.

```cpp
// type the essential flow of the application
// edit source
//      |
//      v
// compile with Slang
//      |
//      v
// render with Vulkan
//      |
//      v
// show preview
```

## Now type it again

Reconstruct the subsystem boundaries from memory.

```cpp
// the text editing subsystem
struct Editor {
    ShaderProgram program;
};

// the shader compilation subsystem
struct Compiler {
    bool compile(ShaderProgram& program);
};

// the Vulkan preview subsystem
struct PreviewRenderer {
    void render(const ShaderProgram& program);
};
```

Then reconstruct the golden loop.

```cpp
// run one frame of the application
void runFrame(App& app) {
    // handle window and input events
    pollEvents();

    // draw the editor user interface
    drawEditor(app.editor);

    // check whether the source changed
    if (sourceChanged(app.editor)) {
        // compile the edited source
        app.compiler.compile(app.editor.program);
    }

    // render the current program
    app.renderer.render(app.editor.program);

    // present the rendered preview
    present();
}
```

## Wrap up

The flow:

```text
source -> compile -> SPIR-V -> pipeline -> image -> preview
```

The three subsystems have separate jobs, and the shell owns the loop.
