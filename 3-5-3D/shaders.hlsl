struct VSInput
{
	float4 position : POSITION;
	float4 color : COLOR;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

cbuffer Scene
{
	float4x4 wvp;
};

PSInput VS(VSInput input)
{
	PSInput output;

	output.position = mul(input.position, wvp);
	output.color = input.color;

	return output;
}

float4 PS(PSInput input) : SV_TARGET
{
	return input.color;
}