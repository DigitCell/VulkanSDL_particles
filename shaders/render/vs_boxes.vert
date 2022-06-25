#version 460 core

layout(location=0) in vec3 pos;     /*  Absolute coordinates  */
layout(location=1) in int amountParticles;


layout (binding = 0) uniform UBO
{
	mat4 projection;
	mat4 modelview;
	vec4 lightPos;

  vec2 worldSize;
  vec2 mapSize;

	float locSpeed;
	float globSpeed;
} ubo;


layout (location = 0) out float numParticles;

void main()
{
    int instId=gl_InstanceIndex;
    vec2 worldSize2=ubo.worldSize*1.05;

    vec2 mapCellSize=vec2(ubo.worldSize.x/ubo.mapSize.x,ubo.worldSize.y/ubo.mapSize.y);
    float cubeX=mapCellSize.x*(pos.x+1.0) /int(worldSize2.x);
    float cubeY=mapCellSize.y*(pos.y+1.0) /int(worldSize2.y);
    vec2 scaled = vec2(cubeX, cubeY);//* (offset.z);
    vec2 offset;

    offset.x=mapCellSize.x*float(instId%int(ubo.mapSize.x));
    offset.y=mapCellSize.y*float(instId/int(ubo.mapSize.y));
    gl_Position = vec4(scaled.x +2.0*offset.x/ubo.worldSize.x-1.0, scaled.y +2.0*offset.y/ubo.worldSize.y-1.0, 0.0, 1.0);

     numParticles=amountParticles;
    //OUT.amountParticles=gl_InstanceID;

}
