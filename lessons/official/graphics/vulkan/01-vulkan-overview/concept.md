# Vulkan overview - concepts

Vulkan is a graphics and compute API designed to give an application explicit
control over work submitted to a GPU.

Older graphics APIs often hide much of the work performed by the driver. Vulkan
moves more of that responsibility into the application. This makes Vulkan more
verbose, but it also makes the cost and ordering of GPU work much more visible.

The important idea is that Vulkan is not a collection of functions for drawing
things. It is a system for describing work, recording that work, and submitting
it to a GPU.

## The GPU is a separate processor

A modern GPU is not simply another set of CPU instructions. It is a separate
processor with its own execution resources and memory.

Your application runs primarily on the CPU. It creates Vulkan objects and
prepares commands. The GPU eventually executes those commands.

The driver sits between your application and the hardware:

```
application
    |
    v
Vulkan API
    |
    v
Vulkan driver
    |
    v
GPU
```

Vulkan standardizes the interface between the application and the driver. The
driver translates Vulkan operations into work appropriate for the hardware.

## Vulkan objects describe state

Vulkan represents important pieces of the rendering system as explicit objects.
An object is usually represented by a handle that refers to state managed by
Vulkan.

For example, an instance is represented by a VkInstance handle:

```
VkInstance instance;
```

The handle itself is not the Vulkan instance's data. It is a reference that
your application uses when interacting with that object.

Other lessons will introduce the objects in detail. For now, the important idea
is that Vulkan makes the major pieces of the GPU interface explicit.

## Work is recorded before it is submitted

Vulkan separates describing work from submitting that work.

A command buffer can contain commands such as drawing, copying an image, or
changing GPU state:

```
vkCmdDraw(commandBuffer, 3, 1, 0, 0);
```

This call does not mean that the GPU immediately executes the draw. It records
a command into the command buffer.

The application can later submit that command buffer to a queue:

```
vkQueueSubmit(queue, 1, &submitInfo, fence);
```

This separation is one of Vulkan's central ideas.

The CPU prepares work. The application records that work into command buffers.
A queue then receives those command buffers for execution by the GPU.

## Queues represent execution

A Vulkan device exposes one or more queues. Different queues can support
different kinds of work, such as graphics, compute, or transfer operations.

Conceptually, a queue is a submission point for GPU work:

```
command buffers
      |
      v
    queue
      |
      v
     GPU
```

The application does not simply call a drawing function and wait for the GPU
to finish it. Instead, it builds work and submits that work to a queue.

This makes synchronization important. The application must explicitly describe
when GPU operations may begin and which operations must wait for others.

## A frame is a chain of work

Rendering one frame is not one operation. It is a sequence of operations.

A simplified frame looks like this:

```
acquire image
    |
    v
record rendering commands
    |
    v
submit commands to graphics queue
    |
    v
GPU executes rendering
    |
    v
present rendered image
```

The swapchain provides images that can be presented to the window. The
application acquires an available image, renders into it, and eventually asks
the presentation system to display it.

Later lessons will turn each part of this sequence into actual Vulkan code.

## Why Vulkan is verbose

Vulkan asks the application to make decisions that other APIs may make
implicitly.

The application chooses devices, queue families, formats, synchronization
objects, memory types, pipeline state, and many other details.

That verbosity is intentional.

It allows the application to know more about what the GPU is being asked to do.
It also reduces the amount of hidden work that a driver needs to perform at
runtime.

The tradeoff is that Vulkan requires a much stronger understanding of how the
CPU, driver, GPU, memory, commands, and synchronization interact.

## The mental model

The most useful starting model is:

```
create objects
    |
    v
prepare resources
    |
    v
record commands
    |
    v
submit to queues
    |
    v
synchronize execution
    |
    v
present the result
```

Vulkan is fundamentally about controlling this flow explicitly.

## Next step

Now type the code version of this lesson.

