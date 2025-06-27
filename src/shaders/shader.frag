#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 fragColor;

/*
Render Pass
Has an array of attachment descriptions.
Attachment at index 0 is what the shader output at location = 0 writes into.
*/

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = vec4(fragColor, 1.0);
}