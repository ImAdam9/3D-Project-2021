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

cbuffer materialBuffer : register(b1)
{
	float3 Kd; //diffuse
	float3 Ks; //specular
	float3 Ka; //Ambient
};

struct vs_out
{
	float4 wvp_pos : SV_POSITION;
	float2 uv     : TEX;
};

struct ps_out
{
	float4 color : SV_TARGET0;
};

Texture2D Albedo       : register(t0); 
Texture2D WorldPos     : register(t1);
Texture2D Normal       : register(t2);
Texture2D Emissive     : register(t3);

SamplerState wrapSampler : register(s0);
ps_out ps_main(vs_out input)
{
	ps_out output;
    float4 albedo = Albedo.Sample(wrapSampler, input.uv).rgba;
    float3 worldPosition = WorldPos.Sample(wrapSampler, input.uv).rgb;
    float3 normal = Normal.Sample(wrapSampler, input.uv).rgb;
    float shadow = Albedo.Sample(wrapSampler, input.uv).a;
    float3 emissive = Emissive.Sample(wrapSampler, input.uv).rgb;
	
	// Glow Effect Calc
    float3 glow = (0.f, 0.f, 0.f);
    if (albedo.a == 1.0f) // Skip Glow calc if alpha isn't 1. This prevents unessesary blurring of the background
    {
        uint width, height;
        Emissive.GetDimensions(width, height);
        const int radius = 8;
        const float dx = 1.f / width;
        const float dy = 1.f / height;
        float h = 0.f, v = 0.f;
        for (int j = -radius; j <= radius; j++)
        {
            h = pow(2, -(pow(j, 2) / 6.5f)) * 0.05f;
            for (int i = -radius; i <= radius; i++)
            {
                v = pow(2, -(pow(i, 2) / 6.5f)) * 0.05f;
                const float2 tc = input.uv + float2(dx * i, dy * j);
                glow += Emissive.Sample(wrapSampler, tc).rgb * ((h + v) * 0.5f);
            }
        }
    }
    
	// POINTLIGHT WORKS
	// Ambient
    float3 ambientCalc = Ka * ambientStrength;

	//Diffuse light
	float3 v_to_l = normalize(lightPos - worldPosition);
    float3 diffuseIntensity = max(dot(v_to_l, normal), 0);
    float dotTest = dot(lightDirection, normal);
    if (dotTest < 1.f && dotTest >= 0.5f)
        diffuseIntensity = lightDiffuse * 0.2f * shadow;
    else
        diffuseIntensity = lightDiffuse * shadow;
	
	//SpotLight Calc
    float spot = pow(max(dot(-v_to_l, lightDirection), 0.0f), spotAngle);

	//Attenuation Factor
	float distanceToLight = distance(lightPos, worldPosition);

	float attenuationFactor = spot / (attenuation_a + attenuation_b * distanceToLight + attenuation_c * pow(distanceToLight, 2));
	ambientCalc *= spot;

	diffuseIntensity *= attenuationFactor;

    float3 diffuseFinal = diffuseIntensity * lightDiffuse * lightStrength * Kd;

	//AppliedLights
	float3 appliedLight = diffuseFinal + ambientCalc;

	//Final Color
	float3 finalColor = albedo.rgb * appliedLight;
	
    
    finalColor += emissive;
    finalColor += glow * 0.5f;
	
    output.color = float4(finalColor, 1.0f);
	return output;
}