#ifndef CAMERA_H
#define CAMERA_H
#include "GameObject.h"
#include "Object.h"

class Camera : public GameObject
{
private:
	void UpdateMatrix() override; 
	DirectX::XMMATRIX viewMatrix;
	DirectX::XMMATRIX projectionMatrix;

public:
	Camera();
	~Camera();
	void SetProjectionInput(float fovDegrees, float aspectRatio, float nearZ, float farZ);
	const DirectX::XMMATRIX& GetViewMatrix() const;
	const DirectX::XMMATRIX& GetProjectionMatrix() const;

	const void UpdatePosBuff(ID3D11DeviceContext* device_context_ptr, ID3D11Buffer* cameraPositionBuffer);
	const void UpdateCBuffers(ID3D11DeviceContext* device_context_ptr, ID3D11Buffer** constantBuffArray, Object someObject);
};
#endif