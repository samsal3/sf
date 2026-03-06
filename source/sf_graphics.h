#ifndef SF_GRAPHICS_RENDERER_H
#define SF_GRAPHICS_RENDERER_H

#include "sf_core.h"
#include <vulkan/vulkan.h>

#define SF_GRAPHICS_MAX_GARBAGE_ITEM_COUNT 64
#define SF_GRAPHICS_MAX_FRAMES_IN_FLIGHT_COUNT 3
#define SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT 3
#define SF_GRAPHICS_MAX_TEXTURE_COUNT 256
#define SF_GRAPHICS_MAX_RENDER_TARGET_ATTACHMENT_COUNT 4
#define SF_GRAPHICS_MAX_DESCRIPTOR_ENTRY_COUNT 4
#define SF_GRAPHICS_MAX_DESCRIPTOR_SET_DESCRIPTOR_COUNT 4
#define SF_GRAPHICS_MAX_VERTEX_LAYOUT_ATTRIBUTE_COUNT 4

typedef intptr_t sf_handle;
#define SF_NULL_HANDLE 0

#define SF_AS_HANDLE(v) ((intptr_t)(v))

struct sf_graphics_clear_value_rgba {
	float r;
	float g;
	float b;
	float a;
};

struct sf_graphics_clear_value_depth_stencil {
	float	 depth;
	uint32_t stencil;
};

enum sf_graphics_clear_value_type {
	SF_GRAPHICS_CLEAR_VALUE_TYPE_NONE,
	SF_GRAPHICS_CLEAR_VALUE_TYPE_COLOR,
	SF_GRAPHICS_CLEAR_VALUE_TYPE_DEPTH_STENCIL,
};

union sf_graphics_clear_value_data {
	struct sf_graphics_clear_value_rgba	     rgba;
	struct sf_graphics_clear_value_depth_stencil depth_stencil;
};

struct sf_graphics_clear_value {
	enum sf_graphics_clear_value_type  type;
	union sf_graphics_clear_value_data data;
};

struct sf_graphics_resource_pool {
	struct sf_queue resource_queue;
	struct sf_queue free_resource_queue;
};

enum sf_graphics_descriptor_type {
	SF_GRAPHICS_DESCRIPTOR_UNIFORM_BUFFER,
	SF_GRAPHICS_DESCRIPTOR_TEXTURE,
	SF_GRAPHICS_DESCRIPTOR_SAMPLER
};

struct sf_graphics_descriptor {
	enum sf_graphics_descriptor_type type;
	uint32_t			 binding;
	uint32_t			 slot;
	uint32_t			 entry_count;
	sf_handle			 entries[SF_GRAPHICS_MAX_DESCRIPTOR_ENTRY_COUNT];
};

struct sf_graphics_descriptor_set {
	struct sf_queue		      queue;
	uint32_t		      descriptor_count;
	struct sf_graphics_descriptor descriptors[SF_GRAPHICS_MAX_DESCRIPTOR_SET_DESCRIPTOR_COUNT];
	VkDescriptorPool	      vk_descriptor_pool;
	VkDescriptorSetLayout	      vk_descriptor_set_layout;
	VkDescriptorSet		      vk_descriptor_set;
};

struct sf_graphics_vertex_attribute {
	enum sf_graphics_format format;
	uint32_t		binding;
	uint32_t		location;
	uint32_t		offset;
};

struct sf_graphics_vertex_layout {
	uint32_t			    attribute_count;
	struct sf_graphics_vertex_attribute attributes[SF_GRAPHICS_MAX_VERTEX_LAYOUT_ATTRIBUTE_COUNT];
};

struct sf_graphics_pipeline {
	struct sf_queue	 queue;
	VkShaderModule	 vk_vertex_shader;
	VkShaderModule	 vk_fragment_shader;
	VkPipelineLayout vk_pipeline_layout;
	VkPipeline	 vk_pipeline;
};

enum sf_graphics_sample_count {
	SF_GRAPHICS_SAMPLE_COUNT_1,
	SF_GRAPHICS_SAMPLE_COUNT_2,
	SF_GRAPHICS_SAMPLE_COUNT_4,
	SF_GRAPHICS_SAMPLE_COUNT_8,
	SF_GRAPHICS_SAMPLE_COUNT_16
};

enum sf_graphics_texture_type {
	SF_GRAPHICS_TEXTURE_TYPE_1D,
	SF_GRAPHICS_TEXTURE_TYPE_2D,
	SF_GRAPHICS_TEXTURE_TYPE_3D,
	SF_GRAPHICS_TEXTURE_TYPE_CUBE
};

enum sf_graphics_texture_usage {
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
};
typedef uint32_t sf_graphics_texture_usage_flags;

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

struct sf_graphics_texture {
	struct sf_queue		       queue;
	enum sf_graphics_texture_type  type;
	enum sf_graphics_format	       format;
	enum sf_graphics_sample_count  samples;
	enum sf_graphics_texture_usage usage;
	uint32_t		       width;
	uint32_t		       height;
	uint32_t		       depth;
	uint32_t		       mips;
	sf_bool			       mapped;
	void			      *mapped_data;
	struct sf_graphics_clear_value clear_value;
	sf_bool			       vk_owns_image_and_memory;
	VkImage			       vk_image;
	VkDeviceMemory		       vk_memory;
	VkImageView		       vk_image_view;
};

struct sf_graphics_render_target {
	struct sf_queue		      queue;
	enum sf_graphics_sample_count samples;

	uint32_t width;
	uint32_t height;

	uint32_t total_attachment_count;

	sf_handle depth_stencil_attachment;

	uint32_t  resolve_attachment_count;
	sf_handle resolve_attachments[SF_GRAPHICS_MAX_RENDER_TARGET_ATTACHMENT_COUNT];

	uint32_t  color_attachment_count;
	sf_handle color_attachments[SF_GRAPHICS_MAX_RENDER_TARGET_ATTACHMENT_COUNT];

	VkRenderPass  vk_render_pass;
	VkFramebuffer vk_framebuffer;
};

struct sf_graphics_buffer {
	struct sf_queue			 queue;
	enum tlaloc_renderer_buffer_type type;
	uint64_t			 size;
	void				*mapping;
	VkBuffer			 vk_buffer;
	VkDeviceMemory			 vk_memory;
};

struct sf_graphics_command_buffer {
	struct sf_queue queue;
	VkCommandPool	vk_command_pool;
	VkCommandBuffer vk_command_buffer;
};

struct sf_graphics_renderer_description {
	sf_bool	 enable_vsync;
	uint32_t width;
	uint32_t height;

	void (*create_vulkan_surface)(void *renderer);

	char const *application_name;

	uint32_t	   vk_instance_extension_count;
	char const *const *vk_instance_extensions;

	uint32_t	   vk_instance_layer_count;
	char const *const *vk_instance_layers;

	uint32_t	   vk_device_extension_count;
	char const *const *vk_device_extensions;
};

struct sf_graphics_renderer {
	struct sf_arena			    arena;
	VkInstance			    vk_instance;
	VkAllocationCallbacks		   *vk_allocation_callbacks;
	PFN_vkCreateDebugUtilsMessengerEXT  vk_create_debug_utils_messenger_ext;
	PFN_vkDestroyDebugUtilsMessengerEXT vk_destroy_debug_utils_messenger_ext;
	VkDebugUtilsMessengerEXT	    vk_validation_messenger;
	VkFormat			    vk_surface_format;
	VkFormat			    vk_depth_stencil_format;
	VkPresentModeKHR		    vk_present_mode;
	VkColorSpaceKHR			    vk_surface_color_space;
	VkSurfaceKHR			    vk_surface;
	VkSampleCountFlagBits		    vk_samples;
	VkPhysicalDevice		    vk_physical_device;
	VkDevice			    vk_device;
	VkQueue				    vk_graphics_queue;
	VkQueue				    vk_present_queue;
	uint32_t			    vk_present_queue_family_index;
	uint32_t			    vk_graphics_queue_family_index;
	VkSwapchainKHR			    vk_swapchain;
	uint32_t			    vk_swapchain_image_count;
	VkImage				    vk_swapchain_images[SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT];
	uint32_t			    vk_draw_complete_semaphore_count;
	VkSemaphore			    vk_draw_complete_semaphores[SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT];
	enum sf_graphics_sample_count	    swapchain_sample_count;
	enum sf_graphics_format		    swapchain_color_format;
	enum sf_graphics_format		    swapchain_depth_stencil_format;
	struct sf_graphics_clear_value	    swapchain_color_clear_value;
	struct sf_graphics_clear_value	    swapchain_depth_stencil_clear_value;
	uint32_t			    swapchain_requested_image_count;
	sf_bool				    swapchain_skip_end_frame;
	uint32_t			    swapchain_width;
	uint32_t			    swapchain_height;
	uint32_t			    swapchain_render_target_count;
	sf_handle			    swapchain_render_targets[SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT];
	uint32_t			    vk_in_flight_fence_count;
	VkFence				    vk_in_flight_fences[SF_GRAPHICS_MAX_FRAMES_IN_FLIGHT_COUNT];
	uint32_t			    vk_image_acquired_semaphore_count;
	VkSemaphore			    vk_image_acquired_semaphores[SF_GRAPHICS_MAX_FRAMES_IN_FLIGHT_COUNT];
	uint32_t			    current_swapchain_image_index;
	uint32_t			    current_frame_index;
	struct sf_arena			    render_target_arena;
	struct sf_graphics_resource_pool    texture_pool;
	struct sf_graphics_resource_pool    buffer_pool;
	struct sf_graphics_resource_pool    render_target_pool;
	struct sf_graphics_resource_pool    command_buffer_pool;
	struct sf_graphics_resource_pool    descriptor_set_pool;
	struct sf_graphics_resource_pool    pipeline_pool;
};

SF_EXTERNAL struct sf_graphics_renderer *
sf_graphics_create_renderer(struct sf_arena *arena, struct sf_graphics_renderer_description *description);

SF_EXTERNAL void sf_graphics_destroy_renderer(struct sf_graphics_renderer *renderer);

SF_EXTERNAL sf_handle sf_graphics_create_texture(struct sf_graphics_renderer  *renderer,
						 enum sf_graphics_texture_type type, enum sf_graphics_format format,
						 enum sf_graphics_sample_count	samples,
						 enum sf_graphics_texture_usage usage, uint32_t width, uint32_t height,
						 uint32_t depth, uint32_t mips, sf_bool mapped,
						 struct sf_graphics_clear_value *clear_value);

SF_EXTERNAL void sf_graphics_destroy_texture(struct sf_graphics_renderer *renderer, sf_handle texture_handle);

SF_EXTERNAL sf_handle sf_graphics_create_command_buffer(struct sf_graphics_renderer *renderer, sf_bool transient);

SF_EXTERNAL void sf_graphics_destroy_command_buffer(struct sf_graphics_renderer *renderer,
						    sf_handle			 command_buffer_handle);

SF_EXTERNAL sf_handle sf_graphics_create_render_target(struct sf_graphics_renderer *renderer, uint32_t width,
						       uint32_t height, enum sf_graphics_sample_count samples,
						       enum sf_graphics_format	       color_format,
						       enum sf_graphics_format	       depth_stencil_format,
						       uint32_t			       color_attachment_count,
						       struct sf_graphics_clear_value *color_clear_values,
						       struct sf_graphics_clear_value *depth_stencil_clear_value);

SF_EXTERNAL void sf_graphics_destroy_render_target(struct sf_graphics_renderer *renderer,
						   sf_handle			render_target_handle);

SF_EXTERNAL sf_handle sf_graphics_create_pipeline(struct sf_graphics_renderer		   *renderer,
						  struct sf_graphics_vertex_layout	   *vertex_layout,
						  struct sf_graphics_descriptor_set_layout *descriptor_set_layout,
						  uint32_t vertex_code_size, void const *vertex_code,
						  uint32_t fragment_code_size, void const *fragment_code);

SF_EXTERNAL void sf_graphics_destroy_pipeline(struct sf_graphics_renderer *renderer, sf_handle pipeline_handle);

#endif
