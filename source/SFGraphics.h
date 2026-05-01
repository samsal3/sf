#ifndef SF_GRAPHICS_RENDERER_H
#define SF_GRAPHICS_RENDERER_H

#include "SFCore.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#define SF_GRAPHICS_MAX_GARBAGE_ITEM_COUNT 64
#define SF_GRAPHICS_MAX_FRAMES_IN_FLIGHT_COUNT 3
#define SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT 3
#define SF_GRAPHICS_MAX_TEXTURE_COUNT 256
#define SF_GRAPHICS_MAX_RENDER_TARGET_ATTACHMENT_COUNT 4
#define SF_GRAPHICS_MAX_DESCRIPTOR_ENTRY_COUNT 4
#define SF_GRAPHICS_MAX_DESCRIPTOR_SET_DESCRIPTOR_COUNT 4
#define SF_GRAPHICS_MAX_VERTEX_LAYOUT_ATTRIBUTE_COUNT 4

typedef struct SFHandle {
   intptr_t value;
} SFHandle;

#define SF_NULL_HANDLE (SFHandle){0}
#define SF_AS_HANDLE(v)                                                                                                                    \
   (SFHandle) { v }

typedef struct SFGraphicsGLFWPlataform {
   GLFWwindow *window;
} SFGraphicsGLFWPlatform;

typedef struct SFGraphicsClearValueRGBA {
   float r;
   float g;
   float b;
   float a;
} SFGraphicsClearValueRGBA;

typedef struct SFGraphicsClearValueDepthStencil {
   float depth;
   U32   stencil;
} SFGraphicsClearValueDepthStencil;

typedef enum SFGraphicsClearValueType {
   SF_GRAPHICS_CLEAR_VALUE_TYPE_NONE,
   SF_GRAPHICS_CLEAR_VALUE_TYPE_COLOR,
   SF_GRAPHICS_CLEAR_VALUE_TYPE_DEPTH_STENCIL,
} SFGraphicsClearValueType;

typedef union SFGraphicsClearValueData {
   SFGraphicsClearValueRGBA         rgba;
   SFGraphicsClearValueDepthStencil depthStencil;
} SFGraphicsClearValueData;

typedef struct SFGraphicsClearValue {
   SFGraphicsClearValueType type;
   SFGraphicsClearValueData data;
} SFGraphicsClearValue;

typedef struct SFGraphicsResourcePool {
   SFQueue pool;
   SFQueue free;
} SFGraphicsResourcePool;

typedef enum SFGraphicsDescriptorType {
   SF_GRAPHICS_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
   SF_GRAPHICS_DESCRIPTOR_TYPE_TEXTURE,
   SF_GRAPHICS_DESCRIPTOR_TYPE_SAMPLER
} SFGraphicsDescriptorType;

typedef struct SFGraphicsDescriptor {
   SFGraphicsDescriptorType type;
   U32                      binding;
   U32                      slot;
   U32                      entryCount;
   SFHandle                 entries[SF_GRAPHICS_MAX_DESCRIPTOR_ENTRY_COUNT];
} SFGraphicsDescriptor;

typedef struct SFGraphicsDescriptorSet {
   SFQueue              queue;
   U32                  descriptorCount;
   SFGraphicsDescriptor descriptors[SF_GRAPHICS_MAX_DESCRIPTOR_SET_DESCRIPTOR_COUNT];

   VkDescriptorPool      vkDescriptorPool;
   VkDescriptorSetLayout vkDescriptorSetLayout;
   VkDescriptorSet       vkDescriptorSet;
};

typedef struct SFGraphicsVertexAttribute {
   SFGraphicsFormat format;
   U32              binding;
   U32              location;
   U32              offset;
} SFGraphicsVertexAttribute;

typedef struct SFGraphicsVertexLayout {
   U32                       attributeCount;
   SFGraphicsVertexAttribute attributes[SF_GRAPHICS_MAX_VERTEX_LAYOUT_ATTRIBUTE_COUNT];
} SFGraphicsVertexLayout;

typedef struct SFGraphicsPipeline {
   SFQueue          queue;
   VkShaderModule   vkVertexShader;
   VkShaderModule   vkFragmentShader;
   VkPipelineLayout vkPipelineLayout;
   VkPipeline       vkPipeline;
} SFGraphicsPipeline;

typedef enum SFGraphicsSampleCount {
   SF_GRAPHICS_SAMPLE_COUNT_1,
   SF_GRAPHICS_SAMPLE_COUNT_2,
   SF_GRAPHICS_SAMPLE_COUNT_4,
   SF_GRAPHICS_SAMPLE_COUNT_8,
   SF_GRAPHICS_SAMPLE_COUNT_16
} SFGraphicsSampleCount;

typedef enum SFGraphicsTextureType {
   SF_GRAPHICS_TEXTURE_TYPE_1D,
   SF_GRAPHICS_TEXTURE_TYPE_2D,
   SF_GRAPHICS_TEXTURE_TYPE_3D,
   SF_GRAPHICS_TEXTURE_TYPE_CUBE
} SFGraphicsTextureType;

typedef enum SFGraphicsTextureUsage {
   SF_GRAPHICS_TEXTURE_USAGE_UNDEFINED                = 0X00000000,
   SF_GRAPHICS_TEXTURE_USAGE_TRANSFER_SRC             = 0X00000001,
   SF_GRAPHICS_TEXTURE_USAGE_TRANSFER_DST             = 0X00000002,
   SF_GRAPHICS_TEXTURE_USAGE_SAMPLED                  = 0X00000004,
   SF_GRAPHICS_TEXTURE_USAGE_STORAGE                  = 0X00000008,
   SF_GRAPHICS_TEXTURE_USAGE_COLOR_ATTACHMENT         = 0X00000010,
   SF_GRAPHICS_TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT = 0X00000020,
   SF_GRAPHICS_TEXTURE_USAGE_RESOLVE_SRC              = 0X00000040,
   SF_GRAPHICS_TEXTURE_USAGE_RESOLVE_DST              = 0X00000080,
   SF_GRAPHICS_TEXTURE_USAGE_PRESENT                  = 0X00000100,
} SFGraphicsTextureUsage;
typedef U32 SFGraphicsTextureUsageFlags;

typedef enum SFGraphicsFormat {
   SF_GRAPHICS_FORMAT_UNDEFINED,
   // 1 channel
   SF_GRAPHICS_FORMAT_R8_UNORM,
   SF_GRAPHICS_FORMAT_R16_UNORM,
   SF_GRAPHICS_FORMAT_R16_UINT,
   SF_GRAPHICS_FORMAT_R16_SFLOAT,
   SF_GRAPHICS_FORMAT_R32_UINT,
   SF_GRAPHICS_FORMAT_R32_SFLOAT,
   // 2 channel
   SF_GRAPHICS_FORMAT_R8G8_UNORM,
   SF_GRAPHICS_FORMAT_R16G16_UNORM,
   SF_GRAPHICS_FORMAT_R16G16_SFLOAT,
   SF_GRAPHICS_FORMAT_R32G32_UINT,
   SF_GRAPHICS_FORMAT_R32G32_SFLOAT,
   // 3 channel
   SF_GRAPHICS_FORMAT_R8G8B8_UNORM,
   SF_GRAPHICS_FORMAT_R16G16B16_UNORM,
   SF_GRAPHICS_FORMAT_R16G16B16_SFLOAT,
   SF_GRAPHICS_FORMAT_R32G32B32_UINT,
   SF_GRAPHICS_FORMAT_R32G32B32_SFLOAT,
   // 4 channel
   SF_GRAPHICS_FORMAT_B8G8R8A8_UNORM,
   SF_GRAPHICS_FORMAT_R8G8B8A8_UNORM,
   SF_GRAPHICS_FORMAT_R16G16B16A16_UNORM,
   SF_GRAPHICS_FORMAT_R16G16B16A16_SFLOAT,
   SF_GRAPHICS_FORMAT_R32G32B32A32_UINT,
   SF_GRAPHICS_FORMAT_R32G32B32A32_SFLOAT,
   // Depth/stencil
   SF_GRAPHICS_FORMAT_D16_UNORM,
   SF_GRAPHICS_FORMAT_X8_D24_UNORM_PACK32,
   SF_GRAPHICS_FORMAT_D32_SFLOAT,
   SF_GRAPHICS_FORMAT_S8_UINT,
   SF_GRAPHICS_FORMAT_D16_UNORM_S8_UINT,
   SF_GRAPHICS_FORMAT_D24_UNORM_S8_UINT,
   SF_GRAPHICS_FORMAT_D32_SFLOAT_S8_UINT
} SFGraphicsFormat;

typedef struct SFGraphicsTexture {
   SFQueue                queue;
   SFGraphicsTextureType  type;
   SFGraphicsFormat       format;
   SFGraphicsSampleCount  samples;
   SFGraphicsTextureUsage usage;
   U32                    width;
   U32                    height;
   U32                    depth;
   U32                    mips;
   SFBool                 mapped;
   void                  *mappedData;
   SFGraphicsClearValue   clearValue;
   SFBool                 vkOwnsMemoryAndImage;
   VkImage                vkImage;
   VkDeviceMemory         vkMemory;
   VkImageView            vkImageView;
} SFGraphicsTexture;

typedef struct SFGraphicsRenderTarget {
   SFQueue               queue;
   SFGraphicsSampleCount samples;

   U32 width;
   U32 height;

   U32 totalAttachmentCount;

   SFHandle depthStencilAttachment;

   U32      resolveAttachmentCount;
   SFHandle resolveAttachments[SF_GRAPHICS_MAX_RENDER_TARGET_ATTACHMENT_COUNT];

   U32      colorAttachmentCount;
   SFHandle colorAttachments[SF_GRAPHICS_MAX_RENDER_TARGET_ATTACHMENT_COUNT];

   VkRenderPass  vkRenderPass;
   VkFramebuffer vkFramebuffer;
} SFGraphicsRenderTarget;

typedef struct SFGraphicsCommandBuffer {
   SFQueue         queue;
   VkCommandPool   vkCommandPool;
   VkCommandBuffer vkCommandBuffer;
} SFGraphicsCommandBuffer;

typedef struct SFGraphicsRendererDescription {
   SFBool enableVSYNC;
   U32    width;
   U32    height;

   void (*createVulkanSurface)(void *renderer);

   SFString applicationName;

   U32                vkInstanceExtensionCount;
   char const *const *vkInstanceExtensions;

   U32                vkInstanceLayerCount;
   char const *const *vkInstanceLayers;

   U32                vkDeviceExtensionCount;
   char const *const *vkDeviceExtensions;
} SFGraphicsRendererDescription;

typedef struct SFGraphicsRenderer {
   SFArena                             arena;
   VkInstance                          vkInstance;
   VkAllocationCallbacks              *vkAllocationCallbacks;
   PFN_vkCreateDebugUtilsMessengerEXT  vkCreateDebugUtilsMessengerEXT;
   PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
   VkDebugUtilsMessengerEXT            vkValidationMessenger;
   VkFormat                            vkDepthStencilFormat;
   SFBool                              enableVSYNC;
   VkPresentModeKHR                    vkPresentMode;
   VkSurfaceFormatKHR                  vkSurfaceFormat;
   VkSurfaceKHR                        vkSurface;
   VkSampleCountFlagBits               vkSamples;
   VkPhysicalDevice                    vkPhysicalDevice;
   VkDevice                            vkDevice;
   VkQueue                             vkGraphicsQueue;
   VkQueue                             vkPresentQueue;
   U32                                 vkPresentQueueFamilyIndex;
   U32                                 vkGraphicsQueueFamilyIndex;
   VkSwapchainKHR                      vkSwapchain;
   U32                                 vkSwapchainImageCount;
   VkImage                             vkSwapchainImages[SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT];
   U32                                 vkDrawCompleteSemaphoreCount;
   VkSemaphore                         vkDrawCompleteSemaphores[SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT];
   SFGraphicsSampleCount               swapchainSampleCount;
   SFGraphicsFormat                    swapchainColorFormat;
   SFGraphicsFormat                    swapchainDepthStencilFormat;
   SFGraphicsClearValue                swapchainColorClearValue;
   SFGraphicsClearValue                swapchainDepthStencilClearValue;
   U32                                 swapchainRequestedImageCount;
   SFBool                              swapchainSkipEndFrame;
   U32                                 swapchainWidth;
   U32                                 swapchainHeight;
   U32                                 swapchainRenderTargetCount;
   SFHandle                            swapchainRenderTargets[SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT];
   U32                                 vkInFlightFenceCount;
   VkFence                             vkInFlightFences[SF_GRAPHICS_MAX_FRAMES_IN_FLIGHT_COUNT];
   U32                                 vkImageAcquiredSemaphoreCount;
   VkSemaphore                         vkImageAcquiredSemaphores[SF_GRAPHICS_MAX_FRAMES_IN_FLIGHT_COUNT];
   U32                                 currentSwapchainImageIndex;
   U32                                 currentFrameIndex;
   U32                                 mainCommandBufferCount;
   SFHandle                            mainCommandBuffers[SF_GRAPHICS_MAX_FRAMES_IN_FLIGHT_COUNT];
   SFArena                             renderTargetArena;
   SFGraphicsResourcePool              texturePool;
   SFGraphicsResourcePool              bufferPool;
   SFGraphicsResourcePool              renderTargetPool;
   SFGraphicsResourcePool              commandBufferPool;
   SFGraphicsResourcePool              descriptorSetPool;
   SFGraphicsResourcePool              pipelinePool;
} SFGraphicsRenderer;

SF_EXTERNAL SFGraphicsRenderer *sfGraphicsCreateRenderer(SFArena *arena, SFGraphicsRendererDescription *description);

SF_EXTERNAL void sfGraphicsDestroyRenderer(SFGraphicsRenderer *renderer);

SF_EXTERNAL SFHandle sfGraphicsCreateTexture(SFGraphicsRenderer *renderer, SFGraphicsTextureType type, SFGraphicsFormat format,
                                             SFGraphicsSampleCount samples, SFGraphicsTextureUsage usage, U32 width, U32 height, U32 depth,
                                             U32 mips, SFBool mapped, SFGraphicsClearValue *clearValue);

SF_EXTERNAL void sfGraphicsDestroyTexture(SFGraphicsRenderer *renderer, SFHandle handle);

SF_EXTERNAL SFHandle sfGraphicsCreateCommandBuffer(SFGraphicsRenderer *renderer, SFBool transient);

SF_EXTERNAL void sfGraphicsDestroyCommandBuffer(SFGraphicsRenderer *renderer, SFHandle handle);

SF_EXTERNAL SFHandle sfGraphicsCreateRenderTarget(SFGraphicsRenderer *renderer, U32 width, U32 height, SFGraphicsSampleCount samples,
                                                  SFGraphicsFormat colorFormat, SFGraphicsFormat depthStencilFormat,
                                                  U32 colorAttachmentCount, SFGraphicsClearValue *colorClearValues,
                                                  SFGraphicsClearValue *depthStencilClearValue);

SF_EXTERNAL void sfGraphicsDestroyRenderTarget(SFGraphicsRenderer *renderer, SFHandle handle);

SF_EXTERNAL SFHandle sfGraphicsCreatePipeline(SFGraphicsRenderer *renderer, SFGraphicsVertexLayout *vertexLayout,
                                              SFGraphicsDescriptorSetLayout *descriptorSetLayout, U32 vertexCodeSize,
                                              void const *vertexCode, U32 fragmentCodeSize, void const *fragmentCode);

SF_EXTERNAL void sfGraphicsDestroyPipeline(SFGraphicsRenderer *renderer, SFHandle handle);

#endif
