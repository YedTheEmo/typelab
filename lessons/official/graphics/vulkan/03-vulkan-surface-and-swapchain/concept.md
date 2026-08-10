# Vulkan surface and swapchain - concepts

A Vulkan device can execute GPU work without knowing anything about a window.
Presentation is a separate concern. To display rendered images, the application
must connect Vulkan to a window-system surface and then create a swapchain that
provides images suitable for presentation.

The important relationship is:

```cpp
window -> surface -> swapchain -> presentable images
```

The surface represents the destination provided by the window system. The
swapchain manages a collection of images that can be acquired, rendered into,
and eventually presented.

## The surface is the window boundary

A `VkSurfaceKHR` represents a platform-specific presentation target. It is not
an image and it is not the swapchain itself. It is the Vulkan representation of
where images will eventually be presented.

The surface is created using an instance-level extension:

```cpp
VkSurfaceKHR surface = VK_NULL_HANDLE;
```

The exact creation function depends on the operating system and windowing
library. GLFW, SDL, and native platform APIs can create the appropriate Vulkan
surface for a window.

The important architectural boundary is:

```text
window system
     |
     v
VkSurfaceKHR
     |
     v
Vulkan presentation
```

The surface therefore connects two systems without becoming a rendering
resource itself.

## Surface support belongs to a queue family

A graphics-capable queue is not automatically capable of presenting to every
surface. Presentation support is checked for a particular physical device,
queue family, and surface.

```cpp
VkBool32 presentSupport = VK_FALSE;

vkGetPhysicalDeviceSurfaceSupportKHR(
    physicalDevice,
    queueFamily,
    surface,
    &presentSupport
);
```

The result answers whether queues from that family can present to the selected
surface.

This matters because queue families describe capabilities, and presentation
capability can depend on the surface itself. A renderer may therefore need a
graphics queue and a presentation queue from the same family or from separate
families.

The earlier device-selection model becomes:

```text
physical device
    |
    +-> graphics queue family
    |
    +-> presentation support
```

For a simple renderer, using one queue family that supports both operations is
convenient. More complex applications can use separate queues when necessary.

## Surface capabilities

Before creating a swapchain, the application asks the surface what it can
support.

```cpp
VkSurfaceCapabilitiesKHR capabilities{};

vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
    physicalDevice,
    surface,
    &capabilities
);
```

The capabilities describe constraints such as the minimum and maximum number
of swapchain images, the supported image extent, and the transforms that can be
applied to images.

These values are important because the application does not have unrestricted
choice. A requested swapchain must satisfy the surface's capabilities.

The basic relationship is:

```text
surface capabilities -> valid swapchain configuration
```

The application therefore queries the surface before deciding how its
swapchain should be configured.

## Surface formats

A swapchain image needs a format. The format determines how the image's
components are represented.

The application can enumerate the formats supported by the surface:

```cpp
uint32_t formatCount = 0;

vkGetPhysicalDeviceSurfaceFormatsKHR(
    physicalDevice,
    surface,
    &formatCount,
    nullptr
);
```

The second call retrieves the actual choices:

```cpp
std::vector<VkSurfaceFormatKHR> formats(formatCount);

vkGetPhysicalDeviceSurfaceFormatsKHR(
    physicalDevice,
    surface,
    &formatCount,
    formats.data()
);
```

A `VkSurfaceFormatKHR` combines an image format with a color-space choice.

For a normal windowed renderer, the application commonly prefers a standard
color format such as `VK_FORMAT_B8G8R8A8_SRGB`, but it must verify that the
chosen format is actually supported.

The format is therefore a negotiated property of the presentation system, not
simply an arbitrary image format selected by the renderer.

## Present modes

The surface also exposes presentation modes. A present mode controls how
completed swapchain images are scheduled for presentation.

```cpp
uint32_t presentModeCount = 0;

vkGetPhysicalDeviceSurfacePresentModesKHR(
    physicalDevice,
    surface,
    &presentModeCount,
    nullptr
);
```

The choices include modes such as FIFO and mailbox. They differ in how they
handle synchronization with the display and whether newer frames can replace
older queued frames.

`VK_PRESENT_MODE_FIFO_KHR` is widely available and behaves like a queued,
display-synchronized presentation mode. `VK_PRESENT_MODE_MAILBOX_KHR` can
provide lower-latency behavior when supported by the platform.

The important point is that present mode affects the relationship between the
application's rendering rate and the display's presentation schedule.

## Choosing the swapchain extent

The swapchain extent represents the dimensions of its images.

```cpp
VkExtent2D extent = capabilities.currentExtent;
```

Some platforms provide a fixed current extent. Other platforms allow the
application to choose an extent within the reported minimum and maximum
bounds.

The selected extent must correspond to the window's drawable size rather than
being treated as an arbitrary renderer preference.

Window resizing makes this especially important. A swapchain created for one
window size may no longer be appropriate after the window changes dimensions.

This is why swapchain recreation becomes part of the frame architecture later.

## Choosing the image count

A swapchain contains multiple images so that the application can work with one
image while other images are being displayed or prepared.

The minimum image count comes from the surface:

```cpp
uint32_t imageCount = capabilities.minImageCount + 1;
```

Requesting one more image than the minimum is a common starting point, although
the implementation may impose a maximum:

```cpp
if (capabilities.maxImageCount > 0 &&
    imageCount > capabilities.maxImageCount) {
    imageCount = capabilities.maxImageCount;
}
```

The exact number of images influences how much work can remain in flight.
The swapchain is therefore not merely a container of duplicate images; it is
part of the mechanism that allows rendering and presentation to overlap.

## Creating the swapchain

Once the surface properties have been examined, the application constructs a
`VkSwapchainCreateInfoKHR`.

```cpp
VkSwapchainCreateInfoKHR swapchainInfo{
    VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    nullptr,
    0,
    surface,
    imageCount,
    format.format,
    format.colorSpace,
    extent,
    1,
    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    VK_SHARING_MODE_EXCLUSIVE,
    0,
    nullptr,
    capabilities.currentTransform,
    VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    presentMode,
    VK_TRUE,
    VK_NULL_HANDLE
};
```

The structure connects the surface, image configuration, usage, sharing mode,
transform, and presentation behavior into one swapchain request.

The image usage tells Vulkan what operations the application intends to perform
with the swapchain images. A graphics renderer normally needs them as color
attachments.

The sharing mode describes how queue families access the images. Exclusive
sharing is appropriate when one queue family owns the images for the relevant
operations.

## The swapchain owns its images

Creating the swapchain produces a collection of images managed by the
presentation system.

```cpp
VkSwapchainKHR swapchain = VK_NULL_HANDLE;

vkCreateSwapchainKHR(
    device,
    &swapchainInfo,
    nullptr,
    &swapchain
);
```

The application can retrieve the swapchain's images:

```cpp
uint32_t swapchainImageCount = 0;

vkGetSwapchainImagesKHR(
    device,
    swapchain,
    &swapchainImageCount,
    nullptr
);
```

Then the actual image handles are retrieved:

```cpp
std::vector<VkImage> swapchainImages(swapchainImageCount);

vkGetSwapchainImagesKHR(
    device,
    swapchain,
    &swapchainImageCount,
    swapchainImages.data()
);
```

These are `VkImage` resources, but their lifetime is tied to the swapchain.
The application does not destroy them individually.

This is different from ordinary images created with `vkCreateImage`, where the
application controls the image object's lifetime directly.

## Image views describe image interpretation

Rendering generally uses image views rather than directly binding the image
handle everywhere.

A `VkImageView` describes how an image is interpreted when used by a particular
operation. It specifies the image type, format interpretation, and subresource
range.

```cpp
VkImageViewCreateInfo viewInfo{
    VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    nullptr,
    0,
    swapchainImages[index],
    VK_IMAGE_VIEW_TYPE_2D,
    format.format,
    {},
    {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
};
```

The underlying image is the storage resource. The view provides a particular
way for Vulkan commands and rendering state to refer to that resource.

The relationship is:

```text
swapchain image
      |
      v
  image view
      |
      v
rendering operations
```

This distinction becomes important later when textures and depth images are
introduced.

## Acquiring a swapchain image

Before rendering into a swapchain image, the application acquires one from the
swapchain.

```cpp
uint32_t imageIndex = 0;

vkAcquireNextImageKHR(
    device,
    swapchain,
    UINT64_MAX,
    imageAvailable,
    VK_NULL_HANDLE,
    &imageIndex
);
```

The returned index identifies which swapchain image the application can use.

The semaphore in this call is part of synchronization and tells later GPU work
when the image is available. A complete synchronization strategy belongs to
the next lessons, so this call should currently be understood as an acquisition
operation that participates in the larger frame sequence.

The conceptual flow is:

```text
swapchain -> acquire -> image index -> render -> present
```

## Presenting an image

After rendering has completed, the application presents the acquired image
through a queue that supports presentation.

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

The presentation request identifies the swapchain and the image index that
should be displayed.

The queue then receives the presentation request:

```cpp
vkQueuePresentKHR(presentQueue, &presentInfo);
```

Presentation is therefore a queue operation, but it is not the same thing as
rendering. Rendering produces or modifies an image. Presentation transfers the
completed image into the window-system presentation mechanism.

## Swapchain recreation

A swapchain is tied to the surface's current conditions. Window resizing,
surface changes, or certain presentation results can make the existing
swapchain unsuitable.

The renderer must then recreate it.

The simplified lifecycle is:

```text
create swapchain
    -> acquire images
    -> render
    -> present
    -> detect change
    -> destroy dependent objects
    -> recreate swapchain
```

Objects such as swapchain image views and framebuffers or rendering state may
depend on the swapchain's image format or extent. They therefore often need to
be recreated alongside the swapchain.

This is one reason the final renderer architecture cannot treat swapchain
creation as a one-time operation.

## The complete presentation model

The surface and swapchain add a presentation path to the device and queue
model from the previous lesson:

```text
instance
    -> physical device
    -> logical device
    -> queue
    -> surface
    -> swapchain
    -> swapchain images
    -> acquire
    -> render
    -> present
```

The surface establishes where presentation can occur. The swapchain provides
the images used for that presentation. The queue performs the submission and
presentation operations.

The next lesson focuses on the command buffers that actually record the work
performed between image acquisition and presentation.

## Next step

Now type the code version of this lesson.
