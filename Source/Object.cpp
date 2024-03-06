#include "../Header/Object.h"

Object::Object()
{
	this->mesh.reserve(800);
	std::string empty = "Empty";
	this->meshMaterial.Kd = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	this->meshMaterial.Ks = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	this->meshMaterial.Ka = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	this->meshMaterial.map_Kd = empty;
	this->meshMaterial.map_bump = empty;
	this->meshMaterial.map_Ks = empty;
	this->rotation = { 0.0f, 0.0f, 0.0f };
	this->scale = { 0.0f, 0.0f, 0.0f };
	this->translation = { 0.0f, 0.0f, 0.0f };
	this->vertex_buffer_pointer = NULL;
	this->shader_resource_view_ptr[0] = NULL;
	this->shader_resource_view_ptr[1] = NULL;
	this->shader_resource_view_ptr[2] = NULL;
}
Object::~Object()
{

}

void Object::LoadObj(const char* fileName)
{
	DirectX::XMFLOAT3 float3; 
	DirectX::XMFLOAT2 float2;
	const unsigned __int64 oneStep = 1; //	uint64_t is more versitile
	const unsigned __int64 twoStep = 2;

	std::vector<DirectX::XMFLOAT3> verticeVector;
	std::vector<DirectX::XMFLOAT2> textureVector;
	std::vector<DirectX::XMFLOAT3> normalVector;
	std::vector<DirectX::XMFLOAT4> tangentVector;
	std::vector<int> faceVectorPos;
	std::vector<int> faceVectorUv;
	std::vector<int> faceVectorNor;

	std::string myFile(fileName), prefix;
	std::string line2;
	std::ifstream file(myFile);
	std::istringstream inputString;

	while (std::getline(file, line2))
	{
		inputString.clear();
		inputString.str(line2);
		inputString >> prefix;
		if (prefix == "mtllib")
		{
			std::string mtlString;
			inputString >> mtlString;
			LoadMTL(mtlString);
		}
		else if (prefix == "v")
		{
			inputString >> float3.x >> float3.y >> float3.z;
			verticeVector.push_back(float3);
		}
		else if (prefix == "vt")
		{
			inputString >> float2.x >> float2.y;
			textureVector.push_back(float2);
		}
		else if (prefix == "vn")
		{
			inputString >> float3.x >> float3.y >> float3.z;
			normalVector.push_back(float3);
		}
		else if (prefix == "f")
		{
			int counter = 0;
			int temp = 0;

			std::vector<int> tempIndexPos;
			std::vector<int> tempIndexUv;
			std::vector<int> tempIndexNor;
			while (inputString >> temp)
			{
				if (counter == 0) // collect all position indicies
				{
					tempIndexPos.push_back(temp);
				}
				else if (counter == 1) // collect all uv indicies
				{
					tempIndexUv.push_back(temp);
				}
				else if (counter == 2) // collect all normal indicies
				{
					tempIndexNor.push_back(temp);
				}

				if (inputString.peek() == '/')
				{
					++counter;
					inputString.ignore(1, '/');
				}
				else if (inputString.peek() == ' ')
				{
					++counter;
					inputString.ignore(1, ' ');
				}
				if (counter > 2)
				{
					counter = 0;
				}
			}
			for (int i = 0; i < tempIndexPos.size(); i++)
			{
				if (i > 2)
				{
					faceVectorPos.push_back(tempIndexPos[0]);
					faceVectorPos.push_back(tempIndexPos[i - oneStep]);
					faceVectorPos.push_back(tempIndexPos[i]);
					faceVectorUv.push_back(tempIndexUv[0]);
					faceVectorUv.push_back(tempIndexUv[i - oneStep]);
					faceVectorUv.push_back(tempIndexUv[i]);
					faceVectorNor.push_back(tempIndexNor[0]);
					faceVectorNor.push_back(tempIndexNor[i - oneStep]);
					faceVectorNor.push_back(tempIndexNor[i]);
				}
				else
				{
					faceVectorPos.push_back(tempIndexPos[i]);
					faceVectorUv.push_back(tempIndexUv[i]);
					faceVectorNor.push_back(tempIndexNor[i]);
				}
			}
			

		}
	}
	file.close();
	
	DirectX::XMFLOAT3 edge1;
	DirectX::XMFLOAT3 edge2;
	DirectX::XMFLOAT2 deltaUV1;
	DirectX::XMFLOAT2 deltaUV2;
	DirectX::XMFLOAT4 tangent;

	for (int i = 0; i < faceVectorPos.size(); i+=3)
	{
		DirectX::XMFLOAT3 pos1 = verticeVector[faceVectorPos[i + 0] - oneStep];
		DirectX::XMFLOAT3 pos2 = verticeVector[faceVectorPos[i + oneStep] - oneStep];
		DirectX::XMFLOAT3 pos3 = verticeVector[faceVectorPos[i + twoStep] - oneStep];

		DirectX::XMFLOAT2 uv1 = textureVector[faceVectorUv[i + 0] - oneStep];
		DirectX::XMFLOAT2 uv2 = textureVector[faceVectorUv[i + oneStep] - oneStep];
		DirectX::XMFLOAT2 uv3 = textureVector[faceVectorUv[i + twoStep] - oneStep];
		edge1.x = pos2.x - pos1.x;
		edge1.y = pos2.y - pos1.y;
		edge1.z = pos2.z - pos1.z;
		edge2.x = pos3.x - pos1.x;
		edge2.y = pos3.y - pos1.y;
		edge2.z = pos3.z - pos1.z;
		deltaUV1.x = uv2.x - uv1.x;
		deltaUV1.y = uv2.y - uv1.y;
		deltaUV2.x = uv3.x - uv1.x;
		deltaUV2.y = uv3.y - uv1.y;
		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
		tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent.w = 1.0f;
		tangentVector.push_back(tangent);
		tangentVector.push_back(tangent);
		tangentVector.push_back(tangent);
	}

	finalVertex loadingMesh;
	for (int i = 0; i < faceVectorPos.size(); i++)
	{
		loadingMesh.POS = verticeVector[faceVectorPos[i] - oneStep];
		loadingMesh.NOR = normalVector[faceVectorNor[i] - oneStep];
		loadingMesh.TEX = textureVector[faceVectorUv[i] - oneStep];
		loadingMesh.TAN = tangentVector[i];
		mesh.push_back(loadingMesh);
	}
	
	vertex_stride = this->getStride();
	vertex_count = this->getSize();
}

void Object::LoadMTL(std::string fileName)
{
	std::string myFile("Assets/" + fileName), prefix;
	std::string line2;
	std::ifstream file(myFile);
	std::istringstream inputString;

	while (std::getline(file, line2))
	{
		inputString.clear();
		inputString.str(line2);
		inputString >> prefix;
		if (prefix == "Kd")
		{
			inputString >> this->meshMaterial.Kd.x >> this->meshMaterial.Kd.y >> this->meshMaterial.Kd.z;
		}
		else if (prefix == "Ks")
		{
			inputString >> this->meshMaterial.Ks.x >> this->meshMaterial.Ks.y >> this->meshMaterial.Ks.z;
		}
		else if (prefix == "Ka")
		{
			inputString >> this->meshMaterial.Ka.x >> this->meshMaterial.Ka.y >> this->meshMaterial.Ka.z;
		}
		else if (prefix == "map_Kd")
		{
			inputString >> meshMaterial.map_Kd;
			meshMaterial.map_Kd = "Assets/" + meshMaterial.map_Kd;
		}
		else if (prefix == "map_Bump" || prefix == "map_bump")
		{
			inputString >> meshMaterial.map_bump;
			meshMaterial.map_bump = "Assets/" + meshMaterial.map_bump;
		}
		else if (prefix == "map_Ks")
		{
			inputString >> meshMaterial.map_Ks;
			meshMaterial.map_Ks = "Assets/" + meshMaterial.map_Ks;
		}
		
	}
	file.close();
}

void Object::LoadTextures(ID3D11Device* Device_ptr)
{
	const char* noName = "Assets/Empty.png";
	std::string errorname = this->meshMaterial.map_Kd;
	if (errorname != "Empty")
	{
		this->diffuse.LoadTextureFromName(this->meshMaterial.map_Kd.c_str(), Device_ptr);
	}
	else
	{
		this->diffuse.LoadTextureFromName(noName, Device_ptr);
	}
	errorname = this->meshMaterial.map_bump;
	if (errorname != "Empty")
	{
		this->bump.LoadTextureFromName(this->meshMaterial.map_bump.c_str(), Device_ptr);
	}
	else
	{
		this->bump.LoadTextureFromName(noName, Device_ptr);
	}
	
}

void Object::setRST(DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT3 translation)
{
	this->rotation = rotation;
	this->scale = scale;
	this->translation = translation;
}

void Object::setShadowDepthMap(ID3D11ShaderResourceView*& shadowDepthMap)
{
	shader_resource_view_ptr[2] = shadowDepthMap;
}

void Object::createTexture(ID3D11Device* device_ptr)
{
	this->LoadTextures(device_ptr);
	this->diffuse.getSRView(device_ptr, this->shader_resource_view_ptr[0]);
	this->bump.getSRView(device_ptr, this->shader_resource_view_ptr[1]);
}

HRESULT Object::createVertexBuffer(ID3D11Device* device_ptr)
{
	D3D11_BUFFER_DESC vertex_buff_descr = {};
	vertex_buff_descr.ByteWidth = this->getSize() * this->getStride();
	vertex_buff_descr.StructureByteStride = this->getStride();
	vertex_buff_descr.CPUAccessFlags = 0;
	vertex_buff_descr.Usage = D3D11_USAGE_DEFAULT;
	vertex_buff_descr.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertex_buff_descr.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA subresource_data = {};
	subresource_data.pSysMem = this->getMesh();

	HRESULT hr = device_ptr->CreateBuffer(&vertex_buff_descr, &subresource_data, &vertex_buffer_pointer);
	return hr;
}

ID3D11Buffer*& Object::getVertexBuffer()
{
	return this->vertex_buffer_pointer;
}

DirectX::XMFLOAT3 Object::getRotation()
{
	return this->rotation;
}

DirectX::XMFLOAT3 Object::getScale()
{
	return this->scale;
}

DirectX::XMFLOAT3 Object::getTranslation()
{
	return this->translation;
}


void Object::getDiffuseSRView(ID3D11Device* Device_ptr, ID3D11ShaderResourceView*& ShaderResourceView)
{
	this->diffuse.getSRView(Device_ptr, ShaderResourceView);
}

void Object::getBumpSRView(ID3D11Device* Device_ptr, ID3D11ShaderResourceView*& ShaderResourceView)
{
	this->bump.getSRView(Device_ptr, ShaderResourceView);
}

mtl& Object::getMTL()
{
	return this->meshMaterial;
}

void* Object::getMesh()
{
	return mesh.data();
}

int Object::getSize()
{
	return mesh.size();
}

int Object::getStride()
{
	return sizeof(finalVertex);
}

const void Object::UpdateMaterialBuffer(ID3D11DeviceContext* device_context_ptr, ID3D11Buffer* materialBuffer)
{
	MaterialBuffer Mdata;
	Mdata.Kd = this->getMTL().Kd;
	Mdata.Ks = this->getMTL().Ks;
	Mdata.Ka = this->getMTL().Ka;
	D3D11_MAPPED_SUBRESOURCE materialBufferMap;
	device_context_ptr->Map(materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &materialBufferMap);
	CopyMemory(materialBufferMap.pData, &Mdata, sizeof(MaterialBuffer));
	device_context_ptr->Unmap(materialBuffer, 0);
}

void Object::draw(ID3D11DeviceContext* device_context_ptr, UINT& vertex_offset)
{
	device_context_ptr->PSSetShaderResources(0, 3, shader_resource_view_ptr);
	device_context_ptr->IASetVertexBuffers(0, 1, &vertex_buffer_pointer, &this->vertex_stride, &vertex_offset);
	device_context_ptr->Draw(this->vertex_count, 0);
}

void Object::drawEmissive(ID3D11DeviceContext* device_context_ptr, UINT& vertex_offset, ID3D11ShaderResourceView* emissiveMap)
{
	device_context_ptr->PSSetShaderResources(3, 1, &emissiveMap);
	device_context_ptr->IASetVertexBuffers(0, 1, &vertex_buffer_pointer, &this->vertex_stride, &vertex_offset);
	device_context_ptr->Draw(this->vertex_count, 0);
}

void Object::release()
{
	this->vertex_buffer_pointer->Release();
	for (int i = 0; i < 2; i++) {
		if(this->shader_resource_view_ptr[i])
			this->shader_resource_view_ptr[i]->Release();
	}
}