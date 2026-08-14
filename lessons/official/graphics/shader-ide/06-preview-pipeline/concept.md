# Preview pipeline - concepts

The preview renderer turns compiled SPIR-V into an image. It takes the
fragment shader produced by Slang, builds a Vulkan pipeline around it, runs
the pipeline on a full-screen triangle, and writes the result into an
offscreen target that the preview panel displays.

This lesson covers the renderer's core: the offscreen target, the fixed
vertex shader, the pipeline built from the compiled fragment shader, and the
path from the rendered target into an ImGui preview.

## The preview is a texture

The shader's output is not drawn directly onto the swapchain. It is rendered
into a separate offscreen image, and that image is shown as a texture inside
the ImGui preview panel.

```text
compiled fragment shader
    |
    v
full-screen triangle
    |
    v
offscreen target image
    |
    v
ImGui::Image in the preview panel
```

Rendering offscreen keeps the shader output independent from the window
layout. The preview can be any size, and the editor can keep running while
the preview shows the latest successful render.

## The offscreen target

The offscreen target is a Vulkan image with these properties:

```text
format     -> RGBA8 unorm (color)
usage      -> color attachment + sampled image
layout     -> color attachment while rendering
           -> shader read only while ImGui samples it
```

The image must be sampleable because ImGui reads it as a texture. It must be
a color attachment because the shader writes it as the render target.

The target is recreated when the preview size changes. This is the same
pattern as swapchain recreation, but scoped to one image.

## The full-screen triangle

The compiled shader is a fragment shader. A fragment shader runs per pixel
on geometry, so the IDE supplies a trivial vertex shader and draws three
vertices that cover the entire target.

The classic full-screen triangle uses clip-space positions:

```text
(-1, -1)   (3, -1)   (-1, 3)
```

Any pixel inside the target falls inside this triangle, so every pixel runs
the fragment shader exactly once. No index buffer or vertex buffer is needed;
the positions are emitted by the vertex shader directly.

The IDE's vertex shader is fixed and never changes:

```slang
[shader("vertex")]
float2 main(uint vertexIndex : SV_VertexID)
{
    return float2(
        (float(vertexIndex) * 2.0 - 1.0) % 3.0,
        ...
    );
}
```

A simpler formulation is a small static table of the three positions. The
point is that the vertex shader is supplied by the IDE, not by the user's
shader.

## Shader modules from SPIR-V

Vulkan stores compiled shaders as shader modules. A module is created from a
SPIR-V blob, which is exactly what the Slang compiler produced.

```cpp
VkShaderModuleCreateInfo moduleInfo{
    VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    nullptr,
    0,
    spirv.size() * sizeof(uint32_t),
    spirv.data()
};

vkCreateShaderModule(device, &moduleInfo, nullptr, &module);
```

The renderer creates one module from the IDE's vertex shader and one from the
compiled fragment shader. Every recompile replaces the fragment module.

## The pipeline

The pipeline combines both shader stages with fixed-state configuration:

```text
vertex shader        -> IDE supplied
fragment shader      -> compiled by Slang
topology             -> triangle list
attachments          -> one color attachment (the offscreen target)
depth test           -> disabled
descriptor sets      -> pipeline layout
```

The pipeline is built once per compiled program. When the user edits the
shader and it recompiles, the renderer destroys the old pipeline and builds a
new one from the fresh fragment module.

```text
pipeline
    |
    +--> vertex module (fixed)
    +--> fragment module (from Slang)
    +--> pipeline layout (descriptors)
    +--> render pass (target format)
```

## The pipeline layout and descriptors

The shader expects uniforms, which arrive through descriptor sets. The
pipeline layout declares what the shader can bind. For the preview the layout
has one descriptor set containing the uniform buffer that will hold time,
resolution, and mouse state in the next lesson.

```text
set 0:
    binding 0 -> uniform buffer (per-frame values)
```

The renderer creates the descriptor set layout, the pipeline layout, and the
descriptor set. The uniform buffer data is filled in a later lesson; here the
plumbing exists so the pipeline is complete.

## Rendering into the target

Recording a preview frame has a fixed shape.

```text
transition target to color attachment
    |
    v
begin render pass on the target
    |
    v
bind pipeline
    |
    v
bind descriptor set
    |
    v
draw 3 vertices
    |
    v
end render pass
    |
    v
transition target to shader read only
```

The transitions matter because Vulkan requires explicit image layout changes.
The target is in one layout while it is being rendered and another while
ImGui samples it. Each layout has its own transition with synchronization.

## Passing the target to ImGui

ImGui draws images through its Vulkan backend using a descriptor set that
binds the image and a sampler. The renderer creates such a descriptor set for
the preview target and hands its handle to the UI as an ImTextureID.

```cpp
ImGui::Image(
    reinterpret_cast<ImTextureID>(descriptorSet),
    ImVec2(previewWidth, previewHeight)
);
```

When the UI calls ImGui::Image, the recorded ImGui commands sample the target
image. The shader's output is now visible inside the preview panel.

## The target size

The offscreen target size is derived from the preview panel size. A panel
that is 512 by 384 pixels gets a target of the same size. The shader receives
that resolution through its uniforms so it can compute normalized
coordinates.

When the panel is resized, the target is recreated. The preview then
continues with the new resolution on the next frame.

## Failure does not destroy the renderer

The renderer owns the pipeline and the target. A failed compile produces no
new fragment module, so the renderer keeps the old pipeline and continues to
show the last successful image. This is the same keep-last-good behavior the
compiler established, now implemented in the renderer.

```text
recompile
    |
    +--> success -> replace pipeline
    |
    +--> failure -> keep old pipeline
```

## What this lesson establishes

The preview renderer now renders the compiled shader into an offscreen target
and shows that target in the ImGui preview panel. The remaining pieces are
per-frame input through the uniforms, automatic recompilation, and the
panels that organize the interface.

## Next step

Now type the code version of this lesson.
