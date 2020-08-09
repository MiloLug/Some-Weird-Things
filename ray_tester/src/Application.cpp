#include <windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "Uniforms.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <chrono>
#include <fstream>
#include <streambuf>

#define glCall(x) glClearErrors(); x; if (!glCheckErrors(#x, __FILE__, __LINE__)) __debugbreak();

using namespace GLTools;
using namespace std;

void glClearErrors() {
	while (glGetError() != GL_NO_ERROR);
}

bool glCheckErrors(const char* call, const char* file, int line) {
	unsigned int err;
	bool ok = true;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "[opengl error] (" << err << ")" << ": " << call << " in " << file << " on line " << line << std::endl;
		ok = false;
	}
	return ok;
}




int screenW = 1280;
int screenH = 720;

float playerX = 1;
float playerY = 1;
float playerA = 0;

float horizonY = 0;

#define PI 3.1415926

float POV = PI / 3.5;
float maxDistance = 35.0;

float distanceStep = 0.01;
float cornerDiffA = 0.01;


float pixelBlock[] = {
	0, 2.0f / screenH,
	0, 0,
	2.0f / screenW, 0,
	2.0f / screenW, 2.0f / screenH
};
unsigned int pixelBlockIndices[] = {
	0, 1, 2,
	3, 2, 0
};

bool isItABlock(int x, int y) {
	return (x * y * 1907) % 7907 > 3000;
}


unsigned int arrBuf = 0;
unsigned int indBuf = 0;
void genPixelBlock() {
	pixelBlock[1] = 2.0f / screenH;
	pixelBlock[4] = 2.0f / screenW;
	pixelBlock[6] = 2.0f / screenW;
	pixelBlock[7] = 2.0f / screenH;
	if (arrBuf == 0) {
		glGenBuffers(1, &arrBuf);
	}
	glBindBuffer(GL_ARRAY_BUFFER, arrBuf);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), pixelBlock, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);


	if (indBuf == 0) {
		glGenBuffers(1, &indBuf);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), pixelBlockIndices, GL_STATIC_DRAW);
};

inline glm::mat4 squareTransforms(float x, float y, float w, float h) {
	return glm::mat4(
		w, 0, 0, (x * 2) / screenW - 1.0,
		0, h, 0, 1.0 - (y * 2) / screenH,
		0, 0, 1, 1,
		0, 0, 0, 1
	);
}

void printSquare(float x, float y, float w, float h, UniformMat4& transform, float r, float g, float b, float a, UniformVec4& color) {
	transform.set(squareTransforms(x, y, w, h));
	color.set(r, g, b, a);
	glCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
}


void onMouseMove(GLFWwindow* window, double xpos, double ypos) {
	playerA += (xpos - screenW / 2.0) / screenW * 2.0;
	float tmpHorizon = horizonY + (ypos - screenH / 2.0) / screenH * 2000.0;
	if (fabs(tmpHorizon) < screenH / 1.8) {
		horizonY = tmpHorizon;
	}

	glfwSetCursorPos(window, screenW / 2.0, screenH / 2.0);
}

float rayCenterX = 0;
float rayCenterY = 0;

inline char getTesting(const char* map, float x, float y, int w, int h) {
	if (((y < h) + (y >= 0) + (x < w) + (x >= 0)) == 4)
		return map[(int)y * w + (int)x];
	else
		return 0;
}

inline char bigStepRay(const char* map, const int mapW, const int mapH, const float maxDistance, const glm::vec2 &ray, glm::vec2 &point, float &addDistance, float kX, float kY, UniformMat4& transform, UniformVec4& color) {
	glm::vec2 dtmp(
		ray.x*(maxDistance - addDistance),
		ray.y*(maxDistance - addDistance)
	);
	glm::vec2 delta = glm::abs(dtmp);
	float k = delta.x >= delta.y ? delta.x : delta.y;
	delta = dtmp / k;
	float distStep = sqrtf(glm::dot(delta, delta));

	char testing;

	while (addDistance < maxDistance) {
		printSquare(point.x*kX, point.y*kY, 2, 2, transform, 0, 1, 0, .7, color);
		testing = getTesting(map, point.x, point.y, mapW, mapH);
		switch (testing) {
		case '.':
		case '\0':
		case '@':
			break;
		default:
			addDistance -= distStep;
			point -= delta;
			return testing;
		}
		point += delta;
		addDistance += distStep;
	}
	return '\0';
}

inline char smallStepRay(const char* map, const int mapW, const int mapH, const float step, const const float maxDistance, const glm::vec2& ray, glm::vec2& point, float& addDistance) {
	glm::vec2 delta = ray * step;
	char testing;

	while (addDistance < maxDistance) {
		addDistance += step;
		point += delta;
		testing = getTesting(map, point.x, point.y, mapW, mapH);
		switch (testing) {
		case '.':
		case '\0':
			break;
		default:
			addDistance -= step;
			point -= delta;
			return testing;
		}
	}
	return '\0';
}


int main(void)
{
	GLFWwindow* window;

	if (!glfwInit())
		return -1;

	//glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, 1);
	window = glfwCreateWindow(screenW, screenH, "Hello World", NULL, NULL);

	glfwSwapInterval(1);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, screenW / 2.0, screenH / 2.0);

	glfwSetCursorPosCallback(window, onMouseMove);

	if (glewInit() != GLEW_OK) {
		std::cout << "oh fuck" << std::endl;
		return -1;
	};

	std::cout << glGetString(GL_VERSION) << std::endl;

	genPixelBlock();

	Shader sh1;
	sh1.setVertexFromFile("src/shaders/basic1/shader.vert");
	sh1.setFragmentFromFile("src/shaders/basic1/shader.frag");
	sh1.compile();
	sh1.use();

	std::ifstream t("src/map.txt");
	std::string tmpstr((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());
	std::string map_t = "";

	int mapW = 0;
	int mapH = 1;
	int mapTotal = tmpstr.length();
	int rc = 0;
	for (int i = 0; i < mapTotal; i++) {
		if (tmpstr[i] == '#' || tmpstr[i] == '.' || tmpstr[i] == ' ' || tmpstr[i] == '|' || tmpstr[i] == '@') {
			map_t += tmpstr[i];
			rc++;
		}
		if (tmpstr[i] == '\n') {
			mapW = rc;
			mapH++;
			rc = 0;
		}
	}

	const char* map = map_t.c_str();
	mapTotal = map_t.length();
	
	rayCenterX = map_t.find("@");
	rayCenterY = (int)(rayCenterX / mapW) + 0.5;
	rayCenterX = (int)rayCenterX % mapW + 0.5;
	
	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();

	UniformVec4 color(&sh1, "colorIn");
	UniformMat4 transform(&sh1, "transform");


	glEnable(GL_BLEND);
	//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float fElapsedTime;
	float speedMult;
	float tmpPlayerX;
	float tmpPlayerY;

	

	while (!glfwWindowShouldClose(window))
	{
		int swtmp, shtmp;
		glfwGetWindowSize(window, &swtmp, &shtmp);

		if (swtmp != screenW || shtmp != screenH) {
			glfwSetWindowSize(window, swtmp, shtmp);
			glfwMakeContextCurrent(window);
			glClear(GL_COLOR_BUFFER_BIT);
			screenW = swtmp;
			screenH = shtmp;
			sh1.use();
			genPixelBlock();
		}

		float drawPlaceH = screenH;
		float drawPlaceW = screenW;
		if (drawPlaceW < drawPlaceH) {
			drawPlaceH = drawPlaceW;
		}
		else if (drawPlaceW > drawPlaceH) {
			drawPlaceW = drawPlaceH;
		}

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		tp2 = chrono::system_clock::now();
		chrono::duration <float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		fElapsedTime = elapsedTime.count();

		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		speedMult = 5.0 * fElapsedTime;
		tmpPlayerX = 0;
		tmpPlayerY = 0;

		if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
			speedMult *= 2;
		}

		if (GetAsyncKeyState((unsigned short)'D') & 0x8000) {
			tmpPlayerY += sinf(playerA + PI / 2.0) * speedMult / 2;
			tmpPlayerX += cosf(playerA + PI / 2.0) * speedMult / 2;
		}
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000) {
			tmpPlayerY += sinf(playerA - PI / 2.0) * speedMult / 2;
			tmpPlayerX += cosf(playerA - PI / 2.0) * speedMult / 2;
		}
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
			tmpPlayerY += sinf(playerA) * speedMult;
			tmpPlayerX += cosf(playerA) * speedMult;
		}
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
			tmpPlayerY += -sinf(playerA) * speedMult;
			tmpPlayerX += -cosf(playerA) * speedMult;
		}

		tmpPlayerY += playerY;
		tmpPlayerX += playerX;

		if (!((int)tmpPlayerY >= mapH || (int)tmpPlayerY < 0 || (int)playerX >= mapW || (int)playerX < 0) && (map[(int)tmpPlayerY * mapW + (int)playerX] != L'#')) {
			playerY = tmpPlayerY;
		}

		if (!((int)tmpPlayerX >= mapW || (int)tmpPlayerX < 0 || (int)playerY >= mapH || (int)playerY < 0) && (map[(int)playerY * mapW + (int)tmpPlayerX] != L'#')) {
			playerX = tmpPlayerX;
		}

		int kY = screenH / mapH;
		int kX = screenW / mapW;

		glm::vec2 player(playerX, playerY);
		glm::vec2 rayCenter(rayCenterX, rayCenterY);
		float rayA = atan2(rayCenterY - playerY, rayCenterX - playerX);
		glm::vec2 ray(
			cosf(rayA),
			sinf(rayA)
		);
		glm::vec2 curPoint(player);

		float distance = 0;


		bool hasHit = false;
		int prevPX = 0;
		int prevPY = 0;
		

		for (int y = 0; y < mapH; y++) {
			for (int x = 0; x < mapW; x++) {
				switch (map[(int)y * mapW + (int)x]) {
				case '#':
					printSquare(x * kX, (y + 1) * kY, kX, kY, transform, 1, 0, 0, 1, color);
					break;
				case '|':
					printSquare(x * kX, (y + 1) * kY, kX, kY, transform, .5, .3, 1, 1, color);
					break;
				case '@':
					//printSquare(x * kX - kX/6.0, (y + 1) * kY - kY/6.0, kX/3.0, kY/3.0, transform, 0, 1, 0, .7, color);
					printSquare(x * kX + kX/3.0, (y + 1) * kY - kY/3.0, kX/3.0, kY/3.0, transform, 0, 1, 0, .7, color);
					break;
				}
			}
		}

		//printSquare(rayCenterX * kX, rayCenterY * kY, 7, 7, transform, 0, 1, 0, 1, color);
		printSquare(playerX* kX, playerY*kY, 7, 7, transform, 0, 0, 1, 1, color);
		for (int i = 1, max = 25; i < max; i++) {
			printSquare((playerX) * kX + cos(playerA) * (float)i, (playerY) * kY + sin(playerA) * (float)i, 7.0*(1.0 - float(i)/max), 7.0*(1.0 - float(i)/max), transform, 0, 0, 1, 1, color);
		}
		while (!hasHit && distance < maxDistance) {
			if(bigStepRay(map, mapW, mapH, maxDistance, ray, curPoint, distance, kX, kY, transform, color) != 0) {
				switch (smallStepRay(map, mapW, mapH, distanceStep, maxDistance, ray, curPoint, distance)) {
				case '\0':
					break;
				case '|':
					rayA *= -1;
					ray = { cosf(rayA), sinf(rayA) };
					if (getTesting(map, curPoint.x + ray.x * distanceStep, curPoint.y + ray.y * distanceStep, mapW, mapH) == '|') {
						rayA += PI;
						ray = { cosf(rayA), sinf(rayA) };
					}
					break;
				default:
					printSquare(curPoint.x * kX, curPoint.y * kY, 7, 7, transform, 0, 0, 1, 1, color);
					hasHit = true;
				}
			}
		};
				

		glfwSwapBuffers(window);

		glfwPollEvents();
		Sleep(10);
	}

	glfwTerminate();
	return 0;
}