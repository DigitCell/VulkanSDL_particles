#include "solver.hpp"
 #include "solver.hpp"
#include "solver.hpp"
#include "tinycolormap.hpp"


Solver::Solver()
{
        worldSize=glm::vec2(SIM_WIDTH, SIM_HEIGHT);
        mapSize=glm::vec2(MAP_SIZE_X, MAP_SIZE_Y);
        radius=PARTICLE_RADIUS;
        {
            mapCellSize.x=static_cast<int>(worldSize.x/MAP_SIZE_X);
            mapCellSize.y=static_cast<int>(worldSize.y/MAP_SIZE_Y);
        }

        mapElementsAmount=MAP_SIZE_X*MAP_SIZE_Y;

        // Define number of levels for the colormap quantization. This value should be in [1, 255]; otherwise, it will be cropped to 1 or 255.
        const unsigned int num_levels = 10;
        colorStateMap.clear();
        // Get the mapped color. Here, Parula is specified as the colormap.
        for(int i=0; i<9;i++)
        {
            const tinycolormap::Color color = tinycolormap::GetQuantizedColor(i/10.f, num_levels, tinycolormap::ColormapType::Parula);
            colorStateMap.push_back(glm::vec4(color.r(),color.g(),color.b(),1.0f));
        }
}


std::vector<Particle>&  Solver::ParticleInit(size_t Instance_COUNT)
{
    std::default_random_engine rndEngine((unsigned)time(nullptr));
    std::uniform_real_distribution<float> rndDist(-1.0f, 1.0f);

    // Initial particle positions
    particleList.clear();
    particleList.resize(Instance_COUNT);


    int lineWidth=sqrt(Instance_COUNT);
    float step=0.85*SIM_WIDTH/lineWidth;
   // glm::vec2 initCoord{-step*lineWidth/2.0f, -step*lineWidth/2.0f };
    glm::vec2 initCoord{SIM_WIDTH/2-step*lineWidth/2.0f,SIM_HEIGHT/2-step*lineWidth/2.0f};

    glm::vec2 center{SIM_WIDTH/2,SIM_HEIGHT/2};

    float x=0,y=0;
    int counter=0;

    // Distribute rocks randomly on two different rings
    for (auto i = 0; i < Instance_COUNT ; i++) {

        //theta = 2.0 * M_PI * uniformDist(rndGenerator);
        particleList[i].pos =initCoord+glm::vec2(x,y);
        particleList[i].vel =  0.0f*glm::vec2(rndDist(rndEngine),rndDist(rndEngine));
        particleList[i].force = glm::vec2(0.0,0.0);

        particleList[i].pos_old=    particleList[i].pos;
        particleList[i].vel_old =   particleList[i].vel;
        particleList[i].force_old = particleList[i].force;

        particleList[i].radius = 0.25f;
        particleList[i].constructSize=0;
        /*
        if(i%50==0)
            particleList[i].constructSizeMax=4;
        else if(i%2==0)
            particleList[i].constructSizeMax=1;
        else
            particleList[i].constructSizeMax=2;
            */

        if(int(x/100)%2==0 && int(y/100)%2==0)
        {
            if((rand()%30+1)<3)
               particleList[i].constructSizeMax=5;
            else
               particleList[i].constructSizeMax=3;
        }
        else
        {
            if((rand()%30+1)<3)
               particleList[i].constructSizeMax=2;
            else
               particleList[i].constructSizeMax=3;
        }



        x+=step;
        counter++;
        if(counter>lineWidth-1)
        {
          x=0;
          y+=step;
          counter=0;
        }
    }

    return particleList;
}



