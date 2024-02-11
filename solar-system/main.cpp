/**
* Author: Ben Miller
* Assignment: Simple 2D Scene
* Date due: 2024-02-17, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

// window size
const int WINDOW_WIDTH = 640,
		  WINDOW_HEIGHT = 480;

// background color
const float BG_RED = 0.0f,
			BG_GREEN = 0.067f,
			BG_BLUE = 0.169f,
			BG_OPACITY = 1.0f;

// camera position and size
const int VIEWPORT_X = 0,
		  VIEWPORT_Y = 0,
		  VIEWPORT_WIDTH = WINDOW_WIDTH,
		  VIEWPORT_HEIGHT = WINDOW_HEIGHT;

// paths for shaders
const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
		   F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

// paths for object sprites
const char EARTH_SPRITE_PATH[] = "assets/earth.png",
		   MOON_SPRITE_PATH[] = "assets/moon.png";

// const for deltaTime calc
const float MILLISECONDS_IN_SECOND = 1000.0;

// texture constants
const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0; // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0; // this value MUST be zero

// shader and associated matrices
ShaderProgram g_shaderProgram;
glm::mat4 g_viewMatrix,
		  g_modelMatrixEarth,
		  g_modelMatrixMoon,
		  g_projectionMatrix;

// core globals
SDL_Window* g_displayWindow;
bool g_gameIsRunning = true;
float g_previousTicks;

// custom globals
GLuint g_earthTextureID;
GLuint g_moonTextureID;
bool g_earthMovingLeft = true;
bool g_moonMovingAway = true;
float g_earthCosInput = 0.0;
float g_earthTranslateX = 0.0;
float g_earthRotate = 0.0;
float g_moonDistance = 4.0;
float g_moonRotate = 0.0;
float g_moonScale = 1.0;

GLuint load_texture(const char* filepath) {
	// load image file
	int width, height, numOfComponents;
	unsigned char* image = stbi_load(filepath, &width, &height, &numOfComponents, STBI_rgb_alpha);
	if (image == NULL) {
		std::cout << "Unable to load image. Provided path '" << filepath << "' may be incorrect." << std::endl;
		assert(false);
	}
	// generate and bind texture ID
	GLuint textureID;
	glGenTextures(NUMBER_OF_TEXTURES, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

	// set filter parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// release image from memory and return the bound ID
	stbi_image_free(image);
	return textureID;
}

void draw_object(glm::mat4& modelMatrix, GLuint& textureID) {
	g_shaderProgram.set_model_matrix(modelMatrix);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void initialize() {
	SDL_Init(SDL_INIT_VIDEO);
	g_displayWindow = SDL_CreateWindow("Solar system!",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH, WINDOW_HEIGHT,
		SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(g_displayWindow);
	SDL_GL_MakeCurrent(g_displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 640, 480);

	g_shaderProgram.load(V_SHADER_PATH, F_SHADER_PATH);

	g_viewMatrix = glm::mat4(1.0f);
	g_modelMatrixEarth = glm::mat4(1.0f);
	g_modelMatrixMoon = glm::mat4(1.0f);
	g_projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

	g_shaderProgram.set_projection_matrix(g_projectionMatrix);
	g_shaderProgram.set_view_matrix(g_viewMatrix);

	glUseProgram(g_shaderProgram.get_program_id());

	g_earthTextureID = load_texture(EARTH_SPRITE_PATH);
	g_moonTextureID = load_texture(MOON_SPRITE_PATH);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_OPACITY);
}

void processInput() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			g_gameIsRunning = false;
		}
	}
}

void update() {
	float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
	float deltaTime = ticks - g_previousTicks; // the delta time is the difference from the last frame
	g_previousTicks = ticks;

	// calculations for earth
	g_earthCosInput += 1.2f * deltaTime;
	g_earthTranslateX = cos(g_earthCosInput) * 2.0f;
	
	// calculations for moon
	g_moonRotate += 30 * deltaTime;
	g_moonScale = 1.0f - (sin(glm::radians(g_moonRotate)) * 0.3f);
	g_moonDistance = 3.0f + (cos(glm::radians(2 * g_moonRotate)) * 0.4f);

	// reset model matrices
	g_modelMatrixEarth = glm::mat4(1.0f);
	g_modelMatrixMoon = glm::mat4(1.0f);

	// re-apply transformations
	g_modelMatrixEarth = glm::translate(g_modelMatrixEarth, glm::vec3(g_earthTranslateX, 0.0f, 0.0f));
	g_modelMatrixMoon = glm::translate(g_modelMatrixEarth, glm::vec3(0.0f, 0.0f, 0.0f));
	g_modelMatrixMoon = glm::rotate(g_modelMatrixMoon, glm::radians(g_moonRotate), glm::vec3(0.0f, 0.0f, 1.0f));
	g_modelMatrixMoon = glm::translate(g_modelMatrixMoon, glm::vec3(g_moonDistance, 0.0f, 0.0f));
	g_modelMatrixMoon = glm::scale(g_modelMatrixMoon, glm::vec3(g_moonScale, g_moonScale, 0.0f));
	g_modelMatrixEarth = glm::scale(g_modelMatrixEarth, glm::vec3(1.65f, 1.65f, 0.0f));
}

void render() {
	glClear(GL_COLOR_BUFFER_BIT);

	// vertices for square sprite
	float vertices[] = {
		-0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
		-0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
	};

	// vertices for texture coords
	float texture_coordinates[] = {
		0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
		0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
	};

	glVertexAttribPointer(g_shaderProgram.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(g_shaderProgram.get_position_attribute());

	glVertexAttribPointer(g_shaderProgram.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
	glEnableVertexAttribArray(g_shaderProgram.get_tex_coordinate_attribute());

	// draw the sprites here!
	draw_object(g_modelMatrixMoon, g_moonTextureID);
	draw_object(g_modelMatrixEarth, g_earthTextureID);

	glDisableVertexAttribArray(g_shaderProgram.get_position_attribute());
	glDisableVertexAttribArray(g_shaderProgram.get_tex_coordinate_attribute());

	SDL_GL_SwapWindow(g_displayWindow);
}

void shutdown() {
	SDL_Quit();
}

int main(int argc, char* argv[]) {
	initialize();

	while (g_gameIsRunning) {
		processInput();
		update();
		render();
	}

	shutdown();
	return 0;
}