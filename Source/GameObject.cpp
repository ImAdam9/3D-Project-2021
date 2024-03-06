#include "../Header/GameObject.h"

void GameObject::UpdateMatrix()
{
	DirectX::XMMATRIX vecRotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(this->rotation.x, this->rotation.y, 0.0f);
	this->vec_forward = DirectX::XMVector3TransformCoord(this->FORWARD_VECTOR, vecRotationMatrix);
	this->vec_backward = DirectX::XMVector3TransformCoord(this->BACKWARD_VECTOR, vecRotationMatrix);
	this->vec_left = DirectX::XMVector3TransformCoord(this->LEFT_VECTOR, vecRotationMatrix);
	this->vec_right = DirectX::XMVector3TransformCoord(this->RIGHT_VECTOR, vecRotationMatrix);
}


const DirectX::XMVECTOR& GameObject::GetPositionVector() const
{
	return this->positionVector;
}

const DirectX::XMFLOAT3& GameObject::GetPositionFloat3() const
{
	return this->position;
}

const DirectX::XMVECTOR& GameObject::GetRotationVector() const
{
	return this->rotationVector;
}

const DirectX::XMFLOAT3& GameObject::GetRotationFloat3() const
{
	return this->rotation;
}

void GameObject::SetPosition(const DirectX::XMVECTOR& pos)
{
	DirectX::XMStoreFloat3(&this->position, pos);
	this->positionVector = pos;
	this->UpdateMatrix();
}

void GameObject::SetPosition(const DirectX::XMFLOAT3& pos)
{
	this->position = pos;
	this->positionVector = DirectX::XMLoadFloat3(&this->position);
	this->UpdateMatrix();
}

void GameObject::SetPosition(float x, float y, float z)
{
	this->position = DirectX::XMFLOAT3(x, y, z);
	this->positionVector = DirectX::XMLoadFloat3(&this->position);
	this->UpdateMatrix();
}

void GameObject::AdjustPosition(const DirectX::XMVECTOR& pos)
{
	this->positionVector = DirectX::XMVectorAdd(this->positionVector, pos);
	DirectX::XMStoreFloat3(&this->position, this->positionVector);
	this->UpdateMatrix();
}

void GameObject::AdjustPosition(const DirectX::XMFLOAT3& pos)
{
	this->position.x += pos.x;
	this->position.y += pos.y;
	this->position.z += pos.z;
	this->positionVector = DirectX::XMLoadFloat3(&this->position);
	this->UpdateMatrix();
}

void GameObject::AdjustPosition(float x, float y, float z)
{
	this->position.x += x;
	this->position.y += y;
	this->position.z += z;
	this->positionVector = DirectX::XMLoadFloat3(&this->position);
	this->UpdateMatrix();
}

void GameObject::SetRotation(const DirectX::XMVECTOR& rot)
{
	this->rotationVector = rot;
	DirectX::XMStoreFloat3(&this->rotation, rot);
	this->UpdateMatrix();
}

void GameObject::SetRotation(const DirectX::XMFLOAT3& rot)
{
	this->rotation = rot;
	this->rotationVector = DirectX::XMLoadFloat3(&this->rotation);
	this->UpdateMatrix();
}

void GameObject::SetRotation(float x, float y, float z)
{
	this->rotation = DirectX::XMFLOAT3(x, y, z);
	this->rotationVector = DirectX::XMLoadFloat3(&this->rotation);
	this->UpdateMatrix();
}

void GameObject::AdjustRotation(const DirectX::XMVECTOR& rot)
{
	this->rotationVector = DirectX::XMVectorAdd(this->rotationVector, rot);
	DirectX::XMStoreFloat3(&this->rotation, this->rotationVector);
	this->UpdateMatrix();
}

void GameObject::AdjustRotation(const DirectX::XMFLOAT3& rot)
{
	this->rotation.x += rot.x;
	this->rotation.y += rot.y;
	this->rotation.z += rot.z;
	this->rotationVector = DirectX::XMLoadFloat3(&this->rotation);
	this->UpdateMatrix();
}

void GameObject::AdjustRotation(float x, float y, float z)
{
	this->rotation.x += x;
	this->rotation.y += y;
	this->rotation.z += z;
	this->rotationVector = DirectX::XMLoadFloat3(&this->rotation);
	this->UpdateMatrix();
}

void GameObject::SetLookAtPos(DirectX::XMFLOAT3 lookAtPos)
{
	if (lookAtPos.x == this->position.x && lookAtPos.y == this->position.y && lookAtPos.z == this->position.z)
	{
		return;
	}
	lookAtPos.x = this->position.x - lookAtPos.x;
	lookAtPos.y = this->position.y - lookAtPos.y;
	lookAtPos.z = this->position.z - lookAtPos.z;

	float pitch = 0.0f;
	if (lookAtPos.y != 0.0f)
	{
		const float distance = sqrt(lookAtPos.x * lookAtPos.x + lookAtPos.z * lookAtPos.z);
		pitch = atan(lookAtPos.y / distance);
	}

	float yaw = 0.0f;
	if (lookAtPos.x != 0.0f)
	{
		yaw = atan(lookAtPos.x / lookAtPos.z);
	}
	if (lookAtPos.z > 0.0f)
	{
		yaw += DirectX::XM_PI;
	}
	this->SetRotation(pitch, yaw, 0.0f);
}

const DirectX::XMVECTOR& GameObject::GetForwardVector()
{
	return this->vec_forward;
}

const DirectX::XMVECTOR& GameObject::GetRightVector()
{
	return this->vec_right;
}

const DirectX::XMVECTOR& GameObject::GetBackwardVector()
{
	return this->vec_backward;
}

const DirectX::XMVECTOR& GameObject::GetLeftVector()
{
	return this->vec_left;
}
