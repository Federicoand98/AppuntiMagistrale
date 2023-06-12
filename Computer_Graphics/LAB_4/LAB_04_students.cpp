#define _CRT_SECURE_NO_WARNINGS // for fscanf

#include <stdio.h>
#include <stdlib.h>
#include <GL/freeglut.h>
#include "argparser.h"
#include "glCanvas.h"
#include "mesh.h"
#include "radiosity.h"
#include "raytracer.h"

// =========================================
// =========================================

int
main(int argc, char* argv[])
{
	glutInit(&argc, argv);

	srand(0);
	ArgParser* args = new ArgParser(argc, argv);

	Mesh* mesh = new Mesh();
	mesh->Load(args->input_file, args);
	RayTracer* raytracer = new RayTracer(mesh, args);
	Radiosity* radiosity = new Radiosity(mesh, args);
	raytracer->setRadiosity(radiosity);
	radiosity->setRayTracer(raytracer);

	GLCanvas glcanvas;
	glcanvas.initialize(args, mesh, raytracer, radiosity);

	// well it never returns from the GLCanvas loop...
	delete args;
	return 0;
}
