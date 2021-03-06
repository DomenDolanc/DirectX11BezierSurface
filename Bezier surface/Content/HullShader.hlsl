cbuffer CalculationConstantBuffer : register(b0)
{
    matrix bezierCoeficients;
    matrix transposedBezierCoeficients;
    matrix controlPoints;
    float4 color;
    float tessellationFactor;
    float3 padding;
};

struct VS_CONTROL_POINT_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION0;
    float2 controlPoint : TEXCOORD0;
    float3 color : COLOR0;
};

struct HS_CONTROL_POINT_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION0;
    float2 controlPoint : TEXCOORD0;
    float3 color : COLOR0;
};

struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[4] : SV_TessFactor; 
    float InsideTessFactor[2] : SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 4

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;

	Output.EdgeTessFactor[0] = 
		Output.EdgeTessFactor[1] = 
		Output.EdgeTessFactor[2] = 
		Output.EdgeTessFactor[3] = 
		Output.InsideTessFactor[0] = 
		Output.InsideTessFactor[1] = tessellationFactor;

	return Output;
}

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_CONTROL_POINT_OUTPUT main( 
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip, 
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	HS_CONTROL_POINT_OUTPUT Output;

	Output.pos = ip[i].pos;
    Output.worldPos = ip[i].worldPos;
	Output.controlPoint = ip[i].controlPoint;
	Output.color = ip[i].color;

	return Output;
}
