cbuffer objectBuffer : register(b0)
{
	float4x4 world;
};

cbuffer cameraBuffer : register(b1)
{
	float4x4 view;
	float4x4 projection;
};

cbuffer LightWVPBuffer : register(b2)
{
    float4x4 LightWorld;
    float4x4 LightView;
    float4x4 LightProjection;
};

struct vs_in
{
	float3 position : POS;
	float3 normal   : NOR;
	float2 uv       : TEX;
	float4 tangent  : TAN;
};

struct vs_out
{
	float4 wvp_pos     : SV_POSITION;
	float2 uv		   : TEX;
	float4 w_pos       : WPOS;
	float3 normal      : NOR;
	float3 TangentWS   : TAN;
	float3 BitangentWS : BITAN;
    float4 clipSpace   : CLIPSPACE;
};

vs_out vs_main(vs_in input) 
{
	vs_out output   = (vs_out)0;
	output.wvp_pos  = mul(float4(input.position, 1.0f), mul(world, mul(view, projection)));
    output.uv		= input.uv;
	output.w_pos    = mul(float4(input.position, 1.0f), world);
	float3 normalWS = normalize(mul(input.normal, (float3x3)world));
	output.normal   = normalWS;

	float3 tangentWS = normalize(mul(input.tangent.xyz, (float3x3)world));
	float3 bitangentWS = normalize(cross(normalWS, tangentWS)) * input.tangent.w;
	output.TangentWS = tangentWS;
	output.BitangentWS = bitangentWS;

    output.clipSpace = mul(float4(input.position, 1.0f), mul(world, mul(LightView, LightProjection)));

	return output;
}