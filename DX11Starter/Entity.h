#pragma once
#include "Mesh.h"
#include "Material.h"
#include "Lights.h"
#include "CubeMap.h"
#include "Camera.h"
#include "ParticleManager.h";
#include <DirectXMath.h>

using namespace DirectX;

class Entity
{
public:
	Entity(Mesh*, Material*);
	~Entity();
	XMFLOAT4X4 GetWorldMatrix();
	void SetWorldMatrix(XMFLOAT4X4);
	XMFLOAT3 GetPosition();
	void SetPosition(XMFLOAT3);
	XMFLOAT3 GetRotation();
	void SetRotation(XMFLOAT3);
	XMFLOAT3 GetScale();
	void SetScale(XMFLOAT3);
	void MoveForward();
	Mesh* GetMesh();
	Material* GetMaterial();
	void Activate();
	void Deactivate();
	bool IsActive();
	void PrepareMaterial(XMFLOAT4X4, XMFLOAT4X4, DirectionalLight, DirectionalLight, XMFLOAT3);
	void PrepareTerrainMaterial(XMFLOAT4X4 view, XMFLOAT4X4 projection, float * frequencies, unsigned int length, DirectionalLight light, DirectionalLight light2);

	static CubeMap* activeSkybox;
private:
	bool active;
	Mesh* _mesh;
	Material* _material;
	XMFLOAT4X4 _worldMatrix;
	XMFLOAT3 _pos;
	XMFLOAT3 _rot;
	XMFLOAT3 _scale;
	void UpdateWorldMatrix();
};

