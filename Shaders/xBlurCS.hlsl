// Declare input and output resources
Texture2D backBuffer : register(t0);
RWTexture2D<float4> blur : register(u0);

// Group Size
#define size_x 16
#define size_y 16

static const float oneFilter[5] =
{
  0.15338835280702454, 0.22146110682534667, 0.2503010807352574, 0.22146110682534667, 0.15338835280702454,
};

[numthreads(size_x, size_y, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{

    int3 textureLocation = DTid - int3(3, 0, 0);
    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    for (int i = 2; i < 5; i++)
    {
        color += backBuffer.Load(textureLocation + int3(i, 0, 0)) * oneFilter[i];

    }
    for (int j = 2; j >= 0; j--)
    {
        color += backBuffer.Load(textureLocation + int3(j, 0, 0)) * oneFilter[j];
    }
    
    blur[DTid.xy] = color;
}