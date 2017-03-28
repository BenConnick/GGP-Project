#pragma once
#include <d3d11.h>
#include "SimpleShader.h"

class Material
{
public:
	Material(SimpleVertexShader*, SimplePixelShader*, ID3D11ShaderResourceView*, ID3D11SamplerState*);
	~Material();
	SimpleVertexShader* GetVertexShader();
	SimplePixelShader* GetPixelShader();
	ID3D11ShaderResourceView* GetTexture();
	ID3D11SamplerState* GetSamplerState();
private:
	SimpleVertexShader* _vertexShader;
	SimplePixelShader* _pixelShader;
	ID3D11ShaderResourceView* _texture;
	ID3D11SamplerState* _sampler;
};

