#pragma once
#include <DirectXMath.h>

struct ObjectBuffer
{
    DirectX::XMFLOAT4X4 WorldMatrix;
};

struct CameraBuffer
{
    DirectX::XMFLOAT4X4 ViewMatrix;
    DirectX::XMFLOAT4X4 ProjectionMatrix;
};

struct LightBuffer
{
    DirectX::XMFLOAT3 lightPos;
    float ambientStrength;
    DirectX::XMFLOAT3 lightDiffuse;
    float lightStrength;
    DirectX::XMFLOAT3 lightDirection;
    float spotAngle;
    float attenuation_a;
    float attenuation_b;
    float attenuation_c;
};

struct MaterialBuffer
{
    DirectX::XMFLOAT3 Kd;
    float pad;
    DirectX::XMFLOAT3 Ks;
    float Pad;
    DirectX::XMFLOAT3 Ka;
};

struct CameraPositionBuffer
{
    DirectX::XMFLOAT3 cameraPosition;
};

struct LightWVPBuffer
{
    DirectX::XMFLOAT4X4 WorldMatrix;
    DirectX::XMFLOAT4X4 ViewMatrix;
    DirectX::XMFLOAT4X4 ProjectionMatrix;
};

struct tessFactor
{
    float tessFx;
    float tessFy;
    float tessFz;
    float tessFw;
};

struct emissiveBuffer
{
    DirectX::XMFLOAT3 emissiveIntensity;
    float emissivePower;
    bool isEmissive;
};

struct uvOffsetBuffer
{
    DirectX::XMFLOAT2 uvOffsets;
    float animate;
};