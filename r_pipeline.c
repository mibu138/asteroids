#include "r_pipeline.h"
#include "v_video.h"
#include "r_render.h"
#include "m_math.h"
#include "def.h"
#include <stdio.h>
#include <assert.h>
#include <vulkan/vulkan_core.h>

VkPipeline      pipelines[MAX_PIPELINES];
VkDescriptorSet descriptorSets[MAX_DESCRIPTOR_SETS];
static VkPipelineLayout pipelineLayoutGeometry;
static VkPipelineLayout pipelineLayoutPostProcess;
static VkDescriptorSetLayout descriptorSetLayoutEmpty; 
static VkDescriptorSetLayout descriptorSetLayoutPostProcess; 
static VkDescriptorPool      descriptorPool;

enum shaderStageType { VERT, FRAG };

#define PIPELINE_COUNT 3
#define DESCRIPTOR_SET_COUNT 1

static void initShaderModule(const char* filepath, VkShaderModule* module)
{
    VkResult r;
    int fr;
    FILE* fp;
    fp = fopen(filepath, "rb");
    fr = fseek(fp, 0, SEEK_END);
    assert( fr == 0 ); // success 
    size_t codeSize = ftell(fp);
    rewind(fp);

    unsigned char code[codeSize];
    fread(code, 1, codeSize, fp);
    fclose(fp);

    const VkShaderModuleCreateInfo shaderInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = codeSize,
        .pCode = (uint32_t*)code,
    };

    r = vkCreateShaderModule(device, &shaderInfo, NULL, module);
    assert( VK_SUCCESS == r );
}

void initDescriptorSets(void)
{
    VkResult r;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 0,
        .pBindings = NULL
    };

    r = vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, &descriptorSetLayoutEmpty);
    assert( VK_SUCCESS == r );

    VkDescriptorSetLayoutBinding textureBinding = {
        .binding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = NULL, // no one really seems to use these
    };

    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &textureBinding;

    r = vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, &descriptorSetLayoutPostProcess);
    assert( VK_SUCCESS == r );

    VkDescriptorPoolSize poolSize = {
        .descriptorCount = 1,
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
    };

    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1,
        .poolSizeCount = 1,
        // odd name. not the sizes of pools, but the numbers of descriptors
        // of a certain type that can be created from the pool
        .pPoolSizes = &poolSize, 
    };

    vkCreateDescriptorPool(device, &poolInfo, NULL, &descriptorPool);

    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = DESCRIPTOR_SET_COUNT,
        .pSetLayouts = &descriptorSetLayoutPostProcess,
    };

    vkAllocateDescriptorSets(device, &allocInfo, descriptorSets);
}

static void initPipelineLayouts(void)
{
    VkResult r;
    VkPipelineLayoutCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .setLayoutCount = 1, // ?
        .pSetLayouts = &descriptorSetLayoutEmpty,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = NULL
    };

    r = vkCreatePipelineLayout(device, &info, NULL, &pipelineLayoutGeometry);
    assert( VK_SUCCESS == r );
    
    info.pSetLayouts = &descriptorSetLayoutPostProcess;

    r = vkCreatePipelineLayout(device, &info, NULL, &pipelineLayoutPostProcess);
    assert( VK_SUCCESS == r );
}

void initPipelines(void)
{
    initPipelineLayouts();

    VkShaderModule vertModule;
    VkShaderModule fragModule;

    initShaderModule("shaders/spv/simple-vert.spv", &vertModule);
    initShaderModule("shaders/spv/simple-frag.spv", &fragModule);

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
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .scissorCount = 1,
        .pScissors = &scissor,
        .viewportCount = 1,
        .pViewports = &viewport,
    };

    const VkPipelineRasterizationStateCreateInfo rasterizationState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE, // dunno
        .rasterizerDiscardEnable = VK_FALSE, // actually discards everything
        .polygonMode = VK_POLYGON_MODE_LINE,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 2.0
    };

    const VkPipelineMultisampleStateCreateInfo multisampleState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
        // TODO: alot more settings here. more to look into
    };

    const VkPipelineColorBlendAttachmentState attachmentState = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT, /* need this to actually
                                                                    write anything to the
                                                                    framebuffer */
        .blendEnable = VK_FALSE, // no blending for now
        .srcColorBlendFactor = 0,
        .dstColorBlendFactor = 0,
        .colorBlendOp = 0,
        .srcAlphaBlendFactor = 0,
        .dstAlphaBlendFactor = 0,
        .alphaBlendOp = 0,
    };

    const VkPipelineColorBlendStateCreateInfo colorBlendState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE, // only for integer framebuffer formats
        .logicOp = 0,
        .attachmentCount = 1,
        .pAttachments = &attachmentState /* must have independentBlending device   
            feature enabled for these to be different. each entry would correspond 
            to the blending for a different framebuffer. */
    };

    const VkGraphicsPipelineCreateInfo pipelineInfo = {
        .basePipelineIndex = 0, // not used
        .basePipelineHandle = 0,
        .subpass = 0, // which subpass in the renderpass do we use this pipeline with
        .renderPass = swapchainRenderPass,
        .layout = pipelineLayoutGeometry,
        .pDynamicState = NULL,
        .pColorBlendState = &colorBlendState,
        .pDepthStencilState = NULL,
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

    // ----------------------------------------------------------
    // creating the second pipeline info for the emitables
    // ----------------------------------------------------------

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyEmit = inputAssembly;
    inputAssemblyEmit.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

    VkPipelineRasterizationStateCreateInfo rasterizationStateEmit = rasterizationState;
    rasterizationStateEmit.polygonMode = VK_POLYGON_MODE_POINT;

    VkGraphicsPipelineCreateInfo emitablePipelineInfo = pipelineInfo;
    emitablePipelineInfo.pRasterizationState = &rasterizationStateEmit;
    emitablePipelineInfo.pInputAssemblyState = &inputAssemblyEmit;

    // ----------------------------------------------------------
    // creating the third pipeline info for post processing
    // ----------------------------------------------------------

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyPostProc = inputAssembly;
    inputAssemblyPostProc.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineRasterizationStateCreateInfo rasterizationStatePostProc = rasterizationState;
    rasterizationStatePostProc.polygonMode = VK_POLYGON_MODE_FILL;

    const VkPipelineVertexInputStateCreateInfo postProcInputState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexAttributeDescriptionCount = 0,
        .vertexBindingDescriptionCount = 0,
    };

    VkShaderModule postProcVertModule;

    initShaderModule("shaders/spv/postproc-vert.spv", &postProcVertModule);

    VkPipelineShaderStageCreateInfo postProcShaderStages[2] = {shaderStages[0], shaderStages[1]};
    postProcShaderStages[0].module = postProcVertModule;
    
    // ----------------------------------------------------------
    // ----------------------------------------------------------

    VkGraphicsPipelineCreateInfo postProcPipelineInfo = pipelineInfo;
    postProcPipelineInfo.pRasterizationState = &rasterizationStatePostProc;
    postProcPipelineInfo.pInputAssemblyState = &inputAssemblyPostProc;
    postProcPipelineInfo.pVertexInputState = &postProcInputState;
    postProcPipelineInfo.pStages           = postProcShaderStages;
    postProcPipelineInfo.layout            = pipelineLayoutPostProcess;

    VkGraphicsPipelineCreateInfo infos[PIPELINE_COUNT] = {pipelineInfo, emitablePipelineInfo, postProcPipelineInfo};

    vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, PIPELINE_COUNT, infos, NULL, pipelines);

    vkDestroyShaderModule(device, vertModule, NULL);
    vkDestroyShaderModule(device, fragModule, NULL);
    vkDestroyShaderModule(device, postProcVertModule, NULL);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayoutEmpty, NULL);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayoutPostProcess, NULL);
    vkDestroyPipelineLayout(device, pipelineLayoutGeometry, NULL);
    vkDestroyPipelineLayout(device, pipelineLayoutPostProcess, NULL);
}

void cleanUpPipelines()
{
    for (int i = 0; i < PIPELINE_COUNT; i++) 
    {
        vkDestroyPipeline(device, pipelines[i], NULL);
    }
    vkDestroyDescriptorPool(device, descriptorPool, NULL);
}

