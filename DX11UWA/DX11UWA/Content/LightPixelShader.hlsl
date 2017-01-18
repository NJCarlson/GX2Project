Texture2D baseTexture : register(t0);
sampler filters : register(s0);

// Per-pixel color data passed through the pixel shader.
struct Light
{
	float4      Position;          		
	float4      Direction;    
	float		radius;
    float3      rad_padding;
	float4      Color;               
	float       SpotAngle;           
	float       ConstantAttenuation; 
	float       LinearAttenuation;   
	float       QuadraticAttenuation;
	int         LightType;        
    float3      type_padding;
	bool        Enabled;             
	float3      coneRatio; // x = inner ratio,  y = outerratio,  z = angle
	//int2		padding;
									
};       

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 uv : UV;
    float3 normal : NORMAL;
    float3 world_pos : WORLDPOS;
};

cbuffer LightProperties : register(b0)
{
	float4 EyePosition;                 
	float4 GlobalAmbient;               
	Light Lights[3];           
};                       

float DoAttenuation(Light light, PixelShaderInput input)
{
	return 1.0f - saturate(length(light.Position.xyz - input.world_pos.xyz) / light.radius);
}

float DoSpotlightAttenuation(Light light, PixelShaderInput input)
{
	float ans;
    float lightdir = normalize(light.Position.xyz - input.world_pos.xyz);
	float surfaceratio = saturate(dot(lightdir,light.coneRatio.z));
	ans = 1 - saturate((light.coneRatio.x - surfaceratio)/(light.coneRatio.x - light.coneRatio.y));
	return ans;
}

float4 DoPointLight(Light light, PixelShaderInput input)
{
	float4 result;
    float3 lightdir = normalize(light.Position.xyz - input.world_pos.xyz);
	float lightratio = saturate(dot(lightdir,input.normal));

	result = lightratio * light.Color;
	result = result * DoAttenuation(light, input);
	return result;
}

float4 DoDirectionalLight(Light light, PixelShaderInput input)
{
	
	float4 result;
	float3 lightdir = normalize(light.Direction);
	float lightratio = saturate(dot(-lightdir, input.normal));
	result = lightratio * light.Color;

	return result;
}

float4 DoSpotLight(Light light, PixelShaderInput input)
{
	float4 result;
	
    float lightdir = normalize(light.Position.xyz - -input.world_pos.xyz);
	float surfaceratio = saturate(dot(-lightdir, light.coneRatio.z));
	float spotFactor = (surfaceratio > light.coneRatio.xy) ? 1 : 0;
	float lightratio = saturate(dot(lightdir, input.normal));

	result = spotFactor * lightratio * light.Color;
	result = result * DoAttenuation(light, input);
	result = result * DoSpotlightAttenuation(light, input);
	return result;
}

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 baseColor = baseTexture.Sample(filters, input.uv.xy);
    float4 lightdata = float4(0.0f, 0.0f, 0.0f, 0.0f);



   baseColor *= DoDirectionalLight(Lights[0], input);
   baseColor += DoPointLight(Lights[1], input);
  // baseColor += DoSpotLight(Lights[2], input);
   
	return baseColor;
}
