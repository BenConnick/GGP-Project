#include "Entity.h"



CubeMap* Entity::activeSkybox = NULL;
Entity::Entity(Mesh* mesh, Material* material)
{
	_mesh = mesh;
	_material = material;
	_worldMatrix = XMFLOAT4X4();
	XMStoreFloat4x4(&_worldMatrix, XMMatrixIdentity());
	_pos = XMFLOAT3(0, 0, 0);
	_rot = XMFLOAT3(0, 0, 0);
	_scale = XMFLOAT3(1, 1, 1);
}


Entity::~Entity()
{
	//_mesh = nullptr;
	//_material = nullptr;
}

XMFLOAT4X4 Entity::GetWorldMatrix()
{
	return _worldMatrix;
}

void Entity::SetWorldMatrix(XMFLOAT4X4 world)
{
	_worldMatrix = world;
}

XMFLOAT3 Entity::GetPosition()
{
	return _pos;
}

void Entity::SetPosition(XMFLOAT3 pos)
{
	_pos = pos;
	UpdateWorldMatrix();
}

XMFLOAT3 Entity::GetRotation()
{
	return _rot;
}

void Entity::SetRotation(XMFLOAT3 rot)
{
	_rot = rot;
	UpdateWorldMatrix();
}

XMFLOAT3 Entity::GetScale()
{
	return _scale;
}

void Entity::SetScale(XMFLOAT3 scl)
{
	_scale = scl;
	UpdateWorldMatrix();
}

void Entity::MoveForward()
{
	XMVECTOR forward = XMLoadFloat3(&XMFLOAT3(+1.0f, +0.0f, +0.0f));
	XMVECTOR position = XMLoadFloat3(&_pos);
	XMVECTOR rotation = XMLoadFloat3(&_rot);
	XMVECTOR rotatedForward = XMVector3Rotate(forward, rotation);
	XMVECTOR normalizedForward = XMVector3NormalizeEst(rotatedForward);
	normalizedForward *= 0.00025;
	position += normalizedForward;
	XMStoreFloat3(&_pos, position);
	UpdateWorldMatrix();
}

Mesh * Entity::GetMesh()
{
	return _mesh;
}

Material * Entity::GetMaterial() {
	return _material;
}

void Entity::UpdateWorldMatrix() {
	XMMATRIX trans = XMMatrixTranslation(_pos.x, _pos.y, _pos.z);
	XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&_rot));
	XMMATRIX scale = XMMatrixScalingFromVector(XMLoadFloat3(&_scale));

	XMMATRIX world = scale * rot * trans;

	XMStoreFloat4x4(&_worldMatrix, XMMatrixTranspose(world));
}

void Entity::PrepareMaterial(XMFLOAT4X4 view, XMFLOAT4X4 projection, DirectionalLight light, DirectionalLight light2, XMFLOAT3 cameraPosition) {
	SimpleVertexShader* vs = _material->GetVertexShader();
	vs->SetMatrix4x4("view", view);
	vs->SetMatrix4x4("projection", projection);
	vs->SetMatrix4x4("world", _worldMatrix);
	vs->CopyAllBufferData();
	SimplePixelShader* ps = _material->GetPixelShader();
	ps->SetData("light", &light, sizeof(DirectionalLight));
	ps->SetData("light2", &light2, sizeof(DirectionalLight));
	ps->SetData("CameraPosition", &cameraPosition, sizeof(XMFLOAT3));
	float reflective = (_material->GetReflectivity());
	ps->SetData("reflectivity", &reflective, sizeof(float));
	ps->SetShaderResourceView("diffuseTexture", _material->GetTexture());

	if (_material->GetReflectivity() > 0.0f) {
		ps->SetShaderResourceView("Skybox", activeSkybox->GetResourceView());
	}
	ps->SetSamplerState("basicSampler", _material->GetSamplerState());


	ps->CopyAllBufferData();
	vs->SetShader();
	ps->SetShader();
}

void Entity::PrepareTerrainMaterial(XMFLOAT4X4 view, XMFLOAT4X4 projection, float* frequencies, unsigned int length, DirectionalLight light, DirectionalLight light2) {
	SimpleVertexShader* vs = _material->GetVertexShader();
	vs->SetMatrix4x4("view", view);
	vs->SetMatrix4x4("projection", projection);
	vs->SetMatrix4x4("world", _worldMatrix);
	bool result = vs->SetData("amplitudes", frequencies, sizeof(float) * length);
	vs->CopyAllBufferData();
	SimplePixelShader* ps = _material->GetPixelShader();
	ps->SetData("light", &light, sizeof(DirectionalLight));
	ps->SetData("light2", &light2, sizeof(DirectionalLight));
	ps->SetShaderResourceView("diffuseTexture", _material->GetTexture());
	ps->SetSamplerState("basicSampler", _material->GetSamplerState());
	ps->CopyAllBufferData();
	vs->SetShader();
	ps->SetShader();
}

void Entity::Activate() {
	active = true;
}

void Entity::Deactivate() {
	active = false;
}

bool Entity::IsActive() {
	return active;
}