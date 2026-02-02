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
#define SF_NULL_HANDLE 0

#define SF_AS_HANDLE(p) ((sf_handle)(p))

typedef intptr_t sf_handle;

typedef enum sf_graphics_buffer_usage {
   SF_GRAPHICS_BUFFER_USAGE_NONE,
   SF_GRAPHICS_BUFFER_USAGE_VERTEX,
   SF_GRAPHICS_BUFFER_USAGE_INDEX,
} sf_graphics_buffer_usage;

typedef enum sf_graphics_texture_type {
   SF_GRAPHICS_TEXTURE_TYPE_1D,
   SF_GRAPHICS_TEXTURE_TYPE_2D,
   SF_GRAPHICS_TEXTURE_TYPE_3D,
   SF_GRAPHICS_TEXTURE_TYPE_CUBE
} sf_graphics_texture_type;

typedef enum sf_graphics_texture_usage {
   SF_GRAPHICS_TEXTURE_USAGE_UNDEFINED = 0X00000000,
   SF_GRAPHICS_TEXTURE_USAGE_TRANSFER_SRC = 0X00000001,
   SF_GRAPHICS_TEXTURE_USAGE_TRANSFER_DST = 0X00000002,
   SF_GRAPHICS_TEXTURE_USAGE_SAMPLED = 0X00000004,
   SF_GRAPHICS_TEXTURE_USAGE_STORAGE = 0X00000008,
   SF_GRAPHICS_TEXTURE_USAGE_COLOR_ATTACHMENT = 0X00000010,
   SF_GRAPHICS_TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT = 0X00000020,
   SF_GRAPHICS_TEXTURE_USAGE_RESOLVE_SRC = 0X00000040,
   SF_GRAPHICS_TEXTURE_USAGE_RESOLVE_DST = 0X00000080,
   SF_GRAPHICS_TEXTURE_USAGE_PRESENT = 0X00000100,
} sf_graphics_texture_usage;
typedef uint32_t sf_graphics_texture_usage_flags;

typedef enum sf_graphics_format {
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
} sf_graphics_format;

typedef enum sf_graphics_descriptor_type {
   SF_GRAPHICS_DESCRIPTOR_TYPE_SAMPLER,
   SF_GRAPHICS_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
   SF_GRAPHICS_DESCRIPTOR_TYPE_TEXTURE,
   SF_GRAPHICS_DESCRIPTOR_TYPE_COUNT
} sf_graphics_descriptor_type;

typedef enum sf_graphics_sample_count {
   SF_GRAPHICS_SAMPLE_COUNT_1 = 1,
   SF_GRAPHICS_SAMPLE_COUNT_2 = 2,
   SF_GRAPHICS_SAMPLE_COUNT_4 = 4,
   SF_GRAPHICS_SAMPLE_COUNT_8 = 8,
   SF_GRAPHICS_SAMPLE_COUNT_16 = 16
} sf_graphics_sample_count;

typedef enum sf_graphics_shader_stage {
   SF_GRAPHICS_SHADER_STAGE_NONE = 0X00000000,
   SF_GRAPHICS_SHADER_STAGE_VERTEX = 0X00000001,
   SF_GRAPHICS_SHADER_STAGE_TESSELLATION_CONTROL = 0X00000002,
   SF_GRAPHICS_SHADER_STAGE_TESSELLATION_EVALUATION = 0X00000004,
   SF_GRAPHICS_SHADER_STAGE_GEOMETRY = 0X00000008,
   SF_GRAPHICS_SHADER_STAGE_FRAGMENT = 0X00000010,
   SF_GRAPHICS_SHADER_STAGE_COMPUTE = 0X00000020
} sf_graphics_shader_stage;
typedef uint32_t sf_graphics_shader_stage_flags;

typedef enum sf_graphics_primitive {
   SF_GRAPHICS_PRIMITIVE_POINT_LIST
} sf_graphics_primitive;

typedef enum sf_graphics_culling_mode {
   SF_GRAPHICS_CULLING_MODE_NONE = 0
} sf_graphics_culling_mode;

typedef enum sf_graphics_front {
   SF_GRAPHICS_FRONT_CLOCKWISE,
   SF_GRAPHICS_FRONT_COUNTER_CLOCKWISE
} sf_graphics_front;

typedef enum sf_graphics_pipeline_type {
   SF_GRAPHICS_PIPELINE_TYPE_GRAPHICS,
   SF_GRAPHICS_PIPELINE_TYPE_COMPUTE,
} sf_graphics_pipeline_type;

typedef enum sf_graphics_index_type {
   SF_GRAPHICS_INDEX_TYPE_U16,
   SF_GRAPHICS_INDEX_TYPE_U32
} sf_graphics_index_type;

typedef enum sf_api_os {
   SF_API_OS_MACOS,
   SF_API_OS_WINDOWS,
   SF_API_OS_LINUX
} sf_api_ois;

typedef struct sf_api {
   enum sf_api_os os;
   GLFWwindow *window;
} sf_api;

typedef struct sf_graphics_rgba {
   float r;
   float g;
   float b;
   float a;
} sf_graphics_rgba;

typedef struct sf_graphics_depth {
   float depth;
   uint32_t stencil;
} sf_graphics_depth;

typedef union sf_graphics_clear_value_data {
   sf_graphics_rgba rgba;
   sf_graphics_depth depth;
} sf_graphics_clear_value_data;

typedef enum sf_graphics_clear_value_type {
   SF_GRAPHICS_CLEAR_VALUE_TYPE_NONE,
   SF_GRAPHICS_CLEAR_VALUE_TYPE_RGBA,
   SF_GRAPHICS_CLEAR_VALUE_TYPE_DEPTH,
} sf_graphics_clear_value_type;

typedef struct sf_graphics_clear_value {
   sf_graphics_clear_value_type type;
   sf_graphics_clear_value_data data;
} sf_graphics_clear_value;

typedef struct sf_graphics_texture_description {
   sf_graphics_texture_type type;
   sf_graphics_sample_count sample_count;
   sf_graphics_format format;
   sf_graphics_texture_usage usage;
   uint32_t width;
   uint32_t height;
   uint32_t depth;
   uint32_t mips;
   sf_bool mapped;
   sf_graphics_clear_value clear_value;
   VkImage vk_not_owned_image;
} sf_graphics_texture_description;

typedef struct sf_graphics_texture {
   struct sf_queue queue;
   sf_graphics_texture_type type;
   sf_graphics_sample_count sample_count;
   sf_graphics_format format;
   sf_graphics_texture_usage usage;
   uint32_t width;
   uint32_t height;
   uint32_t depth;
   uint32_t mips;
   sf_bool mapped;
   sf_bool owns_image;
   void *mapped_data;
   sf_graphics_clear_value clear_value;
   VkImageLayout vk_layout;
   VkImageAspectFlags vk_aspect;
   VkImage vk_image;
   VkDeviceMemory vk_memory;
   VkImageView vk_image_view;
   VkSampler vk_sampler;
} sf_graphics_texture;

typedef struct sf_graphics_buffer {
   struct sf_queue queue;
   uint64_t size;
   void *mapping;
   VkBuffer vk_buffer;
   VkDeviceMemory vk_memory;
} sf_graphics_buffer;

typedef struct sf_graphics_descriptor {
   enum sf_graphics_descriptor_type type;
   sf_graphics_shader_stage_flags stages;
   uint32_t binding;
   uint32_t entry_count;
   sf_handle entries[SF_GRAPHICS_MAX_DESCRIPTOR_ENTRY_COUNT];
} sf_graphics_descriptor;

typedef struct sf_graphics_descriptor_set_description {
   uint32_t descriptor_count;
   struct sf_graphics_descriptor descriptors[SF_GRAPHICS_MAX_DESCRIPTOR_COUNT];
} sf_graphics_descriptor_set_description;

typedef struct sf_graphics_descriptor_set {
   struct sf_queue queue;
   uint32_t descriptor_count;
   sf_graphics_descriptor descriptors[SF_GRAPHICS_MAX_DESCRIPTOR_COUNT];
   VkDescriptorSetLayout vk_descriptor_set_layout;
   VkDescriptorPool vk_descriptor_pool;
   VkDescriptorSet vk_descriptor_set;
} sf_graphics_descriptor_set;

typedef struct sf_graphics_command_buffer {
   struct sf_queue queue;
   VkCommandBuffer vk_command_buffer;
} sf_graphics_command_buffer;

typedef struct sf_graphics_program_description {
   uint32_t vertex_code_size;
   void const *vertex_code;
   uint32_t tesselation_control_code_size;
   void const *tesselation_control_code;
   uint32_t tesselation_evaluation_code_size;
   void const *tesselation_evaluation_code;
   uint32_t geometry_code_size;
   void const *geometry_code;
   uint32_t fragment_code_size;
   void const *fragment_code;
   uint32_t compute_code_size;
   void const *compute_code;
} sf_graphics_program_description;

typedef struct sf_graphics_program {
   struct sf_queue queue;
   sf_graphics_shader_stage_flags stages;
   VkShaderModule vk_vertex_shader;
   VkShaderModule vk_tesselation_control_shader;
   VkShaderModule vk_tesselation_evaluation_shader;
   VkShaderModule vk_geometry_shader;
   VkShaderModule vk_compute_shader;
   VkShaderModule vk_fragment_shader;
} sf_graphics_program;

typedef struct sf_graphics_vertex_attribute {
   sf_graphics_format format;
   uint32_t binding;
   uint32_t location;
   uint32_t offset;
} sf_graphics_vertex_attribute;

typedef struct sf_graphics_vertex_layout {
   uint32_t count;
   sf_graphics_vertex_attribute *attributes;
} sf_graphics_vertex_layout;

typedef struct sf_graphics_pipeline_description {
   sf_graphics_vertex_layout vertex_layout;
   sf_graphics_primitive topology;
   sf_graphics_culling_mode culling;
   sf_graphics_front face;
   sf_bool use_depth_stencil;
   sf_handle descriptor_set;
   sf_handle program;
} sf_graphics_pipeline_description;

typedef struct sf_graphics_pipeline {
   sf_queue queue;
   sf_graphics_pipeline_type type;
   VkPipelineLayout vk_pipeline_layout;
   VkPipeline vk_pipeline;
} sf_graphics_pipeline;

typedef struct sf_graphics_render_target_description {
   sf_graphics_sample_count sample_count;
   sf_graphics_format color_format;
   sf_graphics_format depth_stencil_format;
   uint32_t width;
   uint32_t height;
   uint32_t color_attachment_count;
   sf_graphics_clear_value color_attachment_clear_values[SF_GRAPHICS_MAX_ATTACHMENT_COUNT];
   sf_graphics_clear_value depth_stencil_attachment_clear_value;
   VkImage vk_not_owned_color_image;
} sf_graphics_render_target_description;

typedef struct sf_graphics_render_target {
   struct sf_queue queue;
   sf_graphics_sample_count sample_count;
   sf_graphics_format color_format;
   sf_graphics_format depth_stencil_format;
   uint32_t width;
   uint32_t height;
   uint32_t color_attachment_clear_value_count;
   sf_graphics_clear_value color_attachment_clear_values[SF_GRAPHICS_MAX_ATTACHMENT_COUNT];
   sf_graphics_clear_value depth_stencil_attachment_clear_value;
   sf_handle depth_stencil_attachment;
   sf_handle depth_stencil_multisampling_attachment;
   uint32_t color_attachment_count;
   sf_handle color_attachments[SF_GRAPHICS_MAX_ATTACHMENT_COUNT];
   uint32_t color_multisample_attachment_count;
   sf_handle color_multisample_attachments[SF_GRAPHICS_MAX_ATTACHMENT_COUNT];
   VkImage vk_swapchain_image;
   VkRenderPass vk_render_pass;
   VkFramebuffer vk_framebuffer;
} sf_graphics_render_target;

typedef struct sf_graphics_semaphore {
   sf_queue queue;
   VkSemaphore vk_semaphore;
} sf_graphics_semaphore;

typedef struct sf_graphics_fence {
   sf_queue queue;
   VkFence vk_fence;
} sf_graphics_fence;

typedef struct sf_graphics_queue {
   uint32_t vk_queue_family_index;
   VkQueue vk_queue;
} sf_graphics_queue;

typedef struct sf_graphics_command_pool {
   sf_queue queue;
   VkCommandPool vk_command_pool;
} sf_graphics_command_pool;

typedef struct sf_graphics_renderer_description {
   sf_api api;
   sf_arena *arena;
   uint32_t swapchain_width;
   uint32_t swapchain_height;
   uint32_t swapchain_image_count;
   sf_graphics_clear_value swapchain_color_clear_value;
   sf_graphics_clear_value swapchain_depth_stencil_clear_value;
   sf_graphics_sample_count sample_count;
   sf_graphics_format color_attachment_format;
   sf_graphics_format depth_stencil_format;
   uint32_t buffering_count;
   sf_bool enable_vsync;
   char const *application_name;
   uint32_t vk_instance_layer_count;
   char const **vk_instance_layers;
   uint32_t vk_instance_extension_count;
   char const **vk_instance_extensions;
   uint32_t vk_device_extension_count;
   char const **vk_device_extensions;
   VkAllocationCallbacks *vk_allocation_callbacks;
   PFN_vkDebugUtilsMessengerCallbackEXT vk_debug_callback;
} sf_graphics_renderer_description;

typedef struct sf_graphics_renderer {
   sf_arena arena;
   sf_api api;
   sf_graphics_sample_count sample_count;
   sf_graphics_format color_attachment_format;
   sf_graphics_format depth_stencil_format;
   uint32_t buffering_count;
   sf_bool enable_vsync;
   sf_graphics_queue graphics_queue;
   sf_graphics_queue present_queue;
   sf_queue texture_queue;
   sf_queue free_texture_queue;
   sf_queue buffer_queue;
   sf_queue free_buffer_queue;
   sf_queue command_pool_queue;
   sf_queue free_command_pool_queue;
   sf_queue command_buffer_queue;
   sf_queue free_command_buffer_queue;
   sf_queue semaphore_queue;
   sf_queue free_semaphore_queue;
   sf_queue fence_queue;
   sf_queue free_fence_queue;
   sf_arena render_target_arena;
   sf_queue render_target_queue;
   sf_queue free_render_target_queue;
   sf_queue program_queue;
   sf_queue free_program_queue;
   sf_queue descriptor_set_queue;
   sf_queue free_descriptor_set_queue;
   sf_queue error_queue;
   sf_queue pipeline_queue;
   sf_queue free_pipeline_queue;
   uint32_t swapchain_width;
   uint32_t swapchain_height;
   uint32_t swapchain_image_count;
   sf_graphics_clear_value swapchain_color_clear_value;
   sf_graphics_clear_value swapchain_depth_stencil_clear_value;
   sf_bool swapchain_skip_end_frame;
   uint32_t swapchain_current_image_index;
   uint32_t swapchain_render_target_count;
   sf_handle swapchain_render_targets[SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT];
   uint32_t image_acquired_semaphore_count;
   sf_handle image_acquired_semaphores[SF_GRAPHICS_MAX_BUFFERING_COUNT];
   uint32_t in_flight_fence_count;
   sf_handle in_flight_fences[SF_GRAPHICS_MAX_BUFFERING_COUNT];
   uint32_t draw_complete_semaphore_count;
   sf_handle draw_complete_semaphores[SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT];
   char const *application_name;
   uint32_t vk_instance_layer_count;
   char const **vk_instance_layers;
   uint32_t vk_instance_extension_count;
   char const **vk_instance_extensions;
   uint32_t vk_device_extension_count;
   char const **vk_device_extensions;
   VkAllocationCallbacks *vk_allocation_callbacks;
   PFN_vkDebugUtilsMessengerCallbackEXT vk_debug_callback;
   VkInstance vk_instance;
   VkDebugUtilsMessengerEXT vk_validation_messenger;
   VkFormat vk_surface_format;
   VkFormat vk_depth_stencil_format;
   VkPresentModeKHR vk_present_mode;
   VkColorSpaceKHR vk_surface_color_space;
   VkSurfaceKHR vk_surface;
   VkSampleCountFlagBits vk_sample_count;
   VkPhysicalDevice vk_physical_device;
   VkDevice vk_device;
   VkSwapchainKHR vk_swapchain;
   uint32_t vk_swapchain_image_count;
   VkImage vk_swapchain_images[SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT];
   PFN_vkCreateDebugUtilsMessengerEXT vk_create_debug_utils_messenger_ext;
   PFN_vkDestroyDebugUtilsMessengerEXT vk_destroy_debug_utils_messenger_ext;
} sf_graphics_renderer;

sf_graphics_renderer *sf_graphics_create_renderer(sf_graphics_renderer_description const *desc);
void sf_graphics_destroy_renderer(sf_graphics_renderer *r);

sf_handle sf_graphics_get_graphics_queue(sf_graphics_renderer *r);
sf_handle sf_graphics_get_present_queue(sf_graphics_renderer *r);

sf_handle sf_graphics_create_semaphore(sf_graphics_renderer *r);
void sf_graphics_destroy_semaphore(sf_graphics_renderer *r, sf_handle semaphore);

sf_handle sf_graphics_create_fence(sf_graphics_renderer *r);
void sf_graphics_destroy_fence(sf_graphics_renderer *r, sf_handle fence);

sf_handle sf_graphics_create_buffer(sf_graphics_renderer *r, sf_graphics_buffer_usage usage, sf_bool mapped, uint64_t size);
void sf_graphics_destroy_buffer(sf_graphics_renderer *r, sf_handle buffer);

sf_handle sf_graphics_create_texture(sf_graphics_renderer *r, sf_graphics_texture_description const *desc);
void sf_graphics_destroy_texture(sf_graphics_renderer *r, sf_handle texture);

sf_handle sf_graphics_create_render_target(sf_graphics_renderer *r, sf_graphics_render_target_description const *desc);
void sf_graphics_destroy_render_target(sf_graphics_renderer *r, sf_handle render_target);

sf_handle sf_graphics_create_command_pool(sf_graphics_renderer *r, sf_handle queue, sf_bool transient, sf_bool reset);
void sf_graphics_destroy_command_pool(sf_graphics_renderer *r, sf_handle command_pool);

sf_handle sf_graphics_create_program(sf_graphics_renderer *r, sf_graphics_program_description const *desc);
void sf_graphics_destroy_program(sf_graphics_renderer *r, sf_handle program);

sf_handle sf_graphics_create_command_buffer(sf_graphics_renderer *r, sf_handle command_pool, sf_bool secondary);
void sf_graphics_destroy_command_buffer(sf_graphics_renderer *r, sf_handle command_pool, sf_handle command_buffer);

sf_handle sf_graphics_create_descriptor_set(sf_graphics_renderer *r, sf_graphics_descriptor_set_description const *desc);

sf_bool sf_graphics_begin_command(sf_graphics_renderer *r, sf_handle command_buffer);
sf_bool sf_graphics_end_command(sf_graphics_renderer *r, sf_handle command_buffer);
sf_bool sf_graphics_queue_submit_command(sf_graphics_renderer *r, sf_handle queue, uint32_t command_buffer_count, sf_handle const *command_buffers, uint32_t wait_semaphore_count, sf_handle const *wait_semaphores, uint32_t signal_semaphore_count, sf_handle const *signal_semaphores);

sf_bool sf_graphics_queue_present(sf_graphics_renderer *r, sf_handle queue, uint32_t wait_semaphore_count, sf_handle *wait_semaphores);
sf_bool sf_graphics_queue_wait_idle(sf_handle queue);

sf_handle sf_graphics_create_and_begin_single_use_command_buffer(sf_graphics_renderer *r);

#endif
