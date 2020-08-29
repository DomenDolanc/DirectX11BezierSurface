cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
    float4 eye;
};

cbuffer CalculationConstantBuffer : register(b1)
{
    matrix bezierCoeficients;
    matrix transposedBezierCoeficients;
    matrix controlPoints;
    float4 color;
    float tessellationFactor;
    float3 padding;
};

struct VertexShaderInput
{
	float3 pos : POSITION;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
    float3 worldPos : POSITION0;
    float2 controlPoint : TEXCOORD0;
	float3 color : COLOR0;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);
	
	pos = mul(pos, model);
    output.worldPos = pos.xyz;
	
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;
    output.controlPoint = input.pos.xz + 0.5f;
    output.color = color.rgb;

	return output;
}
