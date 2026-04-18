/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Version 3.0 - June 2022
Copyright (C) 2013-22 by Jarek Francik, Kingston University, London, UK

A very simple bitmap class.
----------------------------------------------------------------------------------
This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source distribution.

   Jarek Francik
   jarek@kingston.ac.uk
*********************************************************************************/
#ifndef __3dglBitmap_h_
#define __3dglBitmap_h_

#include "Object.h"
#include <string>

struct aiTexture;

namespace _3dgl
{

class MY3DGL_API C3dglBitmap : public C3dglObject
{
	unsigned int m_idImage;

	long m_width;
	long m_height;
	void* m_pBits;

public:
	C3dglBitmap();
	C3dglBitmap(const std::string fname, unsigned format);
	~C3dglBitmap()	{ destroy(); }

	bool load(const std::string fname, unsigned format);
	bool load(const aiTexture* pTexture, unsigned format);
	void destroy();

	long getWidth()	const			{ return m_idImage ? m_width : 0;  }
	long getHeight() const			{ return m_idImage ? m_height : 0; }
	void *getBits()	const			{ return m_idImage ? m_pBits : NULL; }

	std::string getName() const		{ return "Texture"; }
};

}; // namespace _3dgl

#endif