#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#define PI 3.14159265359
#define UP 0
#define DOWN 1
#define RIGHT 2
#define LEFT 3

struct Point {
	float x;
	float y;
	float r;
	float g;
	float b;
	float a;
};

struct Transform {
	float xPos;
	float yPos;
	float xScale;
	float yScale;
};

struct Vector {
	float x;
	float y;
};

struct Color {
	float r;
	float g;
	float b;
};

struct Particle {
	float x;
	float y;
	float alpha;
	float xFactor;
	float yFactor;
	float drag;
	Color color;
};

inline float distance(Transform a, Transform b) { return sqrt(pow(b.xPos - a.xPos, 2) + pow(b.yPos - a.yPos, 2) * 1); }

inline float lerp(float a, float b, float t) { return (1 - t) * a + t * b; }

#endif
