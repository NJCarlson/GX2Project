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

	struct MODEL
	{
		Microsoft::WRL::ComPtr<ID3D11Buffer>		constantBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
		uint32 indexCount;
		ModelViewProjectionConstantBuffer loadedbufdata;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> vs_shader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> ps_shader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
		Microsoft::WRL::ComPtr<ID3D11CommandList> commandList;
		bool threadComplete;
	};

	class Sample3DSceneRenderer
	{
	public:
		void setContextDraw(MODEL * model,ModelViewProjectionConstantBuffer * data);

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

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampState;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> defCon;

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
			XMFLOAT4    Position;    //16
			XMFLOAT4    Direction;   //16
			XMFLOAT4    radius;    
			XMFLOAT4    Color; //16

			XMFLOAT4    AttenuationData; 
		// x =  SpotAngle;
		// y =  ConstantAttenuation;
		// z =  LinearAttenuation;
		// w =  QuadraticAttenuation; 

			XMFLOAT4 LightTypeEnabled;
			// x  = type
			// y = Enabled;
			
			XMFLOAT4   ConeRatio; // x = inner ratio,  y = outerratio
		
			XMFLOAT4    coneAngle;
			
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
			// Directional light;      Point Light;    Spot Light;
			 Colors::DarkOliveGreen, Colors::Yellow, Colors::DarkOrange
		};

		 bool LightEnabled[3] = 
		{
			true, true, true
		};

		 //Dynamic Variables
		 bool dirLightSwitch = false; // false = negative direction true = positive direction
		 bool pointLightSwitch = false; // false = negative direction true = positive direction
		 float spotRad = 10.0f;
		 float innerConeRat = .8f;
		 float outterConeRat = .45f;
		 XMFLOAT4 coneAng = { 0,-1.0f, -1.0f, 0 };

		 XMFLOAT4 pointLightPos = { 2.0f,1.0f, 5.0f,1 };
		 XMFLOAT4 directionalLightPos = { 2.0f,1.0f, 5.0f,1 };
		 XMFLOAT4 spotPos = { 0.0, 2.0f, 0.0f,1 };

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

		// Water tower variables 
		Microsoft::WRL::ComPtr<ID3D11Buffer>		waterTower_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		waterTower_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> waterTower_srv;
		uint32 waterTower_indexCount;
		ModelViewProjectionConstantBuffer			waterTower_loadedBufferData;
		MODEL waterTower;

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

