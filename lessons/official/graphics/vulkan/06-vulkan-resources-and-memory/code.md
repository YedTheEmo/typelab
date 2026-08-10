````text
# Vulkan resources and memory - typing

This lesson types the resource path: create a buffer, query its requirements,
allocate compatible memory, bind it, upload through staging, and clean up.

## Create the buffer

Create a buffer whose intended use is vertex input.

```cpp
    // describe the vertex buffer
    VkBufferCreateInfo bufferInfo{
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr
    };

    // store the vertex buffer handle
    VkBuffer vertexBuffer = VK_NULL_HANDLE;

    // create the vertex buffer
    vkCreateBuffer(
        device,
        &bufferInfo,
        nullptr,
        &vertexBuffer
    );
````

## Query requirements

Ask the device what memory the buffer requires.

```cpp
    // store the buffer requirements
    VkMemoryRequirements memoryRequirements{};

    // query the buffer requirements
    vkGetBufferMemoryRequirements(
        device,
        vertexBuffer,
        &memoryRequirements
    );
```

## Inspect memory types

Read the memory types exposed by the physical device.

```cpp
    // store the physical-device memory properties
    VkPhysicalDeviceMemoryProperties memoryProperties{};

    // query the available memory types
    vkGetPhysicalDeviceMemoryProperties(
        physicalDevice,
        &memoryProperties
    );
```

## Select device-local memory

Find a memory type compatible with the buffer and suitable for GPU storage.

```cpp
    // store the selected memory type
    uint32_t memoryTypeIndex = UINT32_MAX;

    // search every available memory type
    for (
        uint32_t i = 0;
        i < memoryProperties.memoryTypeCount;
        ++i
    ) {
        // read the properties of this memory type
        VkMemoryPropertyFlags properties =
            memoryProperties.memoryTypes[i].propertyFlags;

        // check compatibility and requested properties
        if (
            (memoryRequirements.memoryTypeBits & (1u << i)) &&
            (properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) ==
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        ) {
            // store the compatible memory type
            memoryTypeIndex = i;

            // stop after finding a suitable type
            break;
        }
    }
```

## Allocate buffer memory

Allocate memory using the size and selected type from the requirements.

```cpp
    // describe the buffer allocation
    VkMemoryAllocateInfo allocateInfo{
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        memoryRequirements.size,
        memoryTypeIndex
    };

    // store the buffer allocation
    VkDeviceMemory vertexMemory = VK_NULL_HANDLE;

    // allocate the buffer memory
    vkAllocateMemory(
        device,
        &allocateInfo,
        nullptr,
        &vertexMemory
    );
```

## Bind the memory

Connect the buffer to the allocation at offset zero.

```cpp
    // bind the buffer to its allocation
    vkBindBufferMemory(
        device,
        vertexBuffer,
        vertexMemory,
        0
    );
```

## Create a staging buffer

Create a transfer source that the CPU can populate.

```cpp
    // describe the staging buffer
    VkBufferCreateInfo stagingInfo{
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        uploadSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr
    };

    // store the staging buffer
    VkBuffer stagingBuffer = VK_NULL_HANDLE;

    // create the staging buffer
    vkCreateBuffer(
        device,
        &stagingInfo,
        nullptr,
        &stagingBuffer
    );
```

## Query staging requirements

The staging buffer has its own memory requirements.

```cpp
    // store the staging requirements
    VkMemoryRequirements stagingRequirements{};

    // query the staging requirements
    vkGetBufferMemoryRequirements(
        device,
        stagingBuffer,
        &stagingRequirements
    );
```

## Select host-visible memory

Find a compatible memory type that the CPU can map.

```cpp
    // store the selected staging memory type
    uint32_t stagingMemoryType = UINT32_MAX;

    // search every available memory type
    for (
        uint32_t i = 0;
        i < memoryProperties.memoryTypeCount;
        ++i
    ) {
        // read the properties of this memory type
        VkMemoryPropertyFlags properties =
            memoryProperties.memoryTypes[i].propertyFlags;

        // check compatibility and host visibility
        if (
            (stagingRequirements.memoryTypeBits & (1u << i)) &&
            (properties & (
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            )) == (
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            )
        ) {
            // store the compatible memory type
            stagingMemoryType = i;

            // stop after finding a suitable type
            break;
        }
    }
```

## Allocate staging memory

Allocate memory using the staging requirements.

```cpp
    // describe the staging allocation
    VkMemoryAllocateInfo stagingAllocateInfo{
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        stagingRequirements.size,
        stagingMemoryType
    };

    // store the staging allocation
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;

    // allocate staging memory
    vkAllocateMemory(
        device,
        &stagingAllocateInfo,
        nullptr,
        &stagingMemory
    );
```

## Bind the staging buffer

Connect the staging buffer to its host-visible allocation.

```cpp
    // bind the staging buffer to its allocation
    vkBindBufferMemory(
        device,
        stagingBuffer,
        stagingMemory,
        0
    );
```

## Map the staging memory

Map the allocation so the CPU can write the upload data.

```cpp
    // store the mapped address
    void* mappedData = nullptr;

    // map the staging allocation
    vkMapMemory(
        device,
        stagingMemory,
        0,
        uploadSize,
        0,
        &mappedData
    );
```

## Write the upload

Copy application data into the mapped staging allocation.

```cpp
    // copy application data into the staging allocation
    std::memcpy(
        mappedData,
        sourceData,
        uploadSize
    );
```

## Unmap the staging memory

Release the CPU mapping after the write is complete.

```cpp
    // release the CPU mapping
    vkUnmapMemory(
        device,
        stagingMemory
    );
```

## Record the transfer

Describe the bytes that the GPU should copy.

```cpp
    // describe the buffer copy
    VkBufferCopy copyRegion{
        0,
        0,
        uploadSize
    };

    // record the staging-to-device copy
    vkCmdCopyBuffer(
        commandBuffer,
        stagingBuffer,
        vertexBuffer,
        1,
        &copyRegion
    );
```

## Wait for completion

Wait until the submitted transfer has finished before destroying staging data.

```cpp
    // wait for the transfer submission
    vkWaitForFences(
        device,
        1,
        &uploadFence,
        VK_TRUE,
        UINT64_MAX
    );
```

## Destroy the staging buffer

The staging buffer can now be destroyed after the GPU has finished using it.

```cpp
    // destroy the staging buffer
    vkDestroyBuffer(
        device,
        stagingBuffer,
        nullptr
    );
```

## Free staging memory

Release the staging allocation separately from the buffer object.

```cpp
    // free the staging allocation
    vkFreeMemory(
        device,
        stagingMemory,
        nullptr
    );
```

## Destroy the vertex buffer

Destroy the final buffer when no submitted work can reference it.

```cpp
    // destroy the vertex buffer
    vkDestroyBuffer(
        device,
        vertexBuffer,
        nullptr
    );
```

## Free vertex memory

Release the allocation backing the vertex buffer.

```cpp
    // free the vertex allocation
    vkFreeMemory(
        device,
        vertexMemory,
        nullptr
    );
```

## Now type it again

Re-drill resource creation and requirement discovery.

```cpp
    // describe the vertex buffer
    VkBufferCreateInfo bufferInfo{
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr
    };

    // store the vertex buffer
    VkBuffer vertexBuffer = VK_NULL_HANDLE;

    // create the vertex buffer
    vkCreateBuffer(
        device,
        &bufferInfo,
        nullptr,
        &vertexBuffer
    );

    // store the memory requirements
    VkMemoryRequirements memoryRequirements{};

    // query the memory requirements
    vkGetBufferMemoryRequirements(
        device,
        vertexBuffer,
        &memoryRequirements
    );
```

Re-drill memory-type selection and allocation.

```cpp
    // store the physical-device memory properties
    VkPhysicalDeviceMemoryProperties memoryProperties{};

    // query the memory properties
    vkGetPhysicalDeviceMemoryProperties(
        physicalDevice,
        &memoryProperties
    );

    // store the selected memory type
    uint32_t memoryTypeIndex = UINT32_MAX;

    // search the memory types
    for (
        uint32_t i = 0;
        i < memoryProperties.memoryTypeCount;
        ++i
    ) {
        // read the memory properties
        VkMemoryPropertyFlags properties =
            memoryProperties.memoryTypes[i].propertyFlags;

        // check compatibility and device-local support
        if (
            (memoryRequirements.memoryTypeBits & (1u << i)) &&
            (properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) ==
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        ) {
            // store the compatible memory type
            memoryTypeIndex = i;

            // stop searching
            break;
        }
    }

    // describe the memory allocation
    VkMemoryAllocateInfo allocateInfo{
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        memoryRequirements.size,
        memoryTypeIndex
    };

    // store the memory allocation
    VkDeviceMemory vertexMemory = VK_NULL_HANDLE;

    // allocate device memory
    vkAllocateMemory(
        device,
        &allocateInfo,
        nullptr,
        &vertexMemory
    );

    // bind the buffer to its allocation
    vkBindBufferMemory(
        device,
        vertexBuffer,
        vertexMemory,
        0
    );
```

Re-drill the CPU-to-GPU staging path.

```cpp
    // map the staging allocation
    vkMapMemory(
        device,
        stagingMemory,
        0,
        uploadSize,
        0,
        &mappedData
    );

    // copy application data into mapped memory
    std::memcpy(
        mappedData,
        sourceData,
        uploadSize
    );

    // release the CPU mapping
    vkUnmapMemory(
        device,
        stagingMemory
    );

    // describe the transfer region
    VkBufferCopy copyRegion{
        0,
        0,
        uploadSize
    };

    // record the GPU copy
    vkCmdCopyBuffer(
        commandBuffer,
        stagingBuffer,
        vertexBuffer,
        1,
        &copyRegion
    );
```

## Wrap up

```text
resource -> requirements -> memory type -> allocate -> bind -> upload -> use
```

```
```

