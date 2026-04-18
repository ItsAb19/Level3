#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <cmath>
#include <GL/glew.h>
#include <3dgl/3dgl.h>
#include <GL/glut.h>
#include <GL/freeglut_ext.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;
using namespace glm;

// 3D models
C3dglModel camera;

// Matrices
mat4 matrixView;
mat4 matrixProjection;

// Camera & navigation
float maxspeed = 4.f;
float accel = 4.f;
vec3 _acc(0), _vel(0);
float _fov = 60.f;

// Window size
int windowWidth = 1280;
int windowHeight = 720;

// ---------------- PARTICLE SYSTEM ----------------
struct Particle
{
	vec3 pos;
	vec3 vel;
	float life;
	float maxLife;
};

vector<Particle> particles;

GLuint particleVAO = 0;
GLuint particleVBO = 0;
GLuint particleShader = 0;

const int MAX_PARTICLES = 400;
vec3 emitterPos(0.0f, 2.5f, 0.0f);

// ---------------- POST-PROCESSING ----------------
GLuint postFBO = 0;
GLuint postColorTex = 0;
GLuint postDepthRBO = 0;

GLuint postVAO = 0;
GLuint postVBO = 0;
GLuint postShader = 0;

// ---------------- WATER ----------------
GLuint waterVAO = 0;
GLuint waterVBO = 0;
GLuint waterEBO = 0;
GLuint waterShader = 0;
GLsizei waterIndexCount = 0;
// -------------------------------------

GLuint compileShader(GLenum type, const char* source)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char info[1024];
		glGetShaderInfoLog(shader, 1024, nullptr, info);
		cout << info << endl;
	}
	return shader;
}

GLuint createShaderProgram(const char* vs, const char* fs)
{
	GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vs);
	GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fs);

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	GLint success = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		char info[1024];
		glGetProgramInfoLog(program, 1024, nullptr, info);
		cout << info << endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return program;
}

vec3 getHeroPosition(float time)
{
	return vec3(0.0f, 1.65f + sin(time * 1.8f) * 0.08f, 0.0f);
}

float randFloat(float a, float b)
{
	return a + (b - a) * (float(rand()) / float(RAND_MAX));
}

void resetParticle(Particle& p)
{
	p.pos = emitterPos;
	p.vel = vec3(
		randFloat(-0.8f, 0.8f),
		randFloat(2.0f, 4.5f),
		randFloat(-0.8f, 0.8f)
	);
	p.maxLife = randFloat(1.5f, 3.0f);
	p.life = p.maxLife;
}

void initParticles()
{
	srand((unsigned int)time(nullptr));

	particles.resize(MAX_PARTICLES);
	for (int i = 0; i < MAX_PARTICLES; i++)
		resetParticle(particles[i]);

	const char* particleVS = R"(
		#version 330 core

		layout(location = 0) in vec3 aPos;
		layout(location = 1) in float aLife;

		uniform mat4 uProjection;
		uniform mat4 uView;

		out float vLife;

		void main()
		{
			vLife = aLife;
			gl_Position = uProjection * uView * vec4(aPos, 1.0);
			gl_PointSize = 20.0;
		}
	)";

	const char* particleFS = R"(
		#version 330 core

		in float vLife;
		out vec4 FragColor;

		void main()
		{
			float alpha = clamp(vLife, 0.0, 1.0);
			FragColor = vec4(1.0, 0.7, 0.2, alpha);
		}
	)";

	particleShader = createShaderProgram(particleVS, particleFS);

	glGenVertexArrays(1, &particleVAO);
	glGenBuffers(1, &particleVBO);

	glBindVertexArray(particleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void updateParticles(float deltaTime)
{
	vector<float> buffer;
	buffer.reserve(MAX_PARTICLES * 4);

	for (auto& p : particles)
	{
		p.life -= deltaTime;

		if (p.life <= 0.0f)
			resetParticle(p);

		p.vel += vec3(0.0f, -2.0f, 0.0f) * deltaTime;
		p.pos += p.vel * deltaTime;

		float normalizedLife = p.life / p.maxLife;

		buffer.push_back(p.pos.x);
		buffer.push_back(p.pos.y);
		buffer.push_back(p.pos.z);
		buffer.push_back(normalizedLife);
	}

	glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, buffer.size() * sizeof(float), buffer.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void renderParticles()
{
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(particleShader);

	glUniformMatrix4fv(glGetUniformLocation(particleShader, "uProjection"), 1, GL_FALSE, value_ptr(matrixProjection));
	glUniformMatrix4fv(glGetUniformLocation(particleShader, "uView"), 1, GL_FALSE, value_ptr(matrixView));

	glBindVertexArray(particleVAO);
	glDrawArrays(GL_POINTS, 0, MAX_PARTICLES);
	glBindVertexArray(0);

	glUseProgram(0);

	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
}

// ---------------- POST-PROCESSING FUNCTIONS ----------------
bool createPostProcessBuffers(int w, int h)
{
	if (postFBO == 0) glGenFramebuffers(1, &postFBO);
	if (postColorTex == 0) glGenTextures(1, &postColorTex);
	if (postDepthRBO == 0) glGenRenderbuffers(1, &postDepthRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, postFBO);

	glBindTexture(GL_TEXTURE_2D, postColorTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postColorTex, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, postDepthRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, postDepthRBO);

	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "Framebuffer is not complete!" << endl;
		return false;
	}

	return true;
}

bool initPostProcessing()
{
	const char* postVS = R"(
		#version 330 core

		layout(location = 0) in vec2 aPos;
		layout(location = 1) in vec2 aTexCoord;

		out vec2 vTexCoord;

		void main()
		{
			vTexCoord = aTexCoord;
			gl_Position = vec4(aPos, 0.0, 1.0);
		}
	)";

	const char* postFS = R"(
		#version 330 core

		in vec2 vTexCoord;
		out vec4 FragColor;

		uniform sampler2D uScene;
		uniform float uTime;

		void main()
		{
			vec2 center = vec2(0.5, 0.5);
			vec2 dir = vTexCoord - center;

			float aberration = 0.0035;
			vec3 color;
			color.r = texture(uScene, vTexCoord + dir * aberration).r;
			color.g = texture(uScene, vTexCoord).g;
			color.b = texture(uScene, vTexCoord - dir * aberration).b;

			color = (color - 0.5) * 1.15 + 0.5;
			color *= vec3(0.98, 1.02, 1.06);

			float dist = distance(vTexCoord, center);
			float vignette = 1.0 - smoothstep(0.30, 0.82, dist);
			color *= vignette;

			float scanline = 0.99 + 0.01 * sin(vTexCoord.y * 950.0 + uTime * 2.0);
			color *= scanline;

			FragColor = vec4(color, 1.0);
		}
	)";

	postShader = createShaderProgram(postVS, postFS);

	float quadVertices[] =
	{
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f,

		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f,
		-1.0f,  1.0f,  0.0f, 1.0f
	};

	glGenVertexArrays(1, &postVAO);
	glGenBuffers(1, &postVBO);

	glBindVertexArray(postVAO);
	glBindBuffer(GL_ARRAY_BUFFER, postVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return createPostProcessBuffers(windowWidth, windowHeight);
}

void renderPostProcessedScene(float time)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glUseProgram(postShader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, postColorTex);
	glUniform1i(glGetUniformLocation(postShader, "uScene"), 0);
	glUniform1f(glGetUniformLocation(postShader, "uTime"), time);

	glBindVertexArray(postVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

// ---------------- WATER FUNCTIONS ----------------
bool initWater()
{
	const char* waterVS = R"(
		#version 330 core

		layout(location = 0) in vec3 aPos;

		uniform mat4 uModel;
		uniform mat4 uView;
		uniform mat4 uProjection;
		uniform float uTime;

		out vec3 vWorldPos;
		out vec3 vNormal;
		out float vWaveMask;

		float getHeight(vec2 p, float t)
		{
			float h = 0.0;
			h += sin(p.x * 3.2 + t * 1.2) * 0.06;
			h += cos(p.y * 2.6 + t * 0.9) * 0.045;
			h += sin((p.x + p.y) * 4.0 - t * 1.6) * 0.025;
			return h;
		}

		void main()
		{
			vec3 pos = aPos;

			float h = getHeight(pos.xz, uTime);
			pos.y += h;

			float eps = 0.01;
			float hL = getHeight(pos.xz - vec2(eps, 0.0), uTime);
			float hR = getHeight(pos.xz + vec2(eps, 0.0), uTime);
			float hD = getHeight(pos.xz - vec2(0.0, eps), uTime);
			float hU = getHeight(pos.xz + vec2(0.0, eps), uTime);

			vec3 dx = vec3(2.0 * eps, hR - hL, 0.0);
			vec3 dz = vec3(0.0, hU - hD, 2.0 * eps);
			vec3 normal = normalize(cross(dx, dz));

			mat4 model = uModel;
			mat3 normalMatrix = transpose(inverse(mat3(model)));

			vec4 worldPos = model * vec4(pos, 1.0);

			vWorldPos = worldPos.xyz;
			vNormal = normalize(normalMatrix * normal);
			vWaveMask = smoothstep(0.045, 0.11, abs(h));

			gl_Position = uProjection * uView * worldPos;
		}
	)";

	const char* waterFS = R"(
		#version 330 core

		in vec3 vWorldPos;
		in vec3 vNormal;
		in float vWaveMask;

		uniform vec3 uCameraPos;
		uniform vec3 uLightPos;
		uniform float uTime;

		out vec4 FragColor;

		void main()
		{
			vec3 N = normalize(vNormal);
			vec3 V = normalize(uCameraPos - vWorldPos);
			vec3 L = normalize(uLightPos - vWorldPos);

			float fresnel = pow(1.0 - max(dot(N, V), 0.0), 3.5);
			float diffuse = max(dot(N, L), 0.0);

			vec3 R = reflect(-L, N);
			float spec = pow(max(dot(R, V), 0.0), 72.0);

			float ripple = 0.5 + 0.5 * sin(vWorldPos.x * 2.5 + vWorldPos.z * 2.1 + uTime * 2.0);
			float ripple2 = 0.5 + 0.5 * cos(vWorldPos.x * 1.2 - vWorldPos.z * 1.6 - uTime * 1.5);

			vec3 deepColor = vec3(0.02, 0.10, 0.18);
			vec3 midColor = vec3(0.03, 0.22, 0.34);
			vec3 shallowColor = vec3(0.08, 0.45, 0.62);

			vec3 color = mix(deepColor, midColor, ripple * 0.45 + ripple2 * 0.15);
			color = mix(color, shallowColor, diffuse * 0.35 + fresnel * 0.25);

			color += fresnel * vec3(0.20, 0.50, 0.70);
			color += spec * vec3(0.95, 0.98, 1.0);
			color = mix(color, vec3(0.85, 0.95, 1.0), vWaveMask * 0.15);

			FragColor = vec4(color, 0.96);
		}
	)";

	waterShader = createShaderProgram(waterVS, waterFS);

	const int grid = 140;
	vector<float> verts;
	vector<unsigned int> indices;

	for (int z = 0; z <= grid; z++)
	{
		for (int x = 0; x <= grid; x++)
		{
			float fx = (float)x / (float)grid;
			float fz = (float)z / (float)grid;

			float px = fx * 2.0f - 1.0f;
			float pz = fz * 2.0f - 1.0f;

			verts.push_back(px);
			verts.push_back(0.0f);
			verts.push_back(pz);
		}
	}

	for (int z = 0; z < grid; z++)
	{
		for (int x = 0; x < grid; x++)
		{
			unsigned int start = z * (grid + 1) + x;

			indices.push_back(start);
			indices.push_back(start + grid + 1);
			indices.push_back(start + 1);

			indices.push_back(start + 1);
			indices.push_back(start + grid + 1);
			indices.push_back(start + grid + 2);
		}
	}

	waterIndexCount = (GLsizei)indices.size();

	glGenVertexArrays(1, &waterVAO);
	glGenBuffers(1, &waterVBO);
	glGenBuffers(1, &waterEBO);

	glBindVertexArray(waterVAO);

	glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waterEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	return true;
}

void renderWater(float time)
{
	glUseProgram(waterShader);

	mat4 model(1.0f);
	model = translate(model, vec3(0.0f, 0.55f, 0.0f));
	model = scale(model, vec3(1.15f, 1.0f, 1.15f));

	mat4 invView = inverse(matrixView);
	vec3 cameraPos = vec3(invView[3]);

	vec3 lightPos(0.0f, 7.0f, 4.0f);

	glUniformMatrix4fv(glGetUniformLocation(waterShader, "uModel"), 1, GL_FALSE, value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(waterShader, "uView"), 1, GL_FALSE, value_ptr(matrixView));
	glUniformMatrix4fv(glGetUniformLocation(waterShader, "uProjection"), 1, GL_FALSE, value_ptr(matrixProjection));
	glUniform1f(glGetUniformLocation(waterShader, "uTime"), time);
	glUniform3fv(glGetUniformLocation(waterShader, "uCameraPos"), 1, value_ptr(cameraPos));
	glUniform3fv(glGetUniformLocation(waterShader, "uLightPos"), 1, value_ptr(lightPos));

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindVertexArray(waterVAO);
	glDrawElements(GL_TRIANGLES, waterIndexCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glDisable(GL_BLEND);
	glUseProgram(0);
}

// ---------------- SCENE HELPERS ----------------
void setMaterialColor(const vec3& color, float ambientStrength = 0.35f, float specStrength = 0.25f, float shininess = 24.0f, const vec3& emission = vec3(0.0f))
{
	GLfloat diffuse[] = { color.r, color.g, color.b, 1.0f };
	GLfloat ambient[] =
	{
		color.r * ambientStrength,
		color.g * ambientStrength,
		color.b * ambientStrength,
		1.0f
	};
	GLfloat specular[] =
	{
		specStrength,
		specStrength,
		specStrength,
		1.0f
	};
	GLfloat emissive[] =
	{
		emission.r,
		emission.g,
		emission.b,
		1.0f
	};

	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emissive);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}

void drawCube(const mat4& view, const vec3& pos, const vec3& size, float rotYDeg, const vec3& color)
{
	setMaterialColor(color);

	mat4 m = view;
	m = translate(m, pos);
	m = rotate(m, radians(rotYDeg), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, size);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrixf(value_ptr(m));

	glutSolidCube(1.0f);
}

void drawSphere(const mat4& view, const vec3& pos, float radius, const vec3& color)
{
	setMaterialColor(color);

	mat4 m = view;
	m = translate(m, pos);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrixf(value_ptr(m));

	glutSolidSphere(radius, 24, 24);
}

void drawSphereGlow(const mat4& view, const vec3& pos, float radius, const vec3& color, const vec3& emission)
{
	setMaterialColor(color, 0.35f, 0.70f, 72.0f, emission);

	mat4 m = view;
	m = translate(m, pos);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrixf(value_ptr(m));

	glutSolidSphere(radius, 24, 24);
}

void drawTorus(const mat4& view, const vec3& pos, float innerRadius, float outerRadius, float rotXDeg, float rotYDeg, const vec3& color, const vec3& emission = vec3(0.0f))
{
	setMaterialColor(color, 0.35f, 0.55f, 64.0f, emission);

	mat4 m = view;
	m = translate(m, pos);
	m = rotate(m, radians(rotYDeg), vec3(0.0f, 1.0f, 0.0f));
	m = rotate(m, radians(rotXDeg), vec3(1.0f, 0.0f, 0.0f));

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrixf(value_ptr(m));

	glutSolidTorus(innerRadius, outerRadius, 24, 48);
}

void drawCameraProp(const mat4& view, const vec3& pos, float rotYDeg, const vec3& scaleValue, const vec3& color)
{
	setMaterialColor(color);

	mat4 m = view;
	m = translate(m, pos);
	m = rotate(m, radians(rotYDeg), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, scaleValue);

	camera.render(m);
}

bool init()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glShadeModel(GL_SMOOTH);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	if (!camera.load("models\\camera.3ds")) return false;

	matrixView = rotate(mat4(1), radians(12.f), vec3(1, 0, 0));
	matrixView *= lookAt(
		vec3(0.0f, 5.0f, 10.0f),
		vec3(0.0f, 5.0f, 0.0f),
		vec3(0.0f, 1.0f, 0.0f));

	initParticles();
	if (!initPostProcessing()) return false;
	if (!initWater()) return false;

	glClearColor(0.06f, 0.08f, 0.10f, 1.0f);

	return true;
}

void renderScene(mat4& matrixView, float time, float deltaTime)
{
	float pulse = 0.78f + 0.22f * sin(time * 2.4f);
	float lightOrbit = time * 0.8f;
	float orbBaseAngle = time * 0.95f;

	vec3 heroPos = getHeroPosition(time);

	vec3 stoneDark(0.14f, 0.15f, 0.18f);
	vec3 stoneMid(0.26f, 0.27f, 0.30f);
	vec3 stoneLight(0.46f, 0.47f, 0.50f);
	vec3 stoneColumn(0.34f, 0.34f, 0.38f);
	vec3 woodA(0.36f, 0.24f, 0.14f);
	vec3 woodB(0.42f, 0.28f, 0.16f);
	vec3 glowBlue(0.45f, 0.85f, 1.0f);
	vec3 glowEmission = vec3(0.07f, 0.18f, 0.30f) * (1.9f * pulse);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrixf(value_ptr(matrixView));

	GLfloat light0Pos[] = { 0.0f, 7.0f, 4.0f, 1.0f };
	GLfloat light0Diffuse[] = { 0.72f, 0.78f, 0.92f, 1.0f };
	GLfloat light0Ambient[] = { 0.10f, 0.10f, 0.13f, 1.0f };
	GLfloat light0Specular[] = { 0.55f, 0.58f, 0.65f, 1.0f };

	glLightfv(GL_LIGHT0, GL_POSITION, light0Pos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0Diffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0Ambient);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0Specular);

	GLfloat light1Pos[] =
	{
		heroPos.x + 0.35f * cos(lightOrbit),
		heroPos.y + 0.05f + 0.12f * sin(time * 1.8f),
		heroPos.z + 0.35f * sin(lightOrbit),
		1.0f
	};
	GLfloat light1Diffuse[] = { 0.18f * pulse, 0.62f * pulse, 1.0f * pulse, 1.0f };
	GLfloat light1Ambient[] = { 0.03f, 0.07f, 0.12f, 1.0f };
	GLfloat light1Specular[] = { 0.45f * pulse, 0.75f * pulse, 1.0f * pulse, 1.0f };
	GLfloat light1ConstAtt[] = { 1.0f };
	GLfloat light1LinearAtt[] = { 0.10f };
	GLfloat light1QuadAtt[] = { 0.03f };

	glLightfv(GL_LIGHT1, GL_POSITION, light1Pos);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light1Diffuse);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light1Ambient);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light1Specular);
	glLightfv(GL_LIGHT1, GL_CONSTANT_ATTENUATION, light1ConstAtt);
	glLightfv(GL_LIGHT1, GL_LINEAR_ATTENUATION, light1LinearAtt);
	glLightfv(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, light1QuadAtt);

	// room
	drawCube(matrixView, vec3(0.0f, -0.55f, 0.0f), vec3(18.0f, 0.4f, 18.0f), 0.0f, stoneMid);
	drawCube(matrixView, vec3(0.0f, 4.0f, -9.0f), vec3(18.0f, 9.0f, 0.4f), 0.0f, stoneDark);
	drawCube(matrixView, vec3(-9.0f, 4.0f, 0.0f), vec3(0.4f, 9.0f, 18.0f), 0.0f, stoneDark);
	drawCube(matrixView, vec3(9.0f, 4.0f, 0.0f), vec3(0.4f, 9.0f, 18.0f), 0.0f, stoneDark);

	// ceiling beams
	drawCube(matrixView, vec3(0.0f, 7.4f, -5.5f), vec3(13.5f, 0.20f, 0.45f), 0.0f, stoneColumn);
	drawCube(matrixView, vec3(0.0f, 7.4f, 0.0f), vec3(13.5f, 0.20f, 0.45f), 0.0f, stoneColumn);
	drawCube(matrixView, vec3(0.0f, 7.4f, 5.5f), vec3(13.5f, 0.20f, 0.45f), 0.0f, stoneColumn);
	drawCube(matrixView, vec3(-5.5f, 7.4f, 0.0f), vec3(0.45f, 0.20f, 13.5f), 0.0f, stoneColumn);
	drawCube(matrixView, vec3(5.5f, 7.4f, 0.0f), vec3(0.45f, 0.20f, 13.5f), 0.0f, stoneColumn);
	drawTorus(matrixView, vec3(0.0f, 7.0f, 0.0f), 0.06f, 1.75f, 90.0f, time * 10.0f, vec3(0.38f, 0.40f, 0.46f), vec3(0.01f, 0.02f, 0.04f));

	// stepped shrine base
	drawCube(matrixView, vec3(0.0f, -0.08f, 0.0f), vec3(6.2f, 0.22f, 6.2f), 0.0f, vec3(0.24f, 0.25f, 0.28f));
	drawCube(matrixView, vec3(0.0f, 0.05f, 0.0f), vec3(5.4f, 0.18f, 5.4f), 0.0f, vec3(0.30f, 0.31f, 0.34f));
	drawCube(matrixView, vec3(0.0f, 0.18f, 0.0f), vec3(4.8f, 0.45f, 4.8f), 0.0f, vec3(0.36f, 0.37f, 0.40f));

	// center shrine / pool frame
	drawCube(matrixView, vec3(0.0f, 0.52f, -1.65f), vec3(4.4f, 0.22f, 1.0f), 0.0f, stoneLight);
	drawCube(matrixView, vec3(0.0f, 0.52f, 1.65f), vec3(4.4f, 0.22f, 1.0f), 0.0f, stoneLight);
	drawCube(matrixView, vec3(-1.65f, 0.52f, 0.0f), vec3(1.0f, 0.22f, 2.3f), 0.0f, stoneLight);
	drawCube(matrixView, vec3(1.65f, 0.52f, 0.0f), vec3(1.0f, 0.22f, 2.3f), 0.0f, stoneLight);

	drawCube(matrixView, vec3(0.0f, 0.05f, 0.0f), vec3(2.4f, 0.18f, 2.4f), 0.0f, vec3(0.08f, 0.10f, 0.12f));

	drawTorus(matrixView, vec3(0.0f, 0.63f, 0.0f), 0.05f, 1.58f, 90.0f, time * 18.0f, vec3(0.42f, 0.82f, 1.0f), vec3(0.03f, 0.08f, 0.14f) * pulse);
	drawTorus(matrixView, vec3(0.0f, 2.28f + 0.05f * sin(time * 1.7f), 0.0f), 0.04f, 1.10f, 90.0f, time * 42.0f, vec3(0.55f, 0.88f, 1.0f), vec3(0.05f, 0.12f, 0.20f) * pulse);

	// corner obelisks
	drawCube(matrixView, vec3(-2.75f, 1.05f, -2.75f), vec3(0.34f, 1.60f, 0.34f), 45.0f, stoneColumn);
	drawCube(matrixView, vec3(2.75f, 1.05f, -2.75f), vec3(0.34f, 1.60f, 0.34f), 45.0f, stoneColumn);
	drawCube(matrixView, vec3(-2.75f, 1.05f, 2.75f), vec3(0.34f, 1.60f, 0.34f), 45.0f, stoneColumn);
	drawCube(matrixView, vec3(2.75f, 1.05f, 2.75f), vec3(0.34f, 1.60f, 0.34f), 45.0f, stoneColumn);

	drawSphereGlow(matrixView, vec3(-2.75f, 1.95f + 0.05f * sin(time * 2.2f), -2.75f), 0.18f, glowBlue, glowEmission);
	drawSphereGlow(matrixView, vec3(2.75f, 1.95f + 0.05f * sin(time * 2.2f + 0.7f), -2.75f), 0.18f, glowBlue, glowEmission);
	drawSphereGlow(matrixView, vec3(-2.75f, 1.95f + 0.05f * sin(time * 2.2f + 1.2f), 2.75f), 0.18f, glowBlue, glowEmission);
	drawSphereGlow(matrixView, vec3(2.75f, 1.95f + 0.05f * sin(time * 2.2f + 1.9f), 2.75f), 0.18f, glowBlue, glowEmission);

	// columns
	drawCube(matrixView, vec3(-6.0f, 2.2f, -5.5f), vec3(1.0f, 5.0f, 1.0f), 0.0f, stoneColumn);
	drawCube(matrixView, vec3(6.0f, 2.2f, -5.5f), vec3(1.0f, 5.0f, 1.0f), 0.0f, stoneColumn);
	drawCube(matrixView, vec3(-6.0f, 2.2f, 5.5f), vec3(1.0f, 5.0f, 1.0f), 0.0f, stoneColumn);
	drawCube(matrixView, vec3(6.0f, 2.2f, 5.5f), vec3(1.0f, 5.0f, 1.0f), 0.0f, stoneColumn);

	// crates / side props
	drawCube(matrixView, vec3(-5.2f, 0.1f, 3.2f), vec3(1.3f, 1.3f, 1.3f), 12.0f, woodA);
	drawCube(matrixView, vec3(-4.2f, 0.6f, 2.4f), vec3(1.0f, 1.0f, 1.0f), -10.0f, woodB);
	drawCube(matrixView, vec3(4.8f, 0.1f, 3.4f), vec3(1.2f, 1.2f, 1.2f), -8.0f, vec3(0.35f, 0.23f, 0.14f));
	drawCube(matrixView, vec3(5.9f, 0.1f, 2.7f), vec3(0.9f, 0.9f, 0.9f), 18.0f, vec3(0.40f, 0.26f, 0.15f));
	drawCube(matrixView, vec3(-5.0f, 0.2f, -3.8f), vec3(1.4f, 0.9f, 1.4f), 0.0f, vec3(0.28f, 0.19f, 0.12f));
	drawCube(matrixView, vec3(5.2f, 0.2f, -3.5f), vec3(1.5f, 0.8f, 1.2f), 0.0f, vec3(0.30f, 0.20f, 0.12f));

	// small decor
	drawSphere(matrixView, vec3(-2.8f, 0.35f, 4.6f), 0.35f, vec3(0.55f, 0.55f, 0.60f));
	drawSphere(matrixView, vec3(2.7f, 0.35f, 4.3f), 0.28f, vec3(0.48f, 0.48f, 0.52f));

	// camera prop
	drawCameraProp(matrixView, vec3(-4.0f, 0.7f, -1.5f), 210.0f, vec3(0.03f, 0.03f, 0.03f), vec3(0.55f, 0.55f, 0.58f));

	// water
	renderWater(time);

	// orbiting energy spheres
	const float TAU = 6.28318530718f;
	for (int i = 0; i < 5; i++)
	{
		float a = orbBaseAngle + i * (TAU / 5.0f);
		vec3 orbPos(
			heroPos.x + cos(a) * 1.35f,
			1.30f + 0.11f * sin(time * 3.0f + i),
			heroPos.z + sin(a) * 1.35f
		);

		drawSphereGlow(matrixView, orbPos, 0.11f, vec3(0.62f, 0.92f, 1.0f), vec3(0.06f, 0.16f, 0.26f) * pulse);
	}

	// hero object
	setMaterialColor(vec3(0.18f, 0.34f, 0.90f), 0.30f, 0.85f, 82.0f, vec3(0.02f, 0.05f, 0.14f) * pulse);
	mat4 m = matrixView;
	m = translate(m, heroPos);
	m = rotate(m, radians(120.0f + time * 24.0f), vec3(0.0f, 1.0f, 0.0f));

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrixf(value_ptr(m));
	glutSolidTeapot(0.95);

	// old particle effect above teapot
	renderParticles();

	setMaterialColor(vec3(0.5f));
}

void onRender()
{
	static float prev = 0.0f;
	float time = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
	float deltaTime = time - prev;
	prev = time;

	_vel = clamp(_vel + _acc * deltaTime, -vec3(maxspeed), vec3(maxspeed));
	float pitch = getPitch(matrixView);
	matrixView = rotate(translate(rotate(mat4(1.0f),
		pitch, vec3(1, 0, 0)),
		_vel * deltaTime),
		-pitch, vec3(1, 0, 0))
		* matrixView;

	emitterPos = getHeroPosition(time) + vec3(0.0f, 0.85f, 0.0f);
	updateParticles(deltaTime);

	glBindFramebuffer(GL_FRAMEBUFFER, postFBO);
	glViewport(0, 0, windowWidth, windowHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	renderScene(matrixView, time, deltaTime);

	renderPostProcessedScene(time);

	glutSwapBuffers();
	glutPostRedisplay();
}

void onReshape(int w, int h)
{
	windowWidth = w;
	windowHeight = h;

	float ratio = w * 1.0f / h;
	glViewport(0, 0, w, h);
	matrixProjection = perspective(radians(_fov), ratio, 0.02f, 1000.0f);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMultMatrixf(value_ptr(matrixProjection));

	createPostProcessBuffers(w, h);
}

void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': _acc.z = accel; break;
	case 's': _acc.z = -accel; break;
	case 'a': _acc.x = accel; break;
	case 'd': _acc.x = -accel; break;
	case 'e': _acc.y = accel; break;
	case 'q': _acc.y = -accel; break;
	}
}

void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w':
	case 's': _acc.z = _vel.z = 0; break;
	case 'a':
	case 'd': _acc.x = _vel.x = 0; break;
	case 'q':
	case 'e': _acc.y = _vel.y = 0; break;
	}
}

void onSpecDown(int key, int x, int y)
{
	maxspeed = glutGetModifiers() & GLUT_ACTIVE_SHIFT ? 20.0f : 4.0f;
	switch (key)
	{
	case GLUT_KEY_F4:		if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:		onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyDown('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyDown('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyDown('e', x, y); break;
	case GLUT_KEY_F11:		glutFullScreenToggle();
	}
}

void onSpecUp(int key, int x, int y)
{
	maxspeed = glutGetModifiers() & GLUT_ACTIVE_SHIFT ? 20.0f : 4.0f;
	switch (key)
	{
	case GLUT_KEY_UP:		onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyUp('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyUp('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyUp('e', x, y); break;
	}
}

void onMouse(int button, int state, int x, int y)
{
	glutSetCursor(state == GLUT_DOWN ? GLUT_CURSOR_CROSSHAIR : GLUT_CURSOR_INHERIT);
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
	if (button == 1)
	{
		_fov = 60.0f;
		onReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	}
}

void onMotion(int x, int y)
{
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);

	float deltaYaw = 0.005f * (x - glutGet(GLUT_WINDOW_WIDTH) / 2);
	float deltaPitch = 0.005f * (y - glutGet(GLUT_WINDOW_HEIGHT) / 2);

	if (fabs(deltaYaw) > 0.3f || fabs(deltaPitch) > 0.3f)
		return;

	constexpr float maxPitch = radians(80.0f);
	float pitch = getPitch(matrixView);
	float newPitch = glm::clamp(pitch + deltaPitch, -maxPitch, maxPitch);
	matrixView = rotate(rotate(rotate(mat4(1.0f),
		newPitch, vec3(1.0f, 0.0f, 0.0f)),
		deltaYaw, vec3(0.0f, 1.0f, 0.0f)),
		-pitch, vec3(1.0f, 0.0f, 0.0f))
		* matrixView;
}

void onMouseWheel(int button, int dir, int x, int y)
{
	_fov = glm::clamp(_fov - dir * 5.0f, 5.0f, 175.0f);
	onReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1280, 720);
	glutCreateWindow("3DGL Scene: Hero Demo - Water Shrine");

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		C3dglLogger::log("GLEW Error {}", (const char*)glewGetErrorString(err));
		return 0;
	}
	C3dglLogger::log("Using GLEW {}", (const char*)glewGetString(GLEW_VERSION));

	glutDisplayFunc(onRender);
	glutReshapeFunc(onReshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);
	glutMouseWheelFunc(onMouseWheel);

	C3dglLogger::log("Vendor: {}", (const char*)glGetString(GL_VENDOR));
	C3dglLogger::log("Renderer: {}", (const char*)glGetString(GL_RENDERER));
	C3dglLogger::log("Version: {}", (const char*)glGetString(GL_VERSION));
	C3dglLogger::log("");

	if (!init())
	{
		C3dglLogger::log("Application failed to initialise\r\n");
		return 0;
	}

	glutMainLoop();

	return 1;
}