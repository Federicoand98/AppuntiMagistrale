/*
 * Lab-01_students.c
 *
 *     This program draws straight lines connecting dots placed with mouse clicks.
 *
 * Usage:
 *   Left click to place a control point.
 *		Maximum number of control points allowed is currently set at 64.
 *	 Press "f" to remove the first control point
 *	 Press "l" to remove the last control point.
 *	 Press escape to exit.
 */


#include <iostream>
#include "ShaderMaker.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include "Curve.h"

 // Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

/*
struct Line {
	float A;
	float B;
	float C;
};

struct Point {
	float x;
	float y;
};
*/

static unsigned int programId;

Curve curve;
CurveType type = SUBDIVISION;
static unsigned int curve_type = 2;

unsigned int VAO;
unsigned int VBO;

unsigned int VAO_2;
unsigned int VBO_2;

using namespace glm;

#define MaxNumPts 300

std::vector<Point> PointArray;
std::vector<Point> CurveArray(MaxNumPts);

int NumPts = 0;
double tol_planarita = 0.01;
int numero_tratti = 0;

// Window size in pixels
int		width = 500;
int		height = 500;

// Drag variables
float delta = 0.01f;
int idx = 0;
bool drag = false;

/* Prototypes */
void addNewPoint(float x, float y);
int main(int argc, char** argv);
void removeFirstPoint();
void removeLastPoint();

void myKeyboardFunc(unsigned char key, int x, int y) {
	switch (key) {
	case 'f':
		removeFirstPoint();
		glutPostRedisplay();
		break;
	case 'l':
		removeLastPoint();
		glutPostRedisplay();
		break;
	case 27:			// Escape key
		exit(0);
		break;
	}
}


void resizeWindow(int w, int h) {
	height = (h > 1) ? h : 2;
	width = (w > 1) ? w : 2;
	gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}

// Left button presses place a new control point.
void myMouseFunc(int button, int state, int x, int y) {
	// (x,y) viewport(0,width)x(0,height)   -->   (xPos,yPos) window(-1,1)x(-1,1)
	float xPos = -1.0f + ((float)x) * 2 / ((float)(width));
	float yPos = -1.0f + ((float)(height - y)) * 2 / ((float)(height));

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		bool found = false;

		for (int i = 0; i < NumPts && !found; i++) {
			if (xPos <= PointArray[i].x + delta && xPos >= PointArray[i].x - delta &&
				yPos <= PointArray[i].y + delta && yPos >= PointArray[i].y - delta) {
				found = true;
				drag = true;
				idx = i;
			}
		}

		if (!found) {
			addNewPoint(xPos, yPos);
			glutPostRedisplay();
		}
	}

	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP && drag) {
		PointArray[idx] = {xPos, yPos};

		idx = 0;
		drag = false;

		glutPostRedisplay();
	}
}

void dragMouseCallback(int x, int y) {
	if (drag) {
		float xPos = -1.0f + ((float)x) * 2 / ((float)(width));
		float yPos = -1.0f + ((float)(height - y)) * 2 / ((float)(height));

		PointArray[idx] = { xPos, yPos };

		glutPostRedisplay();
	}
}

void removeFirstPoint() {
	int i;
	if (NumPts > 0) {
		// Remove the first point, slide the rest down
		NumPts--;
		for (i = 0; i < NumPts; i++) {
			PointArray[i] = PointArray[i + 1];
		}
	}
}

// Add a new point to the end of the list.  
// Remove the first point in the list if too many points.
void removeLastPoint() {
	if (NumPts > 0) {
		PointArray.pop_back();
		NumPts--;
	}
}

// Add a new point to the end of the list.  
// Remove the first point in the list if too many points.
void addNewPoint(float x, float y) {
	if (NumPts >= MaxNumPts) {
		removeFirstPoint();
	}

	PointArray.push_back({ x, y });

	NumPts++;
}

void initShader(void) {
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"vertexShader.glsl";
	char* fragmentShader = (char*)"fragmentShader.glsl";

	programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(programId);

}

void init(void) {
	// VAO for control polygon
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// VAO for curve
	glGenVertexArrays(1, &VAO_2);
	glBindVertexArray(VAO_2);
	glGenBuffers(1, &VBO_2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_2);

	// Background color
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glViewport(0, 0, 500, 500);
}

Point lerp(Point p1, Point p2, float t) {
	Point res;

	res.x = (1 - t) * p1.x + t * p2.x;
	res.y = (1 - t) * p1.y + t * p2.y;

	return res;
}

float distancePointToLine(Point p, Line line) {
	return abs(line.A * p.x + line.B * p.y + line.C) / sqrt(line.A * line.A + line.B * line.B);
}

Line calculateLine(Point p1, Point p2) {
	Line line;
	line.A = p1.y - p2.y;
	line.B = p2.x - p1.x;
	line.C = p1.x * p2.y - p2.x * p1.y;

	return line;
}

Point deCasteljau(float t, std::vector<Point> controlPoints) {
	std::vector<Point> temp1;

	for (int i = 0; i < NumPts; i++) {
		temp1.push_back(controlPoints[i]);
	}

	for (int i = 1; i < NumPts; i++) {
		for (int j = 0; j < NumPts - i; j++) {
			temp1[j] = lerp(temp1[j], temp1[j + 1], t);
		}
	}

	return temp1[0];
}

void deCasteljauAlt(float t, std::vector<Point> controlPoints, std::vector<Point>& left, std::vector<Point>& right) {
	std::vector<Point> temp;

	for (int i = 0; i < controlPoints.size(); i++) {
		temp.push_back(controlPoints[i]);
	}

	left[0] = controlPoints[0];
	right[controlPoints.size() - 1] = controlPoints[controlPoints.size() - 1];

	for (int i = 1; i < controlPoints.size(); i++) {
		for (int j = 0; j < controlPoints.size() - i; j++) {
			temp[j] = lerp(temp[j], temp[j + 1], t);
		}

		left[i] = temp[0];
		right[controlPoints.size() - i - 1] = temp[controlPoints.size() - i - 1];
	}
}

void suddivisioneAdattiva(std::vector<Point>& controlPoints) {
	bool test = true;

	Line line = calculateLine(controlPoints[0], controlPoints[controlPoints.size() - 1]);

	for (int i = 1; i < controlPoints.size() - 1 && test; i++) {
		float distance = distancePointToLine(controlPoints[i], line);
		if (distance > tol_planarita)
			test = false;
	}

	if (test) {
		CurveArray[numero_tratti] = controlPoints[0];
		CurveArray[numero_tratti + 1] = controlPoints[controlPoints.size() - 1];
		numero_tratti++;
	}
	else {
		std::vector<Point> leftPoints(controlPoints.size());
		std::vector<Point> rightPoints(controlPoints.size());

		deCasteljauAlt(0.5, controlPoints, leftPoints, rightPoints);

		suddivisioneAdattiva(leftPoints);
		suddivisioneAdattiva(rightPoints);
	}
}

Point catmullRom(Point p0, Point p1, Point p2, Point p3, float t) {
	Point res;
	float t2 = t * t;
	float t3 = t2 * t;

	res.x = 0.5 * ((p0.x * (-t + 2.0 * t2 - t3)) + (p1.x * (2.0 - 5.0 * t2 + 3.0 * t3)) + (p2.x * (t + 4.0 * t2 - 3.0 * t3)) + (p3.x * (t3 - t2)));
	res.y = 0.5 * ((p0.y * (-t + 2.0 * t2 - t3)) + (p1.y * (2.0 - 5.0 * t2 + 3.0 * t3)) + (p2.y * (t + 4.0 * t2 - 3.0 * t3)) + (p3.y * (t3 - t2)));

	return res;
}

std::vector<Point> catmullRomSpline(std::vector<Point> controlPoints, int numPoints) {
	std::vector<Point> res;

	float tStep = 1.0 / numPoints;

	for (int i = 1; i < controlPoints.size() - 2; i++) {
		Point p0 = controlPoints[i - 1];
		Point p1 = controlPoints[i];
		Point p2 = controlPoints[i + 1];
		Point p3 = controlPoints[i + 2];

		for (int j = 0; j < numPoints; j++) {
			float t = j * tStep;
			Point point = catmullRom(p0, p1, p2, p3, t);
			res.push_back(point);
		}
	}

	return res;
}

void drawCurve(unsigned int vao, unsigned int vbo, int size, void* data, unsigned int count) {
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glPointSize(1.0);
	glLineWidth(2.5);
	glDrawArrays(GL_LINE_STRIP, 0, count);
	glBindVertexArray(0);
}

void drawScene(void) {
	glClear(GL_COLOR_BUFFER_BIT);

	if (NumPts > 1) {
		if (curve_type == DECASTELJAU) {

			for (int i = 0; i <= 100; i++) {
				CurveArray[i] = deCasteljau((GLfloat)i / 100, PointArray);
			}

			drawCurve(VAO_2, VBO_2, CurveArray.size() * 2 * sizeof(float), &CurveArray[0], 101);

		}
		else if (curve_type == SUBDIVISION) {
			numero_tratti = 0;

			suddivisioneAdattiva(PointArray);

			drawCurve(VAO_2, VBO_2, CurveArray.size() * 2 * sizeof(float), &CurveArray[0], numero_tratti + 1);

		}
		else if (curve_type == CATMULLROM) {
			if (NumPts > 3) {
				int numPoints = 20;
				std::vector<Point> splinePoints = catmullRomSpline(PointArray, numPoints);

				drawCurve(VAO_2, VBO_2, splinePoints.size() * 2 * sizeof(float), &splinePoints[0], splinePoints.size());
			}
		}
	}

	//curve.SetControlPoints(PointArray);
	//curve.Draw(type);

	if(PointArray.size() > 0) {
		// Draw control polygon
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, PointArray.size() * sizeof(float) * 2, &PointArray[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// Draw the control points CP
		glPointSize(6.0);
		glDrawArrays(GL_POINTS, 0, NumPts);
		// Draw the line segments between CP
		glLineWidth(1.0);
		glDrawArrays(GL_LINE_STRIP, 0, NumPts);
		glBindVertexArray(0);
	}

	glutSwapBuffers();
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);

	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Draw curves 2D");

	glutDisplayFunc(drawScene);
	glutReshapeFunc(resizeWindow);
	glutKeyboardFunc(myKeyboardFunc);
	glutMouseFunc(myMouseFunc);
	glutMotionFunc(dragMouseCallback);

	glewExperimental = GL_TRUE;
	glewInit();

	initShader();
	init();

	// è necessario inizializzare qui la curva per via della glewInit()
	curve.Init();

	glutMainLoop();
}
