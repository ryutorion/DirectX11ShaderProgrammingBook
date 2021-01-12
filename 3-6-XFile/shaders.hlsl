struct VSInput
{
	float4 position : POSITION;
	float2 uv : TEXCOORD;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

Texture2D tex;
SamplerState smp;

cbuffer Scene
{
	float4x4 wvp;
};

PSInput VS(VSInput input)
{
	PSInput output;

	output.position = mul(input.position, wvp);
	output.uv = input.uv;

	return output;
}

float4 PS(PSInput input) : SV_TARGET
{
	return tex.Sample(smp, input.uv);
}