# Shader hot reload - typing

This lesson types the reload loop: mark edits as dirty, debounce the compile,
rebuild the pipeline on success, keep the old pipeline on failure, and report
the status.

## Mark the buffer as dirty

Every edit flags the source as changed.

```cpp
#include <string>
#include <vector>
#include <cstdint>

// a text buffer that tracks whether it changed
struct TextBuffer {
    // the whole source text
    std::string text;

    // true when the text changed since the last compile
    bool dirty = false;
};

// insert one character and mark the buffer dirty
void insertChar(TextBuffer& buffer, char c) {
    // insert the character at the end
    buffer.text.push_back(c);

    // remember that the source changed
    buffer.dirty = true;
}
```

## Track the reload state

The reload loop needs time and status state.

```cpp
// the hot reload state
struct ReloadState {
    // the time when the source last changed
    double stableTime = 0.0;

    // the pause before compiling, in seconds
    double debounceInterval = 0.25;

    // the current status message
    std::string status = "ready";

    // true while a compile has not yet run
    bool pending = false;
};
```

## Check whether the source is stable

The loop waits for a pause before compiling.

```cpp
#include <SDL3/SDL.h>

// return true when the source has been stable long enough
bool isStable(ReloadState& reload, double now) {
    // compile only after the debounce interval passes
    return (now - reload.stableTime) >= reload.debounceInterval;
}
```

## Recompile and swap the pipeline

The transaction builds the new pipeline before destroying the old one.

```cpp
#include <vulkan/vulkan.h>
#include <vector>

// the preview renderer that owns the pipeline
struct PreviewRenderer {
    // the Vulkan device
    VkDevice device = VK_NULL_HANDLE;

    // the current fragment module
    VkShaderModule fragmentModule = VK_NULL_HANDLE;

    // the current preview pipeline
    VkPipeline pipeline = VK_NULL_HANDLE;

    // the graphics queue
    VkQueue queue = VK_NULL_HANDLE;
};

// wait until the GPU finishes using current objects
void waitForGpu(PreviewRenderer& renderer) {
    // block until the device is idle
    vkDeviceWaitIdle(renderer.device);
}

// replace the fragment module and rebuild the pipeline
bool rebuildPipeline(
    PreviewRenderer& renderer,
    VkPipelineLayout pipelineLayout,
    VkRenderPass renderPass,
    VkShaderModule vertexModule,
    const std::vector<uint32_t>& spirv
) {
    // wait until the old objects are free
    waitForGpu(renderer);

    // destroy the old fragment module
    if (renderer.fragmentModule != VK_NULL_HANDLE) {
        vkDestroyShaderModule(
            renderer.device,
            renderer.fragmentModule,
            nullptr
        );
    }

    // describe the new fragment module
    VkShaderModuleCreateInfo moduleInfo{
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        nullptr,
        0,
        spirv.size() * sizeof(uint32_t),
        spirv.data()
    };

    // create the new fragment module
    if (vkCreateShaderModule(renderer.device, &moduleInfo, nullptr,
        &renderer.fragmentModule) != VK_SUCCESS) {
        return false;
    }

    // describe the vertex shader stage
    VkPipelineShaderStageCreateInfo vertexStage{
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        VK_SHADER_STAGE_VERTEX_BIT,
        vertexModule,
        "main",
        nullptr
    };

    // describe the fragment shader stage
    VkPipelineShaderStageCreateInfo fragmentStage{
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        renderer.fragmentModule,
        "main",
        nullptr
    };

    // collect the stages
    VkPipelineShaderStageCreateInfo stages[] = {
        vertexStage,
        fragmentStage
    };

    // disable the vertex input
    VkPipelineVertexInputStateCreateInfo vertexInput{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        nullptr,
        0,
        0,
        nullptr,
        0,
        nullptr
    };

    // draw triangle lists
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_FALSE
    };

    // describe the viewport
    VkViewport viewport{
        0.0f,
        0.0f,
        512.0f,
        512.0f,
        0.0f,
        1.0f
    };

    // describe the scissor
    VkRect2D scissor{
        VkOffset2D{ 0, 0 },
        VkExtent2D{ 512, 512 }
    };

    // describe the viewport state
    VkPipelineViewportStateCreateInfo viewportState{
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        nullptr,
        0,
        1,
        &viewport,
        1,
        &scissor
    };

    // disable the rasterizer overdraw
    VkPipelineRasterizationStateCreateInfo rasterizer{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_FALSE,
        VK_FALSE,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_NONE,
        VK_FRONT_FACE_COUNTER_CLOCKWISE,
        VK_FALSE,
        0.0f,
        0.0f,
        0.0f,
        1.0f
    };

    // disable multisampling
    VkPipelineMultisampleStateCreateInfo multisample{
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FALSE,
        0.0f,
        nullptr,
        VK_FALSE,
        VK_FALSE
    };

    // describe the color blend attachment
    VkPipelineColorBlendAttachmentState blendAttachment{
        VK_FALSE,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_OP_ADD,
        VK_COLOR_COMPONENT_R_BIT
            | VK_COLOR_COMPONENT_G_BIT
            | VK_COLOR_COMPONENT_B_BIT
            | VK_COLOR_COMPONENT_A_BIT
    };

    // describe the blend state
    VkPipelineColorBlendStateCreateInfo blendState{
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_FALSE,
        VK_LOGIC_OP_COPY,
        1,
        &blendAttachment,
        { 0.0f, 0.0f, 0.0f, 0.0f }
    };

    // describe the graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        nullptr,
        0,
        2,
        stages,
        &vertexInput,
        &inputAssembly,
        nullptr,
        &viewportState,
        &rasterizer,
        &multisample,
        nullptr,
        &blendState,
        nullptr,
        pipelineLayout,
        renderPass,
        0,
        VK_NULL_HANDLE,
        -1
    };

    // store the old pipeline for destruction
    VkPipeline oldPipeline = renderer.pipeline;

    // create the new pipeline
    if (vkCreateGraphicsPipelines(renderer.device, VK_NULL_HANDLE, 1,
        &pipelineInfo, nullptr, &renderer.pipeline) != VK_SUCCESS) {
        return false;
    }

    // destroy the old pipeline
    if (oldPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(renderer.device, oldPipeline, nullptr);
    }

    return true;
}
```

## Run the reload check

The loop decides when to compile.

```cpp
#include "../compiler/Compiler.h"
#include "../buffer/TextBuffer.h"

// run the hot reload check for one frame
void runReloadCheck(
    PreviewRenderer& renderer,
    TextBuffer& buffer,
    Compiler& compiler,
    ReloadState& reload
) {
    // read the current time
    double now = SDL_GetTicks() / 1000.0;

    // skip when the source has not changed
    if (!buffer.dirty) {
        return;
    }

    // skip until the debounce interval passes
    if (!isStable(reload, now)) {
        return;
    }

    // report the compile start
    reload.status = "compiling";

    // store the compile result
    std::vector<uint32_t> spirv;
    std::vector<Diagnostic> diagnostics;

    // compile the current source
    bool ok = compiler.compile(
        buffer.text,
        spirv,
        diagnostics
    );

    // handle the compile result
    if (ok) {
        // rebuild the pipeline with the new SPIR-V
        if (rebuildPipeline(renderer, ...)) {
            // clear the dirty flag after success
            buffer.dirty = false;

            // report the success
            reload.status = "ok";
        }
    } else {
        // keep the old pipeline and report the failure
        reload.status = "error";
    }
}
```

## Track the stable time

Each edit restarts the debounce timer.

```cpp
// record that the source changed at this moment
void markChanged(ReloadState& reload) {
    // restart the stability timer
    reload.stableTime = SDL_GetTicks() / 1000.0;

    // mark the compile as pending
    reload.pending = true;
}
```

## Now type it again

Reconstruct the reload trigger.

```cpp
// skip when the source has not changed
if (!buffer.dirty) {
    return;
}

// skip until the debounce interval passes
if (!isStable(reload, now)) {
    return;
}

// compile the current source
bool ok = compiler.compile(buffer.text, spirv, diagnostics);

// handle the compile result
if (ok) {
    rebuildPipeline(renderer, pipelineLayout, renderPass, vertexModule, spirv);
    buffer.dirty = false;
} else {
    // keep the old pipeline
    reload.status = "error";
}
```

Then reconstruct the keep-last-good rule.

```text
compile succeeds -> swap pipeline, clear dirty
compile fails    -> keep old pipeline, show error
```

## Wrap up

The flow:

```text
edit -> dirty -> debounce -> compile
    -> success -> swap pipeline -> preview updates
    -> failure -> keep old pipeline -> preview keeps running
```

The preview now reloads automatically while staying alive through errors.
