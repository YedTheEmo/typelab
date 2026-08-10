# Vulkan resources and memory - concepts

Vulkan separates a resource from the memory that backs it. A buffer or image
describes how the GPU should interpret storage, while VkDeviceMemory provides
the allocation containing that storage.

The basic relationship is:

```text
resource -> requirements -> memory type -> allocation -> binding -> use
````

The application therefore creates a resource first, asks Vulkan what memory it
requires, chooses a compatible memory type, allocates memory, and finally binds
the resource to that memory.

## Buffers

A VkBuffer represents a linear region of GPU-accessible data. It can contain
vertices, indices, uniform data, storage data, or transfer data depending on
the usage flags supplied during creation.

```cpp
VkBufferCreateInfo bufferInfo{
    VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    nullptr,
    0,
    size,
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    VK_SHARING_MODE_EXCLUSIVE,
    0,
    nullptr
};
```

The usage flag is part of the buffer's creation contract. The buffer itself
does not know that its bytes are "vertices"; the command that consumes it
gives those bytes their meaning.

A buffer is therefore best thought of as a GPU resource describing a linear
range of storage rather than as an array that automatically owns memory.

## Images

A VkImage represents structured storage rather than a simple linear byte
range. Its description includes dimensions, format, mip levels, array layers,
samples, tiling, and intended usages.

```cpp
VkImageCreateInfo imageInfo{
    VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    nullptr,
    0,
    VK_IMAGE_TYPE_2D,
    format,
    {width, height, 1},
    1,
    1,
    VK_SAMPLE_COUNT_1_BIT,
    VK_IMAGE_TILING_OPTIMAL,
    VK_IMAGE_USAGE_SAMPLED_BIT,
    VK_SHARING_MODE_EXCLUSIVE,
    0,
    nullptr,
    VK_IMAGE_LAYOUT_UNDEFINED
};
```

Images can represent textures, color attachments, depth attachments, and
transfer targets. Unlike buffers, images also have layouts describing how
their contents are currently intended to be accessed.

That layout becomes important when an image changes roles between operations.

## Resource creation and allocation

Creating a resource does not allocate its backing memory. After creating a
buffer, the application asks Vulkan for its memory requirements.

```cpp
VkMemoryRequirements requirements{};

vkGetBufferMemoryRequirements(
    device,
    buffer,
    &requirements
);
```

The requirements contain three particularly important values:

```text
size
alignment
memoryTypeBits
```

size specifies how much memory is required. alignment specifies where the
resource may begin inside an allocation. memoryTypeBits specifies which memory
types are compatible with the resource.

The resource therefore constrains the possible allocation.

## Memory types

A physical device exposes a collection of memory types and heaps. The
application obtains their properties from the physical device.

```cpp
VkPhysicalDeviceMemoryProperties properties{};

vkGetPhysicalDeviceMemoryProperties(
    physicalDevice,
    &properties
);
```

Common memory properties include DEVICE_LOCAL, HOST_VISIBLE, HOST_COHERENT,
and HOST_CACHED.

DEVICE_LOCAL memory is generally preferred for resources heavily accessed by
the GPU. HOST_VISIBLE memory can be mapped by the CPU and is therefore useful
for uploading or updating data.

The exact relationship between these properties and physical RAM or VRAM
depends on the device. Vulkan exposes capabilities rather than pretending
that every machine has the same memory architecture.

## Choosing a memory type

Memory selection has two separate conditions.

First, the resource must permit the memory type through memoryTypeBits.
Second, the memory type must provide the properties required by the intended
use.

```cpp
if ((requirements.memoryTypeBits & (1u << i)) &&
    (properties & requested) == requested) {
    memoryTypeIndex = i;
}
```

The first expression checks resource compatibility.

The second checks the application's requested properties.

Both conditions are necessary. Choosing a device-local type without checking
memoryTypeBits is not a valid general memory-selection strategy.

## Allocation

After selecting a compatible type, the application creates a memory
allocation.

```cpp
VkMemoryAllocateInfo allocateInfo{
    VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    nullptr,
    requirements.size,
    memoryTypeIndex
};
```

The logical device creates the allocation:

```cpp
VkDeviceMemory memory = VK_NULL_HANDLE;

vkAllocateMemory(
    device,
    &allocateInfo,
    nullptr,
    &memory
);
```

The allocation now exists independently from the buffer. Vulkan intentionally
does not hide the relationship between the resource and its memory.

## Binding

A buffer is connected to an allocation with vkBindBufferMemory.

```cpp
vkBindBufferMemory(
    device,
    buffer,
    memory,
    0
);
```

The final argument is the offset inside the allocation. It can be nonzero when
multiple resources share one larger allocation, provided the offset satisfies
the resource's alignment requirement.

Images use the corresponding operation:

```cpp
vkBindImageMemory(device, image, memory, 0);
```

The resulting relationship can be pictured as:

```text
VkDeviceMemory
+--------------------------------+
| resource A | resource B | ... |
+--------------------------------+
       ^             ^
       |             |
    offset A      offset B
```

This is the foundation for suballocation.

## Why suballocate

A renderer can contain thousands of buffers and images. Creating one
VkDeviceMemory allocation for every resource can create unnecessary overhead
and can run into implementation limits.

A memory allocator can instead obtain larger blocks and divide them into
regions.

```text
large allocation
+--------------------------------------+
| vertex | index | uniform | texture   |
+--------------------------------------+
```

The allocator tracks offsets, sizes, alignment, and free regions.

Vulkan supplies the low-level primitives. The renderer decides how those
primitives are organized into a practical allocation strategy.

## Host-visible memory

CPU uploads generally require host-visible memory. The application maps the
allocation to obtain a CPU-accessible pointer.

```cpp
void* mapped = nullptr;

vkMapMemory(
    device,
    memory,
    0,
    size,
    0,
    &mapped
);
```

The application can then write data:

```cpp
std::memcpy(mapped, sourceData, size);
```

After writing, the mapping can be released:

```cpp
vkUnmapMemory(device, memory);
```

Mapping is a CPU-side operation. It does not submit GPU work and does not by
itself establish all synchronization required for later GPU access.

## Host coherence

HOST_COHERENT affects how host writes become visible to the device.

With coherent memory, the basic mapped-memory case does not require an
explicit flush after writing.

Without coherence, the application may need to flush written ranges:

```cpp
vkFlushMappedMemoryRanges(
    device,
    1,
    &range
);
```

The important idea is that a valid CPU pointer and GPU visibility are separate
concepts.

Memory visibility is one part of synchronization; execution ordering is
another.

## Staging

A common upload architecture separates CPU-friendly memory from GPU-friendly
memory.

```text
CPU
 |
 v
host-visible staging buffer
 |
 | GPU copy
 v
device-local vertex buffer
```

The CPU writes the staging buffer because it can be mapped. A GPU command then
copies its contents into the final device-local buffer.

This allows the final resource to remain in memory optimized for GPU access
without requiring the CPU to map it.

## Buffer copies

The copy is recorded into a command buffer.

```cpp
VkBufferCopy region{
    0,
    0,
    size
};

vkCmdCopyBuffer(
    commandBuffer,
    stagingBuffer,
    vertexBuffer,
    1,
    &region
);
```

This does not immediately copy the bytes from the CPU's point of view. The
CPU records a GPU command, submits it, and the queue eventually executes it.

The staging buffer must therefore remain alive until the GPU has finished
reading it.

## Lifetime

Vulkan does not automatically determine when a resource is no longer in use.

A typical resource lifetime is:

```text
create -> bind -> record -> submit -> wait -> destroy
```

For example, a fence can tell the CPU that submitted work has completed:

```cpp
vkWaitForFences(
    device,
    1,
    &fence,
    VK_TRUE,
    UINT64_MAX
);
```

Only after the relevant GPU work has completed can the application safely
reclaim resources that those commands referenced.

## Image views

An image is often accessed through an image view. The view describes how an
image or one of its subresources should be interpreted.

```cpp
VkImageViewCreateInfo viewInfo{
    VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    nullptr,
    0,
    image,
    VK_IMAGE_VIEW_TYPE_2D,
    format,
    {},
    {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
};
```

The view does not contain another copy of the image. It references the image
and describes the portion and interpretation used by later operations.

The relationship is:

```text
image -> image view -> descriptor or rendering operation
```

Textures and render targets will make this relationship much more important.

## Memory is not resource state

It is useful to keep several concepts separate.

Memory answers where the resource's storage comes from. Resource usage flags
describe what operations the resource was created to support. Synchronization
and image layouts describe how and when those operations may access it.

Therefore, valid memory does not automatically mean valid GPU access.

```text
memory
  +
resource usage
  +
resource state
  +
synchronization
  =
valid GPU access
```

This separation is one of the central ideas behind Vulkan's explicit design.

## The complete model

A buffer or image starts as a resource description. Its requirements constrain
the possible memory types. The application chooses a suitable type, allocates
device memory, and binds the resource to an aligned offset.

The resource can then participate in command execution and synchronization.
After the GPU has finished using it, the application can destroy the resource
and eventually free its allocation.

```text
create
  -> requirements
  -> memory type
  -> allocate
  -> bind
  -> use
  -> synchronize
  -> destroy
  -> free
```

This model is the foundation for vertex buffers, index buffers, uniform
buffers, textures, and render targets.

## Next step

Now type the code version of this lesson.

```
```

