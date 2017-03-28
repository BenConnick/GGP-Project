#include "Camera.h"
#include <Windows.h>


Camera::Camera(unsigned int width, unsigned int height)
{
	XMMATRIX identity = XMMatrixIdentity();
	_position = XMFLOAT3(0, 0, -5);
	_xRot = 0;
	_yRot = 0;
	XMStoreFloat4x4(&_viewMatrix, identity);
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,		// Field of View Angle
		(float)width / height,		// Aspect ratio
		0.1f,						// Near clip plane distance
		100.0f);					// Far clip plane distance
	XMStoreFloat4x4(&_projectionMatrix, XMMatrixTranspose(P));
}


Camera::~Camera()
{
}

XMFLOAT4X4 Camera::GetViewMatrix()
{
	return _viewMatrix;
}

XMFLOAT4X4 Camera::GetProjectionMatrix()
{
	return _projectionMatrix;
}

void Camera::Update(float deltaTime)
{
	XMFLOAT3 xAxis = XMFLOAT3(1, 0, 0);
	XMFLOAT3 yAxis = XMFLOAT3(0, 1, 0);
	XMFLOAT3 forward = XMFLOAT3(0, 0, 1);
	XMVECTOR rotation = XMQuaternionRotationRollPitchYaw(_xRot, _yRot, 0);
	XMVECTOR direction = XMVector3Rotate(XMLoadFloat3(&forward), rotation);
	XMVECTOR xDirection = XMVector3Rotate(XMLoadFloat3(&xAxis), rotation);

	if (GetAsyncKeyState('W') & 0x8000) {
		XMVECTOR curPos = XMLoadFloat3(&_position);
		XMVECTOR zForward = direction * deltaTime;
		curPos += zForward;
		XMStoreFloat3(&_position, curPos);
	}
	if (GetAsyncKeyState('S') & 0x8000) {
		XMVECTOR curPos = XMLoadFloat3(&_position);
		XMVECTOR zForward = direction * deltaTime;
		curPos -= zForward;
		XMStoreFloat3(&_position, curPos);
	}
	if (GetAsyncKeyState('A') & 0x8000) {
		XMVECTOR curPos = XMLoadFloat3(&_position);
		XMVECTOR xForward = xDirection * deltaTime;
		curPos -= xForward;
		XMStoreFloat3(&_position, curPos);
	}
	if (GetAsyncKeyState('D') & 0x8000) {
		XMVECTOR curPos = XMLoadFloat3(&_position);
		XMVECTOR xForward = xDirection * deltaTime;
		curPos += xForward;
		XMStoreFloat3(&_position, curPos);
	}
	if (GetAsyncKeyState(' ') & 0x8000) {
		_position.y += 1 * deltaTime;
	}
	if (GetAsyncKeyState('X') & 0x8000) {
		_position.y -= 1 * deltaTime;
	}

	XMMATRIX lookAt = XMMatrixLookToLH(XMLoadFloat3(&_position), direction, XMLoadFloat3(&yAxis));
	XMStoreFloat4x4(&_viewMatrix, XMMatrixTranspose(lookAt));
}

void Camera::Look(long xDif, long yDif) {
	float radsPerPixel = (XM_PI * 2) / 960;
	_yRot += xDif * radsPerPixel;
	_xRot += yDif * radsPerPixel;
	if (_xRot >= XM_PI / 2) {
		_xRot = XM_PI / 2;
	}
	else if (_xRot <= -XM_PI / 2) {
		_xRot = -XM_PI / 2;
	}
	if (_yRot >= XM_PI * 2) {
		_yRot -= XM_PI * 2;
	}
	else if (_yRot <= -XM_PI * 2) {
		_yRot += XM_PI * 2;
	}
}

void Camera::OnResize(unsigned int width, unsigned int height) {
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,		// Field of View Angle
		(float)width / height,		// Aspect ratio
		0.1f,						// Near clip plane distance
		100.0f);					// Far clip plane distance
	XMStoreFloat4x4(&_projectionMatrix, XMMatrixTranspose(P));
}