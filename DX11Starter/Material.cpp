#include "Material.h"



Material::Material(SimpleVertexShader* vs, SimplePixelShader* ps, ID3D11ShaderResourceView* srv, ID3D11SamplerState* samp)
{
	_vertexShader = vs;
	_pixelShader = ps;
	_texture = srv;
	_sampler = samp;
}


Material::~Material()
{
	if (_texture != nullptr)
		_texture->Release();
	_texture = nullptr;
}

float Material::GetReflectivity() {
	return reflective;
}
void Material::SetReflective(float refl) {
	reflective = refl;
}
SimpleVertexShader * Material::GetVertexShader()
{
	return _vertexShader;
}

SimplePixelShader * Material::GetPixelShader()
{
	return _pixelShader;
}

ID3D11ShaderResourceView * Material::GetTexture()
{
	return _texture;
}

ID3D11SamplerState * Material::GetSamplerState()
{
	return _sampler;
}
