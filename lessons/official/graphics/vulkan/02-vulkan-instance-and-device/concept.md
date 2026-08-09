# Vulkan instance and device - concepts

A Vulkan instance connects the application to the Vulkan implementation. It is
the first major object created by a Vulkan program.

The instance does not represent the GPU itself. It gives the application access
to the Vulkan environment from which physical devices can be discovered.

The next step is to choose a physical device and create a logical device from
it.

## Physical and logical devices

A physical device represents an actual GPU available to Vulkan.

An application can ask Vulkan which physical devices are available:

```
uint32_t deviceCount = 0;
vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
```

The count tells us how many devices exist. We can then allocate an array and
ask Vulkan to fill it:

```
std::vector<VkPhysicalDevice> devices(deviceCount);
vkEnumeratePhysicalDevices(
    instance, &deviceCount, devices.data());
```

A VkPhysicalDevice is a handle describing a physical GPU. It is not created by
the application. Vulkan discovers it from the installed hardware and driver.

Choosing a physical device is therefore a decision about hardware.

## Device properties

Before choosing a GPU, the application can inspect its properties.

```
VkPhysicalDeviceProperties properties;
vkGetPhysicalDeviceProperties(device, &properties);
```

The properties contain information such as the device name, supported Vulkan
version, limits, and other capabilities.

For example, the application can inspect the device name:

```
std::cout << properties.deviceName << '\n';
```

This is useful because Vulkan does not assume that every GPU provides exactly
the same capabilities.

## Queue families

A GPU exposes queues through queue families.

A queue family describes a group of queues with particular capabilities.
Common capabilities include graphics, compute, and transfer operations.

The application asks the physical device for its queue families:

```
uint32_t queueFamilyCount = 0;

vkGetPhysicalDeviceQueueFamilyProperties(
    device,
    &queueFamilyCount,
    nullptr);
```

It can then retrieve their properties:

```
std::vector<VkQueueFamilyProperties> families(queueFamilyCount);

vkGetPhysicalDeviceQueueFamilyProperties(
    device,
    &queueFamilyCount,
    families.data());
```

Each VkQueueFamilyProperties contains a queueFlags field describing what that
family can do.

For graphics work, the relevant flag is:

```
VK_QUEUE_GRAPHICS_BIT
```

A suitable graphics queue family is therefore one whose flags contain this
bit.

## Choosing a queue family

Suppose the application finds a family that supports graphics:

```
uint32_t graphicsFamily = 0;
```

The index is important because the application will use it when creating the
logical device.

The physical device tells us what hardware exists. The queue family tells us
where a particular kind of work can be submitted.

The relationship is:

```
physical device
    |
    +-- queue family
            |
            +-- queue
            |
            +-- queue
```

## Creating the logical device

A logical device is the application's interface to the selected physical
device.

The application specifies which queue families it wants to use and which
features or extensions it needs.

First, describe the queue:

```
float priority = 1.0f;

VkDeviceQueueCreateInfo queueInfo{};
queueInfo.sType =
    VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
queueInfo.queueFamilyIndex = graphicsFamily;
queueInfo.queueCount = 1;
queueInfo.pQueuePriorities = &priority;
```

Then describe the device:

```
VkDeviceCreateInfo deviceInfo{};
deviceInfo.sType =
    VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
deviceInfo.queueCreateInfoCount = 1;
deviceInfo.pQueueCreateInfos = &queueInfo;
```

Finally, create it:

```
VkDevice logicalDevice = VK_NULL_HANDLE;

vkCreateDevice(
    device,
    &deviceInfo,
    nullptr,
    &logicalDevice);
```

The physical device is discovered. The logical device is created.

That distinction is fundamental to Vulkan.

## Getting a queue

Creating a logical device does not automatically give the application a queue
variable. The application retrieves a queue belonging to one of the selected
queue families:

```
VkQueue graphicsQueue = VK_NULL_HANDLE;

vkGetDeviceQueue(
    logicalDevice,
    graphicsFamily,
    0,
    &graphicsQueue);
```

The final argument identifies which queue within the family should be
retrieved.

The resulting queue is where command buffers can eventually be submitted.

## The complete relationship

The complete hierarchy is:

```
VkInstance
    |
    v
physical device
    |
    +-- queue family
    |       |
    |       v
    |     queue
    |
    v
logical device
```

The instance gives access to the Vulkan environment.

The physical device represents available hardware.

Queue families describe the kinds of work that hardware can accept.

The logical device creates the application's usable interface to that
hardware, including the queues it intends to use.

## Next step

Now type the code version of this lesson.

