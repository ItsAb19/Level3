/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Version 3.0 - June 2022
Copyright (C) 2013-22 by Jarek Francik, Kingston University, London, UK

Implementation of the base library class
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
#ifndef __3dglObject_h_
#define __3dglObject_h_

#include <string>

// Error/Warning messages & logger
#include <3dgl/Logger.h>

// Include 3DGL API import/export settings
#include "3dglapi.h"

namespace _3dgl
{
	class MY3DGL_API C3dglObject
	{
	public:
		C3dglObject()
		{
		}

		// this function must be overriden in each inheriting class
		virtual std::string getName() const = 0;
		
	protected:
		// Warning/Error logger - mostly for internal use
		// Use Warning/Error message codes as listed in Logger.h
		template<typename ... Args>
		bool log(unsigned nCode, Args... args) const
		{
			return C3dglLogger::log(nCode, getName(), args ...);
		}

	};
}; // namespace _3dgl

#endif // __3dglObject_h_
