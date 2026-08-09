# Vulkan resources and memory - typing

This lesson types the resource lifecycle: create a buffer, query its memory
needs, allocate memory, bind it, and fill it from the CPU.

## Create a vertex buffer

A vertex buffer is a resource that still needs memory backing.

```
// one vertex has two floats
struct Vertex
{
    float x;
    float y;
};

// three vertices forming a triangle
Vertex vertices[] = {
    {-0.5f, -0.5f},
    { 0.5f, -0.5f},
    { 0.0f,  0.5f}
};

// total byte size of the vertex data
VkDeviceSize bufferSize =
    sizeof(vertices);

// the create-info struct for the buffer
VkBufferCreateInfo bufferInfo{};
// identify the buffer create-info type
bufferInfo.sType =
    VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
// how many bytes the buffer holds
bufferInfo.size = bufferSize;
// the buffer will feed vertex data to drawing
bufferInfo.usage =
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
// the buffer is not shared across queue families
bufferInfo.sharingMode =
    VK_SHARING_MODE_EXCLUSIVE;

// handle that Vulkan will fill in
VkBuffer vertexBuffer = VK_NULL_HANDLE;

// create the buffer resource
VkResult result = vkCreateBuffer(
    device,
    &bufferInfo,
    nullptr,
    &vertexBuffer);

// bail out if buffer creation failed
if (result != VK_SUCCESS)
    return 1;
```

## Query memory requirements

Vulkan reports what memory the buffer needs.

```
// struct that Vulkan fills with the requirements
VkMemoryRequirements requirements{};

// ask how much memory the buffer requires
vkGetBufferMemoryRequirements(
    device,
    vertexBuffer,
    &requirements);
```

## Inspect memory types

Pick a memory type that is both compatible and CPU-visible.

```
// struct listing every memory type of the device
VkPhysicalDeviceMemoryProperties memoryProperties{};

// retrieve the memory properties
vkGetPhysicalDeviceMemoryProperties(
    physicalDevice,
    &memoryProperties);

// index of the memory type we choose
uint32_t memoryTypeIndex = 0;

// walk the memory types looking for a match
for (uint32_t i = 0;
     i < memoryProperties.memoryTypeCount;
     ++i)
{
    // bit i set means this type can back the buffer
    bool compatible =
        requirements.memoryTypeBits & (1 << i);

    // flags describing this memory type
    VkMemoryPropertyFlags properties =
        memoryProperties.memoryTypes[i].propertyFlags;

    // memory that the CPU can see directly
    bool suitable =
        properties &
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

    // CPU writes must also be visible without flushing
    suitable =
        suitable &&
        (properties &
         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // stop at the first compatible, suitable type
    if (compatible && suitable)
    {
        memoryTypeIndex = i;
        break;
    }
}
```

## Allocate device memory

The allocation exists independently of the buffer.

```
// the create-info struct for the allocation
VkMemoryAllocateInfo allocInfo{};
// identify the allocate-info type
allocInfo.sType =
    VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
// number of bytes to allocate
allocInfo.allocationSize =
    requirements.size;
// which memory type to draw from
allocInfo.memoryTypeIndex =
    memoryTypeIndex;

// handle that Vulkan will fill in
VkDeviceMemory vertexMemory = VK_NULL_HANDLE;

// allocate the memory
result = vkAllocateMemory(
    device,
    &allocInfo,
    nullptr,
    &vertexMemory);

// bail out if the allocation failed
if (result != VK_SUCCESS)
    return 1;
```

## Bind the buffer

Binding connects the buffer to its backing allocation.

```
// attach the buffer to the memory at offset zero
result = vkBindBufferMemory(
    device,
    vertexBuffer,
    vertexMemory,
    0);

// bail out if the bind failed
if (result != VK_SUCCESS)
    return 1;
```

## Map the memory

Host-visible memory can be mapped into CPU address space.

```
// pointer that the CPU can write through
void* mapped = nullptr;

// map the allocation's range into CPU memory
result = vkMapMemory(
    device,
    vertexMemory,
    0,            // offset
    bufferSize,   // length
    0,            // flags
    &mapped);

// bail out if the map failed
if (result != VK_SUCCESS)
    return 1;

// copy the vertex data into the mapped memory
std::memcpy(
    mapped,
    vertices,
    static_cast<size_t>(bufferSize));

// release the mapping
vkUnmapMemory(
    device,
    vertexMemory);
```

## Use the buffer

A command buffer binds the buffer before a draw.

```
// vertex data starts at the beginning of the buffer
VkDeviceSize offset = 0;

// bind the vertex buffer at binding point zero
vkCmdBindVertexBuffers(
    commandBuffer,
    0,             // binding number
    1,             // one buffer
    &vertexBuffer,
    &offset);
```

## The device-local alternative

The host-visible memory above is convenient for learning. Production renderers
often copy data into faster device-local memory through a staging buffer.

```
// the staging pattern in one diagram
// CPU data -> host-visible staging buffer -> GPU copy -> device-local buffer
```

## Clean up

Destroy the buffer before freeing its backing memory.

```
// destroy the buffer resource
vkDestroyBuffer(
    device,
    vertexBuffer,
    nullptr);

// free the memory the buffer used
vkFreeMemory(
    device,
    vertexMemory,
    nullptr);
```

## Now type it again

Type the core resource creation sequence.

```
// the create-info struct for the buffer
VkBufferCreateInfo bufferInfo{};
// identify the buffer create-info type
bufferInfo.sType =
    VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
// how many bytes the buffer holds
bufferInfo.size = bufferSize;
// the buffer will feed vertex data to drawing
bufferInfo.usage =
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
// the buffer is not shared across queue families
bufferInfo.sharingMode =
    VK_SHARING_MODE_EXCLUSIVE;
```

Then query and allocate its memory.

```
// struct that Vulkan fills with the requirements
VkMemoryRequirements requirements{};

// ask how much memory the buffer requires
vkGetBufferMemoryRequirements(
    device,
    vertexBuffer,
    &requirements);

// the create-info struct for the allocation
VkMemoryAllocateInfo allocInfo{};
// identify the allocate-info type
allocInfo.sType =
    VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
// number of bytes to allocate
allocInfo.allocationSize =
    requirements.size;
// which memory type to draw from
allocInfo.memoryTypeIndex =
    memoryTypeIndex;
```

Finally bind it.

```
// attach the buffer to the memory at offset zero
vkBindBufferMemory(
    device,
    vertexBuffer,
    vertexMemory,
    0);
```

## Wrap up

The flow: create resource -> query requirements -> allocate memory -> bind -> use.
