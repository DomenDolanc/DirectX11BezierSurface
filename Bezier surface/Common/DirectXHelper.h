#pragma once

#include <ppltasks.h>	// For create_task
#include <DirectXMath.h>

struct ProjectedPoint
{
	int screenX;
	int screenY;
	float depth;
};

namespace DX
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			// Set a breakpoint on this line to catch Win32 API errors.
			throw Platform::Exception::CreateException(hr);
		}
	}

	// Function that reads from a binary file asynchronously.
	inline Concurrency::task<std::vector<byte>> ReadDataAsync(const std::wstring& filename)
	{
		using namespace Windows::Storage;
		using namespace Concurrency;

		auto folder = Windows::ApplicationModel::Package::Current->InstalledLocation;

		return create_task(folder->GetFileAsync(Platform::StringReference(filename.c_str()))).then([] (StorageFile^ file) 
		{
			return FileIO::ReadBufferAsync(file);
		}).then([] (Streams::IBuffer^ fileBuffer) -> std::vector<byte> 
		{
			std::vector<byte> returnBuffer;
			returnBuffer.resize(fileBuffer->Length);
			Streams::DataReader::FromBuffer(fileBuffer)->ReadBytes(Platform::ArrayReference<byte>(returnBuffer.data(), fileBuffer->Length));
			return returnBuffer;
		});
	}

	// Converts a length in device-independent pixels (DIPs) to a length in physical pixels.
	inline float ConvertDipsToPixels(float dips, float dpi)
	{
		static const float dipsPerInch = 96.0f;
		return floorf(dips * dpi / dipsPerInch + 0.5f); // Round to nearest integer.
	}

	inline ProjectedPoint ProjectToScreen(DirectX::XMFLOAT3 point, DirectX::XMMATRIX model, DirectX::XMMATRIX view, DirectX::XMMATRIX proj, D3D11_VIEWPORT viewPort)
	{
		ProjectedPoint result;
		DirectX::XMFLOAT4 homogenous = { point.x, point.y, point.z, 1.0 };
		DirectX::XMVECTOR vector = XMLoadFloat4(&homogenous);
		auto transformed = XMVector4Transform(vector, model);
		auto eyeSpace = XMVector4Transform(transformed, view);
		auto projected = XMVector4Transform(eyeSpace, proj);
		XMStoreFloat4(&homogenous, projected);
		if (homogenous.w > 0.0)
		{
			homogenous.x /= homogenous.w;
			homogenous.y /= homogenous.w;
			homogenous.z /= homogenous.w;
		}
		result.screenX = (int)round(((homogenous.x + 1) / 2.0) * viewPort.Width);
		result.screenY = (int)round(((1 - homogenous.y) / 2.0) * viewPort.Height);
		result.depth = homogenous.z;
		return result;
	}

#if defined(_DEBUG)
	// Check for SDK Layer support.
	inline bool SdkLayersAvailable()
	{
		HRESULT hr = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_NULL,       // There is no need to create a real hardware device.
			0,
			D3D11_CREATE_DEVICE_DEBUG,  // Check for the SDK layers.
			nullptr,                    // Any feature level will do.
			0,
			D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Microsoft Store apps.
			nullptr,                    // No need to keep the D3D device reference.
			nullptr,                    // No need to know the feature level.
			nullptr                     // No need to keep the D3D device context reference.
			);

		return SUCCEEDED(hr);
	}
#endif
}
