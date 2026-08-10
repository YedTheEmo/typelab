# Vulkan instance and device - concepts

Vulkan does not begin by choosing a GPU and immediately issuing commands.
The application first creates a `VkInstance`, uses it to discover the
physical devices available through the Vulkan implementation, examines those
devices, and then creates a `VkDevice` representing the hardware interface it
actually intends to use.

This lesson turns the overview's abstract chain into a concrete initialization
process:

```cpp
instance -> physical device -> logical device -> queue
```

The important distinction is that each stage answers a different question.
The instance establishes the Vulkan environment, the physical device describes
what hardware is available, and the logical device is the application's
usable interface to the selected hardware.

## Creating the instance

`VkInstance` is the first major Vulkan handle created by an application. It
represents an application-level connection to a Vulkan implementation and is
the object through which physical devices can be enumerated.

Creation is driven by `VkApplicationInfo` and `VkInstanceCreateInfo`. The first
describes the application and requested API version. The second describes the
instance configuration, including enabled layers and extensions.

```cpp
VkApplicationInfo applicationInfo{
    VK_STRUCTURE_TYPE_APPLICATION_INFO,
    nullptr,
    "Typelab",
    VK_MAKE_VERSION(1, 0, 0),
    "Typelab",
    VK_MAKE_VERSION(1, 0, 0),
    VK_API_VERSION_1_3
};
```

`VkApplicationInfo` is optional configuration metadata rather than a GPU
object. The `sType` field identifies the structure to Vulkan, while `pNext`
allows Vulkan's extensible structure system to attach additional information.

The instance creation structure then points at that application information:

```cpp
VkInstanceCreateInfo instanceInfo{
    VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    nullptr,
    0,
    &applicationInfo,
    0,
    nullptr,
    0,
    nullptr
};
```

The `enabledLayerCount` and `ppEnabledLayerNames` fields select instance layers.
The extension fields select instance extensions. These are not arbitrary
features that can simply be requested: the implementation must expose the
requested layer or extension, so applications normally enumerate and validate
them before creating the instance.

The instance therefore establishes the API context, but it does not select a
particular GPU.

## Enumerating physical devices

With an instance available, the application can ask Vulkan which physical
devices are exposed by the implementation.

```cpp
uint32_t deviceCount = 0;
vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
```

The first call asks for the number of devices. A second call fills an array
with the handles:

```cpp
std::vector<VkPhysicalDevice> devices(deviceCount);
vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
```

A `VkPhysicalDevice` is a handle to a physical GPU or another Vulkan-capable
device exposed by the implementation. It is a discovery and inspection
object; the application does not use it as the ordinary destination for
command submission.

A machine may expose an integrated GPU, a discrete GPU, or other Vulkan
devices. Vulkan deliberately leaves the selection decision to the application.

## Inspecting device properties

Enumeration only gives handles. The application must inspect each physical
device to decide whether it is appropriate.

`vkGetPhysicalDeviceProperties` returns properties such as the device name,
device type, API version, driver version, and implementation limits.

```cpp
VkPhysicalDeviceProperties properties{};
vkGetPhysicalDeviceProperties(physicalDevice, &properties);
```

The device type can be useful when ranking candidates. A discrete GPU might
be preferred for a demanding renderer, while an integrated GPU might be the
appropriate choice for a system where power consumption matters.

The properties also contain limits that later determine whether requested
resource sizes, descriptor counts, workgroup dimensions, and other values are
valid.

Properties describe what the device is and what limits it has. They do not
represent optional capabilities that the application can simply turn on.

## Inspecting supported features

Features answer a different question: which optional Vulkan capabilities does
the physical device support?

```cpp
VkPhysicalDeviceFeatures features{};
vkGetPhysicalDeviceFeatures(physicalDevice, &features);
```

A feature such as `samplerAnisotropy` may be supported by one device and absent
from another. The application must discover support before enabling the feature
during logical-device creation.

This creates an important distinction:

```text
properties -> what the device is and its limits
features   -> optional capabilities the device supports
extensions -> additional API interfaces it exposes
```

The application should not treat these categories as interchangeable. A
renderer chooses its requirements and then checks whether the selected
physical device can satisfy them.

## Queue families

A physical device also exposes queue families. A queue family represents a
set of queues with a particular set of supported operations.

```cpp
uint32_t queueFamilyCount = 0;
vkGetPhysicalDeviceQueueFamilyProperties(
    physicalDevice,
    &queueFamilyCount,
    nullptr
);
```

The second call retrieves the family descriptions:

```cpp
std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
vkGetPhysicalDeviceQueueFamilyProperties(
    physicalDevice,
    &queueFamilyCount,
    queueFamilies.data()
);
```

Each `VkQueueFamilyProperties` contains a `queueFlags` field. Flags such as
`VK_QUEUE_GRAPHICS_BIT`, `VK_QUEUE_COMPUTE_BIT`, and `VK_QUEUE_TRANSFER_BIT`
describe operations supported by queues from that family.

A graphics-capable family can therefore be found by testing its flags:

```cpp
if (queueFamilies[index].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
    graphicsFamily = index;
}
```

The family index is not itself a queue. It identifies the class of queues
from which the logical device can request queues.

## Device selection is a policy decision

Vulkan does not prescribe a universal rule for choosing a physical device.
The application normally evaluates several requirements together.

A renderer might require a graphics queue, a particular device extension, a
feature such as anisotropic sampling, and a minimum set of resource limits.
Only after those checks succeed is a candidate suitable.

A simple application can choose the first device:

```cpp
VkPhysicalDevice physicalDevice = devices[0];
```

That is useful for demonstrating the API, but it is not a robust selection
policy. Production code usually scores or filters candidates according to
what the renderer needs.

The important mental model is:

```text
enumerate -> inspect -> test requirements -> choose
```

Selection happens before logical-device creation because the logical device
must be created for one specific physical device.

## Creating the logical device

`VkDevice` is the logical device. It is the application-facing interface
created from a selected physical device.

Device creation specifies which queue families and queue counts the
application wants, along with the device features and extensions it intends
to enable.

A queue request contains a priority:

```cpp
float queuePriority = 1.0f;

VkDeviceQueueCreateInfo queueInfo{
    VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    nullptr,
    0,
    graphicsFamily,
    1,
    &queuePriority
};
```

The `queueFamilyIndex` selects the family, while `queueCount` selects how many
queues from that family are requested. The requested count cannot exceed the
number of queues exposed by that family.

Queue priorities influence scheduling between queues in the same family. They
are floating-point values from zero through one and are a scheduling hint,
not a guarantee that a queue will receive a particular fraction of GPU time.

The device creation structure then describes the complete request:

```cpp
VkDeviceCreateInfo deviceInfo{
    VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    nullptr,
    0,
    1,
    &queueInfo,
    0,
    nullptr,
    0,
    nullptr,
    &features
};
```

The `pEnabledFeatures` field points at the features the application wants to
enable. Every enabled feature must have been reported as supported by the
selected physical device.

Device extensions are selected through the extension count and name array.
Unlike instance extensions, these extensions belong to the logical-device
interface and are therefore checked against the selected physical device.

## Retrieving a queue

Creating a logical device creates the requested queues as part of the device
configuration. The application then retrieves a queue handle.

```cpp
VkQueue graphicsQueue = VK_NULL_HANDLE;
vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
```

The family index and queue index identify which requested queue is retrieved.
The returned `VkQueue` becomes the submission destination for command buffers.

This explains why queue selection happened before device creation. The logical
device needs to know which queue families and queue counts the application
requires before it can establish the queue interface.

The resulting relationship is:

```text
physical device
    |
    +-> queue families
    |
    v
logical device
    |
    v
queues
```

The physical device remains the object used for capability discovery. The
logical device and its queues are the objects used to perform ordinary GPU
work.

## Instance extensions and device extensions

Vulkan separates extensions according to what they extend.

Instance extensions affect functionality available around the instance and
physical-device level. Device extensions affect functionality exposed by a
particular logical device.

This distinction becomes especially important when a window system is
involved. A presentation surface extension is part of the instance-level
interface, while the swapchain extension is enabled on the logical device.

The application therefore needs to validate extensions at the correct stage.
A device extension cannot be treated as globally available merely because the
Vulkan loader exists.

The general process is:

```text
instance extensions
    -> create instance
    -> enumerate physical devices
    -> inspect device extensions
    -> create logical device
```

This separation prevents an application from requesting functionality before
it knows whether the relevant Vulkan object can support it.

## Features are enabled during device creation

One subtle part of Vulkan's design is that discovering a feature and enabling a
feature are different operations.

The physical device may report:

```cpp
VkPhysicalDeviceFeatures supportedFeatures{};
vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);
```

The application can then request a supported feature:

```cpp
VkPhysicalDeviceFeatures enabledFeatures{};
enabledFeatures.samplerAnisotropy = VK_TRUE;
```

The second structure does not discover anything. It is a request made during
logical-device creation.

This distinction is important because a feature being present in the
implementation does not automatically make it active for the application.
The application explicitly declares the features it wants enabled.

## What the logical device does not represent

It is tempting to think of `VkDevice` as a virtual GPU that contains all
state used by the renderer. That model is too broad.

The logical device is the interface through which many Vulkan objects are
created and managed. Command pools, buffers, images, pipelines, descriptor
objects, synchronization objects, and other resources are associated with a
device, but they remain separate Vulkan objects with their own lifetimes.

The device therefore acts more like the root of the application's usable GPU
context than a container holding every object physically inside it.

That distinction becomes increasingly important as the renderer grows.

## The initialization chain

The complete initialization process can now be expressed without hiding the
individual decisions:

```text
create instance
    -> enumerate physical devices
    -> inspect properties and features
    -> inspect queue families
    -> inspect required extensions
    -> select physical device
    -> request queues and features
    -> create logical device
    -> retrieve queues
```

The instance answers where Vulkan begins. The physical device answers what
hardware is available. The logical device answers what the application has
chosen to use, and the queue is where later command submissions will go.

This is the foundation on which the rest of the renderer is built. The next
lesson adds the window-system surface and swapchain so that the queue can
eventually produce images that can be presented to a screen.

## Next step

Now type the code version of this lesson.

