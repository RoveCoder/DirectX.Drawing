#include "GeometryGenerator.h"
#include <DirectXMath.h>

namespace
{
	void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, unsigned int sliceCount, unsigned int stackCount, MeshData* meshData)
	{
		unsigned int baseIndex = (unsigned int)meshData->vertices.size();

		float y = 0.5f * height;
		float dTheta = 2.0f * DirectX::XM_PI / sliceCount;

		// Duplicate cap ring vertices because the texture coordinates and normals differ.
		for (unsigned int i = 0; i <= sliceCount; ++i)
		{
			float x = topRadius * cosf(i * dTheta);
			float z = topRadius * sinf(i * dTheta);

			// Scale down by the height to try and make top cap texture coord area
			// proportional to base.
			float u = x / height + 0.5f;
			float v = z / height + 0.5f;

			meshData->vertices.push_back(Vertex(x, y, z));
		}

		// Cap center vertex.
		meshData->vertices.push_back(Vertex(0.0f, y, 0.0f));

		// Index of center vertex.
		unsigned int centerIndex = (unsigned int)meshData->vertices.size() - 1;

		for (unsigned int i = 0; i < sliceCount; ++i)
		{
			meshData->indices.push_back(centerIndex);
			meshData->indices.push_back(baseIndex + i + 1);
			meshData->indices.push_back(baseIndex + i);
		}
	}

	void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, unsigned int sliceCount, unsigned int stackCount, MeshData* meshData)
	{
		// 
		// Build bottom cap.
		//

		unsigned int baseIndex = (unsigned int)meshData->vertices.size();
		float y = -0.5f * height;

		// vertices of ring
		float dTheta = 2.0f * DirectX::XM_PI / sliceCount;
		for (unsigned int i = 0; i <= sliceCount; ++i)
		{
			float x = bottomRadius * cosf(i * dTheta);
			float z = bottomRadius * sinf(i * dTheta);

			// Scale down by the height to try and make top cap texture coord area
			// proportional to base.
			float u = x / height + 0.5f;
			float v = z / height + 0.5f;

			meshData->vertices.push_back(Vertex(x, y, z));
		}

		// Cap center vertex.
		meshData->vertices.push_back(Vertex(0.0f, y, 0.0f));

		// Cache the index of center vertex.
		unsigned int centerIndex = (unsigned int)meshData->vertices.size() - 1;

		for (unsigned int i = 0; i < sliceCount; ++i)
		{
			meshData->indices.push_back(centerIndex);
			meshData->indices.push_back(baseIndex + i);
			meshData->indices.push_back(baseIndex + i + 1);
		}
	}
}

void Geometry::CreateBox(float width, float height, float depth, MeshData* mesh)
{
	Vertex vertices[] =
	{
		{ -width, -height, -depth },
		{ -width, +height, -depth },
		{ +width, +height, -depth },
		{ +width, -height, -depth },

		{ -width, -height, +depth },
		{ +width, -height, +depth },
		{ +width, +height, +depth },
		{ -width, +height, +depth },

		{ -width, +height, -depth },
		{ -width, +height, +depth },
		{ +width, +height, +depth },
		{ +width, +height, -depth },

		{ -width, -height, -depth },
		{ +width, -height, -depth },
		{ +width, -height, +depth },
		{ -width, -height, +depth },

		{ -width, -height, +depth },
		{ -width, +height, +depth },
		{ -width, +height, -depth },
		{ -width, -height, -depth },

		{ +width, -height, -depth },
		{ +width, +height, -depth },
		{ +width, +height, +depth },
		{ +width, -height, +depth }
	};

	unsigned int indices[] =
	{
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		16, 17, 18,
		16, 18, 19,

		20, 21, 22,
		20, 22, 23,
	};

	mesh->vertices.assign(&vertices[0], &vertices[24]);
	mesh->indices.assign(&indices[0], &indices[36]);
}

void Geometry::CreateGrid(float width, float depth, unsigned int m, unsigned int n, MeshData* mesh)
{
	unsigned int vertexCount = m * n;
	unsigned int faceCount = (m - 1) * (n - 1) * 2;

	// Create the vertices.
	float halfWidth = 0.5f * width;
	float halfDepth = 0.5f * depth;

	float dx = width / (n - 1);
	float dz = depth / (m - 1);

	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);

	mesh->vertices.resize(vertexCount);
	for (unsigned int i = 0; i < m; ++i)
	{
		float z = halfDepth - i * dz;
		for (unsigned int j = 0; j < n; ++j)
		{
			float x = -halfWidth + j * dx;

			mesh->vertices[i * n + j].x = x;
			mesh->vertices[i * n + j].y = 0.0f;
			mesh->vertices[i * n + j].z = z;
		}
	}

	// Create the indices.
	mesh->indices.resize(faceCount * 3); // 3 indices per face

	// Iterate over each quad and compute indices.
	unsigned int k = 0;
	for (unsigned int i = 0; i < m - 1; ++i)
	{
		for (unsigned int j = 0; j < n - 1; ++j)
		{
			mesh->indices[k] = i * n + j;
			mesh->indices[k + 1] = i * n + j + 1;
			mesh->indices[k + 2] = (i + 1) * n + j;

			mesh->indices[k + 3] = (i + 1) * n + j;
			mesh->indices[k + 4] = i * n + j + 1;
			mesh->indices[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}
}

void Geometry::CreateCylinder(float bottomRadius, float topRadius, float height, unsigned int sliceCount, unsigned int stackCount, MeshData* meshData)
{
	meshData->vertices.clear();
	meshData->indices.clear();

	//
	// Build Stacks.
	// 

	float stackHeight = height / stackCount;

	// Amount to increment radius as we move up each stack level from bottom to top.
	float radiusStep = (topRadius - bottomRadius) / stackCount;

	unsigned int ringCount = stackCount + 1;

	// Compute vertices for each stack ring starting at the bottom and moving up.
	for (unsigned int i = 0; i < ringCount; ++i)
	{
		float y = -0.5f * height + i * stackHeight;
		float r = bottomRadius + i * radiusStep;

		// vertices of ring
		float dTheta = 2.0f * DirectX::XM_PI / sliceCount;
		for (unsigned int j = 0; j <= sliceCount; ++j)
		{
			Vertex vertex;

			float c = cosf(j * dTheta);
			float s = sinf(j * dTheta);

			DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(r * c, y, r * s);

			vertex.x = position.x;
			vertex.y = position.y;
			vertex.z = position.z;

			meshData->vertices.push_back(vertex);
		}
	}

	// Add one because we duplicate the first and last vertex per ring
	// since the texture coordinates are different.
	unsigned int ringVertexCount = sliceCount + 1;

	// Compute indices for each stack.
	for (unsigned int i = 0; i < stackCount; ++i)
	{
		for (unsigned int j = 0; j < sliceCount; ++j)
		{
			meshData->indices.push_back(i * ringVertexCount + j);
			meshData->indices.push_back((i + 1) * ringVertexCount + j);
			meshData->indices.push_back((i + 1) * ringVertexCount + j + 1);

			meshData->indices.push_back(i * ringVertexCount + j);
			meshData->indices.push_back((i + 1) * ringVertexCount + j + 1);
			meshData->indices.push_back(i * ringVertexCount + j + 1);
		}
	}

	BuildCylinderTopCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);
	BuildCylinderBottomCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);
}
