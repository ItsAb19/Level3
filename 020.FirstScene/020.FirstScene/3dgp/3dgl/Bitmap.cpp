/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Version 3.0 - June 2022
Copyright (C) 2013-22 by Jarek Francik, Kingston University, London, UK
*********************************************************************************/
#include "pch.h"
#include <iostream>
#include <3dgl/Bitmap.h>

// DevIL include file
#undef _UNICODE
#include <IL/il.h>

// assimp include file
#include "assimp/scene.h"

using namespace _3dgl;

C3dglBitmap::C3dglBitmap() 
{ 
	m_idImage = 0; 
	m_width = m_height = 0; 
	m_pBits = NULL; 
}

C3dglBitmap::C3dglBitmap(std::string fname, unsigned format)
{
	m_idImage = 0;
	m_width = m_height = 0;
	m_pBits = NULL;
	load(fname, format);
}

static bool bIlInitialised = false;

bool C3dglBitmap::load(std::string fname, unsigned format)
{
	// initialise IL
	if (!bIlInitialised)
		ilInit(); 
	bIlInitialised = true;

	// destroy previous image
	destroy();
	
	// generate IL image id
	ilGenImages(1, &m_idImage); 

	// bind IL image and load
	ilBindImage(m_idImage);
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT); 
	if (ilLoadImage((ILstring)fname.c_str()))
	{
		ilConvertImage(format, IL_UNSIGNED_BYTE); 
		log(M3DGL_SUCCESS_LOADED, fname);

		m_width = ilGetInteger(IL_IMAGE_WIDTH);
		m_height = ilGetInteger(IL_IMAGE_HEIGHT);
		m_pBits = ilGetData();

		return true;
	}
	else 
	{
		log(M3DGL_WARNING_CANNOT_LOAD, fname);
		return false;
	}
}

bool C3dglBitmap::load(const aiTexture* pTexture, unsigned format)
{
	std::string shortName = pTexture->mFilename.data;
	size_t i = shortName.find_last_of("/\\");
	if (i != std::string::npos) shortName = shortName.substr(i + 1);

	if (pTexture->mWidth && pTexture->mHeight && pTexture->pcData)
	{
		m_width = pTexture->mWidth;
		m_height = pTexture->mHeight;
		m_pBits = pTexture->pcData;
		return log(M3DGL_SUCCESS_LOADED_FROM_EMBED_FILE, shortName);
	}
	else if (pTexture->mWidth && pTexture->mHeight == 0 && pTexture->pcData)
	{
		ILenum formatHint;
		if (pTexture->CheckFormat("jpg"))
			formatHint = IL_JPG;
		else 
			if (pTexture->CheckFormat("png"))
			formatHint = IL_PNG;
		else
		{
			log(M3DGL_WARNING_EMBED_FILE_UNKNOWN_FORMAT, pTexture->achFormatHint, shortName);
			return false;
		}

		// initialise IL
		if (!bIlInitialised)
			ilInit();
		bIlInitialised = true;

		// destroy previous image
		destroy();

		// generate IL image id
		ilGenImages(1, &m_idImage);

		// bind IL image and load
		ilBindImage(m_idImage);
		ilEnable(IL_ORIGIN_SET);
		ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
		if (ilLoadL(formatHint, pTexture->pcData, pTexture->mWidth))
		{
			ilConvertImage(format, IL_UNSIGNED_BYTE);

			m_width = ilGetInteger(IL_IMAGE_WIDTH);
			m_height = ilGetInteger(IL_IMAGE_HEIGHT);
			m_pBits = ilGetData();

			return log(M3DGL_SUCCESS_LOADED_FROM_EMBED_FILE, shortName);
		}
		else
		{
			log(M3DGL_WARNING_CANNOT_LOAD_FROM_EMBED_FILE, shortName);
			return false;
		}
	}
	else
	{
		log(M3DGL_WARNING_CANNOT_LOAD_FROM_EMBED_FILE, shortName);
		return false;
	}
}

void C3dglBitmap::destroy()
{
	if (m_idImage)
		ilDeleteImages(1, &m_idImage);
	m_idImage = 0;
	m_width = m_height = 0;
	m_pBits = NULL;
}
