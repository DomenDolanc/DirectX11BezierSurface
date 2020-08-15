#include "pch.h"
#include "Surface.h"
#include <stdint.h>
#include <algorithm>

using namespace Bezier_surface;

using namespace DirectX;
using namespace Windows::Foundation;

Surface::Surface(std::shared_ptr<DX::DeviceResources> deviceResources)
{
    m_deviceResources = deviceResources;
    m_scaling = 1;

    CreateBezierMatrix();
}

Surface::Surface(std::shared_ptr<DX::DeviceResources> deviceResources, int columns, int rows)
{
    m_deviceResources = deviceResources;

    m_scaling = 1;
    m_Columns = columns;
    m_Rows = rows;

    CreateBezierMatrix();
}

Surface::~Surface()
{
    m_vertices.clear();
    m_indices.clear();
    m_controlIndices.clear();
}

void Surface::CreateVertices()
{
    m_vertices.clear();
    m_vertexBuffer.Reset();

    XMFLOAT3 gridColor{ 1.0f, 1.0f, 1.0f };

    m_verticesCount = m_Columns * m_Rows;

    float halfScaling = m_scaling / 2.0f;
    const float startX = -halfScaling;
    const float endX = halfScaling;

    const float startZ = -halfScaling;
    const float endZ = halfScaling;

    const float stepX = (endX - startX) / (m_Columns - 1);
    const float stepZ = (endZ - startZ) / (m_Rows - 1);

    float tempX = startX;

    for (int i = 0; i < m_Columns; i++)
    {
        float tempZ = startZ;
        for (int j = 0; j < m_Rows; j++)
        {
            VertexPosition vertex;
            vertex.pos = XMFLOAT3(tempX, 0.0, tempZ);
            if (i == 0 || j == 0 || i == 3 || j == 3)
                vertex.pos.y = -0.5f;
            m_controlPointsMatrix.m[i][j] = vertex.pos.y;
            m_vertices.emplace_back(vertex);
            tempZ += stepZ;
        }
        tempX += stepX;
    }

    m_verticesCount = m_vertices.size();

    D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
    vertexBufferData.pSysMem = &m_vertices.front();
    vertexBufferData.SysMemPitch = 0;
    vertexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC vertexBufferDesc(m_verticesCount * sizeof(VertexPosition), D3D11_BIND_VERTEX_BUFFER);
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer));
}

void Surface::CreateIndices()
{
    m_controlIndexBuffer.Reset();
    m_controlIndices.clear();
    for (int i = 0; i < m_Columns; i++)
    {
        for (int j = 0; j < m_Rows - 1; j++)
        { 
            m_controlIndices.emplace_back(i * m_Rows + j);
            m_controlIndices.emplace_back(i * m_Rows + j + 1); 

            m_controlIndices.emplace_back(j * m_Columns + i);
            m_controlIndices.emplace_back((j + 1) * m_Columns + i); 
        }
    }

    m_controlIndexCount = m_controlIndices.size();

    D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
    indexBufferData.pSysMem = &m_controlIndices.front();
    indexBufferData.SysMemPitch = 0;
    indexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC indexBufferDesc(m_controlIndexCount * sizeof(uint32_t), D3D11_BIND_INDEX_BUFFER);
    DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_controlIndexBuffer));
}

void Surface::CreateQuadIndices()
{
    m_indexBuffer.Reset();
    m_indices.clear();
    for (int i = 0; i < m_Columns - 1; i++)
    {
        for (int j = 0; j < m_Rows - 1; j++)
        {
            m_indices.emplace_back(i * m_Rows + j);             // 0
            m_indices.emplace_back((i + 1) * m_Rows + j);       // 2
            m_indices.emplace_back(i * m_Rows + j + 1);         // 1
            m_indices.emplace_back((i + 1) * m_Rows + j + 1);   // 3
        }
        m_indices.emplace_back(-1);
    }

    m_indexCount = m_indices.size();

    D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
    indexBufferData.pSysMem = &m_indices.front();
    indexBufferData.SysMemPitch = 0;
    indexBufferData.SysMemSlicePitch = 0;
    CD3D11_BUFFER_DESC indexBufferDesc(m_indexCount * sizeof(uint32_t), D3D11_BIND_INDEX_BUFFER);
    DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer));
}

void Bezier_surface::Surface::UpdateControlPoint(int index, VertexPosition& vertex)
{
    m_vertices[index].pos.y = vertex.pos.y;
    m_controlPointsMatrix.m[index / m_Rows][index - (index / m_Rows * m_Columns)] = vertex.pos.y;
    UpdateVertices();
}

void Bezier_surface::Surface::UpdateVertices()
{
    m_isReadyForDrawing = false;

    D3D11_MAPPED_SUBRESOURCE subResource;
    DX::ThrowIfFailed(m_deviceResources->GetD3DDeviceContext()->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource));
    memcpy(subResource.pData, &m_vertices.front(), sizeof(VertexPosition) * m_verticesCount);
    m_deviceResources->GetD3DDeviceContext()->Unmap(m_vertexBuffer.Get(), 0);

    m_isReadyForDrawing = true;
}

size_t Surface::getVerticesCount()
{
    return m_verticesCount;
}

size_t Surface::getIndicesCount()
{
    return m_indexCount;
}

void Surface::setScaling(double scaling)
{
    m_scaling = scaling;
}

XMFLOAT2 Surface::getGridSize()
{
    return XMFLOAT2((float)m_Columns, (float)m_Rows);
}

void Surface::setGridSize(float columns, float rows)
{
    m_Columns = (int) columns;
    m_Rows = (int) rows;
}

void Surface::ResetBuffers()
{
    m_vertexBuffer.Reset();
    m_indexBuffer.Reset();
    m_controlIndexBuffer.Reset();
}

void Surface::Draw()
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    UINT stride = sizeof(VertexPosition);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    context->DrawIndexed(m_indexCount, 0, 0);
}

void Surface::DoDrawControlPoints()
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    UINT stride = sizeof(VertexPosition);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_controlIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    context->DrawIndexed(m_controlIndexCount, 0, 0);
}

void Bezier_surface::Surface::CreateBezierMatrix()
{
    m_bezierMatrix._11 = 1.0;
    m_bezierMatrix._12 = 0.0;
    m_bezierMatrix._13 = 0.0;
    m_bezierMatrix._14 = 0.0;

    m_bezierMatrix._21 = -3.0;
    m_bezierMatrix._22 = 3.0;
    m_bezierMatrix._23 = 0.0;
    m_bezierMatrix._24 = 0.0;

    m_bezierMatrix._31 = 3.0;
    m_bezierMatrix._32 = -6.0;
    m_bezierMatrix._33 = 3.0;
    m_bezierMatrix._34 = 0.0;

    m_bezierMatrix._41 = -1.0;
    m_bezierMatrix._42 = 3.0;
    m_bezierMatrix._43 = -3.0;
    m_bezierMatrix._44 = 1.0;
}
