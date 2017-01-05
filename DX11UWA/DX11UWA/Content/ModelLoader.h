#include <vector>
#include <DirectXMath.h>
#pragma once

using namespace std;
using namespace DirectX;

class ModelLoader
{
	bool loadModel(
	const char * path,
	vector<XMFLOAT3>  out_vertices,
	vector<XMFLOAT3>  out_uvs,
	vector<XMFLOAT3>  out_normals);

	

	vector< unsigned int > vertexIndices, uvIndices, normalIndices;
	vector< XMFLOAT3 > temp_vertices;
	vector< XMFLOAT3 > temp_uvs;
	vector< XMFLOAT3 > temp_normals;





};