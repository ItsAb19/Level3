/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Version 3.0 - June 2022
Copyright (C) 2013-22 by Jarek Francik, Kingston University, London, UK

3dglTools: Simple tool functions
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

#ifndef __3dglTools_h_
#define __3dglTools_h_

#include "../glm/glm.hpp"
#include "../glm/gtc/constants.hpp"

// Include 3DGL API import/export settings
#include "3dglapi.h"

namespace _3dgl
{
	class C3dglMesh;
	class C3dglProgram;



	// pass the matrixView to get the current camera position
	inline glm::vec3 getPos(glm::mat4 m)
	{
		return glm::inverse(m)[3];
	}

	// pass the matrixView to get the current camera pitch
	inline float getPitch(glm::mat4 m)
	{
		return atan2(m[1][2], sqrt(m[0][2] * m[0][2] + m[2][2] * m[2][2]));
	}

	// pass the matrixView to get the current camera yaw
	inline float getYaw(glm::mat4 m)
	{
		return atan2(m[2][2], -m[0][2]);
	}

	// pass the matrixView to get the current camera roll
	inline float getRoll(glm::mat4 m)
	{
		return atan2(m[1][0], sqrt(m[0][0] * m[0][0] + m[2][0] * m[2][0]));
	}

	// prints text on-screen at (x, y) screen coordinates, using color, font and align mode (left, right or centre)
	// x: x coordinate; if x < 0 than |x| determines the distance from the right margin of the window. The text will be right-aligned regardless of the align setting
	// y: y coordinate; if y < 0 than |y| determines the distance from the top margin of the window
	// Examples: print(0, 0, "lower-left corner"); print(-1, -1, "upper-right corner"); 
	//           print(640, -10, std::format("X = {}  Y = {}", X, Y), vec3(1.0f, 0.5f, 0.0f), FONT_FIXED_15, CENTRE);
	enum FONT { FONT_FIXED_15, FONT_FIXED_13, FONT_TIMES_ROMAN_10, FONT_TIMES_ROMAN_24, FONT_HELVETICA_10, FONT_HELVETICA_12, FONT_HELVETICA_18 };
	enum ALIGN { LEFT, RIGHT, CENTRE };
	void MY3DGL_API print(int x, int y, std::string text, glm::vec3 color = glm::vec3(1, 1, 1), enum FONT = FONT_HELVETICA_18, enum ALIGN = LEFT);

	// calculates fps from deltaTime (time since last frame) and displays it on-screen;
	void MY3DGL_API print(int x, int y, float deltaTime, glm::vec3 color = glm::vec3(1, 1, 1), enum FONT = FONT_HELVETICA_18, enum ALIGN = LEFT);

	// calculates camera position from the view matrix and displays it on-screen;
	void MY3DGL_API print(int x, int y, glm::mat4 matrixView, glm::vec3 color = glm::vec3(1, 1, 1), enum FONT = FONT_HELVETICA_18, enum ALIGN = LEFT);

	// converts a height map provided as an image file (fileImage) to a terrain mesh using scaleHeight to scale the terrain height
	// output stored either externally as an OBJ mesh file or internally in a C3dglMesh mesh file provided
	bool MY3DGL_API convHeightmap2OBJ(const std::string fileImage, float scaleHeight, const std::string fileOBJ);
	bool MY3DGL_API convHeightmap2Mesh(const std::string fileImage, float scaleHeight, C3dglMesh* pMesh, C3dglProgram* pProgram = NULL);

}; // namespace _3dgl

#endif