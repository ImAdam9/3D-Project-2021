struct vs_in
{
	float3 position : POS;
	float3 normal   : NOR;
	float2 uv       : TEX;
	float4 tangent  : TAN;
};

struct vs_out
{
	float4 wvp_pos : SV_POSITION;
	float2 uv      : TEX;
};

vs_out vs_main(vs_in input)
{
	vs_out output;
	output.wvp_pos   = float4(input.position, 1.0f);
	output.uv = input.uv;
	
	return output;
}