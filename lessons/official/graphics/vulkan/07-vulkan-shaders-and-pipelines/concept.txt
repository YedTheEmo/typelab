# Vulkan shaders and pipelines - concepts

A Vulkan graphics pipeline describes how the GPU turns vertex data into a
rendered image.

Shaders provide programmable stages inside that pipeline. Vulkan does not
normally accept source code such as GLSL directly. Shader source is compiled
into an intermediate representation called SPIR-V, which Vulkan can load into
shader modules.

The basic relationship is:

```
shader source
    |
    v
  SPIR-V
    |
    v
shader module
    |
    v
pipeline stage
    |
    v
graphics pipeline
```

## Shader stages

A basic graphics pipeline commonly has a vertex shader and a fragment shader.

The vertex shader processes vertex data and produces positions and other values:

```
#version 450

layout(location = 0) in vec2 position;

void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
}
```

The fragment shader determines the output color:

```
#version 450

layout(location = 0) out vec4 color;

void main()
{
    color = vec4(1.0, 0.0, 0.0, 1.0);
}
```

The vertex shader runs for vertices. The fragment shader runs for fragments
produced by rasterization.

## SPIR-V

Vulkan consumes SPIR-V rather than depending on one particular shader language.

A GLSL source file can be compiled with a tool such as glslc:

```
glslc triangle.vert -o triangle.vert.spv
```

A Slang shader can similarly be compiled to SPIR-V with Slang's compiler.

This separation is useful because the Vulkan application does not need to know
how the shader source was written. It receives compiled shader code.

## Shader modules

Vulkan loads SPIR-V into a VkShaderModule:

```
VkShaderModuleCreateInfo info{};
info.sType =
    VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
info.codeSize = code.size();
info.pCode =
    reinterpret_cast<const uint32_t*>(code.data());
```

The module represents compiled shader code.

A shader module does not by itself define when the shader runs. The application
uses it when describing a pipeline stage.

## Pipeline stages

A VkPipelineShaderStageCreateInfo connects a shader module to a stage:

```
VkPipelineShaderStageCreateInfo stage{};
stage.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
stage.module = vertexModule;
stage.pName = "main";
```

The pName field identifies the entry point inside the shader.

A fragment shader uses the same structure with a different stage:

```
fragmentStage.stage =
    VK_SHADER_STAGE_FRAGMENT_BIT;
```

A graphics pipeline can contain multiple shader stages.

## Fixed-function state

Shaders are only part of the graphics pipeline.

Vulkan also requires the application to describe fixed-function behavior such as
vertex input, input assembly, viewport and scissor state, rasterization,
multisampling, and color blending.

For example, rasterization state can be described with:

```
VkPipelineRasterizationStateCreateInfo rasterizer{};
rasterizer.sType =
    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
rasterizer.lineWidth = 1.0f;
```

The pipeline therefore combines programmable shader stages with explicit GPU
state.

## Pipeline layout

Shaders may need access to external resources such as buffers, textures, and
push constants.

The pipeline layout describes how those resources are exposed to shaders:

```
VkPipelineLayoutCreateInfo layoutInfo{};
layoutInfo.sType =
    VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

VkPipelineLayout pipelineLayout;

vkCreatePipelineLayout(
    device,
    &layoutInfo,
    nullptr,
    &pipelineLayout);
```

Descriptor sets and push constants will be explored more deeply later.

For now, think of the pipeline layout as the interface between shaders and
resources supplied by the application.

## Creating the pipeline

The application combines the shader stages and fixed-function state into a
graphics pipeline:

```
VkGraphicsPipelineCreateInfo pipelineInfo{};
pipelineInfo.sType =
    VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
pipelineInfo.stageCount = 2;
pipelineInfo.pStages = stages;
pipelineInfo.layout = pipelineLayout;
```

The remaining fields describe the rest of the graphics state and the rendering
target.

The result is a VkPipeline object that the command buffer can bind:

```
vkCmdBindPipeline(
    commandBuffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipeline);
```

Once bound, subsequent drawing commands use that pipeline configuration.

## The complete model

The important distinction is:

```
shader source
    |
    v
  SPIR-V
    |
    v
shader module
    |
    v
pipeline stage
    |
    +------------------+
    |                  |
    v                  v
vertex shader      fragment shader
    |                  |
    +--------+---------+
             |
             v
    graphics pipeline
             |
             v
         draw command
```

The pipeline is the complete configuration used to turn submitted vertex data
into fragments and eventually framebuffer output.

## Next step

Now type the code version of this lesson.

