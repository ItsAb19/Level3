/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Version 3.0 - June 2022
Copyright (C) 2013-22 by Jarek Francik, Kingston University, London, UK
*********************************************************************************/
#include "pch.h"
#include <3dgl/Shader.h>
#include <3dgl/Bitmap.h>
#include <3dgl/SkyBox.h>

using namespace _3dgl;

C3dglSkyBox::C3dglSkyBox() : C3dglVertexAttrObject(ATTR_COUNT_BASIC)
{
	std::fill(m_idTex, m_idTex + 6, 0);
}

bool C3dglSkyBox::load(const char* pFd, const char* pRt, const char* pBk, const char* pLt, const char* pUp, const char* pDn, C3dglProgram* pProgram)
{
	if (getAttrCount() != ATTR_COUNT_BASIC)
	{
		log(M3DGL_INTERNAL_ERROR);
		return false;		// this should never happen!
	}

	glGenTextures(6, m_idTex);

	// load six textures
	glActiveTexture(GL_TEXTURE0);
	const char*pFilenames[] = { pBk, pRt, pFd, pLt, pUp, pDn };
	for (int i = 0; i < 6; ++i)
	{
		C3dglBitmap bm(pFilenames[i], GL_RGBA);
		glGenTextures(1, &m_idTex[i]);
		glBindTexture(GL_TEXTURE_2D, m_idTex[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.getWidth(), abs(bm.getHeight()), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.getBits());
	}

	float vertices[] = 
	{
		-1.0f,-1.0f,-1.0f,	 1.0f,-1.0f,-1.0f,	 1.0f, 1.0f,-1.0f,	-1.0f, 1.0f,-1.0f,	//Back
		-1.0f,-1.0f,-1.0f,	-1.0f, 1.0f,-1.0f,	-1.0f, 1.0f, 1.0f,	-1.0f,-1.0f, 1.0f,	//Left
		 1.0f,-1.0f, 1.0f,	-1.0f,-1.0f, 1.0f,	-1.0f, 1.0f, 1.0f,	 1.0f, 1.0f, 1.0f,	//Front
		 1.0f,-1.0f, 1.0f,	 1.0f, 1.0f, 1.0f,	 1.0f, 1.0f,-1.0f,	 1.0f,-1.0f,-1.0f,	//Right
		-1.0f, 1.0f, -1.0f,	 1.0f, 1.0f, -1.0f,	 1.0f, 1.0f,  1.0f,	-1.0f, 1.0f,  1.0f,	//Top
		 1.0f,-1.0f, -1.0f,	-1.0f,-1.0f, -1.0f,	-1.0f,-1.0f,  1.0f,	 1.0f,-1.0f,  1.0f	//Bottom
	};

	float normals[] = 
	{
		0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1,			//Back
		1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0,			//Left
		0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1,		//Front
		-1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0,		//Right
		0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0,		//Top
		0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0			//Bottom
	};

	float texCoords[] = 
	{
		0.0f, 0.0f,		1.0f, 0.0f,		1.0f, 1.0f,		0.0f, 1.0f,		//Back
		1.0f, 0.0f,		1.0f, 1.0f,		0.0f, 1.0f,		0.0f, 0.0f,		//Left
		0.0f, 0.0f,		1.0f, 0.0f,		1.0f, 1.0f,		0.0f, 1.0f,		//Front
		1.0f, 0.0f,		1.0f, 1.0f,		0.0f, 1.0f,		0.0f, 0.0f,		//Right
		0.0f, 0.0f,		1.0f, 0.0f,		1.0f, 1.0f,		0.0f, 1.0f,		//Top
		0.0f, 0.0f,		1.0f, 0.0f,		1.0f, 1.0f,		0.0f, 1.0f		//Bottom
	};

	float* attrData[] = { vertices, normals,  texCoords };
	size_t attrSize[] = { 3 * sizeof(float), 3 * sizeof(float), 2 * sizeof(float) };
	size_t nVertices = 24;

	// additional warning - only checked for skyboxes...
	if (pProgram && pProgram->getAttribLocation(ATTR_TEXCOORD) == -1
	|| !pProgram && C3dglProgram::getCurrentProgram() && C3dglProgram::getCurrentProgram()->getAttribLocation(ATTR_TEXCOORD) == -1)
		log(M3DGL_WARNING_TEXCOORDS_COORDS_NOT_IMPLEMENTED);

	create(getAttrCount(), nVertices, (void**)attrData, attrSize, 0, NULL, 0, pProgram);

	return true;
}

void C3dglSkyBox::render(GLsizei instances) const
{
	// disable depth-buffer write cycles - so that the skybox cannot obscure anything
	GLboolean bDepthMask;
	::glGetBooleanv(GL_DEPTH_WRITEMASK, &bDepthMask);
	glDepthMask(GL_FALSE);

	GLuint prevVAO;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint*)&prevVAO);
	glBindVertexArray(getVAOid());
	glActiveTexture(GL_TEXTURE0);
	for (int i = 0; i < 6; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, m_idTex[i]);
		if (instances == 1)
			glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
		else
			glDrawArraysInstanced(GL_TRIANGLE_FAN, i * 4, 4, instances);
	}
	glBindVertexArray(prevVAO);

	// enable depth-buffer write cycle
	glDepthMask(bDepthMask);
}

void C3dglSkyBox::render(glm::mat4 matrix, C3dglProgram* pProgram) const
{
	// send model view matrix
	matrix[3][0] = matrix[3][1] = matrix[3][2] = 0;

	C3dglVertexAttrObject::render(matrix, 1, pProgram);
}
