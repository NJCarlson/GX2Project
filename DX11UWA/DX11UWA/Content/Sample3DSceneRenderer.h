#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include "Common\DDSTextureLoader.h"
#include <DirectXColors.h>
#include <DirectXMath.h>

using namespace DirectX;

namespace DX11UWA
{
	// This sample renderer instantiates a basic rendering pipeline.
	class Sample3DSceneRenderer
	{
	public:
	
		

		Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources(void);
		void CreateWindowSizeDependentResources(void);
		void ReleaseDeviceDependentResources(void);
		void Update(DX::StepTimer const& timer);
		void Render(void);
		void StartTracking(void);
		void TrackingUpdate(float positionX);
		void StopTracking(void);
		inline bool IsTracking(void) { return m_tracking; }

		// Helper functions for keyboard and mouse input
		void SetKeyboardButtons(const char* list);
		void SetMousePosition(const Windows::UI::Input::PointerPoint^ pos);
		void SetInputDeviceData(const char* kb, const Windows::UI::Input::PointerPoint^ pos);


	private:
		void Rotate(float radians);
		void UpdateCamera(DX::StepTimer const& timer, float const moveSpd, float const rotSpd);

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Direct3D resources for cube geometry.
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;
		// System resources for cube geometry.
		ModelViewProjectionConstantBuffer	m_constantBufferData;
		ModelViewProjectionConstantBufferInstanced m_instanceconstbufdata;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	instancedvertexShader;
		uint32	m_indexCount;



		//Floor resources
		Microsoft::WRL::ComPtr<ID3D11Buffer>		floor_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		floor_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> grass_srv;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> light_pixelShader;

		uint32	floor_indexCount;
		ModelViewProjectionConstantBuffer floor_BufData;

		//lighting
		struct Light
		{
			XMFLOAT4      Position;
			XMFLOAT4      Direction;
			float		radius;
			XMFLOAT3	rad_padding;
			XMFLOAT4      Color;
			float       SpotAngle;
			float       ConstantAttenuation;
			float       LinearAttenuation;
			float       QuadraticAttenuation;
			int         LightType;
			XMFLOAT3	type_padding;
			bool        Enabled;
			XMFLOAT3      coneRatio; // x = inner ratio,  y = outerratio,  z = angle
			//XMINT2		padding;

		};

		struct LightProperties 
		{
			XMFLOAT4 EyePosition;
			XMFLOAT4 GlobalAmbient;
			Light  Lights[3];
		};
		LightProperties m_LightProperties;

		XMVECTORF32 LightColors[3] = 
		{
			 Colors::DarkOliveGreen, Colors::Yellow, Colors::Indigo
		};

		 bool LightEnabled[3] = 
		{
			true, true, true
		};

		 int numLights = 3;
		 float radius = 8.0f;
		 float offset = 2.0f * XM_PI / numLights;
		 Microsoft::WRL::ComPtr<ID3D11Buffer> lightbuffer;


		//Pyramid  resources
		Microsoft::WRL::ComPtr<ID3D11Buffer>		p_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		p_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		instanceConstBuffer;

		uint32	p_indexCount;
		uint32  Instanceindexcount;

		//Model Loading
		Microsoft::WRL::ComPtr<ID3D11Buffer>		load_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		load_indexBuffer;
		uint32 load_indexCount;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_loadedInputLayout;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	loadedvertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_loadedpixelShader;
		ModelViewProjectionConstantBuffer			m_loadedBufferData;

		//Texture Variables
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Alientree_srv;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SkyBox_srv;

		//SkyBox Variables
		Microsoft::WRL::ComPtr<ID3D11Buffer>		Skybox_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		Skybox_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	Skybox_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	Skybox_pixelShader;
		uint32	skyBox_indexCount;
		ModelViewProjectionConstantBuffer			m_skyBoxBufferData;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_skyBoxInputLayout;

		

		// Variables used with the rendering loop.
		bool	m_loadingComplete;
		float	m_degreesPerSecond;
		bool	m_tracking;

		// Data members for keyboard and mouse input
		char	m_kbuttons[256];
		Windows::UI::Input::PointerPoint^ m_currMousePos;
		Windows::UI::Input::PointerPoint^ m_prevMousePos;

		// Matrix data member for the camera
		DirectX::XMFLOAT4X4 m_camera;
	};
}

