#include "raytracer.h"
#include "material.h"
#include "vectors.h"
#include "argparser.h"
#include "raytree.h"
#include "utils.h"
#include "mesh.h"
#include "face.h"
#include "sphere.h"

// casts a single ray through the scene geometry and finds the closest hit
bool RayTracer::CastRay (Ray & ray, Hit & h, bool use_sphere_patches) const {
	bool answer = false;
	Hit nearest;
	nearest = Hit();

	// intersect each of the quads
	for (int i = 0; i < mesh->numQuadFaces (); i++) {
		Face *f = mesh->getFace (i);

		if (f->intersect (ray, h, args->intersect_backfacing)) {
			if( h.getT() < nearest.getT() ) {
				answer = true;
				nearest = h;
			}
		}
	}

	// intersect each of the spheres (either the patches, or the original spheres)
	if (use_sphere_patches) {

		for (int i = mesh->numQuadFaces (); i < mesh->numFaces (); i++) {
			Face *f = mesh->getFace (i);
				
			if (f->intersect (ray, h, args->intersect_backfacing)) {
				if( h.getT() < nearest.getT() ) {
					answer = true;
					nearest = h;
				}
			}
		}
	} else {
		const vector < Sphere * >&spheres = mesh->getSpheres ();

		for (unsigned int i = 0; i < spheres.size (); i++) {
			if (spheres[i]->intersect (ray, h)) {
				if( h.getT() < nearest.getT() ) {
					answer = true;
					nearest = h;
				}
			}
		}
	}

	h = nearest;
	return answer;
}

Vec3f RayTracer::TraceRay (Ray & ray, Hit & hit, int bounce_count) const {
	hit = Hit ();
	bool intersect = CastRay (ray, hit, false);

	if( bounce_count == args->num_bounces )
		RayTree::SetMainSegment (ray, 0, hit.getT () );
	else
		RayTree::AddReflectedSegment(ray, 0, hit.getT() );

	Vec3f answer = args->background_color;

	Material *m = hit.getMaterial();

	if (intersect == true) {
		assert (m != NULL);
		Vec3f normal = hit.getNormal ();
		Vec3f point = ray.pointAtParameter (hit.getT ());

		// ----------------------------------------------
		// ambient light
		answer = args->ambient_light * m->getDiffuseColor ();

		// ----------------------------------------------
		// if the surface is shiny...
		Vec3f reflectiveColor = m->getReflectiveColor ();

		// ==========================================
		// REFLECTIVE LOGIC
		if(reflectiveColor.Length() != 0 && bounce_count > 0) {
			Vec3f VRay = ray.getDirection();
			Vec3f reflectionRay = VRay - (2 * VRay.Dot3(normal) * normal);
			reflectionRay.Normalize();
			Ray* new_ray = new Ray(point, reflectionRay);

			answer += TraceRay(*new_ray, hit, bounce_count - 1) * reflectiveColor;
		}

		int num_lights = mesh->getLights ().size ();

		// ==========================================
		// SHADOW LOGIC 
		Ray* n_ray;
		Hit* n_hit;
		bool colpito;
		Vec3f n_point, dista, pointOnLight, dirToLight;

		if(args->softShadow) {
			for(int i = 0; i < num_lights; i++) {
				Face* f = mesh->getLights()[i];

				std::vector<Vec3f> pointsOnLigh;
				std::vector<Vec3f> directionsToLight;

				// random points on area light
				for(int j = 0; j < 100; j++) {
					pointsOnLigh.push_back(f->RandomPoint());
					Vec3f dirToLight = pointsOnLigh.at(j) - point;
					dirToLight.Normalize();
					directionsToLight.push_back(dirToLight);
				}

				for(int j = 0; j < pointsOnLigh.size(); j++) {
					n_ray = new Ray(point, directionsToLight.at(j));
					n_hit = new Hit();
					colpito = CastRay(*n_ray, *n_hit, false);

					if(colpito) {
						n_point = n_ray->pointAtParameter(n_hit->getT());
						dista.Sub(dista, n_point, pointsOnLigh.at(j));

						if(dista.Length() < 0.01 && normal.Dot3(directionsToLight.at(j)) > 0) {
							Vec3f lightColor = 0.002 * f->getMaterial()->getEmittedColor() * f->getArea();
							answer += m->Shade(ray, hit, directionsToLight.at(j), lightColor, args);
						}
					}
				}
			}
		} else {
			// Contributo luminoso di ogni luce in scena
			for (int i = 0; i < num_lights; i++) {
				Face *f = mesh->getLights ()[i];
				// si tratta la area light come point light
				Vec3f pointOnLight = f->computeCentroid();
				Vec3f dirToLight = pointOnLight - point;
				dirToLight.Normalize ();

				// si casta il raggio dal punto toccato verso la sorgente luminosa
				n_ray = new Ray(point, dirToLight);
				n_hit = new Hit();
				colpito = CastRay(*n_ray, *n_hit, false);

				if(colpito) {
					// si ottiene il punto di collisione
					n_point = n_ray->pointAtParameter(n_hit->getT());

					//calcola disranza fra il punto colpito il punto sulla luce
					dista.Sub(dista, n_point, pointOnLight);
					if(dista.Length() < 0.01) {
						if(normal.Dot3(dirToLight) > 0) {
							// si applica il contributo della luce
							Vec3f lightColor = 0.2 * f->getMaterial()->getEmittedColor() * f->getArea();
							answer += m->Shade(ray, hit, dirToLight, lightColor, args);
						}
					}
				}
			}
		}
	}

	return answer;
}