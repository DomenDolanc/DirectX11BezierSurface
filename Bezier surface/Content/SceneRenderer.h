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
		void StartTracking();
		void TrackingUpdate(float positionX, float positionY);
		void StopTracking();
		bool IsTracking() { return m_tracking; }


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

		bool	m_loadingComplete;
		float	m_degreesPerSecond;
		bool	m_tracking;
		bool	m_isRotating;
		bool	m_isDraggingControlPoint;
	};
}

