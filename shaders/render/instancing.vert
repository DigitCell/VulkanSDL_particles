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

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec3 outUV;
layout (location = 3) out vec3 outViewVec;
layout (location = 4) out vec3 outLightVec;

void main()
{
	outColor =vec3(1.0,1.0,1.0);// inColor;
	int instId=gl_InstanceIndex;
	outUV = vec3(inUV, gl_InstanceIndex);

	mat3 mx, my, mz;
	float s ;
	float c ;
	// rotate around x
	s = sin(instanceRot.x);// + ubo.locSpeed);
	c = cos(instanceRot.x);// + ubo.locSpeed);

	mx[0] = vec3(c, s, 0.0);
	mx[1] = vec3(-s, c, 0.0);
	mx[2] = vec3(0.0, 0.0, 1.0);

	// rotate around y
	s = sin(0 + ubo.locSpeed);
	c = cos(0 + ubo.locSpeed);

	my[0] = vec3(c, 0.0, s);
	my[1] = vec3(0.0, 1.0, 0.0);
	my[2] = vec3(-s, 0.0, c);

	// rot around z
	s = sin(instanceRot.y);
	c = cos(instanceRot.y );

	mz[0] = vec3(1.0, 0.0, 0.0);
	mz[1] = vec3(0.0, c, s);
	mz[2] = vec3(0.0, -s, c);

	mat3 rotMat = mz * my * mx;

	mat4 gRotMat;
	s = sin(instanceRot.y + ubo.globSpeed);
	c = cos(instanceRot.y + ubo.globSpeed);
	gRotMat[0] = vec4(c, 0.0, s, 0.0);
	gRotMat[1] = vec4(0.0, 1.0, 0.0, 0.0);
	gRotMat[2] = vec4(-s, 0.0, c, 0.0);
	gRotMat[3] = vec4(0.0, 0.0, 0.0, 1.0);

	vec4 locPos = vec4(inPos.xyz * rotMat, 1.0);
	vec4 pos = vec4((locPos.xyz * instanceScale) + vec3(instancePos.x,0,instancePos.y), 1.0);

	gl_Position = ubo.projection * ubo.modelview * gRotMat * pos;
	outNormal = mat3(ubo.modelview * gRotMat) * inverse(rotMat) * inNormal;

	pos = ubo.modelview * vec4(inPos.xyz + vec3(instancePos.x,0,instancePos.y), 1.0);
	vec3 lPos = mat3(ubo.modelview) * ubo.lightPos.xyz;
	outLightVec = lPos - pos.xyz;
	outViewVec = -pos.xyz;
}
