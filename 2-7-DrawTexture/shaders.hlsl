struct VSInput
{
	float4 position : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

Texture2D tex;
SamplerState smp;

PSInput VS(VSInput input)
{
	PSInput output;

	output.position = input.position;
	output.color = input.color;
	output.uv = input.uv;

	return output;
}

float4 PS(PSInput input) : SV_TARGET
{
	return tex.Sample(smp, input.uv) * input.color;
}