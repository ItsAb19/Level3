/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Version 3.0 - June 2022
Copyright (C) 2013-22 by Jarek Francik, Kingston University, London, UK

Implementation of a simple 3D model class
Partially based on http://ogldev.atspace.co.uk/www/tutorial38/tutorial38.html
Uses AssImp (Open Asset Import Library) Library to load model files
Uses DevIL Image Library to load textures
Main features:
- VBO based rendering (vertices, normals, tangents, bitangents, colours, bone ids & weights
- automatically loads textures
- integration with C3dglProgram shader program
- very simple Bounding Boxes
- support for skeletal animation
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

#ifndef __3dglModel_h_
#define __3dglModel_h_

#include "Material.h"
#include "Mesh.h"
#include "Animation.h"

// standard libraries
#include <vector>
#include <map>

#include "../glm/mat4x4.hpp"

struct aiScene;
struct aiNode;

namespace _3dgl
{
	class C3dglProgram;

	class MY3DGL_API C3dglModel : public C3dglObject
	{
		const aiScene* m_pScene;					// parent scene (the main AssImp object)
		std::string m_name;							// model name (derived from the filename)
		bool m_bFBXImportPreservePivots;			// binary flag needed to tweak some quirky effects in AssImp FBX importer. Should be set to false

		// vectrors of meshes, materials and animations
#pragma warning(push)
#pragma warning(disable: 4251)
		std::vector<C3dglMesh> m_meshes;
		std::vector<C3dglMaterial> m_materials;
		std::vector<C3dglAnimation> m_animations;

		// Bones: two-way mapping
		std::vector<std::pair<std::string, glm::mat4> > m_vecBones;	// maps ids to pairs<bone name, bone offset matrix>
		std::map<std::string, size_t> m_mapBones;	// maps bone names back to ids
		glm::mat4 m_globInvT;						// global transformation matrix (transposed)
#pragma warning(pop)

	public:
		C3dglModel();
		~C3dglModel() { destroy(); }

		const aiScene* getScene() const { return m_pScene; }

		// Loading
		// load a model from file
		bool load(const char* filename, unsigned int flags = 0, C3dglProgram* pProgram = NULL);
		// create a model from AssImp handle - useful if you are using AssImp directly
		void create(const aiScene* pScene, C3dglProgram* pProgram);
		// create material information and load textures from MTL file - must be preceded by either load or create
		void loadMaterials(const char* pDefTexPath = NULL);
		// load animations. By default loads animations from the current model. 
		// Any other model must be structurally compatible. Returns number of animations successfully loaded.
		unsigned loadAnimations();
		unsigned loadAnimations(C3dglModel* pCompatibleModel);
		// destroy the model (releases all meshes, materials and animations loaded)
		void destroy();

		// Controls some quirky behaviour in AssImp FBX importer. By default set to false (disable preserve pivots mode)
		// Can be changed to true in case the model doesn't appear right. Note: this flag should be set before calling load funcion!
		bool getFBXImportPreservePivotsFlag() const	 { return m_bFBXImportPreservePivots; }
		void setFBXImportPreservePivotsFlag(bool b)	 { m_bFBXImportPreservePivots = b; }

		// Rendering
		// render the entire model
		void render(glm::mat4 matrix, GLsizei instances = 1, C3dglProgram* pProgram = NULL) const;
		// render one of the main nodes - see getMainNodeCount below
		void render(unsigned iNode, glm::mat4 matrix, GLsizei instances = 1, C3dglProgram* pProgram = NULL) const;
		// render a single node
		void renderNode(aiNode* pNode, glm::mat4 m, GLsizei instances = 1, C3dglProgram* pProgram = NULL) const;
		// returns the count of main nodes
		unsigned getMainNodeCount() const;

		// Buffer creation: creates attribute buffers for each mesh
		void createVertexBuffers(GLint attrLocation, size_t instances, GLint size, float* data, GLsizei stride = 0, GLuint divisor = 0, GLenum usage = GL_STATIC_DRAW);
		void createVertexBuffers(GLint attrLocation, size_t instances, GLint size, int* data, GLsizei stride = 0, GLuint divisor = 0, GLenum usage = GL_STATIC_DRAW);

		// Additional pointers: add attrib pointers (with offset) to existing buffers, for each mesh
		void addAttribPointers(GLint attrLocation, GLint attrFirstLocation, size_t instances, GLint size, GLsizei stride, size_t offset, GLuint divisor = 0, GLenum usage = GL_STATIC_DRAW);
		void addAttribIPointers(GLint attrLocation, GLint attrFirstLocation, size_t instances, GLint size, GLsizei stride, size_t offset, GLuint divisor = 0, GLenum usage = GL_STATIC_DRAW);

		// Mesh functions
		bool hasMeshes() const						{ return m_meshes.size() > 0; }
		size_t getMeshCount() const					{ return m_meshes.size(); }
		C3dglMesh *getMesh(size_t i)				{ return (i < m_meshes.size()) ? &m_meshes[i] : NULL; }
		size_t getMeshIndex(const C3dglMesh* p) const { return p - &m_meshes[0]; }
		size_t createNewMesh()						{ size_t nIndex = m_meshes.size(); m_meshes.push_back(C3dglMesh(this)); return nIndex; }

		// Material functions
		bool hasMaterials() const					{ return m_materials.size() > 0; }
		size_t getMaterialCount() const				{ return m_materials.size(); }
		C3dglMaterial *getMaterial(size_t i)		{ return (i < m_materials.size()) ? &m_materials[i] : NULL; }
		size_t getMaterialIndex(C3dglMaterial* p) const { return p - &m_materials[0]; }
		size_t createNewMaterial()					{ size_t nIndex = m_materials.size(); m_materials.push_back(C3dglMaterial(this)); return nIndex; }

		// Animation functions
		bool hasAnimations()  const					{ return m_animations.size() > 0; }
		size_t getAnimationCount()  const			{ return m_animations.size(); }
		bool hasAnimation(unsigned iAnim)  const	{ return iAnim < getAnimationCount(); }
		const C3dglAnimation* getAnimation(size_t i) const	{ return (i < m_animations.size()) ? &m_animations[i] : NULL; }
		size_t getAnimationIndex(C3dglAnimation* p) const	{ return p - &m_animations[0]; }

		// retrieve bone transformations for the given time point. The vector transforms will be cleared and resized to reflect the actual number of bones
		void getAnimData(unsigned iAnim, float time, std::vector<glm::mat4>& transforms) const;

		// Advanced functions
		// retrieves the transform associated with the given node. If (bRecursive) the transform is recursively combined with parental transform(s)
		void getNodeTransform(aiNode *pNode, float pMatrix[16], bool bRecursive = true) const;
	
		// Bone system functions
		bool hasBones() const						{ return m_vecBones.size() > 0; }		// true if any bones found in the model
		size_t getBoneCount() const					{ return m_vecBones.size(); }			// returns the bone count
		bool hasBone(size_t id) const				{ return id < getBoneCount(); }			// true if bone exists in the model
		bool hasBone(std::string boneName) const	{ return m_mapBones.find(boneName) != m_mapBones.end(); }		// true if bone exists in the model
		std::string getBoneName(size_t id) const	{ return hasBone(id) ? m_vecBones[id].first : ""; }				// name of the bone
		glm::mat4 getBone(size_t id) const			{ return hasBone(id) ? m_vecBones[id].second : glm::mat4(1); }	// bone matrix
		size_t getBoneId(std::string boneName) const;						// returns bone id, (size_t)-1 if bone name unknown
		size_t getOrAddBone(std::string boneName, glm::mat4 offsetMatrix);	// as above, creates a new bone if bone name unknown yet
		glm::mat4 getGlobalInvT() const				{ return m_globInvT; }

		// Bounding Box Functions
		// BB for the entire model
		void getAABB(glm::vec3 BB[2]) const;
		// BB for one of the main nodes - see getMainNodeCount function
		void getAABB(unsigned iNode, glm::vec3 BB[2]) const;
		// BB for a single node (low-level, mostly for internal use)
		void getAABB(aiNode* pNode, glm::vec3 BB[2], glm::mat4 m = glm::mat4(1)) const;

		void stats(unsigned level = 0) const;

		std::string getName() const { return "Model \"" + m_name + "\""; }
	};
}; // namespace _3dgl

#endif