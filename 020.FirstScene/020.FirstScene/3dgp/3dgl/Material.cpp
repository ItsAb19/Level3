/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Version 3.0 - June 2022
Copyright (C) 2013-22 by Jarek Francik, Kingston University, London, UK
*********************************************************************************/
#include "pch.h"
#include <fstream>
#include <3dgl/Material.h>
#include <3dgl/Bitmap.h>
#include <3dgl/Model.h>
#include <3dgl/Shader.h>

// assimp include file
#include <assimp/scene.h>

using namespace _3dgl;

unsigned C3dglMaterial::c_idTexBlank = 0xFFFFFFFF;

C3dglMaterial::C3dglMaterial(C3dglModel* pOwner) : m_pOwner(pOwner)
{
	memset(m_idTexture, 0xFFFFFFFF, sizeof(m_idTexture));
	m_bAmb = m_bDiff = m_bSpec = m_bEmiss = m_bShininess = false;

	memset(&m_amb, 0, sizeof(m_amb));
	memset(&m_diff, 0, sizeof(m_diff));
	memset(&m_spec, 0, sizeof(m_spec));
	memset(&m_emiss, 0, sizeof(m_emiss));
	m_shininess = 0.0f;
}

void C3dglMaterial::create(const aiMaterial *pMat, const char* pDefTexPath)
{
	const char* pPath = NULL;

	// texture
	aiString texPath;	// contains filename of texture
	if (pMat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
		pPath = texPath.C_Str();
	
	unsigned int max;
	m_bShininess = AI_SUCCESS == aiGetMaterialFloatArray(pMat, AI_MATKEY_SHININESS, &m_shininess, &max);
	m_bAmb = AI_SUCCESS == aiGetMaterialColor(pMat, AI_MATKEY_COLOR_AMBIENT, (aiColor4D*)&m_amb);
	m_bDiff = AI_SUCCESS == aiGetMaterialColor(pMat, AI_MATKEY_COLOR_DIFFUSE, (aiColor4D*)&m_diff);
	m_bSpec = aiGetMaterialColor(pMat, AI_MATKEY_COLOR_SPECULAR, (aiColor4D*)&m_spec);
	m_bEmiss = AI_SUCCESS == aiGetMaterialColor(pMat, AI_MATKEY_COLOR_EMISSIVE, (aiColor4D*)&m_emiss);

	// texture
	if (pPath)
		if (pDefTexPath)
			loadTexture(pDefTexPath, pPath);
		else
			loadTexture(pPath);
	if (m_idTexture[0] == 0xFFFFFFFF)
		loadTexture();
}

void C3dglMaterial::destroy()
{
	for (unsigned& idTexture : m_idTexture)
		if (idTexture != 0xffffffff)
			glDeleteTextures(1, &idTexture);
}

void C3dglMaterial::render(C3dglProgram *pProgram) const
{
	for (unsigned texUnit = GL_TEXTURE0; texUnit <= GL_TEXTURE31; texUnit++)
	{
		unsigned idTex;
		if (getTexture(texUnit, idTex))
		{
			glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*) & m_back_idTexture[texUnit - GL_TEXTURE0]);
			glActiveTexture(texUnit);
			glBindTexture(GL_TEXTURE_2D, idTex);
		}
	}

	if (!pProgram)
		pProgram = C3dglProgram::getCurrentProgram();

	if (pProgram)
	{
		if (getAmbient()) pProgram->retrieveUniform(UNI_MAT_AMBIENT, m_back_amb);
		if (getDiffuse()) pProgram->retrieveUniform(UNI_MAT_DIFFUSE, m_back_diff);
		if (getSpecular()) pProgram->retrieveUniform(UNI_MAT_SPECULAR, m_back_spec);
		if (getEmissive()) pProgram->retrieveUniform(UNI_MAT_EMISSIVE, m_back_emiss);
		if (getShininess()) pProgram->retrieveUniform(UNI_MAT_SHININESS, m_back_shininess);

		glm::vec3 vec;
		if (getAmbient(vec)) pProgram->sendUniform(UNI_MAT_AMBIENT, vec);
		if (getDiffuse(vec)) pProgram->sendUniform(UNI_MAT_DIFFUSE, vec);
		if (getSpecular(vec)) pProgram->sendUniform(UNI_MAT_SPECULAR, vec);
		if (getEmissive(vec)) pProgram->sendUniform(UNI_MAT_EMISSIVE, vec);
		float v;
		if (getShininess(v)) pProgram->sendUniform(UNI_MAT_SHININESS, v);
	}
}

void C3dglMaterial::postRender(C3dglProgram* pProgram) const
{
	for (unsigned texUnit = GL_TEXTURE0; texUnit <= GL_TEXTURE31; texUnit++)
	{
		if (getTexture(texUnit))
		{
			glActiveTexture(texUnit);
			glBindTexture(GL_TEXTURE_2D, m_back_idTexture[texUnit - GL_TEXTURE0]);
		}
	}

	if (!pProgram)
		pProgram = C3dglProgram::getCurrentProgram();

	if (pProgram)
	{
		if (getAmbient()) pProgram->sendUniform(UNI_MAT_AMBIENT, m_back_amb);
		if (getDiffuse()) pProgram->sendUniform(UNI_MAT_DIFFUSE, m_back_diff);
		if (getSpecular()) pProgram->sendUniform(UNI_MAT_SPECULAR, m_back_spec);
		if (getEmissive()) pProgram->sendUniform(UNI_MAT_EMISSIVE, m_back_emiss);
		if (getShininess()) pProgram->sendUniform(UNI_MAT_SHININESS, m_back_shininess);
	}
}

void C3dglMaterial::loadTexture(GLenum texUnit, std::string strDefTexPath, std::string strPath)
{
	// first of all, check for the embedded texture!
	const aiTexture *pTexture = m_pOwner ? m_pOwner->getScene()->GetEmbeddedTexture(strPath.c_str()) : NULL;
	if (pTexture)
		loadTexture(texUnit, pTexture);
	else
	{
		// prepare path
		std::ifstream f(strPath);
		if (f.is_open())
		{
			f.close();
		}
		else if (!strDefTexPath.empty())
		{
			size_t i = strPath.find_last_of("/\\");
			if (i != std::string::npos) strPath = strPath.substr(i + 1);


			if (strDefTexPath.back() == '/' || strDefTexPath.back() == '\\')
				strPath = strDefTexPath + strPath;
			else
				strPath = strDefTexPath + "/" + strPath;
		}

		loadTexture(texUnit, strPath);
	}
}

void C3dglMaterial::loadTexture(GLenum texUnit, std::string strPath)
{
	// generate IL image id
	C3dglBitmap bm;
	if (bm.load(strPath, GL_RGBA))
	{
		// generate texture id
		glGenTextures(1, &m_idTexture[texUnit - GL_TEXTURE0]);

		// load texture
		glActiveTexture(texUnit);
		glBindTexture(GL_TEXTURE_2D, m_idTexture[texUnit - GL_TEXTURE0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.getWidth(), bm.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.getBits());
	}
}

void C3dglMaterial::loadTexture(GLenum texUnit, const aiTexture* pTexture)
{
	// generate texture from aiTexture data
	C3dglBitmap bm;
	if (bm.load(pTexture, GL_RGBA))
	{
		// generate texture id
		glGenTextures(1, &m_idTexture[texUnit - GL_TEXTURE0]);

		// load texture
		glActiveTexture(texUnit);
		glBindTexture(GL_TEXTURE_2D, m_idTexture[texUnit - GL_TEXTURE0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.getWidth(), bm.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.getBits());
	}
}

void C3dglMaterial::loadTexture(GLenum texUnit)
{
	if (c_idTexBlank == 0xffffffff)
	{
		glGenTextures(1, &c_idTexBlank);
		glActiveTexture(texUnit);
		glBindTexture(GL_TEXTURE_2D, c_idTexBlank);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		unsigned char bytes[] = { 255, 255, 255 };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, &bytes);
	}
	m_idTexture[texUnit - GL_TEXTURE0] = c_idTexBlank;
}

