/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Version 3.0 - June 2022
Copyright (C) 2013-22 by Jarek Francik, Kingston University, London, UK

Common definitions
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

#ifndef __CommonDef_h_
#define __CommonDef_h_

namespace _3dgl
{
	// Standard Vertex Attributes / Buffers
	enum ATTRIB_STD {
		ATTR_VERTEX,
		ATTR_NORMAL,
		ATTR_TEXCOORD,
		ATTR_TANGENT,
		ATTR_BITANGENT,
		ATTR_COLOR,
		ATTR_BONE_ID,
		ATTR_BONE_WEIGHT,
		
		ATTR_COUNT,							// total attribute count
		ATTR_COUNT_BASIC = ATTR_TANGENT,	// basic attribute count (vertex, normal, tex-coord) used in fixed pipeline and sky boxes
		ATTR_COUNT_EXT = ATTR_COLOR			// extended attribute count, used by terrain objects (incl. tangent and bitangent)
	};

	// Standard Uniform Variables
	enum UNI_STD {
		UNI_MODELVIEW,
		UNI_MAT_AMBIENT,
		UNI_MAT_DIFFUSE,
		UNI_MAT_SPECULAR,
		UNI_MAT_EMISSIVE,
		UNI_MAT_SHININESS,
		
		UNI_COUNT							// total standard unoform count
	};

	// Maximum Bones per Vertex
	const int MAX_BONES_PER_VERTEX = 4;


}; // namespace _3dgl

#endif