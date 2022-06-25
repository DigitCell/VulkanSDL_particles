#extension GL_EXT_scalar_block_layout : enable

//---COMPUTE BUFFERS
struct Particle {
		vec2 pos;								// Particle position (xy present zw past)
		vec2 vel;								// Particle velocity (xy present zw past)
		vec2 force;

		vec2 pos_old;								// Particle position (xy present zw past)
		vec2 vel_old;								// Particle velocity (xy present zw past)
		vec2 force_old;

		float radius;
		float energy;

    int constructSize;
    int constructSizeMax;
    int neightbSize;
    int creature_id;

};

struct Creature {
		vec2 pos;								// Particle position (xy present zw past)
		vec2 vel;								// Particle velocity (xy present zw past)
		vec2 force;
		vec2 coeff;
		vec2 phase;
		vec2 radius;

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


struct InstanceData {
    vec2 pos;
    vec2 rot;
    float scale;
    int  texIndex;
};

struct InstanceConstraintData {
    vec2 pos1;
    vec2 pos2;
    int texIndex;
};


struct SimParams
{
	ivec2 worldSize;
	ivec2 mapSize;
	ivec2 mapCellSize;

	float radius;

	int numParticle;
	int maxNumParticles;
	int maxMapCellSize;
	int maxNeightbSize;
};

struct SimDynamicParametrs
{
	  ivec2 worldSize;
    float maxVelocity;
    float radius;
    float noiseCoeff;
    float velocityDamping;

    float stiffnessCoeff;
    float dampingCoeff;

    float constraintDist;
		float initConstraintDistStart;
    float initConstraintDistStop;
    float breakConstraintDist;

		float rateChangePhase;

    float colorDecrease;
    float dt;
    int tick;
		int constraintsNum;
		int polygonsNum;
		int numParticles;
		int max_constraints;
		int removeOneEndConstraints;
	//	int numCreatures;
	//   int max_Creatures;

};


//layout (std430, binding = PointData_BP ) buffer PointDataBuffer{
//    PointData pointData[];
//};


//Iq palettes https://www.shadertoy.com/view/ll2GD3
vec3 pal( in float t, in vec3 a, in vec3 b, in vec3 c, in vec3 d )
{
    return a + b*cos( 6.28318*(c*t+d) );
}
