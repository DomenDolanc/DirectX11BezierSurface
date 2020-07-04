struct DS_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
};

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[4] : SV_TessFactor; // e.g. would be [4] for a quad domain
    float InsideTessFactor[2] : SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
	// TODO: change/add other stuff
};

#define NUM_CONTROL_POINTS 4

[domain("quad")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float2 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
	DS_OUTPUT Output;

    Output.pos = lerp(lerp(patch[0].pos, patch[1].pos, domain.x), lerp(patch[2].pos, patch[3].pos, domain.x), domain.y);
    Output.color = lerp(lerp(patch[0].color, patch[1].color, domain.x), lerp(patch[2].color, patch[3].color, domain.x), domain.y);

	return Output;
}
