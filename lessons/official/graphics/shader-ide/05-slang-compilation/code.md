# Slang compilation - typing

This lesson types the compiler wrapper: create the Slang sessions, load the
source into a module, select the entry point, link a program, extract SPIR-V,
and collect diagnostics.

## Represent a diagnostic

Diagnostics are parsed from compiler text into a structured form.

```cpp
#include <string>
#include <vector>
#include <cstdint>

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
```

## Include the Slang header

The compiler API lives in the Slang header.

```cpp
#include <slang.h>
```

## Create the sessions

A global session and a session are created once.

```cpp
#include <slang.h>

// the compiler wrapper
struct Compiler {
    // the process-wide compiler state
    slang::ComPtr<slang::IGlobalSession> globalSession;

    // the compilation session with the SPIR-V target
    slang::ComPtr<slang::ISession> session;
};

// initialize the Slang compiler state
bool initCompiler(Compiler& compiler) {
    // create the process-wide global session
    if (SLANG_FAILED(slang::createGlobalSession(
        compiler.globalSession.writeRef()
    ))) {
        return false;
    }

    // describe the target format
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;

    // describe the compilation session
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    // create the compilation session
    if (SLANG_FAILED(compiler.globalSession->createSession(
        sessionDesc,
        compiler.session.writeRef()
    ))) {
        return false;
    }

    return true;
}
```

## Load the source into a module

The edited source string becomes a module.

```cpp
// load source text into a Slang module
slang::ComPtr<slang::IModule> loadModule(
    Compiler& compiler,
    const std::string& source,
    slang::IBlob** diagnostics
) {
    // compile the source string into a module
    return compiler.session->loadModuleFromSourceString(
        "shader",
        "shader.slang",
        source.c_str(),
        diagnostics
    );
}
```

## Select the entry point

The entry point is found by name within the module.

```cpp
// find the shader entry point by name
bool findEntryPoint(
    slang::IModule* module,
    const std::string& name,
    slang::IEntryPoint** entryPoint
) {
    // look up the named entry point
    if (SLANG_FAILED(module->findEntryPointByName(
        name.c_str(),
        entryPoint
    ))) {
        return false;
    }

    // reject a missing entry point
    if (*entryPoint == nullptr) {
        return false;
    }

    return true;
}
```

## Link the program

The module and entry point are combined and linked.

```cpp
// link the module and entry point into a program
bool linkProgram(
    Compiler& compiler,
    slang::IModule* module,
    slang::IEntryPoint* entryPoint,
    slang::IComponentType** linkedProgram
) {
    // collect the components of the program
    slang::IComponentType* components[] = {
        module,
        entryPoint
    };

    // combine the components
    slang::ComPtr<slang::IComponentType> composite;
    if (SLANG_FAILED(compiler.session->createCompositeComponentType(
        components,
        2,
        composite.writeRef()
    ))) {
        return false;
    }

    // link the composite into an executable program
    *linkedProgram = composite->link();

    // reject a failed link
    if (*linkedProgram == nullptr) {
        return false;
    }

    return true;
}
```

## Extract the SPIR-V

The linked program produces the target code.

```cpp
// extract the compiled SPIR-V from the linked program
bool getSpirv(
    slang::IComponentType* linkedProgram,
    std::vector<uint32_t>& spirv
) {
    // store the code blob and its diagnostics
    slang::ComPtr<slang::IBlob> codeBlob;
    slang::ComPtr<slang::IBlob> codeDiagnostics;

    // request the code for target zero
    slang::Result result = linkedProgram->getTargetCode(
        0,
        codeBlob.writeRef(),
        codeDiagnostics.writeRef()
    );

    // reject compilation failure
    if (SLANG_FAILED(result)) {
        return false;
    }

    // reject an empty code blob
    if (codeBlob == nullptr) {
        return false;
    }

    // copy the SPIR-V words into a vector
    const uint32_t* words =
        static_cast<const uint32_t*>(codeBlob->getBufferPointer());

    // compute the number of 32-bit words
    size_t byteSize = codeBlob->getBufferSize();
    size_t wordCount = byteSize / sizeof(uint32_t);

    // store the words
    spirv.assign(words, words + wordCount);

    return true;
}
```

## Collect diagnostics

Diagnostics text is converted into the structured list.

```cpp
#include <sstream>

// parse a diagnostic blob into structured messages
void collectDiagnostics(
    slang::IBlob* blob,
    std::vector<Diagnostic>& diagnostics
) {
    // return early when there are no diagnostics
    if (blob == nullptr) {
        return;
    }

    // read the diagnostic text
    const char* text =
        static_cast<const char*>(blob->getBufferPointer());

    // split the text into lines
    std::istringstream stream(text);
    std::string line;

    // process each diagnostic line
    while (std::getline(stream, line)) {
        // skip non-diagnostic lines
        if (line.find("error") == std::string::npos) {
            continue;
        }

        // record the whole message
        Diagnostic diagnostic;
        diagnostic.isError = true;
        diagnostic.message = line;

        // store the parsed diagnostic
        diagnostics.push_back(diagnostic);
    }
}
```

## The compile entry point

The wrapper exposes one operation to the application.

```cpp
// compile source text into SPIR-V and diagnostics
bool compileShader(
    Compiler& compiler,
    const std::string& source,
    std::vector<uint32_t>& spirv,
    std::vector<Diagnostic>& diagnostics
) {
    // start with an empty result
    diagnostics.clear();
    spirv.clear();

    // store the module diagnostics
    slang::ComPtr<slang::IBlob> moduleDiagnostics;

    // load the source into a module
    slang::ComPtr<slang::IModule> module = loadModule(
        compiler,
        source,
        moduleDiagnostics.writeRef()
    );

    // collect the module-level diagnostics
    collectDiagnostics(moduleDiagnostics.get(), diagnostics);

    // reject module loading failure
    if (module == nullptr) {
        return false;
    }

    // find the main entry point
    slang::ComPtr<slang::IEntryPoint> entryPoint;
    if (!findEntryPoint(module.get(), "main", entryPoint.writeRef())) {
        // report the missing entry point
        Diagnostic diagnostic;
        diagnostic.isError = true;
        diagnostic.message = "entry point 'main' not found";
        diagnostics.push_back(diagnostic);
        return false;
    }

    // link the program
    slang::ComPtr<slang::IComponentType> linkedProgram;
    if (!linkProgram(
        compiler,
        module.get(),
        entryPoint.get(),
        linkedProgram.writeRef()
    )) {
        return false;
    }

    // extract the SPIR-V
    if (!getSpirv(linkedProgram.get(), spirv)) {
        return false;
    }

    return true;
}
```

## Now type it again

Reconstruct the session creation.

```cpp
// create the process-wide global session
slang::createGlobalSession(compiler.globalSession.writeRef());

// describe the target format
slang::TargetDesc targetDesc = {};
targetDesc.format = SLANG_SPIRV;

// create the compilation session
compiler.globalSession->createSession(
    sessionDesc,
    compiler.session.writeRef()
);
```

Then reconstruct the compile sequence.

```cpp
// load the source into a module
slang::ComPtr<slang::IModule> module =
    session->loadModuleFromSourceString(
        "shader",
        "shader.slang",
        source.c_str(),
        diagnostics
    );

// find the entry point
module->findEntryPointByName("main", entryPoint.writeRef());

// combine the module and entry point
session->createCompositeComponentType(components, 2, &composite);

// link the program
slang::IComponentType* linkedProgram = composite->link();

// extract the SPIR-V
linkedProgram->getTargetCode(0, codeBlob.writeRef(), nullptr);
```

## Wrap up

The flow:

```text
source -> module -> entry point -> program -> link -> SPIR-V
    and failure -> diagnostics
```

The compiler produces either GPU code or structured error information.
