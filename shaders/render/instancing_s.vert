#version 450

// Vertex attributes
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
//layout (location = 3) in vec3 inColor;

// Instanced attributes
layout (location = 3) in vec2  instancePos;
layout (location = 4) in vec2  instanceRot;
layout (location = 5) in float instanceScale;
layout (location = 6) in int   instanceTexIndex;

layout (binding = 0) uniform UBO
{
	mat4 projection;
	mat4 modelview;
	vec4 lightPos;

	vec2 worldSize;
	vec2 mapDivider;

	float locSpeed;
	float globSpeed;
} ubo;

layout (location = 0) out int texIndex;


void main()
{

	int instId=gl_InstanceIndex;

	vec2 scaledPos=vec2(instanceScale*inPos.x,instanceScale*inPos.z);
	vec3 pos = vec3(2.0*(scaledPos.x+instancePos.x)/ubo.worldSize.x-1.0,2.0*(scaledPos.y+instancePos.y)/ubo.worldSize.y-1.0,0.0);

  gl_Position = vec4(pos, 1.0f);

	texIndex=instanceTexIndex;

}
