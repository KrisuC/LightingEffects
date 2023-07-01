#pragma once

#include "Component.h"
#include "EngineUtility.h"
#include <vector>

class Actor
{
public:
	void Render()
	{
		for (RefCountPtr<Component> pComponent : m_components)
		{
			pComponent->Render();
		}
	}

private:
	std::vector< RefCountPtr<Component> > m_components;
};