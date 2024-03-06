cbuffer LightWVPBuffer : register(b0)
{
    float4x4 LightWorld;
    float4x4 LightView;
    float4x4 LightProjection;
};

struct vertexInput
{
	float3 pos : POS;
};

struct vertexOutput
{
	float4 pos : SV_POSITION;
};

vertexOutput shadow_main(vertexInput input)
{
	vertexOutput output;
    output.pos = mul(float4(input.pos, 1.0f), mul(LightWorld, mul(LightView, LightProjection)));
	return output;
}