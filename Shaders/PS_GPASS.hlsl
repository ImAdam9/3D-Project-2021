cbuffer lightBuffer : register(b0)
{
    float3 lightPos;
    float ambientStrength;
    float3 lightDiffuse;
    float lightStrength;
    float3 lightDirection;
    float spotAngle;
    float attenuation_a;
    float attenuation_b;
    float attenuation_c;
};

cbuffer emissiveBuffer : register(b1)
{
    float3 emissiveIntensity;
    float emissivePower;
    bool isEmissive;
};

cbuffer uvOffsetBuffer : register(b2)
{
    float2 uvOffset;
    float animate;
};

struct ps_in
{
	float4 wvp_pos	   : SV_POSITION;
	float2 uv		   : TEX;
	float4 w_pos	   : WPOS;
	float3 normal	   : NOR;
	float3 TangentWS   : TAN;
	float3 BitangentWS : BITAN;
    float4 clipSpace   : CLIPSPACE;
};

struct ps_out
{
	float4 color    : SV_Target0;
	float4 position : SV_Target1;
	float4 normal   : SV_Target2;
    float4 emissive : SV_Target3;
};
Texture2D map_Kd       : register(t0); 
Texture2D map_bump     : register(t1);
Texture2D depthMap     : register(t2);
Texture2D emissiveMap  : register(t3);

SamplerState wrapSampler    : register(s0);
SamplerState boarderSampler : register(s1);
ps_out ps_main(ps_in input)
{
	ps_out output;
    float3 diffuseAlbedo;
    float3 normalTS;
    
    if (animate == 1.f)
    {
        diffuseAlbedo = map_Kd.Sample(wrapSampler, input.uv + uvOffset).rgb;
        normalTS = map_bump.Sample(wrapSampler, input.uv + uvOffset).rgb;
    }
    else
    {
        diffuseAlbedo = map_Kd.Sample(wrapSampler, input.uv).rgb;
        normalTS = map_bump.Sample(wrapSampler, input.uv).rgb;
    }
    
    float3 emissive = emissiveMap.Sample(wrapSampler, input.uv).rgb;
	float3x3 tangentFrameWS = float3x3(normalize(input.TangentWS), normalize(input.BitangentWS), normalize(input.normal));
	normalTS = normalize(normalTS * 2.0f - 1.0f);
	float3 normalWS = mul(normalTS, tangentFrameWS);
    output.position = input.w_pos; 
	output.normal = float4(normalWS, 1.0f);
    output.emissive = (isEmissive) ? float4(emissive * emissivePower * emissiveIntensity, 1.f) : float4(0.f, 0.f, 0.f, 1.f);
	
    int radius = 3;
    float w, h;
    depthMap.GetDimensions(w, h);
    float2 ndc = float2(0.0f, 0.0f);
    ndc.x = (input.clipSpace.x / input.clipSpace.w) * 0.5f + 0.5f;
    ndc.y = (input.clipSpace.y / input.clipSpace.w) * 0.5f + 0.5f;
    float bias = max(0.04f * (1.f - dot(input.normal, lightDirection)), 0.005f);
    float px = 1.0f / w;
    float py = 1.0f / h;
    float hor = 0.f;
    float vir = 0.f;
    float o = 2.0f; 
    float s = 0.05f; 
    float shadow = 0;
    
    for (int i = -radius; i < radius; i++)
    {
        hor = pow(2, -(pow(i, 2) / o)) * s;
        for (int j = -radius; j < radius; j++)
        {
            vir = pow(2, -(pow(j, 2) / o)) * s;
            float depth = depthMap.Sample(boarderSampler, float2(ndc.x, 1.f - ndc.y) + float2(j * px, i * py)).r;
            shadow += (depth > input.clipSpace.z - bias || input.clipSpace.z > 1.0f) ? 1.f : (0.01f / (1.01f - (1.f / shadow.x))) * ((hor + vir) * 0.5f);
        }
    }
    
    shadow /= pow(radius * 2.f, 2.f);
    output.color = float4(diffuseAlbedo, shadow.x);
    
    return output;
}