#pragma once

namespace Bezier_surface
{
	// Constant buffer used to send MVP matrices to the vertex shader.
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
		DirectX::XMFLOAT4 eye;
	};

	struct CalculationConstantBuffer
	{
		DirectX::XMFLOAT4X4 bezierCoeficients;
		DirectX::XMFLOAT4X4 transposedBezierCoeficients;
		DirectX::XMFLOAT4X4 controlPoints;

		DirectX::XMFLOAT4 color;

		float tessellationFactor = 1.0;
		DirectX::XMFLOAT3 padding;
	};

	// Used to send per-vertex data to the vertex shader.
	struct VertexPosition
	{
		DirectX::XMFLOAT3 pos;
	};
}