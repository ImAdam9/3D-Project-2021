#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
#include "Structs.h"
#include <d3d11.h>

class GameObject
{
protected:
	virtual void UpdateMatrix();
	
	DirectX::XMVECTOR positionVector;
	DirectX::XMVECTOR rotationVector;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;

	const DirectX::XMVECTOR FORWARD_VECTOR = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	const DirectX::XMVECTOR UP_VECTOR = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	const DirectX::XMVECTOR BACKWARD_VECTOR = DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
	const DirectX::XMVECTOR LEFT_VECTOR = DirectX::XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f);
	const DirectX::XMVECTOR RIGHT_VECTOR = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

	DirectX::XMVECTOR vec_forward;
	DirectX::XMVECTOR vec_left;
	DirectX::XMVECTOR vec_right;
	DirectX::XMVECTOR vec_backward;

public:
	const DirectX::XMVECTOR& GetPositionVector() const;
	const DirectX::XMFLOAT3& GetPositionFloat3() const;
	const DirectX::XMVECTOR& GetRotationVector() const;
	const DirectX::XMFLOAT3& GetRotationFloat3() const;

	void SetPosition(const DirectX::XMVECTOR& pos);
	void SetPosition(const DirectX::XMFLOAT3& pos);
	void SetPosition(float x, float y, float z);

	void AdjustPosition(const DirectX::XMVECTOR& pos);
	void AdjustPosition(const DirectX::XMFLOAT3& pos);
	void AdjustPosition(float x, float y, float z);

	void SetRotation(const DirectX::XMVECTOR& rot);
	void SetRotation(const DirectX::XMFLOAT3& rot);
	void SetRotation(float x, float y, float z);

	void AdjustRotation(const DirectX::XMVECTOR& rot);
	void AdjustRotation(const DirectX::XMFLOAT3& rot);
	void AdjustRotation(float x, float y, float z);

	void SetLookAtPos(DirectX::XMFLOAT3 lookAtPos);

	const DirectX::XMVECTOR& GetForwardVector();
	const DirectX::XMVECTOR& GetRightVector();
	const DirectX::XMVECTOR& GetBackwardVector();
	const DirectX::XMVECTOR& GetLeftVector();
};
#endif