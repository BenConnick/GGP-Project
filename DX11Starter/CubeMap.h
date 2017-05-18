#pragma once
#include "Mesh.h"
#include "Material.h"
#include "Lights.h"
#include "Camera.h"
#include <DirectXMath.h>

#pragma once
class CubeMap
{
public:
	CubeMap();
	~CubeMap();

	void DrawSkybox(ID3D11DeviceContext* context, Camera* camera, ID3D11SamplerState* sampler);

	//Getters and Setters
	Mesh* GetMesh();
	void SetMesh(Mesh* cube);

	ID3D11ShaderResourceView* GetResourceView();
	void SetResourceView(ID3D11ShaderResourceView* srv);
	ID3D11RasterizerState* GetRasterizerState();
	void SetRasterizerState(ID3D11RasterizerState* rs);
	ID3D11DepthStencilState* GetStencilState();
	void SetStencilState(ID3D11DepthStencilState* dss);
	SimpleVertexShader* GetSVS();
	void SetSVS(SimpleVertexShader* svs);
	SimplePixelShader* GetSPS();
	void SetSPS(SimplePixelShader* sps);
private:
	Mesh* cube; //Cube maps are generally implied to be cubes
	ID3D11ShaderResourceView* skyboxSRV;
	ID3D11RasterizerState*  rsSkybox;
	ID3D11DepthStencilState* dsSkybox;
	SimpleVertexShader* skyboxVS;
	SimplePixelShader* skyboxPS;
};

