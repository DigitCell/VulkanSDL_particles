#version 450

layout (binding = 1) uniform sampler2D samplerArray;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inUV;
layout (location = 3) in vec3 inViewVec;
layout (location = 4) in vec3 inLightVec;

layout (location = 0) out vec4 outFragColor;

void main()
{
	float MAX_WIDTH = 1024.0;
  float MAX_HEIGHT =1024.0;

  float half_size=1024/32;

  float yy=half_size*int(inUV.z/32);
	float xx=half_size*float(int(inUV.z)%32);
	float LEFT = xx;
	float TOP =  yy;
	float WIDTH = half_size;
	float HEIGHT =half_size;

  vec2 TexCoord;
	TexCoord.x = inUV.x * (WIDTH/MAX_WIDTH)+(LEFT/MAX_WIDTH);
	TexCoord.y = inUV.y * (HEIGHT/MAX_HEIGHT)+(TOP/MAX_HEIGHT);
	//TexCoord.z=0.0;

	vec4 color = texture(samplerArray, TexCoord) * vec4(inColor, 1.0);
	vec3 N = normalize(inNormal);
	vec3 L = normalize(inLightVec);
	vec3 V = normalize(inViewVec);
	vec3 R = reflect(-L, N);

	vec3 diffuse = max(dot(N, L), 0.1) * inColor;
	vec3 specular = (dot(N,L) > 0.0) ? pow(max(dot(R, V), 0.0), 16.0) * vec3(0.75) * color.r : vec3(0.0);
	outFragColor = vec4(0.3,0.3,0.7, 1.0);
}
