/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Version 3.0 - June 2022
Copyright (C) 2013-22 by Jarek Francik, Kingston University, London, UK
*********************************************************************************/
#include "pch.h"
#include <3dgl/Bitmap.h>
#include <3dgl/Terrain.h>
#include <3dgl/Shader.h>

using namespace _3dgl;

C3dglTerrain::C3dglTerrain() : C3dglVertexAttrObject(ATTR_COUNT_EXT)	// we use "extended set" of attributes - with tangents and bitangents but no color and bones
{
	// height map
	m_heights = NULL;
    m_nSizeX = m_nSizeZ = 0;
	m_fScaleHeight = 1;
}

void C3dglTerrain::createHeightMap(int nSizeX, int nSizeZ, float fScaleHeight, void* pBytes)
{
	m_nSizeX = nSizeX;
	m_nSizeZ = nSizeZ;
	m_fScaleHeight = fScaleHeight;

	// Collect Height Values
	m_heights = new float[m_nSizeX * m_nSizeZ];
	for (int i = 0; i < m_nSizeX; i++)
		for (int j = 0; j < m_nSizeZ; j++)
		{
			int index = (i + (m_nSizeZ - j - 1) * m_nSizeX) * 4;
			unsigned char val = static_cast<unsigned char*>(pBytes)[index];
			float f = (float)val / 256.0f;
			m_heights[i * m_nSizeZ + j] = f * m_fScaleHeight;
		}
}

size_t C3dglTerrain::getBuffers(size_t attrCount, float** attrData, size_t* attrSize)
{
	if (!attrCount) return 0;

	size_t nVertices = m_nSizeX * m_nSizeZ;
	GLint mul[] = { 3, 3, 2, 3, 3 };

	for (size_t attr = 0; attr < attrCount; attr++)
	{
		attrData[attr] = new float[nVertices * mul[attr]];
		attrSize[attr] = sizeof(float) * mul[attr];
	}
	float* pVertex = attrData[0];
	float* pNormal = attrData[1];
	float* pTexCoord = attrData[2];
	float* pTangent = attrData[3];
	float* pBiTangent = attrData[4];

	int minx = -m_nSizeX / 2;
	int minz = -m_nSizeZ / 2;
	for (int x = minx; x < minx + m_nSizeX; x++)
		for (int z = minz; z < minz + m_nSizeZ; z++)
		{
			*pVertex++ = (float)x;
			*pVertex++ = getHeight(x, z);
			*pVertex++ = (float)z;

			if (attrCount <= ATTR_NORMAL) continue;
			int x0 = (x == minx) ? x : x - 1;
			int x1 = (x == minx + m_nSizeX - 1) ? x : x + 1;
			int z0 = (z == minz) ? z : z - 1;
			int z1 = (z == minz + m_nSizeZ - 1) ? z : z + 1;

			float dy_x = getHeight(x1, z) - getHeight(x0, z);
			float dy_z = getHeight(x, z1) - getHeight(x, z0);
			float m = sqrt(dy_x * dy_x + 4 + dy_z * dy_z);
			*pNormal++ = -dy_x / m;
			*pNormal++ = 2 / m;
			*pNormal++ = -dy_z / m;

			if (attrCount <= ATTR_TEXCOORD) continue;
			*pTexCoord++ = (float)x / 2.f;
			*pTexCoord++ = (float)z / 2.f;

			if (attrCount < ATTR_TANGENT) continue;
			*pTangent++ = 1;
			*pTangent++ = dy_x / (x1 - x0);
			*pTangent++ = 0;

			if (attrCount < ATTR_BITANGENT) continue;
			*pBiTangent++ = 0;
			*pBiTangent++ = dy_z / (z1 - z0);
			*pBiTangent++ = 1;
		}

	return m_nSizeX * m_nSizeZ;
}

size_t C3dglTerrain::getIndexBuffer(GLuint** indexData, size_t* indSize)
{
	// Generate Indices

	/*
		We loop through building the triangles that
		make up each grid square in the heightmap

		(z*w+x) *----* (z*w+x+1)
				|   /|
				|  / |
				| /  |
	 ((z+1)*w+x)*----* ((z+1)*w+x+1)
	*/
	//Generate the triangle indices

	size_t indicesSize = (m_nSizeZ - 1) * (m_nSizeX - 1);
	GLuint* indices = new GLuint[indicesSize * 6], * pIndice = indices;
	for (int z = 0; z < m_nSizeZ - 1; ++z)
		for (int x = 0; x < m_nSizeX - 1; ++x)
		{
			*pIndice++ = x * m_nSizeZ + z; // current point
			*pIndice++ = x * m_nSizeZ + z + 1; // next row
			*pIndice++ = (x + 1) * m_nSizeZ + z; // same row, next col

			*pIndice++ = x * m_nSizeZ + z + 1; // next row
			*pIndice++ = (x + 1) * m_nSizeZ + z + 1; //next row, next col
			*pIndice++ = (x + 1) * m_nSizeZ + z; // same row, next col
		}

	*indexData = indices;
	*indSize = sizeof(GLuint);
	return indicesSize * 6;
}

void C3dglTerrain::cleanUp(size_t attrCount, float** attrData, GLuint* indexData)
{
	for (int attr = 0; attr < attrCount; attr++)
		delete[] attrData[attr];
	delete[] indexData;
}


float C3dglTerrain::getHeight(int x, int z)
{
	x += m_nSizeX/2;
	z += m_nSizeZ/2;
	if (x < 0 || x >= m_nSizeX) return 0;
	if (z < 0 || z >= m_nSizeZ) return 0;
	return m_heights[x * m_nSizeZ + z];
}

	glm::vec3 barycentric(glm::vec2 p, glm::vec2 a, glm::vec2 b, glm::vec2 c)
	{
		auto v0 = b - a;
		auto v1 = c - a;
		auto v2 = p - a;

		float d00 = glm::dot(v0, v0);
		float d01 = glm::dot(v0, v1);
		float d11 = glm::dot(v1, v1);
		float d20 = glm::dot(v2, v0);
		float d21 = glm::dot(v2, v1);
		float denom = d00 * d11 - d01 * d01;

		float v = (d11 * d20 - d01 * d21) / denom;
		float w = (d00 * d21 - d01 * d20) / denom;
		float u = 1.0f - v - w;

		return glm::vec3(u, v, w);
	}

float C3dglTerrain::getInterpolatedHeight(float fx, float fz)
{
	int x = (int)floor(fx);
	int z = (int)floor(fz);
	fx -= x;
	fz -= z;
	if (fx + fz < 1)
		return glm::dot(barycentric(glm::vec2(fx, fz), glm::vec2(0, 0), glm::vec2(0, 1), glm::vec2(1, 0)), glm::vec3(getHeight(x, z), getHeight(x, z + 1), getHeight(x + 1, z)));
	else
		return glm::dot(barycentric(glm::vec2(fx, fz), glm::vec2(0, 1), glm::vec2(1, 0), glm::vec2(1, 1)), glm::vec3(getHeight(x, z + 1), getHeight(x + 1, z), getHeight(x + 1, z + 1)));
}

bool C3dglTerrain::load(const std::string filename, float scaleHeight, C3dglProgram* pProgram)
{
	m_name = filename;
	size_t i = m_name.find_last_of("/\\");
	if (i != std::string::npos) m_name = m_name.substr(i + 1);
	i = m_name.find_last_of(".");
	if (i != std::string::npos) m_name = m_name.substr(0, i);

	C3dglBitmap bm;
	if (!bm.load(filename, GL_RGBA))
		return false;

	create(bm.getWidth(), abs(bm.getHeight()), scaleHeight, bm.getBits(), pProgram);
	return true;
}

void C3dglTerrain::create(int nSizeX, int nSizeZ, float fScaleHeight, void* pBytes, C3dglProgram* pProgram)
{
	if (getAttrCount() != ATTR_COUNT_EXT)
	{
		log(M3DGL_INTERNAL_ERROR);
		return;		// this should never happen!
	}

	if (m_heights)
		destroy();

	// Terrain-specific preparation
	createHeightMap(nSizeX, nSizeZ, fScaleHeight, pBytes);

	// Prepare Attributes - and pack them into temporary buffers
	float* attrData[ATTR_COUNT_EXT];	// we use "extended set" of attributes - with tangents and bitangents but no color and bones
	size_t attrSize[ATTR_COUNT_EXT];
	size_t nVertices = getBuffers(getAttrCount(), attrData, attrSize);

	GLuint* indices = NULL;
	size_t indSize = 0;
	size_t nIndices = getIndexBuffer(&indices, &indSize);

	C3dglVertexAttrObject::create(getAttrCount(), nVertices, (void**)attrData, attrSize, nIndices, indices, indSize, pProgram);
	cleanUp(getAttrCount(), attrData, indices);
}

void C3dglTerrain::destroy()
{
	C3dglVertexAttrObject::destroy();
	delete[] m_heights;
	m_heights = NULL;
}
