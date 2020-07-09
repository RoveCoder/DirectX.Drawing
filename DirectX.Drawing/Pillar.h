#pragma once

#include "Renderer.h"
#include "Camera.h"
#include "Mesh.h"
#include <DirectXMath.h>

class Pillar
{
public:
	Pillar(Renderer* renderer);

	bool Load();
	void Render(Camera* camera);

	DirectX::XMFLOAT3 Position;

private:
	Renderer* m_Renderer = nullptr;

	MeshData m_MeshData;

	ID3D11Buffer* m_VertexBuffer = nullptr;
	ID3D11Buffer* m_IndexBuffer = nullptr;

	ID3D11Buffer* m_VertexColourBuffer = nullptr;
	ID3D11Buffer* m_ConstantBuffer = nullptr;
};