cbuffer viewProjBuffer : register(b0)
{
    float4x4 view;
    float4x4 projection;
};

cbuffer tessFactor : register(b1)
{
    float tessFx;
    float tessFy;
    float tessFz;
    float tessFw;
};

cbuffer uvOffsetBuffer : register(b2)
{
    float2 uvOffset;
    float animate;
};

struct DomainShaderOutput
{
    float4 wvp_pos : SV_POSITION;
    float2 uv : TEX;
    float4 w_pos : WPOS;
    float3 normal : NOR;
    float3 TangentWS : TAN;
    float3 BitangentWS : BITAN;
    float4 clipSpace : CLIPSPACE;
};

struct HullShaderInput
{
    float4 wvp_pos : SV_POSITION;
    float2 uv : TEX;
    float4 w_pos : WPOS;
    float3 normal : NOR;
    float3 TangentWS : TAN;
    float3 BitangentWS : BITAN;
    float4 clipSpace : CLIPSPACE;
};

struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[3] : SV_TessFactor;
    float InsideTessFactor : SV_InsideTessFactor;
};

Texture2D displacementMap : register(t0);
SamplerState wrapSampler : register(s0);

[domain("tri")]
DomainShaderOutput main(HS_CONSTANT_DATA_OUTPUT input, float3 uvw : SV_DomainLocation,
    const OutputPatch<HullShaderInput, 3> patch)
{
    DomainShaderOutput output;
    
    output.wvp_pos = float4(0.0f, 0.0f, 0.0f, 0.0f);
    output.uv = float2(0.0f, 0.0f);
    output.w_pos = float4(0.0f, 0.0f, 0.0f, 0.0f);
    output.normal = float3(0.0f, 0.0f, 0.0f);
    output.TangentWS = float3(0.0f, 0.0f, 0.0f);
    output.BitangentWS = float3(0.0f, 0.0f, 0.0f);
    output.clipSpace = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float domain[3] = { uvw.x, uvw.y, uvw.z };

    for (int i = 0; i < 3; i++)
    {
        output.wvp_pos += patch[i].wvp_pos * domain[i];
        output.uv += patch[i].uv * domain[i];
        output.w_pos += patch[i].w_pos * domain[i];
        output.normal += patch[i].normal * domain[i];
        output.TangentWS += patch[i].TangentWS * domain[i];
        output.BitangentWS += patch[i].BitangentWS * domain[i];
        output.clipSpace += patch[i].clipSpace * domain[i];
    }
    output.normal = normalize(output.normal);
    
    float3 displacement;
    if (animate == 1.f)
        displacement = displacementMap.SampleLevel(wrapSampler, output.uv + uvOffset, 6).rgb;
    else
        displacement = displacementMap.SampleLevel(wrapSampler, output.uv, 6).rgb;
    float height = (displacement.x + displacement.y + displacement.z) / 3.f;
    output.w_pos.xyz += (output.normal * tessFw * (height - 1.f));
    output.wvp_pos = mul(output.w_pos, mul(view, projection));

    return output;
}
