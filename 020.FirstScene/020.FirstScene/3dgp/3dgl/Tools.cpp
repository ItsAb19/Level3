/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Version 3.0 - June 2022
Copyright (C) 2013-22 by Jarek Francik, Kingston University, London, UK
*********************************************************************************/
#include "pch.h"
#include <iostream>
#include <fstream>
#include <vector>

// GLM include files
#include "../glm/gtc/type_ptr.hpp"

#include <GL/glut.h>
#include <3dgl/Tools.h>
#include <3dgl/CommonDef.h>
#include <3dgl/Bitmap.h>
#include <3dgl/Terrain.h>
#include <3dgl/Shader.h>
#include <3dgl/Mesh.h>

using namespace _3dgl;

void MY3DGL_API _3dgl::print(int x, int y, std::string text, glm::vec3 color, enum FONT font, enum ALIGN align)
{
	void*fonts[] = { GLUT_BITMAP_9_BY_15, GLUT_BITMAP_8_BY_13, GLUT_BITMAP_TIMES_ROMAN_10, GLUT_BITMAP_TIMES_ROMAN_24, GLUT_BITMAP_HELVETICA_10, GLUT_BITMAP_HELVETICA_12, GLUT_BITMAP_HELVETICA_18 };
	int heights[] = { 15, 13, 10, 24, 10, 12, 18 };
	int width = 0;
	for (char ch : text)
		width += glutBitmapWidth(fonts[font], ch);

	if (x < 0)
		x += glutGet(GLUT_WINDOW_WIDTH) - width;
	else if (align == RIGHT)
		x -= width;
	else if (align == CENTRE)
		x -= width / 2;
	if (y < 0)
		y += glutGet(GLUT_WINDOW_HEIGHT) - heights[font];
	void* f = fonts[font];

	C3dglProgram* pProgram = C3dglProgram::getCurrentProgram();
	glUseProgram(0);
	glColor3f(color.r, color.g, color.b);
	glWindowPos2i(x, y);  // move in 10 pixels from the left and bottom edges
	for (char ch : text)
		glutBitmapCharacter(fonts[font], ch);
	if (pProgram) pProgram->use(false);
}

void MY3DGL_API _3dgl::print(int x, int y, float deltaTime, glm::vec3 color, enum FONT font, enum ALIGN align)
{
	const size_t N = 20;
	static float buf[N];
	static float sum = 0;
	static int i = 0;
	static bool valid = false;

	buf[i++] = deltaTime;
	if (i == N) { i = 0; valid = true; 	}
	float fps = 0;
	for (float f : buf)
		fps += f;

	if (valid) print(x, y, std::format("fps = {}", (int)round(N / fps)), color, font, align);
}

void MY3DGL_API _3dgl::print(int x, int y, glm::mat4 matrixView, glm::vec3 color, enum FONT font, enum ALIGN align)
{
	glm::vec3 pos = getPos(matrixView);
	print(x, y, std::format("X: {:.2f} Y: {:.2f} Z: {:.2f}", pos.x, pos.y, pos.z), color, font, align);
}

bool MY3DGL_API _3dgl::convHeightmap2OBJ(const std::string fileImage, float scaleHeight, const std::string fileOBJ)
{
	C3dglBitmap bm;
	if (!bm.load(fileImage, GL_RGBA) || !bm.getBits())
		return false;

	C3dglTerrain terrain;
	terrain.createHeightMap(bm.getWidth(), abs(bm.getHeight()), scaleHeight, bm.getBits());

	// Prepare Attributes - and pack them into temporary buffers
	const size_t attrCount = ATTR_TANGENT;	// This is a OBJ file, no need to create more than 3 attrins as tangents and bitangents are not supported!
	float* attrData[attrCount];
	size_t attrSize[attrCount];
	size_t nVertices = terrain.getBuffers(attrCount, attrData, attrSize);

	GLuint* indices = NULL;
	size_t indSize = 0;
	size_t nIndices = terrain.getIndexBuffer(&indices, &indSize);

	// Writing part...
	std::ofstream wf(fileOBJ, std::ios::out);
	wf << "# 3dgl Mesh Exporter - (c)2021 Jarek Francik" << std::endl << std::endl;
	wf << "# object Terrain1" << std::endl << std::endl;
	
	
	for (size_t i = 0; i < nVertices; i++)
		wf << "v  " << attrData[ATTR_VERTEX][3 * i] << " " << attrData[ATTR_VERTEX][3 * i + 1] << " " << attrData[ATTR_VERTEX][3 * i + 2] << std::endl;
	wf << std::endl;
	
	for (size_t i = 0; i < nVertices; i ++)
		wf << "vn  " << attrData[ATTR_NORMAL][3 * i] << " " << attrData[ATTR_NORMAL][3 * i + 1] << " " << attrData[ATTR_NORMAL][3 * i + 2] << std::endl;
	wf << std::endl;
	
	for (size_t i = 0; i < nVertices; i++)
		wf << "vt  " << attrData[ATTR_TEXCOORD][2 * i] << " " << attrData[ATTR_TEXCOORD][2 * i + 1] << " " << 0.0f << std::endl;
	wf << std::endl;
	
	wf << "g Terrain1" << std::endl;
	wf << "s 1" << std::endl;

	for (size_t i = 0; i < nIndices; i += 3)
		wf << "f  " << indices[i]+1 << "/" << indices[i] + 1 << "/" << indices[i] + 1 << " "
					<< indices[i+1] + 1 << "/" << indices[i+1] + 1 << "/" << indices[i+1] + 1 << " "
					<< indices[i+2] + 1 << "/" << indices[i+2] + 1 << "/" << indices[i+2] + 1 << std::endl;

	terrain.cleanUp(attrCount, attrData, indices);

	return true;
}

bool MY3DGL_API _3dgl::convHeightmap2Mesh(const std::string fileImage, float scaleHeight, C3dglMesh *pMesh, C3dglProgram* pProgram)
{
	C3dglBitmap bm;
	if (!bm.load(fileImage, GL_RGBA) || !bm.getBits())
		return false;

	C3dglTerrain terrain;
	terrain.createHeightMap(bm.getWidth(), abs(bm.getHeight()), scaleHeight, bm.getBits());

	// Prepare Attributes - and pack them into temporary buffers
	const size_t attrCount = ATTR_COLOR;	// 5 attribs but no colour and no bones
	float* attrData[attrCount];
	size_t attrSize[attrCount];
	size_t nVertices = terrain.getBuffers(attrCount, attrData, attrSize);

	GLuint* indices = NULL;
	size_t indSize = 0;
	size_t nIndices = terrain.getIndexBuffer(&indices, &indSize);

	static_cast<C3dglVertexAttrObject*>(pMesh)->create(attrCount, nVertices, (void**)attrData, attrSize, nIndices, indices, indSize, pProgram);

	terrain.cleanUp(attrCount, attrData, indices);

	return true;
}
