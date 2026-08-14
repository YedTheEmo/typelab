# Compute shader preview - concepts

The fragment path renders a full-screen image. The compute path is different:
a compute shader writes directly into a storage image, and the IDE displays
that image. Both paths end in the same preview panel, but they reach it
through different pipelines.

Compute shaders matter because they can do work that fragment shaders cannot:
global memory, arbitrary loops, and multiple outputs. The shaders course in
this track is built around compute kernels, so the IDE must be able to run
them.

## The compute model

A compute shader runs as a grid of threads. Each thread has an index, and it
writes its result into an output image at that index.

```slang
[shader("compute")]
[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    // compute a value from the thread id
    float4 color = ...;

    // write the value into the output image
    outputImage[id.xy] = color;
}
```

The output image is declared as a storage image: an image the shader can read
and write at arbitrary positions. This is the same RWTexture2D pattern the
shaders course uses.

## Two pipelines

The IDE now owns two preview pipelines.

The fragment pipeline renders the compiled fragment shader as the color
attachment of the offscreen target, exactly as in the preview pipeline lesson.

The compute pipeline dispatches the compiled compute shader, which writes into
a storage image. The storage image is then shown in the preview panel.

```text
fragment shader -> graphics pipeline -> target image -> preview
compute shader  -> compute pipeline  -> storage image -> preview
```

Which pipeline is used depends on the selected entry point of the active
shader. The entry point selection from the file lesson chooses the execution
model.

## The storage image

The compute shader writes to an image that must be both a storage resource
and a sampled texture.

```text
format   -> RGBA8 unorm
usage    -> storage + sampled
layout   -> general while the compute shader writes
         -> shader read only while the preview samples it
```

The storage usage lets the compute shader write it. The sampled usage lets
ImGui read it as a texture. The two usages require a layout transition
between the dispatch and the preview draw.

## Dispatch

A dispatch launches the grid of threads. The thread count is derived from the
image size and the shader's workgroup size:

```text
dispatch.x = ceil(width / workgroup.x)
dispatch.y = ceil(height / workgroup.y)
dispatch.z = 1
```

Vulkan dispatches with vkCmdDispatch. No geometry, no render pass, and no
vertex shader are involved. The compute pipeline is bound, the descriptor
sets are bound, and the dispatch records the grid.

## Synchronization

After the dispatch, the storage image is full of new values. The preview
needs those values, but the GPU may still be writing. A pipeline barrier
makes the writes visible before the image is sampled:

```text
dispatch (storage write)
    |
    v
barrier: storage write -> shader read
    |
    v
preview samples the image
```

The barrier also transitions the image layout from general to shader read
only. This is the compute counterpart of the layout transitions the graphics
path performs through its render pass.

## Descriptor sets for compute

The compute pipeline needs its own descriptor layout. It binds the uniform
buffer for time, resolution, and mouse, plus the storage image the shader
writes.

```text
set 0:
    binding 0 -> uniform buffer
    binding 1 -> storage image (output)
```

The storage image binding is what makes the output visible. The shader's
parameter block and the image parameter map to these bindings.

## Hot reload works the same

The compute path reloads exactly like the fragment path. A successful compile
rebuilds the compute pipeline; a failed compile keeps the last good one. The
only difference is which pipeline the reload rebuilds.

```text
entry point == compute  -> rebuild compute pipeline
entry point == fragment -> rebuild graphics pipeline
```

The reload logic from the hot reload lesson is generalized to dispatch on the
shader's execution model.

## Previewing the storage image

The storage image is displayed in the preview panel the same way the offscreen
target is: through a descriptor set that binds the image and a sampler.

```cpp
ImGui::Image(
    reinterpret_cast<ImTextureID>(storageDescriptorSet),
    fitted
);
```

The preview panel code from the layout lesson is reused unchanged. The
compute path simply points the panel at a different texture.

## Resolution coupling

The storage image must match the preview resolution so that each pixel of the
preview corresponds to one thread of the compute shader. When the preview
panel resizes, the storage image is recreated at the new size, exactly like
the offscreen target in the layout lesson.

## What this lesson establishes

The IDE now runs both execution models. Fragment shaders render through the
graphics pipeline, and compute kernels write into a storage image that the
preview shows. Both paths share the uniform buffer, the panel, and the hot
reload machinery. The final lesson assembles everything into one application.

## Next step

Now type the code version of this lesson.
