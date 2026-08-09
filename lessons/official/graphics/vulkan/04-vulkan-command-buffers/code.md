# Vulkan command buffers - typing

This lesson types the lifetime of recorded GPU work: create a command pool,
allocate a command buffer, record a command, and submit it to a queue.

## Create the command pool

A command pool belongs to a device and a queue family.

```
// the create-info struct for the command pool
VkCommandPoolCreateInfo poolInfo{};
// identify the pool create-info type
poolInfo.sType =
    VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
// buffers from this pool run on this family
poolInfo.queueFamilyIndex = graphicsFamily;

// handle that Vulkan will fill in
VkCommandPool commandPool = VK_NULL_HANDLE;

// create the command pool on the device
VkResult result = vkCreateCommandPool(
    device,
    &poolInfo,
    nullptr,
    &commandPool);

// bail out if pool creation failed
if (result != VK_SUCCESS)
    return 1;
```

## Allocate a command buffer

Command buffers are allocated from a pool, not created directly.

```
// describes which pool and level to allocate
VkCommandBufferAllocateInfo allocInfo{};
// identify the allocate-info type
allocInfo.sType =
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
// allocate from the pool created above
allocInfo.commandPool = commandPool;
// a primary buffer can be submitted to a queue
allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
// allocate exactly one buffer
allocInfo.commandBufferCount = 1;

// handle that Vulkan will fill in
VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

// allocate the command buffer
result = vkAllocateCommandBuffers(
    device,
    &allocInfo,
    &commandBuffer);

// bail out if allocation failed
if (result != VK_SUCCESS)
    return 1;
```

## Begin recording

Recording happens in three phases: begin, record, end.

```
// the create-info struct for begin
VkCommandBufferBeginInfo beginInfo{};
// identify the begin-info type
beginInfo.sType =
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

// move the buffer into the recording state
result = vkBeginCommandBuffer(
    commandBuffer,
    &beginInfo);

// bail out if begin failed
if (result != VK_SUCCESS)
    return 1;
```

## Record a command

Commands recorded here do not execute until the buffer is submitted.

```
// record a draw of 3 vertices as one instance
vkCmdDraw(
    commandBuffer,
    3,   // vertex count
    1,   // instance count
    0,   // first vertex
    0);  // first instance
```

## Finish recording

Ending closes the recording session.

```
// leave the recording state
result = vkEndCommandBuffer(commandBuffer);

// bail out if ending failed
if (result != VK_SUCCESS)
    return 1;
```

## Describe the submission

The submit-info points at the buffer to hand to the queue.

```
// the create-info struct for the submission
VkSubmitInfo submitInfo{};
// identify the submit-info type
submitInfo.sType =
    VK_STRUCTURE_TYPE_SUBMIT_INFO;
// submit one command buffer
submitInfo.commandBufferCount = 1;
// pointer to the recorded buffer
submitInfo.pCommandBuffers = &commandBuffer;

// place the command buffer into the graphics queue
result = vkQueueSubmit(
    graphicsQueue,
    1,
    &submitInfo,
    VK_NULL_HANDLE);

// bail out if the submission failed
if (result != VK_SUCCESS)
    return 1;
```

## Wait for completion

This example blocks the CPU until the queue finishes all work.

```
// block until the graphics queue is idle
vkQueueWaitIdle(graphicsQueue);
```

## Clean up

Command buffers are freed through the pool that owns them.

```
// free the command buffer back to the pool
vkFreeCommandBuffers(
    device,
    commandPool,
    1,
    &commandBuffer);

// destroy the pool itself
vkDestroyCommandPool(
    device,
    commandPool,
    nullptr);
```

## Now type it again

Type the core recording sequence again.

```
// the create-info struct for begin
VkCommandBufferBeginInfo beginInfo{};
// identify the begin-info type
beginInfo.sType =
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

// move the buffer into the recording state
vkBeginCommandBuffer(
    commandBuffer,
    &beginInfo);

// record a draw of 3 vertices as one instance
vkCmdDraw(
    commandBuffer,
    3,   // vertex count
    1,   // instance count
    0,   // first vertex
    0);  // first instance

// leave the recording state
vkEndCommandBuffer(commandBuffer);

// the create-info struct for the submission
VkSubmitInfo submitInfo{};
// identify the submit-info type
submitInfo.sType =
    VK_STRUCTURE_TYPE_SUBMIT_INFO;
// submit one command buffer
submitInfo.commandBufferCount = 1;
// pointer to the recorded buffer
submitInfo.pCommandBuffers = &commandBuffer;

// place the command buffer into the graphics queue
vkQueueSubmit(
    graphicsQueue,
    1,
    &submitInfo,
    VK_NULL_HANDLE);
```

## Wrap up

The flow: pool -> allocate -> begin -> record -> end -> submit -> execute.
