#include "Pillar.h"
#include <DirectXMath.h>
#include "GeometryGenerator.h"
#include <DirectXColors.h>

_declspec(align(16)) struct ConstantBuffer
{
    DirectX::XMMATRIX mWorld;
    DirectX::XMMATRIX mView;
    DirectX::XMMATRIX mProjection;
};

Pillar::Pillar(Renderer* renderer) : m_Renderer(renderer)
{
}

bool Pillar::Load()
{
    Geometry::CreateCylinder(0.5f, 0.5f, 4.0f, 8, 8, &m_MeshData);

    // Create vertex buffer
    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = (UINT)(sizeof(Vertex) * m_MeshData.vertices.size());
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vInitData = {};
    vInitData.pSysMem = &m_MeshData.vertices[0];

    DX::ThrowIfFailed(m_Renderer->GetDevice()->CreateBuffer(&vbd, &vInitData, &m_VertexBuffer));

    // Create index buffer
    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = (UINT)(sizeof(UINT) * m_MeshData.indices.size());
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA iInitData = {};
    iInitData.pSysMem = &m_MeshData.indices[0];

    DX::ThrowIfFailed(m_Renderer->GetDevice()->CreateBuffer(&ibd, &iInitData, &m_IndexBuffer));

    // Constant buffer
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    DX::ThrowIfFailed(m_Renderer->GetDevice()->CreateBuffer(&bd, nullptr, &m_ConstantBuffer));

    // Colour buffer
    std::vector<DirectX::XMVECTORF32> vertexColours(m_MeshData.vertices.size());
    for (auto& x : vertexColours)
    {
        x = DirectX::Colors::SandyBrown;
    }

    D3D11_BUFFER_DESC vcbd = {};
    vcbd.Usage = D3D11_USAGE_DEFAULT;
    vcbd.ByteWidth = (UINT)(sizeof(DirectX::XMVECTORF32) * vertexColours.size());
    vcbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vcbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vcInitData = {};
    vcInitData.pSysMem = &vertexColours[0];

    DX::ThrowIfFailed(m_Renderer->GetDevice()->CreateBuffer(&vcbd, &vcInitData, &m_VertexColourBuffer));

    return true;
}

void Pillar::Render(Camera* camera)
{
    // Bind the vertex buffer
    UINT stride[2] = { sizeof(Vertex), sizeof(DirectX::XMVECTORF32) };
    UINT offset[2] = { 0, 0 };

    ID3D11Buffer* buffers[2];
    buffers[0] = m_VertexBuffer;
    buffers[1] = m_VertexColourBuffer;

    m_Renderer->GetDeviceContext()->IASetVertexBuffers(0, 2, buffers, stride, offset);

    // Bind the index buffer
    m_Renderer->GetDeviceContext()->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set topology
    m_Renderer->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Set buffer
    Position.y = 1;
    DirectX::XMMATRIX world = DirectX::XMMatrixTranslation(Position.x, Position.y, Position.z);

    ConstantBuffer cb;
    cb.mWorld = DirectX::XMMatrixTranspose(world);
    cb.mView = DirectX::XMMatrixTranspose(camera->GetView());
    cb.mProjection = DirectX::XMMatrixTranspose(camera->GetProjection());

    m_Renderer->GetDeviceContext()->VSSetConstantBuffers(0, 1, &m_ConstantBuffer);
    m_Renderer->GetDeviceContext()->PSSetConstantBuffers(0, 1, &m_ConstantBuffer);
    m_Renderer->GetDeviceContext()->UpdateSubresource(m_ConstantBuffer, 0, nullptr, &cb, 0, 0);

    // Render geometry
    m_Renderer->GetDeviceContext()->DrawIndexed((UINT)m_MeshData.indices.size(), 0, 0);
}
