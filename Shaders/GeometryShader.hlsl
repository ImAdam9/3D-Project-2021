cbuffer CameraPositionBuffer : register(b0)
{
	float3 cameraPosition;
};

struct GSInput
{
	float4 wvp_pos     : SV_POSITION;
	float2 uv		   : TEX;
	float4 w_pos       : WPOS;
	float3 normal      : NOR;
	float3 TangentWS   : TAN;
	float3 BitangentWS : BITAN;
    float4 clipSpace   : CLIPSPACE;
};

struct GSOutput
{
	float4 wvp_pos     : SV_POSITION;
	float2 uv          : TEX;
	float4 w_pos       : WPOS;
	float3 normal      : NOR;
	float3 TangentWS   : TAN;
	float3 BitangentWS : BITAN;
    float4 clipSpace   : CLIPSPACE;
};

[maxvertexcount(3)]
void main(
	triangle GSInput input[3] : SV_POSITION,
	inout TriangleStream< GSOutput > output
)
{
	GSOutput element;
	// Face Normal
	float3 faceEdge1 = input[1].w_pos.xyz - input[0].w_pos.xyz;
	float3 faceEdge2 = input[2].w_pos.xyz - input[0].w_pos.xyz;
	float3 faceNormal = normalize(cross(faceEdge1, faceEdge2));
	
	// Vector from camera position to triangle
	float3 c_to_t = normalize(cameraPosition.xyz - input[0].w_pos.xyz);
	float dotProd = dot(c_to_t, faceNormal);

	if (dotProd > 0.0f)
	{
		for (uint i = 0; i < 3; i++)
		{
			element.wvp_pos		= input[i].wvp_pos;
			element.uv			= input[i].uv;
			element.w_pos		= input[i].w_pos;
			element.normal      = input[i].normal;
			element.TangentWS   = input[i].TangentWS;
			element.BitangentWS = input[i].BitangentWS;
            element.clipSpace   = input[i].clipSpace;
			output.Append(element);
		}
	}
	
	output.RestartStrip();
}