# Slang compilation - concepts

The compiler is the middle of the golden loop. It receives shader source text
and produces SPIR-V for Vulkan, or it produces diagnostics explaining why the
source is invalid. This lesson covers the Slang compiler library: how a
global session, a session, a module, an entry point, and a program fit
together, and how SPIR-V and diagnostics come out of that structure.

The compiler does no rendering. Its only job is the translation from source
to a form Vulkan can execute, and from source to the structured error
information the editor can display.

## Slang is a shader language and a compiler

Slang is a high-level shading language, and it ships a compiler library that
the IDE links against. The library can compile Slang source to several
targets. For Vulkan the target is SPIR-V, the binary format Vulkan consumes.

Using the library instead of a command-line tool keeps the IDE self-contained
and fast. The source string moves directly into the compiler in memory, and
the resulting SPIR-V moves directly into Vulkan without any files on disk.

## The session hierarchy

Slang organizes compiler state in a small hierarchy.

The global session holds global compiler state: built-in libraries and
default options. An application typically creates one global session at
startup and keeps it for the whole run.

```cpp
slang::IGlobalSession* globalSession;
slang::createGlobalSession(&globalSession);
```

A session is created from the global session and holds one set of compilation
options. It defines the target format and profile.

```cpp
slang::TargetDesc targetDesc = {};
targetDesc.format = SLANG_SPIRV;

slang::SessionDesc sessionDesc = {};
sessionDesc.targetCount = 1;
sessionDesc.targets = &targetDesc;
```

The session is the workbench. Modules and programs are created inside it.

```text
global session
    |
    +--> session (target = SPIR-V)
             |
             +--> module
             +--> entry point
             +--> program
```

## Modules and entry points

A module is a unit of Slang source. The IDE compiles the edited source into a
module directly from the string.

```cpp
slang::IModule* module = session->loadModuleFromSourceString(
    "shader",
    "shader.slang",
    source,
    &diagnostics
);
```

A module can contain many functions. The IDE needs the function that will be
the shader entry point. Slang marks entry points with attributes, and the
compiler can find them by name.

```cpp
slang::IEntryPoint* entryPoint = nullptr;
module->findEntryPointByName("main", &entryPoint);
```

The entry point is what the rest of the toolchain expands into a full shader.

## Programs and linking

A module alone is not a shader. A shader is a module plus a selected entry
point, combined and linked into a program.

```text
module + entry point
    |
    v
composite component
    |
    v
link
    |
    v
linked program
```

The IDE builds the composite from the module and the entry point, then links
it. Linking resolves types and generates the final code for the target.

```cpp
slang::IComponentType* components[] = { module, entryPoint };
slang::IComponentType* program = nullptr;
session->createCompositeComponentType(components, 2, &program);

slang::IComponentType* linked = program->link();
```

## Getting the SPIR-V

The linked program can produce code for each requested target. The IDE asks
for target zero, which is the SPIR-V target configured on the session.

```cpp
slang::IBlob* code = nullptr;
linked->getTargetCode(0, &code, &compileDiagnostics);
```

The result is a blob: a binary chunk of memory. For SPIR-V the blob contains
the compiled shader as a sequence of 32-bit words, which is exactly what
Vulkan expects when creating a shader module.

```text
source text
    -> module
    -> program
    -> linked program
    -> SPIR-V blob
    -> VkShaderModule
```

## Diagnostics are structured data

The compiler can fail, and the IDE must treat failure as information. Slang
returns diagnostics as text. A typical diagnostic contains the file name, a
line and column, a severity, and a message:

```text
shader.slang(12, 5): error: unknown identifier 'flaot4'
```

The IDE parses this text into a structured form the editor can use:

```cpp
struct Diagnostic {
    int line;
    int column;
    bool isError;
    std::string message;
};
```

The line and column are the coordinates the text buffer already knows how to
display. This is where the buffer lesson pays off: the compiler speaks line
and column, and the buffer converts those to offsets.

## Separate compile diagnostics from code

Slang can return diagnostics even when compilation succeeds. Warnings are
diagnostics too. The IDE must not discard the diagnostics blob on success.

```text
success + warnings  -> SPIR-V + diagnostics
failure + errors    -> no SPIR-V + diagnostics
```

The IDE stores every diagnostic, then decides how to display it based on the
line, column, and severity.

## Object lifetimes

The compiler objects are reference counted. The IDE holds them with
reference-counted pointers so that modules and programs are released
automatically when they are no longer used.

Because a recompile replaces the program, the old program must be released
before the new one is created. The compiler wrapper owns the current program
and swaps it on each compile.

## The compiler wrapper

The IDE wraps the Slang calls in a small Compiler struct. The rest of the
application sees only one function:

```cpp
bool compile(
    const std::string& source,
    std::vector<uint32_t>& spirv,
    std::vector<Diagnostic>& diagnostics
);
```

Everything about Slang sessions, targets, and linking stays inside the
wrapper. This keeps the editor and the renderer free of compiler details.

## Compilation is fast

Shader compilation is fast enough to run on every edit after a short pause.
That is what makes hot reload practical. The compiler keeps no GPU state and
creates no windows, so it can run many times without side effects.

## What this lesson establishes

The compiler turns source into SPIR-V through the session hierarchy, and
turns failure into structured diagnostics. It is the bridge between the text
buffer and the Vulkan pipeline. Later lessons feed the SPIR-V blob to Vulkan
and feed the diagnostics back to the editor.

## Next step

Now type the code version of this lesson.
