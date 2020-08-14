#include "r_pipeline.h"
#include "r_render.h"
#include "m_math.h"
#include "d_display.h"
#include <stdio.h>
#include <assert.h>
#include <vulkan/vulkan_core.h>

VkPipeline pipelines[MAX_PIPELINES];

enum shaderStageType { VERT, FRAG };

static void initShaderModules(VkShaderModule* vertModule, VkShaderModule* fragModule) 
{
    VkResult r;

    int fr;
    FILE* fp;
    fp = fopen("shaders/spv/simple-vert.spv", "rb");
    fr = fseek(fp, 0, SEEK_END);
    assert( fr == 0 ); // success 
    size_t vertCodeSize = ftell(fp);
    rewind(fp);

    unsigned char vertCode[vertCodeSize];
    fread(vertCode, 1, vertCodeSize, fp);
    fclose(fp);

    fp = fopen("shaders/spv/simple-frag.spv", "rb");
    assert( fp );
    fr = fseek(fp, 0, SEEK_END);
    assert( fr == 0 ); // success 
    size_t fragCodeSize = ftell(fp);
    rewind(fp);

    unsigned char fragCode[fragCodeSize];
    fread(fragCode, 1, fragCodeSize, fp);
    fclose(fp);

    const VkShaderModuleCreateInfo vertShaderInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = vertCodeSize,
        .pCode = (uint32_t*)vertCode,
    };

    const VkShaderModuleCreateInfo fragShaderInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = fragCodeSize,
        .pCode = (uint32_t*)fragCode,
    };

    r = vkCreateShaderModule(device, &vertShaderInfo, NULL, vertModule);
    assert( VK_SUCCESS == r );
    r = vkCreateShaderModule(device, &fragShaderInfo, NULL, fragModule);
    assert( VK_SUCCESS == r );
}

static void initVertexInputState(VkPipelineVertexInputStateCreateInfo* vertexInputState)
{
}

void initPipelines(void)
{
    VkShaderModule vertModule;
    VkShaderModule fragModule;

    initShaderModules(&vertModule, &fragModule);

    const VkSpecializationInfo shaderSpecialInfo = {
        // TODO
    };

    const VkPipelineShaderStageCreateInfo shaderStages[2] = {
        [0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        [0].stage = VK_SHADER_STAGE_VERTEX_BIT,
        [0].module = vertModule,
        [0].pName = "main",
        [0].pSpecializationInfo = NULL,
        [1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        [1].stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        [1].module = fragModule,
        [1].pName = "main",
        [1].pSpecializationInfo = NULL,
    }; // vert and frag

    const VkVertexInputBindingDescription bindingDescription = {
        .binding = 0,
        .stride  = sizeof(Vec2), // all our verts will be 2D
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    const VkVertexInputAttributeDescription positionAttributeDescription = {
        .binding = 0,
        .location = 0, 
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = 0,
    };

    const VkPipelineVertexInputStateCreateInfo vertexInput = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = 1,
        .pVertexAttributeDescriptions = &positionAttributeDescription
    };

    const VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
        .primitiveRestartEnable = VK_FALSE // applies only to index calls
    };

    const VkViewport viewport = {
        .height = WINDOW_HEIGHT,
        .width = WINDOW_WIDTH,
        .x = 0,
        .y = 0,
        .minDepth = 0.0,
        .maxDepth = 1.0
    };

    const VkRect2D scissor = {
        .extent = {WINDOW_WIDTH, WINDOW_HEIGHT},
        .offset = {0, 0}
    };

    const VkPipelineViewportStateCreateInfo viewportState = {
        .scissorCount = 1,
        .pScissors = &scissor,
        .viewportCount = 1,
        .pViewports = &viewport,
    };

    const VkPipelineRasterizationStateCreateInfo rasterizationState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE, // dunno
        .rasterizerDiscardEnable = VK_TRUE, // a guess
        .polygonMode = VK_POLYGON_MODE_LINE,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0
    };

    const VkPipelineMultisampleStateCreateInfo multisampleState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
        // TODO: alot more settings here. more to look into
    };

    const VkGraphicsPipelineCreateInfo pipelineInfo = {
        .pMultisampleState = &multisampleState,
        .pRasterizationState = &rasterizationState,
        .pViewportState = &viewportState,
        .pTessellationState = NULL, // may be able to do splines with this
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .flags = 0,
        .stageCount = sizeof(shaderStages) / sizeof(shaderStages[0]),
        .pStages = shaderStages,
        .pVertexInputState = &vertexInput,
        .pInputAssemblyState = &inputAssembly,
    };
    //vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, pipelines);
    printf("PLACEHOLDER INITPIPELINES\n");
}
