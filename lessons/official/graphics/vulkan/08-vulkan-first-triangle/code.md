# Vulkan first triangle - typing

This lesson types the rendering path: vertices, a graphics pipeline, recorded
draw commands, and the submit-and-present cycle that shows the triangle.

## Define the triangle

The vertex shader reads three positions that form one triangle.

```
// one vertex has two floats
struct Vertex
{
    float x;
    float y;
};

// three vertices forming a triangle
const Vertex vertices[] = {
    {-0.5f, -0.5f},
    { 0.5f, -0.5f},
    { 0.0f,  0.5f}
};
```

## Describe vertex input

The pipeline needs to know how the vertex buffer is laid out.

```
// describes one binding of the vertex buffer
VkVertexInputBindingDescription binding{};
// binding point zero
binding.binding = 0;
// bytes from one vertex to the next
binding.stride = sizeof(Vertex);
// data advances per vertex, not per instance
binding.inputRate =
    VK_VERTEX_INPUT_RATE_VERTEX;

// describes one attribute inside the binding
VkVertexInputAttributeDescription attribute{};
// attribute comes from binding zero
attribute.binding = 0;
// shader input location zero
attribute.location = 0;
// two 32-bit floats per vertex
attribute.format = VK_FORMAT_R32G32_SFLOAT;
// the attribute starts at byte zero of the vertex
attribute.offset = 0;
```

## Describe input assembly

Three vertices should be interpreted as one triangle.

```
// the create-info struct for input assembly
VkPipelineInputAssemblyStateCreateInfo assembly{};
// identify the assembly create-info type
assembly.sType =
    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
// every three vertices become one triangle
assembly.topology =
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
// no primitive restart used
assembly.primitiveRestartEnable = VK_FALSE;
```

## Describe the viewport

The viewport maps coordinates onto the framebuffer.

```
// the create-info struct for the viewport
VkViewport viewport{};
// left edge of the viewport
viewport.x = 0.0f;
// top edge of the viewport
viewport.y = 0.0f;
// width matches the swapchain extent
viewport.width =
    static_cast<float>(extent.width);
// height matches the swapchain extent
viewport.height =
    static_cast<float>(extent.height);
// depth range starts at zero
viewport.minDepth = 0.0f;
// depth range ends at one
viewport.maxDepth = 1.0f;

// the region that can receive fragments
VkRect2D scissor{};
// starts at the top-left corner
scissor.offset = {0, 0};
// covers the whole swapchain extent
scissor.extent = extent;

// packages viewport and scissor together
VkPipelineViewportStateCreateInfo viewportState{};
// identify the viewport-state type
viewportState.sType =
    VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
// one viewport
viewportState.viewportCount = 1;
// the viewport above
viewportState.pViewports = &viewport;
// one scissor
viewportState.scissorCount = 1;
// the scissor above
viewportState.pScissors = &scissor;
```

## Describe rasterization

The rasterizer turns the triangle into fragments.

```
// the create-info struct for rasterization
VkPipelineRasterizationStateCreateInfo rasterizer{};
// identify the rasterizer create-info type
rasterizer.sType =
    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
// fill every pixel of the triangle
rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
// discard back-facing triangles
rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
// vertices wind clockwise on the front face
rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
// raster line width in pixels
rasterizer.lineWidth = 1.0f;
```

## Describe multisampling and color output

Use one sample per pixel and write color without blending.

```
// the create-info struct for multisampling
VkPipelineMultisampleStateCreateInfo multisampling{};
// identify the multisample create-info type
multisampling.sType =
    VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
// one sample per pixel
multisampling.rasterizationSamples =
    VK_SAMPLE_COUNT_1_BIT;

// color output for one attachment
VkPipelineColorBlendAttachmentState colorBlend{};
// write every color channel
colorBlend.colorWriteMask =
    VK_COLOR_COMPONENT_R_BIT |
    VK_COLOR_COMPONENT_G_BIT |
    VK_COLOR_COMPONENT_B_BIT |
    VK_COLOR_COMPONENT_A_BIT;
// no blending needed for the first triangle
colorBlend.blendEnable = VK_FALSE;

// the global color blend state
VkPipelineColorBlendStateCreateInfo colorState{};
// identify the color-state type
colorState.sType =
    VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
// one color attachment
colorState.attachmentCount = 1;
// the attachment state above
colorState.pAttachments = &colorBlend;
```

## Create the pipeline

The pipeline create-info assembles every state structure.

```
// the shader stages from the previous lesson
VkPipelineShaderStageCreateInfo stages[] = {
    vertexStage,
    fragmentStage
};

// the create-info struct for vertex input
VkPipelineVertexInputStateCreateInfo vertexInput{};
// identify the vertex-input type
vertexInput.sType =
    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
// one vertex binding
vertexInput.vertexBindingDescriptionCount = 1;
// the binding built above
vertexInput.pVertexBindingDescriptions = &binding;
// one vertex attribute
vertexInput.vertexAttributeDescriptionCount = 1;
// the attribute built above
vertexInput.pVertexAttributeDescriptions = &attribute;

// the create-info struct for the graphics pipeline
VkGraphicsPipelineCreateInfo pipelineInfo{};
// identify the pipeline create-info type
pipelineInfo.sType =
    VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
// two shader stages
pipelineInfo.stageCount = 2;
// the stage array
pipelineInfo.pStages = stages;
// how vertices are read
pipelineInfo.pVertexInputState = &vertexInput;
// how vertices become primitives
pipelineInfo.pInputAssemblyState = &assembly;
// how primitives map to the framebuffer
pipelineInfo.pViewportState = &viewportState;
// how primitives become fragments
pipelineInfo.pRasterizationState = &rasterizer;
// how many samples per pixel
pipelineInfo.pMultisampleState = &multisampling;
// how color is written
pipelineInfo.pColorBlendState = &colorState;
// the pipeline layout from the previous lesson
pipelineInfo.layout = pipelineLayout;
```

## Record the draw

The command buffer is filled with the draw operations.

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

// bind the graphics pipeline to the buffer
vkCmdBindPipeline(
    commandBuffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    graphicsPipeline);

// vertex data starts at the beginning of the buffer
VkDeviceSize offset = 0;

// bind the vertex buffer at binding point zero
vkCmdBindVertexBuffers(
    commandBuffer,
    0,             // binding number
    1,             // one buffer
    &vertexBuffer,
    &offset);

// draw three vertices as one instance
vkCmdDraw(
    commandBuffer,
    3,   // vertex count
    1,   // instance count
    0,   // first vertex
    0);  // first instance

// leave the recording state
vkEndCommandBuffer(commandBuffer);
```

## Submit and present

The recorded buffer travels through the queue to the screen.

```
// block the CPU until the previous frame is done
vkWaitForFences(
    device,
    1,
    &inFlightFence,
    VK_TRUE,
    UINT64_MAX);

// prepare the fence for this frame
vkResetFences(
    device,
    1,
    &inFlightFence);

// index of the image handed to us
uint32_t imageIndex = 0;

// acquire an image and signal imageAvailable when ready
vkAcquireNextImageKHR(
    device,
    swapchain,
    UINT64_MAX,
    imageAvailable,
    VK_NULL_HANDLE,
    &imageIndex);

// only the color attachment stage needs to wait
VkPipelineStageFlags waitStage =
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

// the create-info struct for the submission
VkSubmitInfo submitInfo{};
// identify the submit-info type
submitInfo.sType =
    VK_STRUCTURE_TYPE_SUBMIT_INFO;
// wait on one semaphore
submitInfo.waitSemaphoreCount = 1;
// the semaphore to wait on
submitInfo.pWaitSemaphores = &imageAvailable;
// the stage that must wait
submitInfo.pWaitDstStageMask = &waitStage;
// submit one command buffer
submitInfo.commandBufferCount = 1;
// the recorded draw buffer
submitInfo.pCommandBuffers = &commandBuffer;
// signal one semaphore when done
submitInfo.signalSemaphoreCount = 1;
// the semaphore that reports rendering finished
submitInfo.pSignalSemaphores = &renderFinished;

// submit the rendering work and arm the fence
vkQueueSubmit(
    graphicsQueue,
    1,
    &submitInfo,
    inFlightFence);

// the create-info struct for presentation
VkPresentInfoKHR presentInfo{};
// identify the present-info type
presentInfo.sType =
    VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
// wait for rendering before presenting
presentInfo.waitSemaphoreCount = 1;
// the semaphore that says rendering is done
presentInfo.pWaitSemaphores = &renderFinished;
// present to one swapchain
presentInfo.swapchainCount = 1;
// the swapchain to present to
presentInfo.pSwapchains = &swapchain;
// which image to present
presentInfo.pImageIndices = &imageIndex;

// request the presentation
vkQueuePresentKHR(
    presentQueue,
    &presentInfo);
```

## Now type it again

Type the essential draw sequence.

```
// bind the graphics pipeline to the buffer
vkCmdBindPipeline(
    commandBuffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    graphicsPipeline);

// vertex data starts at the beginning of the buffer
VkDeviceSize offset = 0;

// bind the vertex buffer at binding point zero
vkCmdBindVertexBuffers(
    commandBuffer,
    0,             // binding number
    1,             // one buffer
    &vertexBuffer,
    &offset);

// draw three vertices as one instance
vkCmdDraw(
    commandBuffer,
    3,   // vertex count
    1,   // instance count
    0,   // first vertex
    0);  // first instance
```

Then type the submission.

```
// submit the rendering work and arm the fence
vkQueueSubmit(
    graphicsQueue,
    1,
    &submitInfo,
    inFlightFence);
```

And the presentation.

```
// request the presentation
vkQueuePresentKHR(
    presentQueue,
    &presentInfo);
```

## Wrap up

The flow: vertices -> pipeline -> command buffer -> queue -> GPU -> swapchain.
