# Vulkan shaders and pipelines - typing

This lesson types the shader path: load compiled SPIR-V, build modules,
describe stages, create a pipeline layout, and sketch a graphics pipeline.

## Load SPIR-V

Vulkan expects shader code already compiled to SPIR-V bytes.

```
// file reading for shader files
#include <fstream>
// byte storage for shader code
#include <vector>

// helper that reads a whole file into a byte vector
std::vector<char> readFile(
    const std::string& filename)
{
    // open the file positioned at its end
    std::ifstream file(
        filename,
        std::ios::ate |
        std::ios::binary);

    // use the file size to size the buffer
    size_t size =
        static_cast<size_t>(file.tellg());

    // allocate the byte buffer
    std::vector<char> buffer(size);

    // rewind to the start of the file
    file.seekg(0);
    // read every byte into the buffer
    file.read(buffer.data(), size);

    // return the loaded bytes
    return buffer;
}
```

## Create a shader module

Each shader file becomes a module of SPIR-V code.

```
// load the compiled vertex shader
std::vector<char> vertexCode =
    readFile("triangle.vert.spv");

// the create-info struct for the module
VkShaderModuleCreateInfo moduleInfo{};
// identify the module create-info type
moduleInfo.sType =
    VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
// byte size of the shader code
moduleInfo.codeSize = vertexCode.size();
// reinterpret the bytes as SPIR-V words
moduleInfo.pCode =
    reinterpret_cast<const uint32_t*>(
        vertexCode.data());

// handle that Vulkan will fill in
VkShaderModule vertexModule =
    VK_NULL_HANDLE;

// create the vertex module
VkResult result = vkCreateShaderModule(
    device,
    &moduleInfo,
    nullptr,
    &vertexModule);

// bail out if module creation failed
if (result != VK_SUCCESS)
    return 1;

// load the compiled fragment shader
std::vector<char> fragmentCode =
    readFile("triangle.frag.spv");

// reuse the module info for the fragment code
moduleInfo.codeSize = fragmentCode.size();
moduleInfo.pCode =
    reinterpret_cast<const uint32_t*>(
        fragmentCode.data());

// handle that Vulkan will fill in
VkShaderModule fragmentModule =
    VK_NULL_HANDLE;

// create the fragment module
result = vkCreateShaderModule(
    device,
    &moduleInfo,
    nullptr,
    &fragmentModule);

// bail out if module creation failed
if (result != VK_SUCCESS)
    return 1;
```

## Describe the shader stages

Each module is wired to a stage of the pipeline.

```
// describes the vertex shader stage
VkPipelineShaderStageCreateInfo vertexStage{};
// identify the stage create-info type
vertexStage.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
// this stage runs on every vertex
vertexStage.stage =
    VK_SHADER_STAGE_VERTEX_BIT;
// the module holding the code
vertexStage.module = vertexModule;
// entry point name inside the shader
vertexStage.pName = "main";

// describes the fragment shader stage
VkPipelineShaderStageCreateInfo fragmentStage{};
// identify the stage create-info type
fragmentStage.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
// this stage runs on every fragment
fragmentStage.stage =
    VK_SHADER_STAGE_FRAGMENT_BIT;
// the module holding the code
fragmentStage.module = fragmentModule;
// entry point name inside the shader
fragmentStage.pName = "main";

// both stages together
VkPipelineShaderStageCreateInfo stages[] = {
    vertexStage,
    fragmentStage
};
```

## Create the pipeline layout

The layout declares the shaders' external resources; this example needs none.

```
// the create-info struct for the layout
VkPipelineLayoutCreateInfo layoutInfo{};
// identify the layout create-info type
layoutInfo.sType =
    VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

// handle that Vulkan will fill in
VkPipelineLayout pipelineLayout =
    VK_NULL_HANDLE;

// create the pipeline layout
result = vkCreatePipelineLayout(
    device,
    &layoutInfo,
    nullptr,
    &pipelineLayout);

// bail out if layout creation failed
if (result != VK_SUCCESS)
    return 1;
```

## Describe rasterization

The rasterizer converts primitives into fragments.

```
// the create-info struct for rasterization
VkPipelineRasterizationStateCreateInfo rasterizer{};
// identify the rasterizer create-info type
rasterizer.sType =
    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
// do not clamp depth values
rasterizer.depthClampEnable = VK_FALSE;
// keep rasterization enabled
rasterizer.rasterizerDiscardEnable = VK_FALSE;
// fill every pixel of the triangle
rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
// raster line width in pixels
rasterizer.lineWidth = 1.0f;
// discard back-facing triangles
rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
// vertices wind clockwise on the front face
rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
```

## Describe the pipeline

The graphics pipeline create-info ties the pieces together.

```
// the create-info struct for the graphics pipeline
VkGraphicsPipelineCreateInfo pipelineInfo{};
// identify the pipeline create-info type
pipelineInfo.sType =
    VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
// two shader stages
pipelineInfo.stageCount = 2;
// the stage array built above
pipelineInfo.pStages = stages;
// the pipeline layout
pipelineInfo.layout = pipelineLayout;
// the rasterization state
pipelineInfo.pRasterizationState =
    &rasterizer;
```

## Bind the pipeline

A command buffer binds the pipeline before issuing draws.

```
// handle that the application already owns
VkPipeline graphicsPipeline =
    VK_NULL_HANDLE;

// bind the graphics pipeline to the command buffer
vkCmdBindPipeline(
    commandBuffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    graphicsPipeline);

// any draw now runs through the bound pipeline
vkCmdDraw(
    commandBuffer,
    3,   // vertex count
    1,   // instance count
    0,   // first vertex
    0);  // first instance
```

## Clean up

Modules can be destroyed after pipeline creation; the layout must outlive it.

```
// destroy the fragment module
vkDestroyShaderModule(
    device,
    fragmentModule,
    nullptr);

// destroy the vertex module
vkDestroyShaderModule(
    device,
    vertexModule,
    nullptr);

// destroy the pipeline layout
vkDestroyPipelineLayout(
    device,
    pipelineLayout,
    nullptr);

// destroy the graphics pipeline itself
vkDestroyPipeline(
    device,
    graphicsPipeline,
    nullptr);
```

## Now type it again

Type the essential shader-stage setup.

```
// describes the vertex shader stage
VkPipelineShaderStageCreateInfo vertexStage{};
// identify the stage create-info type
vertexStage.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
// this stage runs on every vertex
vertexStage.stage =
    VK_SHADER_STAGE_VERTEX_BIT;
// the module holding the code
vertexStage.module = vertexModule;
// entry point name inside the shader
vertexStage.pName = "main";

// describes the fragment shader stage
VkPipelineShaderStageCreateInfo fragmentStage{};
// identify the stage create-info type
fragmentStage.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
// this stage runs on every fragment
fragmentStage.stage =
    VK_SHADER_STAGE_FRAGMENT_BIT;
// the module holding the code
fragmentStage.module = fragmentModule;
// entry point name inside the shader
fragmentStage.pName = "main";
```

Then type the pipeline connection.

```
// the create-info struct for the graphics pipeline
VkGraphicsPipelineCreateInfo pipelineInfo{};
// identify the pipeline create-info type
pipelineInfo.sType =
    VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
// two shader stages
pipelineInfo.stageCount = 2;
// the stage array built above
pipelineInfo.pStages = stages;
// the pipeline layout
pipelineInfo.layout = pipelineLayout;
```

## Wrap up

The flow: shader source -> SPIR-V -> modules -> stages -> pipeline -> bind -> draw.
