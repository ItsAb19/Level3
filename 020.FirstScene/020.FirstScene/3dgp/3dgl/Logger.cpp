/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Version 3.0 - June 2022
Copyright (C) 2013-22 by Jarek Francik, Kingston University, London, UK
*********************************************************************************/

#include "pch.h"

#include <3dgl/Logger.h>

#include <iostream>
#include <map>
#include <set>

#include <GL/glut.h>

using namespace _3dgl;

unsigned C3dglLogger::c_options = LOGGER_COLLAPSE_MESSAGES;

C3dglLogger::C3dglLogger()
{
	operator[](M3DGL_SUCCESS) = "{}";
	operator[](M3DGL_SUCCESS_IMPORTING_FILE) = "is importing file: {}.";
	operator[](M3DGL_SUCCESS_CREATED) = "created successfully.";
	operator[](M3DGL_SUCCESS_SRC_CODE_LOADED) = "source code loaded.";
	operator[](M3DGL_SUCCESS_COMPILED) = "compiled successfully.";
	operator[](M3DGL_SUCCESS_LINKED) = "linked successfully.";
	operator[](M3DGL_SUCCESS_ATTACHED) = "has successfully attached a {}.";
	operator[](M3DGL_SUCCESS_ATTRIB_FOUND) = "attribute location found: {} = {}.";
	operator[](M3DGL_SUCCESS_UNIFORM_FOUND) = "uniform location found: {} = {}.";
	operator[](M3DGL_SUCCESS_VERIFICATION) = "verification result: {}.";
	operator[](M3DGL_SUCCESS_LOADED) = "loaded from: {}.";
	operator[](M3DGL_SUCCESS_LOADED_FROM_EMBED_FILE) = "loaded from embedded file: {}.";

	operator[](M3DGL_WARNING_GENERIC) = "{}";
	operator[](M3DGL_WARNING_UNIFORM_NOT_FOUND) = "uniform location not found: {}.";
	operator[](M3DGL_WARNING_UNIFORM_NOT_REGISTERED) = "unregistered uniform used: {}.";
	operator[](M3DGL_WARNING_ATTRIBUTE_NOT_FOUND) = "attribute location not found: {}.";
	operator[](M3DGL_WARNING_ATTRIBUTE_NOT_REGISTERED) = "unregistered attribute used: {}.";
	operator[](M3DGL_WARNING_NO_PROGRAMMABLE_PIPELINE) = "failed to detect a programmable pipeline. Are you trying to load a model before initialisaing a shader program?";
	operator[](M3DGL_WARNING_VERTEX_COORDS_NOT_IMPLEMENTED) = "requires vertex coordinates but vertex coordinate buffer is not implemented in the current shader program. Consider another shader program.";
	operator[](M3DGL_WARNING_NORMAL_COORDS_NOT_IMPLEMENTED) = "requires normal coordinates but normal buffer is not implemented in the current shader program. Consider another shader program.";
	operator[](M3DGL_WARNING_TEXCOORDS_COORDS_NOT_IMPLEMENTED) = "requires texture implementation but it is not implemented in the current shader program. Consider another shader program.";
	operator[](M3DGL_WARNING_BONE_WEIGHTS_NOT_IMPLEMENTED) = "implements bone ids but bone weights are not implemented in the current shader program.";
	operator[](M3DGL_WARNING_BONE_IDS_NOT_IMPLEMENTED) = "implements bone weights but bone ids are not implemented in the current shader program.";
	operator[](M3DGL_WARNING_SKINNING_NOT_IMPLEMENTED) = "comes with animations but skinning is not implemented.";
	operator[](M3DGL_WARNING_DIFFERENT_PROGRAM_USED_BUT_COMPATIBLE) = "is rendered by a different shader program than the one registered at load time but both appear to be compatible.";
	operator[](M3DGL_WARNING_INCOMPATIBLE_PROGRAM_USED) = "is rendered by a different shader program than the one registered at load time. Check further warnings for details.";
	operator[](M3DGL_WARNING_VERTEX_BUFFER_PREPARED_BUT_NOT_USED) = "has prepared a vertex buffer at load time but it doesn't appear to be used at render time.";
	operator[](M3DGL_WARNING_NORMAL_BUFFER_PREPARED_BUT_NOT_USED) = "has prepared a normal buffer at load time but it doesn't appear to be used at render time.";
	operator[](M3DGL_WARNING_TEXCOORD_BUFFER_PREPARED_BUT_NOT_USED) = "has prepared a texture coordinate buffer at load time but it doesn't appear to be used at render time.";
	operator[](M3DGL_WARNING_TANGENT_BUFFER_PREPARED_BUT_NOT_USED) = "has prepared a tangent buffer at load time but it doesn't appear to be used at render time.";
	operator[](M3DGL_WARNING_BITANGENT_BUFFER_PREPARED_BUT_NOT_USED) = "has prepared a bitangent buffer at load time but it doesn't appear to be used at render time.";
	operator[](M3DGL_WARNING_COLOR_BUFFER_PREPARED_BUT_NOT_USED) = "has prepared a color buffer at load time but it doesn't appear to be used at render time.";
	operator[](M3DGL_WARNING_BONE_ID_BUFFER_PREPARED_BUT_NOT_USED) = "has prepared a bone ID buffer at load time but it doesn't appear to be used at render time.";
	operator[](M3DGL_WARNING_BONE_WEIGHT_BUFFER_PREPARED_BUT_NOT_USED) = "has prepared a bone weight buffer at load time but it doesn't appear to be used at render time.";
	operator[](M3DGL_WARNING_VERTEX_BUFFER_NOT_LOADED_BUT_REQUESTED) = "was not requested to load a vertex buffer but it appears to be used by the current shader program.";
	operator[](M3DGL_WARNING_NORMAL_BUFFER_NOT_LOADED_BUT_REQUESTED) = "was not requested to load a normal buffer but it appears to be used by the current shader program.";
	operator[](M3DGL_WARNING_TEXCOORD_BUFFER_NOT_LOADED_BUT_REQUESTED) = "was not requested to load a texture coordinate buffer but it appears to be used by the current shader program.";
	operator[](M3DGL_WARNING_TANGENT_BUFFER_NOT_LOADED_BUT_REQUESTED) = "was not requested to load a tangent buffer but it appears to be used by the current shader program.";
	operator[](M3DGL_WARNING_BITANGENT_BUFFER_NOT_LOADED_BUT_REQUESTED) = "was not requested to load a bitangent buffer but it appears to be used by the current shader program.";
	operator[](M3DGL_WARNING_COLOR_BUFFER_NOT_LOADED_BUT_REQUESTED) = "was not requested to load a color buffer but it appears to be used by the current shader program.";
	operator[](M3DGL_WARNING_BONE_ID_BUFFER_NOT_LOADED_BUT_REQUESTED) = "was not requested to load a bone ID buffer but it appears to be used by the current shader program.";
	operator[](M3DGL_WARNING_BONE_WEIGHT_BUFFER_NOT_LOADED_BUT_REQUESTED) = "was not requested to load a bone weight buffer but it appears to be used by the current shader program.";
	operator[](M3DGL_WARNING_NON_TRIANGULAR_MESH) = "is loading non-triangular mesh. Only triangular meshes are supported.";
	operator[](M3DGL_WARNING_NO_VERTICES) = "has no vertices.";
	operator[](M3DGL_WARNING_COMPATIBLE_TEXTURE_COORDS_MISSING) = "is using {} texture coordinates. Only 2 UV coordinates are supported; any excessive coordinates will be ignored.";
	operator[](M3DGL_WARNING_MAX_BONES_EXCEEDED) = "has exceeded the maximum number of bones.";
	operator[](M3DGL_WARNING_VERTEX_BUFFER_MISSING) = "is missing vertex buffer information.";
	operator[](M3DGL_WARNING_NORMAL_BUFFER_MISSING) = "is missing normal buffer information.";
	operator[](M3DGL_WARNING_TEXCOORD_BUFFER_MISSING) = "is missing texture coordinate buffer information.";
	operator[](M3DGL_WARNING_TANGENT_BUFFER_MISSING) = "is missing tangent buffer information.";
	operator[](M3DGL_WARNING_BITANGENT_BUFFER_MISSING) = "is missing bitangent buffer information.";
	operator[](M3DGL_WARNING_COLOR_BUFFER_MISSING) = "is missing color buffer information.";
	operator[](M3DGL_WARNING_BONE_ID_BUFFER_MISSING) = "is missing bone ID information.";
	operator[](M3DGL_WARNING_BONE_WEIGHT_BUFFER_MISSING) = "is missing bone weight information.";
	operator[](M3DGL_WARNING_CANNOT_LOAD) = "couldn't load from: {}.";
	operator[](M3DGL_WARNING_CANNOT_LOAD_FROM_EMBED_FILE) = "couldn't load from embedded file: {}.";
	operator[](M3DGL_WARNING_EMBED_FILE_UNKNOWN_FORMAT) = "encountered unknown file format {} in embedded file: {}.";

	operator[](M3DGL_ERROR_GENERIC) = "{}";
	operator[](M3DGL_ERROR_TYPE_MISMATCH) = "type mismatch in uniform: {}: sending value of {} but {} was expected.";
	operator[](M3DGL_ERROR_WRONG_STD_UNIFORM_ID) = "standard uniform index out of scope. Should be less then {}.";
	operator[](M3DGL_ERROR_ATTRIBUTE_NOT_FOUND) = "buffer creation failed. Attribute location does not exist.";
	operator[](M3DGL_ERROR_AI) = "internal ASSIMP error: {}";
	operator[](M3DGL_ERROR_COMPILATION) = "compilation error: {}";
	operator[](M3DGL_ERROR_LINKING) = "linking error: {}";
	operator[](M3DGL_ERROR_WRONG_SHADER) = "creation error. Wrong type of shader.";
	operator[](M3DGL_ERROR_NO_SOURCE_CODE) = "creation error. Source code not provided.";
	operator[](M3DGL_ERROR_CREATION) = "creation error. Reason unknown.";
	operator[](M3DGL_ERROR_UNKNOWN_COMPILATION_ERROR) = "unknown compilation error.";
	operator[](M3DGL_ERROR_SHADER_NOT_CREATED) = "cannot attach shader: shader not created.";
	operator[](M3DGL_ERROR_PROGRAM_NOT_CREATED) = "shader program not created.";
	operator[](M3DGL_ERROR_UNKNOWN_LINKING_ERROR) = "unknown linking error";

	operator[](M3DGL_INTERNAL_ERROR) = "INTERNAL ERROR";
}

C3dglLogger& C3dglLogger::getInstance()
{
	static C3dglLogger inst;
	return inst;
}

std::string& C3dglLogger::operator[](const unsigned i)
{
	static std::map<unsigned, std::string> msgs;
	return msgs[i];
}

bool C3dglLogger::_log(unsigned nCode, std::string name, std::string message)
{
	int nSeverity = 0;
	if (nCode >= M3DGL_WARNING_GENERIC)
		nSeverity++;
	if (nCode >= M3DGL_ERROR_GENERIC)
		nSeverity++;

	std::string msg;
	switch (nSeverity)
	{
	case 0: msg = std::format("{} {}", name, message); break;
	case 1: msg = std::format("Warning {}: {} {}", nCode, name, message); break;
	case 2: msg = std::format("*** Error {}: {} {}", nCode, name, message); break;
	}

	static std::set<std::string> errlookup;	// used to prevent displaying the same message twice
	bool bFirstTimeSeen = false;					// set to true if first time seen (should not be collapsed)
	if (errlookup.find(msg) == errlookup.end())
	{
		bFirstTimeSeen = true;
		errlookup.insert(msg);
	}
	else
		bFirstTimeSeen = false;

	if (bFirstTimeSeen || (getOptions() & LOGGER_COLLAPSE_MESSAGES) == 0)
	{
		CONSOLE_SCREEN_BUFFER_INFO Info;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &Info);
		switch (nSeverity)
		{
		case 1: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN); break;
		case 2: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY); break;
		}
		log(msg);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), Info.wAttributes);
	}
	return (nSeverity <= 1);
}

void C3dglLogger::log(std::string msg)
{
	std::cout << msg << std::endl;
}
