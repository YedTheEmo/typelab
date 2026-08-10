# Vulkan render passes and dynamic rendering - concepts

Vulkan has two major ways to describe where graphics commands render:
traditional render passes and dynamic rendering.

Both express the same fundamental idea: before drawing, the GPU needs to know
which images are being used as attachments and how those images participate in
the rendering operation.

```text
rendering operation
      |
      +--> color attachment
      |
      +--> depth attachment
```

## The traditional render-pass model

A traditional render pass describes attachment behavior before the render pass
is created.

```cpp
VkRenderPassCreateInfo renderPassInfo{
    VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    nullptr,
    0,
    attachmentCount,
    attachments,
    subpassCount,
    subpasses,
    dependencyCount,
    dependencies
};
```

The render pass contains the abstract description of the rendering operation.
It does not itself identify the concrete swapchain image used for a particular
frame.

A framebuffer supplies those concrete image views.

## Subpasses

A traditional render pass can contain multiple subpasses.

```text
render pass
    |
    +-- subpass 0
    +-- subpass 1
    +-- subpass 2
```

A subpass describes which attachments it reads and writes.

This was designed to let Vulkan implementations understand relationships
between several rendering stages, potentially keeping data in efficient
attachment storage.

For a simple renderer, one graphics subpass is usually enough.

## Dependencies

Traditional render passes can also describe dependencies between subpasses
and external operations.

```cpp
VkSubpassDependency dependency{
    VK_SUBPASS_EXTERNAL,
    0,
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    0,
    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    0
};
```

The dependency establishes ordering and access relationships.

Modern synchronization2 provides a more explicit general synchronization
model, but understanding render-pass dependencies remains important when
reading existing Vulkan applications and APIs.

## Framebuffers

The traditional model connects an abstract render pass to concrete image views
through a framebuffer.

```cpp
VkFramebufferCreateInfo framebufferInfo{
    VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    nullptr,
    0,
    renderPass,
    attachmentCount,
    attachments,
    width,
    height,
    1
};
```

The attachment order must match the render pass's attachment descriptions.

The render pass says what the slots mean. The framebuffer says which images
fill those slots.

## Beginning a render pass

A traditional render pass is begun with a render-pass begin structure.

```cpp
VkRenderPassBeginInfo beginInfo{
    VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    nullptr,
    renderPass,
    framebuffer,
    renderArea,
    clearValueCount,
    clearValues
};
```

The command starts the render pass:

```cpp
vkCmdBeginRenderPass(
    commandBuffer,
    &beginInfo,
    VK_SUBPASS_CONTENTS_INLINE
);
```

Draw commands then execute inside the active subpass.

## Ending a render pass

The render pass ends explicitly.

```cpp
vkCmdEndRenderPass(commandBuffer);
```

The ending operation is significant because the render pass defines how its
attachments transition toward their final layouts and what data is preserved.

The entire operation is therefore bounded:

```text
begin render pass
    |
    +--> draw
    +--> draw
    +--> draw
    |
end render pass
```

## Why dynamic rendering exists

Dynamic rendering removes much of the static render-pass and framebuffer
boilerplate.

Instead of creating a render pass and framebuffer for every attachment
configuration, the command buffer describes the active attachments directly.

```cpp
VkRenderingInfo renderingInfo{
    VK_STRUCTURE_TYPE_RENDERING_INFO,
    nullptr,
    0,
    renderArea,
    1,
    0,
    nullptr,
    &colorAttachment,
    &depthAttachment,
    nullptr
};
```

This makes attachment state much closer to the commands that use it.

## Dynamic rendering attachments

A dynamic color attachment is described with:

```cpp
VkRenderingAttachmentInfo colorAttachment{
    VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
    nullptr,
    colorView,
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    VK_RESOLVE_MODE_NONE,
    VK_NULL_HANDLE,
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_ATTACHMENT_LOAD_OP_CLEAR,
    VK_ATTACHMENT_STORE_OP_STORE,
    clearValue
};
```

The attachment contains the actual image view and its intended layout.

There is no separate framebuffer object supplying the image view.

## Dynamic depth attachments

Depth is described in the same direct way.

```cpp
VkRenderingAttachmentInfo depthAttachment{
    VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
    nullptr,
    depthView,
    VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
    VK_RESOLVE_MODE_NONE,
    VK_NULL_HANDLE,
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_ATTACHMENT_LOAD_OP_CLEAR,
    VK_ATTACHMENT_STORE_OP_DONT_CARE,
    depthClear
};
```

The rendering info then points to it.

```cpp
renderingInfo.pDepthAttachment = &depthAttachment;
```

The command buffer now contains enough information to begin the rendering
operation.

## Beginning dynamic rendering

Dynamic rendering uses commands introduced by Vulkan 1.3 or the corresponding
extension.

```cpp
vkCmdBeginRendering(commandBuffer, &renderingInfo);
```

Drawing occurs normally:

```cpp
vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
```

Then the operation ends:

```cpp
vkCmdEndRendering(commandBuffer);
```

The command sequence is structurally similar to the traditional model, but
the attachment description is local to the rendering command.

## The major architectural difference

The traditional model separates configuration from command recording:

```text
render pass
    +
framebuffer
    +
begin command
```

Dynamic rendering moves more of the configuration into command recording:

```text
rendering info
    |
    v
begin rendering
    |
    v
draw
```

This makes it easier to change attachment combinations without creating a
large collection of compatible render-pass and framebuffer objects.

## Pipeline compatibility

A graphics pipeline still needs to know the formats and sample counts of the
attachments it will render into.

With traditional render passes, this information is tied to the render-pass
compatibility model.

With dynamic rendering, the pipeline creation structure specifies the formats
expected during rendering.

```cpp
VkPipelineRenderingCreateInfo renderingFormats{
    VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
    nullptr,
    1,
    &swapchainFormat,
    depthFormat,
    VK_FORMAT_UNDEFINED
};
```

This keeps the pipeline's attachment interface explicit without requiring a
render pass object.

## Load and store behavior

Dynamic rendering keeps the same fundamental attachment decisions.

```cpp
colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
```

The beginning still needs to determine whether previous contents are cleared,
loaded, or ignored.

The ending still needs to determine whether the result must remain available.

The mechanism changed, but the rendering semantics did not.

## Image layouts still matter

Dynamic rendering does not eliminate image layout management.

A color image still needs an appropriate layout:

```cpp
VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
```

A depth image still needs a depth-attachment layout:

```cpp
VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
```

The renderer remains responsible for transitions and synchronization.

Dynamic rendering simplifies attachment declaration; it does not remove
Vulkan's explicit resource model.

## Render passes are not "old and useless"

Traditional render passes remain relevant.

They appear in existing engines, tutorials, applications, and compatibility
code. They also expose concepts such as subpasses and attachment dependencies
that remain useful for understanding how a GPU rendering workload is organized.

Dynamic rendering is generally a simpler starting point for new renderers,
especially when the application does not need complex subpass structures.

## Choosing between them

For a small modern renderer, dynamic rendering is often attractive because it
reduces object creation and makes the attachment configuration local.

A traditional render pass can still make sense when an application needs
subpasses, compatibility with an existing architecture, or APIs designed
around the older model.

The important skill is understanding both models rather than memorizing one
as universally correct.

## A direct comparison

The traditional path looks like:

```text
attachment descriptions
    -> subpass
    -> render pass
    -> framebuffer
    -> begin render pass
    -> draw
    -> end render pass
```

The dynamic path looks like:

```text
attachment infos
    -> rendering info
    -> begin rendering
    -> draw
    -> end rendering
```

The second path is smaller because the attachment configuration is expressed
directly at the point where it is used.

## What this lesson establishes

A render pass is a description of an attachment-based rendering operation.
Traditional Vulkan expresses that description through render passes, subpasses,
framebuffers, and dependencies.

Dynamic rendering expresses the active attachments directly in command
recording, eliminating much of that static structure while preserving the
underlying concepts of attachment formats, layouts, load operations, store
operations, and synchronization.

The final lesson uses these pieces to build a complete frame loop and then
turns that loop into a practical renderer architecture.

## Next step

Now type the code version of this lesson.
