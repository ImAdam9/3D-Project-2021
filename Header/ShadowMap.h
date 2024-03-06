#ifndef SHADOWMAP_H
#define SHADOWMAP_H
#include <windows.h>
#include <d3d11.h>
#include <assert.h>
class ShadowMap
{
private:
	UINT width;
	UINT height;
	D3D11_VIEWPORT viewPort;
	ID3D11RenderTargetView* renderTargetView;
	ID3D11ShaderResourceView* shaderResource;
	ID3D11DepthStencilView* depthStencil;
public:
	ShadowMap(ID3D11Device* device, UINT width, UINT height);
	~ShadowMap();

	ID3D11ShaderResourceView*& DepthMapSRV();
	void BindTargets(ID3D11DeviceContext* deviceContext, ID3D11VertexShader* vertexShader);
};
#endif