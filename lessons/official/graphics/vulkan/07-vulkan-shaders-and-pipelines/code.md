# Vulkan shaders and pipelines - typing

This lesson types the graphics pipeline: shader stages, vertex input,
rasterization state, pipeline layout, and graphics pipeline creation.

## Describe shader stages

Connect compiled shader modules to their roles in the graphics pipeline.

```cpp
    // describe the vertex shader stage
    VkPipelineShaderStageCreateInfo vertexStage{
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        VK_SHADER_STAGE_VERTEX_BIT,
        vertexModule,
        "main",
        nullptr
    };

    // describe the fragment shader stage
    VkPipelineShaderStageCreateInfo fragmentStage{
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        fragmentModule,
        "main",
        nullptr
    };

    // collect both shader stages
    VkPipelineShaderStageCreateInfo shaderStages[]{
        vertexStage,
        fragmentStage
    };
```

## Describe vertex input

Tell Vulkan how vertex-buffer bytes map to vertex shader inputs.

```cpp
    // describe the distance between vertices
    VkVertexInputBindingDescription binding{
        0,
        sizeof(Vertex),
        VK_VERTEX_INPUT_RATE_VERTEX
    };

    // describe the position attribute
    VkVertexInputAttributeDescription attribute{
        0,
        0,
        VK_FORMAT_R32G32B32_SFLOAT,
        offsetof(Vertex, position)
    };

    // describe the vertex input state
    VkPipelineVertexInputStateCreateInfo vertexInput{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        nullptr,
        0,
        1,
        &binding,
        1,
        &attribute
    };
```

## Choose primitive assembly

Tell Vulkan how the incoming vertices form primitives.

```cpp
    // describe independent triangles
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_FALSE
    };
```

## Configure viewport state

Describe the viewport and scissor used by the pipeline.

```cpp
    // describe the framebuffer viewport
    VkViewport viewport{
        0.0f,
        0.0f,
        width,
        height,
        0.0f,
        1.0f
    };

    // describe the framebuffer scissor
    VkRect2D scissor{
        {0, 0},
        {width, height}
    };

    // describe viewport and scissor state
    VkPipelineViewportStateCreateInfo viewportState{
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        nullptr,
        0,
        1,
        &viewport,
        1,
        &scissor
    };
```

## Configure rasterization

Configure filled polygons, back-face culling, and winding order.

```cpp
    // describe rasterization behavior
    VkPipelineRasterizationStateCreateInfo rasterizer{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_FALSE,
        VK_FALSE,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_BACK_BIT,
        VK_FRONT_FACE_CLOCKWISE,
        VK_FALSE,
        0.0f,
        0.0f,
        0.0f,
        1.0f
    };
```

## Configure multisampling

Use one sample per pixel for the initial pipeline.

```cpp
    // describe single-sample rendering
    VkPipelineMultisampleStateCreateInfo multisampling{
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FALSE,
        1.0f,
        nullptr,
        VK_FALSE,
        VK_FALSE
    };
```

## Configure depth state

Leave depth testing disabled until a depth attachment exists.

```cpp
    // describe disabled depth testing
    VkPipelineDepthStencilStateCreateInfo depthStencil{
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_FALSE,
        VK_FALSE,
        VK_COMPARE_OP_LESS,
        VK_FALSE,
        VK_FALSE,
        {},
        {},
        0.0f,
        1.0f
    };
```

## Configure color output

Describe a single color attachment with replacement rather than blending.

```cpp
    // describe direct color replacement
    VkPipelineColorBlendAttachmentState blendAttachment{
        VK_FALSE,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_OP_ADD,
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT
    };

    // describe the color blending state
    VkPipelineColorBlendStateCreateInfo colorBlending{
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_FALSE,
        VK_LOGIC_OP_COPY,
        1,
        &blendAttachment,
        {0.0f, 0.0f, 0.0f, 0.0f}
    };
```

## Create the pipeline layout

Create the empty resource interface used by the initial shaders.

```cpp
    // describe an empty pipeline layout
    VkPipelineLayoutCreateInfo layoutInfo{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        0,
        nullptr,
        0,
        nullptr
    };

    // store the pipeline layout
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

    // create the pipeline layout
    vkCreatePipelineLayout(
        device,
        &layoutInfo,
        nullptr,
        &pipelineLayout
    );
```

## Configure dynamic state

Move viewport and scissor values to command recording.

```cpp
    // list the state supplied during command recording
    VkDynamicState dynamicStates[]{
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    // describe the dynamic pipeline state
    VkPipelineDynamicStateCreateInfo dynamicState{
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        nullptr,
        0,
        2,
        dynamicStates
    };
```

## Assemble the graphics pipeline

Combine the shader stages and fixed-function state into one pipeline.

```cpp
    // describe the complete graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        nullptr,
        0,
        2,
        shaderStages,
        &vertexInput,
        &inputAssembly,
        nullptr,
        &viewportState,
        &rasterizer,
        &multisampling,
        &depthStencil,
        &colorBlending,
        &dynamicState,
        pipelineLayout,
        renderPass,
        0,
        VK_NULL_HANDLE,
        -1
    };

    // store the graphics pipeline
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;

    // create the graphics pipeline
    vkCreateGraphicsPipelines(
        device,
        VK_NULL_HANDLE,
        1,
        &pipelineInfo,
        nullptr,
        &graphicsPipeline
    );
```

## Bind the pipeline

Select the graphics pipeline before recording a draw.

```cpp
    // bind the graphics pipeline
    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        graphicsPipeline
    );
```

## Set dynamic state

Provide the viewport and scissor values for this command buffer.

```cpp
    // set the dynamic viewport
    vkCmdSetViewport(
        commandBuffer,
        0,
        1,
        &viewport
    );

    // set the dynamic scissor
    vkCmdSetScissor(
        commandBuffer,
        0,
        1,
        &scissor
    );
```

## Destroy the pipeline

Destroy the pipeline after submitted work can no longer reference it.

```cpp
    // destroy the graphics pipeline
    vkDestroyPipeline(
        device,
        graphicsPipeline,
        nullptr
    );

    // destroy the pipeline layout
    vkDestroyPipelineLayout(
        device,
        pipelineLayout,
        nullptr
    );
```

## Now type it again

Re-drill shader stages and vertex input.

```cpp
    // describe the vertex shader stage
    VkPipelineShaderStageCreateInfo vertexStage{
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        VK_SHADER_STAGE_VERTEX_BIT,
        vertexModule,
        "main",
        nullptr
    };

    // describe the fragment shader stage
    VkPipelineShaderStageCreateInfo fragmentStage{
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        fragmentModule,
        "main",
        nullptr
    };

    // collect both shader stages
    VkPipelineShaderStageCreateInfo shaderStages[]{
        vertexStage,
        fragmentStage
    };

    // describe the vertex binding
    VkVertexInputBindingDescription binding{
        0,
        sizeof(Vertex),
        VK_VERTEX_INPUT_RATE_VERTEX
    };

    // describe the vertex attribute
    VkVertexInputAttributeDescription attribute{
        0,
        0,
        VK_FORMAT_R32G32B32_SFLOAT,
        offsetof(Vertex, position)
    };

    // describe the vertex input state
    VkPipelineVertexInputStateCreateInfo vertexInput{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        nullptr,
        0,
        1,
        &binding,
        1,
        &attribute
    };
```

Re-drill the final pipeline assembly and binding.

```cpp
    // describe the graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        nullptr,
        0,
        2,
        shaderStages,
        &vertexInput,
        &inputAssembly,
        nullptr,
        &viewportState,
        &rasterizer,
        &multisampling,
        &depthStencil,
        &colorBlending,
        &dynamicState,
        pipelineLayout,
        renderPass,
        0,
        VK_NULL_HANDLE,
        -1
    };

    // store the graphics pipeline
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;

    // create the graphics pipeline
    vkCreateGraphicsPipelines(
        device,
        VK_NULL_HANDLE,
        1,
        &pipelineInfo,
        nullptr,
        &graphicsPipeline
    );

    // bind the graphics pipeline
    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        graphicsPipeline
    );
```

## Wrap up

```text
shader -> stages -> vertex input -> fixed state -> pipeline -> bind -> draw
```
