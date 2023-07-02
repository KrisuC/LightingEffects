#pragma once

#include "Component.h"
#include "RefCountPtr.h"
#include <vector>

class Actor : public IRefCountResource
{
public:
	void RenderComponents()
	{
		for (RefCountPtr<Component> pComponent : m_pComponents)
		{
			pComponent->Render();
		}
	}

private:
	std::vector< RefCountPtr<Component> > m_pComponents;
};
