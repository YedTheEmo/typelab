# Capstone full IDE - typing

This lesson types the assembled application: the App struct that owns every
subsystem, the frame function that runs the golden loop, the reload decision,
and the shutdown sequence.

## Define the application

The App owns the shell and every subsystem.

```cpp
#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>
#include <imgui.h>

#include "../shell/Shell.h"
#include "../editor/Editor.h"
#include "../compiler/Compiler.h"
#include "../renderer/PreviewRenderer.h"
#include "../renderer/ComputeRenderer.h"
#include "../ui/Layout.h"
#include "../files/ShaderManager.h"

// the complete shader IDE
struct App {
    // the window shell
    Shell shell;

    // the text editor
    Editor editor;

    // the Slang compiler
    Compiler compiler;

    // the fragment preview renderer
    PreviewRenderer preview;

    // the compute preview renderer
    ComputeRenderer compute;

    // the shader file manager
    ShaderManager shaders;

    // the hot reload state
    ReloadState reload;
};
```

## Run one frame

The frame runs the golden loop in order.

```cpp
// run one frame of the IDE
void runFrame(App& app) {
    // poll window and input events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        app.shell.handleEvent(event);
    }

    // begin the ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // draw the layout and panels
    drawDockspace();
    drawMenuBar(app.shaders);
    drawShaderTabs(app.shaders);
    drawEditorPanel(app.editor);
    drawDiagnosticsPanel(app.result);

    // run the hot reload check
    runReloadCheck(app);

    // update the shader uniforms
    updateUniforms(
        app.preview.uniformBuffer,
        app.clock,
        app.preview.width,
        app.preview.height
    );

    // render the active execution path
    if (isComputeShader(app.shaders.activeSlot())) {
        renderComputePreview(app.compute, app.preview.uniformSet);
    } else {
        renderFragmentPreview(app.preview, app.preview.uniformSet);
    }

    // finish the ImGui frame
    ImGui::Render();

    // present the rendered frame
    app.shell.presentFrame(app.preview.commandBuffer);
}
```

## Run the reload decision

The reload chooses which pipeline to rebuild.

```cpp
// run the hot reload decision
void runReloadCheck(App& app) {
    // read the current time
    double now = SDL_GetTicks() / 1000.0;

    // read the active shader
    ShaderSlot& slot = app.shaders.activeSlot();

    // skip when nothing changed
    if (!slot.dirty) {
        return;
    }

    // skip until the debounce interval passes
    if ((now - app.reload.stableTime) < app.reload.debounceInterval) {
        return;
    }

    // compile the active source
    std::vector<uint32_t> spirv;
    std::vector<Diagnostic> diagnostics;

    bool ok = app.compiler.compile(
        slot.source,
        slot.entryPoint,
        spirv,
        diagnostics
    );

    // apply the compile result
    applyCompileResult(app.result, diagnostics, ok);

    // handle the result
    if (ok) {
        // rebuild the pipeline for the selected path
        if (isComputeShader(slot)) {
            rebuildComputePipeline(app.compute, spirv);
        } else {
            rebuildFragmentPipeline(app.preview, spirv);
        }

        // the source is now compiled
        slot.dirty = false;

        // autosave the working shader
        autosaveActive(app.shaders);
    } else {
        // keep the last good pipeline
        app.reload.status = "error";
    }
}
```

## Draw the preview

The preview panel shows the current execution path.

```cpp
// draw the preview for the active shader
void drawPreview(App& app) {
    // open the preview window
    ImGui::Begin("Preview");

    // read the active slot
    ShaderSlot& slot = app.shaders.activeSlot();

    // choose the texture for the active path
    ImTextureID texture;
    uint32_t width;
    uint32_t height;

    if (isComputeShader(slot)) {
        // use the storage image
        texture = reinterpret_cast<ImTextureID>(
            app.compute.storageDescriptorSet);
        width = app.compute.width;
        height = app.compute.height;
    } else {
        // use the offscreen target
        texture = reinterpret_cast<ImTextureID>(
            app.preview.targetDescriptorSet);
        width = app.preview.width;
        height = app.preview.height;
    }

    // draw the fitted image
    ImVec2 panel = ImGui::GetContentRegionAvail();
    float scale = std::min(panel.x / width, panel.y / height);
    ImGui::Image(texture, ImVec2(width * scale, height * scale));

    // close the preview window
    ImGui::End();
}
```

## Initialize the application

The subsystems are built in dependency order.

```cpp
// initialize the complete application
bool initApp(App& app) {
    // create the window and Vulkan shell
    if (!app.shell.init("Shader IDE", 1280, 800)) {
        return false;
    }

    // initialize the ImGui backends
    if (!initImGui(app.shell)) {
        return false;
    }

    // initialize the Slang compiler
    if (!initCompiler(app.compiler)) {
        return false;
    }

    // initialize the preview renderer
    if (!initPreviewRenderer(app.preview, app.shell)) {
        return false;
    }

    // initialize the compute renderer
    if (!initComputeRenderer(app.compute, app.shell)) {
        return false;
    }

    // load the default shader
    loadShaderSlot(app.shaders.activeSlot(), "shader.slang");

    return true;
}
```

## Shut down in order

Subsystems are destroyed in reverse order of creation.

```cpp
// shut down the complete application
void shutdownApp(App& app) {
    // wait for the GPU to finish
    vkDeviceWaitIdle(app.shell.device);

    // destroy the compute renderer
    shutdownComputeRenderer(app.compute);

    // destroy the preview renderer
    shutdownPreviewRenderer(app.preview);

    // shut down ImGui
    shutdownImGui();

    // destroy the Vulkan shell
    app.shell.cleanup();
}
```

## Now type it again

Reconstruct the frame loop.

```cpp
// poll window and input events
while (SDL_PollEvent(&event)) {
    app.shell.handleEvent(event);
}

// begin the ImGui frame
ImGui::NewFrame();

// draw the panels
drawEditorPanel(app.editor);
drawPreview(app);

// run the hot reload check
runReloadCheck(app);

// update the shader uniforms
updateUniforms(app.preview.uniformBuffer, app.clock, width, height);

// render the active execution path
if (isComputeShader(slot)) {
    renderComputePreview(app.compute, app.preview.uniformSet);
} else {
    renderFragmentPreview(app.preview, app.preview.uniformSet);
}

// finish and present
ImGui::Render();
app.shell.presentFrame(app.preview.commandBuffer);
```

Then reconstruct the reload decision.

```cpp
// skip until the debounce interval passes
if ((now - app.reload.stableTime) < app.reload.debounceInterval) {
    return;
}

// compile the active source
bool ok = app.compiler.compile(
    slot.source,
    slot.entryPoint,
    spirv,
    diagnostics
);

// handle the result
if (ok) {
    rebuildFragmentPipeline(app.preview, spirv);
    slot.dirty = false;
    autosaveActive(app.shaders);
} else {
    // keep the last good pipeline
    app.reload.status = "error";
}
```

## Wrap up

The complete loop:

```text
edit -> pause -> compile -> SPIR-V -> pipeline -> image -> preview
    -> success -> swap pipeline + autosave
    -> failure -> keep last good + highlight the error line
```

The shader IDE is complete: one window, one loop, both execution models, and
live feedback at the exact line you are editing.
