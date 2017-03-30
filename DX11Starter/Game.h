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
#include <fmod_common.h>
#include <fmod.hpp>
#include "Player.h"
#include "Rail.h"

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

	// Overridden mouse input helper methods
	void OnMouseDown (WPARAM buttonState, int x, int y);
	void OnMouseUp	 (WPARAM buttonState, int x, int y);
	void OnMouseMove (WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta,   int x, int y);
private:

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders(); 
	void CreateMatrices();
	void CreateBasicGeometry();

	// Meshes!
	std::vector<Mesh*> meshes;
	std::vector<Entity*> entities;
	std::vector<Material*> materials;

	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;
	ID3D11SamplerState* sampler;

	// The matrices to go from model space to screen space
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;

	Camera* camera;

	Player* player;

	// FMOD handles
	FMOD::System* system;
	FMOD::Sound* song;
	FMOD::Channel* songChannel;

	DirectionalLight dirLight;
	DirectionalLight dirLight2;

	// temporary
	float myTimer = 0.0;
	int counter = 0;
	vector<Entity*> noteMarkers;
	SMParser parser = SMParser();
	// ----
};

