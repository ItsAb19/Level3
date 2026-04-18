/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Version 3.0 - June 2022
Copyright (C) 2013-22 by Jarek Francik, Kingston University, London, UK

Implementation of 3D mesh class
Partially based on http://ogldev.atspace.co.uk/www/tutorial38/tutorial38.html
Uses AssImp (Open Asset Import Library) Library to load model files
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

#ifndef __3dglMesh_h_
#define __3dglMesh_h_

// Include GLM core features
#include "../glm/glm.hpp"

#include "VAO.h"

struct aiMesh;

namespace _3dgl
{
	class C3dglModel;
	class C3dglMaterial;
	class C3dglProgram;

	class MY3DGL_API C3dglMesh : public C3dglVertexAttrObject
	{
		C3dglModel *m_pOwner;		// owner model
		std::string m_name;			// mesh name
		const aiMesh *m_pMesh;		// underlying ASSIMP data structure

		size_t m_nBones;			// number of bones

		size_t m_matIndex;			// Material Index - points to the main m_materials collection
		
		glm::vec3 m_aabb[2];		// Bounding Volume

	protected:
		size_t getBuffers(const aiMesh* pMesh, const GLint *attrId, size_t attrCount, void** attrData, size_t* attrSize) const;
		size_t getIndexBuffer(const aiMesh* pMesh, void** indexData, size_t *indSize) const;
		void cleanUp(size_t attrCount, void** attrData, void *indexData) const;		// call after getBuffers well data no longer required
		void getBoundingVolume(const aiMesh* pMesh, size_t nVertices, glm::vec3& aabb0, glm::vec3& aabb1) const;

	public:
		C3dglMesh(C3dglModel* pOwner = NULL);
		virtual ~C3dglMesh() { destroy(); }

		// Create a mesh using ASSIMP data and a shader progrem (currently used one if NULL)
		void create(const aiMesh* pMesh, C3dglProgram* pProgram = NULL);

		// Using ASSIMP data, read attribute or index buffer data. A binary buffer will be allocated and a pointer stored in *ppData, 
		// *indSize will be filled with the element size and the function returns number of elements (0 if data unavailable).
		// The retuen value is also: number of vertices for getAttrData, number of indices for getIndexData.
		size_t getAttrData(enum ATTRIB_STD attr, void** ppData, size_t* indSize) const;
		size_t getIndexData(void** ppData, size_t* indSize) const;

		// Bone count
		size_t getBoneCount() const				{ return m_nBones; }

		// Material functions
		C3dglMaterial *getMaterial() const;
		C3dglMaterial *createNewMaterial();

		// Bounding volume
		void getAABB(glm::vec3 aabb[2]) const	{ aabb[0] = m_aabb[0]; aabb[1] = m_aabb[1]; }
	
		std::string getName() const;
	};

}; // namespace _3dgl

#endif