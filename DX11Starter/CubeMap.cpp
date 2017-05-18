#include "CubeMap.h"



CubeMap::CubeMap()
{
	
}


CubeMap::~CubeMap()
{

	skyboxSRV->Release();
	delete skyboxVS;
	delete skyboxPS;
}

void CubeMap::DrawSkybox(ID3D11DeviceContext* context, Camera* camera, ID3D11SamplerState* sampler) {
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	//*/
	//render sky, must occur after all solid objects
	ID3D11Buffer* skyboxVB = cube->GetVertexBuffer();
	ID3D11Buffer* skyboxIB = cube->GetIndexBuffer();


	context->IASetVertexBuffers(0, 1, &skyboxVB, &stride, &offset);
	context->IASetIndexBuffer(skyboxIB, DXGI_FORMAT_R32_UINT, 0);

	skyboxVS->SetMatrix4x4("view", camera->GetViewMatrix());
	skyboxVS->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	skyboxVS->CopyAllBufferData();
	skyboxVS->SetShader();

	skyboxPS->SetShaderResourceView("Skybox", skyboxSRV);
	skyboxPS->SetSamplerState("Sampler", sampler);
	skyboxPS->CopyAllBufferData();
	skyboxPS->SetShader();

	context->RSSetState(rsSkybox);
	context->OMSetDepthStencilState(dsSkybox, 0);
	context->DrawIndexed(cube->GetIndexCount(), 0, 0);

	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
}

//Getters and Setters
Mesh* CubeMap::GetMesh() {
	return cube;
}
void CubeMap::SetMesh(Mesh* m) {
	cube = m;
}

ID3D11ShaderResourceView* CubeMap::GetResourceView() {
	return skyboxSRV;
}
void CubeMap::SetResourceView(ID3D11ShaderResourceView* srv) {
	skyboxSRV = srv;
}
ID3D11RasterizerState* CubeMap::GetRasterizerState() {
	return rsSkybox;
}
void CubeMap::SetRasterizerState(ID3D11RasterizerState* rs) {
	rsSkybox = rs;
}
ID3D11DepthStencilState* CubeMap::GetStencilState() {
	return dsSkybox;
}
void CubeMap::SetStencilState(ID3D11DepthStencilState* dss) {
	dsSkybox = dss;
}

SimpleVertexShader* CubeMap::GetSVS() {
	return skyboxVS;
}
void CubeMap::SetSVS(SimpleVertexShader* svs) {
	skyboxVS = svs;
}
SimplePixelShader* CubeMap::GetSPS() {
	return skyboxPS;
}
void CubeMap::SetSPS(SimplePixelShader* sps) {
	skyboxPS = sps;
}