#include "HUD_Logger.h"
#include <GL/glew.h>
#include <GL/freeglut.h>


using namespace std;

extern float WindowWidth, WindowHeight;

HUD_Logger* HUD_Logger::logger;
bool HUD_Logger::ready;

void stroke_char_output(int x, int y, int z, std::string s, void * font)
{
	int i;
	glRasterPos3f(x, y, 0);
	for (i = 0; i < s.length(); i++)
		glutStrokeCharacter(font, s[i]);
}

HUD_Logger::HUD_Logger() {
}

HUD_Logger* HUD_Logger::get()
{
	if (!ready) {
		logger =  new HUD_Logger();
		ready = true;
	}
	return logger;
}

void HUD_Logger::printInfo(std::vector<std::string> lines)
{
	for (unsigned int i = 0; i < lines.size(); i++) {
		int total_width = 0, total_height = glutStrokeHeight(GLUT_STROKE_ROMAN);

		for (unsigned int j = 0; j < lines[i].length(); j++)
		{
			total_width += glutStrokeWidth(GLUT_STROKE_ROMAN, lines[i][j]);
		}
		glPushMatrix();
			glTranslatef(10.0, 10.0 + total_height * i * 0.2, 0.0);
			glScalef(0.2, 0.2, 1.0);
			glColor4f(0.0, 0.0, 0.0, 1.0);
			glLineWidth(3.0f);
			stroke_char_output(0.0, 0.0, 0.0, lines[i], GLUT_STROKE_ROMAN);
		glPopMatrix();
		glPushMatrix();
			glTranslatef(10.0, 10.0 + total_height * i * 0.2, 0.0);
			glScalef(0.2, 0.2, 1.0);
			glColor4f(1.0, 0.0, 0.0, 1.0);
			glLineWidth(1.0f);
			stroke_char_output(0.0, 0.0, 0.0, lines[i], GLUT_STROKE_ROMAN);
		glPopMatrix();
	}
}


