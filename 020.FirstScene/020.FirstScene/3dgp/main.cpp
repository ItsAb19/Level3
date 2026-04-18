#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <GL/glew.h>
#include <3dgl/3dgl.h>
#include <GL/glut.h>
#include <GL/freeglut_ext.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

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
vec3 emitterPos(15.0f, 2.0f, 0.0f);

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
			FragColor = vec4(1.0, 1.0, 1.0, 1.0);
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

	glUseProgram(particleShader);

	glUniformMatrix4fv(glGetUniformLocation(particleShader, "uProjection"), 1, GL_FALSE, (GLfloat*)&matrixProjection);
	glUniformMatrix4fv(glGetUniformLocation(particleShader, "uView"), 1, GL_FALSE, (GLfloat*)&matrixView);

	glBindVertexArray(particleVAO);
	glDrawArrays(GL_POINTS, 0, MAX_PARTICLES);
	glBindVertexArray(0);

	glUseProgram(0);

	glEnable(GL_LIGHTING);
}
// -------------------------------------------------

bool init()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glShadeModel(GL_SMOOTH);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	if (!camera.load("models\\camera.3ds")) return false;

	matrixView = rotate(mat4(1), radians(12.f), vec3(1, 0, 0));
	matrixView *= lookAt(
		vec3(0.0, 5.0, 10.0),
		vec3(0.0, 5.0, 0.0),
		vec3(0.0, 1.0, 0.0));

	initParticles();

	glClearColor(0.18f, 0.25f, 0.22f, 1.0f);

	return true;
}

void renderScene(mat4& matrixView, float time, float deltaTime)
{
	mat4 m;

	GLfloat rgbaGrey[] = { 0.6f, 0.6f, 0.6f, 1.0f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, rgbaGrey);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, rgbaGrey);

	m = matrixView;
	m = translate(m, vec3(-3.0f, 0, 0.0f));
	m = rotate(m, radians(180.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.04f, 0.04f, 0.04f));
	camera.render(m);

	GLfloat rgbaBlue[] = { 0.2f, 0.2f, 0.8f, 1.0f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, rgbaBlue);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, rgbaBlue);

	m = matrixView;
	m = translate(m, vec3(15.0f, 0, 0.0f));
	m = rotate(m, radians(120.f), vec3(0.0f, 1.0f, 0.0f));

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrixf((GLfloat*)&m);
	glutSolidTeapot(2.0);

	renderParticles();
}

void onRender()
{
	static float prev = 0;
	float time = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
	float deltaTime = time - prev;
	prev = time;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_vel = clamp(_vel + _acc * deltaTime, -vec3(maxspeed), vec3(maxspeed));
	float pitch = getPitch(matrixView);
	matrixView = rotate(translate(rotate(mat4(1),
		pitch, vec3(1, 0, 0)),
		_vel * deltaTime),
		-pitch, vec3(1, 0, 0))
		* matrixView;

	updateParticles(deltaTime);
	renderScene(matrixView, time, deltaTime);

	glutSwapBuffers();
	glutPostRedisplay();
}

void onReshape(int w, int h)
{
	float ratio = w * 1.0f / h;
	glViewport(0, 0, w, h);
	matrixProjection = perspective(radians(_fov), ratio, 0.02f, 1000.f);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMultMatrixf((GLfloat*)&matrixProjection);
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
	maxspeed = glutGetModifiers() & GLUT_ACTIVE_SHIFT ? 20.f : 4.f;
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
	maxspeed = glutGetModifiers() & GLUT_ACTIVE_SHIFT ? 20.f : 4.f;
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

	if (abs(deltaYaw) > 0.3f || abs(deltaPitch) > 0.3f)
		return;

	constexpr float maxPitch = radians(80.f);
	float pitch = getPitch(matrixView);
	float newPitch = glm::clamp(pitch + deltaPitch, -maxPitch, maxPitch);
	matrixView = rotate(rotate(rotate(mat4(1.f),
		newPitch, vec3(1.f, 0.f, 0.f)),
		deltaYaw, vec3(0.f, 1.f, 0.f)),
		-pitch, vec3(1.f, 0.f, 0.f))
		* matrixView;
}

void onMouseWheel(int button, int dir, int x, int y)
{
	_fov = glm::clamp(_fov - dir * 5.f, 5.0f, 175.f);
	onReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1280, 720);
	glutCreateWindow("3DGL Scene: First Example");

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