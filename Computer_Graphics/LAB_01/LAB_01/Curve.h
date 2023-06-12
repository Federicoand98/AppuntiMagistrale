#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include <GL/glew.h>

#define MAXNUMPOINTS 300
#define PLANTEST 0.001

struct Point {
	float x;
	float y;
};

struct Line {
	float A;
	float B;
	float C;
};

enum CurveType {
	DECASTELJAU,
	SUBDIVISION,
	CATMULLROM
};

class Curve {
public:
	Curve();
	~Curve();

	void SetControlPoints(const std::vector<Point>& controlPoints);

	void Init();

	void Draw(CurveType type);

	Point Lerp(Point A, Point B, float t);

	float DistancePointToLine(Point p, Line l);

	Line CalculateLine(Point p1, Point p2);
private:
	unsigned int m_VAO;
	unsigned int m_VBO;
	int m_numTratti;
	std::vector<Point> m_ControlPoints;
	std::vector<Point> m_CurvePoints;

	Point deCasteljau(const std::vector<Point>& controlPoints, float t);
	void deCasteljauAlt(const std::vector<Point>& controlPoints, std::vector<Point>& left, std::vector<Point>& right, float t);
	void adaptiveSubdivision(const std::vector<Point>& controlPoints);
	Point catmullRomPoint(Point p0, Point p1, Point p2, Point p3, float t);
	std::vector<Point> catmullRomSpline(const std::vector<Point>& controlPoints, int numPoints);

	void glDraw(unsigned int vao, unsigned int vbo, int size, void* data, unsigned int count);
};

