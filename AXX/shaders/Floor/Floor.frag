#version 450#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;layout(location = 1) in vec3 fragNorm;layout(location = 2) in vec2 fragUV;// in teoria set 0 si può togliere perché è uguale ovunquelayout(set = 0, binding = 0) uniform UniformBufferObject {	float amb;	mat4 mvpMat;	mat4 mMat;	mat4 nMat;} ubo;layout(set = 0, binding = 1) uniform sampler2D tex;
layout(location = 0) out vec4 outColor;
void main() {
	outColor = vec4(ubo.amb * texture(tex, fragUV).rgb, 1.0f);	// output color}