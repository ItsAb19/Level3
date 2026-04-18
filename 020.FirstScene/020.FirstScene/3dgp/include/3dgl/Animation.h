/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Version 3.0 - June 2022
Copyright (C) 2013-22 by Jarek Francik, Kingston University, London, UK

Implementation of an Animation loader/solver class
Uses AssImp (Open Asset Import Library) Library to interpret animation information
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

#ifndef __3dglAnimation_h_
#define __3dglAnimation_h_

// Include GLM core features
#include "../glm/glm.hpp"

// Include 3DGL API import/export settings
#include "Object.h"

// standard libraries
#include <vector>
#include <map>

struct aiAnimation;
struct aiNode;

namespace _3dgl
{
	class C3dglModel;

	class MY3DGL_API C3dglAnimation : public C3dglObject
	{
	private:
		// Owner
		C3dglModel* m_pOwner;

		// Animation data
		const aiAnimation* m_pAnim;

		// Look-up table
#pragma warning(push)
#pragma warning(disable: 4251)
		std::map<const aiNode*, std::pair<size_t, size_t> > m_lookUp;
#pragma warning(pop)

	public:
		C3dglAnimation(C3dglModel* pOwner);

		void create(const aiAnimation* pAnim, const aiNode* pRootNode);

		std::string getName() const;
		double getDuration() const;
		double getTicksPerSecond() const;

		void readNodeHierarchy(float time, const aiNode* pNode, std::vector<glm::mat4>& transforms, const glm::mat4 parentT = glm::mat4(1)) const;
	};
}

#endif