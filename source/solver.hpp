#ifndef SOLVER_HPP
#define SOLVER_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fstream>
#include <vector>
#include <exception>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "nvulkanbase.hpp"
#include "supportStructs.hpp"
#include "Constants.hpp"
#include "map"
#include "set"

#define TINYCOLORMAP_WITH_GLM
#include "tinycolormap.hpp"


using namespace std;


inline bool sortbysecdesc(const pair<int,float> &a,
                   const pair<int,float> &b)
{
       return a.second>b.second;
}

class Solver
{
public:

    Solver();

    std::vector<Creature> creatureList;

    std::vector<Particle> particleList;
    std::vector<InstanceData> instanceList;
    std::vector<Particle>&  ParticleInit(size_t Instance_COUNT);

    glm::vec2 worldSize;
    glm::ivec2 mapSize;
    glm::vec2 mapCellSize;
    int mapElementsAmount;


    int Tick;
    std::vector<uint8_t> colors;
    vector<glm::vec4> colorStateMap;

    float radius;
    float voronoi_scaleradius=1.0f;


    int getIndex(int x,int y)
    {
        return MAP_SIZE_X*y+x;
    }

    glm::vec2 getFromIndex(int index)
    {
        return glm::vec2(int(index%MAP_SIZE_X),int(index/MAP_SIZE_X));
    }


    vector<vector<glm::vec2>> polygons;
    vector<vector<glm::vec2>> polygons_scale;
    vector<vector<glm::vec2>> polygons_draw;

    vector<vector<TrianglesDrawStruct>> triangles_draw_vertex;
    vector<vector<int>> triangles_draw_indexes;

    vector<pair<int,float>> areaList;

};

#endif // SOLVER_HPP
