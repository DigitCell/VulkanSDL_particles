#version 460 core

layout (location = 0) in float numParticles;

layout (location = 0) out vec4 outFragColor;

float rand(float a, float b)
{
    return fract(sin(a*12.9898 + b*78.233) * 43758.5453);
  //  float t = 12.9898*a + 78.233*b;
  //  return fract((123.0+t) * sin(t));
}

void main()
{

    float deltaColor=numParticles;
    outFragColor= vec4(0.1, 0.1+deltaColor*0.05, 0.1, 1.0);

  //  outFragColor= vec4(0.1, 0.7, 0.3, 1.0);
  //  color=  vec4(rand(t.x, t.y), rand(t.x, t.x), rand(t.x - t.y, t.x), 1.0);
}
