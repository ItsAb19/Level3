/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Version 3.0 - June 2022
Copyright (C) 2013-22 by Jarek Francik, Kingston University, London, UK

Implementation of a simple 3D model class
Partially based on Luke Benstead GLSLProgram class.
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
#ifndef __3dglShader_h_
#define __3dglShader_h_

#include "Object.h"
#include "CommonDef.h"
#include <map>

#include "../glm/mat4x4.hpp"


//////////////////////////////////////////////////////////
// NOTE: To keep compatibility with older versions
// function names in this file start with a capital letter
// unlike the rest of 3DGL
//////////////////////////////////////////////////////////

namespace _3dgl
{
	class MY3DGL_API C3dglShader : public C3dglObject
	{
		GLenum m_type;
		GLuint m_id;
		std::string m_source;
		std::string m_fname;
	public:
		C3dglShader() : C3dglObject()		{ m_type = 0; m_id = 0; }

		bool create(GLenum type);
		bool load(std::string source);
		bool loadFromFile(std::string fname);
		bool compile() const;

		GLenum getType() const			{ return m_type; }
		GLuint getId()  const			{ return m_id; }
		std::string getSource()  const	{ return m_source; }
		std::string getFName()  const	{ return m_fname; }
		std::string getName() const;	// "Vertex Shader", "Fragment Shader" etc
	};

	class MY3DGL_API C3dglProgram : public C3dglObject
	{
	private:
		static C3dglProgram *c_pCurrentProgram;

		struct MY3DGL_API UNIFORM
		{
			GLint location;		// uniform location
			GLenum datatype;	// index to an internal data structure (c_uniTypes)
		};

		struct MY3DGL_API UNIFORM_VALUE
		{
			GLenum datatype;	// uniform type (GL_FLOAT, GL_INT, GL_UNSIGNED_INT, GL_FLOAT_VEC2 and so on)
			union				// unform value (ONE of the values listed below)
			{
				GLfloat val_FLOAT;
				GLint val_INT;
				GLuint val_UNSIGNED_INT;
				glm::vec2 val_FLOAT_VEC2;
				glm::vec3 val_FLOAT_VEC3;
				glm::vec4 val_FLOAT_VEC4;
				glm::ivec2 val_INT_VEC2;
				glm::ivec3 val_INT_VEC3;
				glm::ivec4 val_INT_VEC4;
				glm::uvec2 val_UNSIGNED_INT_VEC2;
				glm::uvec3 val_UNSIGNED_INT_VEC3;
				glm::uvec4 val_UNSIGNED_INT_VEC4;
				glm::mat2 val_FLOAT_MAT2;
				glm::mat3 val_FLOAT_MAT3;
				glm::mat4 val_FLOAT_MAT4;
			};
		};

		GLuint m_id;		// Program id

		// Maps of Shader Objects
#pragma warning(push)
#pragma warning(disable: 4251)
		mutable std::map<std::string, GLint> m_attribs;			// map of attributes: name => attribute locaion 
		mutable std::map<std::string, UNIFORM> m_uniforms;		// map of uniforms: name => uniform location
		mutable std::map<GLint, UNIFORM_VALUE> m_values;		// map of uniform value: location => value
#pragma warning(pop)

		size_t m_stdAttrNum = ATTR_COUNT;				// number of standard attributes (8)
		GLint m_stdAttr[ATTR_COUNT];					// array of standard attribute locations (see enum ATTRIB_STD in CommonDef.h)
		GLint m_stdUni[UNI_COUNT];						// array of standard uniform locations (see enum UNI_STD in CommonDef.h)

	public:
		C3dglProgram();

		bool create();
		bool attach(C3dglShader &shader);
		bool link(std::string std_attrib_names = "", std::string std_uni_names = "");
		bool use(bool bValidate = false);

		GLuint getId() const			{ return m_id; }
		bool isUsed() const				{ return c_pCurrentProgram == this; }

		static C3dglProgram *getCurrentProgram()			{ return c_pCurrentProgram; }

		// numerical locations for attributes
		GLint getAttribLocation(std::string idUniform) const;
		GLint getAttribLocation(ATTRIB_STD attr) const		{ return m_stdAttr[attr]; }

		// shader "signature" - array of all standard attribute locations which defines the shader program functionality
		const GLint* getShaderSignature() const				{ return m_stdAttr; }
		size_t getShaderSignatureLength() const				{ return m_stdAttrNum; }

		// numerical locations and types for attribute and uniform names
		GLint getUniformLocation(std::string idUniform) const;
		GLint getUniformLocation(std::string idUniform, size_t index) const;
		GLint getUniformLocation(UNI_STD uniId) const;

		void getUniformLocationAndType(std::string idUniform, GLint& location, GLenum& type) const;
		void getUniformLocationAndType(std::string idUniform, size_t index, GLint& location, GLenum& type) const;

		// Send Uniform functions

		// Sending uniforms using location names
		// Usage: sendUniform("uniform-name", value);
		// single values
		bool sendUniform(std::string name, GLfloat);
		bool sendUniform(std::string name, double d) { return sendUniform(name, static_cast<float>(d)); }
		bool sendUniform(std::string name, GLint);
		bool sendUniform(std::string name, GLuint);
		// glm vectors: vec, bvec, ivec, uvec x 2, 3, 4
		bool sendUniform(std::string name, glm::vec2);
		bool sendUniform(std::string name, glm::vec3);
		bool sendUniform(std::string name, glm::vec4);
		bool sendUniform(std::string name, glm::ivec2);
		bool sendUniform(std::string name, glm::ivec3);
		bool sendUniform(std::string name, glm::ivec4);
		bool sendUniform(std::string name, glm::uvec2);
		bool sendUniform(std::string name, glm::uvec3);
		bool sendUniform(std::string name, glm::uvec4);
		// glm matrix
		bool sendUniform(std::string name, glm::mat2);
		bool sendUniform(std::string name, glm::mat3);
		bool sendUniform(std::string name, glm::mat4);

		// Sending uniforms using location ids
		// Usage: sendUniform(location-code, value);
		// single values
		void sendUniform(GLint location, GLfloat);
		void sendUniform(GLint location, double d) { return sendUniform(location, static_cast<float>(d)); }
		void sendUniform(GLint location, GLint);
		void sendUniform(GLint location, GLuint);
		// glm vectors: vec, bvec, ivec, uvec x 2, 3, 4
		void sendUniform(GLint location, glm::vec2);
		void sendUniform(GLint location, glm::vec3);
		void sendUniform(GLint location, glm::vec4);
		void sendUniform(GLint location, glm::ivec2);
		void sendUniform(GLint location, glm::ivec3);
		void sendUniform(GLint location, glm::ivec4);
		void sendUniform(GLint location, glm::uvec2);
		void sendUniform(GLint location, glm::uvec3);
		void sendUniform(GLint location, glm::uvec4);
		// glm matrix
		void sendUniform(GLint location, glm::mat2);
		void sendUniform(GLint location, glm::mat3);
		void sendUniform(GLint location, glm::mat4);

		// Sending arrays using location names
		// Usage: sendUniform("location-name", array address, array count);
		// Note there is no option to send arrays of doubles
		bool sendUniform(std::string name, GLfloat*, size_t count);
		bool sendUniform(std::string name, GLint*, size_t count);
		bool sendUniform(std::string name, GLuint*, size_t count);
		// glm vectors: vec, bvec, ivec, uvec x 2, 3, 4
		bool sendUniform(std::string name, glm::vec2*, size_t count);
		bool sendUniform(std::string name, glm::vec3*, size_t count);
		bool sendUniform(std::string name, glm::vec4*, size_t count);
		bool sendUniform(std::string name, glm::ivec2*, size_t count);
		bool sendUniform(std::string name, glm::ivec3*, size_t count);
		bool sendUniform(std::string name, glm::ivec4*, size_t count);
		bool sendUniform(std::string name, glm::uvec2*, size_t count);
		bool sendUniform(std::string name, glm::uvec3*, size_t count);
		bool sendUniform(std::string name, glm::uvec4*, size_t count);
		// glm matrix
		bool sendUniform(std::string name, glm::mat2*, size_t count);
		bool sendUniform(std::string name, glm::mat3*, size_t count);
		bool sendUniform(std::string name, glm::mat4*, size_t count);

		// Sending arrays using location ids
		// Usage: sendUniform(location-code, array address, array count);
		// Note there is no option to send arrays of doubles
		void sendUniform(GLint location, GLfloat*, size_t count);
		void sendUniform(GLint location, GLint*, size_t count);
		void sendUniform(GLint location, GLuint*, size_t count);
		// glm vectors: vec, bvec, ivec, uvec x 2, 3, 4
		void sendUniform(GLint location, glm::vec2*, size_t count);
		void sendUniform(GLint location, glm::vec3*, size_t count);
		void sendUniform(GLint location, glm::vec4*, size_t count);
		void sendUniform(GLint location, glm::ivec2*, size_t count);
		void sendUniform(GLint location, glm::ivec3*, size_t count);
		void sendUniform(GLint location, glm::ivec4*, size_t count);
		void sendUniform(GLint location, glm::uvec2*, size_t count);
		void sendUniform(GLint location, glm::uvec3*, size_t count);
		void sendUniform(GLint location, glm::uvec4*, size_t count);
		// glm matrix
		void sendUniform(GLint location, glm::mat2*, size_t count);
		void sendUniform(GLint location, glm::mat3*, size_t count);
		void sendUniform(GLint location, glm::mat4*, size_t count);

		// Sending array items using location names and index
		// Usage: sendUniform("location-name", index, value);
		// single values
		bool sendUniform(std::string name, size_t index, GLfloat);
		bool sendUniform(std::string name, size_t index, double d) { return sendUniform(name, index, static_cast<float>(d)); }
		bool sendUniform(std::string name, size_t index, GLint);
		bool sendUniform(std::string name, size_t index, GLuint);
		// glm vectors: vec, bvec, ivec, uvec x 2, 3, 4
		bool sendUniform(std::string name, size_t index, glm::vec2);
		bool sendUniform(std::string name, size_t index, glm::vec3);
		bool sendUniform(std::string name, size_t index, glm::vec4);
		bool sendUniform(std::string name, size_t index, glm::ivec2);
		bool sendUniform(std::string name, size_t index, glm::ivec3);
		bool sendUniform(std::string name, size_t index, glm::ivec4);
		bool sendUniform(std::string name, size_t index, glm::uvec2);
		bool sendUniform(std::string name, size_t index, glm::uvec3);
		bool sendUniform(std::string name, size_t index, glm::uvec4);
		// glm matrix
		bool sendUniform(std::string name, size_t index, glm::mat2);
		bool sendUniform(std::string name, size_t index, glm::mat3);
		bool sendUniform(std::string name, size_t index, glm::mat4);

		// send a standard uniform using one of the UNI_STD values
		bool sendUniform(enum UNI_STD stdloc, GLfloat v);
		bool sendUniform(enum UNI_STD stdloc, double d) { return sendUniform(stdloc, static_cast<float>(d)); }
		bool sendUniform(enum UNI_STD stdloc, glm::vec2 v);
		bool sendUniform(enum UNI_STD stdloc, glm::vec3 v);
		bool sendUniform(enum UNI_STD stdloc, glm::vec4 v);
		bool sendUniform(enum UNI_STD stdloc, glm::mat2 v);
		bool sendUniform(enum UNI_STD stdloc, glm::mat3 v);
		bool sendUniform(enum UNI_STD stdloc, glm::mat4 v);

		// Retrieveing uniforms
		// NOTE: Uniform values cannot be read from GPU
		// Instead they are intercepted internally when send with sendUniform functions
		// Uniforms set in any other way, e.g. from within the GLSL code or by direct calls to glUniform* functions will not be properly retrieved

		// single values
		bool retrieveUniform(std::string name, GLfloat&);
		bool retrieveUniform(std::string name, GLint&);
		bool retrieveUniform(std::string name, GLuint&);
		// glm vectors: vec, bvec, ivec, uvec x 2, 3, 4
		bool retrieveUniform(std::string name, glm::vec2&);
		bool retrieveUniform(std::string name, glm::vec3&);
		bool retrieveUniform(std::string name, glm::vec4&);
		bool retrieveUniform(std::string name, glm::ivec2&);
		bool retrieveUniform(std::string name, glm::ivec3&);
		bool retrieveUniform(std::string name, glm::ivec4&);
		bool retrieveUniform(std::string name, glm::uvec2&);
		bool retrieveUniform(std::string name, glm::uvec3&);
		bool retrieveUniform(std::string name, glm::uvec4&);
		// glm matrix
		bool retrieveUniform(std::string name, glm::mat2&);
		bool retrieveUniform(std::string name, glm::mat3&);
		bool retrieveUniform(std::string name, glm::mat4&);

		// Sending uniforms using location ids
		// Usage: sendUniform(location-code, value);
		// single values
		bool retrieveUniform(GLint location, GLfloat&);
		bool retrieveUniform(GLint location, GLint&);
		bool retrieveUniform(GLint location, GLuint&);
		// glm vectors: vec, bvec, ivec, uvec x 2, 3, 4
		bool retrieveUniform(GLint location, glm::vec2&);
		bool retrieveUniform(GLint location, glm::vec3&);
		bool retrieveUniform(GLint location, glm::vec4&);
		bool retrieveUniform(GLint location, glm::ivec2&);
		bool retrieveUniform(GLint location, glm::ivec3&);
		bool retrieveUniform(GLint location, glm::ivec4&);
		bool retrieveUniform(GLint location, glm::uvec2&);
		bool retrieveUniform(GLint location, glm::uvec3&);
		bool retrieveUniform(GLint location, glm::uvec4&);
		// glm matrix
		bool retrieveUniform(GLint location, glm::mat2&);
		bool retrieveUniform(GLint location, glm::mat3&);
		bool retrieveUniform(GLint location, glm::mat4&);

		// Sending array items using location names and index
		// Usage: sendUniform("location-name", index, value);
		// single values
		bool retrieveUniform(std::string name, size_t index, GLfloat&);
		bool retrieveUniform(std::string name, size_t index, GLint&);
		bool retrieveUniform(std::string name, size_t index, GLuint&);
		// glm vectors: vec, bvec, ivec, uvec x 2, 3, 4
		bool retrieveUniform(std::string name, size_t index, glm::vec2&);
		bool retrieveUniform(std::string name, size_t index, glm::vec3&);
		bool retrieveUniform(std::string name, size_t index, glm::vec4&);
		bool retrieveUniform(std::string name, size_t index, glm::ivec2&);
		bool retrieveUniform(std::string name, size_t index, glm::ivec3&);
		bool retrieveUniform(std::string name, size_t index, glm::ivec4&);
		bool retrieveUniform(std::string name, size_t index, glm::uvec2&);
		bool retrieveUniform(std::string name, size_t index, glm::uvec3&);
		bool retrieveUniform(std::string name, size_t index, glm::uvec4&);
		// glm matrix
		bool retrieveUniform(std::string name, size_t index, glm::mat2&);
		bool retrieveUniform(std::string name, size_t index, glm::mat3&);
		bool retrieveUniform(std::string name, size_t index, glm::mat4&);

		// send a standard uniform using one of the UNI_STD values
		bool retrieveUniform(enum UNI_STD stdloc, GLfloat&);
		bool retrieveUniform(enum UNI_STD stdloc, glm::vec2&);
		bool retrieveUniform(enum UNI_STD stdloc, glm::vec3&);
		bool retrieveUniform(enum UNI_STD stdloc, glm::vec4&);
		bool retrieveUniform(enum UNI_STD stdloc, glm::mat2&);
		bool retrieveUniform(enum UNI_STD stdloc, glm::mat3&);
		bool retrieveUniform(enum UNI_STD stdloc, glm::mat4&);

		std::string getName() const	{ return "GLSL Program"; }
	
	private:
		// private implementation helpers
		template<class T> bool _sendUniform(std::string name, T*, size_t count, GLenum type);
		template<class T> bool _sendUniform(enum UNI_STD stdloc, T v);
	};

}; // namespace _3dgl

#endif // __3dglShader_h_
