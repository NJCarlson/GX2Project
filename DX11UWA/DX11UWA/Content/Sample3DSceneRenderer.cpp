﻿#include "pch.h"
#include "Sample3DSceneRenderer.h"
#include "ModelLoader.h"
#include <thread>
#include "..\Common\DirectXHelper.h"

using namespace DX11UWA;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),
	m_indexCount(0),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	memset(m_kbuttons, 0, sizeof(m_kbuttons));
	m_currMousePos = nullptr;
	m_prevMousePos = nullptr;
	memset(&m_camera, 0, sizeof(XMFLOAT4X4));

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void Sample3DSceneRenderer::CreateWindowSizeDependentResources(void)
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
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, 0.01f, 100.0f);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(&m_constantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { 5.0f, 10.0f, -12.0f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_camera, XMMatrixInverse(nullptr, XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));
}


// Called once per frame, rotates the cube and calculates the model and view matrices.
void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	if (!m_tracking)
	{
		// Convert degrees to radians, then convert seconds to rotation angle
		float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
		float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

		Rotate(radians);
	}

	//The AlienTree
	m_loadedBufferData.view = m_constantBufferData.view;
	m_loadedBufferData.projection = m_constantBufferData.projection;
	XMStoreFloat4x4(&m_loadedBufferData.model, XMMatrixTranspose(XMMatrixTranslation(-5, -2, 0)));

	//waterTower_loadedBufferData
	waterTower.loadedbufdata.view = m_constantBufferData.view;
	waterTower.loadedbufdata.projection = m_constantBufferData.projection;
	XMStoreFloat4x4(&waterTower.loadedbufdata.model, XMMatrixTranspose(XMMatrixTranslation(-10, -2, 8)));

	//Sky Box 
	m_skyBoxBufferData.view = m_constantBufferData.view;
	m_skyBoxBufferData.projection = m_constantBufferData.projection;
	XMStoreFloat4x4(&m_skyBoxBufferData.model, XMMatrixTranspose(XMMatrixTranslation(m_camera._41, m_camera._42, m_camera._43)));

	//instancing
	m_instanceconstbufdata.view = m_constantBufferData.view;
	m_instanceconstbufdata.projection = m_constantBufferData.projection;
	XMStoreFloat4x4(&m_instanceconstbufdata.model[0], XMMatrixTranspose(XMMatrixTranslation(2, 0, 0)));
	XMStoreFloat4x4(&m_instanceconstbufdata.model[1], XMMatrixTranspose(XMMatrixTranslation(4, 0, 0)));
	XMStoreFloat4x4(&m_instanceconstbufdata.model[2], XMMatrixTranspose(XMMatrixTranslation(6, 0, 0)));

	
	//floor lights
	XMStoreFloat4(&m_LightProperties.EyePosition, XMVectorSet(m_camera._41, m_camera._42, m_camera._43, 1.0f));

	for (int i = 0; i < numLights; ++i)
	{
		Light light;
		XMFLOAT4 LightPosition;
		memset(&light, 0, sizeof(Light));
		light.LightTypeEnabled.y = LightEnabled[i];
		light.LightTypeEnabled.x = i;
		light.Color = XMFLOAT4(LightColors[i]);
		light.AttenuationData.x = XMConvertToRadians(45.0f);
		light.AttenuationData.y = 1.0f;
		light.AttenuationData.z = 0.08f;
		light.AttenuationData.w = 0.0f;
		
		//Directional light location
		if (i == 0)
		{
			if ( dirLightSwitch)
			{
				directionalLightPos.x += 1;
				if (directionalLightPos.x >= 20)
				{
					dirLightSwitch = false;
				}
			}
			else
			{
				directionalLightPos.x -= 1;
				if (directionalLightPos.x <= -20)
				{
					dirLightSwitch = true;
				}
			}

			LightPosition = directionalLightPos;
		}

		//point light location
		if (i == 1)
		{
			if (pointLightSwitch)
			{
				pointLightPos.x += .25f;
				if (pointLightPos.x >= 20)
				{
					pointLightSwitch = false;
				}
			}
			else
			{
				pointLightPos.x -= .25f;
				if (pointLightPos.x <= -20)
				{
					pointLightSwitch = true;
				}
			}
			LightPosition = pointLightPos;
		}
		
		//Spot Light Information
		if (i == 2) // Spot light Location
		{
			LightPosition = spotPos;
		}

		light.radius.x = spotRad;
		light.ConeRatio.x = innerConeRat;
		light.ConeRatio.y = outterConeRat;
		light.coneAngle = coneAng;

		light.Position = LightPosition;
		XMVECTOR LightDirection = XMVectorSet(-LightPosition.x, -LightPosition.y, -LightPosition.z, 0.0f);
		LightDirection = XMVector3Normalize(LightDirection);
		XMStoreFloat4(&light.Direction, LightDirection);

		m_LightProperties.Lights[i] = light;
	}

	//m_d3dDeviceContext->UpdateSubresource(m_d3dLightPropertiesConstantBuffer.Get(), 0, nullptr, &m_LightProperties, 0, 0);
	m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(lightbuffer.Get(), 0, NULL, &m_LightProperties, 0, 0);


	// Update or move camera here
	UpdateCamera(timer, 1.0f, 0.75f);

}

// Rotate the 3D cube model a set amount of radians.
void Sample3DSceneRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
}

void Sample3DSceneRenderer::UpdateCamera(DX::StepTimer const& timer, float const moveSpd, float const rotSpd)
{
	const float delta_time = (float)timer.GetElapsedSeconds();

	if (m_kbuttons['W'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['S'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, -moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['A'])
	{
		XMMATRIX translation = XMMatrixTranslation(-moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['D'])
	{
		XMMATRIX translation = XMMatrixTranslation(moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['X'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, -moveSpd * delta_time, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons[VK_SPACE])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, moveSpd * delta_time, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}

	if (m_currMousePos)
	{
		if (m_currMousePos->Properties->IsRightButtonPressed && m_prevMousePos)
		{
			float dx = m_currMousePos->Position.X - m_prevMousePos->Position.X;
			float dy = m_currMousePos->Position.Y - m_prevMousePos->Position.Y;

			XMFLOAT4 pos = XMFLOAT4(m_camera._41, m_camera._42, m_camera._43, m_camera._44);

			m_camera._41 = 0;
			m_camera._42 = 0;
			m_camera._43 = 0;

			XMMATRIX rotX = XMMatrixRotationX(dy * rotSpd * delta_time);
			XMMATRIX rotY = XMMatrixRotationY(dx * rotSpd * delta_time);

			XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
			temp_camera = XMMatrixMultiply(rotX, temp_camera);
			temp_camera = XMMatrixMultiply(temp_camera, rotY);

			XMStoreFloat4x4(&m_camera, temp_camera);

			m_camera._41 = pos.x;
			m_camera._42 = pos.y;
			m_camera._43 = pos.z;
		}
		m_prevMousePos = m_currMousePos;
	}

	//Spot Light Controls 
	if (m_kbuttons[VK_NUMPAD8])
	{
		// increases the Z axis of spotlight position
		spotPos.z += 1.0f;

	}
	if (m_kbuttons[VK_NUMPAD5])
	{
		//decreases the Z axis of the spotlight position
		spotPos.z -= 1.0f;

	}
	if (m_kbuttons[VK_NUMPAD4])
	{
		//decreases the x axis of the spotlight position
		spotPos.x -= 1.0f;

	}
	if (m_kbuttons[VK_NUMPAD6])
	{
		//increases the x axis of the spotlight position
		spotPos.x += 1.0f;

	}
	if (m_kbuttons[VK_NUMPAD7])
	{
		// decrease angle
		coneAng.z -= .1f;
	}
	if (m_kbuttons[VK_NUMPAD9])
	{
		// increase angle
		coneAng.z += .1f;
	}
	if (m_kbuttons['L'])
	{
		spotPos = { 0.0, 2.0f, 0.0f,1 };

	}
}

void Sample3DSceneRenderer::SetKeyboardButtons(const char* list)
{
	memcpy_s(m_kbuttons, sizeof(m_kbuttons), list, sizeof(m_kbuttons));
}

void Sample3DSceneRenderer::SetMousePosition(const Windows::UI::Input::PointerPoint^ pos)
{
	m_currMousePos = const_cast<Windows::UI::Input::PointerPoint^>(pos);
}

void Sample3DSceneRenderer::SetInputDeviceData(const char* kb, const Windows::UI::Input::PointerPoint^ pos)
{
	SetKeyboardButtons(kb);
	SetMousePosition(pos);
}

void DX11UWA::Sample3DSceneRenderer::StartTracking(void)
{
	m_tracking = true;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void Sample3DSceneRenderer::TrackingUpdate(float positionX)
{
	if (m_tracking)
	{
		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate(radians);
	}
}

void Sample3DSceneRenderer::StopTracking(void)
{
	m_tracking = false;
}

// Renders one frame using the vertex and pixel shaders.
void Sample3DSceneRenderer::Render(void)
{

	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));



	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(VertexPositionColor);
	UINT loadStride = sizeof(VERTEX);
	UINT offset = 0;


	//////////////////////////////////////////////////////////////
	//SkyBox from an OBJfile
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_skyBoxBufferData, 0, 0, 0);
	context->IASetVertexBuffers(0, 1, Skybox_vertexBuffer.GetAddressOf(), &loadStride, &offset);
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer(Skybox_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_skyBoxInputLayout.Get());
	// Attach our vertex shader.
	context->VSSetShader(Skybox_vertexShader.Get(), nullptr, 0);
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
	// Attach our pixel shader.
	context->PSSetShader(Skybox_pixelShader.Get(), nullptr, 0);
	context->PSSetShaderResources(0, 1, SkyBox_srv.GetAddressOf());
	context->PSSetSamplers(0, 1, sampState.GetAddressOf());
	// Draw the objects.
	context->DrawIndexed(skyBox_indexCount, 0, 0);
	//clear the Z-buffer
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//CUBE
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_inputLayout.Get());
	// Attach our vertex shader.
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
	// Attach our pixel shader.
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	// Draw the objects.
	context->DrawIndexed(m_indexCount, 0, 0);



	////////////////////////////////////////////////////////
	//render the pyramid
	context->UpdateSubresource1(instanceConstBuffer.Get(), 0, NULL, &m_instanceconstbufdata, 0, 0, 0);
	context->IASetVertexBuffers(0, 1, p_vertexBuffer.GetAddressOf(), &stride, &offset);
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer(p_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_inputLayout.Get());
	// Attach our vertex shader.
	context->VSSetShader(instancedvertexShader.Get(), nullptr, 0);
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, instanceConstBuffer.GetAddressOf(), nullptr, nullptr);
	// Attach our pixel shader.
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	// Draw the objects.
	context->DrawIndexedInstanced(p_indexCount, 3, 0, 0, 0);

	//////////////////////////////////////////////////////////////
	//Loaded obj file
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_loadedBufferData, 0, 0, 0);
	context->IASetVertexBuffers(0, 1, load_vertexBuffer.GetAddressOf(), &loadStride, &offset);
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer(load_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_loadedInputLayout.Get());
	// Attach our vertex shader.
	context->VSSetShader(loadedvertexShader.Get(), nullptr, 0);
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
	// Attach our pixel shader.
	context->PSSetShader(light_pixelShader.Get(), nullptr, 0);
	context->PSSetShaderResources(0, 1, Alientree_srv.GetAddressOf());
	context->PSSetSamplers(0, 1, sampState.GetAddressOf());
	// Draw the objects.
	context->DrawIndexed(load_indexCount, 0, 0);



	//////////////////////////////////////////////////////////////
	//WaterTower 
	//context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &waterTower_loadedBufferData, 0, 0, 0);
	//context->IASetVertexBuffers(0, 1, waterTower_vertexBuffer.GetAddressOf(), &loadStride, &offset);
	//// Each index is one 16-bit unsigned integer (short).
	//context->IASetIndexBuffer(waterTower_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//context->IASetInputLayout(m_loadedInputLayout.Get());
	//// Attach our vertex shader.
	//context->VSSetShader(loadedvertexShader.Get(), nullptr, 0);
	//// Send the constant buffer to the graphics device.
	//context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
	//// Attach our pixel shader.
	//context->PSSetShader(light_pixelShader.Get(), nullptr, 0);
	//context->PSSetShaderResources(0, 1, waterTower_srv.GetAddressOf());
	//// Draw the objects.
	//context->DrawIndexed(waterTower_indexCount, 0, 0);

	
	//the threading process here may break release mode, comment the next 8 lines out if you must run in relase mode.
	m_deviceResources->GetD3DDevice()->CreateDeferredContext(0, defCon.GetAddressOf());
	waterTower.threadComplete = false;
	auto thread = std::thread(&Sample3DSceneRenderer::setContextDraw, this, &waterTower, &waterTower.loadedbufdata);
	while (!waterTower.threadComplete);
	context->ExecuteCommandList(waterTower.commandList.Get(), true);
	waterTower.commandList.Reset();
	waterTower.commandList = nullptr;
	thread.join();

	//Floor
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_loadedBufferData, 0, 0, 0);
	context->IASetVertexBuffers(0, 1, floor_vertexBuffer.GetAddressOf(), &loadStride, &offset);
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer(floor_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_loadedInputLayout.Get());
	// Attach our vertex shader.
	context->VSSetShader(loadedvertexShader.Get(), nullptr, 0);
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
	// Attach our pixel shader.
	context->PSSetShader(light_pixelShader.Get(), nullptr, 0);
	context->PSSetShaderResources(0, 1, grass_srv.GetAddressOf());
	//lighting 
	context->PSSetConstantBuffers(0, 1, lightbuffer.GetAddressOf());
	// Draw the objects.
	context->DrawIndexed(floor_indexCount, 0, 0);




}

void Sample3DSceneRenderer::CreateDeviceDependentResources(void)
{
	// Load shaders asynchronously.
	CD3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc,sizeof(CD3D11_SAMPLER_DESC));

	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.MipLODBias = 0.0f;
	sampDesc.MaxAnisotropy = 1;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.BorderColor[0] = 1.0f;
	sampDesc.BorderColor[1] = 1.0f;
	sampDesc.BorderColor[2] = 1.0f;
	sampDesc.BorderColor[3] = 1.0f;
	sampDesc.MinLOD = -FLT_MAX;
	sampDesc.MaxLOD = FLT_MAX;

	m_deviceResources->GetD3DDevice()->CreateSamplerState(&sampDesc, sampState.GetAddressOf());

	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

	auto loadInstanceVStask = DX::ReadDataAsync(L"InstancedVertexShader.cso");

	auto loadModelVStask = DX::ReadDataAsync(L"LoadedVertexShader.cso");
	auto loadedModelPStask = DX::ReadDataAsync(L"LoadedPixelShader.cso");

	auto lightPStask = DX::ReadDataAsync(L"LightPixelShader.cso");


	auto skyBoxVStask = DX::ReadDataAsync(L"SkyBoxVertexShader.cso");
	auto skyBoxPStask = DX::ReadDataAsync(L"SkyBoxPixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &m_vertexShader));

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &m_inputLayout));
	});

	auto createLoadedModelVSTask = loadModelVStask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &loadedvertexShader));

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &m_loadedInputLayout));
	});

	auto createSkyBoxVSTask = skyBoxVStask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &Skybox_vertexShader));

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &m_skyBoxInputLayout));
	});

	auto creatInstanceVSTask = loadInstanceVStask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &instancedvertexShader));

	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &m_pixelShader));

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer));

		CD3D11_BUFFER_DESC instancedconstantBufferDesc(sizeof(ModelViewProjectionConstantBufferInstanced), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&instancedconstantBufferDesc, nullptr, &instanceConstBuffer));
	});

	auto createLoadedPSTask = loadedModelPStask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &m_loadedpixelShader));
	});

	//light_pixelShader
	auto createlightPSTask = lightPStask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &light_pixelShader));
		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(LightProperties), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &lightbuffer));
	});

	auto createSkyBoxPSTask = skyBoxPStask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &Skybox_pixelShader));
	});

#pragma region Cube
	// Once both shaders are loaded, create the mesh.
	auto createCubeTask = (createPSTask && createVSTask).then([this]()
	{
		// Load mesh vertices. Each vertex has a position and a color.
		static const VertexPositionColor cubeVertices[] =
		{
			{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer));

		// Load mesh indices. Each trio of indices represents
		// a triangle to be rendered on the screen.
		// For example: 0,2,1 means that the vertices with indexes
		// 0, 2 and 1 from the vertex buffer compose the 
		// first triangle of this mesh.
		static const unsigned short cubeIndices[] =
		{
			0,1,2, // -x
			1,3,2,

			4,6,5, // +x
			5,6,7,

			0,5,1, // -y
			0,4,5,

			2,7,6, // +y
			2,3,7,

			0,6,4, // -z
			0,2,6,

			1,7,3, // +z
			1,5,7,
		};

		m_indexCount = ARRAYSIZE(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer));
	});

	// Once the cube is loaded, the object is ready to be rendered.
	createCubeTask.then([this]()
	{
		m_loadingComplete = true;
	});
#pragma endregion

#pragma region Pyramid
	auto createPyramidTask = (createPSTask && createVSTask).then([this]()
	{
		static const VertexPositionColor pyramidVertices[] =
		{
			{ XMFLOAT3(0.0f,  -1.0f, 0.0f),  XMFLOAT3(0.0f, 0.0f, 0.0f) },  //E
			{ XMFLOAT3(-0.5f,  0.0f, -0.5f),  XMFLOAT3(0.0f, 0.0f, 1.0f) },  //A
			{ XMFLOAT3(0.5f,  0.0f, -0.5f),  XMFLOAT3(0.0f, 1.0f, 0.0f) },  //B
			{ XMFLOAT3(0.5f,  0.0f,  0.5f),  XMFLOAT3(0.0f, 1.0f, 1.0f) },  //C
			{ XMFLOAT3(-0.5f,  0.0f,  0.5f),  XMFLOAT3(1.0f, 0.0f, 0.0f) },  //D
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = pyramidVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(pyramidVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &p_vertexBuffer));

		static const unsigned short pyramidIndices[] =
		{
		2,1,4,
		4,3,2,
		1,2,0,
		2,3,0,
		3,4,0,
		4,1,0
		};

		p_indexCount = ARRAYSIZE(pyramidIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = pyramidIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(pyramidIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &p_indexBuffer));
	});

	createPyramidTask.then([this]()
	{
		m_loadingComplete = true;
	});

#pragma endregion

#pragma region AlienTree

	auto createAlienTreeTask = (createPSTask && createVSTask).then([this]()
	{
		vector<VERTEX> testModelVerts;
		vector<unsigned int> testModelIndices;
		ModelLoader mloader;

		mloader.loadModel("Assets/Alientree.obj", testModelVerts, testModelIndices);

		D3D11_SUBRESOURCE_DATA vertexBufferData;
		vertexBufferData.pSysMem = testModelVerts.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(unsigned int(sizeof(VERTEX)*testModelVerts.size()), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &load_vertexBuffer));

		load_indexCount = unsigned int(testModelIndices.size());

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = testModelIndices.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(unsigned int(sizeof(unsigned int) * testModelIndices.size()), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &load_indexBuffer));
	});

	CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/AlienTree.dds", nullptr, &Alientree_srv);

	createAlienTreeTask.then([this]()
	{
		m_loadingComplete = true;
	});

#pragma endregion

#pragma region Skybox

	auto createSkyBoxTask = (createPSTask && createVSTask).then([this]()
	{
		vector<VERTEX> SkyBoxVerts;
		vector<unsigned int> SkyBoxIndices;
		ModelLoader mloader;

		mloader.loadModel("Assets/SkyboxCube.obj", SkyBoxVerts, SkyBoxIndices);

		D3D11_SUBRESOURCE_DATA vertexBufferData;
		vertexBufferData.pSysMem = SkyBoxVerts.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(unsigned int(sizeof(VERTEX)*SkyBoxVerts.size()), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &Skybox_vertexBuffer));

		skyBox_indexCount = unsigned int(SkyBoxIndices.size());

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = SkyBoxIndices.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(unsigned int(sizeof(unsigned int) * SkyBoxIndices.size()), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &Skybox_indexBuffer));
	});

	CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/OutputCube.dds", nullptr, &SkyBox_srv);

	createSkyBoxTask.then([this]()
	{
		m_loadingComplete = true;
	});


#pragma endregion

#pragma region Floor
	auto createFloorTask = (createPSTask && createVSTask).then([this]()
	{
		vector<VERTEX> testModelVerts;
		vector<unsigned int> testModelIndices;
		ModelLoader mloader;

		mloader.loadModel("Assets/FloorPlane.obj", testModelVerts, testModelIndices);

		D3D11_SUBRESOURCE_DATA vertexBufferData;
		vertexBufferData.pSysMem = testModelVerts.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(unsigned int(sizeof(VERTEX)*testModelVerts.size()), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &floor_vertexBuffer));

		floor_indexCount = unsigned int(testModelIndices.size());

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = testModelIndices.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(unsigned int(sizeof(unsigned int) * testModelIndices.size()), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &floor_indexBuffer));
	});

	CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/grass_seamless.dds", nullptr, &grass_srv);

	// Once the cube is loaded, the object is ready to be rendered.
	createFloorTask.then([this]()
	{
		m_loadingComplete = true;
	});
#pragma endregion

#pragma region WaterTower

	auto createWaterTowerTask = (createPSTask && createVSTask).then([this]()
	{
		vector<VERTEX> ModelVerts;
		vector<unsigned int> ModelIndices;
		ModelLoader mloader;
		mloader.loadModel("Assets/WaterTower.obj", ModelVerts, ModelIndices); 

		D3D11_SUBRESOURCE_DATA vertexBufferData;
		vertexBufferData.pSysMem = ModelVerts.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(unsigned int(sizeof(VERTEX)*ModelVerts.size()), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &waterTower_vertexBuffer));

		waterTower_indexCount = unsigned int(ModelIndices.size());

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = ModelIndices.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(unsigned int(sizeof(unsigned int) * ModelIndices.size()), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &waterTower_indexBuffer));

		CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/watertower_diffuse.dds", nullptr, &waterTower_srv);
	
	//set up the model struct for deferred context
		waterTower.indexBuffer = waterTower_indexBuffer;
		waterTower.vertexBuffer = waterTower_vertexBuffer;
		waterTower.indexCount = waterTower_indexCount;
		waterTower.srv = waterTower_srv;
		waterTower.inputLayout = m_loadedInputLayout;
		waterTower.ps_shader = light_pixelShader;
		waterTower.vs_shader = loadedvertexShader;
		waterTower.constantBuffer = m_constantBuffer;
		waterTower.loadedbufdata = waterTower_loadedBufferData;
	
	});


	createWaterTowerTask.then([this]()
	{
		m_loadingComplete = true;
	});

#pragma endregion 
}

void Sample3DSceneRenderer::ReleaseDeviceDependentResources(void)
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
	p_indexBuffer.Reset();
	p_vertexBuffer.Reset();
	instanceConstBuffer.Reset();
	load_vertexBuffer.Reset();
	load_indexBuffer.Reset();
	Skybox_vertexBuffer.Reset();
	Skybox_indexBuffer.Reset();
	floor_indexBuffer.Reset();
	floor_vertexBuffer.Reset();

}

void Sample3DSceneRenderer::setContextDraw(MODEL * model, ModelViewProjectionConstantBuffer * data)
{
	UINT loadStride = sizeof(VERTEX);
	UINT offset = 0;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv = m_deviceResources->GetBackBufferRenderTargetView();
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsv = m_deviceResources->GetDepthStencilView();

	defCon->RSSetViewports(1, &m_deviceResources->GetScreenViewport());
	defCon->OMSetRenderTargets(1, rtv.GetAddressOf(), dsv.Get());

	defCon->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
	defCon->PSSetConstantBuffers(0, 1, lightbuffer.GetAddressOf());

	defCon->UpdateSubresource(m_constantBuffer.Get(), 0, NULL, data, 0, 0);
	defCon->IASetVertexBuffers(0, 1, model->vertexBuffer.GetAddressOf(), &loadStride, &offset);
	defCon->IASetIndexBuffer(model->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	defCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	defCon->IASetInputLayout(model->inputLayout.Get());
	defCon->VSSetShader(model->vs_shader.Get(), nullptr, 0);
	defCon->PSSetShader(model->ps_shader.Get(), nullptr, 0);
	defCon->PSSetShaderResources(0, 1, model->srv.GetAddressOf());
	defCon->PSSetSamplers(0, 1, sampState.GetAddressOf());
	defCon->DrawIndexed(waterTower_indexCount, 0, 0);
	defCon->FinishCommandList(true,&model->commandList);

	model->threadComplete = true;

}