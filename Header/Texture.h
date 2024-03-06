#ifndef TEXTURE_H
#define TEXTURE_H

#include <d3d11.h>
#include <assert.h>
#include "../stb_image.h" 

class Texture 
{
private:
	ID3D11Texture2D* texture;

	D3D11_TEXTURE2D_DESC textureDesc; 
	D3D11_SUBRESOURCE_DATA textureSubResourceData; 
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
public:
	Texture();
	~Texture();
	void LoadTextureFromName(const char* fileName, ID3D11Device* Device_ptr);
	ID3D11Texture2D*& getTexture();
	void getSRView(ID3D11Device* Device_ptr, ID3D11ShaderResourceView* &ShaderResourceView);
};
#endif