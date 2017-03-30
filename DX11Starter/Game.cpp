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

	Material* defMaterial = new Material(vertexShader, pixelShader, metalTex, sampler);
	materials.push_back(defMaterial);
	Material* woodMaterial = new Material(vertexShader, pixelShader, woodTex, sampler);
	materials.push_back(woodMaterial);

	Entity* ent1 = new Entity(cone, defMaterial);
	//entities.push_back(ent1);

	Entity* ent2 = new Entity(cube, woodMaterial);
	ent2->SetPosition(XMFLOAT3(-2, 0, 0));
	//entities.push_back(ent2);

	Entity* ent3 = new Entity(cylinder, woodMaterial);
	ent3->SetPosition(XMFLOAT3(2, 0, 0));
	//entities.push_back(ent3);

	Entity* ent4 = new Entity(helix, woodMaterial);
	ent4->SetPosition(XMFLOAT3(0, 2, 0));
	//entities.push_back(ent4);

	Entity* ent5 = new Entity(sphere, defMaterial);
	ent5->SetPosition(XMFLOAT3(0, -2, 0));
	//entities.push_back(ent5);

	Entity* ent6 = new Entity(torus, defMaterial);
	ent6->SetPosition(XMFLOAT3(0, 0, 2));
	//entities.push_back(ent6);

	// create a dozen test objects
	for (int i = 0; i < 12; i++) {
		// create a new entity
		entities.push_back(new Entity(meshes[0], materials[0]));
		// put it in the recycler
		Recycler::GetInstance().Deactivate(entities[entities.size() - 1]);
	}

	//// Create some temporary variables to represent colors
	//// - Not necessary, just makes things more readable
	//XMFLOAT3 genericNormal = XMFLOAT3(0, 0, -1);
	//XMFLOAT2 genericUv = XMFLOAT2(0, 0);

	//// Set up the vertices of the triangle we would like to draw
	//// - We're going to copy this array, exactly as it exists in memory
	////    over to a DirectX-controlled data structure (the vertex buffer)
	//Vertex vertices1[] =
	//{
	//	{ XMFLOAT3(+0.0f, +1.0f, +0.0f), genericNormal, genericUv },
	//	{ XMFLOAT3(+1.5f, -1.0f, +0.0f), genericNormal, genericUv },
	//	{ XMFLOAT3(-1.5f, -1.0f, +0.0f), genericNormal, genericUv },
	//};

	//// Set up the indices, which tell us which vertices to use and in which order
	//// - This is somewhat redundant for just 3 vertices (it's a simple example)
	//// - Indices are technically not required if the vertices are in the buffer 
	////    in the correct order and each one will be used exactly once
	//// - But just to see how it's done...
	//unsigned int indices1[] = { 0, 1, 2 };


	//Mesh* mesh1 = new Mesh(vertices1, 3, indices1, 3, device);

	//meshes.push_back(mesh1);

	//Entity* ent1 = new Entity(mesh1, defMaterial);
	//Entity* ent2 = new Entity(mesh1, defMaterial);
	//ent2->SetScale(XMFLOAT3(+0.5f, +0.5f, +0.5f));

	//entities.push_back(ent1);
	//entities.push_back(ent2);

	//Vertex vertices2[] = {
	//	{XMFLOAT3(-0.5f, -0.5f, +0.0f), genericNormal, genericUv },
	//	{XMFLOAT3(-0.5f, +0.5f, +0.0f), genericNormal, genericUv },
	//	{XMFLOAT3(+0.5f, +0.5f, +0.0f), genericNormal, genericUv },
	//	{XMFLOAT3(+0.5f, -0.5f, +0.0f), genericNormal, genericUv }
	//};

	//unsigned int indices2[] = { 0, 1, 2, 2, 3, 0 };

	//Mesh* mesh2 = new Mesh(vertices2, 4, indices2, 6, device);

	//meshes.push_back(mesh2);

	//Entity* ent3 = new Entity(mesh2, defMaterial);
	//ent3->SetPosition(XMFLOAT3(+1.5f, +1.5f, +0.0f));
	//Entity* ent4 = new Entity(mesh2, defMaterial);

	//entities.push_back(ent3);
	//entities.push_back(ent4);

	//Vertex vertices3[] = {
	//	{XMFLOAT3(+0.0f, +0.0f, +0.0f), genericNormal, genericUv},
	//	{XMFLOAT3(+0.0f, +1.0f, +0.0f), genericNormal, genericUv},
	//	{XMFLOAT3(+0.75f, +0.66f, +0.0f), genericNormal, genericUv},
	//	{XMFLOAT3(+1.0f, +0.0f, +0.0f), genericNormal, genericUv},
	//	{XMFLOAT3(+0.75f, -0.66f, +0.0f), genericNormal, genericUv},
	//	{XMFLOAT3(+0.0f, -1.0f, +0.0f), genericNormal, genericUv},
	//	{XMFLOAT3(-0.75f, -0.66f, +0.0f), genericNormal, genericUv},
	//	{XMFLOAT3(-1.0f, +0.0f, +0.0f), genericNormal, genericUv},
	//	{XMFLOAT3(-0.75f, +0.66f, +0.0f), genericNormal, genericUv}
	//};

	//unsigned int indices3[] = { 0, 1, 2, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 7, 0, 7, 8, 0, 8, 1, 0 };

	//Mesh* mesh3 = new Mesh(vertices3, 9, indices3, 24, device);

	//meshes.push_back(mesh3);

	//Entity* ent5 = new Entity(mesh3, defMaterial);
	//ent5->SetPosition(XMFLOAT3(-1.5f, +1.5f, +0.0f));

	//entities.push_back(ent5);

	//// Create the VERTEX BUFFER description -----------------------------------
	//// - The description is created on the stack because we only need
	////    it to create the buffer.  The description is then useless.
	//D3D11_BUFFER_DESC vbd;
	//vbd.Usage				= D3D11_USAGE_IMMUTABLE;
	//vbd.ByteWidth			= sizeof(Vertex) * 3;       // 3 = number of vertices in the buffer
	//vbd.BindFlags			= D3D11_BIND_VERTEX_BUFFER; // Tells DirectX this is a vertex buffer
	//vbd.CPUAccessFlags		= 0;
	//vbd.MiscFlags			= 0;
	//vbd.StructureByteStride	= 0;

	//// Create the proper struct to hold the initial vertex data
	//// - This is how we put the initial data into the buffer
	//D3D11_SUBRESOURCE_DATA initialVertexData;
	//initialVertexData.pSysMem = vertices;

	//// Actually create the buffer with the initial data
	//// - Once we do this, we'll NEVER CHANGE THE BUFFER AGAIN
	//device->CreateBuffer(&vbd, &initialVertexData, &vertexBuffer);



	//// Create the INDEX BUFFER description ------------------------------------
	//// - The description is created on the stack because we only need
	////    it to create the buffer.  The description is then useless.
	//D3D11_BUFFER_DESC ibd;
	//ibd.Usage               = D3D11_USAGE_IMMUTABLE;
	//ibd.ByteWidth           = sizeof(int) * 3;         // 3 = number of indices in the buffer
	//ibd.BindFlags           = D3D11_BIND_INDEX_BUFFER; // Tells DirectX this is an index buffer
	//ibd.CPUAccessFlags      = 0;
	//ibd.MiscFlags           = 0;
	//ibd.StructureByteStride = 0;

	//// Create the proper struct to hold the initial index data
	//// - This is how we put the initial data into the buffer
	//D3D11_SUBRESOURCE_DATA initialIndexData;
	//initialIndexData.pSysMem = indices;

	//// Actually create the buffer with the initial data
	//// - Once we do this, we'll NEVER CHANGE THE BUFFER AGAIN
	//device->CreateBuffer(&ibd, &initialIndexData, &indexBuffer);
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

	//entities[0]->SetScale(XMFLOAT3(sinTime, sinTime, sinTime));
	//entities[1]->SetRotation(XMFLOAT3(0, 0, totalTime / 2));
	//entities[1]->MoveForward();
	//entities[3]->SetRotation(XMFLOAT3(0, totalTime / 2, 0));
	//entities[3]->MoveForward();
	//entities[2]->SetRotation(XMFLOAT3(0, 0, -totalTime));
	//entities[2]->SetScale(XMFLOAT3(cosTime, cosTime, cosTime));
	//entities[4]->SetRotation(XMFLOAT3(0, 0, totalTime));
	//entities[4]->SetScale(XMFLOAT3(cosTime, cosTime, cosTime));

	// timer
	myTimer += deltaTime;

	for (int i = 0; i < noteMarkers.size(); i++) {
		if (noteMarkers[i]->IsActive()) {
			XMFLOAT3 p = noteMarkers[i]->GetPosition();
			noteMarkers[i]->SetPosition(XMFLOAT3(p.x,p.y-deltaTime*5,p.z));
		}
	}

	// create entities dynamically
	float max = 0.2;
	if (myTimer > max) {
		counter++;
		myTimer -= max;
		//entities.push_back(new Entity(meshes[0], materials[0]));
		// FOR DEMONSTRATION ONLY
		// remove old
		if (noteMarkers.size() > 2) {
			Recycler::GetInstance().Deactivate(noteMarkers.back());
			noteMarkers.pop_back();
		}
		// new at front
		Entity* e = Recycler::GetInstance().Reactivate();
		noteMarkers.insert(noteMarkers.begin(), e);
		e->SetPosition(XMFLOAT3(parser.GetNote(counter + 10), 3, 0));
		//entities[entities.size() - 1]->

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

	//// Send data to shader variables
	////  - Do this ONCE PER OBJECT you're drawing
	////  - This is actually a complex process of copying data to a local buffer
	////    and then copying that entire buffer to the GPU.  
	////  - The "SimpleShader" class handles all of that for you.
	//vertexShader->SetMatrix4x4("view", camera->GetViewMatrix());
	//vertexShader->SetMatrix4x4("projection", camera->GetProjectionMatrix());

	// Once you've set all of the data you care to change for
	// the next draw call, you need to actually send it to the GPU
	//  - If you skip this, the "SetMatrix" calls above won't make it to the GPU!
	//vertexShader->CopyAllBufferData();

	// Set the vertex and pixel shaders to use for the next Draw() command
	//  - These don't technically need to be set every frame...YET
	//  - Once you start applying different shaders to different objects,
	//    you'll need to swap the current shaders before each draw
	//vertexShader->SetShader();
	//pixelShader->SetShader();

	// Set buffers in the input assembler
	//  - Do this ONCE PER OBJECT you're drawing, since each object might
	//    have different geometry.
	//UINT stride = sizeof(Vertex);
	//UINT offset = 0;
	//context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	//context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Finally do the actual drawing
	//  - Do this ONCE PER OBJECT you intend to draw
	//  - This will use all of the currently set DirectX "stuff" (shaders, buffers, etc)
	//  - DrawIndexed() uses the currently set INDEX BUFFER to look up corresponding
	//     vertices in the currently set VERTEX BUFFER
	//context->DrawIndexed(
	//	3,     // The number of indices to use (we could draw a subset if we wanted)
	//	0,     // Offset to the first index we want to use
	//	0);    // Offset to add to each index when looking up vertices

	/*for (auto mesh : meshes) {
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		ID3D11Buffer* vb = mesh->GetVertexBuffer();
		context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
		context->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

		context->DrawIndexed(
			mesh->GetIndexCount(),
			0,
			0);
	}*/

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