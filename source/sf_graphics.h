#ifndef SF_GRAPHICS_H
#define SF_GRAPHICS_H

#define GLFW_INCLUDE_VULKAN
#include <GLfW/glfw3.h>
#include <vulkan/vulkan.h>

#include <sf.h>

#define SF_GRAPHICS_MAX_PHYSICAL_DEVICE_COUNT 8
#define SF_GRAPHICS_MAX_IMAGE_IN_FLIGHT_COUNT 4
#define SF_GRAPHICS_MAX_BUFFERING_COUNT 4
#define SF_GRAPHICS_MAX_DESCRIPTOR_ENTRY_COUNT 8
#define SF_GRAPHICS_MAX_ATTACHMENT_COUNT 4
#define SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT 3
#define SF_GRAPHICS_MAX_COMMAND_BUFFER_SUBMIT_COUNT 8
#define SF_GRAPHICS_MAX_WAIT_SEMAPHORE_COUNT 8
#define SF_GRAPHICS_MAX_SIGNAL_SEMAPHORE_COUNT 8
#define SF_GRAPHICS_MAX_DESCRIPTOR_COUNT 8
#define SF_GRAPHICS_MAX_VERTEX_ATTRIBUTE_COUNT 8
#define SF_GRAPHICS_MAX_VERTEX_BINDING_COUNT 8
#define SF_NULL_HANDLE 0

#define SF_AS_HANDLE(p) ((SfHandle)(p))

typedef intptr_t SfHandle;

typedef enum SfGraphicsBufferUsage {
	SF_GRAPHICS_BUFFER_USAGE_NONE,
	SF_GRAPHICS_BUFFER_USAGE_VERTEX,
	SF_GRAPHICS_BUFFER_USAGE_INDEX,
} SfGraphicsBufferUsage;

typedef enum SfGraphicsTextureType {
	SF_GRAPHICS_TEXTURE_TYPE_1D,
	SF_GRAPHICS_TEXTURE_TYPE_2D,
	SF_GRAPHICS_TEXTURE_TYPE_3D,
	SF_GRAPHICS_TEXTURE_TYPE_CUBE
} SfGraphicsTextureType;

typedef enum SfGraphicsTextureUsage {
	SF_GRAPHICS_TEXTURE_USAGE_UNDEFINED		   = 0X00000000,
	SF_GRAPHICS_TEXTURE_USAGE_TRANSFER_SRC		   = 0X00000001,
	SF_GRAPHICS_TEXTURE_USAGE_TRANSFER_DST		   = 0X00000002,
	SF_GRAPHICS_TEXTURE_USAGE_SAMPLED		   = 0X00000004,
	SF_GRAPHICS_TEXTURE_USAGE_STORAGE		   = 0X00000008,
	SF_GRAPHICS_TEXTURE_USAGE_COLOR_ATTACHMENT	   = 0X00000010,
	SF_GRAPHICS_TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT = 0X00000020,
	SF_GRAPHICS_TEXTURE_USAGE_RESOLVE_SRC		   = 0X00000040,
	SF_GRAPHICS_TEXTURE_USAGE_RESOLVE_DST		   = 0X00000080,
	SF_GRAPHICS_TEXTURE_USAGE_PRESENT		   = 0X00000100,
} SfGraphicsTextureUsage;
typedef uint32_t SfGraphicsTextureUsageFlags;

typedef enum SfGraphicsFormat {
	SF_GRAPHICS_FORMAT_UNDEFINED = 0,
	SF_GRAPHICS_FORMAT_R8_UNORM,
	SF_GRAPHICS_FORMAT_R16_UNORM,
	SF_GRAPHICS_FORMAT_R16_UINT,
	SF_GRAPHICS_FORMAT_R16_SFLOAT,
	SF_GRAPHICS_FORMAT_R32_UINT,
	SF_GRAPHICS_FORMAT_R32_SFLOAT,
	SF_GRAPHICS_FORMAT_R8G8_UNORM,
	SF_GRAPHICS_FORMAT_R16G16_UNORM,
	SF_GRAPHICS_FORMAT_R16G16_SFLOAT,
	SF_GRAPHICS_FORMAT_R32G32_UINT,
	SF_GRAPHICS_FORMAT_R32G32_SFLOAT,
	SF_GRAPHICS_FORMAT_R8G8B8_UNORM,
	SF_GRAPHICS_FORMAT_R16G16B16_UNORM,
	SF_GRAPHICS_FORMAT_R16G16B16_SFLOAT,
	SF_GRAPHICS_FORMAT_R32G32B32_UINT,
	SF_GRAPHICS_FORMAT_R32G32B32_SFLOAT,
	SF_GRAPHICS_FORMAT_B8G8R8A8_UNORM,
	SF_GRAPHICS_FORMAT_R8G8B8A8_UNORM,
	SF_GRAPHICS_FORMAT_R16G16B16A16_UNORM,
	SF_GRAPHICS_FORMAT_R16G16B16A16_SFLOAT,
	SF_GRAPHICS_FORMAT_R32G32B32A32_UINT,
	SF_GRAPHICS_FORMAT_R32G32B32A32_SFLOAT,
	SF_GRAPHICS_FORMAT_D16_UNORM,
	SF_GRAPHICS_FORMAT_X8_D24_UNORM_PACK32,
	SF_GRAPHICS_FORMAT_D32_SFLOAT,
	SF_GRAPHICS_FORMAT_S8_UINT,
	SF_GRAPHICS_FORMAT_D16_UNORM_S8_UINT,
	SF_GRAPHICS_FORMAT_D24_UNORM_S8_UINT,
	SF_GRAPHICS_FORMAT_D32_SFLOAT_S8_UINT
} SfGraphicsFormat;

typedef enum SfGraphicsDescriptorType {
	SF_GRAPHICS_DESCRIPTOR_TYPE_SAMPLER,
	SF_GRAPHICS_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	SF_GRAPHICS_DESCRIPTOR_TYPE_TEXTURE,
	SF_GRAPHICS_DESCRIPTOR_TYPE_COUNT
} SfGraphicsDescriptorType;

typedef enum SfGraphicsSampleCount {
	SF_GRAPHICS_SAMPLE_COUNT_1  = 1,
	SF_GRAPHICS_SAMPLE_COUNT_2  = 2,
	SF_GRAPHICS_SAMPLE_COUNT_4  = 4,
	SF_GRAPHICS_SAMPLE_COUNT_8  = 8,
	SF_GRAPHICS_SAMPLE_COUNT_16 = 16
} SfGraphicsSampleCount;

typedef enum SfGraphicsShaderStage {
	SF_GRAPHICS_SHADER_STAGE_NONE			 = 0X00000000,
	SF_GRAPHICS_SHADER_STAGE_VERTEX			 = 0X00000001,
	SF_GRAPHICS_SHADER_STAGE_TESSELLATION_CONTROL	 = 0X00000002,
	SF_GRAPHICS_SHADER_STAGE_TESSELLATION_EVALUATION = 0X00000004,
	SF_GRAPHICS_SHADER_STAGE_GEOMETRY		 = 0X00000008,
	SF_GRAPHICS_SHADER_STAGE_FRAGMENT		 = 0X00000010,
	SF_GRAPHICS_SHADER_STAGE_COMPUTE		 = 0X00000020
} SfGraphicsShaderStage;
typedef uint32_t SfGraphicsShaderStageFlags;

typedef enum SfGraphicsPrimitiveTopology {
	SF_GRAPHICS_PRIMITIVE_TOPOLOGY_POINT_LIST,
	SF_GRAPHICS_PRIMITIVE_TOPOLOGY_LINE_LIST,
	SF_GRAPHICS_PRIMITIVE_TOPOLOGY_LINE_STRIP,
	SF_GRAPHICS_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	SF_GRAPHICS_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
	SF_GRAPHICS_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
	SF_GRAPHICS_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
	SF_GRAPHICS_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY,
	SF_GRAPHICS_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
	SF_GRAPHICS_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY,
	SF_GRAPHICS_PRIMITIVE_TOPOLOGY_PATCH_LIST
} SfGraphicsPrimitiveTopology;

typedef enum SfGraphicsCullMode {
	SF_GRAPHICS_CULL_MODE_NONE = 0,
	SF_GRAPHICS_CULL_MODE_FRONT,
	SF_GRAPHICS_CULL_MODE_BACK,
	SF_GRAPHICS_CULL_MODE_FRONT_AND_BACK
} SfGraphicsCullMode;

typedef enum SfGraphicsFrontFace {
	SF_GRAPHICS_FRONT_FACE_CLOCKWISE,
	SF_GRAPHICS_FRONT_FACE_COUNTER_CLOCKWISE
} SfGraphicsFrontFace;

typedef enum SfGraphicsPipelineType {
	SF_GRAPHICS_PIPELINE_TYPE_GRAPHICS,
	SF_GRAPHICS_PIPELINE_TYPE_COMPUTE,
} SfGraphicsPipelineType;

typedef enum SfGraphicsIndexType {
	SF_GRAPHICS_INDEX_TYPE_U16,
	SF_GRAPHICS_INDEX_TYPE_U32
} SfGraphicsIndexType;

typedef enum SFPlataformType {
	SF_PLATAFORM_TYPE_MACOS,
	SF_PLATAFORM_TYPE_WINDOWS,
	SF_PLATAFORM_TYPE_LINUX
} SFPlataformType;

typedef enum SfGraphicsClearValueType {
	SF_GRAPHICS_CLEAR_VALUE_TYPE_NONE,
	SF_GRAPHICS_CLEAR_VALUE_TYPE_RGBA,
	SF_GRAPHICS_CLEAR_VALUE_TYPE_DEPTH,
} SfGraphicsClearValueType;

typedef struct SFPlataform {
	SFPlataformType type;
	GLFWwindow     *window;
} SFPlataform;

typedef struct SfGraphicsRGBA {
	float r;
	float g;
	float b;
	float a;
} SfGraphicsRGBA;

typedef struct SfGraphicsDepthStencil {
	float	 depth;
	uint32_t stencil;
} SfGraphicsDepthStencil;

typedef union SfGraphicsClearValueData {
	SfGraphicsRGBA	       rgba;
	SfGraphicsDepthStencil depth;
} SfGraphicsClearValueData;

typedef struct SfGraphicsClearValue {
	SfGraphicsClearValueType type;
	SfGraphicsClearValueData data;
} SfGraphicsClearValue;

typedef struct SfGraphicsTextureDescription {
	SfGraphicsTextureType  type;
	SfGraphicsSampleCount  samples;
	SfGraphicsFormat       format;
	SfGraphicsTextureUsage usage;
	uint32_t	       width;
	uint32_t	       height;
	uint32_t	       depth;
	uint32_t	       mips;
	SfBool		       mapped;
	SfGraphicsClearValue   clearValue;
	VkImage		       vulkanNotOwnedImage;
} SfGraphicsTextureDescription;

typedef struct SfGraphicsTexture {
	SfQueue		       queue;
	SfGraphicsTextureType  type;
	SfGraphicsSampleCount  samples;
	SfGraphicsFormat       format;
	SfGraphicsTextureUsage usage;
	uint32_t	       width;
	uint32_t	       height;
	uint32_t	       depth;
	uint32_t	       mips;
	SfBool		       mapped;
	SfBool		       ownsImage;
	void		      *mappedData;
	SfGraphicsClearValue   clearValue;
	VkImageLayout	       vulkanLayout;
	VkImageAspectFlags     vulkanAspectFlags;
	VkImage		       vulkanImage;
	VkDeviceMemory	       vulkanMemory;
	VkImageView	       vulkanImageView;
	VkSampler	       vulkanSampler;
} SfGraphicsTexture;

typedef struct SfGraphicsBuffer {
	SfQueue	       queue;
	uint64_t       size;
	void	      *mappedData;
	VkBuffer       vulkanBuffer;
	VkDeviceMemory vulkanMemory;
} SfGraphicsBuffer;

typedef struct SfGraphicsDescriptor {
	SfGraphicsDescriptorType   type;
	SfGraphicsShaderStageFlags stageFlags;
	uint32_t		   binding;
	uint32_t		   entryCount;
	SfHandle		   entries[SF_GRAPHICS_MAX_DESCRIPTOR_ENTRY_COUNT];
} SfGraphicsDescriptor;

typedef struct SfGraphicsDescriptorSetDescription {
	uint32_t	     descriptorCount;
	SfGraphicsDescriptor descriptors[SF_GRAPHICS_MAX_DESCRIPTOR_COUNT];
} SfGraphicsDescriptorSetDescription;

typedef struct SfGraphicsDescriptorSet {
	SfQueue		      queue;
	uint32_t	      descriptorCount;
	SfGraphicsDescriptor  descriptors[SF_GRAPHICS_MAX_DESCRIPTOR_COUNT];
	VkDescriptorSetLayout vulkanDescriptorSetLayout;
	VkDescriptorPool      vulkanDescriptorPool;
	VkDescriptorSet	      vulkanDescriptorSet;
} SfGraphicsDescriptorSet;

typedef struct SfGraphicsCommandBuffer {
	SfQueue		queue;
	VkCommandBuffer vulkanCommandBuffer;
} SfGraphicsCommandBuffer;

typedef struct SfGraphicsProgramDescription {
	uint32_t    vertexCodeSize;
	void const *vertexCode;

	uint32_t    tessellationControlCodeSize;
	void const *tessellationControlCode;

	uint32_t    tessellationEvaluationCodeSize;
	void const *tessellationEvaluationCode;

	uint32_t    geometryCodeSize;
	void const *geometryCode;

	uint32_t    fragmentCodeSize;
	void const *fragmentCode;

	uint32_t    computeCodeSize;
	void const *computeCode;
} SfGraphicsProgramDescription;

typedef struct SfGraphicsProgram {
	SfQueue			   queue;
	SfGraphicsShaderStageFlags stageFlags;
	VkShaderModule		   vulkanVertexShader;
	VkShaderModule		   vulkanTessellationControlShader;
	VkShaderModule		   vulkanTessellationEvaluationShader;
	VkShaderModule		   vulkanGeometryShader;
	VkShaderModule		   vulkanComputeShader;
	VkShaderModule		   vulkanFragmentShader;
} SfGraphicsProgram;

typedef struct SfGraphicsVertexAttribute {
	SfGraphicsFormat format;
	uint32_t	 binding;
	uint32_t	 location;
	uint32_t	 offset;
} SfGraphicsVertexAttribute;

typedef struct SfGraphicsVertexLayout {
	uint32_t		   count;
	SfGraphicsVertexAttribute *attributes;
} SfGraphicsVertexLayout;

typedef struct SfGraphicsPipelineDescription {
	SfGraphicsPipelineType	    type;
	SfGraphicsVertexLayout	    vertexLayout;
	SfGraphicsPrimitiveTopology topology;
	uint32_t		    patchControlPointCount;
	SfGraphicsCullMode	    cullMode;
	SfGraphicsFrontFace	    frontFace;
	SfBool			    useDepthStencil;
	SfHandle		    descriptorSet;
	SfHandle		    program;
	SfHandle		    renderTarget;
} SfGraphicsPipelineDescription;

typedef struct SfGraphicsPipeline {
	SfQueue		       queue;
	SfGraphicsPipelineType type;
	VkPipelineLayout       vulkanPipelineLayout;
	VkPipeline	       vulkanPipeline;
} SfGraphicsPipeline;

typedef struct SfGraphicsRenderTargetDescription {
	SfGraphicsSampleCount sampleCount;
	SfGraphicsFormat      colorFormat;
	SfGraphicsFormat      depthStencilFormat;
	uint32_t	      width;
	uint32_t	      height;
	uint32_t	      colorAttachmentCount;
	SfGraphicsClearValue  colorAttachmentClearValues[SF_GRAPHICS_MAX_ATTACHMENT_COUNT];
	SfGraphicsClearValue  depthStencilAttachmentClearValue;
	VkImage		      vulkanNotOwnedImage;
} SfGraphicsRenderTargetDescription;

typedef struct SfGraphicsRenderTarget {
	SfQueue		      queue;

	SfGraphicsSampleCount samples;
	SfGraphicsFormat      colorFormat;
	SfGraphicsFormat      depthStencilFormat;

	uint32_t	      width;
	uint32_t	      height;

	SfGraphicsClearValue  depthStencilAttachmentClearValue;
	SfHandle	      depthStencilAttachment;
	SfHandle	      depthStencilMultisamplingAttachment;

	uint32_t	      colorAttachmentCount;
	SfGraphicsClearValue  colorAttachmentClearValues[SF_GRAPHICS_MAX_ATTACHMENT_COUNT];
	SfHandle	      colorAttachments[SF_GRAPHICS_MAX_ATTACHMENT_COUNT];

	uint32_t	      colorMultisampleAttachmentCount;
	SfHandle	      colorMultisampleAttachments[SF_GRAPHICS_MAX_ATTACHMENT_COUNT];

	VkImage		      vulkanSwapchainImage;
	VkRenderPass	      vulkanRenderPass;
	VkFramebuffer	      vulkanFramebuffer;
} SfGraphicsRenderTarget;

typedef struct SfGraphicsSemaphore {
	SfQueue	    queue;
	VkSemaphore vulkanSemaphore;
} SfGraphicsSemaphore;

typedef struct SfGraphicsFence {
	SfQueue queue;
	VkFence vulkanFence;
} SfGraphicsFence;

typedef struct SfGraphicsQueue {
	uint32_t vulkanQueueFamilyIndex;
	VkQueue	 vulkanQueue;
} SfGraphicsQueue;

typedef struct SfGraphicsCommandPool {
	SfQueue	      queue;
	VkCommandPool vulkanCommandPool;
} SfGraphicsCommandPool;

typedef struct SfGraphicsRendererDescription {
	SFPlataform			     plataform;
	SfArena				    *arena;
	uint32_t			     swapchainWidth;
	uint32_t			     swapchainHeight;
	uint32_t			     swapchainImageCount;
	SfGraphicsClearValue		     swapchainColorClearValue;
	SfGraphicsClearValue		     swapchainDepthStencilClearValue;
	SfGraphicsSampleCount		     sampleCount;
	SfGraphicsFormat		     colorAttachmentFormat;
	SfGraphicsFormat		     depthStencilFormat;
	uint32_t			     bufferingCount;
	SfBool				     enableVSYNC;

	char const			    *applicationName;

	uint32_t			     vulkanInstanceLayerCount;
	char const			   **vulkanInstanceLayers;

	uint32_t			     vulkanInstanceExtensionCount;
	char const			   **vulkanInstanceExtensions;

	uint32_t			     vulkanDeviceExtensionCount;
	char const			   **vulkanDeviceExtensions;

	VkAllocationCallbacks		    *vulkanAllocationCallbacks;
	PFN_vkDebugUtilsMessengerCallbackEXT vulkanDebugCallback;
} SfGraphicsRendererDescription;

typedef struct SfGraphicsRenderer {
	SfArena				     arena;
	SFPlataform			     plataform;

	SfGraphicsSampleCount		     samples;
	SfGraphicsFormat		     colorAttachmentFormat;
	SfGraphicsFormat		     depthStencilFormat;

	uint32_t			     bufferingCount;
	SfBool				     enableVSYNC;

	SfGraphicsQueue			     graphicsQueue;
	SfGraphicsQueue			     presentQueue;

	SfQueue				     textureQueue;
	SfQueue				     freeTextureQueue;

	SfQueue				     bufferQueue;
	SfQueue				     freeBufferQueue;

	SfQueue				     commandPoolQueue;
	SfQueue				     freeCommandPoolQueue;

	SfQueue				     commandBufferQueue;
	SfQueue				     freeCommandBufferQueue;

	SfQueue				     semaphoreQueue;
	SfQueue				     freeSemaphoreQueue;

	SfQueue				     fenceQueue;
	SfQueue				     freeFenceQueue;

	SfArena				     renderTargetArena;
	SfQueue				     renderTargetQueue;
	SfQueue				     freeRenderTargetQueue;

	SfQueue				     programQueue;
	SfQueue				     freeProgramQueue;

	SfQueue				     descriptorSetQueue;
	SfQueue				     freeDescriptorSetQueue;

	SfQueue				     errorQueue;

	SfQueue				     pipelineQueue;
	SfQueue				     freePipelineQueue;

	uint32_t			     swapchainWidth;
	uint32_t			     swapchainHeight;
	uint32_t			     swapchainImageCount;
	SfGraphicsClearValue		     swapchainColorClearValue;
	SfGraphicsClearValue		     swapchainDepthStencilClearValue;
	SfBool				     swapchainSkipEndFrame;
	uint32_t			     swapchainCurrentImageIndex;

	uint32_t			     swapchainRenderTargetCount;
	SfHandle			     swapchainRenderTargets[SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT];

	uint32_t			     imageAcquiredSemaphoreCount;
	SfHandle			     imageAcquiredSemaphores[SF_GRAPHICS_MAX_BUFFERING_COUNT];

	uint32_t			     inFlightFenceCount;
	SfHandle			     inFlightFences[SF_GRAPHICS_MAX_BUFFERING_COUNT];

	uint32_t			     drawCompleteSemaphoreCount;
	SfHandle			     drawCompleteSemaphores[SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT];

	char const			    *applicationName;

	uint32_t			     vulkanInstanceLayerCount;
	char const			   **vulkanInstanceLayers;

	uint32_t			     vulkanInstanceExtensionCount;
	char const			   **vulkanInstanceExtensions;

	uint32_t			     vulkanDeviceExtensionCount;
	char const			   **vulkanDeviceExtensions;

	VkAllocationCallbacks		    *vulkanAllocationCallbacks;
	PFN_vkDebugUtilsMessengerCallbackEXT vulkanDebugCallback;

	VkInstance			     vulkanInstance;
	VkDebugUtilsMessengerEXT	     vulkanValidationMessenger;
	VkFormat			     vulkanSurfaceFormat;
	VkFormat			     vulkanDepthStencilFormat;
	VkPresentModeKHR		     vulkanPresentMode;
	VkColorSpaceKHR			     vulkanSurfaceColorSpace;
	VkSurfaceKHR			     vulkanSurface;
	VkSampleCountFlagBits		     vulkanSamples;
	VkPhysicalDevice		     vulkanPhysicalDevice;
	VkDevice			     vulkanDevice;
	VkSwapchainKHR			     vulkanSwapchain;
	uint32_t			     vulkanSwapchainImageCount;
	VkImage				     vulkanSwapchainImages[SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT];
	PFN_vkCreateDebugUtilsMessengerEXT   vulkanCreateDebugUtilsMessengerEXT;
	PFN_vkDestroyDebugUtilsMessengerEXT  vulkanDestroyDebugUtilsMessengerEXT;
} SfGraphicsRenderer;

SfGraphicsRenderer *sfCreateGraphicsRenderer(SfGraphicsRendererDescription const *description);
void sfDestroyGraphicsRenderer(SfGraphicsRenderer *renderer);

SfHandle sfGetGraphicsQueueHandle(SfGraphicsRenderer *renderer);
SfHandle sfGetPresentQueueHandle(SfGraphicsRenderer *renderer);

SfHandle sfCreateGraphicsSemaphore(SfGraphicsRenderer *renderer);
void sfDestroyGraphicsSemaphore(SfGraphicsRenderer *renderer, SfHandle semaphore);

SfHandle sfCreateGraphicsFence(SfGraphicsRenderer *renderer);
void sfDestroyGraphicsFence(SfGraphicsRenderer *renderer, SfHandle fence);

SfHandle sfCreateGraphicsBuffer(SfGraphicsRenderer *renderer, SfGraphicsBufferUsage usage, SfBool mapped,
				uint64_t size);
void sfDestroyGraphicsBuffer(SfGraphicsRenderer *renderer, SfHandle buffer);

SfHandle sfCreateGraphicsTexture(SfGraphicsRenderer *renderer, SfGraphicsTextureDescription const *description);
void sfDestroyGraphicsTexture(SfGraphicsRenderer *renderer, SfHandle texture);

SfHandle sfCreateGraphicsRenderTarget(SfGraphicsRenderer		      *renderer,
				      SfGraphicsRenderTargetDescription const *description);
void sfDestroyGraphicsRenderTarget(SfGraphicsRenderer *renderer, SfHandle renderTarget);

SfHandle sfCreateGraphicsCommandPool(SfGraphicsRenderer *renderer, SfHandle queue, SfBool transient, SfBool reset);
void sfDestroyGraphicsCommandPool(SfGraphicsRenderer *renderer, SfHandle commandPool);

SfHandle sfCreateGraphicsProgram(SfGraphicsRenderer *renderer, SfGraphicsProgramDescription const *description);
void sfDestroyGraphicsProgram(SfGraphicsRenderer *renderer, SfHandle program);

SfHandle sfCreateGraphicsCommandBuffer(SfGraphicsRenderer *renderer, SfHandle commandPool, SfBool secondary);
void sfDestroyGraphicsCommandBuffer(SfGraphicsRenderer *renderer, SfHandle commandPool, SfHandle commandBuffer);

SfHandle sfCreateGraphicsDescriptorSet(SfGraphicsRenderer			*renderer,
				       SfGraphicsDescriptorSetDescription const *description);

SfBool sfBeginGraphicsCommand(SfGraphicsRenderer *renderer, SfHandle commandBuffer);
SfBool sfEndGraphicsCommand(SfGraphicsRenderer *renderer, SfHandle commandBuffer);
SfBool sfGraphicsQueueSubmitGraphicsCommand(SfGraphicsRenderer *renderer, SfHandle queue, uint32_t commandBufferCount,
					    SfHandle const *commandBuffers, uint32_t waitSemaphoreCount,
					    SfHandle const *waitSemaphore, uint32_t signalSemaphoreCount,
					    SfHandle const *signalSemaphore);

SfBool sfGraphicsQueuePresent(SfGraphicsRenderer *renderer, SfHandle queue, uint32_t waitSemaphoreCount,
			      SfHandle const *waitSemaphore);
SfBool sfGraphicsQueueWaitIdle(SfHandle queue);

#endif
