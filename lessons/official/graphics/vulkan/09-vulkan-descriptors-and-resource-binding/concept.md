# Vulkan descriptors and resource binding - concepts

A graphics pipeline can execute shader code without external resources, but
useful rendering quickly needs data that lives outside the shader itself.
Transforms, camera parameters, material values, textures, and storage buffers
all need a defined path from application-owned resources to shader-visible
resources.

Vulkan uses descriptor sets to make that path explicit.

The useful mental model is:

```text
GPU resource
    |
    v
descriptor
    |
    v
descriptor set
    |
    v
pipeline layout
    |
    v
shader
```

A descriptor does not normally contain the resource's data. It describes how
the shader should access a resource such as a buffer or image view.

## Why descriptors exist

A shader needs a stable interface for external data. Instead of making the
shader know where a particular allocation happens to live in GPU memory, the
application binds resources through descriptors.

For example, a uniform buffer might contain transformation data:

```cpp
struct UniformBufferObject {
    float model[16];
    float view[16];
    float projection[16];
};
```

The shader can then read those values through a uniform-buffer binding.

The important separation is that the buffer stores bytes while the descriptor
describes how those bytes are exposed to a shader binding.

## Descriptor types

Vulkan provides different descriptor types for different resource access
patterns. A uniform buffer is commonly represented as:

```cpp
VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
```

A storage buffer uses:

```cpp
VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
```

A sampled image and sampler can be represented through descriptors as well.

```cpp
VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
```

The descriptor type is part of the interface contract. The shader expects a
particular kind of resource at a particular binding, and the descriptor set
must provide a compatible resource there.

## Descriptor bindings

A descriptor set layout describes which bindings a set contains. A binding
has a number, a descriptor type, a count, and the shader stages allowed to use
it.

```cpp
VkDescriptorSetLayoutBinding binding{
    0,
    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    1,
    VK_SHADER_STAGE_VERTEX_BIT,
    nullptr
};
```

Binding zero in this example represents one uniform buffer visible to the
vertex shader.

The binding number is important because shader declarations refer to that
number. Vulkan does not infer that the first buffer created by the application
should become binding zero.

## Descriptor set layouts

A descriptor set layout groups the binding declarations into an interface.

```cpp
VkDescriptorSetLayoutCreateInfo layoutInfo{
    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    nullptr,
    0,
    1,
    &binding
};
```

The resulting layout is not the resource itself. It is a description of the
shape of a descriptor set.

Think of it like a type:

```text
layout:
    binding 0 -> one uniform buffer
```

A descriptor set allocated from that layout must provide resources matching
that shape.

## Descriptor pools

Descriptor sets are allocated from descriptor pools. The pool specifies the
kinds and quantities of descriptors it can provide.

```cpp
VkDescriptorPoolSize poolSize{
    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    1
};
```

The pool creation information uses that capacity:

```cpp
VkDescriptorPoolCreateInfo poolInfo{
    VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    nullptr,
    0,
    1,
    1,
    &poolSize
};
```

The pool is therefore an allocation source rather than a description of the
shader interface.

This gives three distinct objects:

```text
descriptor set layout -> describes shape
descriptor pool       -> provides allocation capacity
descriptor set        -> contains bindings
```

Keeping these roles separate makes descriptor management much easier to reason
about.

## Allocating a descriptor set

Once the layout and pool exist, a descriptor set can be allocated.

```cpp
VkDescriptorSetAllocateInfo allocateInfo{
    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    nullptr,
    descriptorPool,
    1,
    &descriptorSetLayout
};
```

The resulting descriptor set is the object that will eventually be bound for
a draw.

It is useful to think of the descriptor set as a populated instance of the
descriptor-set layout.

```text
layout = template
set    = populated instance
```

The resources themselves are still supplied separately.

## Buffer descriptors

A buffer descriptor needs to identify the buffer and the byte range visible
through the binding.

```cpp
VkDescriptorBufferInfo bufferInfo{
    uniformBuffer,
    0,
    sizeof(UniformBufferObject)
};
```

The offset identifies where the visible range begins. The range identifies how
many bytes are exposed.

This is important because a large buffer can contain multiple logical data
regions. A descriptor can expose only the relevant region.

## Writing descriptors

The descriptor set is populated with write operations.

```cpp
VkWriteDescriptorSet write{
    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    nullptr,
    descriptorSet,
    0,
    0,
    1,
    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    nullptr,
    &bufferInfo,
    nullptr
};
```

The destination set and binding identify where the resource goes. The
descriptor type must agree with the layout. The buffer information supplies
the actual resource.

The update is then performed:

```cpp
vkUpdateDescriptorSets(
    device,
    1,
    &write,
    0,
    nullptr
);
```

This changes the descriptor set's resource bindings. It does not rewrite the
shader or recreate the pipeline.

## Pipeline layout and descriptor sets

The pipeline layout connects descriptor set layouts to the graphics pipeline.

```cpp
VkPipelineLayoutCreateInfo pipelineLayoutInfo{
    VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    nullptr,
    0,
    1,
    &descriptorSetLayout,
    0,
    nullptr
};
```

The layout says that set zero follows the descriptor-set layout supplied here.

The shader's resource interface, descriptor set layout, and pipeline layout
therefore form one connected contract.

```text
shader binding
      |
      v
descriptor set layout
      |
      v
pipeline layout
      |
      v
graphics pipeline
```

Changing this interface can require a different pipeline layout and therefore
a different graphics pipeline.

## Binding a descriptor set

A descriptor set becomes active for subsequent draw commands when it is bound.

```cpp
vkCmdBindDescriptorSets(
    commandBuffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipelineLayout,
    0,
    1,
    &descriptorSet,
    0,
    nullptr
);
```

The first set number is zero in this example.

The descriptor set does not need to be rebound for every command if the same
resource state remains appropriate. A renderer can bind a set and then issue
multiple compatible draws.

## Sets and bindings

Descriptor sets provide another level of organization.

Suppose a renderer uses:

```text
set 0 -> frame data
set 1 -> material data
set 2 -> object data
```

Each set can then contain several bindings:

```text
set 1
    binding 0 -> albedo texture
    binding 1 -> sampler
    binding 2 -> material buffer
```

The exact organization is a renderer design decision. Vulkan provides the
mechanism but does not prescribe which information belongs in each set.

The useful question is not "what set number should I use?" but "which
resources change together?"

## Frequency of change

A practical descriptor design often groups resources according to how often
they change.

Frame-wide data might change once per frame:

```text
set 0 -> camera and frame data
```

Material data might change when the material changes:

```text
set 1 -> textures and material parameters
```

Object data might change for every object:

```text
set 2 -> model transform and object data
```

This organization can reduce unnecessary rebinding and makes the renderer's
resource lifetime easier to reason about.

It is not a Vulkan requirement. It is an architectural strategy built on top
of Vulkan's descriptor model.

## Uniform buffers

Uniform buffers are intended for relatively small shader-readable data that
is updated frequently enough to justify a simple structured interface.

```cpp
VkDescriptorBufferInfo frameInfo{
    frameUniformBuffer,
    0,
    sizeof(FrameData)
};
```

The application updates the buffer's contents separately from descriptor
management.

This distinction matters:

```text
update buffer contents
        !=
update descriptor binding
```

If the same buffer remains bound but its bytes change, the descriptor itself
does not necessarily need to be rewritten.

## Storage buffers

Storage buffers provide a more flexible buffer interface and are commonly
used for larger arrays or data that shaders need to read or write.

```cpp
VkDescriptorSetLayoutBinding storageBinding{
    1,
    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    1,
    VK_SHADER_STAGE_VERTEX_BIT |
    VK_SHADER_STAGE_FRAGMENT_BIT,
    nullptr
};
```

The shader stage visibility can include multiple stages.

Storage buffers are especially useful when the shader needs access to arrays
of objects, particles, instances, or other structured data whose size or usage
does not fit the simpler uniform-buffer model.

## Image descriptors

Textures are exposed through image-related descriptors.

A sampled image descriptor points toward an image view and specifies its layout:

```cpp
VkDescriptorImageInfo imageInfo{
    sampler,
    imageView,
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
};
```

The image view describes which part and format of an image is exposed. The
sampler describes how texture coordinates are converted into samples.

A combined image sampler puts both resources into one descriptor:

```text
texture image
    +
sampler
    |
    v
combined image sampler
```

Later lessons will examine textures and samplers in much greater detail.

## Descriptor arrays

A binding can contain more than one descriptor.

```cpp
VkDescriptorSetLayoutBinding textures{
    0,
    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    4,
    VK_SHADER_STAGE_FRAGMENT_BIT,
    nullptr
};
```

The count of four means that binding zero contains an array of four image
sampler descriptors.

This mechanism is useful for material systems, texture arrays, and other
rendering designs that need multiple resources under one logical binding.

## Dynamic uniform buffers

Vulkan also provides dynamic buffer descriptor types.

```cpp
VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
```

The descriptor identifies a base region, while a dynamic offset supplied during
binding selects a particular range.

This allows one large buffer to contain multiple objects while the command
buffer selects which object's data a draw should read.

The concept is:

```text
large uniform buffer
       |
       +-> object 0
       +-> object 1
       +-> object 2
       |
       v
dynamic offset selects one region
```

Dynamic descriptors are useful, but they are not automatically the best choice
for every renderer. Modern Vulkan applications may use other allocation and
buffering strategies depending on their resource model.

## Push constants

Push constants are a related but distinct mechanism for small amounts of
shader-visible data.

```cpp
VkPushConstantRange pushRange{
    VK_SHADER_STAGE_VERTEX_BIT,
    0,
    sizeof(ObjectPushConstants)
};
```

They live in the pipeline layout rather than in descriptor sets.

The key distinction is:

```text
descriptor -> resource binding
push constant -> small inline values
```

Push constants are particularly useful for small per-draw parameters that do
not justify a separate buffer descriptor.

## Binding order

Descriptor sets are bound while recording commands, normally after the
pipeline has been selected.

```text
bind pipeline
    |
    v
bind descriptor set
    |
    v
bind vertex resources
    |
    v
draw
```

The bound descriptor sets remain part of the command state until another
compatible binding changes them.

This allows a renderer to organize commands around changes in resource state.

## Descriptor lifetime

Descriptor sets refer to resources, so those resources must remain valid while
the GPU can use them.

Destroying a buffer or image while a descriptor set still refers to it is not
made safe merely because the descriptor itself exists.

The renderer therefore needs lifetime rules that connect:

```text
resource lifetime
    +
descriptor lifetime
    +
GPU execution lifetime
```

This is one reason Vulkan applications commonly delay destruction until fences
or other synchronization mechanisms establish that the GPU is finished.

## The complete resource path

A descriptor is only one link in a larger chain.

```text
buffer or image
      |
      v
descriptor info
      |
      v
descriptor set
      |
      v
pipeline layout
      |
      v
bind descriptor set
      |
      v
shader access
```

Once this path is understood, textures, materials, camera buffers, and object
arrays become variations on the same resource-binding model.

## What this lesson establishes

Descriptors provide Vulkan's explicit interface between resources owned by the
application and resource bindings expected by shaders.

The descriptor set layout defines the shape, the descriptor pool provides
allocation capacity, the descriptor set contains actual bindings, and the
pipeline layout connects those bindings to the pipeline. Command recording then
binds the set so shaders can access the resources during a draw.

The next lesson builds on this model by focusing specifically on images,
samplers, layouts, and texture sampling.

## Next step

Now type the code version of this lesson.
