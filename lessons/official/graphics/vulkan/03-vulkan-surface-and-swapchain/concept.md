# Vulkan surface and swapchain - concepts

Vulkan does not create or manage a window. A window belongs to the operating
system or to a windowing library such as GLFW.

Vulkan needs a way to connect that external window to its presentation system.
That connection is represented by a VkSurfaceKHR.

The surface is therefore not the window itself. It is Vulkan's representation
of a place where rendered images can eventually be presented.

## The window and surface

A windowing library creates the actual window:

```
GLFWwindow* window = glfwCreateWindow(
    1280, 720, "Vulkan", nullptr, nullptr);
```

Vulkan then creates a surface associated with that window:

```
VkSurfaceKHR surface;
```

The exact surface creation mechanism depends on the operating system. GLFW
hides those platform differences and creates the appropriate Vulkan surface.

The relationship is:

```
operating system
      |
      v
    window
      |
      v
VkSurfaceKHR
      |
      v
Vulkan presentation
```

The surface does not contain the rendered image. It describes where Vulkan can
present images.

## Why the surface matters

A physical device may support graphics operations without supporting
presentation to a particular surface.

This means the application must consider two separate capabilities.

A queue family may support graphics:

```
VK_QUEUE_GRAPHICS_BIT
```

But presentation support is checked against the specific surface.

```
vkGetPhysicalDeviceSurfaceSupportKHR(
    physicalDevice,
    queueFamily,
    surface,
    &presentSupported);
```

The result tells us whether that queue family can present images to this
surface.

This matters because graphics and presentation are related but distinct
operations.

## The swapchain

The surface tells Vulkan where an image can be presented. The swapchain manages
a collection of images that can be presented there.

Conceptually:

```
swapchain
   |
   +-- image 0
   +-- image 1
   +-- image 2
```

While the GPU renders into one image, another image may be available for
presentation or preparation.

The application acquires an available image:

```
vkAcquireNextImageKHR(
    device,
    swapchain,
    timeout,
    imageAvailable,
    VK_NULL_HANDLE,
    &imageIndex);
```

The returned imageIndex identifies which swapchain image the application should
render into.

## Choosing swapchain properties

A surface exposes capabilities that constrain the swapchain.

The application can query them with:

```
VkSurfaceCapabilitiesKHR capabilities;

vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
    physicalDevice,
    surface,
    &capabilities);
```

These capabilities include the minimum and maximum number of images, supported
image dimensions, and transformations.

The surface also exposes supported formats:

```
uint32_t formatCount = 0;

vkGetPhysicalDeviceSurfaceFormatsKHR(
    physicalDevice,
    surface,
    &formatCount,
    nullptr);
```

A format describes how pixels are represented, including their color format
and color space.

The application also chooses a present mode.

A present mode controls how completed images are handed to the presentation
system. Common modes include FIFO and MAILBOX.

FIFO is guaranteed to be available and behaves similarly to a synchronized
display queue. MAILBOX can provide lower-latency behavior when supported.

## Creating the swapchain

The chosen properties are placed into VkSwapchainCreateInfoKHR:

```
VkSwapchainCreateInfoKHR info{};

info.sType =
    VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
info.surface = surface;
info.minImageCount = imageCount;
info.imageFormat = surfaceFormat.format;
info.imageColorSpace = surfaceFormat.colorSpace;
info.imageExtent = extent;
info.imageArrayLayers = 1;
info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
info.presentMode = presentMode;
info.clipped = VK_TRUE;
```

The swapchain is then created:

```
vkCreateSwapchainKHR(
    device,
    &info,
    nullptr,
    &swapchain);
```

The swapchain now owns a collection of presentation images.

## Acquiring and presenting

Rendering starts by acquiring an available swapchain image.

The application records commands that render into that image, then submits
those commands to a queue.

After rendering completes, the image is presented:

```
VkPresentInfoKHR presentInfo{};

presentInfo.sType =
    VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
presentInfo.swapchainCount = 1;
presentInfo.pSwapchains = &swapchain;
presentInfo.pImageIndices = &imageIndex;

vkQueuePresentKHR(
    presentQueue,
    &presentInfo);
```

The simplified frame flow is:

```
acquire image
    |
    v
record commands
    |
    v
submit rendering
    |
    v
present image
```

Synchronization connects these stages so that the image is not presented
before rendering has finished.

## Swapchain recreation

A swapchain depends on the window's size and the surface's capabilities.

When a window is resized, the existing swapchain may no longer match the
surface. The application then needs to recreate it.

This is why swapchain creation is normally isolated from the rest of the
rendering system.

The swapchain is not the renderer. It is the mechanism that supplies images
which can move between rendering and presentation.

## Next step

Now type the code version of this lesson.

