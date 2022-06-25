#ifndef SUPPOERSTRUCTS_H
#define SUPPOERSTRUCTS_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include "Constants.hpp"

struct SimCommandParametrs
{
   bool runLoop=true;
   bool resetSim=false;
   bool addParticles=false;
   float zoomCoeffParticleDraw=1.6;
   float zoomCoeffConstraintDraw=1.0;
   bool drawParticles=true;
   bool drawConstraints=true;
   float lineWidth=1.7;

   bool readPolygons=false;
   bool offsetPolygons=false;
   bool triangulate=false;

   bool drawPolygons=false;
   bool drawTriangles=false;

   float offsetSize=3.5f;

   int numColors=16;

   bool saveImage=false;


};

struct SimFieldParametrs
{
   size_t width= SIM_WIDTH;
   size_t height=SIM_HEIGHT;

};

struct SimParametrs
{
    glm::ivec2 worldSize=glm::ivec2(SIM_WIDTH,SIM_HEIGHT);
    glm::ivec2 mapSize=  glm::ivec2(MAP_SIZE_X,MAP_SIZE_Y);
    glm::ivec2 mapCellSize;

    float radius=PARTICLE_RADIUS;

    int numParticles=   0;
    int maxNumParticles=NUM_PARTICLES_MAX;
    int maxMapCellSize= MAPCELL_MAX_PARTILCLES;
    int maxNeightbSize= NEIGHTB_MAX_PARTICLES;
};

struct SimDynamicParametrs
{
    glm::ivec2 worldSize=glm::ivec2(SIM_WIDTH,SIM_HEIGHT);
    float maxVelocity=1.0;
    float radius=1.0;
    float noiseCoeff=0.0;
    float velocityDamping=0.0;

    float stiffnessCoeff;
    float dampingCoeff;

    float constraintDist=21.0f;
    float initConstraintDistStart=25.0f;
    float initConstraintDistStop=25.0f;
    float breakConstraintDist=35.0f;

    float rateChangePhase=0;

    float colorDecrease=0.005;
    float dt=0.01;

    int tick;
    int constraintsNum;
    int polygonsNum;
    int numParticles;
    int max_constraints=CONSTRAINTS_MAX_PARTICLE;
    int removeOneEndConstraints=0;


   // int numCreatures;
  //  int max_Creatures=NUM_CREATURES_MAX;

};

struct VertexPosNormUv {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct VertexPos {
    glm::vec3 pos;
};


struct VertexPosNormUvColor {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec3 color;
};



struct InstanceData {
    glm::vec2 pos;
    glm::vec2 rot;
    float scale;
    int texIndex;
};

struct InstanceConstraintData {
    glm::vec2 pos1;
    glm::vec2 pos2;
    int texIndex;
};


struct TrianglesDrawStruct {
    glm::vec2 pos;
    glm::vec4 color;
};

// SSBO particle declaration
struct Particle {

    glm::vec2 pos;								// Particle position (xy present zw past)
    glm::vec2 vel;								// Particle velocity (xy present zw past)
    glm::vec2 force;

    glm::vec2 pos_old;								// Particle position (xy present zw past)
    glm::vec2 vel_old;								// Particle velocity (xy present zw past)
    glm::vec2 force_old;

    float radius;
    float energy;

    int constructSize;
    int constructSizeMax;
    int neightbSize;
    int creature_id;
};

struct Creature {

    glm::vec2 pos;								// Particle position (xy present zw past)
    glm::vec2 vel;								// Particle velocity (xy present zw past)
    glm::vec2 force;
    glm::vec2 coeff;
    glm::vec2 phase;
    glm::vec2 radius;

    int constructSize;
    int constructSizeMax;
    int neightbSize;
    int creature_id;
};

struct Constraint
{
    int cactive;
    int id1;
    int id2;
    float length_init;
};



#endif // SUPPOERSTRUCTS_H
