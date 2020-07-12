#pragma once

#include "..\Common\DeviceResources.h"
#include "Content/ShaderStructures.h"
#include "..\Common\DirectXHelper.h"
#include <map>

namespace Bezier_surface
{
    struct ControlPointParams
    {
        VertexPosition Vertex;
        int VertexIndex;
        double Depth;
    };

    class Surface
    {
    public:
        Surface(std::shared_ptr<DX::DeviceResources> deviceResources);
        Surface(std::shared_ptr<DX::DeviceResources> deviceResources, int columns, int rows);
        ~Surface();

        void CreateVertices();
        void CreateIndices();
        void CreateQuadIndices();

        void UpdateControlPoint(int index, VertexPosition& vertex);

        void UpdateVertices();

        size_t getVerticesCount();
        size_t getIndicesCount();

        std::vector<VertexPosition> getVertices() const { return m_vertices; };

        boolean isReadyForDrawing() const { return m_loadingComplete; };

        void setScaling(double scaling);

        DirectX::XMFLOAT2 getGridSize();
        void setGridSize(float columns, float rows);

        DirectX::XMFLOAT4X4 getControlPointsMatrix() const { return m_controlPointsMatrix; };
        DirectX::XMFLOAT4X4 getBezierMatrix() const { return m_bezierMatrix; };

        void ResetBuffers();

        void Draw();
        void DrawControlPoints();

    private:
        void CreateBezierMatrix();

        std::shared_ptr<DX::DeviceResources> m_deviceResources;

        int m_Columns = 100;
        int m_Rows = 100;
        float m_scaling;

        std::vector<VertexPosition> m_vertices;
        std::vector<uint32_t> m_indices;
        std::vector<uint32_t> m_controlIndices;
        size_t	m_verticesCount;
        size_t	m_controlIndexCount;
        size_t	m_indexCount;

        DirectX::XMFLOAT4X4 m_controlPointsMatrix;
        DirectX::XMFLOAT4X4 m_bezierMatrix;

        
        bool m_loadingComplete = true;

        Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_controlIndexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
    };
}

