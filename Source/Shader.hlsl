struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PSInput VSMain(float3 position : POSITION, float4 color : COLOR)
{
	PSInput interpolants;
	
	interpolants.position = float4(position, 1.0f);
	interpolants.color = color;

	return interpolants;
}

float4 PSMain(PSInput input) : SV_Target
{
	return input.color;
}
