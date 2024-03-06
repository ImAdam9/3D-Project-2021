#include "../Header/ShadowMap.h"

ShadowMap::ShadowMap(ID3D11Device* device, UINT width, UINT height)
{
	this->width = width;
	this->height = height;
	this->renderTargetView = NULL;
	this->shaderResource = NULL;
	this->depthStencil = NULL;

	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;
	viewPort.Width = static_cast<FLOAT>(width);
	viewPort.Height = static_cast<FLOAT>(height);
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;

	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	ID3D11Texture2D* depthMap = NULL;
	assert(SUCCEEDED(device->CreateTexture2D(&textureDesc, NULL, &depthMap)));

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	depthStencilViewDesc.Flags = 0;
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	assert(SUCCEEDED(device->CreateDepthStencilView(depthMap, &depthStencilViewDesc, &depthStencil)));
	
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	assert(SUCCEEDED(device->CreateShaderResourceView(depthMap, &shaderResourceViewDesc, &shaderResource)));

	depthMap->Release();
}

ShadowMap::~ShadowMap()
{
	this->shaderResource->Release();
	this->depthStencil->Release();
}

ID3D11ShaderResourceView*& ShadowMap::DepthMapSRV()
{
	return this->shaderResource;
}

void ShadowMap::BindTargets(ID3D11DeviceContext* deviceContext, ID3D11VertexShader* vertexShader)
{
	
	deviceContext->OMSetRenderTargets(1, &this->renderTargetView, depthStencil);
	deviceContext->VSSetShader(vertexShader, NULL, 0);
	deviceContext->GSSetShader(NULL, NULL, 0);
	deviceContext->PSSetShader(NULL, NULL, 0);
	deviceContext->RSSetViewports(1, &this->viewPort);
	deviceContext->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);
	
}
