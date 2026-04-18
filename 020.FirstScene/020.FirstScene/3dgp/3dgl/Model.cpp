/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Version 3.0 - June 2022
Copyright (C) 2013-22 by Jarek Francik, Kingston University, London, UK
*********************************************************************************/
#include "pch.h"
#include <iostream>
#include <3dgl/Model.h>
#include <3dgl/Shader.h>

// assimp include file
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include <assimp/cimport.h>
#include <assimp/DefaultLogger.hpp>

// GLM include files
#include "../glm/vec3.hpp"
#include "../glm/vec4.hpp"
#include "../glm/mat4x4.hpp"
#include "../glm/gtc/type_ptr.hpp"

using namespace _3dgl;

C3dglModel::C3dglModel() : C3dglObject(), m_globInvT(1)
{ 
	m_pScene = NULL; 
	m_bFBXImportPreservePivots = false;
}

bool C3dglModel::load(const char* filename, unsigned int flags, C3dglProgram* pProgram)
{
	if (flags == 0)
		flags = aiProcessPreset_TargetRealtime_MaxQuality;

	m_name = filename;
	size_t i = m_name.find_last_of("/\\");
	if (i != std::string::npos) m_name = m_name.substr(i + 1);
	i = m_name.find_last_of(".");
	if (i != std::string::npos) m_name = m_name.substr(0, i);

	unsigned logOptions = (C3dglLogger::getOptions() & C3dglLogger::LOGGER_SHOW_ASSIMP_VERBOSE_MESSAGES) >> 2;
	if (logOptions != 0)
		Assimp::DefaultLogger::create("", (Assimp::Logger::LogSeverity)(logOptions - 1), aiDefaultLogStream_STDOUT);

	log(M3DGL_SUCCESS_IMPORTING_FILE, filename);
	aiPropertyStore* ps = ::aiCreatePropertyStore();
	::aiSetImportPropertyInteger(ps, AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, m_bFBXImportPreservePivots);
	const aiScene* pScene = aiImportFileExWithProperties(filename, flags, NULL, ps);
	
	if ((C3dglLogger::getOptions() & C3dglLogger::LOGGER_SHOW_ASSIMP_VERBOSE_MESSAGES) != 0)
		Assimp::DefaultLogger::kill();
	
	if (pScene == NULL)
		return log(M3DGL_ERROR_AI, aiGetErrorString());
	create(pScene, pProgram);
	return true;
}

void C3dglModel::create(const aiScene* pScene, C3dglProgram *pProgram)
{
	// create meshes
	m_pScene = pScene;
	m_meshes.resize(m_pScene->mNumMeshes, C3dglMesh(this));
	aiMesh** ppMesh = m_pScene->mMeshes;
	for (C3dglMesh& mesh : m_meshes)
		mesh.create(*ppMesh++, pProgram);
}

void C3dglModel::loadMaterials(const char* pTexRootPath)
{
	if (!m_pScene) return;

	m_materials.resize(m_pScene->mNumMaterials, C3dglMaterial(this));
	aiMaterial** ppMaterial = m_pScene->mMaterials;
	for (C3dglMaterial& material : m_materials)
		material.create(*ppMaterial++, pTexRootPath);
}

unsigned C3dglModel::loadAnimations()
{
	return loadAnimations(this);
}

unsigned C3dglModel::loadAnimations(C3dglModel* pCompatibleModel)
{
	m_globInvT = glm::inverse(glm::transpose(glm::make_mat4((float*)&m_pScene->mRootNode->mTransformation)));

	if (pCompatibleModel == NULL)
		pCompatibleModel = this;

	const aiScene* pScene = pCompatibleModel->m_pScene;

	if (!pScene || !pScene->HasAnimations()) return 0;
	if (getBoneCount() == 0)
	{
		pCompatibleModel->log(M3DGL_WARNING_SKINNING_NOT_IMPLEMENTED);
		return 0;
	}

	m_animations.resize(pScene->mNumAnimations, C3dglAnimation(this));
	aiAnimation** ppAnimation = pScene->mAnimations;
	for (C3dglAnimation& animation : m_animations)
		animation.create(*ppAnimation++, getScene()->mRootNode);
	
	return pScene->mNumAnimations;
}

void C3dglModel::destroy()
{
	if (m_pScene)
	{
		for (C3dglMesh mesh : m_meshes)
			mesh.destroy();
		for (C3dglMaterial mat : m_materials)
			mat.destroy();
		aiReleaseImport(m_pScene);
		m_pScene = NULL;
	}
}

void C3dglModel::renderNode(aiNode* pNode, glm::mat4 m, GLsizei instances, C3dglProgram* pProgram) const
{
	m *= glm::transpose(glm::make_mat4((GLfloat*)&pNode->mTransformation));

	// render all meshes (and their materials)
	for (unsigned iMesh : std::vector<unsigned>(pNode->mMeshes, pNode->mMeshes + pNode->mNumMeshes))
	{
		const C3dglMesh* pMesh = &m_meshes[iMesh];
		const C3dglMaterial* pMaterial = pMesh->getMaterial();
		if (pMaterial)
			pMaterial->render(pProgram);
		pMesh->render(m, instances, pProgram);
		if (pMaterial)
			pMaterial->postRender(pProgram);
	}

	// draw all children
	for (aiNode* p : std::vector<aiNode*>(pNode->mChildren, pNode->mChildren + pNode->mNumChildren))
		renderNode(p, m, instances, pProgram);
}

void C3dglModel::render(glm::mat4 matrix, GLsizei instances, C3dglProgram* pProgram) const
{ 
	if (m_pScene->mRootNode) 
		renderNode(m_pScene->mRootNode, matrix, instances, pProgram);
}

void C3dglModel::render(unsigned iNode, glm::mat4 matrix, GLsizei instances, C3dglProgram* pProgram) const
{
	// update transform
	matrix *= glm::transpose(glm::make_mat4((GLfloat*)&m_pScene->mRootNode->mTransformation));

	if (iNode <= getMainNodeCount())
		renderNode(m_pScene->mRootNode->mChildren[iNode], matrix, instances, pProgram);
}

unsigned C3dglModel::getMainNodeCount() const
{ 
	return (m_pScene && m_pScene->mRootNode) ? m_pScene->mRootNode->mNumChildren : 0; 
}

void C3dglModel::createVertexBuffers(GLint attrLocation, size_t instances, GLint size, float* data, GLsizei stride, GLuint divisor, GLenum usage)
{
	for (int i = 0; i < getMeshCount(); i++)
		getMesh(i)->createVertexBuffer(attrLocation, instances, size, data, stride, divisor, usage);
}

void C3dglModel::createVertexBuffers(GLint attrLocation, size_t instances, GLint size, int* data, GLsizei stride, GLuint divisor, GLenum usage)
{
	for (int i = 0; i < getMeshCount(); i++)
		getMesh(i)->createVertexBuffer(attrLocation, instances, size, data, stride, divisor, usage);
}

void C3dglModel::addAttribPointers(GLint attrLocation, GLint attrFirstLocation, size_t instances, GLint size, GLsizei stride, size_t offset, GLuint divisor, GLenum usage)
{
	for (int i = 0; i < getMeshCount(); i++)
	{
		GLuint bufferId;
		C3dglMesh* pMesh = getMesh(i);
		pMesh->getVertexBufferId(attrFirstLocation, bufferId);
		pMesh->addAttribPointer(attrLocation, bufferId, instances, size, stride, offset, divisor, usage);
	}
}

void C3dglModel::addAttribIPointers(GLint attrLocation, GLint attrFirstLocation, size_t instances, GLint size, GLsizei stride, size_t offset, GLuint divisor, GLenum usage)
{
	for (int i = 0; i < getMeshCount(); i++)
	{
		GLuint bufferId;
		C3dglMesh* pMesh = getMesh(i);
		pMesh->getVertexBufferId(attrFirstLocation, bufferId);
		pMesh->addAttribIPointer(attrLocation, bufferId, instances, size, stride, offset, divisor, usage);
	}
}

void C3dglModel::getNodeTransform(aiNode* pNode, float pMatrix[16], bool bRecursive) const
{
	aiMatrix4x4 m1, m2;
	if (bRecursive && pNode->mParent)
		getNodeTransform(pNode->mParent, (float*)&m1, true);

	m2 = pNode->mTransformation;
	aiTransposeMatrix4(&m2);

	*((aiMatrix4x4*)pMatrix) = m2 * m1;
}

size_t C3dglModel::getBoneId(std::string boneName) const
{
	auto it = m_mapBones.find(boneName);
	if (it != m_mapBones.end())
		return it->second;
	else
		return (size_t)-1;
}

size_t C3dglModel::getOrAddBone(std::string boneName, glm::mat4 offsetMatrix)
{
	auto it = m_mapBones.find(boneName);
	if (it != m_mapBones.end())
		return it->second;
	else
	{
		size_t id = m_vecBones.size();
		m_vecBones.push_back(std::pair<std::string, glm::mat4>(boneName, offsetMatrix));
		m_mapBones[boneName] = id;
		return id;
	}
}

void C3dglModel::getAABB(glm::vec3 BB[2]) const
{ 
	BB[0] = glm::vec3(1e10f, 1e10f, 1e10f);
	BB[1] = glm::vec3(-1e10f, -1e10f, -1e10f);
	if (m_pScene->mRootNode)
		getAABB(m_pScene->mRootNode, BB); 
}

void C3dglModel::getAABB(unsigned iNode, glm::vec3 BB[2]) const
{
	BB[0] = glm::vec3(1e10f, 1e10f, 1e10f);
	BB[1] = glm::vec3(-1e10f, -1e10f, -1e10f);

	if (iNode <= getMainNodeCount())
		getAABB(m_pScene->mRootNode->mChildren[iNode], BB, glm::transpose(glm::make_mat4((GLfloat*)&m_pScene->mRootNode->mTransformation)));
}

void C3dglModel::getAABB(aiNode* pNode, glm::vec3 BB[2], glm::mat4 m) const
{
	m *= glm::transpose(glm::make_mat4((GLfloat*)&pNode->mTransformation));

	for (unsigned iMesh : std::vector<unsigned>(pNode->mMeshes, pNode->mMeshes + pNode->mNumMeshes))
	{
		glm::vec3 bb[2];
		m_meshes[iMesh].getAABB(bb);

		glm::vec4 bb4[2];
		bb4[0] = m * glm::vec4(bb[0], 1);
		bb4[1] = m * glm::vec4(bb[1], 1);

		BB[0].x = std::min(BB[0].x, bb4[0].x);
		BB[0].y = std::min(BB[0].y, bb4[0].y);
		BB[0].z = std::min(BB[0].z, bb4[0].z);
		BB[0].x = std::min(BB[0].x, bb4[1].x);
		BB[0].y = std::min(BB[0].y, bb4[1].y);
		BB[0].z = std::min(BB[0].z, bb4[1].z);

		BB[1].x = std::max(BB[1].x, bb4[0].x);
		BB[1].y = std::max(BB[1].y, bb4[0].y);
		BB[1].z = std::max(BB[1].z, bb4[0].z);
		BB[1].x = std::max(BB[1].x, bb4[1].x);
		BB[1].y = std::max(BB[1].y, bb4[1].y);
		BB[1].z = std::max(BB[1].z, bb4[1].z);
	}

	// check all children
	for (aiNode* p : std::vector<aiNode*>(pNode->mChildren, pNode->mChildren + pNode->mNumChildren))
		getAABB(p, BB, m);
}

//////////////////////////////////////////////////////////////////////////////////////
// Articulated Animation Functions

void C3dglModel::getAnimData(unsigned iAnim, float time, std::vector<glm::mat4>& transforms) const
{
	transforms.resize(getBoneCount());	// 16 floats per bone matrix
	if (!hasAnimation(iAnim))
	{
		if (!hasBones()) transforms.resize(1);
		aiMatrix4x4 m;
		for (unsigned iBone = 0; iBone < transforms.size(); iBone++)
			memcpy(&transforms[iBone], &m, sizeof(m));
	}
	else
	{
		float fTicksPerSecond = (float)getAnimation(iAnim)->getTicksPerSecond();
		if (fTicksPerSecond == 0) fTicksPerSecond = 25.0f;
		time = fmod(time * fTicksPerSecond, (float)getAnimation(iAnim)->getDuration());

		m_animations[iAnim].readNodeHierarchy(time, getScene()->mRootNode, transforms);
	}
}

void C3dglModel::stats(unsigned level) const
{
	// prep: count nodes
	unsigned nNodes = 0;
	auto countNodes = [&](auto&& countNodes, const aiNode* pNode) -> void {
		nNodes++;
		for (aiNode* pChildNode : std::vector<aiNode*>(pNode->mChildren, pNode->mChildren + pNode->mNumChildren))
			countNodes(countNodes, pChildNode);
	};
	countNodes(countNodes, m_pScene->mRootNode);

	C3dglLogger::log("** Statistics for the model: {}", getName());
	C3dglLogger::log("Nodes: {}, Meshes: {}, Materials: {}, Bones: {}, Animations: {}, Channels: {}",
		nNodes, getMeshCount(), getMaterialCount(), getBoneCount(), getAnimationCount(), hasAnimations() ? m_pScene->mAnimations[0]->mNumChannels : 0);
	if (level == 0) return;

	auto statNode = [&](auto&& statNode, std::string pred, aiNode* pNode) -> void
	{
		size_t idBone = getBoneId(pNode->mName.data);
		if (hasBone(idBone))
			C3dglLogger::log("{}Node: {} - BONE #{}", pred, pNode->mName.data, idBone);
		else
			C3dglLogger::log("{}Node: {} (no bone identified)", pred, pNode->mName.data);

		for (unsigned iMesh : std::vector<unsigned>(pNode->mMeshes, pNode->mMeshes + pNode->mNumMeshes))
			C3dglLogger::log("{}Mesh #{} ({}) - {} vertices, {} faces, {} bones", pred + "-", iMesh , m_pScene->mMeshes[iMesh]->mName.data, m_pScene->mMeshes[iMesh]->mNumVertices, m_pScene->mMeshes[iMesh]->mNumFaces, m_pScene->mMeshes[iMesh]->mNumBones);
		for (aiNode* pChildNode : std::vector<aiNode*>(pNode->mChildren, pNode->mChildren + pNode->mNumChildren))
			statNode(statNode, pred + " ", pChildNode);
	};
	statNode(statNode, "", m_pScene->mRootNode);

	if (m_pScene->mNumAnimations > 0)
	{
		C3dglLogger::log("Animation channels:");
		for (unsigned idChannel = 0; idChannel < m_pScene->mAnimations[0]->mNumChannels; idChannel++)
		{
			std::string strNodeName = m_pScene->mAnimations[0]->mChannels[idChannel]->mNodeName.data;
			size_t idBone = getBoneId(strNodeName);
			if (hasBone(idBone))
				C3dglLogger::log(" {}. {} - BONE #{}", idChannel, strNodeName, idBone);
			else
				C3dglLogger::log(" {}. {} - BONELESS", idChannel, strNodeName);
		}
	}
}
