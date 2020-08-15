cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
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

struct DS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 controlPoint : TEXCOORD0;
    float3 color : COLOR0;
};

struct HS_CONTROL_POINT_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 controlPoint : TEXCOORD0;
    float3 color : COLOR0;
};

struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[4] : SV_TessFactor; 
    float InsideTessFactor[2] : SV_InsideTessFactor;
};

float getInterpolatedHeght(float2 controlPos)
{
    float4 uVec = float4(1.0, controlPos.x, pow(controlPos.x, 2.0), pow(controlPos.x, 3.0));
    float4 vVec = float4(1.0, controlPos.y, pow(controlPos.y, 2.0), pow(controlPos.y, 3.0));
    float4 row = mul(mul(mul(uVec, bezierCoeficients), controlPoints), transposedBezierCoeficients);
    return dot(row, vVec);
}

float4 getInterpolatedPoint(float2 controlPos)
{
    float height = getInterpolatedHeght(controlPos);
    controlPos -= 0.5f;
    float4 pos = float4(controlPos.x, height, controlPos.y, 1.0);
    pos = mul(pos, model);
    pos = mul(pos, view);
    pos = mul(pos, projection);
    return pos;
}

#define NUM_CONTROL_POINTS 4

[domain("quad")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float2 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
    DS_OUTPUT Output;

    Output.controlPoint = lerp(lerp(patch[0].controlPoint, patch[1].controlPoint, domain.x), lerp(patch[2].controlPoint, patch[3].controlPoint, domain.x), domain.y);
    Output.pos = getInterpolatedPoint(Output.controlPoint);
    Output.color = lerp(lerp(patch[0].color, patch[1].color, domain.x), lerp(patch[2].color, patch[3].color, domain.x), domain.y);

    return Output;
}