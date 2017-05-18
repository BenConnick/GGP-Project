#pragma once
#include <DirectXMath.h>
using namespace DirectX;

class Camera
{
public:
	Camera(unsigned int, unsigned int);
	~Camera();
	XMFLOAT3 GetPosition();
	XMFLOAT4X4 GetViewMatrix();
	XMFLOAT4X4 GetProjectionMatrix();
	void Update(float);
	void Look(long, long);
	void OnResize(unsigned int, unsigned int);
	void Shake(float duration, float frequency, float magnitude);
private:
	XMFLOAT4X4 _viewMatrix;
	XMFLOAT4X4 _projectionMatrix;
	XMFLOAT3 _position;
	XMFLOAT3 _direction;
	float _xRot;
	float _yRot;
	
	float shakeTimer;
	float shakeMagnitude;
	float shakeFreq;
	XMFLOAT3 truePosition;

	bool _userControlled=false;
};

