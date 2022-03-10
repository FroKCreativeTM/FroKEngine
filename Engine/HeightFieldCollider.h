#pragma once

#include "BaseCollider.h"

class HeightFieldCollider : public BaseCollider
{
public : 
	HeightFieldCollider();
	~HeightFieldCollider();

	void FinalUpdate();

private : 
	// the geometry for the height field
	const physx::PxHeightFieldGeometry& geometry;

	//The height field has three scaling parameters
	physx::PxReal    rs;
	physx::PxReal    hs;
	physx::PxReal    cs;

	// a shared data structure, stores the row and column count
	physx::PxHeightField* hf = geometry.heightField;
	physx::PxU32     nbCols = hf->getNbColumns();
	physx::PxU32     nbRows = hf->getNbRows();


};

