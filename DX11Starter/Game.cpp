#include "Game.h"
#include "Vertex.h"
#include "WICTextureLoader.h"
#include "Recycler.h"
#include <fmod_errors.h>

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
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
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

	song->release();
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

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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

	CreateWICTextureFromFile(device, context, L"Assets/Textures/metal.jpg", 0, &metalTex);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/wood.jpg", 0, &woodTex);

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

	Material* defMaterial = new Material(vertexShader, pixelShader, metalTex, sampler);
	materials.push_back(defMaterial);
	Material* woodMaterial = new Material(vertexShader, pixelShader, woodTex, sampler);
	materials.push_back(woodMaterial);

	Entity* playerEnt = new Entity(car, defMaterial);
	playerEnt->SetScale({ 0.5f, 0.5f, 0.5f });
	playerEnt->Activate();
	Rail** rails = new Rail*[Player::railCount];
	for (int i = 0; i < Player::railCount; i++) {
		Entity* railEnt = new Entity(cube, defMaterial);
		railEnt->SetPosition({1*i - 1.0f,-1.0f,0.0f});
		railEnt->SetScale({ 0.1f, 0.1f, 200.0f });
		railEnt->Activate();
		rails[i] = new Rail(railEnt);
		entities.push_back(railEnt);
	}
	player = new Player(playerEnt, rails);
	entities.push_back(playerEnt);
	/*
	nodeManager = new MusicNodeManager(rails, cube, woodMaterial);
	for (int j = 1; j < 7; j++) {
		Entity* nodeEnt = new Entity(cube, woodMaterial);
		nodeManager->AddNode(nodeEnt, j % 3, j*1.0f);
		nodeEnt->SetScale({ 0.5f,0.5f,0.5f });

		nodeEnt->Activate();
		entities.push_back(nodeEnt);
	}
	*/
	///*
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
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	bool songNotStarted;
	songChannel->getPaused(&songNotStarted);

	if (totalTime >= 5.0f && songNotStarted) {
		songChannel->setPaused(false);
	}

	float sinTime = abs(sinf(totalTime));
	float cosTime = abs(cosf(totalTime));

	// timer
	myTimer += deltaTime;

	// move notes
	for (int i = 0; i < noteMarkers.size(); i++) {
		if (noteMarkers[i]->IsActive()) {
			XMFLOAT3 p = noteMarkers[i]->GetPosition();
			noteMarkers[i]->SetPosition(XMFLOAT3(p.x,p.y,p.z - deltaTime * 100));
			// remove old
			if (noteMarkers[i]->GetPosition().z < 0) {
				if (player->GetRail() == noteMarkers[i]->GetPosition().x+1) {
					cout << "note hit on rail " << player->GetRail() << "! ";
				}
				Recycler::GetInstance().Deactivate(noteMarkers.back());
				noteMarkers.pop_back();
			}
		}
	}


	player->Update();
	//nodeManager->Update(deltaTime);

	int numNotes = parser.GetMeasure(parser.measureNum)->size();
	float secPerBeat = 4*60.0 / parser.BPMS;

	// create entities dynamically
	float max = secPerBeat / numNotes;
	if (myTimer > max) {
		counter++;
		if (counter >= numNotes) {
			counter = 0;
			parser.measureNum++;
		}
		myTimer -= max;
		
		// FOR DEMONSTRATION ONLY
		int value = parser.GetNote(parser.measureNum, counter);
		if (value > -1) {
			// new at front
			Entity* e = Recycler::GetInstance().Reactivate();
			noteMarkers.insert(noteMarkers.begin(), e);
			e->SetPosition(XMFLOAT3(value - 1, -1, 100));
		}
	}
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	for (auto entity : entities) {
		if (!entity->IsActive()) continue;
		Mesh* mesh = entity->GetMesh();
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		ID3D11Buffer* vb = mesh->GetVertexBuffer();
		context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
		context->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
		entity->PrepareMaterial(camera->GetViewMatrix(), camera->GetProjectionMatrix(), dirLight, dirLight2);
		context->DrawIndexed(mesh->GetIndexCount(), 0, 0);
	}



	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);
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