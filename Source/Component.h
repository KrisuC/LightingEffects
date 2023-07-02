#pragma once

#include "EngineUtility.h"
#include "RefCountPtr.h"

/** Holding data and infomation to render the primivite */
class Component : public IRefCountResource
{
public:
	/** Init resources, called at loading */
	virtual void InitResource() = 0;

	/** Called every frame, before actual rendering happens */
	virtual void Update() = 0;

	/** Render the primitive */
	virtual void Render() = 0;

	/** Free resouces, unload component */
	virtual void FreeResouce() = 0;
};


