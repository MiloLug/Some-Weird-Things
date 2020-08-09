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

float lightCenterX;
float lightCenterY;


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
		glBindBuffer(GL_ARRAY_BUFFER, arrBuf);
		glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), NULL, GL_STATIC_DRAW);
	}

	glBufferSubData(GL_ARRAY_BUFFER, 0, 8 * sizeof(float), pixelBlock);
	
	
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
		0, h, 0, (y * 2) / screenH,
		0, 0, 1, 0,
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


inline char getTesting(const char* map, float x, float y, int w, int h) {
	if (((y < h) + (y >= 0) + (x < w) + (x >= 0)) == 4)
		return map[(int)y * w + (int)x];
	
	return 0;
}

class Object {};

class Light : public Object{
public:
	float x;
	float y;
	bool checkRay(float x, float y, float &distance) {
		
	};
};


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
		if (tmpstr[i] == '#' || tmpstr[i] == '.' || tmpstr[i] == ' ' || tmpstr[i] == '|' || tmpstr[i] == '+' || tmpstr[i] == '@' || tmpstr[i] == 'P') {
			map_t += tmpstr[i];
			rc++;
		}
		if (tmpstr[i] == '\n') {
			mapW = rc;
			mapH++;
			rc = 0;
		}
	}

	lightCenterX = map_t.find("@");
	lightCenterY = (int)(lightCenterX / mapW) + 0.5;
	lightCenterX = (int)lightCenterX % mapW + 0.5;

	playerX = map_t.find("P");
	playerY = (int)(playerX / mapW) + 0.5;
	playerX = (int)playerX % mapW + 0.5;

	const char* map = map_t.c_str();
	mapTotal = map_t.length();
	
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

	glm::vec2 curPoint(0, 0);
	glm::vec2 ray(0, 0);
	float rayA;
	float startRayA;
	bool hasHit;
	float colorbit;
	float distance;
	float ceilY;
	float floorY;
	float lineHeight;

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
			glViewport(0, 0, swtmp, shtmp);
			genPixelBlock();
			sh1.use();
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

		//printSquare(0, screenH/2.0 - 100.0, 100, 100, transform, 1, 1, 1, 1, color);

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

		switch (getTesting(map, playerX, tmpPlayerY, mapW, mapH)) {
		case '#':
		case '|':
		case '+':
			break;
		default:
			playerY = tmpPlayerY;
		}

		switch (getTesting(map, tmpPlayerX, playerY, mapW, mapH)) {
		case '#':
		case '|':
		case '+':
			break;
		default:
			playerX = tmpPlayerX;
		}

		tmpPlayerX = 0;
		tmpPlayerY = 0;

		if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
			tmpPlayerX -= speedMult;
		}
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
			tmpPlayerX += speedMult;
		}
		if (GetAsyncKeyState(VK_UP) & 0x8000) {
			tmpPlayerY -= speedMult;
		}
		if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
			tmpPlayerY += speedMult;
		}
		
		tmpPlayerX += lightCenterX;
		tmpPlayerY += lightCenterY;

		switch (getTesting(map, lightCenterY, tmpPlayerY, mapW, mapH)) {
		case '\0':
			break;
		default:
			lightCenterY = tmpPlayerY;
		}

		switch (getTesting(map, tmpPlayerX, lightCenterY, mapW, mapH)) {
		case '\0':
			break;
		default:
			lightCenterX = tmpPlayerX;
		}

		//printSquare(rayCenterX * kX, rayCenterY * kY, 7, 7, transform, 0, 1, 0, 1, color);
		/*printSquare(playerX* kX, playerY*kY, 7, 7, transform, 0, 0, 1, 1, color);
		for (int i = 1, max = 25; i < max; i++) {
			printSquare((playerX) * kX + cos(playerA) * (float)i, (playerY) * kY + sin(playerA) * (float)i, 7.0*(1.0 - float(i)/max), 7.0*(1.0 - float(i)/max), transform, 0, 0, 1, 1, color);
		}*/



		bool corner;
		float prev = 0;
		float halfScreen = screenH / 2.0;

		for (int y = -halfScreen, cy = -horizonY; cy < halfScreen; y++, cy++) {
			colorbit = fmin(fmax(.9 - (cy) / halfScreen, 0), .8);
			printSquare(0, y, screenW, 1, transform, colorbit, colorbit, colorbit, 1, color);
		}

		for (int x = 0; x < screenW; x++) {
			startRayA = rayA = (playerA - POV / 2.0) + POV * ((float)x / screenW);
			ray = { cosf(rayA), sinf(rayA) };
			distance = 0;
			curPoint = { playerX, playerY };
			hasHit = false;
			corner = false;


			while (!hasHit && distance < maxDistance) {
				distance += distanceStep;

				curPoint += ray * distanceStep;
				switch (getTesting(map, curPoint.x, curPoint.y, mapW, mapH)) {
				case '+':
				case '|':
					rayA *= -1;
					ray = { cosf(rayA), sinf(rayA) };
					switch (getTesting(map, curPoint.x + ray.x * distanceStep, curPoint.y + ray.y * distanceStep, mapW, mapH)) {
						case '|':
							hasHit = true;
							break;
						case '+':
							rayA += PI;
							ray = { cosf(rayA), sinf(rayA) };
							break;
						default:
							ceilY = (float)(screenH / 2.0) - screenH / (distance * fabs(cos(playerA - startRayA)));
							floorY = screenH - ceilY;
							lineHeight = ceilY - floorY;
							printSquare(x, -lineHeight + horizonY, 1, lineHeight * 2, transform, 1, 1, 1, 0.1, color);
					}
					break;
				case '#':
					hasHit = true;
					break;
				}
			}

			distance *= fabs(cos(playerA - startRayA));
			
			if (hasHit) {
				hasHit = false;
				bool lightHasHit = false;
				float lightDistance = 0;
				rayA = atan2(lightCenterY - curPoint.y, lightCenterX - curPoint.x);
				curPoint -= ray * distanceStep * 2.0f;
				ray = { cosf(rayA), sinf(rayA) };

				while (!lightHasHit && lightDistance < maxDistance) {
					lightDistance += distanceStep;

					curPoint += ray * distanceStep;
					if ((int)curPoint.x == (int)lightCenterX && (int)curPoint.y == (int)lightCenterY) {
						lightHasHit = true;
					}
					else {
						switch (getTesting(map, curPoint.x, curPoint.y, mapW, mapH)) {
							/*case '@':
								lightHasHit = true;
								break;*/
						case '|':
						case '+':
						case '#':
							lightDistance += maxDistance;
							break;
						}
					}
				}


				if (x > 0 && fabs(prev - distance) > 0.5) {
					corner = true;
				}

				ceilY = (float)(screenH / 2.0) - screenH / distance;
				floorY = screenH - ceilY;

				lineHeight = ceilY - floorY;

				if (hasHit) {
					colorbit = .1;
				}
				else {
					//colorbit = corner ? .1 : 1.0 - (fmin(lightDistance, maxDistance-4) / maxDistance);
					colorbit = corner ? .1 : 1.0 -  (fmin(lightDistance, maxDistance - 4) / maxDistance);
				}

				printSquare(x, -lineHeight + horizonY, 1, lineHeight * 2, transform, colorbit, colorbit, colorbit, 1, color);
			}
			prev = distance;
		}

		float mapX = 3, mapY = 3;
		int mapMaxW = 50;
		int mapMaxH = 50;

		float boundX = fmax(fmin(playerX, mapW), 0);
		float boundY = fmax(fmin(playerY, mapH), 0);

		float boundXLight = fmax(fmin(lightCenterX, mapW), 0);
		float boundYLight = fmax(fmin(lightCenterY, mapH), 0);

		int mapStartX = fmax(0, fmin(boundX - mapMaxW / 2, mapW - mapMaxW));
		int mapStartY = fmax(0, fmin(boundY - mapMaxH / 2, mapH - mapMaxH));

		int mapEndX = fmin(mapStartX + mapMaxW, mapW);
		int mapEndY = fmin(mapStartY + mapMaxH, mapH);


		printSquare(mapX, -(mapMaxH - 1) * 3.0 - mapY + screenH / 2.0, 3 * mapMaxW, 3 * mapMaxH, transform, 0.0, 0.0, 0.0, 0.8, color);

		for (int y_m = mapStartY, y_r = 0; y_m < mapEndY; y_m++, y_r++) {
			for (int x_m = mapStartX, x_r = 0; x_m < mapEndX; x_m++, x_r++) {
				if (map[y_m * mapW + x_m] == '#') {
					printSquare(x_r * 3.0 + mapX, -y_r * 3.0 - mapY + screenH / 2.0, 3, 3, transform, 1, 1, 1, 1, color);
				}
			}
		}

		boundX -= mapStartX;
		boundY -= mapStartY;
		
		boundXLight -= mapStartX;
		boundYLight -= mapStartY;

		printSquare(boundXLight * 3.0 + mapX, screenH / 2.0 - (boundYLight * 3.0 + mapY + 3.5), 7, 7, transform, .3, .5, 1, 1, color);

		printSquare(boundX * 3.0 + mapX, screenH / 2.0 - (boundY * 3.0 + mapY + 3.5), 7, 7, transform, 1, .5, 1, 1, color);
		printSquare((boundX + 2.0 + cos(playerA)) * 3.0, screenH / 2.0 - ((boundY + .8 + sin(playerA)) * 3.0 + 1.0), 3, 3, transform, 0, 0, 1, 1, color);
		printSquare((boundX + 2.0 + cos(playerA) * 2.0) * 3.0, screenH / 2.0 - ((boundY + .8 + sin(playerA) * 2.0) * 3.0 + 1.0), 3, 3, transform, 0, 0, 1, 1, color);
		printSquare((boundX + 2.0 + cos(playerA) * 3.0) * 3.0, screenH / 2.0 - ((boundY + .8 + sin(playerA) * 3.0) * 3.0 + 1.0), 3, 3, transform, 0, 0, 1, 1, color);

		glfwSwapBuffers(window);

		glfwPollEvents();
		Sleep(10);
	}

	glfwTerminate();
	return 0;
}