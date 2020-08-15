#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include "..\Common\Objects\Surface.h"


namespace Bezier_surface
{
	// This sample renderer instantiates a basic rendering pipeline.
	class SceneRenderer
	{
	public:
		SceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();
		void RenderPatch();
		void RenderControlPoints();
		void StartTracking();
		void TrackingUpdate(float positionX, float positionY);
		void StopTracking();
		bool IsTracking() { return m_tracking; }
		void UpdateZoom(int zoomFactor);

		void UsePatchWireframe(bool usePatchWireframe);
		void DoDrawControlPoints(bool drawControlPoints);
		void UpdateTessellationFactor(int tesselationFactor);
		void UpdatePatchColor(Windows::UI::Color patchColor);
	private:
		void Rotate(float radiansX, float radiansY);

		void FindAndSetDraggedControlPoint(int x, int y);
		DirectX::XMFLOAT3 UnprojectDraggedControlPoint(int x, int y);

	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11HullShader>	m_hullShader;
		Microsoft::WRL::ComPtr<ID3D11DomainShader>	m_domainShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_calculationConstantBuffer;

		ModelViewProjectionConstantBuffer	m_constantBufferData;
		CalculationConstantBuffer	m_calculationBufferData;

		std::shared_ptr<Surface> m_Surface;

		ControlPointParams m_HoveredControlPoint;

		DirectX::XMVECTORF32 m_Eye = { 0.0f, 0.7f, 1.5f, 0.0f };
		DirectX::XMVECTORF32 m_At = { 0.0f, -0.1f, 0.0f, 0.0f };
		DirectX::XMVECTORF32 m_Up = { 0.0f, 1.0f, 0.0f, 0.0f };

		bool	m_isReadyForDrawing;
		float	m_degreesPerSecond;
		
		float	m_mousePosX;
		float	m_mousePosY;

		int		m_zoomFactor = 1;

		float	m_radiansX;
		float	m_radiansY;

		bool	m_tracking;
		bool	m_isRotating;
		bool	m_isDraggingControlPoint;

		bool	m_usePatchWireframe = true;
		bool	m_doDrawControlPoints = true;
		int		m_tessellationFactor = 1;
		DirectX::XMFLOAT4 m_patchColor = { 1.0, 1.0, 1.0, 1.0 };
	};
}

