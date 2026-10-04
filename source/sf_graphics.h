#ifndef SF_GRAPHICS_RENDERER_H
#define SF_GRAPHICS_RENDERER_H

#include "sf_core.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#define SF_GRAPHICS_MAX_GARBAGE_ITEM_COUNT 64
#define SF_GRAPHICS_MAX_FRAMES_IN_FLIGHT_COUNT 3
#define SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT 3
#define SF_GRAPHICS_MAX_TEXTURE_COUNT 256
#define SF_GRAPHICS_MAX_RENDER_TARGET_ATTACHMENT_COUNT 4
#define SF_GRAPHICS_MAX_DESCRIPTOR_ENTRY_COUNT 4
#define SF_GRAPHICS_MAX_DESCRIPTOR_SET_DESCRIPTOR_COUNT 4
#define SF_GRAPHICS_MAX_VERTEX_LAYOUT_ATTRIBUTE_COUNT 4
#define SF_GRAPHICS_MAX_RESOURCE_POOL_COUNT 16
#define SF_GRAPHICS_MAX_IMAGE_DESCRIPTOR_COUNT 8


struct sf_graphics_renderer;

typedef uintptr_t sf_handle;
#define SF_NULL_HANDLE 0

struct sf_graphics_resource_base {
	sf_bool is_occupied;
};

#define SF_GRAPHICS_CALCULATE_MIPS (u32)-1

enum sf_graphics_format {
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
	SF_GRAPHICS_FORMAT_B8G8R8A8_SRGB,
	SF_GRAPHICS_FORMAT_R8G8B8A8_SRGB,
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
};

enum sf_graphics_clear_value_type {
	SF_GRAPHICS_CLEAR_VALUE_TYPE_NONE,
	SF_GRAPHICS_CLEAR_VALUE_TYPE_COLOR,
	SF_GRAPHICS_CLEAR_VALUE_TYPE_DEPTH_STENCIL
};

enum sf_graphics_descriptor_type {
	SF_GRAPHICS_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	SF_GRAPHICS_DESCRIPTOR_TYPE_STORAGE_BUFFER,
	SF_GRAPHICS_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
	SF_GRAPHICS_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
	SF_GRAPHICS_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
	SF_GRAPHICS_DESCRIPTOR_TYPE_TEXTURE,
	SF_GRAPHICS_DESCRIPTOR_TYPE_SAMPLER
};

enum sf_graphics_sample_count {
	SF_GRAPHICS_SAMPLE_COUNT_1,
	SF_GRAPHICS_SAMPLE_COUNT_2,
	SF_GRAPHICS_SAMPLE_COUNT_4,
	SF_GRAPHICS_SAMPLE_COUNT_8,
	SF_GRAPHICS_SAMPLE_COUNT_16
};

enum sf_graphics_image_type {
	SF_GRAPHICS_IMAGE_TYPE_1D,
	SF_GRAPHICS_IMAGE_TYPE_2D,
	SF_GRAPHICS_IMAGE_TYPE_3D,
	SF_GRAPHICS_IMAGE_TYPE_CUBE
};

enum sf_graphics_image_usage {
	SF_GRAPHICS_IMAGE_USAGE_UNDEFINED = 0X00000000,
	SF_GRAPHICS_IMAGE_USAGE_TRANSFER_SOURCE = 0X00000001,
	SF_GRAPHICS_IMAGE_USAGE_TRANSFER_DESTINATION = 0X00000002,
	SF_GRAPHICS_IMAGE_USAGE_SAMPLED = 0X00000004,
	SF_GRAPHICS_IMAGE_USAGE_STORAGE = 0X00000008,
	SF_GRAPHICS_IMAGE_USAGE_COLOR_ATTACHMENT = 0X00000010,
	SF_GRAPHICS_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT = 0X00000020,
	SF_GRAPHICS_IMAGE_USAGE_RESOLVE_SOURCE = 0X00000040,
	SF_GRAPHICS_IMAGE_USAGE_RESOLVE_DESTINATION = 0X00000080,
	SF_GRAPHICS_IMAGE_USAGE_PRESENT = 0X00000100
};
typedef u32 sf_graphics_image_usage_flags;

enum sf_graphics_buffer_usage {
	SF_GRAPHICS_BUFFER_USAGE_TRANSFER_SOURCE = 0x00000001,
	SF_GRAPHICS_BUFFER_USAGE_TRANSFER_DESTINATION = 0x00000002,
	SF_GRAPHICS_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER = 0x00000004,
	SF_GRAPHICS_BUFFER_USAGE_STORAGE_TEXEL_BUFFER = 0x00000008,
	SF_GRAPHICS_BUFFER_USAGE_UNIFORM_BUFFER = 0x00000010,
	SF_GRAPHICS_BUFFER_USAGE_STORAGE_BUFFER = 0x00000020,
	SF_GRAPHICS_BUFFER_USAGE_INDEX_BUFFER = 0x00000040,
	SF_GRAPHICS_BUFFER_USAGE_VERTEX_BUFFER = 0x00000080,
	SF_GRAPHICS_BUFFER_USAGE_INDIRECT_BUFFER = 0x00000100,
	SF_GRAPHICS_BUFFER_USAGE_SHADER_DEVICE_ADDRESS = 0x00000200,
	SF_GRAPHICS_BUFFER_USAGE_DESCRIPTOR_HEAP = 0x00000400,
};
typedef u32 sf_graphics_buffer_usage_flags;

enum sf_graphics_memory_property {
	SF_GRAPHICS_MEMORY_PROPERTY_DEVICE_LOCAL = 0x00000001,
	SF_GRAPHICS_MEMORY_PROPERTY_CPU_VISIBLE = 0x00000002,
	SF_GRAPHICS_MEMORY_PROPERTY_LAZILY_ALLOCATED = 0x00000004,
	SF_GRAPHICS_MEMORY_PROPERTY_PROTECTED = 0x0000008,
	SF_GRAPHICS_MEMORY_PROPERTY_DEVICE_ADDRESS = 0x00000010
};
typedef u32 sf_graphics_memory_property_flags;

enum sf_graphics_command_buffer_usage {
	SF_GRAPHICS_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT = 0x0000001,
};
typedef u32 sf_graphics_command_buffer_usage_flags;


struct sf_graphics_device;

typedef void (*sf_graphics_init_surface_callback)(void *data, struct sf_graphics_device *device);
typedef void (*sf_graphics_request_surface_dimensions_callback)(void *data, u32 *width, u32 *height);

struct sf_graphics_init_device_info {
	struct sf_string application_name;

	void *plataform_data;
	sf_graphics_init_surface_callback init_surface;
	sf_graphics_request_surface_dimensions_callback request_surface_dimensions;

	struct {
		sf_bool request_enable_validation_layers;

		u32 instance_extension_count;
		char const **instance_extensions;

		u32 instance_layer_count;
		char const **instance_layers;

		u32 device_extension_count;
		char const **device_extensions;
		
		VkAllocationCallbacks const *allocation_callbacks;
	} vk;
};

struct sf_graphics_device {
	struct sf_graphics_init_device_info info;

	struct {
		VkAllocationCallbacks const *allocation_callbacks;
		VkInstance instance;
		VkDebugUtilsMessengerEXT validation_messenger;
		VkSurfaceKHR surface;

		VkPhysicalDevice physical_device;
		VkPhysicalDeviceProperties physical_device_properties;
		VkPhysicalDeviceFeatures physical_device_features;
		VkDevice device;

		u32 graphics_queue_family_index;
		u32 present_queue_family_index;
		u32 compute_queue_family_index;

		VkQueue graphics_queue;
		VkQueue present_queue;
		VkQueue compute_queue;

		VkSurfaceCapabilitiesKHR physical_device_surface_capabilities;

		VkDescriptorPool global_descriptor_pool;
		VkDescriptorSetLayout image_descriptor_set_layout;
		VkDescriptorSetLayout uniform_descriptor_set_layout;
		VkDescriptorSetLayout dynamic_uniform_descriptor_set_layout;
		VkPipelineLayout general_pipeline_layout;
		VkSampler linear_sampler;

		PFN_vkCreateDebugUtilsMessengerEXT create_debug_utils_messenger_ext;
		PFN_vkDestroyDebugUtilsMessengerEXT destroy_debug_utils_messenger_ext;
	} vk;
};

struct sf_graphics_memory_allocation {
	u32 size;
	struct {
		VkDeviceMemory memory;
	} vk;
};


struct sf_graphics_init_image_info {
	enum sf_graphics_image_type type;
	sf_graphics_image_usage_flags usage;
	enum sf_graphics_format format;
	enum sf_graphics_sample_count samples;
	sf_graphics_memory_property_flags memory_properties;
	u32 width;
	u32 height;
	u32 mips;

	struct {
		VkImage not_owned_image;
	} vk;
};

struct sf_graphics_image {
	struct sf_graphics_resource_base resource;
	struct sf_graphics_init_image_info info;

	struct sf_graphics_memory_allocation *memory;

	struct  {
		VkImage image;
		VkDeviceMemory memory;
		VkImageView image_view;
		VkDescriptorSet descriptor_set;
	} vk;
};

struct sf_graphics_init_buffer_info {
	u64 size;
	sf_graphics_buffer_usage_flags usage;
	sf_graphics_memory_property_flags memory_properties;
};


struct sf_graphics_buffer {
	struct sf_graphics_resource_base resource;
	struct sf_graphics_init_buffer_info info;

	void *cpu_mapped_data;
	sf_bool is_dynamic;

	struct {
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDescriptorSet descriptor_set;
	} vk;
};

struct sf_graphics_command_buffer {
	struct sf_graphics_resource_base resource;
	sf_bool is_recording;
	sf_bool is_executable;
	sf_graphics_command_buffer_usage_flags usage;

	struct {
		VkCommandPool command_pool;
		VkCommandBuffer command_buffer;
	} vk;
};

struct sf_graphics_semaphore {
	struct sf_graphics_resource_base resource;
	struct {
		VkSemaphore semaphore;
	} vk;
};

struct sf_graphics_fence {
	struct sf_graphics_resource_base resource;
	struct {
		VkFence fence;
	} vk;
};

struct sf_graphics_shader {
	struct {
		VkShaderModule shader;
	} vk;
};



struct sf_graphics_clear_value_rgba {
	float r;
	float g;
	float b;
	float a;
};

struct sf_graphics_clear_value_depth_stencil {
	float depth;
	u32 stencil;
};

union sf_graphics_clear_value_data {
	struct sf_graphics_clear_value_rgba rgba;
	struct sf_graphics_clear_value_depth_stencil depth_stencil;
};

struct sf_graphics_clear_value {
	enum sf_graphics_clear_value_type type;
	union sf_graphics_clear_value_data data;
};

struct sf_graphics_init_swapchain_info {
	u32 requested_image_count;
	sf_bool requested_enable_vsync;
	enum sf_graphics_sample_count sample_count;
	struct sf_graphics_clear_value color_clear_value;
	struct sf_graphics_clear_value depth_stencil_clear_value;
};

struct sf_graphics_swapchain {
	struct sf_graphics_init_swapchain_info info;

	u32 width;
	u32 height;

	sf_bool is_vsync_enabled;

	enum sf_graphics_format color_format;
	enum sf_graphics_format depth_stencil_format;

	u32 image_count;
	struct sf_graphics_image *images[SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT];

	u32 draw_complete_semaphore_count;
	struct sf_graphics_semaphore *draw_complete_semaphores[SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT];

	u32 render_target_count;
	struct sf_graphics_render_target *render_targets[SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT];

	u32 current_image_index;
	sf_bool requires_rebuild;

	struct {
		VkSwapchainKHR swapchain;
		VkColorSpaceKHR color_space;
		VkPresentModeKHR present_mode;
	} vk;
};

struct sf_graphics_init_render_target_info {
	u32 width; 
	u32 height; 
	enum sf_graphics_format color_attachment_format; 
	struct sf_graphics_image *color_attachment;
	enum sf_graphics_format depth_stencil_attachment_format;
	enum sf_graphics_sample_count sample_count;
	struct sf_graphics_clear_value color_clear_value;
	struct sf_graphics_clear_value depth_stencil_clear_value;
	sf_bool will_be_presented;
};

struct sf_graphics_render_target {
	struct sf_graphics_init_render_target_info info;

	struct sf_graphics_image *color_attachment;
	struct sf_graphics_image *resolve_attachment;
	struct sf_graphics_image *depth_stencil_attachment;

	struct {
		VkRenderPass render_pass;
		VkFramebuffer framebuffer;
	} vk;
};

struct sf_graphics_dynamic_buffer_allocation {
	struct sf_graphics_buffer *buffer;
	u64 offset64;
	u32 offset32;
	void *memory;
};

struct sf_graphics_dynamic_buffer {
	struct sf_graphics_buffer *buffer;

	struct sf_arena mapped_buffer_arena;

	u32 garbage_buffer_count;
	struct sf_graphics_buffer *garbage_buffers[SF_GRAPHICS_MAX_GARBAGE_ITEM_COUNT];
};

struct sf_graphics_frame {
	struct sf_graphics_semaphore *image_acquired_semaphore;
	struct sf_graphics_fence *in_flight_fence;
	struct sf_graphics_command_buffer *command_buffer;

	struct sf_graphics_dynamic_buffer *uniform_buffer;
	struct sf_graphics_dynamic_buffer *index_buffer;
	struct sf_graphics_dynamic_buffer *vertex_buffer;
};

struct sf_graphics_vertex_layout_attribute {
	enum sf_graphics_format format;
	u32 offset;
};

struct sf_graphics_vertex_layout {
	u32 stride;
	u32 attribute_count;
	struct sf_graphics_vertex_layout_attribute attributes[SF_GRAPHICS_MAX_VERTEX_LAYOUT_ATTRIBUTE_COUNT];
};

struct sf_graphics_init_pipeline_info {
	struct sf_graphics_vertex_layout vertex_layout;
	struct sf_graphics_render_target const *reference_render_target;
	struct sf_graphics_shader const *vertex_shader;
	struct sf_graphics_shader const *fragment_shader;
};


struct sf_graphics_pipeline {
	struct sf_graphics_init_pipeline_info info;

	struct {
		VkPipeline pipeline;
	} vk;
};



struct sf_graphics_init_context_info {
	struct sf_arena *arena;
	
	u32 buffering_count;
	
	struct sf_graphics_init_device_info device_info;
	struct sf_graphics_init_swapchain_info swapchain_info;
};

struct sf_graphics_context {
	struct sf_graphics_init_context_info info;

	struct sf_arena arena;

	struct sf_graphics_device *device;

	u32 current_swapchain_arena_index;
	struct sf_arena swapchain_arenas[2];
	struct sf_graphics_swapchain *swapchain;

	sf_bool skip_end_frame;
	u32 current_frame_index;
	u32 frame_count;

	struct sf_graphics_image *default_bound_image;


	struct sf_graphics_frame *frames[SF_GRAPHICS_MAX_FRAMES_IN_FLIGHT_COUNT];
};

struct sf_graphics_vertex {
	vec3 position;
	vec3 normal;
	vec2 uv;
};


struct sf_graphics_glfw_platform {
	GLFWwindow *window;
	i32 window_width;
	i32 window_height;
};


sf_public struct sf_graphics_context *sf_graphics_init_context(struct sf_arena *arena, struct sf_graphics_init_context_info *init_info);
sf_public void sf_graphics_deinit_context(struct sf_graphics_context *context);

sf_public void sf_graphics_begin_frame(struct sf_graphics_context *context);
sf_public void sf_graphics_end_frame(struct sf_graphics_context *context);

sf_public struct sf_graphics_pipeline *sf_graphics_init_pipeline(struct sf_graphics_context *context, struct sf_graphics_init_pipeline_info const *info);
sf_public void sf_graphics_deinit_pipeline(struct sf_graphics_context *context, struct sf_graphics_pipeline *pipeline);

sf_public struct sf_graphics_image *sf_graphics_init_image(struct sf_graphics_context *context, struct sf_graphics_init_image_info const *info);
sf_public void sf_graphics_deinit_image(struct sf_graphics_context *context, struct sf_graphics_image *image);

sf_public struct sf_graphics_buffer *sf_graphics_init_buffer(struct sf_graphics_context *context, struct sf_graphics_init_buffer_info const *info);
sf_public void sf_graphics_deinit_buffer(struct sf_graphics_context *context, struct sf_graphics_buffer *buffer);

sf_public void *sf_graphics_command_allocate_and_bind_immediate_vertex_memory(struct sf_graphics_context *context, u64 size);
sf_public void *sf_graphics_command_allocate_and_bind_immediate_index_memory(struct sf_graphics_context *context, u64 size);
sf_public void *sf_graphics_command_allocate_and_bind_immediate_uniform_memory(struct sf_graphics_context *context, u64 size);

sf_public void sf_graphics_command_bind_image_to_slot0(struct sf_graphics_context *context, struct sf_graphics_image *image);
sf_public void sf_graphics_command_bind_image_to_slot1(struct sf_graphics_context *context, struct sf_graphics_image *image);

sf_public void sf_graphics_command_draw_indexed(struct sf_graphics_context *context, u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance);

sf_public struct sf_graphics_glfw_platform *sf_graphics_init_glfw_platform(struct sf_arena *arena, i32 width, i32 height, struct sf_string const *title);
sf_public void sf_graphics_deinit_glfw_platform(struct sf_graphics_glfw_platform *platform);

sf_public void sf_graphics_glfw_process_events(struct sf_graphics_glfw_platform *platform);
sf_public sf_bool sf_graphics_glfw_should_close(struct sf_graphics_glfw_platform *platform);
sf_public void sf_graphics_glfw_fill_init_context_info(struct sf_arena *arena, struct sf_graphics_glfw_platform *platform, struct sf_graphics_init_context_info *info);

#endif
