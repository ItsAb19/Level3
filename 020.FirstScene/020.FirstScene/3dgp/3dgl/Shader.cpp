/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Version 3.0 - June 2022
Copyright (C) 2013-22 by Jarek Francik, Kingston University, London, UK
*********************************************************************************/
#include "pch.h"
#include <3dgl/Shader.h>

#include <fstream>
#include <vector>

using namespace _3dgl;


/////////////////////////////////////////////////////////////////////////////////////////////////
// Uniform variable types

static struct {
	GLenum glType;
	GLenum targetType;
	std::string name;
} c_uniTypes[] =
{
	{ GLenum(0), GLenum(0), "???" },
	
	{ GL_FLOAT, GL_FLOAT, "GL_FLOAT" },
	{ GL_FLOAT_VEC2, GL_FLOAT_VEC2, "GL_FLOAT_VEC2" },
	{ GL_FLOAT_VEC3, GL_FLOAT_VEC3, "GL_FLOAT_VEC3" },
	{ GL_FLOAT_VEC4, GL_FLOAT_VEC4, "GL_FLOAT_VEC4" },
	{ GL_INT, GL_INT, "GL_INT" },
	{ GL_INT_VEC2, GL_INT_VEC2, "GL_INT_VEC2" },
	{ GL_INT_VEC3, GL_INT_VEC3, "GL_INT_VEC3" },
	{ GL_INT_VEC4, GL_INT_VEC4, "GL_INT_VEC4" },
	{ GL_UNSIGNED_INT, GL_UNSIGNED_INT, "GL_UNSIGNED_INT" },
	{ GL_UNSIGNED_INT_VEC2, GL_UNSIGNED_INT_VEC2, "GL_UNSIGNED_INT_VEC2" },
	{ GL_UNSIGNED_INT_VEC3, GL_UNSIGNED_INT_VEC3, "GL_UNSIGNED_INT_VEC3" },
	{ GL_UNSIGNED_INT_VEC4, GL_UNSIGNED_INT_VEC4, "GL_UNSIGNED_INT_VEC4" },
	{ GL_BOOL, GL_BOOL, "GL_BOOL" },
	{ GL_BOOL_VEC2, GL_BOOL_VEC2, "GL_BOOL_VEC2" },
	{ GL_BOOL_VEC3, GL_BOOL_VEC3, "GL_BOOL_VEC3" },
	{ GL_BOOL_VEC4, GL_BOOL_VEC4, "GL_BOOL_VEC4" },
	
	{ GL_FLOAT_MAT2, GL_FLOAT_MAT2, "GL_FLOAT_MAT2" },
	{ GL_FLOAT_MAT3, GL_FLOAT_MAT3, "GL_FLOAT_MAT3" },
	{ GL_FLOAT_MAT4, GL_FLOAT_MAT4, "GL_FLOAT_MAT4" },
	{ GL_FLOAT_MAT2x3, GL_FLOAT_MAT2x3, "GL_FLOAT_MAT2x3" },
	{ GL_FLOAT_MAT2x4, GL_FLOAT_MAT2x4, "GL_FLOAT_MAT2x4" },
	{ GL_FLOAT_MAT3x2, GL_FLOAT_MAT3x2, "GL_FLOAT_MAT3x2" },
	{ GL_FLOAT_MAT3x4, GL_FLOAT_MAT3x4, "GL_FLOAT_MAT3x4" },
	{ GL_FLOAT_MAT4x2, GL_FLOAT_MAT4x2, "GL_FLOAT_MAT4x2" },
	{ GL_FLOAT_MAT4x3, GL_FLOAT_MAT4x3, "GL_FLOAT_MAT4x3" },

	{ GL_SAMPLER_1D, GL_INT, "GL_SAMPLER_1D" },
	{ GL_SAMPLER_2D, GL_INT, "GL_SAMPLER_2D" },
	{ GL_SAMPLER_3D, GL_INT, "GL_SAMPLER_3D" },
	{ GL_SAMPLER_CUBE, GL_INT, "GL_SAMPLER_CUBE" },
	{ GL_SAMPLER_1D_SHADOW, GL_INT, "GL_SAMPLER_1D_SHADOW" },
	{ GL_SAMPLER_2D_SHADOW, GL_INT, "GL_SAMPLER_2D_SHADOW" },
	{ GL_SAMPLER_1D_ARRAY, GL_INT, "GL_SAMPLER_1D_ARRAY" },
	{ GL_SAMPLER_2D_ARRAY, GL_INT, "GL_SAMPLER_2D_ARRAY" },
	{ GL_SAMPLER_1D_ARRAY_SHADOW, GL_INT, "GL_SAMPLER_1D_ARRAY_SHADOW" },
	{ GL_SAMPLER_2D_ARRAY_SHADOW, GL_INT, "GL_SAMPLER_2D_ARRAY_SHADOW" },
	{ GL_SAMPLER_2D_MULTISAMPLE, GL_INT, "GL_SAMPLER_2D_MULTISAMPLE" },
	{ GL_SAMPLER_2D_MULTISAMPLE_ARRAY, GL_INT, "GL_SAMPLER_2D_MULTISAMPLE_ARRAY" },
	{ GL_SAMPLER_CUBE_SHADOW, GL_INT, "GL_SAMPLER_CUBE_SHADOW" },
	{ GL_SAMPLER_BUFFER, GL_INT, "GL_SAMPLER_BUFFER" },
	{ GL_SAMPLER_2D_RECT, GL_INT, "GL_SAMPLER_2D_RECT" },
	{ GL_SAMPLER_2D_RECT_SHADOW, GL_INT, "GL_SAMPLER_2D_RECT_SHADOW" },

	{ GL_INT_SAMPLER_1D, GL_INT, "GL_INT_SAMPLER_1D" },
	{ GL_INT_SAMPLER_2D, GL_INT, "GL_INT_SAMPLER_2D" },
	{ GL_INT_SAMPLER_3D, GL_INT, "GL_INT_SAMPLER_3D" },
	{ GL_INT_SAMPLER_CUBE, GL_INT, "GL_INT_SAMPLER_CUBE" },
	{ GL_INT_SAMPLER_1D_ARRAY, GL_INT, "GL_INT_SAMPLER_1D_ARRAY" },
	{ GL_INT_SAMPLER_2D_ARRAY, GL_INT, "GL_INT_SAMPLER_2D_ARRAY" },
	{ GL_INT_SAMPLER_2D_MULTISAMPLE, GL_INT, "GL_INT_SAMPLER_2D_MULTISAMPLE" },
	{ GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY, GL_INT, "GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY" },
	{ GL_INT_SAMPLER_BUFFER, GL_INT, "GL_INT_SAMPLER_BUFFER" },
	{ GL_INT_SAMPLER_2D_RECT, GL_INT, "GL_INT_SAMPLER_2D_RECT" },

	{ GL_UNSIGNED_INT_SAMPLER_1D, GL_INT, "GL_UNSIGNED_INT_SAMPLER_1D" },
	{ GL_UNSIGNED_INT_SAMPLER_2D, GL_INT, "GL_UNSIGNED_INT_SAMPLER_2D" },
	{ GL_UNSIGNED_INT_SAMPLER_3D, GL_INT, "GL_UNSIGNED_INT_SAMPLER_3D" },
	{ GL_UNSIGNED_INT_SAMPLER_CUBE, GL_INT, "GL_UNSIGNED_INT_SAMPLER_CUBE" },
	{ GL_UNSIGNED_INT_SAMPLER_1D_ARRAY, GL_INT, "GL_UNSIGNED_INT_SAMPLER_1D_ARRAY" },
	{ GL_UNSIGNED_INT_SAMPLER_2D_ARRAY, GL_INT, "GL_UNSIGNED_INT_SAMPLER_2D_ARRAY" },
	{ GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE, GL_INT, "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE" },
	{ GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY, GL_INT, "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY" },
	{ GL_UNSIGNED_INT_SAMPLER_BUFFER, GL_INT, "GL_UNSIGNED_INT_SAMPLER_BUFFER" },
	{ GL_UNSIGNED_INT_SAMPLER_2D_RECT, GL_INT, "GL_UNSIGNED_INT_SAMPLER_2D_RECT" },
};

static std::map<GLenum, size_t> c_mapTypes;				// map of uniform types: uniform name => data type

static void _initMapTypes()
{
	if (c_mapTypes.size() > 0) return;
	size_t i = 0;
	for (auto type : c_uniTypes)
		c_mapTypes[type.glType] = i++;

}

/////////////////////////////////////////////////////////////////////////////////////////////////
// C3dglShader

bool C3dglShader::create(GLenum type)
{
	m_type = type;
	m_id = glCreateShader(m_type);
	if (m_id == 0) return log(M3DGL_ERROR_WRONG_SHADER);
	return log(M3DGL_SUCCESS_CREATED);
}

bool C3dglShader::load(std::string source)
{
	if (m_id == 0) return log(M3DGL_ERROR_WRONG_SHADER);
	m_source = source;
	if (m_source.empty()) return log(M3DGL_ERROR_NO_SOURCE_CODE);
	const GLchar *pSource = static_cast<const GLchar*>(m_source.c_str());
	glShaderSource(m_id, 1, &pSource, NULL);
	return log(M3DGL_SUCCESS_SRC_CODE_LOADED);
}

bool C3dglShader::loadFromFile(std::string fname)
{
	m_fname = fname;
	std::ifstream file(m_fname.c_str());
	std::string source(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));
	return load(source);
}

bool C3dglShader::compile() const
{
	if (m_id == 0) return log(M3DGL_ERROR_WRONG_SHADER);

	// compile
	glCompileShader(m_id);

	// check status
	GLint result = 0;
	glGetShaderiv(m_id, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		// collect info log
		GLint infoLen;
		glGetShaderiv(m_id, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen < 1) return log(M3DGL_ERROR_UNKNOWN_COMPILATION_ERROR);
		std::vector<char> info(infoLen);
		glGetShaderInfoLog(m_id, infoLen, &infoLen, &info[0]);
		return log(M3DGL_ERROR_COMPILATION, std::string(info.begin(), info.end()));
	}
	return log(M3DGL_SUCCESS_COMPILED);
}

std::string C3dglShader::getName() const
{
	switch (m_type)
	{
	case GL_VERTEX_SHADER: return "Vertex Shader";
	case GL_FRAGMENT_SHADER: return "Fragment Shader";
	case GL_COMPUTE_SHADER: return "Compute Shader";
	case GL_TESS_CONTROL_SHADER: return "Tesselation Control Shader";
	case GL_TESS_EVALUATION_SHADER: return "Tesselation Evaluation Shader";
	case GL_GEOMETRY_SHADER: return "Geometry Shader";
	default: return "Shader";
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// C3dglProgram

C3dglProgram *C3dglProgram::c_pCurrentProgram = NULL;

C3dglProgram::C3dglProgram() : C3dglObject()
{
	m_id = 0;
	std::fill(m_stdAttr, m_stdAttr + ATTR_COUNT, -1);
	std::fill(m_stdUni, m_stdUni + UNI_COUNT, -1);
	// init the static (global) map of uniform types
	_initMapTypes();
}

bool C3dglProgram::create()
{
	m_id = glCreateProgram();
	if (m_id == 0) return log(M3DGL_ERROR_CREATION);
	return log(M3DGL_SUCCESS_CREATED);
}

bool C3dglProgram::attach(C3dglShader &shader)
{
	if (m_id == 0) return log(M3DGL_ERROR_PROGRAM_NOT_CREATED);
	if (shader.getId() == 0) return log(M3DGL_ERROR_SHADER_NOT_CREATED);;

	glAttachShader(m_id, shader.getId());
	return log(M3DGL_SUCCESS_ATTACHED, shader.getName());
}

bool C3dglProgram::link(std::string std_attrib_names, std::string std_uni_names)
{
	if (m_id == 0) return log(M3DGL_ERROR_PROGRAM_NOT_CREATED);

	// link
	glLinkProgram(m_id);

	// check status
	GLint result = 0;
	glGetProgramiv(m_id, GL_LINK_STATUS, &result);
	if (!result)
	{
		// collect info log
		GLint infoLen;
		glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen < 1) return log(M3DGL_ERROR_UNKNOWN_LINKING_ERROR);
		std::vector<char> info(infoLen);
		glGetProgramInfoLog(m_id, infoLen, &infoLen, &info[0]);
		return log(M3DGL_ERROR_LINKING, std::string(info.begin(), info.end()));
	}

	// register active variables
	GLint nActive, maxLen;
	glGetProgramiv(getId(), GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen);
	glGetProgramiv(getId(), GL_ACTIVE_UNIFORMS, &nActive);
	GLchar *buf = new GLchar[maxLen];
	for (int i = 0; i < nActive; ++i) 
	{
		GLsizei written;
		GLint size, location;
		GLenum type;
		glGetActiveUniform(getId(), i, maxLen, &written, &size, &type, buf);
		location = glGetUniformLocation(getId(), buf);
		std::string name = buf;
		m_uniforms[name] = { location, c_uniTypes[c_mapTypes[type]].targetType };

		// special entry for arrays...
		size_t nPos = name.find('[');
		if (nPos != std::string::npos)
		{
			std::string nameArray = name.substr(0, nPos);
			location = glGetUniformLocation(getId(), nameArray.c_str());
			m_uniforms[nameArray] = { location, c_uniTypes[c_mapTypes[type]].targetType };
		}
	}
	delete[] buf;

	// register active attributes
	glGetProgramiv(getId(), GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLen);
	glGetProgramiv(getId(), GL_ACTIVE_ATTRIBUTES, &nActive);
	buf = new GLchar[maxLen];
	for (int i = 0; i < nActive; ++i)
	{
		GLsizei written;
		GLint size, location;
		GLenum type;
		glGetActiveAttrib(getId(), i, maxLen, &written, &size, &type, buf);
		location = glGetAttribLocation(getId(), buf);
		std::string name = buf;
		m_attribs[name] = location;
	}
	delete[] buf;

	// Collect Standard Attribute Locations
	std::string STD_ATTRIB_NAMES[] = {
		"a_vertex|a_Vertex|aVertex|avertex|vertex|Vertex",
		"a_normal|a_Normal|aNormal|anormal|normal|Normal",
		"a_texcoord|a_TexCoord|aTexCoord|atexcoord|texcoord|TexCoord",
		"a_tangent|a_Tangent|aTangent|atangent|tangent|Tangent",
		"a_bitangent|a_Bitangent|aBitangent|abitangent|bitangent|Bitangent|a_biTangent|a_BiTangent|aBiTangent|abiTangent|biTangent|BiTangent",
		"a_color|a_Color|aColor|acolor|color|Color",
		"a_boneid|a_Boneid|aBoneid|aboneid|boneid|Boneid|a_boneId|a_BoneId|aBoneId|aboneId|boneId|BoneId|"
		"a_boneids|a_Boneids|aBoneids|aboneids|boneids|Boneids|a_boneIds|a_BoneIds|aBoneIds|aboneIds|boneIds|BoneIds",
		"a_boneweight|a_Boneweight|aBoneweight|aboneweight|boneweight|Boneweight|a_boneWeight|a_BoneWeight|aBoneWeight|aboneWeight|boneWeight|BoneWeight|a_weight|aweight|weight|a_Weight|aWeight|Weight|"
		"a_boneweights|a_Boneweights|aBoneweights|aboneweights|boneweights|Boneweights|a_boneWeights|a_BoneWeights|aBoneWeights|aboneWeights|boneWeights|BoneWeights|a_weights|aweights|weights|a_Weights|aWeights|Weights",
	};
	size_t astart = 0, aend = 0;
	std_attrib_names += ";";
	for (size_t attr = 0; attr < ATTR_COUNT; attr++)
	{
		std::string str = "";
		aend = std_attrib_names.find(";", astart);
		if (aend != std::string::npos)
		{
			str = std_attrib_names.substr(astart, aend - astart);
			astart = aend + 1;
		}
		if (str.empty()) str = STD_ATTRIB_NAMES[attr];
		str += "|";

		size_t nstart = 0, nend = 0;
		while ((nend = str.find("|", nstart)) != std::string::npos)
		{
			std::string name = str.substr(nstart, nend - nstart);
			nstart = nend + 1;
			if (name.empty()) continue;
			
			m_stdAttr[attr] = glGetAttribLocation(m_id, name.c_str());
			if (m_stdAttr[attr] != -1)
			{
				log(M3DGL_SUCCESS_ATTRIB_FOUND, name, m_stdAttr[attr]);
				break;
			}
		}
	}
	
	// Collect Standard Uniform Locations
	std::string STD_UNI_NAMES[] = {
		"modelview_matrix|modelView_matrix|ModelView_matrix|Modelview_matrix|modelview_Matrix|modelView_Matrix|ModelView_Matrix|Modelview_Matrix|"
		"matrix_modelview|matrix_modelView|matrix_ModelView|matrix_Modelview|Matrix_modelview|Matrix_modelView|Matrix_ModelView|Matrix_Modelview|" 
		"modelviewmatrix|modelViewmatrix|ModelViewmatrix|Modelviewmatrix|modelviewMatrix|modelViewMatrix|ModelViewMatrix|ModelviewMatrix|" 
		"matrixmodelview|matrixmodelView|matrixModelView|matrixModelview|Matrixmodelview|MatrixmodelView|MatrixModelView|MatrixModelview|",
		"mat_ambient|material_ambient|mat_Ambient|material_Ambient|matambient|materialambient|matAmbient|materialAmbient",
		"mat_diffuse|material_diffuse|mat_Diffuse|material_Diffuse|matdiffuse|materialdiffuse|matDiffuse|materialDiffuse",
		"mat_specular|material_specular|mat_Specular|material_Specular|matspecular|materialspecular|matSpecular|materialSpecular",
		"mat_emissive|material_emissive|mat_Emissive|material_Emissive|matemissive|materialemissive|matEmissive|materialEmissive",
		"shininess|Shininess|mat_shininess|material_shininess|mat_Shininess|material_Shininess|matshininess|materialshininess|matShininess|materialShininess"
	};
	size_t lstart = 0, lend = 0;
	std_uni_names += ";";
	for (size_t i = 0; i < UNI_COUNT; i++)
	{
		std::string str = "";
		lend = std_uni_names.find(";", lstart);
		if (lend != std::string::npos)
		{
			str = std_uni_names.substr(lstart, lend - lstart);
			lstart = lend + 1;
		}
		if (str.empty()) str = STD_UNI_NAMES[i];
		str += "|";

		size_t nstart = 0, nend = 0;
		while ((nend = str.find("|", nstart)) != std::string::npos)
		{
			std::string name = str.substr(nstart, nend - nstart);
			nstart = nend + 1;
			if (name.empty()) continue;

			// find the name among the registered variables
			std::map<std::string, UNIFORM>::iterator it = m_uniforms.find(name.c_str());
			if (it != m_uniforms.end())
			{
				m_stdUni[i] = it->second.location;
				log(M3DGL_SUCCESS_UNIFORM_FOUND, name, m_stdUni[i]);
			}
		}
	}

	return log(M3DGL_SUCCESS_LINKED);
}

bool C3dglProgram::use(bool bValidate)
{
	if (m_id == 0) return log(M3DGL_ERROR_PROGRAM_NOT_CREATED);
	
	glUseProgram(m_id);

	if (isUsed()) return true;	// nothing to do

	c_pCurrentProgram = this;

	if (!bValidate) return true;

	glValidateProgram(m_id);

	// collect info log
	GLint infoLen;
	glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &infoLen);
	infoLen = 0;
	if (infoLen < 1) return log(M3DGL_SUCCESS_VERIFICATION, "OK");
	std::vector<char> info(infoLen);
	glGetProgramInfoLog(m_id, infoLen, &infoLen, &info[0]);
	return log(M3DGL_SUCCESS_VERIFICATION, (info.size() <= 1 ? "OK" : std::string(info.begin(), info.end())));
}

GLint C3dglProgram::getAttribLocation(std::string idAttrib) const
{
	auto i = m_attribs.find(idAttrib);
	if (i == m_attribs.end())
	{
		GLint nLocation = glGetAttribLocation(m_id, idAttrib.c_str());
		m_attribs[idAttrib] = nLocation;
		if (nLocation == -1) log(M3DGL_WARNING_ATTRIBUTE_NOT_FOUND, idAttrib);
		else log(M3DGL_WARNING_ATTRIBUTE_NOT_REGISTERED, idAttrib);
		return nLocation;
	}
	else
		return i->second;
}

GLint C3dglProgram::getUniformLocation(std::string idUniform) const
{ 
	GLint location; 
	GLenum type; 
	getUniformLocationAndType(idUniform, location, type); 
	return location; 
}

GLint C3dglProgram::getUniformLocation(std::string idUniform, size_t index) const
{
	return getUniformLocation(idUniform + "[" + std::to_string(index) + "]");
}

GLint C3dglProgram::getUniformLocation(UNI_STD uniId) const
{ 
	if (uniId < 0 || uniId >= UNI_COUNT)
	{
		log(M3DGL_ERROR_WRONG_STD_UNIFORM_ID, (unsigned)UNI_COUNT);
		return -1;
	}
	return m_stdUni[uniId]; 
}

void C3dglProgram::getUniformLocationAndType(std::string idUniform, GLint &location, GLenum &type) const
{
	auto i = m_uniforms.find(idUniform);
	
	// first - a direct hit
	if (i != m_uniforms.end())
	{
		// Uniform found
		UNIFORM uni = (i->second);
		location = uni.location;
		type = uni.datatype;
		return;
	}

	// if not found, search for corresponding array name...
	size_t nPos = idUniform.find('[');
	if (nPos != std::string::npos)
	{
		// look for an array
		i = m_uniforms.find(idUniform.substr(0, nPos));
		if (i != m_uniforms.end())
		{
			// array located...
			UNIFORM uni = (i->second);
			uni.location = glGetUniformLocation(m_id, idUniform.c_str());
			m_uniforms[idUniform] = uni;
			type = uni.datatype;
			return;
		}
	}

	// if all else fails, process as unregistred variable
	location = glGetUniformLocation(m_id, idUniform.c_str());
	type = GLenum(0);
	m_uniforms[idUniform] = UNIFORM(location, 0);
	if (location == -1) log(M3DGL_WARNING_UNIFORM_NOT_FOUND, idUniform);
	else log(M3DGL_WARNING_UNIFORM_NOT_REGISTERED, idUniform);
}

void C3dglProgram::getUniformLocationAndType(std::string idUniform, size_t index, GLint& location, GLenum& type) const
{
	return getUniformLocationAndType(idUniform + "[" + std::to_string(index) + "]", location, type);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// sendUniform functions:

// Sending uniforms using location ids

void C3dglProgram::sendUniform(GLint location, GLfloat v0) { use(); glUniform1f(location, v0); m_values[location] = { .datatype = GL_FLOAT, .val_FLOAT = v0 }; }
void C3dglProgram::sendUniform(GLint location, GLint v0) { use(); glUniform1i(location, v0); m_values[location] = { .datatype = GL_INT, .val_INT = v0 }; }
void C3dglProgram::sendUniform(GLint location, GLuint v0) { use(); glUniform1ui(location, v0); m_values[location] = { .datatype = GL_UNSIGNED_INT, .val_UNSIGNED_INT = v0 }; }
void C3dglProgram::sendUniform(GLint location, glm::vec2 v) { use(); glUniform2f(location, v.x, v.y); m_values[location] = { .datatype = GL_FLOAT_VEC2, .val_FLOAT_VEC2 = v }; }
void C3dglProgram::sendUniform(GLint location, glm::vec3 v) { use(); glUniform3f(location, v.x, v.y, v.z); m_values[location] = { .datatype = GL_FLOAT_VEC3, .val_FLOAT_VEC3 = v }; }
void C3dglProgram::sendUniform(GLint location, glm::vec4 v) { use(); glUniform4f(location, v.x, v.y, v.z, v.w); m_values[location] = { .datatype = GL_FLOAT_VEC4, .val_FLOAT_VEC4 = v }; }
void C3dglProgram::sendUniform(GLint location, glm::ivec2 v) { use(); glUniform2i(location, v.x, v.y); m_values[location] = { .datatype = GL_INT_VEC2, .val_INT_VEC2 = v }; }
void C3dglProgram::sendUniform(GLint location, glm::ivec3 v) { use(); glUniform3i(location, v.x, v.y, v.z); m_values[location] = { .datatype = GL_INT_VEC3, .val_INT_VEC3 = v }; }
void C3dglProgram::sendUniform(GLint location, glm::ivec4 v) { use(); glUniform4i(location, v.x, v.y, v.z, v.w); m_values[location] = { .datatype = GL_INT_VEC4, .val_INT_VEC4 = v }; }
void C3dglProgram::sendUniform(GLint location, glm::uvec2 v) { use(); glUniform2ui(location, v.x, v.y); m_values[location] = { .datatype = GL_UNSIGNED_INT_VEC2, .val_UNSIGNED_INT_VEC2 = v }; }
void C3dglProgram::sendUniform(GLint location, glm::uvec3 v) { use(); glUniform3ui(location, v.x, v.y, v.z); m_values[location] = { .datatype = GL_UNSIGNED_INT_VEC3, .val_UNSIGNED_INT_VEC3 = v }; }
void C3dglProgram::sendUniform(GLint location, glm::uvec4 v) { use(); glUniform4ui(location, v.x, v.y, v.z, v.w); m_values[location] = { .datatype = GL_UNSIGNED_INT_VEC4, .val_UNSIGNED_INT_VEC4 = v }; }
void C3dglProgram::sendUniform(GLint location, glm::mat2 matrix) { use(); glUniformMatrix2fv(location, 1, GL_FALSE, &matrix[0][0]); m_values[location] = { .datatype = GL_FLOAT_MAT2, .val_FLOAT_MAT2 = matrix }; }
void C3dglProgram::sendUniform(GLint location, glm::mat3 matrix) { use(); glUniformMatrix3fv(location, 1, GL_FALSE, &matrix[0][0]); m_values[location] = { .datatype = GL_FLOAT_MAT3, .val_FLOAT_MAT3 = matrix }; }
void C3dglProgram::sendUniform(GLint location, glm::mat4 matrix) { use(); glUniformMatrix4fv(location, 1, GL_FALSE, &matrix[0][0]); m_values[location] = { .datatype = GL_FLOAT_MAT4, .val_FLOAT_MAT4 = matrix }; }

// Sending uniforms using location strings

bool C3dglProgram::sendUniform(std::string name, GLfloat v0)
{
	GLint location; GLenum t; getUniformLocationAndType(name, location, t);
	if (t == GL_FLOAT || t == 0) sendUniform(location, v0);
	else return log(M3DGL_ERROR_TYPE_MISMATCH, name, c_uniTypes[c_mapTypes[GL_FLOAT]].name, c_uniTypes[c_mapTypes[t]].name);
	return true;
}

bool C3dglProgram::sendUniform(std::string name, GLint v0)
{
	GLint location; GLenum t; getUniformLocationAndType(name, location, t);
	if (t == GL_INT || t == 0) sendUniform(location, v0);
	else if (t == GL_UNSIGNED_INT) sendUniform(location, (GLuint)v0);
	else if (t == GL_BOOL) sendUniform(location, v0);
	else if (t == GL_FLOAT) sendUniform(location, (GLfloat)v0);
	else return log(M3DGL_ERROR_TYPE_MISMATCH, name, c_uniTypes[c_mapTypes[GL_INT]].name, c_uniTypes[c_mapTypes[t]].name);
	return true;
}

bool C3dglProgram::sendUniform(std::string name, GLuint v0)
{
	GLint location; GLenum t; getUniformLocationAndType(name, location, t);
	if (t == GL_INT) sendUniform(location, (GLint)v0);
	else if (t == GL_UNSIGNED_INT || t == 0) sendUniform(location, v0);
	else if (t == GL_BOOL) sendUniform(location, v0);
	else if (t == GL_FLOAT) sendUniform(location, (GLfloat)v0);
	else return log(M3DGL_ERROR_TYPE_MISMATCH, name, c_uniTypes[c_mapTypes[GL_UNSIGNED_INT]].name, c_uniTypes[c_mapTypes[t]].name);
	return true;
}

bool C3dglProgram::sendUniform(std::string name, glm::vec2 v)
{
	GLint location; GLenum t; getUniformLocationAndType(name, location, t);
	if (t == GL_FLOAT_VEC2 || t == 0) sendUniform(location, v);
	else return log(M3DGL_ERROR_TYPE_MISMATCH, name, c_uniTypes[c_mapTypes[GL_FLOAT_VEC2]].name, c_uniTypes[c_mapTypes[t]].name);
	return true;
}

bool C3dglProgram::sendUniform(std::string name, glm::vec3 v)
{
	GLint location; GLenum t; getUniformLocationAndType(name, location, t);
	if (t == GL_FLOAT_VEC3 || t == 0) sendUniform(location, v);
	else return log(M3DGL_ERROR_TYPE_MISMATCH, name, c_uniTypes[c_mapTypes[GL_FLOAT_VEC3]].name, c_uniTypes[c_mapTypes[t]].name);
	return true;
}

bool C3dglProgram::sendUniform(std::string name, glm::vec4 v)
{
	GLint location; GLenum t; getUniformLocationAndType(name, location, t);
	if (t == GL_FLOAT_VEC4 || t == 0) sendUniform(location, v);
	else return log(M3DGL_ERROR_TYPE_MISMATCH, name, c_uniTypes[c_mapTypes[GL_FLOAT_VEC4]].name, c_uniTypes[c_mapTypes[t]].name);
	return true;
}

bool C3dglProgram::sendUniform(std::string name, glm::ivec2 v)
{
	GLint location; GLenum t; getUniformLocationAndType(name, location, t);
	if (t == GL_INT_VEC2 || t == 0) sendUniform(location, v);
	else if (t == GL_UNSIGNED_INT_VEC2) sendUniform(location, glm::uvec2(v));
	else if (t == GL_BOOL_VEC2) sendUniform(location, v);
	else if (t == GL_FLOAT_VEC2) sendUniform(location, glm::vec2(v));
	else return log(M3DGL_ERROR_TYPE_MISMATCH, name, c_uniTypes[c_mapTypes[GL_INT_VEC2]].name, c_uniTypes[c_mapTypes[t]].name);
	return true;
}

bool C3dglProgram::sendUniform(std::string name, glm::ivec3 v)
{
	GLint location; GLenum t; getUniformLocationAndType(name, location, t);
	if (t == GL_INT_VEC3 || t == 0) sendUniform(location, v);
	else if (t == GL_UNSIGNED_INT_VEC3) sendUniform(location, glm::uvec3(v));
	else if (t == GL_BOOL_VEC3) sendUniform(location, v);
	else if (t == GL_FLOAT_VEC3) sendUniform(location, glm::vec3(v));
	else return log(M3DGL_ERROR_TYPE_MISMATCH, name, c_uniTypes[c_mapTypes[GL_INT_VEC3]].name, c_uniTypes[c_mapTypes[t]].name);
	return true;
}

bool C3dglProgram::sendUniform(std::string name, glm::ivec4 v)
{
	GLint location; GLenum t; getUniformLocationAndType(name, location, t);
	if (t == GL_INT_VEC4 || t == 0) sendUniform(location, v);
	else if (t == GL_UNSIGNED_INT_VEC4) sendUniform(location, glm::uvec4(v));
	else if (t == GL_BOOL_VEC4) sendUniform(location, v);
	else if (t == GL_FLOAT_VEC4) sendUniform(location, glm::vec4(v));
	else return log(M3DGL_ERROR_TYPE_MISMATCH, name, c_uniTypes[c_mapTypes[GL_INT_VEC4]].name, c_uniTypes[c_mapTypes[t]].name);
	return true;
}

bool C3dglProgram::sendUniform(std::string name, glm::uvec2 v)
{
	GLint location; GLenum t; getUniformLocationAndType(name, location, t);
	if (t == GL_INT_VEC2) sendUniform(location, glm::ivec2(v));
	else if (t == GL_UNSIGNED_INT_VEC2 || t == 0) sendUniform(location, v);
	else if (t == GL_BOOL_VEC2) sendUniform(location, v);
	else if (t == GL_FLOAT_VEC2) sendUniform(location, glm::vec2(v));
	else return log(M3DGL_ERROR_TYPE_MISMATCH, name, c_uniTypes[c_mapTypes[GL_UNSIGNED_INT_VEC2]].name, c_uniTypes[c_mapTypes[t]].name);
	return true;
}

bool C3dglProgram::sendUniform(std::string name, glm::uvec3 v)
{
	GLint location; GLenum t; getUniformLocationAndType(name, location, t);
	if (t == GL_INT_VEC3) sendUniform(location, glm::ivec3(v));
	else if (t == GL_UNSIGNED_INT_VEC3 || t == 0) sendUniform(location, v);
	else if (t == GL_BOOL_VEC3) sendUniform(location, v);
	else if (t == GL_FLOAT_VEC3) sendUniform(location, glm::vec3(v));
	else return log(M3DGL_ERROR_TYPE_MISMATCH, name, c_uniTypes[c_mapTypes[GL_UNSIGNED_INT_VEC3]].name, c_uniTypes[c_mapTypes[t]].name);
	return true;
}

bool C3dglProgram::sendUniform(std::string name, glm::uvec4 v)
{
	GLint location; GLenum t; getUniformLocationAndType(name, location, t);
	if (t == GL_INT_VEC4) sendUniform(location, glm::ivec4(v));
	else if (t == GL_UNSIGNED_INT_VEC4 || t == 0) sendUniform(location, v);
	else if (t == GL_BOOL_VEC4) sendUniform(location, v);
	else if (t == GL_FLOAT_VEC4) sendUniform(location, glm::vec4(v));
	else return log(M3DGL_ERROR_TYPE_MISMATCH, name, c_uniTypes[c_mapTypes[GL_UNSIGNED_INT_VEC4]].name, c_uniTypes[c_mapTypes[t]].name);
	return true;
}

bool C3dglProgram::sendUniform(std::string name, glm::mat2 matrix)
{
	GLint location; GLenum t; getUniformLocationAndType(name, location, t);
	if (t == GL_FLOAT_MAT2 || t == 0) sendUniform(location, matrix);
	else return log(M3DGL_ERROR_TYPE_MISMATCH, name, c_uniTypes[c_mapTypes[GL_FLOAT_MAT2]].name, c_uniTypes[c_mapTypes[t]].name);
	return true;
}

bool C3dglProgram::sendUniform(std::string name, glm::mat3 matrix)
{
	GLint location; GLenum t; getUniformLocationAndType(name, location, t);
	if (t == GL_FLOAT_MAT3 || t == 0) sendUniform(location, matrix);
	else return log(M3DGL_ERROR_TYPE_MISMATCH, name, c_uniTypes[c_mapTypes[GL_FLOAT_MAT3]].name, c_uniTypes[c_mapTypes[t]].name);
	return true;
}

bool C3dglProgram::sendUniform(std::string name, glm::mat4 matrix)
{
	GLint location; GLenum t; getUniformLocationAndType(name, location, t);
	if (t == GL_FLOAT_MAT4 || t == 0) sendUniform(location, matrix);
	else return log(M3DGL_ERROR_TYPE_MISMATCH, name, c_uniTypes[c_mapTypes[GL_FLOAT_MAT4]].name, c_uniTypes[c_mapTypes[t]].name);
	return true;
}

// Sending arrays using location ids

void C3dglProgram::sendUniform(GLint location, GLfloat *p, size_t count) { use(); glUniform1fv(location, (GLuint)count, p); }
void C3dglProgram::sendUniform(GLint location, GLint* p, size_t count) { use(); glUniform1iv(location, (GLuint)count, p); }
void C3dglProgram::sendUniform(GLint location, GLuint* p, size_t count) { use(); glUniform1uiv(location, (GLuint)count, p); }
void C3dglProgram::sendUniform(GLint location, glm::vec2* p, size_t count) { use(); glUniform2fv(location, (GLuint)count, (GLfloat*)p); }
void C3dglProgram::sendUniform(GLint location, glm::vec3* p, size_t count) { use(); glUniform3fv(location, (GLuint)count, (GLfloat*)p); }
void C3dglProgram::sendUniform(GLint location, glm::vec4* p, size_t count) { use(); glUniform4fv(location, (GLuint)count, (GLfloat*)p); }
void C3dglProgram::sendUniform(GLint location, glm::ivec2* p, size_t count) { use(); glUniform2iv(location, (GLuint)count, (GLint*)p); }
void C3dglProgram::sendUniform(GLint location, glm::ivec3* p, size_t count) { use(); glUniform3iv(location, (GLuint)count, (GLint*)p); }
void C3dglProgram::sendUniform(GLint location, glm::ivec4* p, size_t count) { use(); glUniform4iv(location, (GLuint)count, (GLint*)p); }
void C3dglProgram::sendUniform(GLint location, glm::uvec2* p, size_t count) { use(); glUniform2uiv(location, (GLuint)count, (GLuint*)p); }
void C3dglProgram::sendUniform(GLint location, glm::uvec3* p, size_t count) { use(); glUniform3uiv(location, (GLuint)count, (GLuint*)p); }
void C3dglProgram::sendUniform(GLint location, glm::uvec4* p, size_t count) { use(); glUniform4uiv(location, (GLuint)count, (GLuint*)p); }
void C3dglProgram::sendUniform(GLint location, glm::mat2* p, size_t count) { use(); glUniformMatrix2fv(location, (GLuint)count, GL_FALSE, (GLfloat*)p); }
void C3dglProgram::sendUniform(GLint location, glm::mat3* p, size_t count) { use(); glUniformMatrix3fv(location, (GLuint)count, GL_FALSE, (GLfloat*)p); }
void C3dglProgram::sendUniform(GLint location, glm::mat4* p, size_t count) { use(); glUniformMatrix4fv(location, (GLuint)count, GL_FALSE, (GLfloat*)p); }

// Sending arrays using location strings

template<class T> bool C3dglProgram::_sendUniform(std::string name, T* p, size_t count, GLenum type)
{
	GLint location; GLenum t; getUniformLocationAndType(name, location, t);
	if (t == type || t == 0) sendUniform(location, p, count);
	else return log(M3DGL_ERROR_TYPE_MISMATCH, name, c_uniTypes[c_mapTypes[type]].name, c_uniTypes[c_mapTypes[t]].name);
	return true;
}

bool C3dglProgram::sendUniform(std::string name, GLfloat* p, size_t count) { return _sendUniform(name, p, count, GL_FLOAT); }
bool C3dglProgram::sendUniform(std::string name, GLint* p, size_t count) { return _sendUniform(name, p, count, GL_INT); }
bool C3dglProgram::sendUniform(std::string name, GLuint* p, size_t count) { return _sendUniform(name, p, count, GL_UNSIGNED_INT); }
bool C3dglProgram::sendUniform(std::string name, glm::vec2* p, size_t count) { return _sendUniform(name, p, count, GL_FLOAT_VEC2); }
bool C3dglProgram::sendUniform(std::string name, glm::vec3* p, size_t count) { return _sendUniform(name, p, count, GL_FLOAT_VEC3); }
bool C3dglProgram::sendUniform(std::string name, glm::vec4* p, size_t count) { return _sendUniform(name, p, count, GL_FLOAT_VEC4); }
bool C3dglProgram::sendUniform(std::string name, glm::ivec2* p, size_t count) { return _sendUniform(name, p, count, GL_INT_VEC2); }
bool C3dglProgram::sendUniform(std::string name, glm::ivec3* p, size_t count) { return _sendUniform(name, p, count, GL_INT_VEC3); }
bool C3dglProgram::sendUniform(std::string name, glm::ivec4* p, size_t count) { return _sendUniform(name, p, count, GL_INT_VEC4); }
bool C3dglProgram::sendUniform(std::string name, glm::uvec2* p, size_t count) { return _sendUniform(name, p, count, GL_UNSIGNED_INT_VEC2); }
bool C3dglProgram::sendUniform(std::string name, glm::uvec3* p, size_t count) { return _sendUniform(name, p, count, GL_UNSIGNED_INT_VEC3); }
bool C3dglProgram::sendUniform(std::string name, glm::uvec4* p, size_t count) { return _sendUniform(name, p, count, GL_UNSIGNED_INT_VEC4); }
bool C3dglProgram::sendUniform(std::string name, glm::mat2* p, size_t count) { return _sendUniform(name, p, count, GL_FLOAT_MAT2); }
bool C3dglProgram::sendUniform(std::string name, glm::mat3* p, size_t count) { return _sendUniform(name, p, count, GL_FLOAT_MAT3); }
bool C3dglProgram::sendUniform(std::string name, glm::mat4* p, size_t count) { return _sendUniform(name, p, count, GL_FLOAT_MAT4); }

// Sending array items using location names and index

bool C3dglProgram::sendUniform(std::string name, size_t index, GLfloat v) { return sendUniform(name + "[" + std::to_string(index) + "]", v); }
bool C3dglProgram::sendUniform(std::string name, size_t index, GLint v) { return sendUniform(name + "[" + std::to_string(index) + "]", v); }
bool C3dglProgram::sendUniform(std::string name, size_t index, GLuint v) { return sendUniform(name + "[" + std::to_string(index) + "]", v); }
bool C3dglProgram::sendUniform(std::string name, size_t index, glm::vec2 v) { return sendUniform(name + "[" + std::to_string(index) + "]", v); }
bool C3dglProgram::sendUniform(std::string name, size_t index, glm::vec3 v) { return sendUniform(name + "[" + std::to_string(index) + "]", v); }
bool C3dglProgram::sendUniform(std::string name, size_t index, glm::vec4 v) { return sendUniform(name + "[" + std::to_string(index) + "]", v); }
bool C3dglProgram::sendUniform(std::string name, size_t index, glm::ivec2 v) { return sendUniform(name + "[" + std::to_string(index) + "]", v); }
bool C3dglProgram::sendUniform(std::string name, size_t index, glm::ivec3 v) { return sendUniform(name + "[" + std::to_string(index) + "]", v); }
bool C3dglProgram::sendUniform(std::string name, size_t index, glm::ivec4 v) { return sendUniform(name + "[" + std::to_string(index) + "]", v); }
bool C3dglProgram::sendUniform(std::string name, size_t index, glm::uvec2 v) { return sendUniform(name + "[" + std::to_string(index) + "]", v); }
bool C3dglProgram::sendUniform(std::string name, size_t index, glm::uvec3 v) { return sendUniform(name + "[" + std::to_string(index) + "]", v); }
bool C3dglProgram::sendUniform(std::string name, size_t index, glm::uvec4 v) { return sendUniform(name + "[" + std::to_string(index) + "]", v); }
bool C3dglProgram::sendUniform(std::string name, size_t index, glm::mat2 v) { return sendUniform(name + "[" + std::to_string(index) + "]", v); }
bool C3dglProgram::sendUniform(std::string name, size_t index, glm::mat3 v) { return sendUniform(name + "[" + std::to_string(index) + "]", v); }
bool C3dglProgram::sendUniform(std::string name, size_t index, glm::mat4 v) { return sendUniform(name + "[" + std::to_string(index) + "]", v); }

// send a standard uniform using one of the UNI_STD values

template<class T> bool C3dglProgram::_sendUniform(enum UNI_STD stdloc, T v)
{
	GLint i = getUniformLocation(stdloc); 
	if (i == -1) 
		return false; 
	sendUniform(i, v); 
	return true;
}

bool C3dglProgram::sendUniform(enum UNI_STD stdloc, GLfloat v) { return _sendUniform(stdloc, v); }
bool C3dglProgram::sendUniform(enum UNI_STD stdloc, glm::vec2 v) { return _sendUniform(stdloc, v); }
bool C3dglProgram::sendUniform(enum UNI_STD stdloc, glm::vec3 v) { return _sendUniform(stdloc, v); }
bool C3dglProgram::sendUniform(enum UNI_STD stdloc, glm::vec4 v) { return _sendUniform(stdloc, v); }
bool C3dglProgram::sendUniform(enum UNI_STD stdloc, glm::mat2 v) { return _sendUniform(stdloc, v); }
bool C3dglProgram::sendUniform(enum UNI_STD stdloc, glm::mat3 v) { return _sendUniform(stdloc, v); }
bool C3dglProgram::sendUniform(enum UNI_STD stdloc, glm::mat4 v) { return _sendUniform(stdloc, v); }

// Retrieve stored uniforms

bool C3dglProgram::retrieveUniform(GLint location, GLfloat &v) 
{ 
	auto i = m_values.find(location); 
	if (i == m_values.end() || i->second.datatype != GL_FLOAT)
		return false;
	v = i->second.val_FLOAT;
	return true;
}

bool C3dglProgram::retrieveUniform(GLint location, GLint &v)
{
	auto i = m_values.find(location);
	if (i == m_values.end() || i->second.datatype != GL_INT)
		return false;
	v = i->second.val_INT;
	return true;
}

bool C3dglProgram::retrieveUniform(GLint location, GLuint &v)
{
	auto i = m_values.find(location);
	if (i == m_values.end() || i->second.datatype != GL_UNSIGNED_INT)
		return false;
	v = i->second.val_UNSIGNED_INT;
	return true;
}

bool C3dglProgram::retrieveUniform(GLint location, glm::vec2& v)
{
	auto i = m_values.find(location);
	if (i == m_values.end() || i->second.datatype != GL_FLOAT_VEC2)
		return false;
	v = i->second.val_FLOAT_VEC2;
	return true;
}

bool C3dglProgram::retrieveUniform(GLint location, glm::vec3& v)
{
	auto i = m_values.find(location);
	if (i == m_values.end() || i->second.datatype != GL_FLOAT_VEC3)
		return false;
	v = i->second.val_FLOAT_VEC3;
	return true;
}

bool C3dglProgram::retrieveUniform(GLint location, glm::vec4& v)
{
	auto i = m_values.find(location);
	if (i == m_values.end() || i->second.datatype != GL_FLOAT_VEC4)
		return false;
	v = i->second.val_FLOAT_VEC4;
	return true;
}

bool C3dglProgram::retrieveUniform(GLint location, glm::ivec2& v)
{
	auto i = m_values.find(location);
	if (i == m_values.end() || i->second.datatype != GL_INT_VEC2)
		return false;
	v = i->second.val_INT_VEC2;
	return true;
}

bool C3dglProgram::retrieveUniform(GLint location, glm::ivec3& v)
{
	auto i = m_values.find(location);
	if (i == m_values.end() || i->second.datatype != GL_INT_VEC3)
		return false;
	v = i->second.val_INT_VEC3;
	return true;
}

bool C3dglProgram::retrieveUniform(GLint location, glm::ivec4& v)
{
	auto i = m_values.find(location);
	if (i == m_values.end() || i->second.datatype != GL_INT_VEC4)
		return false;
	v = i->second.val_INT_VEC4;
	return true;
}

bool C3dglProgram::retrieveUniform(GLint location, glm::uvec2& v)
{
	auto i = m_values.find(location);
	if (i == m_values.end() || i->second.datatype != GL_UNSIGNED_INT_VEC2)
		return false;
	v = i->second.val_UNSIGNED_INT_VEC2;
	return true;
}

bool C3dglProgram::retrieveUniform(GLint location, glm::uvec3& v)
{
	auto i = m_values.find(location);
	if (i == m_values.end() || i->second.datatype != GL_UNSIGNED_INT_VEC3)
		return false;
	v = i->second.val_UNSIGNED_INT_VEC3;
	return true;
}

bool C3dglProgram::retrieveUniform(GLint location, glm::uvec4& v)
{
	auto i = m_values.find(location);
	if (i == m_values.end() || i->second.datatype != GL_UNSIGNED_INT_VEC4)
		return false;
	v = i->second.val_UNSIGNED_INT_VEC4;
	return true;
}

bool C3dglProgram::retrieveUniform(GLint location, glm::mat2& v)
{
	auto i = m_values.find(location);
	if (i == m_values.end() || i->second.datatype != GL_FLOAT_MAT2)
		return false;
	v = i->second.val_FLOAT_MAT2;
	return true;
}

bool C3dglProgram::retrieveUniform(GLint location, glm::mat3& v)
{
	auto i = m_values.find(location);
	if (i == m_values.end() || i->second.datatype != GL_FLOAT_MAT3)
		return false;
	v = i->second.val_FLOAT_MAT3;
	return true;
}

bool C3dglProgram::retrieveUniform(GLint location, glm::mat4& v)
{
	auto i = m_values.find(location);
	if (i == m_values.end() || i->second.datatype != GL_FLOAT_MAT4)
		return false;
	v = i->second.val_FLOAT_MAT4;
	return true;
}

bool C3dglProgram::retrieveUniform(std::string name, GLfloat& v)					{ return retrieveUniform(getUniformLocation(name), v); }
bool C3dglProgram::retrieveUniform(std::string name, GLint& v)						{ return retrieveUniform(getUniformLocation(name), v); }
bool C3dglProgram::retrieveUniform(std::string name, GLuint& v)						{ return retrieveUniform(getUniformLocation(name), v); }
bool C3dglProgram::retrieveUniform(std::string name, glm::vec2 &v)					{ return retrieveUniform(getUniformLocation(name), v); }
bool C3dglProgram::retrieveUniform(std::string name, glm::vec3& v)					{ return retrieveUniform(getUniformLocation(name), v); }
bool C3dglProgram::retrieveUniform(std::string name, glm::vec4& v)					{ return retrieveUniform(getUniformLocation(name), v); }
bool C3dglProgram::retrieveUniform(std::string name, glm::ivec2& v)					{ return retrieveUniform(getUniformLocation(name), v); }
bool C3dglProgram::retrieveUniform(std::string name, glm::ivec3& v)					{ return retrieveUniform(getUniformLocation(name), v); }
bool C3dglProgram::retrieveUniform(std::string name, glm::ivec4& v)					{ return retrieveUniform(getUniformLocation(name), v); }
bool C3dglProgram::retrieveUniform(std::string name, glm::uvec2& v)					{ return retrieveUniform(getUniformLocation(name), v); }
bool C3dglProgram::retrieveUniform(std::string name, glm::uvec3& v)					{ return retrieveUniform(getUniformLocation(name), v); }
bool C3dglProgram::retrieveUniform(std::string name, glm::uvec4& v)					{ return retrieveUniform(getUniformLocation(name), v); }
bool C3dglProgram::retrieveUniform(std::string name, glm::mat2& v)					{ return retrieveUniform(getUniformLocation(name), v); }
bool C3dglProgram::retrieveUniform(std::string name, glm::mat3& v)					{ return retrieveUniform(getUniformLocation(name), v); }
bool C3dglProgram::retrieveUniform(std::string name, glm::mat4& v)					{ return retrieveUniform(getUniformLocation(name), v); }

bool C3dglProgram::retrieveUniform(std::string name, size_t index, GLfloat& v)		{ return retrieveUniform(getUniformLocation(name, index), v); }
bool C3dglProgram::retrieveUniform(std::string name, size_t index, GLint& v)		{ return retrieveUniform(getUniformLocation(name, index), v); }
bool C3dglProgram::retrieveUniform(std::string name, size_t index, GLuint& v)		{ return retrieveUniform(getUniformLocation(name, index), v); }
bool C3dglProgram::retrieveUniform(std::string name, size_t index, glm::vec2& v)	{ return retrieveUniform(getUniformLocation(name, index), v); }
bool C3dglProgram::retrieveUniform(std::string name, size_t index, glm::vec3& v)	{ return retrieveUniform(getUniformLocation(name, index), v); }
bool C3dglProgram::retrieveUniform(std::string name, size_t index, glm::vec4& v)	{ return retrieveUniform(getUniformLocation(name, index), v); }
bool C3dglProgram::retrieveUniform(std::string name, size_t index, glm::ivec2& v)	{ return retrieveUniform(getUniformLocation(name, index), v); }
bool C3dglProgram::retrieveUniform(std::string name, size_t index, glm::ivec3& v)	{ return retrieveUniform(getUniformLocation(name, index), v); }
bool C3dglProgram::retrieveUniform(std::string name, size_t index, glm::ivec4& v)	{ return retrieveUniform(getUniformLocation(name, index), v); }
bool C3dglProgram::retrieveUniform(std::string name, size_t index, glm::uvec2& v)	{ return retrieveUniform(getUniformLocation(name, index), v); }
bool C3dglProgram::retrieveUniform(std::string name, size_t index, glm::uvec3& v)	{ return retrieveUniform(getUniformLocation(name, index), v); }
bool C3dglProgram::retrieveUniform(std::string name, size_t index, glm::uvec4& v)	{ return retrieveUniform(getUniformLocation(name, index), v); }
bool C3dglProgram::retrieveUniform(std::string name, size_t index, glm::mat2& v)	{ return retrieveUniform(getUniformLocation(name, index), v); }
bool C3dglProgram::retrieveUniform(std::string name, size_t index, glm::mat3& v)	{ return retrieveUniform(getUniformLocation(name, index), v); }
bool C3dglProgram::retrieveUniform(std::string name, size_t index, glm::mat4& v)	{ return retrieveUniform(getUniformLocation(name, index), v); }

bool C3dglProgram::retrieveUniform(enum UNI_STD stdloc, GLfloat& v)					{ return retrieveUniform(getUniformLocation(stdloc), v); } 
bool C3dglProgram::retrieveUniform(enum UNI_STD stdloc, glm::vec2& v)				{ return retrieveUniform(getUniformLocation(stdloc), v); }
bool C3dglProgram::retrieveUniform(enum UNI_STD stdloc, glm::vec3& v)				{ return retrieveUniform(getUniformLocation(stdloc), v); }
bool C3dglProgram::retrieveUniform(enum UNI_STD stdloc, glm::vec4& v)				{ return retrieveUniform(getUniformLocation(stdloc), v); }
bool C3dglProgram::retrieveUniform(enum UNI_STD stdloc, glm::mat2& v)				{ return retrieveUniform(getUniformLocation(stdloc), v); }
bool C3dglProgram::retrieveUniform(enum UNI_STD stdloc, glm::mat3& v)				{ return retrieveUniform(getUniformLocation(stdloc), v); }
bool C3dglProgram::retrieveUniform(enum UNI_STD stdloc, glm::mat4& v)				{ return retrieveUniform(getUniformLocation(stdloc), v); }
