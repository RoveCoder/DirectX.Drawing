#pragma once

#include "Mesh.h"

namespace Geometry
{
	void CreateBox(float width, float height, float depth, MeshData* mesh);

	void CreateGrid(float width, float depth, unsigned int m, unsigned int n, MeshData* mesh);

	void CreateCylinder(float bottomRadius, float topRadius, float height, unsigned int  sliceCount, unsigned int  stackCount, MeshData* meshData);
}