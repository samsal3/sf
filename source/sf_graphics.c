#include "sf_graphics.h"

#include <sf.h>

#define SF_VULKAN_CHECK(e) ((e) == VK_SUCCESS)
#define SF_VULKAN_PROC(name, i) (PFN_##name) vkGetInstanceProcAddr(i, #name)

SF_INTERNAL SfBool sfVulkanValidateQueueFamilyIndices(SfGraphicsRenderer *renderer) {
	return renderer->graphicsQueue.vulkanQueueFamilyIndex != (uint32_t)-1 &&
	       renderer->presentQueue.vulkanQueueFamilyIndex != (uint32_t)-1;
}

typedef struct SfVulkanQueueFamilyPropertyList {
	uint32_t		 count;
	VkQueueFamilyProperties *data;
} SfVulkanQueueFamilyPropertyList;

SF_INTERNAL SfVulkanQueueFamilyPropertyList sfVulkanCreateQueueFamilyPropertyList(SfArena	     *arena,
										  SfGraphicsRenderer *renderer) {
	SfVulkanQueueFamilyPropertyList result = {0};

	vkGetPhysicalDeviceQueueFamilyProperties(renderer->vulkanPhysicalDevice, &result.count, NULL);
	result.data = sfAllocate(arena, result.count * sizeof(*result.data));
	if (result.data)
		vkGetPhysicalDeviceQueueFamilyProperties(renderer->vulkanPhysicalDevice, &result.count, result.data);

	return result;
}

SF_INTERNAL void sfVulkanFindQueueFamilyIndices(SfGraphicsRenderer *renderer) {
	SfVulkanQueueFamilyPropertyList list = sfVulkanCreateQueueFamilyPropertyList(&renderer->arena, renderer);

	renderer->graphicsQueue.vulkanQueueFamilyIndex = (uint32_t)-1;
	renderer->presentQueue.vulkanQueueFamilyIndex  = (uint32_t)-1;

	for (uint32_t i = 0; i < list.count && !sfVulkanValidateQueueFamilyIndices(renderer); ++i) {
		VkBool32 supportsSurface = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(
		    renderer->vulkanPhysicalDevice, i, renderer->vulkanSurface, &supportsSurface);

		if (supportsSurface)
			renderer->presentQueue.vulkanQueueFamilyIndex = i;

		if (list.data[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			renderer->graphicsQueue.vulkanQueueFamilyIndex = i;
	}
}

typedef struct SfVulkanExtensionPropertyList {
	uint32_t	       count;
	VkExtensionProperties *data;
} SfVulkanExtensionPropertyList;

SF_INTERNAL SfVulkanExtensionPropertyList sfVulkanCreateExtensionPropertyList(SfArena		 *arena,
									      SfGraphicsRenderer *renderer) {
	SfVulkanExtensionPropertyList result = {0};

	if (SF_VULKAN_CHECK(
		vkEnumerateDeviceExtensionProperties(renderer->vulkanPhysicalDevice, NULL, &result.count, NULL))) {
		result.data = sfAllocate(arena, result.count * sizeof(*result.data));
		if (result.data) {
			SF_VULKAN_CHECK(vkEnumerateDeviceExtensionProperties(
			    renderer->vulkanPhysicalDevice, NULL, &result.count, result.data));
		}
	}

	return result;
}

SF_INTERNAL SfBool sfVulkanCheckDeviceExtensionSupport(SfGraphicsRenderer *renderer) {
	SfBool				    foundAllExtensions = SF_TRUE;
	SfVulkanExtensionPropertyList const list = sfVulkanCreateExtensionPropertyList(&renderer->arena, renderer);

	for (uint32_t i = 0; i < renderer->vulkanDeviceExtensionCount && foundAllExtensions; ++i) {
		SfBool foundCurrentExtension = SF_FALSE;
		SfS8   currentExtension	     = sfCreateS8FromNonLiteral(renderer->vulkanDeviceExtensions[i]);

		for (uint32_t j = 0; j < list.count && !foundCurrentExtension; ++j) {
			SfS8 currentAvailableExtension = sfCreateS8FromNonLiteral(list.data[j].extensionName);
			foundCurrentExtension	       = sf_s8_compare(
			     currentExtension, currentAvailableExtension, VK_MAX_EXTENSION_NAME_SIZE);
		}

		foundAllExtensions = foundAllExtensions && foundCurrentExtension;
	}

	return foundAllExtensions;
}

SF_INTERNAL SfBool sfVulkanCheckDeviceSwapchainSupport(SfGraphicsRenderer const *renderer) {
	uint32_t surfaceFormatCount = 0, presentModeCount = 0;

	SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
	    renderer->vulkanPhysicalDevice, renderer->vulkanSurface, &surfaceFormatCount, NULL));
	SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
	    renderer->vulkanPhysicalDevice, renderer->vulkanSurface, &presentModeCount, NULL));

	return surfaceFormatCount && presentModeCount;
}

SF_INTERNAL SfBool sfVulkanCheckDeviceSupport(SfGraphicsRenderer *renderer) {
	SfBool result = SF_TRUE;

	if (result && !sfVulkanCheckDeviceExtensionSupport(renderer))
		result = SF_FALSE;

	if (result && !sfVulkanCheckDeviceSwapchainSupport(renderer))
		result = SF_FALSE;

	sfVulkanFindQueueFamilyIndices(renderer);
	if (result && !sfVulkanValidateQueueFamilyIndices(renderer))
		result = SF_FALSE;

	return result;
}

typedef struct SfVulkanPhysicalDeviceList {
	uint32_t	  count;
	VkPhysicalDevice *data;
} SfVulkanPhysicalDeviceList;

SF_INTERNAL SfVulkanPhysicalDeviceList sfVulkanCreatePhysicalDeviceList(SfArena *arena, SfGraphicsRenderer *renderer) {
	SfVulkanPhysicalDeviceList result = {0};

	if (SF_VULKAN_CHECK(vkEnumeratePhysicalDevices(renderer->vulkanInstance, &result.count, NULL))) {
		result.data = sfAllocate(arena, result.count * sizeof(*result.data));
		if (result.data) {
			SF_VULKAN_CHECK(vkEnumeratePhysicalDevices(renderer->vulkanInstance, &result.count, NULL));
		}
	}
	return result;
}

typedef struct SfVulkanSurfaceFormatList {
	uint32_t	    count;
	VkSurfaceFormatKHR *data;
} SfVulkanSurfaceFormatList;

SF_INTERNAL SfVulkanSurfaceFormatList sfVulkanCreateSurfaceFormatList(SfArena *arena, SfGraphicsRenderer *renderer) {
	SfVulkanSurfaceFormatList result = {0};

	if (SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
		renderer->vulkanPhysicalDevice, renderer->vulkanSurface, &result.count, NULL))) {
		result.data = sfAllocate(arena, result.count * sizeof(*result.data));
		if (result.data) {
			SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
			    renderer->vulkanPhysicalDevice, renderer->vulkanSurface, &result.count, result.data));
		}
	}

	return result;
}
typedef struct SfVulkanPresentModeList {
	uint32_t	  count;
	VkPresentModeKHR *data;
} SfVulkanPresentModeList;

SF_INTERNAL SfVulkanPresentModeList sfVulkanCreatePresentModeList(SfArena *arena, SfGraphicsRenderer *renderer) {
	SfVulkanPresentModeList result = {0};

	if (SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
		renderer->vulkanPhysicalDevice, renderer->vulkanSurface, &result.count, NULL))) {
		result.data = sfAllocate(arena, result.count * sizeof(*result.data));
		if (result.data) {
			SF_VULKAN_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
			    renderer->vulkanPhysicalDevice, renderer->vulkanSurface, &result.count, result.data));
		}
	}

	return result;
}

SF_INTERNAL uint32_t sfVulkanFindMemoryTypeIndex(VkPhysicalDevice device, VkMemoryPropertyFlags memoryProperties,
						 uint32_t filter) {
	uint32_t result = (uint32_t)-1;

	if (device) {
		VkPhysicalDeviceMemoryProperties availableProperties = {0};
		vkGetPhysicalDeviceMemoryProperties(device, &availableProperties);

		for (uint32_t currentType = 0;
		     currentType < availableProperties.memoryTypeCount && result == (uint32_t)-1; ++currentType)
			if ((filter & (1 << currentType)) &&
			    (availableProperties.memoryTypes[currentType].propertyFlags & memoryProperties) ==
				memoryProperties)
				result = currentType;
	}

	return result;
}

SF_INTERNAL VkDeviceMemory sfAllocateVulkanMemory(SfGraphicsRenderer *renderer, VkMemoryPropertyFlags memoryProperties,
						  uint32_t filter, uint64_t count) {
	VkDeviceMemory memory = VK_NULL_HANDLE;

	uint32_t       memoryTypeIndex = sfVulkanFindMemoryTypeIndex(
		  renderer->vulkanPhysicalDevice, memoryProperties, filter);
	if (memoryTypeIndex != (uint32_t)-1) {
		VkMemoryAllocateInfo info = {.sType	      = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
					     .pNext	      = NULL,
					     .allocationSize  = count,
					     .memoryTypeIndex = memoryTypeIndex};
		SF_VULKAN_CHECK(
		    vkAllocateMemory(renderer->vulkanDevice, &info, renderer->vulkanAllocationCallbacks, &memory));
	}

	return memory;
}

SF_INTERNAL VkDeviceMemory sfVulkanAllocateAndBindMemoryForImage(SfGraphicsRenderer *renderer, VkImage image,
								 VkMemoryPropertyFlags properties) {
	VkDeviceMemory memory = VK_NULL_HANDLE;

	if (renderer && image) {
		VkMemoryRequirements requirements = {0};
		vkGetImageMemoryRequirements(renderer->vulkanDevice, image, &requirements);

		memory = sfAllocateVulkanMemory(renderer, properties, requirements.memoryTypeBits, requirements.size);
		if (memory && SF_VULKAN_CHECK(vkBindImageMemory(renderer->vulkanDevice, image, memory, 0))) {
			vkFreeMemory(renderer->vulkanDevice, memory, renderer->vulkanAllocationCallbacks);
			memory = VK_NULL_HANDLE;
		}
	}

	return memory;
}

SF_INTERNAL VkDeviceMemory sf_vulkan_allocate_and_bind_memory_to_buffer(SfGraphicsRenderer *renderer, VkBuffer buffer,
									VkMemoryPropertyFlags properties) {
	VkDeviceMemory memory = VK_NULL_HANDLE;

	if (renderer && buffer) {
		VkMemoryRequirements requirements = {0};
		vkGetBufferMemoryRequirements(renderer->vulkanDevice, buffer, &requirements);

		memory = sfAllocateVulkanMemory(renderer, properties, requirements.memoryTypeBits, requirements.size);
		if (memory && !SF_VULKAN_CHECK(vkBindBufferMemory(renderer->vulkanDevice, buffer, memory, 0))) {
			vkFreeMemory(renderer->vulkanDevice, memory, renderer->vulkanAllocationCallbacks);
			memory = VK_NULL_HANDLE;
		}
	}

	return memory;
}

void sfGraphicsDeviceWaitIdle(SfGraphicsRenderer const *renderer) { vkDeviceWaitIdle(renderer->vulkanDevice); }

SF_INTERNAL void sfDestroySwapchainResources(SfGraphicsRenderer *renderer) {
	sfGraphicsDeviceWaitIdle(renderer);

	for (uint32_t i = 0; i < SF_SIZE(renderer->swapchainRenderTargets); ++i) {
		sfDestroyGraphicsRenderTarget(renderer, renderer->swapchainRenderTargets[i]);
		renderer->swapchainRenderTargets[i] = SF_NULL_HANDLE;
	}

	SF_ARRAY_INIT(renderer->vulkanSwapchainImages, VK_NULL_HANDLE);
	renderer->vulkanSwapchainImageCount = 0;

	if (renderer->vulkanDevice && renderer->vulkanSwapchain) {
		vkDestroySwapchainKHR(
		    renderer->vulkanDevice, renderer->vulkanSwapchain, renderer->vulkanAllocationCallbacks);
		renderer->vulkanSwapchain = VK_NULL_HANDLE;
	}
}

SF_INTERNAL void sfDefaultInitGraphicsTexture(SfGraphicsTexture *texture) {
	SF_QUEUE_INIT(&texture->queue);
	texture->type		   = SF_GRAPHICS_TEXTURE_TYPE_2D;
	texture->samples	   = SF_GRAPHICS_SAMPLE_COUNT_1;
	texture->format		   = SF_GRAPHICS_FORMAT_UNDEFINED;
	texture->usage		   = SF_GRAPHICS_TEXTURE_USAGE_SAMPLED;
	texture->width		   = 0;
	texture->height		   = 0;
	texture->depth		   = 0;
	texture->mips		   = 0;
	texture->mapped		   = 0;
	texture->clearValue.type   = SF_GRAPHICS_CLEAR_VALUE_TYPE_NONE;
	texture->ownsImage	   = SF_FALSE;
	texture->mappedData	   = NULL;
	texture->vulkanLayout	   = VK_IMAGE_LAYOUT_UNDEFINED;
	texture->vulkanAspectFlags = 0;
	texture->vulkanImage	   = VK_NULL_HANDLE;
	texture->vulkanMemory	   = VK_NULL_HANDLE;
	texture->vulkanImageView   = VK_NULL_HANDLE;
	texture->vulkanSampler	   = VK_NULL_HANDLE;
}

SF_INTERNAL SfGraphicsTexture *sfGetOrAllocateGraphicsTexture(SfGraphicsRenderer *renderer) {
	SfGraphicsTexture *texture = NULL;

	if (SF_QUEUE_IS_EMPTY(&renderer->freeTextureQueue)) {
		texture = sfAllocate(&renderer->arena, sizeof(*texture));
	} else {
		SfQueue *queue = SF_QUEUE_HEAD(&renderer->freeTextureQueue);
		SF_QUEUE_REMOVE(queue);
		texture = SF_QUEUE_DATA(queue, SfGraphicsTexture, queue);
	}
	if (texture) {
		sfDefaultInitGraphicsTexture(texture);
		SF_QUEUE_INSERT_HEAD(&renderer->textureQueue, &texture->queue);
	}
	return texture;
}

SF_INTERNAL void sfCreateGraphicsSwapchainResources(SfGraphicsRenderer *renderer) {
	uint32_t const queueFamilyIndices[] = {
	    renderer->graphicsQueue.vulkanQueueFamilyIndex, renderer->presentQueue.vulkanQueueFamilyIndex};

	VkSurfaceCapabilitiesKHR capabilities = {0};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
	    renderer->vulkanPhysicalDevice, renderer->vulkanSurface, &capabilities);

	VkSwapchainCreateInfoKHR const swapchainInfo = {
	    .sType		   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
	    .pNext		   = NULL,
	    .flags		   = 0,
	    .surface		   = renderer->vulkanSurface,
	    .minImageCount	   = SF_GRAPHICS_MAX_SWAPCHAIN_IMAGE_COUNT,
	    .imageFormat	   = renderer->vulkanSurfaceFormat,
	    .imageColorSpace	   = renderer->vulkanSurfaceColorSpace,
	    .imageExtent.width	   = renderer->swapchainWidth,
	    .imageExtent.height	   = renderer->swapchainHeight,
	    .imageArrayLayers	   = 1,
	    .imageUsage		   = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
	    .imageSharingMode	   = queueFamilyIndices[0] == queueFamilyIndices[1] ? VK_SHARING_MODE_EXCLUSIVE
										    : VK_SHARING_MODE_CONCURRENT,
	    .queueFamilyIndexCount = queueFamilyIndices[0] == queueFamilyIndices[1] ? 1 : SF_SIZE(queueFamilyIndices),
	    .pQueueFamilyIndices   = queueFamilyIndices,
	    .preTransform	   = capabilities.currentTransform,
	    .compositeAlpha	   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
	    .presentMode	   = renderer->vulkanPresentMode,
	    .clipped		   = VK_TRUE,
	    .oldSwapchain	   = VK_NULL_HANDLE,
	};

	if (SF_VULKAN_CHECK(vkCreateSwapchainKHR(renderer->vulkanDevice, &swapchainInfo,
						 renderer->vulkanAllocationCallbacks, &renderer->vulkanSwapchain))) {
		if (SF_VULKAN_CHECK(vkGetSwapchainImagesKHR(renderer->vulkanDevice, renderer->vulkanSwapchain,
							    &renderer->vulkanSwapchainImageCount, NULL))) {
			if (SF_SIZE(renderer->vulkanSwapchainImages) < renderer->vulkanSwapchainImageCount) {
				renderer->swapchainImageCount = renderer->vulkanSwapchainImageCount;
				SF_VULKAN_CHECK(vkGetSwapchainImagesKHR(
				    renderer->vulkanDevice, renderer->vulkanSwapchain,
				    &renderer->vulkanSwapchainImageCount, renderer->vulkanSwapchainImages));
			}
		}
	}

	renderer->drawCompleteSemaphoreCount = renderer->swapchainImageCount;
	for (uint32_t i = 0; i < renderer->drawCompleteSemaphoreCount; ++i)
		renderer->drawCompleteSemaphores[i] = sfCreateGraphicsSemaphore(renderer);

	renderer->swapchainRenderTargetCount = renderer->swapchainImageCount;
	for (uint32_t i = 0; i < renderer->swapchainRenderTargetCount; ++i) {
		SfGraphicsRenderTargetDescription const description = {
		    .sampleCount		      = renderer->samples,
		    .colorFormat		      = renderer->colorAttachmentFormat,
		    .depthStencilFormat		      = renderer->depthStencilFormat,
		    .width			      = renderer->swapchainWidth,
		    .height			      = renderer->swapchainHeight,
		    .colorAttachmentCount	      = 1,
		    .colorAttachmentClearValues[0]    = renderer->swapchainColorClearValue,
		    .depthStencilAttachmentClearValue = renderer->swapchainDepthStencilClearValue,
		    .vulkanNotOwnedImage	      = renderer->vulkanSwapchainImages[i]};
		renderer->swapchainRenderTargets[i] = sfCreateGraphicsrenderTarget(renderer, &description);
	}
}

SfBool sfBeginGraphicsCommand(SfGraphicsRenderer *renderer, SfHandle commandBufferHandle) {
	SfBool result = SF_FALSE;
	if (renderer && commandBufferHandle) {
		SfGraphicsCommandBuffer	      *commandBuffer = (SfGraphicsCommandBuffer *)commandBufferHandle;
		VkCommandBufferBeginInfo const info	     = {.sType		  = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
								.pNext		  = NULL,
								.flags		  = 0,
								.pInheritanceInfo = NULL};
		result = SF_VULKAN_CHECK(vkBeginCommandBuffer(commandBuffer->vulkanCommandBuffer, &info));
	}
	return result;
}

SfBool sfEndGraphicsCommand(SfGraphicsRenderer *renderer, SfHandle commandBufferHandle) {
	SfBool result = SF_FALSE;
	if (commandBufferHandle) {
		SfGraphicsCommandBuffer *commandBuffer = (SfGraphicsCommandBuffer *)commandBufferHandle;
		result = SF_VULKAN_CHECK(vkEndCommandBuffer(commandBuffer->vulkanCommandBuffer));
	}
	return result;
}

SfBool sfGraphicsQueueSubmitGraphicsCommand(SfGraphicsRenderer *renderer, SfHandle queueHandle,
					    uint32_t commandBufferCount, SfHandle const *commandBufferHandles,
					    uint32_t waitSemaphoreCount, SfHandle const *waitSemaphoreHandles,
					    uint32_t signalSemaphoreCount, SfHandle const *signalSemaphoreHandles) {
	SfBool result = SF_FALSE;
	if (renderer && queueHandle) {
		SfGraphicsQueue	    *queue = (SfGraphicsQueue *)queueHandle;
		VkCommandBuffer	     vulkanCommandBuffers[SF_GRAPHICS_MAX_COMMAND_BUFFER_SUBMIT_COUNT] = {0};
		VkSemaphore	     vulkanWaitSemaphores[SF_GRAPHICS_MAX_WAIT_SEMAPHORE_COUNT]	       = {0};
		VkPipelineStageFlags vulkanStageFlags[SF_GRAPHICS_MAX_WAIT_SEMAPHORE_COUNT]	       = {0};
		VkSemaphore	     vulkanSignalSemaphores[SF_GRAPHICS_MAX_WAIT_SEMAPHORE_COUNT]      = {0};

		for (uint32_t i = 0; i < commandBufferCount; i++)
			vulkanCommandBuffers[i] =
			    ((SfGraphicsCommandBuffer const *)commandBufferHandles[i])->vulkanCommandBuffer;

		for (uint32_t i = 0; i < waitSemaphoreCount; i++)
			vulkanWaitSemaphores[i] =
			    ((SfGraphicsSemaphore const *)waitSemaphoreHandles[i])->vulkanSemaphore;

		for (uint32_t i = 0; i < waitSemaphoreCount; i++)
			vulkanStageFlags[i] = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

		for (uint32_t i = 0; i < signalSemaphoreCount; i++)
			vulkanSignalSemaphores[i] =
			    ((SfGraphicsSemaphore const *)signalSemaphoreHandles[i])->vulkanSemaphore;

		VkSubmitInfo submitInfo = {.sType		 = VK_STRUCTURE_TYPE_SUBMIT_INFO,
					   .pNext		 = NULL,
					   .waitSemaphoreCount	 = waitSemaphoreCount,
					   .pWaitSemaphores	 = vulkanWaitSemaphores,
					   .pWaitDstStageMask	 = vulkanStageFlags,
					   .commandBufferCount	 = commandBufferCount,
					   .pCommandBuffers	 = vulkanCommandBuffers,
					   .signalSemaphoreCount = signalSemaphoreCount,
					   .pSignalSemaphores	 = vulkanSignalSemaphores};

		result = SF_VULKAN_CHECK(vkQueueSubmit(queue->vulkanQueue, 1, &submitInfo, VK_NULL_HANDLE));
	}

	return result;
}

SfBool sfGraphicsQueuePresent(SfGraphicsRenderer *renderer, SfHandle queueHandle, uint32_t waitSemaphoreCount,
			      SfHandle const *waitSemaphoreHandles) {
	SfBool result = SF_FALSE;

	if (renderer && queueHandle) {
		SfGraphicsQueue *queue = (SfGraphicsQueue *)queueHandle;

		VkSemaphore	 vulkanWaitSemaphores[SF_GRAPHICS_MAX_WAIT_SEMAPHORE_COUNT] = {0};
		for (uint32_t i = 0; i < waitSemaphoreCount; i++)
			vulkanWaitSemaphores[i] = ((SfGraphicsSemaphore *)waitSemaphoreHandles[i])->vulkanSemaphore;

		VkPresentInfoKHR submitInfo = {.sType		   = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
					       .pNext		   = NULL,
					       .waitSemaphoreCount = waitSemaphoreCount,
					       .pWaitSemaphores	   = vulkanWaitSemaphores,
					       .swapchainCount	   = 1,
					       .pSwapchains	   = &renderer->vulkanSwapchain,
					       .pImageIndices	   = &renderer->swapchainCurrentImageIndex,
					       .pResults	   = NULL};
		result			    = SF_VULKAN_CHECK(vkQueuePresentKHR(queue->vulkanQueue, &submitInfo));
	}

	return result;
}

SfBool sfGraphicsQueueWaitIdle(SfHandle queueHandle) {
	SfBool result = SF_FALSE;
	if (queueHandle) {
		SfGraphicsQueue *queue = (SfGraphicsQueue *)queueHandle;
		result		       = SF_VULKAN_CHECK(vkQueueWaitIdle(queue->vulkanQueue));
	}
	return result;
}

void sfDestroyGraphicsTexture(SfGraphicsRenderer *renderer, SfHandle textureHandle) {
	if (renderer && textureHandle) {
		SfGraphicsTexture *texture = (SfGraphicsTexture *)texture;

		SF_QUEUE_REMOVE(&texture->queue);
		SF_QUEUE_INSERT_HEAD(&renderer->freeTextureQueue, &texture->queue);

		if (texture->vulkanImageView) {
			vkDestroyImageView(
			    renderer->vulkanDevice, texture->vulkanImageView, renderer->vulkanAllocationCallbacks);
			texture->vulkanImageView = VK_NULL_HANDLE;
		}

		if (texture->ownsImage) {
			if (texture->vulkanMemory) {
				vkFreeMemory(renderer->vulkanDevice, texture->vulkanMemory,
					     renderer->vulkanAllocationCallbacks);
			}
			if (texture->vulkanImage) {
				vkDestroyImage(
				    renderer->vulkanDevice, texture->vulkanImage, renderer->vulkanAllocationCallbacks);
			}
		}

		texture->vulkanMemory = VK_NULL_HANDLE;
		texture->vulkanImage  = VK_NULL_HANDLE;
	}
}

SF_INTERNAL void sfDefaultInitGraphicsSemaphore(SfGraphicsSemaphore *semaphore) {
	SF_QUEUE_INIT(&semaphore->queue);
	semaphore->vulkanSemaphore = VK_NULL_HANDLE;
}

SF_INTERNAL SfGraphicsSemaphore *sfGetOrAllocateGraphicsSemaphore(SfGraphicsRenderer *renderer) {
	SfGraphicsSemaphore *result = NULL;

	if (SF_QUEUE_IS_EMPTY(&renderer->freeSemaphoreQueue)) {
		result = sfAllocate(&renderer->arena, sizeof(*result));
	} else {
		SfQueue *queue = SF_QUEUE_HEAD(&renderer->freeSemaphoreQueue);
		SF_QUEUE_REMOVE(queue);
		result = SF_QUEUE_DATA(queue, SfGraphicsSemaphore, queue);
	}

	if (result) {
		sfDefaultInitGraphicsSemaphore(result);
		SF_QUEUE_INSERT_HEAD(&renderer->semaphoreQueue, &result->queue);
	}

	return result;
}

void sfDestroyGraphicsSemaphore(SfGraphicsRenderer *renderer, SfHandle semaphoreHandle) {
	if (renderer && semaphoreHandle) {
		SfGraphicsSemaphore *semaphore = (SfGraphicsSemaphore *)semaphoreHandle;

		SF_QUEUE_REMOVE(&semaphore->queue);
		SF_QUEUE_INSERT_HEAD(&renderer->freeSemaphoreQueue, &semaphore->queue);

		if (renderer->vulkanDevice && semaphore->vulkanSemaphore) {
			vkDestroySemaphore(
			    renderer->vulkanDevice, semaphore->vulkanSemaphore, renderer->vulkanAllocationCallbacks);
			semaphore->vulkanSemaphore = VK_NULL_HANDLE;
		}
	}
}

SfHandle sfCreateGraphicsSemaphore(SfGraphicsRenderer *renderer) {
	SfGraphicsSemaphore *semaphore = sfGetOrAllocateGraphicsSemaphore(renderer);
	if (semaphore) {
		VkSemaphoreCreateInfo info = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
					      .pNext = NULL,
					      .flags = 0};
		if (!SF_VULKAN_CHECK(vkCreateSemaphore(renderer->vulkanDevice, &info,
						       renderer->vulkanAllocationCallbacks,
						       &semaphore->vulkanSemaphore))) {
			sfDestroyGraphicsSemaphore(renderer, SF_AS_HANDLE(semaphore));
			semaphore = SF_NULL_HANDLE;
		}
	}
	return SF_AS_HANDLE(semaphore);
}

SF_INTERNAL void sfDefaultInitGraphicsFence(SfGraphicsFence *fence) {
	SF_QUEUE_INIT(&fence->queue);
	fence->vulkanFence = VK_NULL_HANDLE;
}

SF_INTERNAL SfGraphicsFence *sfGetOrAllocateGraphicsFence(SfGraphicsRenderer *renderer) {
	SfGraphicsFence *result = NULL;

	if (SF_QUEUE_IS_EMPTY(&renderer->freeFenceQueue)) {
		result = sfAllocate(&renderer->arena, sizeof(*result));
	} else {
		SfQueue *queue = SF_QUEUE_HEAD(&renderer->freeFenceQueue);
		SF_QUEUE_REMOVE(queue);
		result = SF_QUEUE_DATA(queue, SfGraphicsFence, queue);
	}

	if (result) {
		sfDefaultInitGraphicsFence(result);
		SF_QUEUE_INSERT_HEAD(&renderer->fenceQueue, &result->queue);
	}

	return result;
}

void sfDestroyGraphicsFence(SfGraphicsRenderer *renderer, SfHandle fenceHandle) {
	if (renderer && fenceHandle) {
		SfGraphicsFence *fence = (SfGraphicsFence *)fenceHandle;

		SF_QUEUE_REMOVE(&fence->queue);
		SF_QUEUE_INSERT_HEAD(&renderer->freeFenceQueue, &fence->queue);

		if (renderer->vulkanDevice && fence->vulkanFence) {
			vkDestroyFence(
			    renderer->vulkanDevice, fence->vulkanFence, renderer->vulkanAllocationCallbacks);
			fence->vulkanFence = VK_NULL_HANDLE;
		}
	}
}

SfHandle sfCreateGraphicsFence(SfGraphicsRenderer *renderer) {
	SfGraphicsFence *fence = sfGetOrAllocateGraphicsFence(renderer);
	if (fence) {
		VkFenceCreateInfo const info = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
						.pNext = NULL,
						.flags = VK_FENCE_CREATE_SIGNALED_BIT};

		if (!SF_VULKAN_CHECK(vkCreateFence(
			renderer->vulkanDevice, &info, renderer->vulkanAllocationCallbacks, &fence->vulkanFence))) {
			sfDestroyGraphicsFence(renderer, SF_AS_HANDLE(fence));
			fence = SF_NULL_HANDLE;
		}
	}
	return SF_AS_HANDLE(fence);
}

SF_INTERNAL void sfDefaultInitCommandPool(SfGraphicsCommandPool *commandPool) {
	SF_QUEUE_INIT(&commandPool->queue);
	commandPool->vulkanCommandPool = VK_NULL_HANDLE;
}

SF_INTERNAL SfGraphicsCommandPool *sfGetOrAllocateGraphicsCommandPool(SfGraphicsRenderer *renderer) {
	SfGraphicsCommandPool *result = NULL;

	if (SF_QUEUE_IS_EMPTY(&renderer->freeCommandPoolQueue)) {
		result = sfAllocate(&renderer->arena, sizeof(*result));
	} else {
		SfQueue *queue = SF_QUEUE_HEAD(&renderer->freeCommandBufferQueue);
		SF_QUEUE_REMOVE(queue);
		result = SF_QUEUE_DATA(queue, SfGraphicsCommandPool, queue);
	}
	if (result) {
		sfDefaultInitCommandPool(result);
		SF_QUEUE_INSERT_HEAD(&renderer->commandPoolQueue, &result->queue);
	}
	return result;
}

void sfDestroyGraphicsCommandPool(SfGraphicsRenderer *renderer, SfHandle commandPoolHandle) {
	if (renderer && commandPoolHandle) {
		SfGraphicsCommandPool *commandPool = (SfGraphicsCommandPool *)commandPoolHandle;

		SF_QUEUE_REMOVE(&commandPool->queue);
		SF_QUEUE_INSERT_HEAD(&renderer->freeCommandBufferQueue, &commandPool->queue);

		if (renderer->vulkanDevice && commandPool->vulkanCommandPool) {
			vkDestroyCommandPool(renderer->vulkanDevice, commandPool->vulkanCommandPool,
					     renderer->vulkanAllocationCallbacks);
			commandPool->vulkanCommandPool = VK_NULL_HANDLE;
		}
	}
}

SfHandle sfCreateGraphicsCommandPool(SfGraphicsRenderer *renderer, SfHandle queueHandle, SfBool transient,
				     SfBool reset) {
	SfGraphicsCommandPool *commandPool = sfGetOrAllocateGraphicsCommandPool(renderer);
	if (commandPool) {
		SfGraphicsQueue		     *queue = (SfGraphicsQueue *)queueHandle;

		VkCommandPoolCreateInfo const info = {.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
						      .pNext = NULL,
						      .flags = 0 | transient ? VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
							       : 0 | reset
								   ? VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
								   : 0,
						      .queueFamilyIndex = queue->vulkanQueueFamilyIndex};
		if (!SF_VULKAN_CHECK(vkCreateCommandPool(renderer->vulkanDevice, &info,
							 renderer->vulkanAllocationCallbacks,
							 &commandPool->vulkanCommandPool))) {
			sfDestroyGraphicsCommandPool(renderer, SF_AS_HANDLE(commandPool));
			commandPool = SF_NULL_HANDLE;
		}
	}
	return SF_AS_HANDLE(commandPool);
}

SF_INTERNAL void sfDefaultInitGraphicsCommandBuffer(SfGraphicsCommandBuffer *commandBuffer) {
	SF_QUEUE_INIT(&commandBuffer->queue);
	commandBuffer->vulkanCommandBuffer = VK_NULL_HANDLE;
}

SF_INTERNAL SfGraphicsCommandBuffer *sfGetOrAllocateGraphicsCommandBuffer(SfGraphicsRenderer *renderer) {
	SfGraphicsCommandBuffer *result = NULL;

	if (SF_QUEUE_IS_EMPTY(&renderer->freeCommandBufferQueue)) {
		result = sfAllocate(&renderer->arena, sizeof(*result));
	} else {
		SfQueue *queue = SF_QUEUE_HEAD(&renderer->freeCommandBufferQueue);
		SF_QUEUE_REMOVE(queue);
		result = SF_QUEUE_DATA(queue, SfGraphicsCommandBuffer, queue);
	}

	if (result) {
		sfDefaultInitGraphicsCommandBuffer(result);
		SF_QUEUE_INSERT_HEAD(&renderer->commandBufferQueue, &result->queue);
	}
	return result;
}

void sfDestroyGraphicsCommandBuffer(SfGraphicsRenderer *renderer, SfHandle commandPoolHandle,
				    SfHandle commandBufferHandle) {
	if (renderer && commandPoolHandle && commandBufferHandle) {
		SfGraphicsCommandBuffer *commandBuffer = (SfGraphicsCommandBuffer *)commandBufferHandle;
		SfGraphicsCommandPool	*commandPool   = (SfGraphicsCommandPool *)commandPoolHandle;

		SF_QUEUE_REMOVE(&commandBuffer->queue);
		SF_QUEUE_INSERT_HEAD(&renderer->freeCommandBufferQueue, &commandBuffer->queue);

		if (renderer->vulkanDevice && commandPool->vulkanCommandPool && commandBuffer->vulkanCommandBuffer) {
			vkFreeCommandBuffers(renderer->vulkanDevice, commandPool->vulkanCommandPool, 1,
					     &commandBuffer->vulkanCommandBuffer);
			commandBuffer->vulkanCommandBuffer = VK_NULL_HANDLE;
		}
	}
}

SfHandle sfCreateGraphicsCommandBuffer(SfGraphicsRenderer *renderer, SfHandle commandPoolHandle, SfBool secondary) {
	SfGraphicsCommandBuffer *commandBuffer = sfGetOrAllocateGraphicsCommandBuffer(renderer);
	if (commandBuffer) {
		SfGraphicsCommandPool		 *commandPool = (SfGraphicsCommandPool *)commandPoolHandle;
		VkCommandBufferAllocateInfo const info	      = {
			   .sType	= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			   .pNext	= NULL,
			   .commandPool = commandPool->vulkanCommandPool,
			   .level	= secondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			   .commandBufferCount = 1};
		if (!SF_VULKAN_CHECK(vkAllocateCommandBuffers(
			renderer->vulkanDevice, &info, &commandBuffer->vulkanCommandBuffer))) {
			sfDestroyGraphicsCommandBuffer(renderer, commandPoolHandle, SF_AS_HANDLE(commandBuffer));
			commandBuffer = SF_NULL_HANDLE;
		}
	}
	return SF_AS_HANDLE(commandBuffer);
}

SF_INTERNAL VkImageType sfAsVulkanImageType(SfGraphicsTextureType type) {
	switch (type) {
		case SF_GRAPHICS_TEXTURE_TYPE_1D: return VK_IMAGE_TYPE_1D;
		case SF_GRAPHICS_TEXTURE_TYPE_2D: return VK_IMAGE_TYPE_2D;
		case SF_GRAPHICS_TEXTURE_TYPE_3D: return VK_IMAGE_TYPE_3D;
		case SF_GRAPHICS_TEXTURE_TYPE_CUBE: return VK_IMAGE_TYPE_2D;
	}
}

SF_INTERNAL VkFormat sfAsVulkanFormat(SfGraphicsFormat format) {
	switch (format) {
		case SF_GRAPHICS_FORMAT_UNDEFINED: return VK_FORMAT_UNDEFINED;
		case SF_GRAPHICS_FORMAT_R8_UNORM: return VK_FORMAT_R8_UNORM;
		case SF_GRAPHICS_FORMAT_R16_UNORM: return VK_FORMAT_R16_UNORM;
		case SF_GRAPHICS_FORMAT_R16_UINT: return VK_FORMAT_R16_UINT;
		case SF_GRAPHICS_FORMAT_R16_SFLOAT: return VK_FORMAT_R16_SFLOAT;
		case SF_GRAPHICS_FORMAT_R32_UINT: return VK_FORMAT_R32_UINT;
		case SF_GRAPHICS_FORMAT_R32_SFLOAT: return VK_FORMAT_R32_SFLOAT;
		case SF_GRAPHICS_FORMAT_R8G8_UNORM: return VK_FORMAT_R8G8_UNORM;
		case SF_GRAPHICS_FORMAT_R16G16_UNORM: return VK_FORMAT_R16G16_UNORM;
		case SF_GRAPHICS_FORMAT_R16G16_SFLOAT: return VK_FORMAT_R16G16_SFLOAT;
		case SF_GRAPHICS_FORMAT_R32G32_UINT: return VK_FORMAT_R32G32_UINT;
		case SF_GRAPHICS_FORMAT_R32G32_SFLOAT: return VK_FORMAT_R32G32_SFLOAT;
		case SF_GRAPHICS_FORMAT_R8G8B8_UNORM: return VK_FORMAT_R8G8B8_UNORM;
		case SF_GRAPHICS_FORMAT_R16G16B16_UNORM: return VK_FORMAT_R16G16B16_UNORM;
		case SF_GRAPHICS_FORMAT_R16G16B16_SFLOAT: return VK_FORMAT_R16G16B16_SFLOAT;
		case SF_GRAPHICS_FORMAT_R32G32B32_UINT: return VK_FORMAT_R32G32B32_UINT;
		case SF_GRAPHICS_FORMAT_R32G32B32_SFLOAT: return VK_FORMAT_R32G32B32_SFLOAT;
		case SF_GRAPHICS_FORMAT_B8G8R8A8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
		case SF_GRAPHICS_FORMAT_R8G8B8A8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
		case SF_GRAPHICS_FORMAT_R16G16B16A16_UNORM: return VK_FORMAT_R16G16B16A16_UNORM;
		case SF_GRAPHICS_FORMAT_R16G16B16A16_SFLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
		case SF_GRAPHICS_FORMAT_R32G32B32A32_UINT: return VK_FORMAT_R32G32B32A32_UINT;
		case SF_GRAPHICS_FORMAT_R32G32B32A32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
		case SF_GRAPHICS_FORMAT_D16_UNORM: return VK_FORMAT_D16_UNORM;
		case SF_GRAPHICS_FORMAT_X8_D24_UNORM_PACK32: return VK_FORMAT_X8_D24_UNORM_PACK32;
		case SF_GRAPHICS_FORMAT_D32_SFLOAT: return VK_FORMAT_D32_SFLOAT;
		case SF_GRAPHICS_FORMAT_S8_UINT: return VK_FORMAT_S8_UINT;
		case SF_GRAPHICS_FORMAT_D16_UNORM_S8_UINT: return VK_FORMAT_D16_UNORM_S8_UINT;
		case SF_GRAPHICS_FORMAT_D24_UNORM_S8_UINT: return VK_FORMAT_D24_UNORM_S8_UINT;
		case SF_GRAPHICS_FORMAT_D32_SFLOAT_S8_UINT: return VK_FORMAT_D32_SFLOAT_S8_UINT;
		default: return VK_FORMAT_UNDEFINED;
	}
}

SF_INTERNAL VkImageUsageFlags sfAsVulkanUsage(SfGraphicsTextureUsage usage) {
	VkImageUsageFlags result = 0;
	if (SF_GRAPHICS_TEXTURE_USAGE_TRANSFER_SRC == (usage & SF_GRAPHICS_TEXTURE_USAGE_TRANSFER_SRC))
		result |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	if (SF_GRAPHICS_TEXTURE_USAGE_TRANSFER_DST == (usage & SF_GRAPHICS_TEXTURE_USAGE_TRANSFER_DST)) {
		result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	if (SF_GRAPHICS_TEXTURE_USAGE_SAMPLED == (usage & SF_GRAPHICS_TEXTURE_USAGE_SAMPLED))
		result |= (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			   VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

	if (SF_GRAPHICS_TEXTURE_USAGE_STORAGE == (usage & SF_GRAPHICS_TEXTURE_USAGE_STORAGE)) {
		result |= VK_IMAGE_USAGE_STORAGE_BIT;
	}
	if (SF_GRAPHICS_TEXTURE_USAGE_COLOR_ATTACHMENT == (usage & SF_GRAPHICS_TEXTURE_USAGE_COLOR_ATTACHMENT)) {
		result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	if (SF_GRAPHICS_TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT ==
	    (usage & SF_GRAPHICS_TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT)) {
		result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	if (SF_GRAPHICS_TEXTURE_USAGE_RESOLVE_SRC == (usage & SF_GRAPHICS_TEXTURE_USAGE_RESOLVE_SRC)) {
		result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	if (SF_GRAPHICS_TEXTURE_USAGE_RESOLVE_DST == (usage & SF_GRAPHICS_TEXTURE_USAGE_RESOLVE_DST)) {
		result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	return result;
}

SF_INTERNAL VkSampleCountFlagBits sfAsVulkanSampleCount(SfGraphicsSampleCount samples) {
	switch (samples) {
		case SF_GRAPHICS_SAMPLE_COUNT_1: return VK_SAMPLE_COUNT_1_BIT;
		case SF_GRAPHICS_SAMPLE_COUNT_2: return VK_SAMPLE_COUNT_2_BIT;
		case SF_GRAPHICS_SAMPLE_COUNT_4: return VK_SAMPLE_COUNT_4_BIT;
		case SF_GRAPHICS_SAMPLE_COUNT_8: return VK_SAMPLE_COUNT_8_BIT;
		case SF_GRAPHICS_SAMPLE_COUNT_16: return VK_SAMPLE_COUNT_16_BIT;
	}
}

SF_INTERNAL VkImageViewType sfAsVulkanImageViewType(SfGraphicsTextureType type) {
	switch (type) {
		case SF_GRAPHICS_TEXTURE_TYPE_1D: return VK_IMAGE_VIEW_TYPE_1D;
		case SF_GRAPHICS_TEXTURE_TYPE_2D: return VK_IMAGE_VIEW_TYPE_2D;
		case SF_GRAPHICS_TEXTURE_TYPE_3D: return VK_IMAGE_VIEW_TYPE_3D;
		case SF_GRAPHICS_TEXTURE_TYPE_CUBE: return VK_IMAGE_VIEW_TYPE_CUBE;
	}
}

SF_INTERNAL VkImageAspectFlags sfFindVulkanAspectFlags(VkFormat format) {
	switch (format) {
		case VK_FORMAT_D16_UNORM:
		case VK_FORMAT_X8_D24_UNORM_PACK32:
		case VK_FORMAT_D32_SFLOAT: return VK_IMAGE_ASPECT_DEPTH_BIT;

		case VK_FORMAT_S8_UINT: return VK_IMAGE_ASPECT_STENCIL_BIT;

		case VK_FORMAT_D16_UNORM_S8_UINT:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D32_SFLOAT_S8_UINT: return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

		default: return VK_IMAGE_ASPECT_COLOR_BIT;
	}
}

SfHandle sfCreateGraphicsTexture(SfGraphicsRenderer *renderer, SfGraphicsTextureDescription const *description) {
	SfGraphicsTexture *texture = sfGetOrAllocateGraphicsTexture(renderer);
	if (texture) {
		texture->type	    = description->type;
		texture->samples    = description->samples;
		texture->format	    = description->format;
		texture->usage	    = description->usage;
		texture->width	    = description->width;
		texture->height	    = description->height;
		texture->depth	    = description->depth;
		texture->mips	    = description->mips;
		texture->mapped	    = description->mapped;
		texture->clearValue = description->clearValue;

		if (description->vulkanNotOwnedImage) {
			texture->ownsImage   = SF_FALSE;
			texture->vulkanImage = description->vulkanNotOwnedImage;
			texture->mapped	     = SF_FALSE;
		} else {
			VkImageCreateInfo const info = {
			    .sType	   = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			    .pNext	   = NULL,
			    .flags	   = 0,
			    .imageType	   = sfAsVulkanImageType(texture->type),
			    .format	   = sfAsVulkanFormat(texture->format),
			    .extent.width  = texture->width,
			    .extent.height = texture->height,
			    .extent.depth  = texture->depth,
			    .mipLevels	   = texture->mips,
			    .arrayLayers   = 1,
			    .samples	   = sfAsVulkanSampleCount(texture->samples),
			    .tiling	   = texture->mapped ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL,
			    .usage	   = sfAsVulkanUsage(texture->usage),
			    .sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
			    .queueFamilyIndexCount = 0,
			    .pQueueFamilyIndices   = NULL,
			    .initialLayout	   = VK_IMAGE_LAYOUT_UNDEFINED};

			if (SF_VULKAN_CHECK(vkCreateImage(renderer->vulkanDevice, &info,
							  renderer->vulkanAllocationCallbacks,
							  &texture->vulkanImage))) {
				VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

				if (texture->mapped)
					flags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

				texture->vulkanMemory = sfVulkanAllocateAndBindMemoryForImage(
				    renderer, texture->vulkanImage, flags);
				if (texture->vulkanMemory && texture->mapped) {
					SF_VULKAN_CHECK(vkMapMemory(renderer->vulkanDevice, texture->vulkanMemory, 0,
								    VK_WHOLE_SIZE, 0, &texture->mappedData));
				}
			}
		}

		if (texture->vulkanImage && (texture->ownsImage ? !!texture->vulkanMemory : SF_TRUE) &&
		    (texture->mapped ? !!texture->mappedData : SF_TRUE)) {
			VkImageViewCreateInfo const info = {
			    .sType	      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			    .pNext	      = NULL,
			    .flags	      = 0,
			    .image	      = texture->vulkanImage,
			    .viewType	      = sfAsVulkanImageViewType(texture->type),
			    .format	      = sfAsVulkanFormat(texture->format),
			    .components.r     = VK_COMPONENT_SWIZZLE_R,
			    .components.g     = VK_COMPONENT_SWIZZLE_G,
			    .components.b     = VK_COMPONENT_SWIZZLE_B,
			    .components.a     = VK_COMPONENT_SWIZZLE_A,
			    .subresourceRange = {
				.aspectMask	= sfFindVulkanAspectFlags(sfAsVulkanFormat(texture->format)),
				.baseMipLevel	= 0,
				.levelCount	= texture->mips,
				.baseArrayLayer = 0,
				.layerCount	= 1,
			    }};

			if (SF_VULKAN_CHECK(vkCreateImageView(renderer->vulkanDevice, &info,
							      renderer->vulkanAllocationCallbacks,
							      &texture->vulkanImageView))) {
				texture->vulkanAspectFlags = info.subresourceRange.aspectMask;
				texture->vulkanLayout	   = (SF_GRAPHICS_TEXTURE_USAGE_STORAGE & texture->usage) ==
								     SF_GRAPHICS_TEXTURE_USAGE_STORAGE
								 ? VK_IMAGE_LAYOUT_GENERAL
								 : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}
		}

		if (!texture->vulkanImage || !(texture->ownsImage ? !!texture->vulkanMemory : SF_TRUE) ||
		    !(texture->mapped ? !!texture->mappedData : SF_TRUE) || !texture->vulkanImageView) {
			sfDestroyGraphicsTexture(renderer, SF_AS_HANDLE(texture));
			texture = SF_NULL_HANDLE;
		}
	}

	return SF_AS_HANDLE(texture);
}

SF_INTERNAL uint32_t sfCalculateMipLevels(uint32_t width, uint32_t height) {
	uint32_t result = 0;

	if (width != 0 && height != 0) {
		result = 1;
		while (width > 1 || height > 1) {
			width >>= 1;
			height >>= 1;
			result++;
		}
	}

	return result;
}

SF_INTERNAL void sfDefaultInitGraphicsRenderTarget(SfGraphicsRenderTarget *renderTarget) {
	SF_QUEUE_INIT(&renderTarget->queue);

	renderTarget->samples		   = SF_GRAPHICS_SAMPLE_COUNT_1;
	renderTarget->colorFormat	   = SF_GRAPHICS_FORMAT_UNDEFINED;
	renderTarget->depthStencilFormat   = SF_GRAPHICS_FORMAT_UNDEFINED;
	renderTarget->width		   = 0;
	renderTarget->height		   = 0;
	renderTarget->colorAttachmentCount = 0;

	for (uint32_t i = 0; i < SF_SIZE(renderTarget->colorAttachmentClearValues); ++i)
		renderTarget->colorAttachmentClearValues[i].type = SF_GRAPHICS_CLEAR_VALUE_TYPE_NONE;

	renderTarget->depthStencilAttachmentClearValue.type = SF_GRAPHICS_CLEAR_VALUE_TYPE_NONE;
	renderTarget->depthStencilAttachment		    = SF_NULL_HANDLE;
	renderTarget->depthStencilMultisamplingAttachment   = SF_NULL_HANDLE;

	renderTarget->colorAttachmentCount = 0;
	SF_ARRAY_INIT(renderTarget->colorAttachments, SF_NULL_HANDLE);

	renderTarget->colorMultisampleAttachmentCount = 0;
	SF_ARRAY_INIT(renderTarget->colorMultisampleAttachments, SF_NULL_HANDLE);

	renderTarget->vulkanSwapchainImage = VK_NULL_HANDLE;
	renderTarget->vulkanRenderPass	   = VK_NULL_HANDLE;
	renderTarget->vulkanFramebuffer	   = VK_NULL_HANDLE;
}

SF_INTERNAL SfGraphicsRenderTarget *sfGetOrAllocateGraphicsRenderTarget(SfGraphicsRenderer *renderer) {
	SfGraphicsRenderTarget *result = NULL;

	if (SF_QUEUE_IS_EMPTY(&renderer->freeRenderTargetQueue)) {
		result = sfAllocate(&renderer->arena, sizeof(*result));
	} else {
		SfQueue *queue = SF_QUEUE_HEAD(&renderer->freeRenderTargetQueue);
		SF_QUEUE_REMOVE(queue);
		result = SF_QUEUE_DATA(queue, SfGraphicsRenderTarget, queue);
	}
	if (result) {
		sfDefaultInitGraphicsRenderTarget(result);
		SF_QUEUE_INSERT_HEAD(&renderer->renderTargetQueue, &result->queue);
	}

	return result;
}

void sfDestroyGraphicsRenderTarget(SfGraphicsRenderer *renderer, SfHandle renderTargetHandle) {
	if (renderer && renderTargetHandle) {
		SfGraphicsRenderTarget *renderTarget = (SfGraphicsRenderTarget *)renderTargetHandle;

		SF_QUEUE_REMOVE(&renderTarget->queue);
		SF_QUEUE_INSERT_HEAD(&renderer->freeTextureQueue, &renderTarget->queue);

		if (renderer->vulkanDevice) {
			if (renderTarget->vulkanFramebuffer) {
				vkDestroyFramebuffer(renderer->vulkanDevice, renderTarget->vulkanFramebuffer,
						     renderer->vulkanAllocationCallbacks);
				renderTarget->vulkanFramebuffer = VK_NULL_HANDLE;
			}

			if (renderTarget->vulkanRenderPass) {
				vkDestroyRenderPass(renderer->vulkanDevice, renderTarget->vulkanRenderPass,
						    renderer->vulkanAllocationCallbacks);
				renderTarget->vulkanRenderPass = VK_NULL_HANDLE;
			}
		}

		sfDestroyGraphicsTexture(renderer, renderTarget->depthStencilMultisamplingAttachment);
		sfDestroyGraphicsTexture(renderer, renderTarget->depthStencilAttachment);

		for (uint32_t i = 0; i < SF_SIZE(renderTarget->colorMultisampleAttachments); ++i) {
			sfDestroyGraphicsTexture(renderer, renderTarget->colorMultisampleAttachments[i]);
			renderTarget->colorMultisampleAttachments[i] = SF_NULL_HANDLE;
		}
		renderTarget->colorMultisampleAttachmentCount = 0;

		for (uint32_t i = 0; i < SF_SIZE(renderTarget->colorAttachments); ++i) {
			sfDestroyGraphicsTexture(renderer, renderTarget->colorAttachments[i]);
			renderTarget->colorAttachments[i] = SF_NULL_HANDLE;
		}
		renderTarget->colorAttachmentCount = 0;
	}
}

SF_INTERNAL SfBool sfValidateGraphicsRenderTargetAttachments(SfGraphicsRenderTarget const *renderTarget) {
	SfBool result = SF_TRUE;

	for (uint32_t i = 0; i < renderTarget->colorAttachmentCount && result; ++i)
		result = !!renderTarget->colorAttachments[i];

	for (uint32_t i = 0; i < renderTarget->colorMultisampleAttachmentCount && result; ++i)
		result = !!renderTarget->colorMultisampleAttachments[i];

	if (result && renderTarget->depthStencilFormat != SF_GRAPHICS_FORMAT_UNDEFINED)
		result = result && !!renderTarget->depthStencilAttachment;

	return result;
}

SfHandle sfCreateGraphicsRenderTarget(SfGraphicsRenderer		      *renderer,
				      SfGraphicsRenderTargetDescription const *description) {
	SfGraphicsRenderTarget *renderTarget = sfGetOrAllocateGraphicsRenderTarget(renderer);
	if (renderTarget) {
		SfArena *arena = &renderer->renderTargetArena;

		renderTarget->samples		 = description->sampleCount;
		renderTarget->colorFormat	 = description->colorFormat;
		renderTarget->depthStencilFormat = description->depthStencilFormat;
		renderTarget->width		 = description->width;
		renderTarget->height		 = description->height;

		renderTarget->colorAttachmentCount = description->colorAttachmentCount;
		for (uint32_t i = 0; i < renderTarget->colorAttachmentCount; ++i)
			renderTarget->colorAttachmentClearValues[i] = description->colorAttachmentClearValues[i];

		renderTarget->depthStencilAttachmentClearValue = description->depthStencilAttachmentClearValue;
		renderTarget->vulkanSwapchainImage	       = description->vulkanNotOwnedImage;

		uint32_t		 totalAttachmentCount		  = 0;
		VkAttachmentDescription *attachmentDescriptions		  = NULL;
		VkAttachmentReference	*colorAttachmentReferences	  = NULL;
		VkAttachmentReference	*colorResolveAttachmentReferences = NULL;
		VkAttachmentReference	 depthStencilReference		  = {0};
		VkImageView		*attachmentImageViews		  = NULL;

		if (renderTarget->samples == SF_GRAPHICS_SAMPLE_COUNT_1) {
			totalAttachmentCount = renderTarget->colorAttachmentCount;
			if (renderTarget->depthStencilFormat != SF_GRAPHICS_FORMAT_UNDEFINED)
				++totalAttachmentCount;

			attachmentDescriptions = sfAllocate(
			    arena, totalAttachmentCount * sizeof(*attachmentDescriptions));
			colorAttachmentReferences = sfAllocate(
			    arena, renderTarget->colorAttachmentCount * sizeof(*colorResolveAttachmentReferences));
			attachmentImageViews = sfAllocate(arena, totalAttachmentCount * sizeof(*attachmentImageViews));
			if (attachmentDescriptions && colorAttachmentReferences && attachmentImageViews) {
				for (uint32_t i = 0; i < renderTarget->colorAttachmentCount; ++i) {
					uint32_t		    colorIndex = i;
					SfGraphicsClearValue const *clearValue =
					    &description->colorAttachmentClearValues[i];

					SfGraphicsTextureDescription const textureDescription = {
					    .type    = SF_GRAPHICS_TEXTURE_TYPE_2D,
					    .width   = renderTarget->width,
					    .height  = renderTarget->height,
					    .samples = renderTarget->samples,
					    .format  = renderTarget->colorFormat,
					    .mips    = 1,
					    .mapped  = SF_FALSE,
					    .usage   = SF_GRAPHICS_TEXTURE_USAGE_COLOR_ATTACHMENT |
						     SF_GRAPHICS_TEXTURE_USAGE_SAMPLED,
					    .vulkanNotOwnedImage = renderTarget->vulkanSwapchainImage,
					    .clearValue		 = *clearValue};

					renderTarget->colorAttachments[i] = sfCreateGraphicsTexture(
					    renderer, &textureDescription);
					if (renderTarget->colorAttachments[i]) {
						attachmentDescriptions[colorIndex].flags  = 0;
						attachmentDescriptions[colorIndex].format = sfAsVulkanFormat(
						    renderTarget->colorFormat);
						attachmentDescriptions[colorIndex].samples = VK_SAMPLE_COUNT_1_BIT;
						attachmentDescriptions[colorIndex].loadOp =
						    VK_ATTACHMENT_LOAD_OP_CLEAR;
						attachmentDescriptions[colorIndex].storeOp =
						    VK_ATTACHMENT_STORE_OP_STORE;
						attachmentDescriptions[colorIndex].stencilLoadOp =
						    VK_ATTACHMENT_LOAD_OP_CLEAR;
						attachmentDescriptions[colorIndex].stencilStoreOp =
						    VK_ATTACHMENT_STORE_OP_STORE;
						attachmentDescriptions[colorIndex].initialLayout =
						    VK_IMAGE_LAYOUT_UNDEFINED;
						if (renderTarget->vulkanSwapchainImage)
							attachmentDescriptions[colorIndex].finalLayout =
							    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
						else
							attachmentDescriptions[colorIndex].finalLayout =
							    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

						colorAttachmentReferences[i].attachment = colorIndex;
						colorAttachmentReferences[i].layout =
						    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

						attachmentImageViews[colorIndex] =
						    ((SfGraphicsTexture *)renderTarget->colorAttachments[i])
							->vulkanImageView;
					}
				}
			}
		} else {
			renderTarget->colorMultisampleAttachmentCount = description->colorAttachmentCount;

			totalAttachmentCount = 2 * renderTarget->colorAttachmentCount;
			if (renderTarget->depthStencilFormat != SF_GRAPHICS_FORMAT_UNDEFINED)
				++totalAttachmentCount;

			attachmentDescriptions = sfAllocate(
			    arena, totalAttachmentCount * sizeof(*attachmentDescriptions));
			colorAttachmentReferences = sfAllocate(
			    arena, renderTarget->colorAttachmentCount * sizeof(*colorAttachmentReferences));
			colorResolveAttachmentReferences = sfAllocate(
			    arena, renderTarget->colorAttachmentCount * sizeof(*colorAttachmentReferences));
			attachmentImageViews = sfAllocate(arena, totalAttachmentCount * sizeof(*attachmentImageViews));
			if (attachmentDescriptions && colorAttachmentReferences && colorResolveAttachmentReferences &&
			    attachmentImageViews) {
				for (uint32_t i = 0; i < renderTarget->colorAttachmentCount; ++i) {
					uint32_t		    colorIndex	  = 2 * i;
					uint32_t		    resolve_index = colorIndex + 1;
					SfGraphicsClearValue const *clearValue =
					    &description->colorAttachmentClearValues[i];

					SfGraphicsTextureDescription const colorAttachmentDescription = {
					    .type    = SF_GRAPHICS_TEXTURE_TYPE_2D,
					    .width   = renderTarget->width,
					    .height  = renderTarget->height,
					    .samples = renderTarget->samples,
					    .format  = renderTarget->colorFormat,
					    .mips    = 1,
					    .mapped  = SF_FALSE,
					    .usage   = SF_GRAPHICS_TEXTURE_USAGE_COLOR_ATTACHMENT |
						     SF_GRAPHICS_TEXTURE_USAGE_SAMPLED,
					    .vulkanNotOwnedImage = renderTarget->vulkanSwapchainImage,
					    .clearValue		 = *clearValue};
					renderTarget->colorAttachments[i] = sfCreateGraphicsTexture(
					    renderer, &colorAttachmentDescription);

					SfGraphicsTextureDescription const resolveAttachmentDescription = {
					    .type    = SF_GRAPHICS_TEXTURE_TYPE_2D,
					    .width   = renderTarget->width,
					    .height  = renderTarget->height,
					    .samples = renderTarget->samples,
					    .format  = renderTarget->colorFormat,
					    .mips    = 1,
					    .mapped  = SF_FALSE,
					    .usage   = SF_GRAPHICS_TEXTURE_USAGE_COLOR_ATTACHMENT |
						     SF_GRAPHICS_TEXTURE_USAGE_SAMPLED,
					    .vulkanNotOwnedImage = VK_NULL_HANDLE,
					    .clearValue		 = *clearValue};
					renderTarget->colorMultisampleAttachments[i] = sfCreateGraphicsTexture(
					    renderer, &resolveAttachmentDescription);
					if (renderTarget->colorAttachments[i] &&
					    renderTarget->colorMultisampleAttachments[i]) {
						attachmentDescriptions[colorIndex].flags  = 0;
						attachmentDescriptions[colorIndex].format = sfAsVulkanFormat(
						    renderTarget->colorFormat);
						attachmentDescriptions[colorIndex].samples = VK_SAMPLE_COUNT_1_BIT;
						attachmentDescriptions[colorIndex].loadOp =
						    VK_ATTACHMENT_LOAD_OP_CLEAR;
						attachmentDescriptions[colorIndex].storeOp =
						    VK_ATTACHMENT_STORE_OP_STORE;
						attachmentDescriptions[colorIndex].stencilLoadOp =
						    VK_ATTACHMENT_LOAD_OP_CLEAR;
						attachmentDescriptions[colorIndex].stencilStoreOp =
						    VK_ATTACHMENT_STORE_OP_STORE;
						attachmentDescriptions[colorIndex].initialLayout =
						    VK_IMAGE_LAYOUT_UNDEFINED;
						if (renderTarget->vulkanSwapchainImage)
							attachmentDescriptions[colorIndex].finalLayout =
							    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
						else
							attachmentDescriptions[colorIndex].finalLayout =
							    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

						attachmentDescriptions[resolve_index].flags  = 0;
						attachmentDescriptions[resolve_index].format = sfAsVulkanFormat(
						    renderTarget->colorFormat);
						attachmentDescriptions[resolve_index].samples = sfAsVulkanSampleCount(
						    renderTarget->samples);
						attachmentDescriptions[resolve_index].loadOp =
						    VK_ATTACHMENT_LOAD_OP_CLEAR;
						attachmentDescriptions[resolve_index].storeOp =
						    VK_ATTACHMENT_STORE_OP_STORE;
						attachmentDescriptions[resolve_index].stencilLoadOp =
						    VK_ATTACHMENT_LOAD_OP_CLEAR;
						attachmentDescriptions[resolve_index].stencilStoreOp =
						    VK_ATTACHMENT_STORE_OP_STORE;
						attachmentDescriptions[resolve_index].initialLayout =
						    VK_IMAGE_LAYOUT_UNDEFINED;
						attachmentDescriptions[resolve_index].finalLayout =
						    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

						colorAttachmentReferences[i].attachment = colorIndex;
						colorAttachmentReferences[i].layout =
						    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

						colorResolveAttachmentReferences[i].attachment = resolve_index;
						colorResolveAttachmentReferences[i].layout =
						    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

						attachmentImageViews[colorIndex] =
						    ((SfGraphicsTexture *)renderTarget->colorAttachments[i])
							->vulkanImageView;
						attachmentImageViews[resolve_index] =
						    ((SfGraphicsTexture *)renderTarget->colorMultisampleAttachments[i])
							->vulkanImageView;
					}
				}
			}
		}

		if (renderTarget->depthStencilFormat != SF_GRAPHICS_FORMAT_UNDEFINED) {
			uint32_t			   depthStencilIndex = totalAttachmentCount - 1;

			SfGraphicsTextureDescription const textureDescription = {
			    .type    = SF_GRAPHICS_TEXTURE_TYPE_2D,
			    .width   = renderTarget->width,
			    .height  = renderTarget->height,
			    .samples = renderTarget->samples,
			    .format  = renderTarget->depthStencilFormat,
			    .mips    = 1,
			    .mapped  = SF_FALSE,
			    .usage   = SF_GRAPHICS_TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT |
				     SF_GRAPHICS_TEXTURE_USAGE_SAMPLED,
			    .vulkanNotOwnedImage = VK_NULL_HANDLE,
			    .clearValue		 = description->depthStencilAttachmentClearValue};

			renderTarget->depthStencilAttachment = sfCreateGraphicsTexture(renderer, &textureDescription);
			if (renderTarget->depthStencilAttachment) {
				attachmentDescriptions[depthStencilIndex].flags	 = 0;
				attachmentDescriptions[depthStencilIndex].format = sfAsVulkanFormat(
				    renderTarget->depthStencilFormat);
				attachmentDescriptions[depthStencilIndex].samples = sfAsVulkanSampleCount(
				    renderTarget->samples);
				attachmentDescriptions[depthStencilIndex].loadOp	= VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachmentDescriptions[depthStencilIndex].storeOp	= VK_ATTACHMENT_STORE_OP_STORE;
				attachmentDescriptions[depthStencilIndex].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachmentDescriptions[depthStencilIndex].stencilStoreOp =
				    VK_ATTACHMENT_STORE_OP_STORE;
				attachmentDescriptions[depthStencilIndex].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attachmentDescriptions[depthStencilIndex].finalLayout =
				    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

				depthStencilReference.attachment = depthStencilIndex;
				depthStencilReference.layout	 = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

				attachmentImageViews[depthStencilIndex] =
				    ((SfGraphicsTexture *)renderTarget->depthStencilAttachment)->vulkanImageView;
			}
		}

		if (sfValidateGraphicsRenderTargetAttachments(renderTarget)) {
			VkRenderPassCreateInfo const renderPassInfo = {
			    .sType	     = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			    .pNext	     = NULL,
			    .flags	     = 0,
			    .attachmentCount = totalAttachmentCount,
			    .pAttachments    = attachmentDescriptions,
			    .subpassCount    = 1,
			    .pSubpasses	     = &(
				     VkSubpassDescription){.flags		    = 0,
							   .pipelineBindPoint	    = VK_PIPELINE_BIND_POINT_GRAPHICS,
							   .inputAttachmentCount    = 0,
							   .pInputAttachments	    = NULL,
							   .colorAttachmentCount    = renderTarget->colorAttachmentCount,
							   .pColorAttachments	    = colorAttachmentReferences,
							   .pResolveAttachments	    = colorResolveAttachmentReferences,
							   .pDepthStencilAttachment = renderTarget->depthStencilFormat !=
											      SF_GRAPHICS_FORMAT_UNDEFINED
											  ? &depthStencilReference
											  : NULL,
							   .preserveAttachmentCount = 0,
							   .pPreserveAttachments    = NULL},
			    .dependencyCount = 1,
			    .pDependencies   = &(VkSubpassDependency){
				  .srcSubpass	   = 0,
				  .dstSubpass	   = 0,
				  .srcStageMask	   = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				  .dstStageMask	   = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				  .srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				  .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				  .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT}};

			if (SF_VULKAN_CHECK(vkCreateRenderPass(renderer->vulkanDevice, &renderPassInfo,
							       renderer->vulkanAllocationCallbacks,
							       &renderTarget->vulkanRenderPass))) {
				VkFramebufferCreateInfo const framebufferInfo = {
				    .sType	     = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				    .pNext	     = NULL,
				    .flags	     = 0,
				    .renderPass	     = renderTarget->vulkanRenderPass,
				    .attachmentCount = totalAttachmentCount,
				    .pAttachments    = attachmentImageViews,
				    .width	     = renderTarget->width,
				    .height	     = renderTarget->height,
				    .layers	     = 1};
				SF_VULKAN_CHECK(vkCreateFramebuffer(renderer->vulkanDevice, &framebufferInfo,
								    renderer->vulkanAllocationCallbacks,
								    &renderTarget->vulkanFramebuffer));
			}
		}

		if (!sfValidateGraphicsRenderTargetAttachments(renderTarget) || !renderTarget->vulkanRenderPass ||
		    !renderTarget->vulkanFramebuffer) {
			sfDestroyGraphicsRenderTarget(renderer, SF_AS_HANDLE(renderTarget));
			renderTarget = SF_NULL_HANDLE;
		}
	}

	return SF_AS_HANDLE(renderTarget);
}

SF_INTERNAL void sfDefaultInitGraphicsProgram(SfGraphicsProgram *program) {
	SF_QUEUE_INIT(&program->queue);
	program->stageFlags			    = 0;
	program->vulkanVertexShader		    = VK_NULL_HANDLE;
	program->vulkanTessellationControlShader    = VK_NULL_HANDLE;
	program->vulkanTessellationEvaluationShader = VK_NULL_HANDLE;
	program->vulkanGeometryShader		    = VK_NULL_HANDLE;
	program->vulkanComputeShader		    = VK_NULL_HANDLE;
	program->vulkanFragmentShader		    = VK_NULL_HANDLE;
}

SF_INTERNAL SfGraphicsProgram *sfGetOrAllocateGraphicsProgram(SfGraphicsRenderer *renderer) {
	SfGraphicsProgram *result = NULL;

	if (SF_QUEUE_IS_EMPTY(&renderer->freeProgramQueue)) {
		result = sfAllocate(&renderer->arena, sizeof(*result));
	} else {
		SfQueue *queue = SF_QUEUE_HEAD(&renderer->freeProgramQueue);
		SF_QUEUE_REMOVE(queue);
		result = SF_QUEUE_DATA(queue, SfGraphicsProgram, queue);
	}
	if (result) {
		sfDefaultInitGraphicsProgram(result);
		SF_QUEUE_INSERT_HEAD(&renderer->programQueue, &result->queue);
	}
	return result;
}

SF_INTERNAL VkShaderModule sfCreateVulkanShaderModule(SfGraphicsRenderer *renderer, uint32_t codeSizeInBytes,
						      void const *code) {
	VkShaderModule shader = VK_NULL_HANDLE;

	if (renderer && codeSizeInBytes && code) {
		VkShaderModuleCreateInfo const info = {
		    .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		    .pNext    = NULL,
		    .flags    = 0,
		    .codeSize = codeSizeInBytes,
		    .pCode    = code,
		};

		SF_VULKAN_CHECK(
		    vkCreateShaderModule(renderer->vulkanDevice, &info, renderer->vulkanAllocationCallbacks, &shader));
	}

	return shader;
}

void sfDestroyGraphicsProgram(SfGraphicsRenderer *renderer, SfHandle programHandle) {
	if (renderer && programHandle) {
		SfGraphicsProgram *program = (SfGraphicsProgram *)programHandle;

		SF_QUEUE_REMOVE(&program->queue);
		SF_QUEUE_INSERT_HEAD(&renderer->freeProgramQueue, &program->queue);

		if (renderer->vulkanDevice) {
			if (program->vulkanComputeShader) {
				vkDestroyShaderModule(renderer->vulkanDevice, program->vulkanComputeShader,
						      renderer->vulkanAllocationCallbacks);
				program->vulkanComputeShader = VK_NULL_HANDLE;
			}

			if (program->vulkanFragmentShader) {
				vkDestroyShaderModule(renderer->vulkanDevice, program->vulkanFragmentShader,
						      renderer->vulkanAllocationCallbacks);
				program->vulkanFragmentShader = VK_NULL_HANDLE;
			}

			if (program->vulkanGeometryShader) {
				vkDestroyShaderModule(renderer->vulkanDevice, program->vulkanGeometryShader,
						      renderer->vulkanAllocationCallbacks);
				program->vulkanGeometryShader = VK_NULL_HANDLE;
			}

			if (program->vulkanTessellationEvaluationShader) {
				vkDestroyShaderModule(renderer->vulkanDevice,
						      program->vulkanTessellationEvaluationShader,
						      renderer->vulkanAllocationCallbacks);
				program->vulkanTessellationEvaluationShader = VK_NULL_HANDLE;
			}

			if (program->vulkanTessellationControlShader) {
				vkDestroyShaderModule(renderer->vulkanDevice, program->vulkanTessellationControlShader,
						      renderer->vulkanAllocationCallbacks);
				program->vulkanTessellationControlShader = VK_NULL_HANDLE;
			}

			if (program->vulkanVertexShader) {
				vkDestroyShaderModule(renderer->vulkanDevice, program->vulkanVertexShader,
						      renderer->vulkanAllocationCallbacks);
				program->vulkanVertexShader = VK_NULL_HANDLE;
			}
		}
	}
}

SfHandle sfCreateGraphicsProgram(SfGraphicsRenderer *renderer, SfGraphicsProgramDescription const *description) {
	SfGraphicsProgram *program = sfGetOrAllocateGraphicsProgram(renderer);
	if (program) {
		if (description->vertexCodeSize) {
			program->stageFlags |= SF_GRAPHICS_SHADER_STAGE_VERTEX;
			program->vulkanVertexShader = sfCreateVulkanShaderModule(
			    renderer, description->vertexCodeSize, description->vertexCode);
		}

		if (description->tessellationControlCodeSize) {
			program->stageFlags |= SF_GRAPHICS_SHADER_STAGE_TESSELLATION_CONTROL;
			program->vulkanTessellationControlShader = sfCreateVulkanShaderModule(
			    renderer, description->tessellationControlCodeSize, description->tessellationControlCode);
		}

		if (description->tessellationEvaluationCodeSize) {
			program->stageFlags |= SF_GRAPHICS_SHADER_STAGE_TESSELLATION_EVALUATION;
			program->vulkanTessellationEvaluationShader = sfCreateVulkanShaderModule(
			    renderer, description->tessellationEvaluationCodeSize,
			    description->tessellationEvaluationCode);
		}

		if (description->geometryCodeSize) {
			program->stageFlags |= SF_GRAPHICS_SHADER_STAGE_GEOMETRY;
			program->vulkanGeometryShader = sfCreateVulkanShaderModule(
			    renderer, description->geometryCodeSize, description->geometryCode);
		}

		if (description->fragmentCodeSize) {
			program->stageFlags |= SF_GRAPHICS_SHADER_STAGE_FRAGMENT;
			program->vulkanFragmentShader = sfCreateVulkanShaderModule(
			    renderer, description->fragmentCodeSize, description->fragmentCode);
		}

		if (description->computeCodeSize) {
			program->stageFlags |= SF_GRAPHICS_SHADER_STAGE_COMPUTE;
			program->vulkanComputeShader = sfCreateVulkanShaderModule(
			    renderer, description->computeCodeSize, description->computeCode);
		}

		if ((program->stageFlags & SF_GRAPHICS_SHADER_STAGE_VERTEX) ? !program->vulkanVertexShader
		    : SF_FALSE || (program->stageFlags & SF_GRAPHICS_SHADER_STAGE_TESSELLATION_CONTROL)
			? !program->vulkanTessellationControlShader
		    : SF_FALSE || (program->stageFlags & SF_GRAPHICS_SHADER_STAGE_TESSELLATION_EVALUATION)
			? !program->vulkanTessellationEvaluationShader
		    : SF_FALSE || (program->stageFlags & SF_GRAPHICS_SHADER_STAGE_GEOMETRY)
			? !program->vulkanGeometryShader
		    : SF_FALSE || (program->stageFlags & SF_GRAPHICS_SHADER_STAGE_FRAGMENT)
			? !program->vulkanFragmentShader
		    : SF_FALSE || (program->stageFlags & SF_GRAPHICS_SHADER_STAGE_COMPUTE)
			? !program->vulkanComputeShader
			: SF_FALSE) {
			sfDestroyGraphicsProgram(renderer, SF_AS_HANDLE(program));
			program = SF_NULL_HANDLE;
		}
	}

	return SF_AS_HANDLE(program);
}

SF_INTERNAL VkDescriptorType sfAsVulkanDescriptorType(SfGraphicsDescriptorType type) {
	switch (type) {
		case SF_GRAPHICS_DESCRIPTOR_TYPE_SAMPLER: return VK_DESCRIPTOR_TYPE_SAMPLER;
		case SF_GRAPHICS_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case SF_GRAPHICS_DESCRIPTOR_TYPE_TEXTURE: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		default: return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}
}

SF_INTERNAL VkShaderStageFlags sfAsVulkanShaderStages(SfGraphicsShaderStageFlags stages) {
	VkShaderStageFlags result = 0;

	if (stages & SF_GRAPHICS_SHADER_STAGE_VERTEX)
		result |= VK_SHADER_STAGE_VERTEX_BIT;

	if (stages & SF_GRAPHICS_SHADER_STAGE_TESSELLATION_CONTROL)
		result |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

	if (stages & SF_GRAPHICS_SHADER_STAGE_TESSELLATION_EVALUATION)
		result |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

	if (stages & SF_GRAPHICS_SHADER_STAGE_GEOMETRY)
		result |= VK_SHADER_STAGE_GEOMETRY_BIT;

	if (stages & SF_GRAPHICS_SHADER_STAGE_COMPUTE)
		result |= VK_SHADER_STAGE_COMPUTE_BIT;

	return result;
}

SF_INTERNAL void sfDefaultInitGraphicsDescriptor(SfGraphicsDescriptor *descriptor) {
	descriptor->type       = SF_GRAPHICS_DESCRIPTOR_TYPE_SAMPLER;
	descriptor->stageFlags = 0;
	descriptor->binding    = 0;
	descriptor->entryCount = 0;
	SF_ARRAY_INIT(descriptor->entries, SF_NULL_HANDLE);
}

SF_INTERNAL void sfDefaultInitGraphicsDescriptorSet(SfGraphicsDescriptorSet *descriptorSet) {
	SF_QUEUE_INIT(&descriptorSet->queue);
	descriptorSet->descriptorCount = 0;
	for (uint32_t i = 0; SF_SIZE(descriptorSet->descriptors); ++i)
		sfDefaultInitGraphicsDescriptor(&descriptorSet->descriptors[i]);
	descriptorSet->vulkanDescriptorSetLayout = VK_NULL_HANDLE;
	descriptorSet->vulkanDescriptorPool	 = VK_NULL_HANDLE;
	descriptorSet->vulkanDescriptorSet	 = VK_NULL_HANDLE;
}

SF_INTERNAL SfGraphicsDescriptorSet *sfGetOrAllocateGraphicsDescriptorSet(SfGraphicsRenderer *renderer) {
	SfGraphicsDescriptorSet *result = NULL;

	if (SF_QUEUE_IS_EMPTY(&renderer->freeDescriptorSetQueue)) {
		result = sfAllocate(&renderer->arena, sizeof(*result));
	} else {
		SfQueue *queue = SF_QUEUE_HEAD(&renderer->freeDescriptorSetQueue);
		SF_QUEUE_REMOVE(queue);
		result = SF_QUEUE_DATA(queue, SfGraphicsDescriptorSet, queue);
	}

	if (result) {
		sfDefaultInitGraphicsDescriptorSet(result);
		SF_QUEUE_INSERT_HEAD(&renderer->descriptorSetQueue, &result->queue);
	}

	return result;
}

void sfDestroyGraphicsDescriptorSet(SfGraphicsRenderer *renderer, SfHandle descriptorSetHandle) {
	if (renderer && descriptorSetHandle) {
		SfGraphicsDescriptorSet *descriptorSet = (SfGraphicsDescriptorSet *)descriptorSetHandle;

		SF_QUEUE_REMOVE(&descriptorSet->queue);
		SF_QUEUE_INSERT_HEAD(&renderer->freeDescriptorSetQueue, &descriptorSet->queue);

		if (renderer->vulkanDevice) {
			if (descriptorSet->vulkanDescriptorPool && descriptorSet->vulkanDescriptorSet) {
				vkFreeDescriptorSets(renderer->vulkanDevice, descriptorSet->vulkanDescriptorPool, 1,
						     &descriptorSet->vulkanDescriptorSet);
				descriptorSet->vulkanDescriptorSet = VK_NULL_HANDLE;
			}

			if (descriptorSet->vulkanDescriptorPool) {
				vkDestroyDescriptorPool(renderer->vulkanDevice, descriptorSet->vulkanDescriptorPool,
							renderer->vulkanAllocationCallbacks);
				descriptorSet->vulkanDescriptorPool = VK_NULL_HANDLE;
			}

			if (descriptorSet->vulkanDescriptorSetLayout) {
				vkDestroyDescriptorSetLayout(renderer->vulkanDevice,
							     descriptorSet->vulkanDescriptorSetLayout,
							     renderer->vulkanAllocationCallbacks);
				descriptorSet->vulkanDescriptorSetLayout = VK_NULL_HANDLE;
			}
		}
	}
}

SfHandle sfCreateGraphicsDescriptorSet(SfGraphicsRenderer			*renderer,
				       SfGraphicsDescriptorSetDescription const *description) {
	SfGraphicsDescriptorSet *descriptorSet = sfGetOrAllocateGraphicsDescriptorSet(renderer);
	if (descriptorSet) {
		descriptorSet->descriptorCount = description->descriptorCount;
		for (uint32_t i = 0; i < descriptorSet->descriptorCount; ++i)
			descriptorSet->descriptors[i] = description->descriptors[i];

		uint32_t descriptor_count_by_type[SF_GRAPHICS_DESCRIPTOR_TYPE_COUNT] = {0};
		for (uint32_t i = 0; i < descriptorSet->descriptorCount; ++i)
			++descriptor_count_by_type[description->descriptors[i].type];

		VkDescriptorPoolSize	     pool_sizes[SF_GRAPHICS_MAX_DESCRIPTOR_COUNT] = {0};
		VkDescriptorSetLayoutBinding bindings[SF_GRAPHICS_MAX_DESCRIPTOR_COUNT]	  = {0};
		for (uint32_t i = 0; i < descriptorSet->descriptorCount; ++i) {
			SfGraphicsDescriptor *descriptor = &descriptorSet->descriptors[i];

			pool_sizes[i].type	      = sfAsVulkanDescriptorType(descriptor->type);
			pool_sizes[i].descriptorCount = descriptor_count_by_type[descriptor->type];

			bindings[i].binding	       = descriptor->binding;
			bindings[i].descriptorType     = sfAsVulkanDescriptorType(descriptor->type);
			bindings[i].descriptorCount    = descriptor->entryCount;
			bindings[i].stageFlags	       = sfAsVulkanShaderStages(descriptor->stageFlags);
			bindings[i].pImmutableSamplers = NULL;
		}

		VkDescriptorSetLayoutCreateInfo const layoutInfo = {
		    .sType	  = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		    .pNext	  = NULL,
		    .flags	  = 0,
		    .bindingCount = descriptorSet->descriptorCount,
		    .pBindings	  = bindings};
		if (SF_VULKAN_CHECK(vkCreateDescriptorSetLayout(renderer->vulkanDevice, &layoutInfo,
								renderer->vulkanAllocationCallbacks,
								&descriptorSet->vulkanDescriptorSetLayout))) {
			VkDescriptorPoolCreateInfo const descriptorPoolInfo = {
			    .sType	   = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			    .pNext	   = NULL,
			    .flags	   = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
			    .maxSets	   = 1,
			    .poolSizeCount = descriptorSet->descriptorCount,
			    .pPoolSizes	   = pool_sizes};
			if (SF_VULKAN_CHECK(vkCreateDescriptorPool(renderer->vulkanDevice, &descriptorPoolInfo,
								   renderer->vulkanAllocationCallbacks,
								   &descriptorSet->vulkanDescriptorPool))) {
				VkDescriptorSetAllocateInfo const descriptorSetInfo = {
				    .sType		= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
				    .pNext		= NULL,
				    .descriptorPool	= descriptorSet->vulkanDescriptorPool,
				    .descriptorSetCount = 1,
				    .pSetLayouts	= &descriptorSet->vulkanDescriptorSetLayout};
				SF_VULKAN_CHECK(vkAllocateDescriptorSets(
				    renderer->vulkanDevice, &descriptorSetInfo, &descriptorSet->vulkanDescriptorSet));
			}
		}

		if (!descriptorSet->vulkanDescriptorSetLayout || !descriptorSet->vulkanDescriptorPool ||
		    !descriptorSet->vulkanDescriptorSet) {
			sfDestroyGraphicsDescriptorSet(renderer, SF_AS_HANDLE(descriptorSet));
			descriptorSet = SF_NULL_HANDLE;
		}
	}

	return SF_AS_HANDLE(descriptorSet);
}

SF_INTERNAL void sfDefaultInitGraphicsPipeline(SfGraphicsPipeline *pipeline) {
	SF_QUEUE_INIT(&pipeline->queue);
	pipeline->type		       = SF_GRAPHICS_PIPELINE_TYPE_GRAPHICS;
	pipeline->vulkanPipelineLayout = VK_NULL_HANDLE;
	pipeline->vulkanPipeline       = VK_NULL_HANDLE;
}

SF_INTERNAL VkShaderModule sfGetCorrespondingGraphicsShaderModule(SfGraphicsProgram const   *program,
								  SfGraphicsShaderStageFlags stage) {
	switch (stage) {
		case SF_GRAPHICS_SHADER_STAGE_VERTEX: return program->vulkanVertexShader;
		case SF_GRAPHICS_SHADER_STAGE_TESSELLATION_CONTROL: return program->vulkanTessellationControlShader;
		case SF_GRAPHICS_SHADER_STAGE_TESSELLATION_EVALUATION:
			return program->vulkanTessellationEvaluationShader;
		case SF_GRAPHICS_SHADER_STAGE_GEOMETRY: return program->vulkanGeometryShader;
		case SF_GRAPHICS_SHADER_STAGE_FRAGMENT: return program->vulkanFragmentShader;
		case SF_GRAPHICS_SHADER_STAGE_COMPUTE: return program->vulkanComputeShader;
		default: return VK_NULL_HANDLE;
	}
}

SF_INTERNAL SfGraphicsPipeline *sfGetOrAllocateGraphicsPipeline(SfGraphicsRenderer *renderer) {
	SfGraphicsPipeline *result = NULL;

	if (SF_QUEUE_IS_EMPTY(&renderer->freePipelineQueue)) {
		result = sfAllocate(&renderer->arena, sizeof(*result));
	} else {
		SfQueue *queue = SF_QUEUE_HEAD(&renderer->freePipelineQueue);
		SF_QUEUE_REMOVE(queue);
		result = SF_QUEUE_DATA(queue, SfGraphicsPipeline, queue);
	}

	if (result) {
		sfDefaultInitGraphicsPipeline(result);
		SF_QUEUE_INSERT_HEAD(&renderer->pipelineQueue, &result->queue);
	}

	return result;
}

typedef struct SfVulkanShaderStageList {
	uint32_t count;
	VkPipelineShaderStageCreateInfo *data
} SfVulkanShaderStageList;

SF_INTERNAL SfVulkanShaderStageList sfCreateVulkanShaderStageList(SfArena			      *arena,
								  SfGraphicsPipelineDescription const *description) {
	SfVulkanShaderStageList result = {0};
	result.data		       = sfAllocate(arena, 5 * sizeof(*result.data));
	if (result.data) {
		uint32_t stage_count = 0;
		for (uint32_t i = 0; i < 5; ++i) {
			SfGraphicsShaderStageFlags const stage	 = (1 << i);
			SfGraphicsProgram const		*program = (SfGraphicsProgram const *)description->program;
			if (stage & program->stageFlags) {
				result.data[stage_count].sType	= VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT;
				result.data[stage_count].pNext	= NULL;
				result.data[stage_count].flags	= 0;
				result.data[stage_count].stage	= sfAsVulkanShaderStages(stage);
				result.data[stage_count].module = sfGetCorrespondingGraphicsShaderModule(
				    program, stage);
				result.data[stage_count].pName		     = "main";
				result.data[stage_count].pSpecializationInfo = NULL;
				++stage_count;
			}
		}
		result.count = stage_count;
	}
	return result;
}

SF_INTERNAL uint64_t sfGetGraphicsFormatStride(SfGraphicsFormat format) {
	switch (format) {
		case SF_GRAPHICS_FORMAT_R8_UNORM: return 1;
		case SF_GRAPHICS_FORMAT_R16_UNORM: return 2;
		case SF_GRAPHICS_FORMAT_R16_UINT: return 2;
		case SF_GRAPHICS_FORMAT_R16_SFLOAT: return 2;
		case SF_GRAPHICS_FORMAT_R32_UINT: return 4;
		case SF_GRAPHICS_FORMAT_R32_SFLOAT: return 4;
		case SF_GRAPHICS_FORMAT_R8G8_UNORM: return 2;
		case SF_GRAPHICS_FORMAT_R16G16_UNORM: return 4;
		case SF_GRAPHICS_FORMAT_R16G16_SFLOAT: return 4;
		case SF_GRAPHICS_FORMAT_R32G32_UINT: return 8;
		case SF_GRAPHICS_FORMAT_R32G32_SFLOAT: return 8;
		case SF_GRAPHICS_FORMAT_R8G8B8_UNORM: return 3;
		case SF_GRAPHICS_FORMAT_R16G16B16_UNORM: return 6;
		case SF_GRAPHICS_FORMAT_R16G16B16_SFLOAT: return 6;
		case SF_GRAPHICS_FORMAT_R32G32B32_UINT: return 12;
		case SF_GRAPHICS_FORMAT_R32G32B32_SFLOAT: return 12;
		case SF_GRAPHICS_FORMAT_B8G8R8A8_UNORM: return 4;
		case SF_GRAPHICS_FORMAT_R8G8B8A8_UNORM: return 4;
		case SF_GRAPHICS_FORMAT_R16G16B16A16_UNORM: return 8;
		case SF_GRAPHICS_FORMAT_R16G16B16A16_SFLOAT: return 8;
		case SF_GRAPHICS_FORMAT_R32G32B32A32_UINT: return 16;
		case SF_GRAPHICS_FORMAT_R32G32B32A32_SFLOAT: return 16;
		case SF_GRAPHICS_FORMAT_D16_UNORM: return 0;
		case SF_GRAPHICS_FORMAT_X8_D24_UNORM_PACK32: return 0;
		case SF_GRAPHICS_FORMAT_D32_SFLOAT: return 0;
		case SF_GRAPHICS_FORMAT_S8_UINT: return 0;
		case SF_GRAPHICS_FORMAT_D16_UNORM_S8_UINT: return 0;
		case SF_GRAPHICS_FORMAT_D24_UNORM_S8_UINT: return 0;
		case SF_GRAPHICS_FORMAT_D32_SFLOAT_S8_UINT: return 0;
	}
}

typedef struct SfVulkanVertexInputList {
	uint32_t			   bindingCount;
	VkVertexInputBindingDescription	  *bindings;

	uint32_t			   attributeCount;
	VkVertexInputAttributeDescription *attributes;
} SfVulkanVertexInputList;

SF_INTERNAL SfVulkanVertexInputList sfCreateVulkanVertexInputList(SfArena			      *arena,
								  SfGraphicsPipelineDescription const *description) {
	SfVulkanVertexInputList result = {0};
	if (description->vertexLayout.count < SF_GRAPHICS_MAX_VERTEX_ATTRIBUTE_COUNT) {
		result.bindings	  = sfAllocate(arena, SF_GRAPHICS_MAX_VERTEX_BINDING_COUNT * sizeof(*result.bindings));
		result.attributes = sfAllocate(
		    arena, SF_GRAPHICS_MAX_VERTEX_ATTRIBUTE_COUNT * sizeof(*result.attributes));
		if (result.bindings && result.attributes) {
			uint32_t binding = (uint32_t)-1;
			for (uint32_t i = 0; i < description->vertexLayout.count; ++i) {
				SfGraphicsVertexAttribute *attribute = &description->vertexLayout.attributes[i];

				if (attribute->binding != binding) {
					binding = attribute->binding;
					++result.bindingCount;
				}

				if (result.bindingCount &&
				    result.bindingCount < SF_GRAPHICS_MAX_VERTEX_BINDING_COUNT) {
					result.bindings[result.bindingCount - 1].binding = attribute->binding;
					result.bindings[result.bindingCount - 1].inputRate =
					    VK_VERTEX_INPUT_RATE_VERTEX;
					result.bindings[result.bindingCount - 1].stride = sfGetGraphicsFormatStride(
					    attribute->format);

					result.attributes[i].location = attribute->location;
					result.attributes[i].binding  = attribute->binding;
					result.attributes[i].format   = sfAsVulkanFormat(attribute->format);
					result.attributes[i].offset   = attribute->offset;
				}
			}
			result.attributeCount = description->vertexLayout.count;
		}
	}
	return result;
}

SF_INTERNAL VkPrimitiveTopology sfAsVulkanPrimitiveTopology(SfGraphicsPrimitiveTopology topology) {
	switch (topology) {
		case SF_GRAPHICS_PRIMITIVE_TOPOLOGY_POINT_LIST: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case SF_GRAPHICS_PRIMITIVE_TOPOLOGY_LINE_LIST: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case SF_GRAPHICS_PRIMITIVE_TOPOLOGY_LINE_STRIP: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		case SF_GRAPHICS_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case SF_GRAPHICS_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		case SF_GRAPHICS_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		case SF_GRAPHICS_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
		case SF_GRAPHICS_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
			return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
		case SF_GRAPHICS_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
		case SF_GRAPHICS_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
		case SF_GRAPHICS_PRIMITIVE_TOPOLOGY_PATCH_LIST: return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
		default: return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
	}
}

SF_INTERNAL VkCullModeFlags sfAsVulkanCullMode(SfGraphicsCullMode cullMode) {
	switch (cullMode) {
		case SF_GRAPHICS_CULL_MODE_NONE: return VK_CULL_MODE_NONE;
		case SF_GRAPHICS_CULL_MODE_FRONT: return VK_CULL_MODE_FRONT_BIT;
		case SF_GRAPHICS_CULL_MODE_BACK: return VK_CULL_MODE_BACK_BIT;
		case SF_GRAPHICS_CULL_MODE_FRONT_AND_BACK: return VK_CULL_MODE_FRONT_AND_BACK;
		default: return VK_CULL_MODE_NONE;
	}
}

SF_INTERNAL VkFrontFace sfAsVulkanFrontFace(SfGraphicsFrontFace frontFace) {
	switch (frontFace) {
		case SF_GRAPHICS_FRONT_FACE_CLOCKWISE: return VK_FRONT_FACE_CLOCKWISE;
		case SF_GRAPHICS_FRONT_FACE_COUNTER_CLOCKWISE: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
		default: return VK_FRONT_FACE_CLOCKWISE;
	}
}

SfHandle sf_graphics_create_pipeline(SfGraphicsRenderer *renderer, SfGraphicsPipelineDescription const *description) {
	SfGraphicsPipeline *pipeline = sfGetOrAllocateGraphicsPipeline(renderer);
	if (pipeline) {
		// TODO(samuel): Support compute pipelines
		pipeline->type = SF_GRAPHICS_PIPELINE_TYPE_GRAPHICS;

		SfGraphicsDescriptorSet const *descriptorSet = (SfGraphicsDescriptorSet const *)
								   description->descriptorSet;
		VkPipelineLayoutCreateInfo const layout_info = {
		    .sType		    = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		    .pNext		    = NULL,
		    .flags		    = 0,
		    .setLayoutCount	    = descriptorSet ? 1 : 0,
		    .pSetLayouts	    = descriptorSet ? (&descriptorSet->vulkanDescriptorSetLayout) : NULL,
		    .pushConstantRangeCount = 0,
		    .pPushConstantRanges    = NULL,
		};
		if (SF_VULKAN_CHECK(vkCreatePipelineLayout(renderer->vulkanDevice, &layout_info,
							   renderer->vulkanAllocationCallbacks,
							   &pipeline->vulkanPipelineLayout))) {
			SfVulkanShaderStageList const stages = sfCreateVulkanShaderStageList(
			    &renderer->arena, description);
			SfVulkanVertexInputList const input = sfCreateVulkanVertexInputList(
			    &renderer->arena, description);
			SfGraphicsRenderTarget const *renderTarget = (SfGraphicsRenderTarget const *)
									 description->renderTarget;
			if (stages.count && stages.data && input.bindingCount && input.bindings &&
			    input.attributeCount && input.attributes) {
				VkPipelineVertexInputStateCreateInfo const vertexInputInfo = {
				    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
				    .pNext = NULL,
				    .flags = 0,
				    .vertexBindingDescriptionCount   = input.bindingCount,
				    .pVertexBindingDescriptions	     = input.bindings,
				    .vertexAttributeDescriptionCount = input.attributeCount,
				    .pVertexAttributeDescriptions    = input.attributes};
				VkPipelineInputAssemblyStateCreateInfo const inputAssemblyInfo = {
				    .sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
				    .pNext    = NULL,
				    .flags    = 0,
				    .topology = sfAsVulkanPrimitiveTopology(description->topology),
				    .primitiveRestartEnable = VK_FALSE};
				VkPipelineTessellationDomainOriginStateCreateInfoKHR const
				    tessellationDomainOriginInfo = {
					.sType =
					    VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO_KHR,
					.pNext	      = NULL,
					.domainOrigin = VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT_KHR};
				VkPipelineTessellationStateCreateInfo const tessellationInfo = {
				    .sType		= VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
				    .pNext		= NULL,
				    .flags		= 0,
				    .patchControlPoints = description->patchControlPointCount};
				VkPipelineViewportStateCreateInfo const viewportInfo = {
				    .sType	   = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
				    .pNext	   = NULL,
				    .flags	   = 0,
				    .viewportCount = 1,
				    .pViewports	   = NULL,
				    .scissorCount  = 1,
				    .pScissors	   = NULL};
				VkPipelineRasterizationStateCreateInfo const rasterizationInfo = {
				    .sType	      = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
				    .pNext	      = NULL,
				    .flags	      = 0,
				    .depthClampEnable = VK_FALSE,
				    .rasterizerDiscardEnable = VK_FALSE,
				    .polygonMode	     = VK_POLYGON_MODE_FILL,
				    .cullMode		     = sfAsVulkanCullMode(description->cullMode),
				    .frontFace		     = sfAsVulkanFrontFace(description->frontFace),
				    .depthBiasEnable	     = VK_FALSE,
				    .depthBiasConstantFactor = 0.0F,
				    .depthBiasClamp	     = 0.0F,
				    .depthBiasSlopeFactor    = 0.0F,
				    .lineWidth		     = 1.0F};
				VkPipelineMultisampleStateCreateInfo const multisampleInfo = {
				    .sType		   = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
				    .pNext		   = NULL,
				    .flags		   = 0,
				    .rasterizationSamples  = sfAsVulkanSampleCount(renderTarget->samples),
				    .sampleShadingEnable   = VK_FALSE,
				    .minSampleShading	   = 1.0F,
				    .pSampleMask	   = NULL,
				    .alphaToCoverageEnable = VK_FALSE,
				    .alphaToOneEnable	   = VK_FALSE};
				VkPipelineDepthStencilStateCreateInfo const depthStencilInfo = {
				    .sType	      = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
				    .pNext	      = NULL,
				    .flags	      = 0,
				    .depthTestEnable  = description->useDepthStencil,
				    .depthWriteEnable = description->useDepthStencil,
				    .depthCompareOp   = VK_COMPARE_OP_LESS,
				    .depthBoundsTestEnable = VK_FALSE,
				    .stencilTestEnable	   = VK_FALSE,
				    .front		   = {0},
				    .back		   = {0},
				    .minDepthBounds	   = 0.0F,
				    .maxDepthBounds	   = 1.0F};
				VkPipelineColorBlendAttachmentState const colorBlendAttachment = {
				    .blendEnable	 = VK_FALSE,
				    .srcColorBlendFactor = VK_BLEND_FACTOR_ZERO,
				    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
				    .colorBlendOp	 = VK_BLEND_OP_ADD,
				    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
				    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
				    .alphaBlendOp	 = VK_BLEND_OP_ADD,
				    .colorWriteMask	 = 0 | VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
						      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};
				VkPipelineColorBlendStateCreateInfo const colorBlendStateInfo = {
				    .sType	     = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
				    .pNext	     = NULL,
				    .flags	     = 0,
				    .logicOpEnable   = VK_FALSE,
				    .logicOp	     = VK_LOGIC_OP_COPY,
				    .attachmentCount = 1,
				    .pAttachments    = &colorBlendAttachment,
				    .blendConstants  = {0.0F, 0.0F, 0.0F, 0.0F}};
				VkDynamicState const dynamicStates[] = {
				    VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
				VkPipelineDynamicStateCreateInfo const dynamicStateInfo = {
				    .sType	       = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
				    .pNext	       = NULL,
				    .flags	       = 0,
				    .dynamicStateCount = SF_ARRAY_SIZE(dynamicStates),
				    .pDynamicStates    = dynamicStates};
				VkGraphicsPipelineCreateInfo const pipelineInfo = {
				    .sType		 = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				    .pNext		 = NULL,
				    .flags		 = 0,
				    .stageCount		 = stages.count,
				    .pStages		 = stages.data,
				    .pVertexInputState	 = &vertexInputInfo,
				    .pInputAssemblyState = &inputAssemblyInfo,
				    .pTessellationState	 = (description->topology ==
							    SF_GRAPHICS_PRIMITIVE_TOPOLOGY_PATCH_LIST)
							       ? &tessellationInfo
							       : NULL,
				    .pViewportState	 = &viewportInfo,
				    .pRasterizationState = &rasterizationInfo,
				    .pMultisampleState	 = &multisampleInfo,
				    .pDepthStencilState	 = description->useDepthStencil ? &depthStencilInfo : NULL,
				    .pColorBlendState	 = &colorBlendStateInfo,
				    .pDynamicState	 = &dynamicStateInfo,
				    .layout		 = pipeline->vulkanPipelineLayout,
				    .renderPass		 = renderTarget->vulkanRenderPass,
				    .subpass		 = 0,
				    .basePipelineHandle	 = VK_NULL_HANDLE,
				    .basePipelineIndex	 = -1};
				SF_VULKAN_CHECK(vkCreateGraphicsPipelines(
				    renderer->vulkanDevice, VK_NULL_HANDLE, 1, &pipelineInfo,
				    renderer->vulkanAllocationCallbacks, &pipeline->vulkanPipeline));
			}
		}
	}

	if (!pipeline->vulkanPipelineLayout || !pipeline->vulkanPipeline) {
		sfDestroyGraphicsPipeline(renderer, SF_AS_HANDLE(pipeline));
		pipeline = SF_NULL_HANDLE;
	}

	return pipeline;
}

SfHandle sfGetGraphicsQueueHandle(SfGraphicsRenderer *renderer) { return SF_AS_HANDLE(&renderer->graphicsQueue); }

SfHandle sfGetPresentQueueHandle(SfGraphicsRenderer *renderer) { return SF_AS_HANDLE(&renderer->presentQueue); }

SF_INTERNAL SfBool sfTestVulkanFormatFeatures(SfGraphicsRenderer *renderer, VkFormat format, VkImageTiling tiling,
					      VkFormatFeatureFlags features) {
	SfBool result = SF_FALSE;
	if (renderer) {
		VkFormatProperties properties = {0};
		vkGetPhysicalDeviceFormatProperties(renderer->vulkanPhysicalDevice, format, &properties);

		if (VK_IMAGE_TILING_LINEAR == tiling)
			result = (properties.linearTilingFeatures & features) == features;

		if (VK_IMAGE_TILING_OPTIMAL == tiling)
			result = (properties.optimalTilingFeatures & features) == features;
	}
	return result;
}

SfGraphicsRenderer *sfCreateGraphicsRenderer(SfGraphicsRendererDescription const *description) {
	SfGraphicsRenderer *renderer = sfAllocate(description->arena, sizeof(SfGraphicsRenderer));
	if (renderer) {
		SF_QUEUE_INIT(&renderer->textureQueue);
		SF_QUEUE_INIT(&renderer->freeTextureQueue);

		SF_QUEUE_INIT(&renderer->bufferQueue);
		SF_QUEUE_INIT(&renderer->freeBufferQueue);

		SF_QUEUE_INIT(&renderer->commandPoolQueue);
		SF_QUEUE_INIT(&renderer->freeCommandPoolQueue);

		SF_QUEUE_INIT(&renderer->commandBufferQueue);
		SF_QUEUE_INIT(&renderer->freeCommandBufferQueue);

		SF_QUEUE_INIT(&renderer->semaphoreQueue);
		SF_QUEUE_INIT(&renderer->freeSemaphoreQueue);

		SF_QUEUE_INIT(&renderer->fenceQueue);
		SF_QUEUE_INIT(&renderer->freeFenceQueue);

		SF_QUEUE_INIT(&renderer->renderTargetQueue);
		SF_QUEUE_INIT(&renderer->freeRenderTargetQueue);

		SF_QUEUE_INIT(&renderer->programQueue);
		SF_QUEUE_INIT(&renderer->freeProgramQueue);

		SF_QUEUE_INIT(&renderer->descriptorSetQueue);
		SF_QUEUE_INIT(&renderer->freeDescriptorSetQueue);

		SF_QUEUE_INIT(&renderer->errorQueue);

		SF_QUEUE_INIT(&renderer->pipelineQueue);
		SF_QUEUE_INIT(&renderer->freePipelineQueue);

		renderer->plataform			  = description->plataform;
		renderer->swapchainWidth		  = description->swapchainWidth;
		renderer->swapchainHeight		  = description->swapchainHeight;
		renderer->swapchainImageCount		  = description->swapchainImageCount;
		renderer->swapchainColorClearValue	  = description->swapchainColorClearValue;
		renderer->swapchainDepthStencilClearValue = description->swapchainDepthStencilClearValue;
		renderer->samples			  = description->sampleCount;
		renderer->colorAttachmentFormat		  = description->colorAttachmentFormat;
		renderer->depthStencilFormat		  = description->depthStencilFormat;
		renderer->bufferingCount		  = description->bufferingCount;
		renderer->enableVSYNC			  = description->enableVSYNC;

		renderer->applicationName = description->applicationName;

		renderer->vulkanInstanceLayerCount = description->vulkanInstanceLayerCount;
		renderer->vulkanInstanceLayers	   = description->vulkanInstanceLayers;

		renderer->vulkanInstanceExtensionCount = description->vulkanInstanceExtensionCount;
		renderer->vulkanInstanceExtensions     = description->vulkanInstanceExtensions;

		renderer->vulkanDeviceExtensionCount = description->vulkanInstanceExtensionCount;
		renderer->vulkanDeviceExtensions     = description->vulkanDeviceExtensions;

		renderer->vulkanAllocationCallbacks = description->vulkanAllocationCallbacks;
		renderer->vulkanDebugCallback	    = description->vulkanDebugCallback;

		renderer->arena = sfScratch(&description->arena, 1024 * 512);
		if (renderer->arena.data) {
			VkApplicationInfo const	   applicationInfo = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
								      .pNext = NULL,
								      .pApplicationName	  = renderer->applicationName,
								      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
								      .pEngineName	  = NULL,
								      .engineVersion	  = VK_MAKE_VERSION(1, 0, 0),
								      .apiVersion	  = VK_MAKE_VERSION(1, 0, 0)};

			VkInstanceCreateInfo const instanceInfo = {
			    .sType		     = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			    .pNext		     = NULL,
			    .flags		     = 0 | ((renderer->plataform.type == SF_PLATAFORM_TYPE_MACOS)
								? VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
								: 0),
			    .pApplicationInfo	     = &applicationInfo,
			    .enabledLayerCount	     = renderer->vulkanInstanceLayerCount,
			    .ppEnabledLayerNames     = renderer->vulkanInstanceLayers,
			    .enabledExtensionCount   = renderer->vulkanInstanceExtensionCount,
			    .ppEnabledExtensionNames = renderer->vulkanInstanceExtensions};

			if (SF_VULKAN_CHECK(vkCreateInstance(
				&instanceInfo, renderer->vulkanAllocationCallbacks, &renderer->vulkanInstance))) {
				renderer->vulkanCreateDebugUtilsMessengerEXT = SF_VULKAN_PROC(
				    vkCreateDebugUtilsMessengerEXT, renderer->vulkanInstance);
				renderer->vulkanDestroyDebugUtilsMessengerEXT = SF_VULKAN_PROC(
				    vkDestroyDebugUtilsMessengerEXT, renderer->vulkanInstance);

				if (renderer->vulkanCreateDebugUtilsMessengerEXT &&
				    renderer->vulkanDestroyDebugUtilsMessengerEXT) {
					VkDebugUtilsMessengerCreateInfoEXT const validatorInfo = {
					    .sType	     = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
					    .pNext	     = NULL,
					    .flags	     = 0,
					    .messageSeverity = 0 | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
							       VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
							       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
							       VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
					    .messageType = 0 | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
							   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
							   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
					    .pfnUserCallback = renderer->vulkanDebugCallback,
					    .pUserData	     = NULL};

					PFN_vkCreateDebugUtilsMessengerEXT const create =
					    renderer->vulkanCreateDebugUtilsMessengerEXT;
					SF_VULKAN_CHECK(create(renderer->vulkanInstance, &validatorInfo,
							       renderer->vulkanAllocationCallbacks,
							       &renderer->vulkanValidationMessenger));
				}

				if (SF_VULKAN_CHECK(glfwCreateWindowSurface(
					renderer->vulkanInstance, renderer->plataform.window,
					renderer->vulkanAllocationCallbacks, &renderer->vulkanSurface))) {
					SfVulkanPhysicalDeviceList deviceList = sfVulkanCreatePhysicalDeviceList(
					    &renderer->arena, renderer);

					for (uint32_t i = 0; i < deviceList.count && !renderer->vulkanPhysicalDevice;
					     ++i) {
						renderer->vulkanPhysicalDevice = deviceList.data[i];
						if (!sfVulkanCheckDeviceSupport(renderer))
							renderer->vulkanPhysicalDevice = VK_NULL_HANDLE;
					}
				}

				if (renderer->vulkanSurface && renderer->vulkanPhysicalDevice) {
					SfVulkanSurfaceFormatList surfaceFormatList = sfVulkanCreateSurfaceFormatList(
					    &renderer->arena, renderer);
					if (surfaceFormatList.count && surfaceFormatList.data) {

						renderer->vulkanSurfaceFormat = surfaceFormatList.data[0].format;
						renderer->vulkanSurfaceColorSpace =
						    surfaceFormatList.data[0].colorSpace;

						for (uint32_t i = 0; i < surfaceFormatList.count; ++i) {
							VkSurfaceFormatKHR *format = &surfaceFormatList.data[i];

							if (format->format == VK_FORMAT_B8G8R8A8_SRGB ||
							    format->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
								renderer->vulkanSurfaceFormat =
								    VK_FORMAT_B8G8R8A8_SRGB;
								renderer->vulkanSurfaceColorSpace =
								    VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
								break;
							}
						}
					}
				}

				if (renderer->vulkanSurface && renderer->vulkanPhysicalDevice) {
					SfVulkanPresentModeList presentModeList = sfVulkanCreatePresentModeList(
					    &renderer->arena, renderer);
					if (presentModeList.count && presentModeList.data) {
						SfBool foundPresentMode = SF_FALSE;

						if (renderer->enableVSYNC)
							renderer->vulkanPresentMode = VK_PRESENT_MODE_FIFO_KHR;
						else
							renderer->vulkanPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;

						for (uint32_t i = 0; i < presentModeList.count && !foundPresentMode;
						     ++i) {
							if (presentModeList.data[i] == renderer->vulkanPresentMode) {
								foundPresentMode = SF_TRUE;
							}
						}

						if (!foundPresentMode) {
							renderer->enableVSYNC	    = SF_TRUE;
							renderer->vulkanPresentMode = VK_PRESENT_MODE_FIFO_KHR;
						}
					}
				}

				if (renderer->vulkanSurface && renderer->vulkanPhysicalDevice) {
					if (sf_vulkan_test_format_features(
						renderer, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
						VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
						renderer->vulkanDepthStencilFormat = VK_FORMAT_D32_SFLOAT;
					else if (sf_vulkan_test_format_features(
						     renderer, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL,
						     VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
						renderer->vulkanDepthStencilFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
					else if (sf_vulkan_test_format_features(
						     renderer, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL,
						     VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
						renderer->vulkanDepthStencilFormat = VK_FORMAT_D24_UNORM_S8_UINT;
					SF_ASSERT(0);
					renderer->vulkanSamples = VK_SAMPLE_COUNT_2_BIT;
				}

				if (renderer->vulkanPhysicalDevice) {
					float const		      priority = 1.0F;

					VkDeviceQueueCreateInfo const queue_info[2] = {
					    {.sType	       = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					     .pNext	       = NULL,
					     .flags	       = 0,
					     .queueFamilyIndex = renderer->graphicsQueue.vulkanQueueFamilyIndex,
					     .queueCount       = 1,
					     .pQueuePriorities = &priority},
					    {.sType	       = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					     .pNext	       = NULL,
					     .flags	       = 0,
					     .queueFamilyIndex = renderer->presentQueue.vulkanQueueFamilyIndex,
					     .queueCount       = 1,
					     .pQueuePriorities = &priority}};

					VkPhysicalDeviceFeatures features = {0};
					vkGetPhysicalDeviceFeatures(renderer->vulkanPhysicalDevice, &features);

					VkDeviceCreateInfo const device_info = {
					    .sType		   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
					    .pNext		   = NULL,
					    .flags		   = 0,
					    .queueCreateInfoCount  = (renderer->graphicsQueue.vulkanQueueFamilyIndex ==
								      renderer->presentQueue.vulkanQueueFamilyIndex)
									 ? 1
									 : SF_SIZE(queue_info),
					    .pQueueCreateInfos	   = queue_info,
					    .enabledLayerCount	   = 0,
					    .ppEnabledLayerNames   = NULL,
					    .enabledExtensionCount = renderer->vulkanDeviceExtensionCount,
					    .ppEnabledExtensionNames = renderer->vulkanDeviceExtensions,
					    .pEnabledFeatures	     = &features};

					if (SF_VULKAN_CHECK(vkCreateDevice(
						renderer->vulkanPhysicalDevice, &device_info,
						renderer->vulkanAllocationCallbacks, &renderer->vulkanDevice))) {
						vkGetDeviceQueue(renderer->vulkanDevice,
								 renderer->graphicsQueue.vulkanQueueFamilyIndex, 0,
								 &renderer->graphicsQueue.vulkanQueue);
						vkGetDeviceQueue(renderer->vulkanDevice,
								 renderer->presentQueue.vulkanQueueFamilyIndex, 0,
								 &renderer->presentQueue.vulkanQueue);
					}
				}

				renderer->imageAcquiredSemaphoreCount = renderer->bufferingCount;
				for (uint32_t i = 0; i < renderer->bufferingCount; ++i) {
					renderer->imageAcquiredSemaphores[i] = sfCreateGraphicsSemaphore(renderer);
				}

				renderer->inFlightFenceCount = renderer->bufferingCount;
				for (uint32_t i = 0; i < renderer->bufferingCount; ++i) {
					renderer->inFlightFences[i] = sfCreateGraphicsFence(renderer);
				}
			}
		}
	}

	return renderer;
}

void sfDestroyGraphicsRenderer(SfGraphicsRenderer *renderer) {
	if (renderer) {
		sfGraphicsDeviceWaitIdle(renderer);

		for (uint32_t i = 0; i < SF_SIZE(renderer->inFlightFences); ++i) {
			sfDestroyGraphicsFence(renderer, renderer->inFlightFences[i]);
			renderer->inFlightFences[i] = SF_NULL_HANDLE;
		}
		renderer->inFlightFenceCount = 0;

		for (uint32_t i = 0; i < SF_SIZE(renderer->imageAcquiredSemaphores); ++i) {
			sfDestroyGraphicsSemaphore(renderer, renderer->imageAcquiredSemaphores[i]);
			renderer->imageAcquiredSemaphores[i] = SF_NULL_HANDLE;
		}
		renderer->imageAcquiredSemaphoreCount = 0;

		sfDestroySwapchainResources(renderer);

		if (renderer->vulkanDevice) {
			vkDestroyDevice(renderer->vulkanDevice, renderer->vulkanAllocationCallbacks);
			renderer->vulkanDevice = VK_NULL_HANDLE;
		}

		if (renderer->vulkanInstance) {
			if (renderer->vulkanSurface) {
				vkDestroySurfaceKHR(renderer->vulkanInstance, renderer->vulkanSurface,
						    renderer->vulkanAllocationCallbacks);
				renderer->vulkanSurface = VK_NULL_HANDLE;
			}
			if (renderer->vulkanDestroyDebugUtilsMessengerEXT && renderer->vulkanValidationMessenger) {
				renderer->vulkanDestroyDebugUtilsMessengerEXT(renderer->vulkanInstance,
									      renderer->vulkanValidationMessenger,
									      renderer->vulkanAllocationCallbacks);
				renderer->vulkanValidationMessenger = VK_NULL_HANDLE;
			}

			vkDestroyInstance(renderer->vulkanInstance, renderer->vulkanAllocationCallbacks);
			renderer->vulkanInstance = VK_NULL_HANDLE;
		}

		sfDefaultInitGraphicsRenderer(renderer);
	}
}
