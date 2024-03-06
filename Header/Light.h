#ifndef LIGHT_H
#define LIGHT_H
#include "GameObject.h"
#include "Object.h"

class Light : public GameObject
{
private:
	void UpdateMatrix() override;
	DirectX::XMMATRIX viewMatrix;
	DirectX::XMMATRIX projectionMatrix;
	float lightStrength;
	DirectX::XMFLOAT3 lightColor;
	float ambientStrength;
	DirectX::XMFLOAT3 lightDirection;
	float spotAngle;
	float attenuation_a;
	float attenuation_b;
	float attenuation_c;
	
public:
	Light();
	~Light();
	void SetLightStrength(float newStrength);
	void SetAmbientStrength(float newStrength);
	void SetLightDirection(const DirectX::XMVECTOR& newDirection);
	void SetSpotAngle(float newAngle);
	void SetAttenuationABC(float a, float b, float c);
	void SetProjectionInput(float viewWidth, float viewHeight, float nearZ, float farZ);
	const DirectX::XMMATRIX& GetViewMatrix() const;
	const DirectX::XMMATRIX& GetProjectionMatrix() const;
	float GetLightStrength();
	float GetAmbientStrength();
	DirectX::XMFLOAT3 GetLightColor();
	DirectX::XMFLOAT3 GetLightDirection();
	float GetSpotAngle();
	float GetAttenuationA();
	float GetAttenuationB();
	float GetAttenuationC();

	const void UpdateLight(ID3D11DeviceContext* device_context_ptr, ID3D11Buffer** constantBuffArray, Object someObject);
};
#endif
