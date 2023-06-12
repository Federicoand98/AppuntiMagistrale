#include "Curve.h"

Curve::Curve() : m_numTratti(0), m_VAO(0), m_VBO(0), m_CurvePoints(MAXNUMPOINTS) {
}

Curve::~Curve() {
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
	m_ControlPoints.clear();
}

void Curve::Init() {
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);
	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
}

void Curve::Draw(CurveType type) {
	if (m_ControlPoints.size() > 1) {
		if (type == DECASTELJAU) {

			for (int i = 0; i <= 100; i++) {
				Point res = deCasteljau(m_ControlPoints, (GLfloat)i / 100);
				m_CurvePoints[i] = res;
			}

			glDraw(m_VAO, m_VBO, m_CurvePoints.size() * 2 * sizeof(float), &m_CurvePoints[0], 101);
		} else if (type == SUBDIVISION) {
			m_numTratti = 0;

			adaptiveSubdivision(m_ControlPoints);

			glDraw(m_VAO, m_VBO, m_CurvePoints.size() * 2 * sizeof(float), &m_CurvePoints[0], m_numTratti + 1);
		} else if (type == CATMULLROM) {
			if (m_ControlPoints.size() > 3) {
				std::vector<Point> splinePoints = catmullRomSpline(m_ControlPoints, 20);

				glDraw(m_VAO, m_VBO, splinePoints.size() * 2 * sizeof(float), &splinePoints[0], splinePoints.size());
			}
		}
	}
}

void Curve::SetControlPoints(const std::vector<Point>& controlPoints) {
	m_ControlPoints.clear();
	m_ControlPoints = controlPoints;
}


Point Curve::Lerp(Point A, Point B, float t) {
	float x = (1 - t) * A.x + t * B.x;
	float y = (1 - t) * A.y + t * B.y;

	return { x, y };
}

float Curve::DistancePointToLine(Point p, Line l) {
	return abs(l.A * p.x + l.B * p.y + l.C) / sqrt(l.A * l.A + l.B * l.B);
}

Line Curve::CalculateLine(Point p1, Point p2) {
	float A = p1.y - p2.y;
	float B = p2.x - p1.x;
	float C = p1.x * p2.y - p2.x * p1.y;

	return { A, B, C };
}

Point Curve::deCasteljau(const std::vector<Point>& controlPoints, float t) {
	std::vector<Point> temp1;

	for (int i = 0; i < controlPoints.size(); i++) {
		temp1.push_back(controlPoints[i]);
	}

	for (int i = 1; i < controlPoints.size(); i++) {
		for (int j = 0; j < controlPoints.size() - i; j++) {
			temp1[j] = Lerp(temp1[j], temp1[j + 1], t);
		}
	}

	return temp1[0];
}

void Curve::deCasteljauAlt(const std::vector<Point>& controlPoints, std::vector<Point>& left, std::vector<Point>& right, float t) {
	std::vector<Point> temp;

	for (int i = 0; i < controlPoints.size(); i++) {
		temp.push_back(controlPoints[i]);
	}

	left[0] = controlPoints[0];
	right[controlPoints.size() - 1] = controlPoints[controlPoints.size() - 1];

	for (int i = 1; i < controlPoints.size(); i++) {
		for (int j = 0; j < controlPoints.size() - i; j++) {
			temp[j] = Lerp(temp[j], temp[j + 1], t);
		}

		left[i] = temp[0];
		right[controlPoints.size() - i - 1] = temp[controlPoints.size() - i - 1];
	}
}

void Curve::adaptiveSubdivision(const std::vector<Point>& controlPoints) {
	bool test = true;

	Line line = CalculateLine(controlPoints[0], controlPoints[controlPoints.size() - 1]);

	for (int i = 1; i < controlPoints.size() - 1 && test; i++) {
		float distance = DistancePointToLine(controlPoints[i], line);
		if (distance > PLANTEST)
			test = false;
	}

	if (test) {
		m_CurvePoints[m_numTratti] = controlPoints[0];
		m_CurvePoints[m_numTratti+ 1] = controlPoints[controlPoints.size() - 1];
		m_numTratti++;
	}
	else {
		std::vector<Point> leftPoints(controlPoints.size());
		std::vector<Point> rightPoints(controlPoints.size());

		deCasteljauAlt(controlPoints, leftPoints, rightPoints, 0.5);

		adaptiveSubdivision(leftPoints);
		adaptiveSubdivision(rightPoints);
	}
}

Point Curve::catmullRomPoint(Point p0, Point p1, Point p2, Point p3, float t) {
	float t2 = pow(t, 2);
	float t3 = pow(t, 3);

	float x = 0.5 * ((p0.x * (-t + 2.0 * t2 - t3)) + (p1.x * (2.0 - 5.0 * t2 + 3.0 * t3)) + (p2.x * (t + 4.0 * t2 - 3.0 * t3)) + (p3.x * (t3 - t2)));
	float y = 0.5 * ((p0.y * (-t + 2.0 * t2 - t3)) + (p1.y * (2.0 - 5.0 * t2 + 3.0 * t3)) + (p2.y * (t + 4.0 * t2 - 3.0 * t3)) + (p3.y * (t3 - t2)));

	return { x, y };
}

std::vector<Point> Curve::catmullRomSpline(const std::vector<Point>& controlPoints, int numPoints) {
	std::vector<Point> res;

	float tStep = 1.0 / numPoints;

	for (int i = 1; i < controlPoints.size() - 2; i++) {
		Point p0 = controlPoints[i - 1];
		Point p1 = controlPoints[i];
		Point p2 = controlPoints[i + 1];
		Point p3 = controlPoints[i + 2];

		for (int j = 0; j < numPoints; j++) {
			float t = j * tStep;
			Point point = catmullRomPoint(p0, p1, p2, p3, t);
			res.push_back(point);
		}
	}

	return res;
}

void Curve::glDraw(unsigned int vao, unsigned int vbo, int size, void* data, unsigned int count) {
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glPointSize(1.0);
	glLineWidth(2.5);
	glDrawArrays(GL_LINE_STRIP, 0, count);
	glBindVertexArray(0);
}
