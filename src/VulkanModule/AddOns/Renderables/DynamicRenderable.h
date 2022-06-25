//
// DynamicRenderable.h
//	VulkanModule AddOns
//
// A "dynamic" renderable incorporates draw commands that record upon each frame,
//	as the way it's drawn continually changes, or it changes regularly enough
//	that it is simpler to rerecord it continually.
//
// 3/26/20 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef DynamicRenderable_h
#define DynamicRenderable_h

#include "iRenderable.h"
#include "math.h"

struct DynamicRenderable : public iRenderable
{
public:
	DynamicRenderable(Renderable& renderable, VulkanSetup& vulkan, iPlatform& platform);
    int step=0;

	iRenderable* newConcretion(CommandRecording* pRecordingMode) const
	{
		*pRecordingMode = UPON_EACH_FRAME;

		return new DynamicRenderable(*this);
	}

    void IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int bufferIndex);



    void Update(float deltaTime)
    {
        VertexBasedObject exp01= static_cast<VertexBasedObject>(this->vertexObject);
        Vertex2DTextured* exp11= static_cast<Vertex2DTextured*>(exp01.vertices);
        auto& v01=exp11[0].position;
        float sinT=sin(step*0.1f);
        v01+=glm::vec2(sinT*0.001f,sinT*0.001f);
        step++;

    }

};

#endif	// DynamicRenderable_h
