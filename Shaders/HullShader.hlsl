cbuffer tessFactor : register(b0)
{
    float tessFx;
    float tessFy;
    float tessFz;
    float tessFw;
};

struct VSinput
{
    float4 wvp_pos : SV_POSITION;
    float2 uv : TEX;
    float4 w_pos : WPOS;
    float3 normal : NOR;
    float3 TangentWS : TAN;
    float3 BitangentWS : BITAN;
    float4 clipSpace : CLIPSPACE;
};

struct HullShaderOutput
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

HS_CONSTANT_DATA_OUTPUT ConstantPatchFunction(InputPatch<VSinput, 3> inputPatch, uint patchID : SV_PrimitiveID)
{
    HS_CONSTANT_DATA_OUTPUT output;
    
    output.EdgeTessFactor[0] = 0.5f*(tessFx + tessFy);
    output.EdgeTessFactor[1] = 0.5f*(tessFy + tessFz);
    output.EdgeTessFactor[2] = 0.5f*(tessFz + tessFx);
    output.InsideTessFactor = output.EdgeTessFactor[0];
    
    return output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantPatchFunction")]
HullShaderOutput PerPatchFunction(InputPatch<VSinput, 3> inputPatch, uint i : SV_OutputControlPointID, uint patchID : SV_PrimitiveID)
{
    HullShaderOutput output;
    
    output.wvp_pos = inputPatch[i].wvp_pos;
    output.uv = inputPatch[i].uv;
    output.w_pos = inputPatch[i].w_pos;
    output.normal = inputPatch[i].normal;
    output.TangentWS = inputPatch[i].TangentWS;
    output.BitangentWS = inputPatch[i].BitangentWS;
    output.clipSpace = inputPatch[i].clipSpace;

    return output;
}
