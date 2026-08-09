# Vulkan resources and memory - concepts

A Vulkan application does not put data directly into a GPU by calling a single
allocation function. Instead, it creates a resource such as a buffer or image,
allocates suitable device memory, and binds the resource to that memory.

This separation is one of the reasons Vulkan can be explicit and flexible.

The basic relationship is:

```
resource
    |
    v
device memory
    |
    v
   GPU
```

A VkBuffer describes a region of memory intended for structured data. A
VkImage describes memory used for image data such as textures or render
targets.

## Buffers

A buffer represents a linear region of data.

For example, a vertex buffer might contain:

```
vertex 0
vertex 1
vertex 2
vertex 3
```

Create one by describing its size and intended usage:

```
VkBufferCreateInfo info{};
info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
info.size = 1024;
info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

VkBuffer buffer;

vkCreateBuffer(
    device,
    &info,
    nullptr,
    &buffer);
```

The usage flag tells Vulkan what operations the buffer is intended to support.

A buffer itself does not contain allocated device memory yet. It describes a
resource that needs memory backing.

## Images

Images are similar resources, but they have dimensions, formats, and image
layouts.

A color image might be described as:

```
VkImageCreateInfo info{};
info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
info.imageType = VK_IMAGE_TYPE_2D;
info.format = VK_FORMAT_R8G8B8A8_SRGB;
info.extent = { 1280, 720, 1 };
info.mipLevels = 1;
info.arrayLayers = 1;
```

An image may be used as a color attachment, sampled texture, transfer target,
or for other purposes depending on its usage flags.

Images therefore carry more structural information than buffers.

## Query memory requirements

After creating a resource, ask Vulkan what memory it requires.

For a buffer:

```
VkMemoryRequirements requirements;

vkGetBufferMemoryRequirements(
    device,
    buffer,
    &requirements);
```

The result includes the required allocation size, alignment, and a bitmask
describing compatible memory types.

The application cannot simply choose any device memory.

## Memory types

A physical device exposes several memory types.

They differ in properties such as whether the CPU can directly map them or
whether they are intended primarily for GPU access.

Query the available memory properties:

```
VkPhysicalDeviceMemoryProperties memoryProperties;

vkGetPhysicalDeviceMemoryProperties(
    physicalDevice,
    &memoryProperties);
```

The memory requirements provide a bitmask. Each set bit identifies a memory
type that is compatible with the resource.

The application combines that mask with desired properties to find a suitable
type.

For example, CPU-visible memory commonly uses:

```
VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
```

Coherent host memory additionally uses:

```
VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
```

Device-local memory commonly uses:

```
VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
```

## Allocate memory

Once a suitable memory type has been selected, create an allocation:

```
VkMemoryAllocateInfo allocInfo{};
allocInfo.sType =
    VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
allocInfo.allocationSize =
    requirements.size;
allocInfo.memoryTypeIndex =
    memoryTypeIndex;

VkDeviceMemory memory;

vkAllocateMemory(
    device,
    &allocInfo,
    nullptr,
    &memory);
```

The allocation now represents actual device memory.

The resource still needs to be bound to it.

## Bind the resource

Bind the buffer to the allocation:

```
vkBindBufferMemory(
    device,
    buffer,
    memory,
    0);
```

The final argument is the offset into the allocation.

The complete relationship is now:

```
VkBuffer
    |
    | vkBindBufferMemory
    v
VkDeviceMemory
```

This distinction is important. A buffer is a Vulkan resource describing how
memory will be used. VkDeviceMemory is the allocation that backs that resource.

## Uploading data

If memory is host-visible, the CPU can map it:

```
void* mapped = nullptr;

vkMapMemory(
    device,
    memory,
    0,
    dataSize,
    0,
    &mapped);
```

The application can then copy data into the mapped region:

```
std::memcpy(mapped, data, dataSize);
```

Afterward, unmap it:

```
vkUnmapMemory(device, memory);
```

For some memory types, the GPU can access the same allocation directly.

A common high-performance pattern instead uses a CPU-visible staging buffer,
then copies the data into device-local memory using a GPU transfer operation.

## Device-local memory

GPU-local memory is generally preferable for resources that the GPU accesses
frequently.

A typical upload path therefore looks like:

```
CPU data
   |
   v
staging buffer
   |
   v
GPU copy
   |
   v
device-local buffer
```

The staging buffer is temporary. The final resource lives in memory optimized
for GPU access.

This pattern is common for vertex buffers, index buffers, and textures.

## Images have another concern

Images have layouts that describe how the GPU intends to use them.

An image might transition from an undefined layout into a transfer destination,
then into a shader-readable layout.

Conceptually:

```
undefined
    |
    v
transfer destination
    |
    v
shader readable
```

These transitions are recorded as GPU commands and are part of Vulkan's
explicit resource management model.

Synchronization and image layout transitions are closely related because the
GPU must know when a resource changes from one usage to another.

## The resource model

The important distinction is:

```
VkBuffer / VkImage
    = resource description

VkDeviceMemory
    = memory allocation

binding
    = relationship between them
```

This gives the application explicit control over where resources live and how
they are accessed.

Later lessons will use these resources as vertex data, shader inputs, and
images rendered by the graphics pipeline.

## Next step

Now type the code version of this lesson.

