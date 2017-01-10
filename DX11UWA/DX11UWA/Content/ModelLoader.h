#include <vector>
#include <DirectXMath.h>
#pragma once

using namespace std;
using namespace DirectX;

struct VERTEX
{
	XMFLOAT3 position;
	XMFLOAT3 UV;
	XMFLOAT3 normal;
};



class ModelLoader
{
public:
	bool loadModel(const char * path, vector<VERTEX> &out_verts, vector<unsigned int> &out_indices);



	vector< unsigned int > vertexIndices, uvIndices, normalIndices;
	vector< XMFLOAT3 > temp_vertices;
	vector< XMFLOAT3 > temp_uvs;
	vector< XMFLOAT3 > temp_normals;
	
	

};

