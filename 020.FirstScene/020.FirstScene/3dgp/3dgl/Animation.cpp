/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Version 3.0 - June 2022
Copyright (C) 2013-22 by Jarek Francik, Kingston University, London, UK
*********************************************************************************/
#include "pch.h"
#include <fstream>
#include <3dgl/Animation.h>
#include <3dgl/Model.h>

// assimp include file
#include <assimp/scene.h>

// GLM include files
#include "../glm/gtc/type_ptr.hpp"

using namespace _3dgl;

C3dglAnimation::C3dglAnimation(C3dglModel* pOwner) : m_pOwner(pOwner), m_pAnim(NULL)
{
}

std::string C3dglAnimation::getName() const
{ 
	return m_pAnim->mName.data; 
}

double C3dglAnimation::getDuration() const
{ 
	return m_pAnim->mDuration; 
}

double C3dglAnimation::getTicksPerSecond() const 
{ 
	return m_pAnim->mTicksPerSecond; 
}

void C3dglAnimation::create(const aiAnimation* pAnim, const aiNode* pRootNode)
{
	m_pAnim = pAnim;

	// Create the m_lookUp table that maps aiNode* node addresses into pair<channel id, bone id>:
	// Channel Id is the seq no in the animation channel buffer and bone id is stored with the owner model and used directly by shaders

	// first: map node names into node addresses (using a recursive lambda function)
	std::map<std::string, const aiNode*> mymap;
	auto __createMap = [&](auto&& __createMap, const aiNode* pNode, std::map<std::string, const aiNode*>& map) -> void {
		map[pNode->mName.data] = pNode;
		for (aiNode* pChildNode : std::vector<aiNode*>(pNode->mChildren, pNode->mChildren + pNode->mNumChildren))
			__createMap(__createMap, pChildNode, map);
	};
	__createMap(__createMap, pRootNode, mymap);

	// create the lookUp structure for the nodes included in the animation channel
	for (unsigned idChannel = 0; idChannel < pAnim->mNumChannels; idChannel++)
	{
		std::string strNodeName = pAnim->mChannels[idChannel]->mNodeName.data;
		const aiNode* pNode = mymap[strNodeName];
		size_t idBone = m_pOwner->getBoneId(strNodeName);
		m_lookUp[pNode] = std::pair<size_t, size_t>(idChannel, idBone);
	}

	// Some bones have a boneId but are not described in animation channels...
	for (unsigned idBone = 0; idBone < m_pOwner->getBoneCount(); idBone++)
	{
		const aiNode* pNode = mymap[m_pOwner->getBoneName(idBone)];
		if (m_lookUp.find(pNode) != m_lookUp.end())
			continue;	// if the node already in the look-up structure, no longer interesting
		m_lookUp[pNode] = std::pair<size_t, size_t>(size_t(-1), idBone);
	}

	// If a node has no boneId and no ChannelIf - not interesting!
}

static aiVector3D Interpolate(float AnimationTime, const aiVectorKey* pKeys, unsigned nKeys)
{
	// find a pair of keys to interpolate
	unsigned i = 0;
	while (i < nKeys - 1 && AnimationTime >= (float)pKeys[i + 1].mTime)
		i++;

	// if out of bounds, return the last key
	if (i >= nKeys - 1)
		return pKeys[nKeys - 1].mValue;

	// interpolate
	const aiVector3D& Start = pKeys[i].mValue;
	const aiVector3D& End = pKeys[i + 1].mValue;
	float f = (AnimationTime - (float)pKeys[i].mTime) / ((float)(pKeys[i + 1].mTime - pKeys[i].mTime));
	return Start + f * (End - Start);
}

static aiQuaternion Interpolate(float AnimationTime, const aiQuatKey* pKeys, unsigned nKeys)
{
	// find a pair of keys to interpolate
	unsigned i = 0;
	while (i < nKeys - 1 && AnimationTime >= (float)pKeys[i + 1].mTime)
		i++;

	// if out of bounds, return the last key
	if (i >= nKeys - 1)
		return pKeys[nKeys - 1].mValue;

	const aiQuaternion& StartRotationQ = pKeys[i].mValue;
	const aiQuaternion& EndRotationQ = pKeys[i + 1].mValue;
	float f = (AnimationTime - (float)pKeys[i].mTime) / ((float)(pKeys[i + 1].mTime - pKeys[i].mTime));
	aiQuaternion q;
	aiQuaternion::Interpolate(q, StartRotationQ, EndRotationQ, f);	// spherical interpolation (SLERP)
	return q.Normalize();
}

static aiMatrix4x4 Interpolate(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	// Interpolate position, rotation and scaling
	aiVector3D vecTranslate = Interpolate(AnimationTime, pNodeAnim->mPositionKeys, pNodeAnim->mNumPositionKeys);
	aiQuaternion quatRotate = Interpolate(AnimationTime, pNodeAnim->mRotationKeys, pNodeAnim->mNumRotationKeys);
	aiVector3D vecScale = Interpolate(AnimationTime, pNodeAnim->mScalingKeys, pNodeAnim->mNumScalingKeys);

	// create matrices
	aiMatrix4x4 matTranslate, matScale;
	aiMatrix4x4::Translation(vecTranslate, matTranslate);
	aiMatrix4x4 matRotate = aiMatrix4x4(quatRotate.GetMatrix());
	aiMatrix4x4::Scaling(vecScale, matScale);
	return matTranslate * matRotate * matScale;
}


void C3dglAnimation::readNodeHierarchy(float time, const aiNode* pNode, std::vector<glm::mat4>& transforms, const glm::mat4 parentT) const
{
	glm::mat4 transform = parentT * glm::transpose(glm::make_mat4((float*)&pNode->mTransformation));

	auto it = m_lookUp.find(pNode);
	if (it != m_lookUp.end())
	{
		size_t idChannel = it->second.first;
		size_t idBone = it->second.second;
	
		if (idChannel < m_pAnim->mNumChannels)
		{
			aiMatrix4x4 mat = Interpolate(time, m_pAnim->mChannels[idChannel]);
			transform = parentT * glm::transpose(glm::make_mat4((float*)&mat));
		}

		if (idBone < m_pOwner->getBoneCount())
			transforms[idBone] = m_pOwner->getGlobalInvT() * transform * m_pOwner->getBone(idBone);
	}
	for (aiNode* pChildNode : std::vector<aiNode*>(pNode->mChildren, pNode->mChildren + pNode->mNumChildren))
		readNodeHierarchy(time, pChildNode, transforms, transform);
}
