#include "../Header/Texture.h"

Texture::Texture()
{
	this->texture = nullptr;
	this->textureDesc			 = {};
	textureDesc.MipLevels		 = 1;
	textureDesc.ArraySize		 = 1;
	textureDesc.Format			 = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage			 = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags		 = D3D11_BIND_SHADER_RESOURCE;
	this->textureSubResourceData = {};
	this->shaderResourceViewDesc = {};
	shaderResourceViewDesc.Format					 = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension			 = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels		 = 1;
}

Texture::~Texture()
{

}

void Texture::LoadTextureFromName(const char* fileName, ID3D11Device* Device_ptr)
{
	int texWidth, texHeight, texNumChannels;
	int texForceNumChannels = 4;
	unsigned char* testTextureBytes = stbi_load(fileName, &texWidth, &texHeight,
		&texNumChannels, texForceNumChannels);
	assert(testTextureBytes);
	int texBytesPerRow = 4 * texWidth;
	textureDesc.Width = texWidth;
	textureDesc.Height = texHeight;
	this->textureSubResourceData.pSysMem = testTextureBytes;
	this->textureSubResourceData.SysMemPitch = texBytesPerRow;

	HRESULT hr = Device_ptr->CreateTexture2D(&this->textureDesc, &this->textureSubResourceData, &this->texture);
	assert(SUCCEEDED(hr));
}

ID3D11Texture2D*& Texture::getTexture()
{
	return this->texture;
}

void Texture::getSRView(ID3D11Device* Device_ptr, ID3D11ShaderResourceView* &ShaderResourceView)
{
	HRESULT hr = Device_ptr->CreateShaderResourceView(this->texture, &this->shaderResourceViewDesc, &ShaderResourceView);
	assert(SUCCEEDED(hr));
}