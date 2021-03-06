#include "Game.h"
#include "Vertex.h"
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"
#include "Recycler.h"
#include <fmod_errors.h>
#include "ParticleManager.h"

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1920,			   // Width of the window's client area
		1017,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	// Initialize fields
	meshes = std::vector<Mesh*>();
	entities = std::vector<Entity*>();
	materials = std::vector<Material*>();
	vertexShader = 0;
	pixelShader = 0;
	camera = new Camera(width, height);
	skybox = new CubeMap();
	Entity::activeSkybox = skybox;

	FMOD_RESULT res;
	res = FMOD::System_Create(&system);
	if (res != FMOD_OK)
	{
		printf("FMOD error! (%d) %s\n", res, FMOD_ErrorString(res));
		exit(-1);
	}

	res = system->init(512, FMOD_INIT_NORMAL, 0);    // Initialize FMOD.
	if (res != FMOD_OK)
	{
		printf("FMOD error! (%d) %s\n", res, FMOD_ErrorString(res));
		exit(-1);
	}

	res = system->createStream("Assets/Sounds/Goin' Under.ogg", FMOD_CREATESTREAM, nullptr, &song);
	if (res != FMOD_OK)
	{
		printf("FMOD error! (%d) %s\n", res, FMOD_ErrorString(res));
		exit(-1);
	}
	system->getMasterChannelGroup(&mastergroup);

	res = system->playSound(song, nullptr, true, &songChannel);
	songChannel->setVolume(0.5f);

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	for (auto mesh : meshes) {
		delete mesh;
	}

	for (auto entity : entities) {
		delete entity;
	}

	for (auto material : materials) {
		delete material;
	}

	delete camera;
	delete player;
	delete nodeManager;

	sampler->Release();

	delete vertexShader;
	delete pixelShader;
	delete simpleEmitter;

	delete skybox;

	ppsrv->Release();
	ppRenderTargetView->Release();
	delete ppVS;
	delete ppPS;

	//delete fft;

	dsp->release();
	song->release();
	mastergroup->release();
	system->release();

	entities.~vector();
	meshes.~vector();
	materials.~vector();
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateMatrices();
	CreateBasicGeometry();
	dirLight.AmbientColor = XMFLOAT4(0.1, 0.1, 0.1, 1);
	dirLight.DiffuseColor = XMFLOAT4(0, 0.5, 0.5, 1);
	dirLight.Direction = XMFLOAT3(1, -1, 0);

	dirLight2 = { XMFLOAT4(0.1, 0.1, 0.1, 1), XMFLOAT4(1,0.56,0.85,1), XMFLOAT3(-1, 1, 0) };

	// Particle states ------------------------

	// Blend state
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; // ADDITIVE blending
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&blendDesc, &particleBlendState);

	// Depth state
	D3D11_DEPTH_STENCIL_DESC depthDesc = {};
	depthDesc.DepthEnable = true;
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	device->CreateDepthStencilState(&depthDesc, &particleDepthState);

	// create the particle emitters
	simpleEmitter = new Emitter(device, particleVS, particlePS, particleGS, particleTexture, sampler, particleBlendState, particleDepthState);
	ParticleManager::GetInstance().AttachEmitter(simpleEmitter);

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11ShaderResourceView* skySRV = (skybox->GetResourceView());
	HRESULT hr = CreateDDSTextureFromFile(device, L"Assets/Textures/EmptySpace.dds", 0, &skySRV);
	skybox->SetResourceView(skySRV);

	//printf(hr);
	//CreateWICTextureFromFile(device, L"Assets/Textures/skybox.dds", 0, &skyboxSRV);

	// Create a sampler state for texture sampling
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Ask the device to create a state
	device->CreateSamplerState(&samplerDesc, &sampler);

	D3D11_RASTERIZER_DESC rsDesc = {};
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_FRONT;
	rsDesc.DepthClipEnable = true;
	ID3D11RasterizerState* rs;
	device->CreateRasterizerState(&rsDesc, &rs);
	skybox->SetRasterizerState(rs);

	// post process effects
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	// set up render target
	ID3D11Texture2D* ppTexture;
	device->CreateTexture2D(&textureDesc, 0, &ppTexture);

	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	device->CreateRenderTargetView(ppTexture, &rtvDesc, &ppRenderTargetView);

	// Create the Shader Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	device->CreateShaderResourceView(ppTexture, &srvDesc, &ppsrv);

	// We don't need the texture reference itself no mo'
	ppTexture->Release();

	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	ID3D11DepthStencilState* dss;
	device->CreateDepthStencilState(&dsDesc, &dss);
	skybox->SetStencilState(dss);

	ID3D11Texture2D* dofTexture;
	device->CreateTexture2D(&textureDesc, 0, &dofTexture);
	ID3D11Texture2D* dofBlurTexture;
	device->CreateTexture2D(&textureDesc, 0, &dofBlurTexture);

	device->CreateRenderTargetView(dofTexture, &rtvDesc, &dofRTV);
	device->CreateRenderTargetView(dofBlurTexture, &rtvDesc, &dofBlurRTV);

	device->CreateShaderResourceView(dofTexture, &srvDesc, &dofSRV);
	device->CreateShaderResourceView(dofBlurTexture, &srvDesc, &dofBlurSRV);
	dofTexture->Release();
	dofBlurTexture->Release();

	D3D11_TEXTURE2D_DESC depthTexDesc = {};
	depthTexDesc.Width = width;
	depthTexDesc.Height = height;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.SampleDesc.Count = 1;
	depthTexDesc.SampleDesc.Quality = 0;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	ID3D11Texture2D* depthBuffer;
	device->CreateTexture2D(&depthTexDesc, 0, &depthBuffer);

	D3D11_DEPTH_STENCIL_VIEW_DESC dofDepthDesc = {};
	dofDepthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dofDepthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dofDepthDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(depthBuffer, &dofDepthDesc, &depthDSV);

	D3D11_SHADER_RESOURCE_VIEW_DESC depthSrvDesc = {};
	depthSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	depthSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	depthSrvDesc.Texture2D.MipLevels = 1;
	depthSrvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(depthBuffer, &depthSrvDesc, &depthSRV);

	depthBuffer->Release();

	D3D11_RASTERIZER_DESC depthRastDesc = {};
	depthRastDesc.FillMode = D3D11_FILL_SOLID;
	depthRastDesc.CullMode = D3D11_CULL_BACK;
	depthRastDesc.DepthClipEnable = true;
	depthRastDesc.DepthBias = 1000;
	depthRastDesc.DepthBiasClamp = 0.0f;
	depthRastDesc.SlopeScaledDepthBias = 1.0f;
	device->CreateRasterizerState(&depthRastDesc, &depthRS);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Effects
	/*ppRenderTarget = new ID3D11Texture2D();
	renderTargetView;
	ppsrv;
	*/
	// load song beatmap, print success
	cout << "songs loaded: " << parser.OpenFile("Assets/Beatmaps/song.sm");
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files using
// my SimpleShader wrapper for DirectX shader manipulation.
// - SimpleShader provides helpful methods for sending
//   data to individual variables on the GPU
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = new SimpleVertexShader(device, context);
	if (!vertexShader->LoadShaderFile(L"x64/Debug/VertexShader.cso"))
		vertexShader->LoadShaderFile(L"VertexShader.cso");

	pixelShader = new SimplePixelShader(device, context);
	if (!pixelShader->LoadShaderFile(L"x64/Debug/PixelShader.cso"))
		pixelShader->LoadShaderFile(L"PixelShader.cso");

	// Load particle shaders
	particleVS = new SimpleVertexShader(device, context);
	if (!particleVS->LoadShaderFile(L"x64/Debug/ParticleVS.cso"))
		particleVS->LoadShaderFile(L"ParticleVS.cso");

	particlePS = new SimplePixelShader(device, context);
	if (!particlePS->LoadShaderFile(L"x64/Debug/ParticlePS.cso"))
		particlePS->LoadShaderFile(L"ParticlePS.cso");

	particleGS = new SimpleGeometryShader(device, context);
	if (!particleGS->LoadShaderFile(L"x64/Debug/ParticleGS.cso"))
		particleGS->LoadShaderFile(L"ParticleGS.cso");
	particleGS->SetFloat("pixelWidth", 1.0f / width);
	particleGS->SetFloat("pixelHeight", 1.0f / height);

	terrainVS = new SimpleVertexShader(device, context);
	if (!terrainVS->LoadShaderFile(L"x64/Debug/TerrainVS.cso"))
		terrainVS->LoadShaderFile(L"x64/Debug/TerrainVS.cso");
	terrainPS = new SimplePixelShader(device, context);
	if (!terrainPS->LoadShaderFile(L"x64/Debug/ScrollingTexturePS.cso"))
		terrainPS->LoadShaderFile(L"ScrollingTexturePS.cso");

	SimpleVertexShader* skyboxVS = new SimpleVertexShader(device, context);
	if (!skyboxVS->LoadShaderFile(L"x64/Debug/SkyboxVS.cso"))
		skyboxVS->LoadShaderFile(L"SkyboxVS.cso");
	SimplePixelShader* skyboxPS = new SimplePixelShader(device, context);
	if (!skyboxPS->LoadShaderFile(L"x64/Debug/SkyboxPS.cso")) 
		skyboxPS->LoadShaderFile(L"SkyboxPS.cso");
	skybox->SetSVS(skyboxVS);
	skybox->SetSPS(skyboxPS);

	ppVS = new SimpleVertexShader(device, context);
	if (!ppVS->LoadShaderFile(L"x64/Debug/PPVS.cso"))
		ppVS->LoadShaderFile(L"PPVS.cso");
	ppPS = new SimplePixelShader(device, context);
	if (!ppPS->LoadShaderFile(L"x64/Debug/PPPS.cso"))
		ppPS->LoadShaderFile(L"PPPS.cso");

	depthVS = new SimpleVertexShader(device, context);
	if (!depthVS->LoadShaderFile(L"x64/Debug/DepthVS.cso"))
		depthVS->LoadShaderFile(L"DepthVS.cso");
	dofVS = new SimpleVertexShader(device, context);
	if (!dofVS->LoadShaderFile(L"x64/Debug/DepthOfFieldVS.cso"))
		dofVS->LoadShaderFile(L"DepthOfFieldVS.cso");
	dofPS = new SimplePixelShader(device, context);
	if (!dofPS->LoadShaderFile(L"x64/Debug/DepthOfFieldPS.cso"))
		dofPS->LoadShaderFile(L"DepthOfFieldPS.cso");
	dofBlurPS = new SimplePixelShader(device, context);
	if (!dofBlurPS->LoadShaderFile(L"x64/Debug/DepthOfFieldBlurPS.cso"))
		dofBlurPS->LoadShaderFile(L"DepthOfFieldBlurPS.cso");

	// You'll notice that the code above attempts to load each
	// compiled shader file (.cso) from two different relative paths.

	// This is because the "working directory" (where relative paths begin)
	// will be different during the following two scenarios:
	//  - Debugging in VS: The "Project Directory" (where your .cpp files are) 
	//  - Run .exe directly: The "Output Directory" (where the .exe & .cso files are)

	// Checking both paths is the easiest way to ensure both 
	// scenarios work correctly, although others exist
}



// --------------------------------------------------------
// Initializes the matrices necessary to represent our geometry's 
// transformations and our 3D camera
// --------------------------------------------------------
void Game::CreateMatrices()
{
	// Set up world matrix
	// - In an actual game, each object will need one of these and they should
	//   update when/if the object moves (every frame)
	// - You'll notice a "transpose" happening below, which is redundant for
	//   an identity matrix.  This is just to show that HLSL expects a different
	//   matrix (column major vs row major) than the DirectX Math library
	XMMATRIX W = XMMatrixIdentity();
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(W)); // Transpose for HLSL!

	// Create the View matrix
	// - In an actual game, recreate this matrix every time the camera 
	//    moves (potentially every frame)
	// - We're using the LOOK TO function, which takes the position of the
	//    camera and the direction vector along which to look (as well as "up")
	// - Another option is the LOOK AT function, to look towards a specific
	//    point in 3D space
	XMVECTOR pos = XMVectorSet(0, 0, -5, 0);
	XMVECTOR dir = XMVectorSet(0, 0, 1, 0);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX V = XMMatrixLookToLH(
		pos,     // The position of the "camera"
		dir,     // Direction the camera is looking
		up);     // "Up" direction in 3D space (prevents roll)
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V)); // Transpose for HLSL!

	// Create the Projection matrix
	// - This should match the window's aspect ratio, and also update anytime
	//   the window resizes (which is already happening in OnResize() below)
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,		// Field of View Angle
		(float)width / height,		// Aspect ratio
		0.1f,						// Near clip plane distance
		100.0f);					// Far clip plane distance
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!
}


// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	D3D11_SAMPLER_DESC desc = { D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		0,
		16,
		D3D11_COMPARISON_EQUAL,
		{0,0,0,0},
		0.0f,
		D3D11_FLOAT32_MAX };

	device->CreateSamplerState(&desc, &sampler);

	ID3D11ShaderResourceView* metalTex;
	ID3D11ShaderResourceView* woodTex;
	ID3D11ShaderResourceView* terrainTex;
	ID3D11ShaderResourceView* carTex;

	CreateWICTextureFromFile(device, context, L"Assets/Textures/wheel2.bmp", 0, &carTex);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/metal.jpg", 0, &metalTex);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/wood.jpg", 0, &woodTex);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/SimpleParticle.jpg", 0, &particleTexture);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/sand-texture.jpg", 0, &terrainTex);

	Mesh* cone = new Mesh("Assets/Models/cone.obj", device);
	meshes.push_back(cone);
	Mesh* cube = new Mesh("Assets/Models/cube.obj", device);
	meshes.push_back(cube);
	Mesh* cylinder = new Mesh("Assets/Models/cylinder.obj", device);
	meshes.push_back(cylinder);
	Mesh* helix = new Mesh("Assets/Models/helix.obj", device);
	meshes.push_back(helix);
	Mesh* sphere = new Mesh("Assets/Models/sphere.obj", device);
	meshes.push_back(sphere);
	Mesh* torus = new Mesh("Assets/Models/torus.obj", device);
	meshes.push_back(torus);
	Mesh* car = new Mesh("Assets/Models/Porsche_911_GT2.obj", device);
	meshes.push_back(car);

	skybox->SetMesh(cube);
  
	Mesh* terrainMesh = new Mesh(8, 8, device);
	meshes.push_back(terrainMesh);


	Material* defMaterial = new Material(vertexShader, pixelShader, carTex, sampler);
	materials.push_back(defMaterial);
	Material* playerMaterial = new Material(vertexShader, pixelShader, carTex, sampler);
	playerMaterial->SetReflective(0.2f);
	materials.push_back(playerMaterial);
	Material* woodMaterial = new Material(vertexShader, pixelShader, woodTex, sampler);
	materials.push_back(woodMaterial);
	Material* dynMaterial = new Material(terrainVS, terrainPS, terrainTex, sampler);
	materials.push_back(dynMaterial);

	//testCube1 = new Entity(sphere, dynMaterial);
	//testCube2 = new Entity(sphere, dynMaterial);
	//testCube1->SetPosition({ -1.0f, 1.0f, 1.0f });
	//testCube2->SetPosition({ 1.0f,1.0f,1.0f });

	terrainL = new Entity(terrainMesh, dynMaterial);
	terrainL->SetPosition({ -17.4f, -2.0f, 0.0f });
	terrainL->SetRotation({ 0.0f, 0.0f, 0.0f });
	terrainL->SetScale({ 5.0f, 5.0f, 25.0f });
	terrainL->Activate();
	
	terrainR = new Entity(terrainMesh, dynMaterial);
	terrainR->SetRotation({ 0.0f, 3.14f, 0.0f });
	terrainR->SetPosition({ +17.4f, -2.0f, 0.0f });
	terrainR->SetScale({ 5.0f, 5.0f, 25.0f });
	terrainR->Activate();

	Entity* playerEnt = new Entity(car, playerMaterial);
  
	playerEnt->Activate();
	//RailSet* rs = new RailSet(cube,defMaterial,&entities);

	std::vector<XMFLOAT3> railPositions;
	float height = -1;
	railPositions.push_back(XMFLOAT3(-1,height,0));
	railPositions.push_back(XMFLOAT3(0, height, 0));
	railPositions.push_back(XMFLOAT3(1, height, 0));
	player = new Player(playerEnt, railPositions);
	entities.push_back(playerEnt);
  
	nodeManager = new MusicNodeManager(player, railPositions, cube, woodMaterial,&entities,&parser, camera);
	///*
	for (int j = 1; j < 7; j++) {
		//Entity* nodeEnt = new Entity(cube, woodMaterial);
		nodeManager->AddNode(j% 3, j*1.0f);
	}
	//*/
	/*
	// create a dozen test objects
	for (int i = 0; i < 12; i++) {
		// create a new entity
		entities.push_back(new Entity(meshes[0], materials[0]));
		// put it in the recycler
		Recycler::GetInstance().Deactivate(entities[entities.size() - 1]);
	}//*/
}


// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	camera->OnResize(width, height);

	//// Update our projection matrix since the window size changed
	//XMMATRIX P = XMMatrixPerspectiveFovLH(
	//	0.25f * 3.1415926535f,	// Field of View Angle
	//	(float)width / height,	// Aspect ratio
	//	0.1f,				  	// Near clip plane distance
	//	100.0f);			  	// Far clip plane distance
	//XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	system->update();
	camera->Update(deltaTime);
	simpleEmitter->Update(deltaTime);
	ParticleManager::GetInstance().Update(deltaTime);
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	bool songNotStarted;
	songChannel->getPaused(&songNotStarted);
	FMOD_RESULT res;
	float dfft;

	if (totalTime >= 5.0f && songNotStarted) {
		songChannel->setPaused(false);
		system->createDSPByType(FMOD_DSP_TYPE_FFT, &dsp);
		mastergroup->addDSP(0, dsp);
		dsp->setActive(true);
		dsp->setParameterInt(FMOD_DSP_FFT_WINDOWTYPE, FMOD_DSP_FFT_WINDOW_TRIANGLE);
		dsp->setParameterInt(FMOD_DSP_FFT_WINDOWSIZE, 128);
	}
	if (totalTime >= 15.0f) {
		//DebugBreak();
	}

	float sinTime = abs(sinf(totalTime));
	float cosTime = abs(cosf(totalTime));


	player->Update(deltaTime);
	nodeManager->Update(deltaTime);

}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	float freqs[64];
	memset(freqs, 0, sizeof(float) * 64);

	bool songNotStarted;
	songChannel->getPaused(&songNotStarted);
	//get some song data
	if (!songNotStarted && totalTime >= 6.0f) {
		dsp->getParameterData(FMOD_DSP_FFT_SPECTRUMDATA, (void**)&fft, 0, 0, 0);
		for (int i = 0; i < 64; i++) {
			if (fft->spectrum[0] == nullptr) break;
			freqs[i] = fft->spectrum[0][i];
		}
	}

	RenderDepthBuffer(freqs, deltaTime, totalTime);

	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	context->OMSetRenderTargets(1, &dofRTV, depthStencilView);
	context->ClearRenderTargetView(dofRTV, color);
	
	const UINT stride = sizeof(Vertex);
	const UINT offset = 0;

	for (auto entity : entities) {
		if (!entity->IsActive()) continue;
		Mesh* mesh = entity->GetMesh();
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		ID3D11Buffer* vb = mesh->GetVertexBuffer();
		context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
		context->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
		entity->PrepareMaterial(camera->GetViewMatrix(), camera->GetProjectionMatrix(), dirLight, dirLight2, camera->GetPosition());
		context->DrawIndexed(mesh->GetIndexCount(), 0, 0);
	}
	terrainPS->SetFloat("time", totalTime);
	terrainR->SetRotation({ 0.0f, 3.14f, 0.0f });
	terrainR->SetPosition({ +17.4f, -2.0f, 0.0f });
	terrainR->SetScale({ 5.0f, 5.0f, 25.0f });
	terrainL->SetPosition({ -17.4f, -2.0f, 0.0f });
	terrainL->SetRotation({ 0.0f, 0.0f, 0.0f });
	terrainL->SetScale({ 5.0f, 5.0f, 25.0f });
	Mesh* mesh = terrainL->GetMesh();
	ID3D11Buffer* vb = mesh->GetVertexBuffer();
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	context->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	terrainL->PrepareTerrainMaterial(camera->GetViewMatrix(), camera->GetProjectionMatrix(), freqs, 64, dirLight, dirLight2);
	terrainPS->SetFloat("speed", -0.15f);
	context->DrawIndexed(mesh->GetIndexCount(), 0, 0);
	terrainR->PrepareTerrainMaterial(camera->GetViewMatrix(), camera->GetProjectionMatrix(), freqs, 64, dirLight, dirLight2);
	terrainPS->SetFloat("speed", 0.15f);
	context->DrawIndexed(mesh->GetIndexCount(), 0, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	skybox->DrawSkybox(context, camera, sampler);

	// Draw particles
	simpleEmitter->Draw(context, camera, deltaTime, totalTime);

	// Turn off vertex and index buffers
	context->OMSetRenderTargets(1, &dofBlurRTV, 0);
	dofVS->SetShader();
	
	dofBlurPS->SetShader();
	dofBlurPS->SetShaderResourceView("Pixels", dofSRV);
	dofBlurPS->SetSamplerState("Sampler", sampler);
	dofBlurPS->SetFloat("pixelWidth", 1.0f / width);
	dofBlurPS->SetFloat("pixelHeight", 1.0f / height);
	dofBlurPS->SetInt("blurAmount", 3);
	dofBlurPS->CopyAllBufferData();

	ID3D11Buffer* nothing = 0;
	context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	// Draw the post process (3 verts = 1 triangle to fill the screen)
	context->Draw(3, 0);
	
	dofBlurPS->SetShaderResourceView("Pixels", 0);

	context->OMSetRenderTargets(1, &ppRenderTargetView, 0);
	
	dofPS->SetShader();
	dofPS->SetShaderResourceView("Unblurred", dofSRV);
	dofPS->SetShaderResourceView("Blurred", dofBlurSRV);
	dofPS->SetShaderResourceView("DepthBuffer", depthSRV);
	dofPS->SetSamplerState("Sampler", sampler);
	dofPS->SetFloat("Distance", 1.75f);
	dofPS->SetFloat("Range", 2.0f);
	dofPS->SetFloat("Near", 0.5f);
	dofPS->SetFloat("Far", 5.0f);
	dofPS->CopyAllBufferData();

	context->Draw(3, 0);

	// draw output of bloom post process to screen ====================
	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);

	// Turn on VS (no args)
	ppVS->SetShader();

	// Turn on PS
	ppPS->SetShader();
	ppPS->SetShaderResourceView("Pixels", ppsrv);
	ppPS->SetSamplerState("Sampler", sampler);
	ppPS->SetFloat("pixelWidth", 1.0f / width);
	ppPS->SetFloat("pixelHeight", 1.0f / height);
	ppPS->SetInt("blurAmount", 5);
	ppPS->CopyAllBufferData();

	context->Draw(3, 0);

	// Unbind the post process SRV
	ppPS->SetShaderResourceView("Pixels", 0);

	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);
}

void Game::RenderDepthBuffer(float* freqs, float deltaTime, float totalTime) {
	context->OMSetRenderTargets(0, 0, depthDSV);
	context->ClearDepthStencilView(depthDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
	context->RSSetState(depthRS);

	depthVS->SetShader();
	depthVS->SetMatrix4x4("view", camera->GetViewMatrix());
	depthVS->SetMatrix4x4("projection", camera->GetProjectionMatrix());

	context->PSSetShader(0, 0, 0);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	for (auto entity : entities) {
		if (!entity->IsActive()) continue;
		Mesh* mesh = entity->GetMesh();
		ID3D11Buffer* vb = mesh->GetVertexBuffer();
		context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
		context->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
		entity->PrepareMaterial(camera->GetViewMatrix(), camera->GetProjectionMatrix(), dirLight, dirLight2,camera->GetPosition());
		context->DrawIndexed(mesh->GetIndexCount(), 0, 0);
	}

	Mesh* mesh = terrainL->GetMesh();
	ID3D11Buffer* vb = mesh->GetVertexBuffer();
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	context->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	terrainL->PrepareTerrainMaterial(camera->GetViewMatrix(), camera->GetProjectionMatrix(), freqs, 64, dirLight, dirLight2);
	context->DrawIndexed(mesh->GetIndexCount(), 0, 0);
	terrainR->PrepareTerrainMaterial(camera->GetViewMatrix(), camera->GetProjectionMatrix(), freqs, 64, dirLight, dirLight2);
	context->DrawIndexed(mesh->GetIndexCount(), 0, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	simpleEmitter->Draw(context, camera, deltaTime, totalTime);

	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);
	context->RSSetState(0);
}


#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	// Caputure the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	SetCapture(hWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
// --------------------------------------------------------
void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
// --------------------------------------------------------
void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...
	if (buttonState & 0x0001) {
		camera->Look(x - prevMousePos.x, y - prevMousePos.y);
	}

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;
}

// --------------------------------------------------------
// Helper method for mouse wheel scrolling.  
// WheelDelta may be positive or negative, depending 
// on the direction of the scroll
// --------------------------------------------------------
void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	// Add any custom code here...
}
#pragma endregion