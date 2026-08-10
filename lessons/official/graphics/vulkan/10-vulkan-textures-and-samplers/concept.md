# Vulkan textures and samplers - concepts

A texture is GPU image data that a shader can sample. Vulkan separates the
image's storage, the view that describes how part of that storage is exposed,
and the sampler that describes how texture coordinates become samples.

The useful mental model is:

```text
image memory
    |
    v
image view
    |
    +------ sampler
    |          |
    +----------+
         |
         v
descriptor
         |
         v
fragment shader
```

This separation is one of Vulkan's most important resource concepts. An image
does not automatically become a shader-readable texture simply because pixels
exist in its memory.

## Images and buffers are different

Buffers represent linear regions of bytes. Images represent texels with a
format, dimensions, layers, and potentially mip levels.

```cpp
VkImageCreateInfo imageInfo{
    VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    nullptr,
    0,
    VK_IMAGE_TYPE_2D,
    VK_FORMAT_R8G8B8A8_SRGB,
    {width, height, 1},
    1,
    1,
    VK_SAMPLE_COUNT_1_BIT,
    VK_IMAGE_TILING_OPTIMAL,
    VK_IMAGE_USAGE_SAMPLED_BIT |
    VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    VK_SHARING_MODE_EXCLUSIVE,
    0,
    nullptr,
    VK_IMAGE_LAYOUT_UNDEFINED
};
```

The image describes storage and usage. It does not describe a particular way
a shader accesses that storage.

The usage flags matter because Vulkan needs to know how the image will be used
when the resource is created.

## Image usage

A texture uploaded from CPU-visible staging data commonly needs both transfer
destination and sampled usage.

```cpp
VK_IMAGE_USAGE_TRANSFER_DST_BIT |
VK_IMAGE_USAGE_SAMPLED_BIT
```

The transfer usage permits a copy operation to write the image. The sampled
usage permits shader sampling.

A texture that is rendered into and later sampled would need additional usage:

```cpp
VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
VK_IMAGE_USAGE_SAMPLED_BIT
```

The usage declaration is therefore part of the resource's intended lifetime,
not merely an optimization hint.

## Image formats

A format defines how texel bits are interpreted.

```cpp
VK_FORMAT_R8G8B8A8_SRGB
```

This format contains four eight-bit components. The sRGB designation also
affects how color data is interpreted for color-space conversion.

A format such as:

```cpp
VK_FORMAT_R8G8B8A8_UNORM
```

has the same basic component storage but different numeric interpretation.

Format selection therefore matters both for memory representation and for
the meaning of sampled values.

## Image views

Shaders normally access an image through an image view.

```cpp
VkImageViewCreateInfo viewInfo{
    VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    nullptr,
    0,
    textureImage,
    VK_IMAGE_VIEW_TYPE_2D,
    VK_FORMAT_R8G8B8A8_SRGB,
    {},
    {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,
        1,
        0,
        1
    }
};
```

The view identifies the underlying image, the view type, the format, and the
subresource range that is exposed.

This becomes important for mipmaps and texture arrays because one image can
contain multiple levels or layers while different views can expose different
parts.

The relationship is:

```text
VkImage
  |
  +-- mip level 0
  +-- mip level 1
  +-- mip level 2
  +-- layers
       |
       v
   VkImageView
```

## Image layouts

Vulkan tracks image layouts because the way an image is accessed affects how
the implementation can use it.

A newly created image commonly starts in:

```cpp
VK_IMAGE_LAYOUT_UNDEFINED
```

A sampled texture normally needs:

```cpp
VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
```

The application records transitions between layouts as the image changes use.

The important mental model is:

```text
undefined
    -> transfer destination
    -> shader read only
```

The transition is not simply a label change. It also establishes the
synchronization and access relationship required by the new usage.

## Why layout transitions exist

Suppose a copy operation writes texture data and a fragment shader later reads
that same data. The copy must finish before the shader begins reading.

```text
transfer write
      |
      v
layout transition
      |
      v
shader read
```

The transition describes both the old and new usage and provides the required
ordering between them.

This is why Vulkan image layouts should be understood together with
synchronization rather than treated as unrelated enum values.

## Image memory barriers

An image transition is commonly described with an image memory barrier.

```cpp
VkImageMemoryBarrier2 barrier{
    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    nullptr,
    VK_PIPELINE_STAGE_2_TRANSFER_BIT,
    VK_ACCESS_2_TRANSFER_WRITE_BIT,
    VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
    VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    queueFamily,
    queueFamily,
    textureImage,
    {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,
        1,
        0,
        1
    }
};
```

The source stage and access describe the operation that must finish. The
destination stage and access describe the operation that must wait.

The subresource range says which image mip levels and layers participate.

## Samplers

An image contains texels. A sampler defines how coordinates are converted
into sampled values.

```cpp
VkSamplerCreateInfo samplerInfo{
    VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    nullptr,
    0,
    VK_FILTER_LINEAR,
    VK_FILTER_LINEAR,
    VK_SAMPLER_MIPMAP_MODE_LINEAR,
    VK_SAMPLER_ADDRESS_MODE_REPEAT,
    VK_SAMPLER_ADDRESS_MODE_REPEAT,
    VK_SAMPLER_ADDRESS_MODE_REPEAT,
    0.0f,
    VK_FALSE,
    1.0f,
    VK_FALSE,
    VK_COMPARE_OP_ALWAYS,
    0.0f,
    1.0f,
    VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
    VK_FALSE
};
```

The sampler controls filtering, addressing, mip selection, and several other
sampling behaviors.

This is why a sampler is not the texture itself. The same image can be sampled
with different samplers.

## Nearest and linear filtering

Nearest filtering chooses a nearby texel directly. Linear filtering blends
neighboring texels.

```cpp
VK_FILTER_NEAREST
```

and:

```cpp
VK_FILTER_LINEAR
```

are the basic choices.

Filtering changes how a texture appears when its texel grid does not map
one-to-one onto screen pixels.

The texture stores the source data; the sampler controls how that data is
interpolated during lookup.

## Address modes

Texture coordinates can fall outside the normalized range. Address modes
determine what happens then.

```cpp
VK_SAMPLER_ADDRESS_MODE_REPEAT
```

repeats the texture.

```cpp
VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
```

extends the nearest edge texel.

Other modes provide mirrored repetition or border colors.

This makes the sampler part of the texture's visual behavior. Changing only the
sampler can change how the same image appears across a surface.

## Normalized coordinates

A typical 2D texture uses normalized coordinates:

```text
(0, 0) -> one corner
(1, 1) -> the opposite corner
```

A shader might conceptually perform:

```glsl
vec4 color = texture(albedo, uv);
```

The sampler then determines filtering and addressing behavior for that lookup.

The shader provides coordinates. The image provides texels. The sampler
defines how those coordinates become a sampled result.

## Mipmaps

A large texture viewed from far away does not need every original texel.
Mipmaps provide progressively smaller versions of the image.

```text
level 0 -> 1024 x 1024
level 1 ->  512 x  512
level 2 ->  256 x  256
level 3 ->  128 x  128
```

The sampler can select and filter between mip levels.

The image therefore needs enough mip levels to store the chain:

```cpp
mipLevels = floor(log2(max(width, height))) + 1;
```

The actual mip generation process requires either precomputed levels or GPU
operations that produce the smaller images.

## Creating a mipmapped image

An image declares its mip count when it is created.

```cpp
imageInfo.mipLevels = mipLevels;
```

The view can then expose all levels:

```cpp
viewInfo.subresourceRange.levelCount = mipLevels;
```

A texture descriptor can still expose the entire image while the sampler
selects which mip level to read.

The image, view, and sampler therefore cooperate to make mipmapped sampling
possible.

## Uploading texture data

Texture pixels commonly begin in CPU-visible staging memory.

```text
file or generated pixels
        |
        v
staging buffer
        |
        v
copy command
        |
        v
optimal image
```

The staging buffer is convenient for CPU writes. The optimal image is the
resource intended for GPU sampling.

The copy operation connects the two.

```cpp
vkCmdCopyBufferToImage(
    commandBuffer,
    stagingBuffer,
    textureImage,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    1,
    &region
);
```

The destination image must be in the transfer-destination layout for this
operation.

## The upload transition sequence

A simple texture upload has a predictable sequence.

```text
UNDEFINED
    -> TRANSFER_DST_OPTIMAL
    -> copy pixels
    -> SHADER_READ_ONLY_OPTIMAL
```

The first transition prepares the image to receive the copy. The copy writes
the texture data. The second transition prepares the completed data for shader
reads.

This pattern is one of the most reusable image workflows in Vulkan.

## Texture descriptors

Once the image view and sampler exist, they can be exposed through a combined
image sampler descriptor.

```cpp
VkDescriptorImageInfo imageInfo{
    sampler,
    textureView,
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
};
```

The descriptor tells the shader which sampler and image view form the sampled
resource.

The resource path is now complete:

```text
image
  +
view
  +
sampler
  |
  v
descriptor
  |
  v
fragment shader
```

## Sampling in the shader

A texture coordinate normally comes from the vertex shader and is interpolated
across the primitive.

```glsl
layout(location = 1) in vec2 uv;
```

The fragment shader can then sample the texture:

```glsl
vec4 color = texture(albedo, uv);
```

Each fragment receives an interpolated coordinate. The sampler uses that
coordinate to select and filter texels from the image.

This is the first important example of descriptor-backed data changing the
result of a shader without changing the shader's pipeline structure.

## Color data and sRGB

Texture formats can represent color data in different numeric spaces.
sRGB formats are particularly important for ordinary color textures because
the stored values are encoded rather than being simple linear-light values.

A color texture commonly uses:

```cpp
VK_FORMAT_R8G8B8A8_SRGB
```

while data intended to represent linear quantities may use a UNORM format.

Normals, masks, roughness values, and other non-color data generally should not
be treated as sRGB color data.

The format is therefore part of the meaning of the texture, not merely its
storage size.

## Texture lifetime

A sampled texture usually involves several Vulkan objects:

```text
VkImage
VkDeviceMemory or allocation
VkImageView
VkSampler
descriptor
```

These objects have different responsibilities and lifetimes.

The image must remain alive while its view or descriptor can be used. The view
must remain valid while the descriptor refers to it. The sampler must remain
valid while the descriptor uses it.

As with other Vulkan resources, destruction must wait until the GPU can no
longer reference the objects.

## What this lesson establishes

A Vulkan texture is a coordinated group of resources rather than one object.
The image stores texels, the image view exposes a subresource, the sampler
defines sampling behavior, and the descriptor connects that combination to a
shader.

Image layouts and synchronization make transitions between transfer and
shader-read usage explicit. Mipmaps, filtering, addressing, and color formats
then determine how the texture behaves during sampling.

The next lesson moves from color textures to another part of the framebuffer:
rendering attachments for depth and other per-sample information.

## Next step

Now type the code version of this lesson.
