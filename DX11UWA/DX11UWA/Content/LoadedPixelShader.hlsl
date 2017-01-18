// Per-pixel color data passed through the pixel shader.
Texture2D baseTexture : register(t0);
SamplerState filters[2] : register(s0);

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 uv : UV;
	float3 normal : NORMAL;
};

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	//return float4(input.uv, 1.0f);
	float4 baseColor = baseTexture.Sample(filters[0], input.uv.xy);
	if (baseColor.a < 1.0)
	{
		discard;
	}
	return baseColor;
}
