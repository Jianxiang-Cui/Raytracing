#include <ctime>
#include <vector>
#include "defs.h"
#include "Utilities.h"
#include "FrameBuffer.h"
#include "ColorAndMaterials.h"
#include "Rasterization.h"

FrameBuffer frameBuffer(WINDOW_WIDTH, WINDOW_HEIGHT);

void closed5x5Square(int x, int y, color C) {
	drawLine(frameBuffer, x - 2, y, x + 2, y, C);
	drawLine(frameBuffer, x - 2, y + 2, x + 2, y + 2, C);
	drawLine(frameBuffer, x - 2, y + 1, x + 2, y + 1, C);
	drawLine(frameBuffer, x - 2, y - 1, x + 2, y - 1, C);
	drawLine(frameBuffer, x - 2, y - 2, x + 2, y - 2, C);
}

void closed5x5Square(const glm::vec2 &centerPt, color C) {
	closed5x5Square(centerPt.x, centerPt.y, C);
}

void open5x5Square(const glm::vec2 &centerPt, color C) {
	drawLine(frameBuffer, centerPt.x - 2, centerPt.y - 2, centerPt.x + 2, centerPt.y - 2, C);
	drawLine(frameBuffer, centerPt.x - 2, centerPt.y - 2, centerPt.x - 2, centerPt.y + 2, C);
	drawLine(frameBuffer, centerPt.x + 2, centerPt.y - 2, centerPt.x + 2, centerPt.y + 2, C);
	drawLine(frameBuffer, centerPt.x + 2, centerPt.y + 2, centerPt.x - 2, centerPt.y + 2, C);
}

void pieChart(const glm::vec2 &centerPt, float rad, float perc, const color &C1, const color &C2) {
	drawArc(frameBuffer, centerPt, rad, M_PI_4, 3 * M_PI_2, C1);
	drawLine(frameBuffer, centerPt.x, centerPt.y, centerPt.x + rad * std::cos(M_PI_4), centerPt.y + rad * std::sin(M_PI_4), C1);
	drawLine(frameBuffer, centerPt.x, centerPt.y, centerPt.x + rad * std::cos(M_PI_4), centerPt.y - rad * std::sin(M_PI_4), C1);
	drawLine(frameBuffer, centerPt.x + 20, centerPt.y, centerPt.x + 20 + rad * std::cos(M_PI_4), centerPt.y + rad * std::sin(M_PI_4), C2);
	drawLine(frameBuffer, centerPt.x + 20, centerPt.y, centerPt.x + 20 + rad * std::cos(M_PI_4), centerPt.y - rad * std::sin(M_PI_4), C2);
	drawArc(frameBuffer, centerPt + glm::vec2(20, 0), rad, -M_PI_4, M_PI_2, C2);
}

void render() {
	frameBuffer.clearColorAndDepthBuffers();
	closed5x5Square(50, 50, red);
	closed5x5Square(glm::vec2(100, 50), green);
	open5x5Square(glm::vec2(150, 50), blue);
	pieChart(glm::vec2(250, 100), 50, 0.25, red, green);
	frameBuffer.showColorBuffer();
}

void resize(int width, int height) {
	frameBuffer.setFrameBufferSize(width, height);
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case ESCAPE:	glutLeaveMainLoop();
		break;
	}
}

int main(int argc, char *argv[]) {
	
	glm::vec2 test1(-4,2.5);
	glm::vec3 test2(1,1,1);
	std::cout << doubleIt(test1) << std::endl;
	

	return 0;
}