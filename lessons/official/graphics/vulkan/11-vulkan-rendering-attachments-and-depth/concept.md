# Vulkan rendering attachments and depth - concepts

Rendering does not only produce a color image. A rasterized scene also needs
to decide which fragments are visible and, in more advanced renderers, may
produce additional attachment data.

The central idea of this lesson is that a rendering operation works with
attachments: images that receive the outputs of rasterization.

```text
fragment
   |
   +--> color attachment
   |
   +--> depth attachment
```

## Color attachments

A color attachment is an image used as a rendering destination.

```cpp
VkAttachmentDescription colorAttachment{};
colorAttachment.format = swapchainFormat;
colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
```

The format must match the image that will receive the rendered color. In a
swapchain renderer, that image is normally a swapchain image view.

The attachment is therefore another use of the image system introduced in the
previous lesson.

## Depth attachments

Depth answers a different question from color. It stores a value representing
how far a fragment is from the camera so that fragments can be rejected when
they are behind something already rendered.

```cpp
VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
```

A depth image needs a depth-capable format and must be created with depth
attachment usage.

```cpp
VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
```

The depth image is not normally presented to the screen. It exists to support
visibility testing during rendering.

## Depth testing

The depth test compares an incoming fragment's depth with the value already
stored in the depth attachment.

```cpp
depthStencil.depthTestEnable = VK_TRUE;
depthStencil.depthWriteEnable = VK_TRUE;
```

Enabling the test allows fragments to be rejected. Enabling depth writes lets
passing fragments update the depth buffer.

This gives the common opaque rendering behavior:

```text
closer fragment -> passes -> writes depth
farther fragment -> fails -> discarded
```

## Depth comparison

The comparison operation defines what counts as passing.

```cpp
depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
```

With the common LESS operation, a fragment passes when its depth is smaller
than the stored value.

The initial depth value therefore matters. A cleared depth image is commonly
initialized to the far end of the depth range.

```cpp
vkCmdClearDepthStencilImage(
    commandBuffer,
    depthImage,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    &clearValue,
    1,
    &range
);
```

The exact clear and transition sequence depends on the rendering architecture.

## Attachment descriptions

An attachment description records how an attachment is used during rendering.

```cpp
VkAttachmentDescription colorAttachment{
    0,
    swapchainFormat,
    VK_SAMPLE_COUNT_1_BIT,
    VK_ATTACHMENT_LOAD_OP_CLEAR,
    VK_ATTACHMENT_STORE_OP_STORE,
    VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    VK_ATTACHMENT_STORE_OP_DONT_CARE,
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
};
```

The load operation says what happens at the beginning of rendering. The store
operation says what happens when rendering finishes.

For a presented color image, CLEAR plus STORE is a common choice.

## Depth attachment description

Depth uses the same attachment model but has depth-specific final layout.

```cpp
VkAttachmentDescription depthAttachment{
    0,
    depthFormat,
    VK_SAMPLE_COUNT_1_BIT,
    VK_ATTACHMENT_LOAD_OP_CLEAR,
    VK_ATTACHMENT_STORE_OP_DONT_CARE,
    VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    VK_ATTACHMENT_STORE_OP_DONT_CARE,
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
};
```

The depth result is often not needed after the render, so its store operation
can be DONT_CARE.

## Load operations

A load operation controls the beginning of attachment use.

```cpp
VK_ATTACHMENT_LOAD_OP_CLEAR
```

means rendering begins from a supplied clear value.

```cpp
VK_ATTACHMENT_LOAD_OP_LOAD
```

preserves the previous contents.

```cpp
VK_ATTACHMENT_LOAD_OP_DONT_CARE
```

means the previous contents are irrelevant.

The choice should match the actual rendering dependency. Clearing is often
preferable when old contents are not needed because it makes the intended
frame contents explicit.

## Store operations

A store operation controls whether the final attachment contents must remain
available after rendering.

```cpp
VK_ATTACHMENT_STORE_OP_STORE
```

keeps the result.

```cpp
VK_ATTACHMENT_STORE_OP_DONT_CARE
```

allows the implementation to discard it.

A swapchain color image normally needs STORE because it will be presented.
A depth buffer often does not if the next operation does not read its previous
contents.

## Attachment references

An attachment description defines an attachment, but a rendering operation
also needs to identify which attachment a pipeline uses.

```cpp
VkAttachmentReference colorReference{
    0,
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
};
```

The index selects the corresponding attachment description.

A depth reference works the same way:

```cpp
VkAttachmentReference depthReference{
    1,
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
};
```

This creates a relationship between the attachment declarations and the
actual rendering destinations.

## Framebuffer thinking

In the older render-pass model, a framebuffer combines concrete image views
with a compatible render pass.

```text
render pass
    |
    +--> attachment 0 description
    +--> attachment 1 description
              |
              v
          framebuffer
              |
              +--> swapchain view
              +--> depth view
```

The framebuffer says which actual images fill the attachment slots.

This is why each swapchain image commonly has a corresponding framebuffer when
using traditional render-pass rendering.

## Multiple attachments

A rendering operation can have several color attachments.

```text
location 0 -> final color
location 1 -> normal data
location 2 -> material data
```

A fragment shader can write multiple outputs:

```glsl
layout(location = 0) out vec4 color;
layout(location = 1) out vec4 normal;
```

This is the basis of techniques such as deferred rendering, where one geometry
pass produces several screen-sized data buffers.

## Depth and perspective

Depth is generated from transformed geometry. The vertex shader produces a
position whose depth eventually participates in rasterization.

```text
model
  -> view
  -> projection
  -> clip position
  -> depth
  -> depth test
```

The depth attachment therefore belongs to the same rendering pipeline as the
vertex and fragment stages. It is not a separate CPU-side visibility system.

## Depth image views

A depth image also needs an image view.

```cpp
VkImageViewCreateInfo depthViewInfo{
    VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    nullptr,
    0,
    depthImage,
    VK_IMAGE_VIEW_TYPE_2D,
    depthFormat,
    {},
    {
        VK_IMAGE_ASPECT_DEPTH_BIT,
        0,
        1,
        0,
        1
    }
};
```

The important difference from a color view is the aspect:

```cpp
VK_IMAGE_ASPECT_DEPTH_BIT
```

If a format contains both depth and stencil components, the relevant aspects
must be handled appropriately.

## Depth image layout

A depth attachment needs a layout appropriate for attachment use.

```cpp
VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
```

The image must be transitioned into the layout before the rendering operation
uses it.

This is the same general resource rule seen with sampled textures: an image
must be in a layout compatible with its current use.

## Depth clear values

A render operation can clear color and depth independently.

```cpp
VkClearValue clearValues[2]{};
clearValues[0].color = {{0.05f, 0.05f, 0.08f, 1.0f}};
clearValues[1].depthStencil = {1.0f, 0};
```

The first value belongs to the color attachment and the second belongs to the
depth attachment.

The clear value must match the attachment's expected data.

## Depth state and culling

Depth testing is not the only visibility mechanism in a graphics pipeline.
Face culling can remove back-facing triangles before fragment processing.

```cpp
rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
```

Culling and depth testing solve different problems.

```text
culling -> remove triangles by orientation
depth   -> remove fragments by distance
```

A typical opaque renderer uses both.

## Depth attachment lifecycle

A depth image usually survives across many frames.

```text
create depth image
       |
       v
allocate memory
       |
       v
create depth view
       |
       v
use every frame
       |
       v
destroy after GPU completion
```

Unlike a swapchain image, it is normally owned directly by the renderer.

This makes the depth image a persistent piece of renderer state.

## What this lesson establishes

Attachments are images used as outputs of rendering. Color attachments hold
rendered color, while depth attachments provide per-fragment visibility.
Attachment descriptions specify loading, storing, formats, samples, and layouts,
while references connect those descriptions to the rendering operation.

Depth testing then turns the depth attachment into a visibility mechanism,
allowing closer fragments to replace farther ones.

The next lesson moves from attachment concepts to the mechanisms that organize
rendering operations: traditional render passes and modern dynamic rendering.

## Next step

Now type the code version of this lesson.
