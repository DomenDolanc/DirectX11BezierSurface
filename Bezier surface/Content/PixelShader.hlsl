cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    float4 eye;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
    float3 worldPos : POSITION0;
    float2 controlPoint : TEXCOORD0;
	float3 color : COLOR0;
};

static const float3 ambientFactor = float3(0.202f, 0.233f, 0.292f);
static const float3 lightDir = float3(1.0, 0.3, 0.8);

float4 main(PixelShaderInput input) : SV_TARGET
{
    float3 dFdxPos = ddx(input.worldPos);
    float3 dFdyPos = ddy(input.worldPos);
    float3 normal = normalize(cross(dFdyPos, dFdxPos));
    float3 viewDir = normalize(eye.xyz - input.worldPos);
    
    float3 color = ambientFactor;
    float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float lightIntensity = saturate(dot(normal, lightDir));
	
    if (lightIntensity > 0.0f)
    {
        color += (input.color.rgb * lightIntensity);
        color = saturate(color);
        float3 reflection = normalize(2 * lightIntensity * normal - lightDir);
        specular = pow(saturate(dot(reflection, viewDir)), 8.0f);
    }
    return saturate(float4(color, 1.0f) + specular);
}
