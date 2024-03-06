#include "../Header/Camera.h"

void Camera::UpdateMatrix()
{
	DirectX::XMMATRIX camRotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(this->rotation.x, this->rotation.y, this->rotation.z);
	DirectX::XMVECTOR cameraTarget = DirectX::XMVector3TransformCoord(this->FORWARD_VECTOR, camRotationMatrix);
	cameraTarget = DirectX::XMVectorAdd(cameraTarget, this->positionVector);
	DirectX::XMVECTOR upDirection = DirectX::XMVector3TransformCoord(this->UP_VECTOR, camRotationMatrix);
	this->viewMatrix = DirectX::XMMatrixLookAtLH(this->positionVector, cameraTarget, upDirection);

	DirectX::XMMATRIX vecRotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(this->rotation.x, this->rotation.y, 0.0f);
	this->vec_forward  = DirectX::XMVector3TransformCoord(this->FORWARD_VECTOR, vecRotationMatrix);
	this->vec_left     = DirectX::XMVector3TransformCoord(this->LEFT_VECTOR, vecRotationMatrix);
	this->vec_right	   = DirectX::XMVector3TransformCoord(this->RIGHT_VECTOR, vecRotationMatrix);
	this->vec_backward = DirectX::XMVector3TransformCoord(this->BACKWARD_VECTOR, vecRotationMatrix);
}

Camera::Camera()
{
	this->position       = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	this->positionVector = DirectX::XMLoadFloat3(&this->position);
	this->rotation       = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	this->rotationVector = DirectX::XMLoadFloat3(&this->rotation);
	this->UpdateMatrix();
}

Camera::~Camera()
{

}

void Camera::SetProjectionInput(float fovDegrees, float aspectRatio, float nearZ, float farZ)
{
	float fovRadians = (fovDegrees / 360.0f) * DirectX::XM_2PI;
	this->projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, nearZ, farZ);
}

const DirectX::XMMATRIX& Camera::GetViewMatrix() const
{
	return this->viewMatrix;
}

const DirectX::XMMATRIX& Camera::GetProjectionMatrix() const
{
	return this->projectionMatrix;
}
const void Camera::UpdatePosBuff(ID3D11DeviceContext* device_context_ptr, ID3D11Buffer* cameraPositionBuffer)
{
	CameraPositionBuffer CPdata;
	DirectX::XMStoreFloat3(&CPdata.cameraPosition, this->GetPositionVector());
	D3D11_MAPPED_SUBRESOURCE CameraPosMap;
	device_context_ptr->Map(cameraPositionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &CameraPosMap);
	CopyMemory(CameraPosMap.pData, &CPdata, sizeof(CameraPositionBuffer));
	device_context_ptr->Unmap(cameraPositionBuffer, 0);
}

const void Camera::UpdateCBuffers(ID3D11DeviceContext* device_context_ptr, ID3D11Buffer** constantBuffArray, Object someObject)
{
	ObjectBuffer Odata;
	CameraBuffer Cdata;

	DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX view = this->viewMatrix;
	DirectX::XMMATRIX projection = this->projectionMatrix;

	world = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationRollPitchYaw(someObject.getRotation().x, someObject.getRotation().y, someObject.getRotation().z),
		DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(someObject.getScale().x, someObject.getScale().y, someObject.getScale().z),
			DirectX::XMMatrixTranslation(someObject.getTranslation().x, someObject.getTranslation().y, someObject.getTranslation().z)));

	world = DirectX::XMMatrixTranspose(world);
	view = DirectX::XMMatrixTranspose(view);
	projection = DirectX::XMMatrixTranspose(projection);

	DirectX::XMStoreFloat4x4(&Odata.WorldMatrix, world);
	DirectX::XMStoreFloat4x4(&Cdata.ViewMatrix, view);
	DirectX::XMStoreFloat4x4(&Cdata.ProjectionMatrix, projection);

	D3D11_MAPPED_SUBRESOURCE ObjectMap;
	D3D11_MAPPED_SUBRESOURCE CameraMap;

	device_context_ptr->Map(constantBuffArray[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &ObjectMap);
	device_context_ptr->Map(constantBuffArray[1], 0, D3D11_MAP_WRITE_DISCARD, 0, &CameraMap);
	CopyMemory(ObjectMap.pData, &Odata, sizeof(ObjectBuffer));
	CopyMemory(CameraMap.pData, &Cdata, sizeof(CameraBuffer));
	device_context_ptr->Unmap(constantBuffArray[0], 0);
	device_context_ptr->Unmap(constantBuffArray[1], 0);
}
