struct PixelShaderInput
{
	float4 pos : SV_POSITION;
    float3 worldPos : POSITION0;
    float2 controlPoint : TEXCOORD0;
	float3 color : COLOR0;
};

static const float3 ambient = float3(0.202f, 0.233f, 0.292f);
static const float3 lightPos = float3(1.0, 0.3, 0.8);

float4 main(PixelShaderInput input) : SV_TARGET
{
    float3 dFdxPos = ddx(input.worldPos);
    float3 dFdyPos = ddy(input.worldPos);
    float3 normal = normalize(cross(dFdyPos, dFdxPos));
    
    float diffuse = saturate(dot(normal, lightPos));
    float4 shadedColor = float4(saturate((input.color.rgb * diffuse) + (input.color.rgb * ambient)), 1.0);
    return shadedColor;
}
