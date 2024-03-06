#include "../Header/Light.h"

void Light::UpdateMatrix()
{
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(this->rotation.x, this->rotation.y, this->rotation.z);
	DirectX::XMVECTOR target = DirectX::XMVector3TransformCoord(this->FORWARD_VECTOR, rotationMatrix);
	target = DirectX::XMVectorAdd(target, this->positionVector);
	DirectX::XMVECTOR upDirection = DirectX::XMVector3TransformCoord(this->UP_VECTOR, rotationMatrix);
	this->viewMatrix = DirectX::XMMatrixLookAtLH(this->positionVector, target, upDirection);

}

Light::Light()
{
	this->lightStrength = 1.0f;
	this->lightColor = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	this->ambientStrength = 0.2f;
	this->lightDirection = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);
	this->spotAngle = 10.0f;
	this->attenuation_a = 1.0f;
	this->attenuation_b = 0.1f;
	this->attenuation_c = 0.1f;
	this->SetPosition(0.0f, 0.5f, 1.0f);
	this->SetRotation(0.0f, 0.0f, 0.0f);
	this->UpdateMatrix();
}

Light::~Light()
{

}

void Light::SetLightStrength(float newStrength)
{
	this->lightStrength = newStrength;
}

void Light::SetAmbientStrength(float newStrength)
{
	this->ambientStrength = newStrength;
}

void Light::SetLightDirection(const DirectX::XMVECTOR& newDirection)
{
	DirectX::XMStoreFloat3(&this->lightDirection, newDirection);
	this->UpdateMatrix();
}

void Light::SetSpotAngle(float newAngle)
{
	this->spotAngle = newAngle;
}

void Light::SetAttenuationABC(float a, float b, float c)
{
	this->attenuation_a = a;
	this->attenuation_b = b;
	this->attenuation_c = c;
}

void Light::SetProjectionInput(float viewWidth, float viewHeight, float nearZ, float farZ)
{
	this->projectionMatrix = DirectX::XMMatrixOrthographicLH(viewWidth, viewHeight, nearZ, farZ);
}

const DirectX::XMMATRIX& Light::GetViewMatrix() const
{
	return this->viewMatrix;
}

const DirectX::XMMATRIX& Light::GetProjectionMatrix() const
{
	return this->projectionMatrix;
}

float Light::GetLightStrength()
{
	return this->lightStrength;
}

float Light::GetAmbientStrength()
{
	return this->ambientStrength;
}

DirectX::XMFLOAT3 Light::GetLightColor()
{
	return this->lightColor;
}

DirectX::XMFLOAT3 Light::GetLightDirection()
{
	return this->lightDirection;
}

float Light::GetSpotAngle()
{
	return this->spotAngle;
}

float Light::GetAttenuationA()
{
	return this->attenuation_a;
}

float Light::GetAttenuationB()
{
	return this->attenuation_b;
}

float Light::GetAttenuationC()
{
	return this->attenuation_c;
}

const void Light::UpdateLight(ID3D11DeviceContext* device_context_ptr, ID3D11Buffer** constantBuffArray, Object someObject)
{
	LightBuffer Ldata;
	LightWVPBuffer LWVPdata;
	Ldata.lightPos = this->GetPositionFloat3();
	Ldata.ambientStrength = this->ambientStrength;
	Ldata.lightDiffuse = this->lightColor;
	Ldata.lightStrength = this->lightStrength;
	Ldata.lightDirection = this->lightDirection;
	Ldata.spotAngle = this->spotAngle;
	Ldata.attenuation_a = this->attenuation_a;
	Ldata.attenuation_b = this->attenuation_b;
	Ldata.attenuation_c = this->attenuation_c;

	D3D11_MAPPED_SUBRESOURCE lightBufferMap;
	D3D11_MAPPED_SUBRESOURCE LWVPMap;
	DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX view = this->viewMatrix;
	DirectX::XMMATRIX projection = this->projectionMatrix;

	world = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationRollPitchYaw(someObject.getRotation().x, someObject.getRotation().y, someObject.getRotation().z),
		DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(someObject.getScale().x, someObject.getScale().y, someObject.getScale().z),
			DirectX::XMMatrixTranslation(someObject.getTranslation().x, someObject.getTranslation().y, someObject.getTranslation().z)));

	world = DirectX::XMMatrixTranspose(world);
	view = DirectX::XMMatrixTranspose(view);
	projection = DirectX::XMMatrixTranspose(projection);

	DirectX::XMStoreFloat4x4(&LWVPdata.WorldMatrix, world);
	DirectX::XMStoreFloat4x4(&LWVPdata.ViewMatrix, view);
	DirectX::XMStoreFloat4x4(&LWVPdata.ProjectionMatrix, projection);

	device_context_ptr->Map(constantBuffArray[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &lightBufferMap);
	device_context_ptr->Map(constantBuffArray[1], 0, D3D11_MAP_WRITE_DISCARD, 0, &LWVPMap);
	CopyMemory(lightBufferMap.pData, &Ldata, sizeof(LightBuffer));
	CopyMemory(LWVPMap.pData, &LWVPdata, sizeof(LightWVPBuffer));
	device_context_ptr->Unmap(constantBuffArray[0], 0);
	device_context_ptr->Unmap(constantBuffArray[1], 0);
}
