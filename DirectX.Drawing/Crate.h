#pragma once

#include "Renderer.h"
#include "Camera.h"
#include "Mesh.h"

class Crate
{
public:
	Crate(Renderer* renderer);

	bool Load();
	void Render(Camera* camera);

private:
	Renderer* m_Renderer = nullptr;

	MeshData m_MeshData;

	ID3D11Buffer* m_VertexBuffer = nullptr;
	ID3D11Buffer* m_IndexBuffer = nullptr;

	ID3D11Buffer* m_VertexColourBuffer = nullptr;
	ID3D11Buffer* m_ConstantBuffer = nullptr;
};