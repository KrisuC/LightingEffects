#pragma once

#include "Actor.h"
#include "RefCountPtr.h"
#include <vector>

class Scene
{
public:
	void RenderScene()
	{
		for (RefCountPtr<Actor> pActor : m_pActors)
		{
			pActor->RenderComponents();
		}
	}

	std::vector<RefCountPtr<Actor>> m_pActors;
};
