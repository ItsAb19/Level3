/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Version 3.0 - June 2022
Copyright (C) 2013-22 by Jarek Francik, Kingston University, London, UK

Warning and Error Message Codes and the Logger implementation
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
#ifndef __3dglLogger_h_
#define __3dglLogger_h_

#include <format>

// Include 3DGL API import/export settings
#include "3dglapi.h"

namespace _3dgl
{
	enum
	{
		// Information/Success
		M3DGL_SUCCESS,
		M3DGL_SUCCESS_IMPORTING_FILE,
		M3DGL_SUCCESS_CREATED,
		M3DGL_SUCCESS_SRC_CODE_LOADED,
		M3DGL_SUCCESS_COMPILED,
		M3DGL_SUCCESS_LINKED,
		M3DGL_SUCCESS_ATTACHED,
		M3DGL_SUCCESS_ATTRIB_FOUND,
		M3DGL_SUCCESS_UNIFORM_FOUND,
		M3DGL_SUCCESS_VERIFICATION,
		M3DGL_SUCCESS_LOADED,
		M3DGL_SUCCESS_LOADED_FROM_EMBED_FILE,

		// Warnings
		M3DGL_WARNING_GENERIC = 200,
		M3DGL_WARNING_UNIFORM_NOT_FOUND,				// shader.cpp
		M3DGL_WARNING_UNIFORM_NOT_REGISTERED,
		M3DGL_WARNING_ATTRIBUTE_NOT_FOUND,
		M3DGL_WARNING_ATTRIBUTE_NOT_REGISTERED,
		M3DGL_WARNING_NO_PROGRAMMABLE_PIPELINE,			// model.cpp
		M3DGL_WARNING_VERTEX_COORDS_NOT_IMPLEMENTED,
		M3DGL_WARNING_NORMAL_COORDS_NOT_IMPLEMENTED,
		M3DGL_WARNING_TEXCOORDS_COORDS_NOT_IMPLEMENTED,
		M3DGL_WARNING_BONE_WEIGHTS_NOT_IMPLEMENTED,
		M3DGL_WARNING_BONE_IDS_NOT_IMPLEMENTED,
		M3DGL_WARNING_SKINNING_NOT_IMPLEMENTED,
		M3DGL_WARNING_DIFFERENT_PROGRAM_USED_BUT_COMPATIBLE,	// model.cpp, render-time warnings
		M3DGL_WARNING_INCOMPATIBLE_PROGRAM_USED,
		M3DGL_WARNING_VERTEX_BUFFER_PREPARED_BUT_NOT_USED,
		M3DGL_WARNING_NORMAL_BUFFER_PREPARED_BUT_NOT_USED,
		M3DGL_WARNING_TEXCOORD_BUFFER_PREPARED_BUT_NOT_USED,
		M3DGL_WARNING_TANGENT_BUFFER_PREPARED_BUT_NOT_USED,
		M3DGL_WARNING_BITANGENT_BUFFER_PREPARED_BUT_NOT_USED,
		M3DGL_WARNING_COLOR_BUFFER_PREPARED_BUT_NOT_USED,
		M3DGL_WARNING_BONE_ID_BUFFER_PREPARED_BUT_NOT_USED,
		M3DGL_WARNING_BONE_WEIGHT_BUFFER_PREPARED_BUT_NOT_USED,
		M3DGL_WARNING_VERTEX_BUFFER_NOT_LOADED_BUT_REQUESTED,
		M3DGL_WARNING_NORMAL_BUFFER_NOT_LOADED_BUT_REQUESTED,
		M3DGL_WARNING_TEXCOORD_BUFFER_NOT_LOADED_BUT_REQUESTED,
		M3DGL_WARNING_TANGENT_BUFFER_NOT_LOADED_BUT_REQUESTED,
		M3DGL_WARNING_BITANGENT_BUFFER_NOT_LOADED_BUT_REQUESTED,
		M3DGL_WARNING_COLOR_BUFFER_NOT_LOADED_BUT_REQUESTED,
		M3DGL_WARNING_BONE_ID_BUFFER_NOT_LOADED_BUT_REQUESTED,
		M3DGL_WARNING_BONE_WEIGHT_BUFFER_NOT_LOADED_BUT_REQUESTED,
		M3DGL_WARNING_NON_TRIANGULAR_MESH,				// mesh.cpp
		M3DGL_WARNING_NO_VERTICES,
		M3DGL_WARNING_COMPATIBLE_TEXTURE_COORDS_MISSING,
		M3DGL_WARNING_MAX_BONES_EXCEEDED,
		M3DGL_WARNING_VERTEX_BUFFER_MISSING,
		M3DGL_WARNING_NORMAL_BUFFER_MISSING,
		M3DGL_WARNING_TEXCOORD_BUFFER_MISSING,
		M3DGL_WARNING_TANGENT_BUFFER_MISSING,
		M3DGL_WARNING_BITANGENT_BUFFER_MISSING,
		M3DGL_WARNING_COLOR_BUFFER_MISSING,
		M3DGL_WARNING_BONE_ID_BUFFER_MISSING,
		M3DGL_WARNING_BONE_WEIGHT_BUFFER_MISSING,
		M3DGL_WARNING_CANNOT_LOAD,					// bitmap.cpp
		M3DGL_WARNING_CANNOT_LOAD_FROM_EMBED_FILE,
		M3DGL_WARNING_EMBED_FILE_UNKNOWN_FORMAT,

		// Errors
		M3DGL_ERROR_GENERIC = 500,
		M3DGL_ERROR_TYPE_MISMATCH,
		M3DGL_ERROR_WRONG_STD_UNIFORM_ID,
		M3DGL_ERROR_ATTRIBUTE_NOT_FOUND,				// VAO.cpp
		M3DGL_ERROR_AI,									// model.cpp
		M3DGL_ERROR_COMPILATION,						// shader.cpp
		M3DGL_ERROR_LINKING,
		M3DGL_ERROR_WRONG_SHADER,
		M3DGL_ERROR_NO_SOURCE_CODE,
		M3DGL_ERROR_CREATION,
		M3DGL_ERROR_UNKNOWN_COMPILATION_ERROR,
		M3DGL_ERROR_SHADER_NOT_CREATED,
		M3DGL_ERROR_PROGRAM_NOT_CREATED,
		M3DGL_ERROR_UNKNOWN_LINKING_ERROR,

		M3DGL_INTERNAL_ERROR
	};

	class MY3DGL_API C3dglLogger
	{
	protected:
		C3dglLogger();
		static bool _log(unsigned nCode, std::string name, std::string message);

		static C3dglLogger& getInstance();
		std::string& operator[](const unsigned i);

		static unsigned c_options;
	public:
		// Logging options. Default is C3dglLogger::LOGGER_COLLAPSE_MESSAGES. Use any combination with an '|' operator
		enum { LOGGER_COLLAPSE_MESSAGES = 1, LOGGER_USE_MESH_NAMES = 2, LOGGER_SHOW_ASSIMP_MESSAGES = 4, LOGGER_SHOW_ASSIMP_DEBUG_MESSAGES = 8, LOGGER_SHOW_ASSIMP_VERBOSE_MESSAGES = 12 };
		static void setOptions(unsigned options) { c_options = options;  }
		static unsigned getOptions() { return c_options;  }

		// General purpose log function.
		// Example of use: C3dglLogger::log("Vendor: {}", (const char *)glGetString(GL_VENDOR));
		template<typename ... Args>
		static inline void log(std::string format, Args... args)
		{
			return log(std::vformat(format, std::make_format_args(args ...)));
		}

		// same as above
		static void log(std::string);

		// Standard message log function.
		// Expects the message code (see top of this file for available codes) and the initiator object name.
		template<typename ... Args>
		static inline bool log(unsigned nCode, std::string name, Args... args)
		{
			return _log(nCode, name, std::vformat(getInstance()[nCode], std::make_format_args(args ...)));
		}
	};
}

#endif