#ifndef OBJECT_H
#define OBJECT_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Structs.h"
#include "Texture.h"

struct finalVertex { 
	DirectX::XMFLOAT3 POS;
	DirectX::XMFLOAT3 NOR;
	DirectX::XMFLOAT2 TEX;
	DirectX::XMFLOAT4 TAN;
};
struct mtl {
	DirectX::XMFLOAT3 Kd;
	DirectX::XMFLOAT3 Ks;
	DirectX::XMFLOAT3 Ka;
	std::string map_Kd;
	std::string map_bump;
	std::string map_Ks;
};
class Object
{
private:
	std::vector<finalVertex> mesh;
	mtl meshMaterial;
	Texture diffuse;
	Texture bump;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT3 scale;
	DirectX::XMFLOAT3 translation;
	ID3D11Buffer* vertex_buffer_pointer;
	ID3D11ShaderResourceView* shader_resource_view_ptr[3];
public:
	Object();
	~Object();
	
	void LoadObj(const char* fileName);
	void LoadMTL(std::string fileName);
	void LoadTextures(ID3D11Device* Device_ptr);

	void setRST(DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT3 translation);
	void setShadowDepthMap(ID3D11ShaderResourceView*& shadowDepthMap);
	void createTexture(ID3D11Device* device_ptr);
	HRESULT createVertexBuffer(ID3D11Device* device_ptr);
	ID3D11Buffer*& getVertexBuffer();

	DirectX::XMFLOAT3 getRotation();
	DirectX::XMFLOAT3 getScale();
	DirectX::XMFLOAT3 getTranslation();

	void getDiffuseSRView(ID3D11Device* Device_ptr, ID3D11ShaderResourceView*& ShaderResourceView);
	void getBumpSRView(ID3D11Device* Device_ptr, ID3D11ShaderResourceView*& ShaderResourceView);

	mtl& getMTL();
	void* getMesh(); 
	int getSize();
	int getStride();

	UINT vertex_stride = 0;
	UINT vertex_count = 0;
	
	const void UpdateMaterialBuffer(ID3D11DeviceContext* device_context_ptr, ID3D11Buffer* materialBuffer);
	void draw(ID3D11DeviceContext* device_context_ptr, UINT& vertex_offset);
	void drawEmissive(ID3D11DeviceContext* device_context_ptr, UINT& vertex_offset, ID3D11ShaderResourceView* emissiveMap);
	void release();
};
#endif