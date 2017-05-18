#pragma once
#include <d3d11.h>
#include "Vertex.h"
class Mesh
{
public:
	Mesh(Vertex*, unsigned int, unsigned int[], unsigned int, ID3D11Device*);
	Mesh(char*, ID3D11Device*);
	Mesh(unsigned int, unsigned int, ID3D11Device*);
	~Mesh();
	void Initialize(Vertex*, unsigned int, unsigned int[], unsigned int, ID3D11Device*);
	ID3D11Buffer* GetVertexBuffer();
	ID3D11Buffer* GetIndexBuffer();
	unsigned int GetIndexCount();
private:
	ID3D11Buffer* _vertexBuffer;
	ID3D11Buffer* _indexBuffer;
	unsigned int _indexCount;

};

