#version 450

// Vertex attributes
layout (location = 0) in vec2 inPos;
layout (location = 1) in vec4 inColor;

layout (binding = 0) uniform ConstraintUBO
{
	vec2 worldSize;
	vec2 mapDivider;
} constraintUBO;

layout (location = 0) out vec4 outColor;

void main()
{

  vec2 scaledPos=inPos;//vec2(inPos.x+constraintUBO.worldSize.x/64.0,inPos.y+constraintUBO.worldSize.y/64.0);
	vec3 pos = vec3(2.0*(scaledPos.x)/constraintUBO.worldSize.x-1.0,2.0*(scaledPos.y)/constraintUBO.worldSize.y-1.0,0.0);
	outColor=inColor;
	gl_Position = vec4(pos, 1.0f);

  //gl_Position = vec4(1.0*inPos.x/ubo.worldSize.x-0.75, 1.0*inPos.y/ubo.worldSize.y-0.75, 0.0, 1.0);

}
