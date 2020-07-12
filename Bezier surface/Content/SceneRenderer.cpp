#include "pch.h"
#include "SceneRenderer.h"
#include "..\Common\DirectXHelper.h"

using namespace Bezier_surface;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
SceneRenderer::SceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

	m_Surface = std::make_shared<Surface>(m_deviceResources, 4, 4);
}

// Initializes view parameters when the window size changes.
void SceneRenderer::CreateWindowSizeDependentResources()
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
		);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&m_constantBufferData.projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
		);

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixIdentity());
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void SceneRenderer::Update(DX::StepTimer const& timer)
{

}

// Rotate the 3D cube model a set amount of radians.
void SceneRenderer::Rotate(float radiansX, float radiansY)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixMultiply(XMMatrixRotationY(radiansX), XMMatrixRotationX(radiansY))));
}

void SceneRenderer::StartTracking()
{
	m_tracking = true;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void SceneRenderer::TrackingUpdate(float positionX, float positionY)
{
	if (m_Surface->isReadyForDrawing() && !m_isDraggingControlPoint && !m_isRotating)
		FindAndSetDraggedControlPoint(positionX, positionY);

	if (m_tracking)
	{
		if (m_isDraggingControlPoint)
		{
			m_HoveredControlPoint.Vertex.pos = UnprojectDraggedControlPoint(positionX, positionY);
			m_Surface->UpdateControlPoint(m_HoveredControlPoint.VertexIndex, m_HoveredControlPoint.Vertex);
		} 
		else
		{
			float radiansX = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
			float radiansY = XM_2PI * 2.0f * positionY / m_deviceResources->GetOutputSize().Width;
			Rotate(radiansX, radiansY);
			m_isRotating = true;
		}
	}
}

void Bezier_surface::SceneRenderer::FindAndSetDraggedControlPoint(int x, int y)
{
	constexpr double MIN_DISTANCE = 8;

	D3D11_VIEWPORT viewPort = m_deviceResources->GetScreenViewport();
	std::vector<VertexPosition> vertices = m_Surface->getVertices();

	XMMATRIX proj = XMMatrixTranspose(XMLoadFloat4x4(&m_constantBufferData.projection));
	XMMATRIX view = XMMatrixTranspose(XMLoadFloat4x4(&m_constantBufferData.view));
	XMMATRIX model = XMMatrixTranspose(XMLoadFloat4x4(&m_constantBufferData.model));

	for (int i = 0; i < vertices.size(); i++)
	{
		VertexPosition vertex = vertices[i];
		ProjectedPoint projected = DX::ProjectToScreen(vertex.pos, model, view, proj, viewPort);

		int xDiff = projected.screenX - x;
		int yDiff = projected.screenY - y;
		double dist = sqrt(xDiff * xDiff + yDiff * yDiff);
		if (dist < MIN_DISTANCE)
		{
			m_HoveredControlPoint.Depth = projected.depth;
			m_HoveredControlPoint.Vertex = vertex;
			m_HoveredControlPoint.VertexIndex = i;

			m_isDraggingControlPoint = true;
			break;
		}
	}
}

DirectX::XMFLOAT3 Bezier_surface::SceneRenderer::UnprojectDraggedControlPoint(int x, int y)
{
	XMFLOAT3 unprojectedPoint;
	D3D11_VIEWPORT viewPort = m_deviceResources->GetScreenViewport();
	XMMATRIX proj = XMMatrixTranspose(XMLoadFloat4x4(&m_constantBufferData.projection));
	XMMATRIX view = XMMatrixTranspose(XMLoadFloat4x4(&m_constantBufferData.view));
	XMMATRIX model = XMMatrixTranspose(XMLoadFloat4x4(&m_constantBufferData.model));
	XMVECTOR clipSpace = XMVectorSet(x, y, m_HoveredControlPoint.Depth, 0.0f);

	XMVECTOR unprojected = XMVector3Unproject(clipSpace, 0.0f, 0.0f, viewPort.Width, viewPort.Height, 0.0f, 1.0f, proj, view, XMMatrixIdentity());
	unprojected = XMVector3Transform(unprojected, XMMatrixInverse(nullptr, model));

	XMStoreFloat3(&unprojectedPoint, unprojected);
	return unprojectedPoint;
}

void SceneRenderer::StopTracking()
{
	m_tracking = false;
	m_isRotating = false;
	m_isDraggingControlPoint = false;
}

// Renders one frame using the vertex and pixel shaders.
void SceneRenderer::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete || !m_Surface->isReadyForDrawing())
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	XMMATRIX tmpMatrix = XMLoadFloat4x4(&m_Surface->getControlPointsMatrix());
	tmpMatrix = XMMatrixTranspose(tmpMatrix);
	XMStoreFloat4x4(&m_calculationBufferData.controlPoints, tmpMatrix);

	m_calculationBufferData.transposedBezierCoeficients = m_Surface->getBezierMatrix();
	m_calculationBufferData.color = { 1.0, 1.0, 1.0, 1.0 };

	tmpMatrix = XMLoadFloat4x4(&m_Surface->getBezierMatrix());
	tmpMatrix = XMMatrixTranspose(tmpMatrix);
	XMStoreFloat4x4(&m_calculationBufferData.bezierCoeficients, tmpMatrix);

	XMMATRIX m = XMMatrixMultiply(XMMatrixMultiply(XMLoadFloat4x4(&m_Surface->getBezierMatrix()), XMLoadFloat4x4(&m_Surface->getControlPointsMatrix())), tmpMatrix);

	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);
	context->UpdateSubresource1(m_calculationConstantBuffer.Get(), 0, NULL, &m_calculationBufferData, 0, 0, 0);

	auto rasterizer = m_deviceResources->GetRasterizerState();

	context->IASetInputLayout(m_inputLayout.Get());
	context->RSSetState(rasterizer);

	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);

	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
	context->VSSetConstantBuffers1(1, 1, m_calculationConstantBuffer.GetAddressOf(), nullptr, nullptr);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	context->HSSetShader(m_hullShader.Get(), nullptr, 0);
	context->DSSetShader(m_domainShader.Get(), nullptr, 0);
	context->DSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
	context->DSSetConstantBuffers1(1, 1, m_calculationConstantBuffer.GetAddressOf(), nullptr, nullptr);

	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	m_Surface->Draw();

	m_calculationBufferData.color = { 1.0, 0.0, 0.0, 1.0 };
	context->UpdateSubresource1(m_calculationConstantBuffer.Get(), 0, NULL, &m_calculationBufferData, 0, 0, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	context->HSSetShader(nullptr, nullptr, 0);
	context->DSSetShader(nullptr, nullptr, 0);

	m_Surface->DrawControlPoints();
}

void SceneRenderer::CreateDeviceDependentResources()
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"VertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"PixelShader.cso");
	auto loadHSTask = DX::ReadDataAsync(L"HullShader.cso");
	auto loadDSTask = DX::ReadDataAsync(L"DomainShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShader
				)
			);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc [] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayout
				)
			);
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShader
				)
			);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
			)
		);

		CD3D11_BUFFER_DESC calcConstantBufferDesc(sizeof(CalculationConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&calcConstantBufferDesc,
				nullptr,
				&m_calculationConstantBuffer
			)
		);
	});

	auto createHSTask = loadHSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateHullShader(&fileData[0], fileData.size(), nullptr, &m_hullShader));
		});

	auto createDSTask = loadDSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateDomainShader(&fileData[0], fileData.size(), nullptr, &m_domainShader));
		});

	// Once both shaders are loaded, create the mesh.
	auto createCubeTask = (createPSTask && createVSTask && createHSTask && createDSTask).then([this] () {

		m_Surface->CreateVertices();
		m_Surface->CreateIndices();
		m_Surface->CreateQuadIndices();
	});

	// Once the cube is loaded, the object is ready to be rendered.
	createCubeTask.then([this] () {
		m_loadingComplete = true;
	});
}

void SceneRenderer::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_Surface->ResetBuffers();
}