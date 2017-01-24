#include "pch.h"
#include <windows.h>
#include "ModelLoader.h"

bool ModelLoader::loadModel(const char * path, vector<VERTEX> &out_verts, vector<unsigned int> &out_indices)
{
	FILE * file;
	fopen_s(&file, path, "r");

	if (file == NULL)
	{
		printf("Failed to open the file !\n");
		return false;
	}

	while (true)
	{
		char lineHeader[1024];
		// read the first word of the line
		int res = fscanf_s(file, "%s", lineHeader, int(sizeof(lineHeader)));

		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.
			// else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0)  // if its a vertex
		{
			XMFLOAT3 vertex;
			fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) // if it is a UV
		{
			XMFLOAT3 uv;
			fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = 1 - uv.y;
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) // if it is a normal
		{
			XMFLOAT3 normal;
			fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0)
		{
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9)
			{
				printf("File can't be read by our simple parser : ( Try exporting with other options\n");
				return false;
			}

			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
	}

		// For each vertex of each triangle
		for (unsigned int i = 0; i < vertexIndices.size(); i++)
		{
			VERTEX tmp;
			tmp.position = temp_vertices[vertexIndices[i] - 1];
			tmp.UV = temp_uvs[uvIndices[i] - 1];
			tmp.normal = temp_normals[normalIndices[i] - 1];
			out_verts.push_back(tmp);
			out_indices.push_back(i);
		}

		return true;
}

