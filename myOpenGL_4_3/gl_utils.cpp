// compute shaders tutorial
// Dr Anton Gerdelan <gerdela@scss.tcd.ie>
// Trinity College Dublin, Ireland
// 26 Feb 2016

#include "gl_utils.h"
#include <iostream>
#include <sstream>
#include <fstream>
using namespace std;

GLFWwindow* window;

bool start_gl() {
	{ // glfw
		if (!glfwInit()) {
			fprintf(stderr, "ERROR: could not start GLFW3\n");
			return false;
		}
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		window = glfwCreateWindow(WINDOW_W, WINDOW_H, "yiyun final project", NULL,
			NULL);
		if (!window) {
			fprintf(stderr, "ERROR: could not open window with GLFW3\n");
			glfwTerminate();
			return false;
		}
		glfwMakeContextCurrent(window);
	}
	{ // glew
		glewExperimental = GL_TRUE;
		glewInit();
	}
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version %s\n", version);
	return true;
}

void stop_gl() { glfwTerminate(); }

void print_shader_info_log(GLuint shader) {
	int max_length = 4096;
	int actual_length = 0;
	char slog[4096];
	glGetShaderInfoLog(shader, max_length, &actual_length, slog);
	fprintf(stderr, "shader info log for GL index %u\n%s\n", shader, slog);
}

void print_program_info_log(GLuint program) {
	int max_length = 4096;
	int actual_length = 0;
	char plog[4096];
	glGetProgramInfoLog(program, max_length, &actual_length, plog);
	fprintf(stderr, "program info log for GL index %u\n%s\n", program, plog);
}

bool check_shader_errors(GLuint shader) {
	GLint params = -1;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &params);
	cout << "check shader: " << shader << endl;
	if (GL_TRUE != params) {
		fprintf(stderr, "ERROR: shader %u did not compile\n", shader);
		print_shader_info_log(shader);
		return false;
	}
	return true;
}

bool check_program_errors(GLuint program) {
	GLint params = -1;
	glGetProgramiv(program, GL_LINK_STATUS, &params);
	cout << "check program: " << program << endl;
	if (GL_TRUE != params) {
		fprintf(stderr, "ERROR: program %u did not link\n", program);
		print_program_info_log(program);
		return false;
	}
	return true;
}

GLuint create_quad_vao() {
	GLuint vao = 0, vbo = 0;
	float verts[] = { -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 1.0f,
										1.0f,	-1.0f, 1.0f, 0.0f, 1.0f,	1.0f, 1.0f, 1.0f };
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), verts, GL_STATIC_DRAW);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	GLintptr stride = 4 * sizeof(float);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, NULL);
	glEnableVertexAttribArray(1);
	GLintptr offset = 2 * sizeof(float);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offset);
	return vao;
}

GLuint compileShader(GLenum type, string filename, string prepend) {
	// Read the file
	ifstream file(filename);
	if (!file.is_open()) {
		stringstream ss;
		ss << "Could not open " << filename << "!" << endl;
		throw runtime_error(ss.str());
	}
	stringstream buffer;
	buffer << prepend << endl;
	buffer << file.rdbuf();
	string bufStr = buffer.str();
	const char* bufCStr = bufStr.c_str();
	GLint length = bufStr.length();

	// Compile the shader
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &bufCStr, &length);
	glCompileShader(shader);

	// Make sure compilation succeeded
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		// Compilation failed, get the info log
		GLint logLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
		vector<GLchar> logText(logLength);
		glGetShaderInfoLog(shader, logLength, NULL, logText.data());

		// Construct an error message with the compile log
		stringstream ss;
		string typeStr = "";
		switch (type) {
		case GL_VERTEX_SHADER:
			typeStr = "vertex"; break;
		case GL_FRAGMENT_SHADER:
			typeStr = "fragment"; break;
		case GL_COMPUTE_SHADER:
			typeStr = "compute"; break;
		}
		ss << "Error compiling " + typeStr + " shader!" << endl << endl << logText.data() << endl;

		// Cleanup shader and throw an exception
		glDeleteShader(shader);
		throw runtime_error(ss.str());
	}

	return shader;
}

GLuint create_quad_program() {
	GLuint program = glCreateProgram();

	GLuint vert_shader;
	vert_shader = compileShader(GL_VERTEX_SHADER, "vertex_shader.glsl");
	cout << "vert: " << vert_shader << endl;
	check_shader_errors(vert_shader);
	glAttachShader(program, vert_shader);

	GLuint frag_shader;
	frag_shader = compileShader(GL_FRAGMENT_SHADER, "fragment_shader.glsl");
	cout << "frag: " << frag_shader << endl;
	check_shader_errors(frag_shader);
	glAttachShader(program, frag_shader);

	glLinkProgram(program);
	check_program_errors(program);
	return program;
}
