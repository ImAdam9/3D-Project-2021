#ifndef GBUFFER_H
#define GBUFFER_H
#include <d3d11.h>
#include <assert.h>
#include <vector>
#include <DirectXMath.h>
class Gbuffer
{
private:
	ID3D11Texture2D* gBuffer;
	ID3D11RenderTargetView* renderTargetView;
	ID3D11ShaderResourceView* shaderResourceView;
	D3D11_TEXTURE2D_DESC texture2DDesc;
	D3D11_SUBRESOURCE_DATA textureSubResourceData;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	int localWidth;
	int localHeight;
public:
	Gbuffer(int width, int height);
	~Gbuffer();
	void MakeGbuffer(ID3D11Device* Device_ptr);
	void CreateRenderTargetView(ID3D11Device* Device_ptr);
	void CreateShaderResourceView(ID3D11Device* Device_ptr);
	ID3D11RenderTargetView* &getRTView();
	ID3D11ShaderResourceView* &getSRView();
};
#endif