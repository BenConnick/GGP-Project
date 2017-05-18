#pragma once

#include "DXCore.h"
#include "SimpleShader.h"
#include "Mesh.h"
#include "Entity.h"
#include "Camera.h"
#include "Lights.h"
#include "Material.h"
#include "SMParser.h"
#include <DirectXMath.h>
#include <vector>
#include <fmod.h>
#include <fmod_common.h>
#include <fmod.hpp>
#include "Player.h"
#include "Rail.h"
#include "MusicNodeManager.h"
#include "ParticleEmitter.h"

class Game
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void RenderDepthBuffer(float* freqs, float deltaTime, float totalTime);

	// Overridden mouse input helper methods
	void OnMouseDown(WPARAM buttonState, int x, int y);
	void OnMouseUp(WPARAM buttonState, int x, int y);
	void OnMouseMove(WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta, int x, int y);
private:

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders();
	void CreateMatrices();
	void CreateBasicGeometry();

	// Meshes!
	std::vector<Mesh*> meshes;
	std::vector<Entity*> entities;
	std::vector<Material*> materials;
	Entity* terrainL;
	Entity* terrainR;

	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;
	ID3D11SamplerState* sampler;

	ID3D11RenderTargetView* dofRTV;
	ID3D11ShaderResourceView* dofSRV;
	ID3D11RenderTargetView* dofBlurRTV;
	ID3D11ShaderResourceView* dofBlurSRV;
	ID3D11DepthStencilView* depthDSV;
	ID3D11ShaderResourceView* depthSRV;
	ID3D11RasterizerState* depthRS;
	SimpleVertexShader* depthVS;
	SimpleVertexShader* dofVS;
	SimplePixelShader* dofPS;
	SimplePixelShader* dofBlurPS;

	SimpleVertexShader* terrainVS;

	Mesh* skybox;
	ID3D11ShaderResourceView* skyboxSRV;
	ID3D11RasterizerState*  rsSkybox;
	ID3D11DepthStencilState* dsSkybox;
	SimpleVertexShader* skyboxVS;
	SimplePixelShader* skyboxPS;

	// The matrices to go from model space to screen space
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;

	Camera* camera;

	Player* player;
	MusicNodeManager* nodeManager;

	// FMOD handles
	FMOD::System* system;
	FMOD::ChannelGroup* mastergroup;
	FMOD::Sound* song;
	FMOD::Channel* songChannel;
	FMOD::DSP* dsp;
	FMOD_DSP_PARAMETER_FFT* fft;

	DirectionalLight dirLight;
	DirectionalLight dirLight2;

	// temporary
	float myTimer = 0.0;
	int counter = 0;
	vector<Entity*> noteMarkers;
	SMParser parser = SMParser();
	// ----

	// Effects
	ID3D11RenderTargetView* ppRenderTargetView;
	ID3D11ShaderResourceView* ppsrv;
	SimpleVertexShader* ppVS;
	SimplePixelShader* ppPS;

	// Particle stuff
	ID3D11ShaderResourceView* particleTexture;
	ID3D11BlendState* particleBlendState;
	ID3D11DepthStencilState* particleDepthState;
	Emitter* simpleEmitter;
	SimpleVertexShader* particleVS;
	SimplePixelShader* particlePS;
	SimpleGeometryShader* particleGS;
};

