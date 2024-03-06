#include "../Header/Gbuffer.h"

Gbuffer::Gbuffer(int width, int height)
{
	this->localWidth = width;
	this->localHeight = height;
	this->gBuffer = nullptr;
	this->renderTargetView = NULL;
	this->shaderResourceView = nullptr;
	this->texture2DDesc = {};
	texture2DDesc.Width = width;
	texture2DDesc.Height = height;
	texture2DDesc.MipLevels = 1;
	texture2DDesc.ArraySize = 1;
	texture2DDesc.Format =  DXGI_FORMAT_R32G32B32A32_FLOAT;
	texture2DDesc.SampleDesc.Count = 1;
	texture2DDesc.Usage = D3D11_USAGE_DEFAULT;
	texture2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	this->textureSubResourceData = {};
	this->renderTargetViewDesc = {};
	this->shaderResourceViewDesc = {};
}

Gbuffer::~Gbuffer()
{
	this->gBuffer->Release();
	this->renderTargetView->Release();
	this->shaderResourceView->Release();
}

void Gbuffer::MakeGbuffer(ID3D11Device* Device_ptr)
{
	DirectX::XMFLOAT4* pixelArray = new DirectX::XMFLOAT4[(const unsigned __int64)this->localWidth * (const unsigned __int64)this->localHeight];
	DirectX::XMFLOAT4 temp = { 1.0f, 1.0f, 1.0f, 1.0f};
	for (int y = 0; y < this->localHeight; y++)
	{
		for (int x = 0; x < this->localWidth; x++)
		{
			pixelArray[y * localWidth + x] = temp;
		}
	}

	this->textureSubResourceData.pSysMem = pixelArray;
	this->textureSubResourceData.SysMemPitch = 4 * localWidth;
	HRESULT hr = Device_ptr->CreateTexture2D(&this->texture2DDesc, &this->textureSubResourceData, &this->gBuffer);
	assert(SUCCEEDED(hr));
}

void Gbuffer::CreateRenderTargetView(ID3D11Device* Device_ptr)
{
	renderTargetViewDesc.Format = texture2DDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	HRESULT hr = Device_ptr->CreateRenderTargetView(this->gBuffer, &this->renderTargetViewDesc, &this->renderTargetView);
	assert(SUCCEEDED(hr));
}

void Gbuffer::CreateShaderResourceView(ID3D11Device* Device_ptr)
{
	shaderResourceViewDesc.Format = texture2DDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	HRESULT hr = Device_ptr->CreateShaderResourceView(this->gBuffer, &this->shaderResourceViewDesc, &this->shaderResourceView);
	assert(SUCCEEDED(hr));
	
}

ID3D11RenderTargetView* &Gbuffer::getRTView()
{
	return this->renderTargetView;
}

ID3D11ShaderResourceView* &Gbuffer::getSRView()
{
	return this->shaderResourceView;
}
