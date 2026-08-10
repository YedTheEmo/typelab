# Vulkan synchronization - concepts

Vulkan does not assume that commands should wait for one another simply because
the application called them in a particular order on the CPU. The application
must explicitly describe the dependencies that matter to execution.

This is necessary because CPU execution, queue submission, GPU execution, and
presentation all proceed independently.

The central problem is therefore not simply "waiting". It is describing
relationships such as:

```text
operation A must finish before operation B begins
```

or:

```text
GPU work must not access a swapchain image until presentation has released it
```

Vulkan provides several synchronization mechanisms because these relationships
have different meanings. Fences communicate completion from the GPU to the
CPU. Semaphores order GPU operations and queue operations. Pipeline barriers
and related synchronization operations establish memory and execution
dependencies between commands.

## Synchronization is about dependencies

Consider two commands that use the same resource:

```text
command A -> writes image
command B -> reads image
```

If B executes before A's write is complete, B can observe invalid or incomplete
data.

The application therefore needs a dependency:

```text
command A
    |
    | dependency
    v
command B
```

Vulkan exposes this dependency explicitly instead of assuming that every
operation waits for every previous operation.

This explicit model allows unrelated work to overlap.

```text
A: ----write----
B:          ----read----

unrelated work:
C: --compute--
```

The goal is not to make everything execute sequentially. The goal is to make
only the required operations wait for one another.

## The CPU and GPU are independent

Submitting a command buffer does not mean the CPU waits for the GPU.

```cpp
vkQueueSubmit(
    graphicsQueue,
    1,
    &submitInfo,
    fence
);
```

The call submits work to the queue. The GPU may execute that work later while
the CPU continues executing application code.

This creates an important distinction:

```text
CPU:
submit -> continue running

GPU:
        receive -> execute
```

If the CPU needs to know whether the GPU has completed the submission, a fence
can provide that information.

## Fences communicate completion to the CPU

A `VkFence` is primarily a CPU-visible completion mechanism.

The application creates a fence:

```cpp
VkFenceCreateInfo fenceInfo{
    VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    nullptr,
    0
};
```

The fence can then be attached to a queue submission:

```cpp
vkQueueSubmit(
    graphicsQueue,
    1,
    &submitInfo,
    fence
);
```

When the submitted work completes, Vulkan signals the fence.

The CPU can wait for that state:

```cpp
vkWaitForFences(
    device,
    1,
    &fence,
    VK_TRUE,
    UINT64_MAX
);
```

The relationship is:

```text
CPU
 |
 | submit
 v
queue -> GPU work
           |
           | complete
           v
         fence
           |
           v
          CPU
```

The fence therefore answers a CPU-side question:

"Has this submitted work finished?"

## Fences have a state

A fence is either unsignaled or signaled.

A newly created fence is normally unsignaled:

```text
unsignaled
```

A queue submission associated with that fence causes Vulkan to signal it when
the submission completes:

```text
unsignaled -> GPU completes -> signaled
```

The CPU can wait for the signaled state.

Before reusing the same fence for another submission, it must be reset:

```cpp
vkResetFences(
    device,
    1,
    &fence
);
```

The lifecycle is therefore:

```text
create unsignaled
    ->
submit work
    ->
GPU completes
    ->
fence becomes signaled
    ->
CPU observes completion
    ->
reset fence
    ->
submit again
```

This state model becomes important when a renderer maintains several frames
that can be in flight simultaneously.

## Waiting on a fence is a CPU wait

A fence is useful precisely because it communicates with the CPU. Calling
`vkWaitForFences` can block the calling CPU thread until the GPU has reached the
corresponding completion point.

For example:

```cpp
vkWaitForFences(
    device,
    1,
    &renderFence,
    VK_TRUE,
    UINT64_MAX
);
```

This is useful when the application must reuse a resource that the GPU might
still be using.

However, waiting after every submission can destroy parallelism:

```text
submit
  |
  v
wait
  |
  v
submit
  |
  v
wait
```

The CPU and GPU become artificially serialized.

A renderer instead wants something closer to:

```text
CPU:  frame A ---- frame B ---- frame C ----
GPU:       frame A ---- frame B ---- frame C
```

Synchronization should enforce correctness without unnecessarily stopping
independent work.

## Binary semaphores communicate GPU ordering

A `VkSemaphore` can synchronize operations without requiring the CPU to wait.

A binary semaphore represents a signal that can be consumed by another GPU-side
operation.

The application creates one:

```cpp
VkSemaphoreCreateInfo semaphoreInfo{
    VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    nullptr,
    0
};
```

The semaphore can then participate in a submission.

Unlike a fence, the semaphore is not primarily intended to answer a CPU
question. It establishes ordering between operations handled by Vulkan.

The distinction is:

```text
fence:
GPU -> CPU

semaphore:
GPU -> GPU
```

This distinction is one of the most important synchronization concepts in
Vulkan.

## Image acquisition signals a semaphore

When acquiring a swapchain image, the application can provide a semaphore:

```cpp
vkAcquireNextImageKHR(
    device,
    swapchain,
    UINT64_MAX,
    imageAvailable,
    VK_NULL_HANDLE,
    &imageIndex
);
```

The semaphore becomes signaled when the acquired swapchain image is available
for the application to use.

The application can then make graphics submission wait on that semaphore.

The conceptual flow is:

```text
presentation system
        |
        | image available
        v
 imageAvailable semaphore
        |
        | graphics submission waits
        v
     rendering
```

This prevents rendering from accessing the image before the presentation
system has made it available.

## Waiting on a semaphore in a submission

A `VkSubmitInfo` can specify semaphores that must be signaled before the
submission proceeds.

For example:

```cpp
VkSubmitInfo submitInfo{
    VK_STRUCTURE_TYPE_SUBMIT_INFO,
    nullptr,
    1,
    &imageAvailable,
    &waitStage,
    1,
    &commandBuffer,
    1,
    &renderFinished
};
```

The `pWaitSemaphores` array contains the semaphore that must be signaled.

The corresponding `pWaitDstStageMask` array specifies the pipeline stage at
which the wait applies.

This is an important detail. The semaphore establishes an execution
dependency, while the stage mask specifies where the dependency affects the
submitted graphics work.

## Pipeline stages define where work occurs

Vulkan graphics commands do not execute as one indivisible operation. GPU work
passes through different pipeline stages.

Examples include:

```text
vertex input
    ->
vertex shader
    ->
rasterization
    ->
fragment shader
    ->
color output
```

Synchronization can therefore describe a dependency between particular stages.

For example:

```cpp
VkPipelineStageFlags waitStage =
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
```

This says that the submission should wait at the specified stage rather than
conceptually blocking every possible stage before it.

Modern Vulkan provides newer synchronization APIs with more precise stage and
access descriptions. The classic stage-mask model remains useful for
understanding the fundamental idea.

## The rendering-completion semaphore

After rendering has completed, presentation needs to know that the rendered
image is ready.

A second semaphore can represent that dependency:

```text
rendering
    |
    | finished
    v
renderFinished semaphore
    |
    | presentation waits
    v
present
```

The graphics submission signals the semaphore:

```cpp
VkSubmitInfo submitInfo{
    VK_STRUCTURE_TYPE_SUBMIT_INFO,
    nullptr,
    1,
    &imageAvailable,
    &waitStage,
    1,
    &commandBuffer,
    1,
    &renderFinished
};
```

The presentation operation then waits for the semaphore:

```cpp
VkPresentInfoKHR presentInfo{
    VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    nullptr,
    1,
    &renderFinished,
    1,
    &swapchain,
    &imageIndex,
    nullptr
};
```

The complete dependency is now:

```text
acquire
   |
   v
imageAvailable
   |
   v
render
   |
   v
renderFinished
   |
   v
present
```

The CPU does not need to wait between these stages.

## One semaphore has one signal-consumption relationship

A binary semaphore is intended to synchronize a signal with a corresponding
wait. It should not be treated as a general-purpose counter.

Conceptually:

```text
unsignaled
    ->
signal
    ->
signaled
    ->
wait consumes signal
    ->
unsignaled
```

This is different from a fence, whose signaled state can be queried or waited
on by the CPU and then explicitly reset.

The different state models are part of why Vulkan provides separate objects for
different synchronization relationships.

## The classic frame synchronization pattern

A simple Vulkan frame often uses two semaphores and one fence:

```text
imageAvailable
renderFinished
renderFence
```

Their roles differ:

```text
imageAvailable
    acquire -> graphics submission

renderFinished
    graphics submission -> presentation

renderFence
    graphics submission -> CPU
```

This gives the frame a complete dependency chain:

```text
             imageAvailable
                    |
                    v
acquire ------> graphics work ------> present
                    |
                    v
               renderFinished

                    |
                    v
               renderFence
                    |
                    v
                   CPU
```

The semaphore path keeps the GPU-side operations ordered. The fence tells the
CPU when it is safe to reuse resources associated with that frame.

## Why one fence can represent a frame

Suppose a frame owns a command buffer.

The CPU records the command buffer and submits it:

```text
frame 0
  |
  +-> command buffer
  +-> submission
  +-> fence
```

The fence becomes signaled when the submitted work completes.

Before recording new commands into the same command buffer, the application
can wait for that fence:

```cpp
vkWaitForFences(
    device,
    1,
    &frameFence,
    VK_TRUE,
    UINT64_MAX
);
```

This establishes:

```text
old GPU use
    ->
fence signals
    ->
CPU knows it is complete
    ->
command buffer can be reused
```

Without this dependency, the CPU could reset or overwrite a command buffer
while the GPU is still reading it.

## Multiple frames in flight

A renderer does not have to wait for every frame to finish before beginning the
next one.

Instead, it can maintain several frame contexts:

```text
frame 0
    command buffer
    imageAvailable
    renderFinished
    fence

frame 1
    command buffer
    imageAvailable
    renderFinished
    fence
```

The CPU can prepare frame 1 while the GPU is still processing frame 0.

The synchronization boundary becomes:

```text
CPU frame 0 -> GPU frame 0
CPU frame 1 -> GPU frame 1
```

The CPU only waits when it reaches a frame context whose previous GPU work has
not finished.

This is the basic reason frames-in-flight exist.

## A frame fence must be reset at the correct time

A common frame sequence is:

```text
wait for frame fence
reset frame fence
acquire image
submit work with frame fence
present
```

The wait ensures that the previous use of the frame's resources has finished.

The reset returns the fence to the unsignaled state so the next submission can
signal it.

```cpp
vkWaitForFences(
    device,
    1,
    &frameFence,
    VK_TRUE,
    UINT64_MAX
);

vkResetFences(
    device,
    1,
    &frameFence
);
```

The ordering matters because a reset fence is no longer a completion guarantee
until another submission associated with it finishes.

## The acquired image is a separate synchronization problem

A frame context and a swapchain image are not necessarily the same thing.

Suppose there are two frame contexts but three swapchain images. The frame
context currently being used can acquire any one of the swapchain images.

```text
frame context 0
       |
       v
swapchain image 2
```

The renderer therefore has to track both:

```text
which frame context is being reused
```

and:

```text
which swapchain image was acquired
```

These are different identities.

This distinction becomes important when multiple frames are allowed to overlap.

## Waiting on the device is the blunt solution

Vulkan provides:

```cpp
vkDeviceWaitIdle(device);
```

This waits until all previously submitted work on the device has completed.

It is extremely useful during shutdown and some resource-management operations.

However, it is too broad for normal per-frame synchronization.

Instead of:

```text
submit frame
    ->
device wait idle
    ->
submit next frame
```

a renderer normally waits only for the particular frame resources it needs to
reuse.

This allows unrelated GPU work to continue.

## Queue submission order is not enough

Commands submitted to the same queue have ordering rules, but that does not
remove the need to describe resource dependencies.

For example, a resource may be written by one operation and consumed by another.
The application must establish the relevant memory dependency so that the
second operation sees the correct contents.

This is where pipeline barriers enter the synchronization model.

```text
write resource
      |
      | memory dependency
      v
read resource
```

A semaphore can establish ordering between submissions, but resource visibility
and access dependencies may require pipeline synchronization inside command
buffers.

## Pipeline barriers

A pipeline barrier records a synchronization operation into a command buffer.

The classic form is:

```cpp
vkCmdPipelineBarrier(
    commandBuffer,
    srcStage,
    dstStage,
    0,
    0,
    nullptr,
    0,
    nullptr,
    1,
    &imageBarrier
);
```

The barrier establishes a dependency between source and destination stages.

The resource barrier describes the resource whose access is being controlled.

The conceptual model is:

```text
source stage
    |
    | execution + memory dependency
    v
destination stage
```

This is different from a fence, because the dependency remains within GPU
execution rather than stopping the CPU.

## Memory visibility matters

Execution ordering and memory visibility are related but distinct concepts.

Suppose a command writes an image:

```text
write
```

and a later command reads it:

```text
read
```

It is not enough to merely say that the read happens later. Vulkan also needs
the relevant memory writes to become visible to the later access.

A synchronization dependency therefore has two related concerns:

```text
execution dependency
    +
memory dependency
```

The exact access masks and pipeline stages depend on what the producer and
consumer operations are.

## Image layout transitions

Images have layouts that describe how they are intended to be used.

A swapchain image may need to transition between layouts as it moves through
the rendering and presentation lifecycle.

A classic image barrier can describe such a transition:

```cpp
VkImageMemoryBarrier imageBarrier{
    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    nullptr,
    0,
    0,
    oldLayout,
    newLayout,
    VK_QUEUE_FAMILY_IGNORED,
    VK_QUEUE_FAMILY_IGNORED,
    image,
    {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
};
```

The layout transition is part of the synchronization and resource-state model.

The exact synchronization2 API and modern image-layout practices will be used
where appropriate in later lessons, but the fundamental concept remains the
same: image usage changes must be made explicit.

## Synchronization is not ownership

A queue-family ownership transfer is another kind of dependency.

Suppose a resource is accessed by one queue family and then another:

```text
queue family A
      |
      v
    resource
      |
      v
queue family B
```

The application may need to perform an ownership transfer as part of the
resource synchronization.

This is distinct from simply waiting for GPU execution to finish.

The synchronization model therefore includes several dimensions:

```text
execution order
memory visibility
resource layout
queue ownership
CPU completion
```

Vulkan exposes these dimensions separately because they represent different
constraints.

## Timeline semaphores

Modern Vulkan also supports timeline semaphores.

Instead of representing a binary signaled or unsignaled state, a timeline
semaphore contains an increasing integer value.

Conceptually:

```text
value 0
  ->
value 1
  ->
value 2
  ->
value 3
```

An operation can wait for a particular value:

```text
wait until semaphore >= 5
```

This can simplify more complex dependency graphs.

Timeline semaphores are especially useful when many asynchronous operations
need to share a single monotonically increasing synchronization sequence.

Binary semaphores remain useful for the classic acquire-render-present
pattern, while timeline semaphores provide a more general synchronization
primitive.

## Synchronization should describe the real dependency

A useful rule is to ask:

```text
What operation produces this resource?
What operation consumes it?
Where must they be ordered?
When does the CPU need to know about completion?
```

The answer determines which synchronization mechanism is appropriate.

For example:

```text
GPU completion -> CPU
```

suggests a fence.

```text
GPU operation -> GPU operation
```

suggests a semaphore or an in-command-buffer synchronization dependency.

```text
resource write -> resource read
```

suggests a memory and execution dependency such as a pipeline barrier.

These are different problems even though all of them are casually described as
"waiting".

## The frame synchronization chain

A basic frame can now be understood as a dependency graph:

```text
wait for previous frame
        |
        v
     acquire
        |
        v
 imageAvailable
        |
        v
 command submission
        |
        +-------> renderFinished
        |               |
        v               v
     render          present
        |
        v
    renderFence
        |
        v
   CPU can reuse
```

The semaphores keep GPU operations ordered. The fence communicates completion
back to the CPU.

The command buffer from Lesson 4 remains the unit of recorded work, while the
synchronization objects determine when that work may safely proceed.

## Synchronization and performance

Correct synchronization is necessary, but excessive synchronization can make a
renderer unnecessarily slow.

A dependency such as:

```text
everything waits for everything
```

removes the parallelism that Vulkan is designed to expose.

A better dependency graph looks like:

```text
A -----> B
 \
  \----> C

D ----------------> E
```

Only the operations that actually depend on one another are ordered.

This is the central performance principle of explicit synchronization:
synchronize enough to establish correctness, but no more than the dependency
requires.

## The complete model

The Vulkan execution model now has several connected layers:

```text
command pool
    |
    v
command buffer
    |
    v
queue submission
    |
    +-> waits on semaphores
    |
    +-> executes GPU commands
    |
    +-> signals semaphores
    |
    +-> signals fence
```

Within the command buffer, barriers can establish resource-level dependencies.

Outside the command buffer, semaphores can connect GPU operations and fences
can communicate completion to the CPU.

The next lesson moves from execution ordering to the resources being ordered:
buffers, images, memory allocation, and the relationship between Vulkan
resources and device memory.

## Next step

Now type the code version of this lesson.
````

