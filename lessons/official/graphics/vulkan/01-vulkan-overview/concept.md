# Vulkan overview - concepts

Vulkan is a low-level graphics and compute API built around explicit control over GPU work. The important idea is not that Vulkan has many structures. The important idea is that the application constructs an execution system and then gives the GPU explicitly prepared work.

A useful first model is:

```text
application -> Vulkan objects -> recorded commands -> queue -> GPU
```

The objects describe what the application has available. Command buffers describe work that can be executed. Queues are where that work is submitted. The GPU executes the submitted commands asynchronously with the CPU.

## Vulkan starts with an instance

A Vulkan application normally begins by creating a `VkInstance`. The instance establishes the application's connection to the Vulkan implementation and provides the starting point for discovering physical devices.

```cpp
VkInstanceCreateInfo createInfo{};
createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
```

The structure is configuration data for the creation call. Vulkan uses many structures this way: the structure describes what should be created, and the API call creates the object represented by a handle.

The instance is therefore not the GPU and it is not a command queue. It is the root context through which the application begins interacting with Vulkan.

## Physical devices describe available GPUs

Once an instance exists, the application can enumerate `VkPhysicalDevice` handles. A physical device represents hardware exposed by the Vulkan implementation.

The application can inspect its properties and capabilities before choosing it. This is where decisions such as device preference, supported Vulkan features, queue families, memory types, and limits begin.

```cpp
std::vector<VkPhysicalDevice> physicalDevices(count);
vkEnumeratePhysicalDevices(instance, &count, physicalDevices.data());
```

The important distinction is that enumeration is discovery. Nothing about this step means that the application has created its usable device interface yet.

A machine may expose several physical devices. Vulkan gives the application enough information to choose one instead of silently making that decision on its behalf.

## The logical device is the application interface

After choosing a physical device, the application creates a `VkDevice`. The logical device is the application's usable interface to that selected GPU.

This creates an important relationship:

```text
VkInstance
    |
    v
VkPhysicalDevice
    |
    v
VkDevice
```

The physical device answers what hardware can do. The logical device represents what the application has requested to use from that hardware.

Device creation can request queues and enable supported features. It is therefore both a creation step and a declaration of which parts of the physical device the application intends to use.

## Queue families describe execution capability

A GPU does not expose one universal queue through which every operation must pass. A physical device exposes queue families, and each family advertises supported operations through queue flags.

For example, a family can advertise graphics support with `VK_QUEUE_GRAPHICS_BIT`.

```cpp
if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
    graphicsFamily = index;
}
```

The flag describes capability. It does not execute a graphics command and it does not mean that a queue is currently busy.

A queue family can expose graphics, compute, transfer, or combinations of those capabilities. A real renderer may select several families when that provides a useful execution arrangement.

For the first mental model, the important chain is:

```text
physical device -> queue family -> logical device -> queue
```

The application discovers the family first, requests an appropriate queue during device creation, and then retrieves the resulting `VkQueue` handle.

## Queues execute submitted work

A `VkQueue` is a submission destination. The CPU prepares work and submits it to a queue; the GPU can then execute that work.

This is different from calling a function that immediately performs the operation on the CPU. A submission is a request to place previously prepared GPU work into an execution stream.

```cpp
vkQueueSubmit(queue, 1, &submitInfo, fence);
```

The submission call connects the application's prepared command buffers to a queue. It does not mean that every GPU instruction has completed when the function returns.

This distinction is fundamental because CPU and GPU execution overlap. The CPU can continue preparing later work while earlier work is executing on the GPU, provided synchronization rules are respected.

## Command buffers record work

Vulkan normally does not issue rendering work directly from the application into the GPU one operation at a time. Commands are recorded into `VkCommandBuffer` objects and later submitted to a queue.

A simplified recording sequence is:

```cpp
vkBeginCommandBuffer(commandBuffer, &beginInfo);
vkCmdDraw(commandBuffer, 3, 1, 0, 0);
vkEndCommandBuffer(commandBuffer);
```

The draw call here records a command. It does not mean that the GPU has already rasterized three vertices.

The stages are separate:

```text
record -> submit -> execute
```

Recording lets the application construct a sequence of GPU operations before execution. Submission then makes that recorded work available to a queue.

This separation is one of the reasons Vulkan can expose command organization more explicitly than a higher-level API. The application can decide when work is built, how it is grouped, and when it is submitted.

## Command pools provide allocation context

Command buffers are normally allocated from a `VkCommandPool`. The pool belongs to a logical device and is associated with a queue family.

```cpp
VkCommandPoolCreateInfo poolInfo{};
poolInfo.queueFamilyIndex = graphicsFamily;
```

The command pool is not itself the recorded work. It provides the allocation and management context for command buffers.

The relationship is:

```text
logical device
    |
    v
command pool
    |
    +-> command buffer
    +-> command buffer
```

This distinction matters because Vulkan contains many objects whose roles are deliberately narrow. A pool manages command-buffer allocation; a command buffer records commands; a queue accepts submissions.

## Resources are separate from commands

GPU commands need data to operate on. Vulkan represents that data with resource objects such as `VkBuffer` and `VkImage`.

A buffer can contain vertex data, indices, uniform data, storage data, or transfer data. An image can represent a texture, a color target, a depth image, or another image resource.

```cpp
VkBuffer buffer = VK_NULL_HANDLE;
VkImage image = VK_NULL_HANDLE;
```

The handles represent resources, not the commands that use them. A command buffer can contain an operation that reads from a buffer, writes to an image, copies between resources, or binds a resource for later shader access.

This separation allows resources to have lifetimes independent of individual command sequences. A texture can be used by many command buffers, and a buffer can participate in many submissions over its lifetime.

## Memory backs resources

A resource object describes how a resource is used, but storage is handled through Vulkan's memory system. The application can allocate device memory and bind it to resources according to the device's memory properties.

A simplified model is:

```text
resource -> bound memory -> physical storage
```

This is one of Vulkan's most important examples of explicit control. A higher-level API may hide where a resource is placed. Vulkan exposes enough information for the application or an allocator to make informed decisions about memory.

The physical device reports memory types and heaps. Their properties affect how the CPU and GPU can access the memory and therefore influence how resources should be allocated.

The exact allocation strategy is more complicated than the diagram suggests, but the conceptual division is enough for now: the resource describes the usable object, while memory provides its backing storage.

## Pipelines describe execution state

A graphics command also needs to know how the GPU should interpret and process the work. Vulkan represents much of this configuration with pipeline objects.

A graphics pipeline includes shader stages and fixed-function state such as vertex input, rasterization, depth and stencil behavior, multisampling, and color blending.

```cpp
vkCmdBindPipeline(
    commandBuffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipeline
);
```

Binding the pipeline records the state that subsequent graphics commands use. The pipeline itself does not draw anything. A later draw command uses the bound state to describe how the GPU should process its vertices and fragments.

A useful distinction is:

```text
pipeline -> how graphics work is configured
command  -> what operation should be performed
resource -> what data the operation uses
```

These three ideas will repeatedly appear together in later lessons.

## CPU and GPU work asynchronously

Vulkan's explicit model becomes especially important when considering time. The CPU records and submits work, while the GPU executes that work later and potentially in parallel with the CPU.

Suppose the CPU submits a command that reads a buffer. If the CPU immediately overwrites that buffer for another frame, the two operations may conflict because the GPU may still be reading the original contents.

The application therefore needs synchronization to establish ordering and availability.

```text
CPU: record -> submit -> continue
                 |
                 v
GPU:             execute -> finish
```

A `VkFence` can allow the CPU to determine that a submitted operation has completed. Semaphores and other synchronization mechanisms can establish dependencies between GPU operations and submissions.

The names of these mechanisms are less important at this stage than the reason they exist: submitting work and completing work are different events.

## A frame is a chain, not a draw call

A real Vulkan frame combines the systems introduced above. A simplified windowed frame looks like this:

```text
acquire image
    -> record commands
    -> submit to queue
    -> GPU executes
    -> present image
```

The swapchain supplies images that can eventually be presented to a window. The application acquires an available image, records commands that render into it, submits those commands, waits on the required dependencies, and then presents the completed image.

Each arrow represents a relationship that Vulkan makes explicit. The image must be available before rendering uses it. Rendering must complete before presentation uses its result. A resource must not be modified while earlier GPU work still depends on it.

Later lessons will separate these responsibilities into swapchain management, command recording, synchronization, resources, and the frame loop.

## Why Vulkan has so many objects

Vulkan's large API can initially feel like a collection of arbitrary structures. It becomes more coherent when each object is viewed as an explicit representation of one part of the execution model.

The instance establishes the Vulkan environment. A physical device represents discoverable hardware. A logical device represents the application's connection to selected hardware. Queue families describe capabilities, queues accept submissions, command pools manage command-buffer allocation, command buffers record work, resources hold data, pipelines describe graphics state, and synchronization objects express dependencies.

The objects are therefore not the lesson's destination. They are vocabulary for describing a GPU execution system.

A useful question when encountering a new Vulkan object is: what responsibility or relationship does this object make explicit?

That question is more useful than memorizing structure names in isolation.

## The complete mental model

The entire overview can be compressed into one chain:

```text
instance
  -> physical device
  -> logical device
  -> queue
  -> command pool
  -> command buffer
  -> record
  -> submit
  -> GPU execution
```

Resources and pipelines become inputs to the recorded work, while synchronization controls when dependent work is allowed to proceed.

The application is responsible for constructing these pieces and maintaining their lifetimes. The Vulkan implementation remains responsible for translating the API operations to the underlying GPU, but the application exposes substantially more of the decisions that shape execution.

That is the central Vulkan tradeoff: more responsibility in exchange for more explicit control over GPU work, resources, scheduling, and synchronization.

## Next step

Now type the code version of this lesson.

