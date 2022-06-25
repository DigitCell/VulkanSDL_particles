#version 450

// Vertex attributes
layout (location = 0) in vec2 inPos;

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


void main()
{

  vec2 scaledPos=vec2(inPos.x,inPos.y);
	vec3 pos = vec3(2.0*(scaledPos.x)/ubo.worldSize.x-1.0,2.0*(scaledPos.y)/ubo.worldSize.y-1.0,0.0);
	gl_Position = vec4(pos, 1.0f);

  //gl_Position = vec4(1.0*inPos.x/ubo.worldSize.x-0.75, 1.0*inPos.y/ubo.worldSize.y-0.75, 0.0, 1.0);

}
