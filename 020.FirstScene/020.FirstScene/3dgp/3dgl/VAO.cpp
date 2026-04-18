/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Version 3.0 - June 2022
Copyright (C) 2013-22 by Jarek Francik, Kingston University, London, UK
*********************************************************************************/
#include "pch.h"
#include <iostream>
#include <3dgl/VAO.h>
#include <3dgl/Shader.h>

// GLM include files
#include "../glm/gtc/type_ptr.hpp"

using namespace _3dgl;

/*********************************************************************************
** class C3dglVertexAttrObject
*/

C3dglVertexAttrObject::C3dglVertexAttrObject(size_t attrCount) : C3dglObject(), m_attrCount(attrCount)
{
}

C3dglVertexAttrObject::~C3dglVertexAttrObject() 
{ 
	destroy(); 
}

bool C3dglVertexAttrObject::getVertexBufferId(GLint attrLocation, GLuint& bufferId) const
{
	auto it = m_mapBuffers.find(attrLocation);
	if (it != m_mapBuffers.end())
	{
		bufferId = it->second;
		return true;
	}
	else
		return false;
}


void C3dglVertexAttrObject::create(size_t attrCount, size_t nVertices, void** attrData, size_t* attrSize, size_t nIndices, void* indexData, size_t indSize, C3dglProgram* pProgram)
{
	// Find the program to be used
	m_pProgram = pProgram;
	m_pLastProgramUsed = NULL;
	if (m_pProgram == NULL)
		m_pProgram = C3dglProgram::getCurrentProgram();

	// Aquire the Shader Signature - a collection of all standard attributes - see ATTRIB_STD enum for the list
	const GLint* attrId = NULL;
	if (m_pProgram)
	{
		attrId = m_pProgram->getShaderSignature();
		attrCount = glm::min(attrCount, m_pProgram->getShaderSignatureLength());

		// Generate warnings
		if (attrId[ATTR_VERTEX] == -1)
			log(M3DGL_WARNING_VERTEX_COORDS_NOT_IMPLEMENTED);
		if (attrId[ATTR_NORMAL] == -1)
			log(M3DGL_WARNING_NORMAL_COORDS_NOT_IMPLEMENTED);
		if (attrCount >= ATTR_COUNT && attrId[ATTR_BONE_ID] != -1 && attrId[ATTR_BONE_WEIGHT] == -1)
			log(M3DGL_WARNING_BONE_WEIGHTS_NOT_IMPLEMENTED);
		if (attrCount >= ATTR_COUNT && attrId[ATTR_BONE_ID] == -1 && attrId[ATTR_BONE_WEIGHT] != -1)
			log(M3DGL_WARNING_BONE_IDS_NOT_IMPLEMENTED);
	}
	else
		log(M3DGL_WARNING_NO_PROGRAMMABLE_PIPELINE);

	m_nVertices = nVertices;
	m_nIndices = nIndices;

	if (m_nVertices + m_nIndices == 0)
		return;			// nothing to do!

	// create VAO
	GLuint prevVAO;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint*)&prevVAO);
	glGenVertexArrays(1, &m_idVAO);
	glBindVertexArray(m_idVAO);

	// generate attribute buffers, then bind them and send data to OpenGL
	if (m_nVertices)
	{
		if (m_pProgram)
			// programmable pipeline
			for (unsigned attr = 0; attr < attrCount; attr++)
			{
				if (attrId[attr] == -1 || attrData[attr] == NULL)
					continue;
				

				static GLint attribMult[] = { 3, 3, 2, 3, 3, 3, MAX_BONES_PER_VERTEX, MAX_BONES_PER_VERTEX };
				if (attr != ATTR_BONE_ID)
					createVertexBuffer(attrId[attr], nVertices, attribMult[attr], (float*)attrData[attr], (GLsizei)attrSize[attr]);
				else
					createVertexBuffer(attrId[attr], nVertices, attribMult[attr], (int*)attrData[attr], (GLsizei)attrSize[attr]);
			}
		else
			// fixed pipeline only
			for (unsigned attr = 0; attr < ATTR_COUNT_BASIC; attr++)
			{
				if (attrData[attr] == NULL)
					continue;

				const GLenum caps[] = { ATTR_VERTEX, ATTR_NORMAL, ATTR_TEXCOORD };
				createVertexBuffer(caps[attr], nVertices, (float*)attrData[attr], (GLsizei)attrSize[attr]);
			}
	}

	// Index buffer
	if (m_nIndices)
	{
		glGenBuffers(1, &m_idIndex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_idIndex);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indSize * m_nIndices, indexData, GL_STATIC_DRAW);
	}

	// Reset VAO & buffers
	glBindVertexArray(prevVAO);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void C3dglVertexAttrObject::destroy()
{
	for (auto it = m_mapBuffers.begin(); it != m_mapBuffers.end(); it++)
		glDeleteBuffers(1, &it->second);
	m_mapBuffers.clear();
	if (m_idIndex != 0)
		glDeleteBuffers(1, &m_idIndex);
	m_idIndex = 0;
	if (&m_idVAO != 0)
		glDeleteVertexArrays(1, &m_idVAO);
	m_idVAO = 0;
	m_nVertices = m_nIndices = 0;
}

GLuint C3dglVertexAttrObject::createVertexBuffer(GLint attrLocation, size_t instances, GLint size, float* data, GLsizei stride, GLuint divisor, GLenum usage)
{
	if (attrLocation == -1)
	{
		log(M3DGL_ERROR_ATTRIBUTE_NOT_FOUND);
		return (GLuint)-1;
	}

	if (stride == 0) stride = size * sizeof(float);

	destroyVertexBuffer(attrLocation);

	GLuint prevVAO;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint*)&prevVAO);
	if (prevVAO != m_idVAO)
		glBindVertexArray(m_idVAO);

	GLuint bufferId;
	glGenBuffers(1, &bufferId);
	m_mapBuffers[attrLocation] = bufferId;
	glBindBuffer(GL_ARRAY_BUFFER, bufferId);
	glBufferData(GL_ARRAY_BUFFER, instances * stride, data, usage);

	glEnableVertexAttribArray(attrLocation);
	glVertexAttribPointer(attrLocation, size, GL_FLOAT, GL_FALSE, stride, 0);
	if (divisor) glVertexAttribDivisor(attrLocation, divisor);

	// Reset VAO & buffers
	if (prevVAO != m_idVAO)
		glBindVertexArray(prevVAO);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return bufferId;
}

GLuint C3dglVertexAttrObject::createVertexBuffer(GLint attrLocation, size_t instances, GLint size, int* data, GLsizei stride, GLuint divisor, GLenum usage)
{
	if (attrLocation == -1)
	{
		log(M3DGL_ERROR_ATTRIBUTE_NOT_FOUND);
		return (GLuint)-1;
	}

	if (stride == 0)
		stride = size * sizeof(int);

	destroyVertexBuffer(attrLocation);

	GLuint prevVAO;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint*)&prevVAO);
	if (prevVAO != m_idVAO)
		glBindVertexArray(m_idVAO);

	GLuint bufferId;
	glGenBuffers(1, &bufferId);
	m_mapBuffers[attrLocation] = bufferId;
	glBindBuffer(GL_ARRAY_BUFFER, bufferId);
	glBufferData(GL_ARRAY_BUFFER, instances * stride, data, usage);

	glEnableVertexAttribArray(attrLocation);
	glVertexAttribIPointer(attrLocation, size, GL_INT, stride, 0);
	if (divisor) glVertexAttribDivisor(attrLocation, divisor);

	// Reset VAO & buffers
	if (prevVAO != m_idVAO)
		glBindVertexArray(prevVAO);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return bufferId;
}

GLuint C3dglVertexAttrObject::createVertexBuffer(GLenum cap, size_t instances, float* data, GLsizei stride, GLenum usage)
{
	destroyVertexBuffer(cap);

	GLuint prevVAO;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint*)&prevVAO);
	if (prevVAO != m_idVAO)
		glBindVertexArray(m_idVAO);

	GLuint bufferId;
	glGenBuffers(1, &bufferId);
	glBindBuffer(GL_ARRAY_BUFFER, bufferId);

	switch (cap)
	{
	case ATTR_VERTEX:
		m_mapBuffers[GL_VERTEX_ARRAY] = bufferId;
		if (stride == 0) stride = 3 * sizeof(float);
		glBufferData(GL_ARRAY_BUFFER, instances * stride, data, GL_STATIC_DRAW);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, stride, 0);
		break;
	case ATTR_NORMAL:
		m_mapBuffers[GL_NORMAL_ARRAY] = bufferId;
		if (stride == 0) stride = 3 * sizeof(float);
		glBufferData(GL_ARRAY_BUFFER, instances * stride, data, GL_STATIC_DRAW);
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, stride, 0);
		break;
	case ATTR_TEXCOORD:
		m_mapBuffers[GL_TEXTURE_COORD_ARRAY] = bufferId;
		if (stride == 0) stride = 2 * sizeof(float);	// not always true; if this doesn't work, just provide the actual value of strride (!=0)
		glBufferData(GL_ARRAY_BUFFER, instances * stride, data, GL_STATIC_DRAW);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, stride, 0);
		break;
	default:
		log(M3DGL_ERROR_ATTRIBUTE_NOT_FOUND);
		glDeleteBuffers(1, &bufferId);
		break;
	}

	// Reset VAO & buffers
	if (prevVAO != m_idVAO)
		glBindVertexArray(prevVAO);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return bufferId;
}

void C3dglVertexAttrObject::addAttribPointer(GLint attrLocation, GLuint bufferId, size_t instances, GLint size, GLsizei stride, size_t offset, GLuint divisor, GLenum usage)
{
	if (attrLocation == -1)
	{
		log(M3DGL_ERROR_ATTRIBUTE_NOT_FOUND);
		return;
	}

	GLuint prevVAO;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint*)&prevVAO);
	if (prevVAO != m_idVAO)
		glBindVertexArray(m_idVAO);
	
	glBindBuffer(GL_ARRAY_BUFFER, bufferId);

	glEnableVertexAttribArray(attrLocation);
	glVertexAttribPointer(attrLocation, size, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offset));
	if (divisor) glVertexAttribDivisor(attrLocation, divisor);

	// Reset VAO & buffers
	if (prevVAO != m_idVAO)
		glBindVertexArray(prevVAO);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void C3dglVertexAttrObject::addAttribIPointer(GLint attrLocation, GLuint bufferId, size_t instances, GLint size, GLsizei stride, size_t offset, GLuint divisor, GLenum usage)
{
	if (attrLocation == -1)
	{
		log(M3DGL_ERROR_ATTRIBUTE_NOT_FOUND);
		return;
	}

	GLuint prevVAO;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint*)&prevVAO);
	if (prevVAO != m_idVAO)
		glBindVertexArray(m_idVAO);

	glBindBuffer(GL_ARRAY_BUFFER, bufferId);

	glEnableVertexAttribArray(attrLocation);
	glVertexAttribIPointer(attrLocation, size, GL_INT, stride, reinterpret_cast<void*>(offset));
	if (divisor) glVertexAttribDivisor(attrLocation, divisor);

	// Reset VAO & buffers
	if (prevVAO != m_idVAO)
		glBindVertexArray(prevVAO);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void C3dglVertexAttrObject::destroyVertexBuffer(GLint attrLocation)
{
	auto it = m_mapBuffers.find(attrLocation);
	if (it != m_mapBuffers.end())
	{
		glDeleteBuffers(1, &it->second);
		m_mapBuffers.erase(it);
	}
}

void C3dglVertexAttrObject::render(glm::mat4 matrix, GLsizei instances, C3dglProgram* pProgram) const
{
	// check if a shading program is active
	if (pProgram == NULL)
		pProgram = C3dglProgram::getCurrentProgram();

	// perform compatibility checks
	// the only point of this paragraph is to display warnings if the shader used to render and the shader used to load the model are different
	if (pProgram != m_pLastProgramUsed)		// first conditional is used to bypass the check if the current program is the same as the last program used
	{
		m_pLastProgramUsed = pProgram;
		if (pProgram != m_pProgram)	// no checks needed if the current program is the same as the loading program
		{
			const GLint* pLoadSignature = m_pProgram->getShaderSignature();
			const GLint* pRenderSignature = pProgram->getShaderSignature();
			size_t nLoadLen = m_pProgram->getShaderSignatureLength();
			size_t nRenderLen = pProgram->getShaderSignatureLength();
			size_t nLen = glm::min(nLoadLen, nRenderLen);
			if (std::equal(pLoadSignature, pLoadSignature + nLen, pRenderSignature, [](GLint a, GLint b) { return a == -1 && b == -1 || a != -1 && b != -1; }))
				log(M3DGL_WARNING_DIFFERENT_PROGRAM_USED_BUT_COMPATIBLE);
			else
			{
				log(M3DGL_WARNING_INCOMPATIBLE_PROGRAM_USED);
				for (unsigned i = 0; i < nLen; i++)
					if (pLoadSignature[i] != -1 && pRenderSignature[i] == -1)
						log(M3DGL_WARNING_VERTEX_BUFFER_PREPARED_BUT_NOT_USED + i);
					else if (pLoadSignature[i] == -1 && pRenderSignature[i] != -1)
						log(M3DGL_WARNING_VERTEX_BUFFER_NOT_LOADED_BUT_REQUESTED + i);
			}
		}
	}

	// send model view matrix
	if (pProgram)
		pProgram->sendUniform(UNI_MODELVIEW, matrix);
	else
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMultMatrixf((GLfloat*)&matrix);
	}

	render(instances);
}

void C3dglVertexAttrObject::render(GLsizei instances) const
{
	GLuint prevVAO;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint*)&prevVAO);
	if (prevVAO != m_idVAO)
		glBindVertexArray(m_idVAO);
	if (instances == 1)
		glDrawElements(GL_TRIANGLES, (GLsizei)m_nIndices, GL_UNSIGNED_INT, 0);
	else
		glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)m_nIndices, GL_UNSIGNED_INT, 0, instances);
	if (prevVAO != m_idVAO)
		glBindVertexArray(prevVAO);
}

