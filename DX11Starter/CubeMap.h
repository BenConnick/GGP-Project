#pragma once
#include "Mesh.h"
#include "Material.h"
#include "Lights.h"
#include <DirectXMath.h>

#pragma once
class CubeMap
{
public:
	CubeMap();
	~CubeMap();
private:
	Mesh* cube;
	ID3D11ShaderResourceView* skyboxSRV;
	ID3D11RasterizerState*  rsSkybox;
	ID3D11DepthStencilState* dsSkybox;
	SimpleVertexShader* skyboxVS;
	SimplePixelShader* skyboxPS;
};

