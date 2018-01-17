
#include <cmath>
#include <algorithm>
#include <limits>
#include <iostream>
#include <vector>

#include "vec3.h"
#include "mesh.h"

class IntersectKernel {
public:
	static const char AO_METHOD_PERFECT = 0;
	static const char AO_METHOD_RANDOM = 1;


	struct Triangle {
		Mesh * mesh;
		unsigned int faceID;
	};


	struct Ray {
		Vec3f position;
		Vec3f direction;
	// 	Vec3f inverse;
	// 	char4 sign;
	};


	struct Intersection {
		Mesh * mesh;
		unsigned int faceID;
		Vec3f bary;
		Vec3f position;
		float distance;
	};

	struct HemisphereSampler {
		Vec3f basisX;
		Vec3f basisY;
		Vec3f basisZ;
		unsigned int ran [4];
	};


	unsigned int WIDTH;
	unsigned int HEIGHT;
	float FOCALLENGTH;
	unsigned int NSUPERSAMPLES;
	bool SHADING;
	bool AO;
	float AO_MAXDISTANCE;
	unsigned int AO_NUMSAMPLES;
	char AO_METHOD;
	int AO_ALPHA_MIN;
	int AO_ALPHA_MAX;

	IntersectKernel(unsigned int _WIDTH, unsigned int _HEIGHT, float _FOCALLENGTH, unsigned int _NSUPERSAMPLES, bool _SHADING, bool _AO, float _AO_MAXDISTANCE, unsigned int _AO_NUMSAMPLES, char _AO_METHOD, int _AO_ALPHA_MIN, int _AO_ALPHA_MAX) :
		WIDTH(_WIDTH),
		HEIGHT(_HEIGHT),
		FOCALLENGTH(_FOCALLENGTH),
		NSUPERSAMPLES(_NSUPERSAMPLES),
		SHADING(_SHADING),
		AO(_AO),
		AO_MAXDISTANCE(_AO_MAXDISTANCE),
		AO_NUMSAMPLES(_AO_NUMSAMPLES),
		AO_METHOD(_AO_METHOD),
		AO_ALPHA_MIN(_AO_ALPHA_MIN),
		AO_ALPHA_MAX(_AO_ALPHA_MAX) {
	}


	// Smits ray-box intersection test using slabs
	// http://www.cs.utah.edu/~awilliam/box/box.pdf
	inline bool aabb_intersect( Vec3f & bb0,  Vec3f & bb1, Ray & ray, float & maxDistance) {
		float tmin, tmax, tymin, tymax, tzmin, tzmax;
		float div = 1.0f / ray.direction[0];
		if (div >= 0) {
			tmin = (bb0[0] - ray.position[0]) * div;
			tmax = (bb1[0] - ray.position[0]) * div;
		}
		else {
			tmin = (bb1[0] - ray.position[0]) * div;
			tmax = (bb0[0] - ray.position[0]) * div;
		}

		div = 1 / ray.direction[1];
		if (div >= 0) {
			tymin = (bb0[1] - ray.position[1]) * div;
			tymax = (bb1[1] - ray.position[1]) * div;
		}
		else {
			tymin = (bb1[1] - ray.position[1]) * div;
			tymax = (bb0[1] - ray.position[1]) * div;
		}

		if (tmin > tymax || tymin > tmax)
			return false;

		tmin = std::max(tmin, tymin);
		tmax = std::min(tmax, tymax);

		div = 1 / ray.direction[2];
		if (div >= 0) {
			tzmin = (bb0[2] - ray.position[2]) * div;
			tzmax = (bb1[2] - ray.position[2]) * div;
		}
		else {
			tzmin = (bb1[2] - ray.position[2]) * div;
			tzmax = (bb0[2] - ray.position[2]) * div;
		}

		if (tmin > tzmax || tzmin > tmax)
			return false;

		tmin = std::max(tmin, tzmin);
		tmax = std::min(tmax, tzmax);

		//printf("AABB%f\n", tmin);

		return tmin < maxDistance && tmax > 0.0f;
	// this is slower:
	// 	float tmin, tmax, tymin, tymax, tzmin, tzmax;
	// 
	// 	tmin = (bb[ray.sign[0]][0] - ray.position[0]) * ray.inverse[0];
	// 	tmax = (bb[1-ray.sign[0]][0] - ray.position[0]) * ray.inverse[0];
	// 	tymin = (bb[ray.sign[1]][1] - ray.position[1]) * ray.inverse[1];
	// 	tymax = (bb[1-ray.sign[1]][1] - ray.position[1]) * ray.inverse[1];
	// 	if ( (tmin > tymax) || (tymin > tmax) )
	// 	return false;
	// 	if (tymin > tmin)
	// 	tmin = tymin;
	// 	if (tymax < tmax)
	// 	tmax = tymax;
	// 	tzmin = (bb[ray.sign[2]][2] - ray.position[2]) * ray.inverse[2];
	// 	tzmax = (bb[1-ray.sign[2]][2] - ray.position[2]) * ray.inverse[2];
	// 	if ( (tmin > tzmax) || (tzmin > tmax) )
	// 	return false;
	// 	if (tzmin > tmin)
	// 	tmin = tzmin;
	// 	if (tzmax < tmax)
	// 	tmax = tzmax;
	// 	return ( (tmin < (*maxDistance)) && (tmax > 0.0f) );

	}

	inline Vec3f triangle_get_element(Triangle * tri, unsigned int index) {
// 		std::cout << "begin3 " << tri.mesh->faces[tri.faceID + index] <<"|"<<tri.faceID + index << std::endl;
// 		if (tri.mesh->faces[tri.faceID + index] == 1061826143
		return tri->mesh->vertices[tri->mesh->faces[tri->faceID + index]];
	}

	/*
	* Src: http://geomalgorithms.com/a06-_intersect-2.html
	*/
	inline bool triangle_intersect(Triangle * tri, Ray & ray, Intersection & intersection) {
		float const EPSILON2 = 0.000001;

		// Get triangle edge vectors and plane normal
		Vec3f u = triangle_get_element(tri, 1) - triangle_get_element(tri, 0);
		Vec3f v = triangle_get_element(tri, 2) - triangle_get_element(tri, 0);
		Vec3f n = u.cross(v);

		Vec3f w0 = ray.position - triangle_get_element(tri, 0);
		float a = -n.dot(w0);
		float b = n.dot(ray.direction);

		// Check if ray is parallel to triangle plane.
		if (fabs(b) < EPSILON2)
			return false;

		// Get intersect point of ray with triangle plane
		float r = a / b;
		if (r < 0.0) // ray goes away from triangle
			return false;

		// Intersect point of ray and plane
		Vec3f intersectionPoint = ray.position + ray.direction * r;

		// Is I inside T?
		float uu = u.dot(u);
		float uv = u.dot(v);
		float vv = v.dot(v);
		Vec3f w = intersectionPoint - triangle_get_element(tri, 0);
		float wu = u.dot(w);
		float wv = w.dot(v);
		float D = uv * uv - uu * vv;

		// Get and test parametric coords
		float s = (uv * wv - vv * wu) / D;
		if (s < -0.00001f || s > 1.00001)		// I is outside T
			return false;
		float t = (uv * wu - uu * wv) / D;
		if (t < -0.00001f || (s + t) > 1.00001)	// I is outside T
			return false;

		// Intersection looks good. Fill result.
		float const distance = (intersectionPoint - ray.position).length();
		if (intersection.distance > distance) {
			intersection.mesh = tri->mesh;
			intersection.faceID = tri->faceID;
			intersection.bary = Vec3f(1.0f - s - t, s, t);
			intersection.position = intersectionPoint;
			intersection.distance = distance;
		}
		return true;
	}

	inline float shading(Ray & ray, Vec3f & normal) {
		return std::min(1.0f, std::abs(normal.dot(ray.direction)));
	}

	inline Vec3f get_smooth_normal(Intersection const * intersection) {
		unsigned int v0 = intersection->mesh->faces[intersection->faceID + 0];
		unsigned int v1 = intersection->mesh->faces[intersection->faceID + 1];
		unsigned int v2 = intersection->mesh->faces[intersection->faceID + 2];
		return (intersection->mesh->vnormals[v0] * (intersection->bary[0])
				+ intersection->mesh->vnormals[v1] * (intersection->bary[1])
				+ intersection->mesh->vnormals[v2] * (intersection->bary[2])).normalized();
	}

	// RANDOM
	inline unsigned int random_int(unsigned int * ran) {
		unsigned int t;

		t = ran[0] ^ (ran[0] << 11u);
		ran[0] = ran[1];
		ran[1] = ran[2];
		ran[2] = ran[3];

		return ran[3] = ran[3] ^ (ran[3] >> 19u) ^ (t ^ (t >> 8u));
	}
	inline void random_initialize_seed(unsigned int * ran, unsigned int seed) {
		ran[0] = (123456789u ^ seed) * 88675123u;
		ran[1] = (362436069u ^ seed) * 123456789u;
		ran[2] = (521288629u ^ seed) * 362436069u;
		ran[3] = (88675123u ^ seed) * 521288629u;

		random_int(ran);
	}
	inline void random_initialize(unsigned int * ran, unsigned int a, unsigned int b, unsigned int c, unsigned int d) {
		ran[0] = a;
		ran[1] = b;
		ran[2] = c;
		ran[3] = d;

		random_int(ran);
	}

	inline float random_float(unsigned int * ran) {
		return 2.32830643653869629E-10f * random_int(ran);
	}

	// HEMISPHERE SAMPLER
	inline void hemisphere_sampler(HemisphereSampler & hemi, Vec3f normal, unsigned int index) {
		hemi.basisY = normal.normalized();
		Vec3f h = hemi.basisY;

		if (fabs(h[0]) <= fabs(h[1]) && fabs(h[0]) <= fabs(h[2]))
			h[0] = 1.0;
		else if (fabs(h[1]) <= fabs(h[0]) && fabs(h[1]) <= fabs(h[2]))
			h[1] = 1.0;
		else
			h[2] = 1.0;

		hemi.basisX = h.cross(hemi.basisY);
		hemi.basisX = hemi.basisX.normalized();
		hemi.basisZ = hemi.basisX.cross(hemi.basisY);
		hemi.basisZ = hemi.basisZ.normalized();

		random_initialize_seed(hemi.ran, 536870923u * index);
	}
	inline Vec3f hemisphere_sampler_sample(HemisphereSampler & hemi) {
		/* use better random generator */
		float xi1 = random_float(hemi.ran);
		float xi2 = random_float(hemi.ran);

		float theta = acos(sqrt(1.0 - xi1));
		float phi = 2.0 * xi2 * M_PI;

		float xs = sin(theta) * cos(phi);
		float ys = cos(theta);
		float zs = sin(theta) * sin(phi);

		Vec3f direction = hemi.basisX * xs + hemi.basisY * ys + hemi.basisZ * zs;
		return direction.normalized();
	}

	inline bool scene_intersect(std::vector<unsigned int> &nodes, std::vector<Vec3f> &aabbs, Mesh & mesh, Ray & ray, Intersection & intersection, float & max) {
		bool isIntersecting = false;
		unsigned int triIndex = 0;
		for (unsigned int i = 0; i < nodes[0];) {
			unsigned int const nodeCount = nodes[i];
			if (!aabb_intersect(aabbs[i << 1], aabbs[(i << 1) + 1], ray, max)) {
				// Skip this node and all its children
				triIndex += (nodeCount + 1) >> 1;

				i += nodeCount;
			}
			else {
				if (nodeCount == 1) {
					// If node_count is 1 it's a leaf
					unsigned int faceID = triIndex * 3;
					Triangle tri;
					tri.mesh = &mesh;
					tri.faceID = faceID;
					isIntersecting |= triangle_intersect(&tri, ray, intersection);

					++triIndex;
				}

				++i;
			}
		}

		return isIntersecting;
	}



	// AMBIENT OCCULUTION
	inline float ambient_occlusion(std::vector<unsigned int> &nodes, std::vector<Vec3f> &aabbs, Mesh & mesh, Vec3f const point, Vec3f const normal, int const index) {
		Vec3f p = point + (normal * (1.0f / 100000.0f));

		unsigned int hits = 0;
		float max = AO_MAXDISTANCE;

		if (AO_METHOD == AO_METHOD_PERFECT) {
			unsigned int n = 0;

			unsigned int circleCount = AO_NUMSAMPLES;

			float degrees = M_PI / 180;

			float alpha_min = (float) AO_ALPHA_MIN * degrees;
			float alpha_max = (float) AO_ALPHA_MAX * degrees; // don't change this


			Vec3f basisY = normal; // already normalized
			Vec3f h = basisY;

			if (fabs(h[0]) <= fabs(h[1]) && fabs(h[0]) <= fabs(h[2]))
				h[0] = 1.0;
			else if (fabs(h[1]) <= fabs(h[0]) && fabs(h[1]) <= fabs(h[2]))
				h[1] = 1.0;
			else
				h[2] = 1.0;

			Vec3f basisX = h.cross(basisY);
			basisX = basisX.normalized();
			Vec3f basisZ = basisX.cross(basisY);
			basisZ = basisZ.normalized();

			for (unsigned int currentCircle = 0; currentCircle < circleCount; ++currentCircle) {
				float const stepAngleRad = alpha_max / circleCount; // the angle of each step

				float const angleRad = (stepAngleRad * currentCircle) + alpha_min; // the "horizontal" angle

				unsigned int const rayCount = (unsigned int) ((2.0f * M_PI * cos(angleRad)) / stepAngleRad);

				float const theta = M_PI_2 - angleRad;

				for (unsigned int currentRay = 0; currentRay <= rayCount; ++currentRay) {
					float const phi = ((2.0 * M_PI * currentRay) / rayCount) * M_PI;

					float xs = sin(theta) * cos(phi);
					float ys = cos(theta);
					float zs = sin(theta) * sin(phi);

					Ray ray;
					ray.position = p;
					ray.direction = basisX * xs + basisY * ys + basisZ * zs; // is normalized

					Intersection intersection;

					++n;

					if (scene_intersect(nodes, aabbs, mesh, ray, intersection, max))
						++hits;
				}
			}

			return 1.0f - ((float) hits / (float) n);
		}
		else if (AO_METHOD == AO_METHOD_RANDOM) {
			HemisphereSampler hemi;
			hemisphere_sampler(hemi, normal, index);

			unsigned int n = AO_NUMSAMPLES;

			// intersect normal
			Ray ray;
			ray.position = p;
			ray.direction = normal;

			Intersection intersection;

			++n;

			if (scene_intersect(nodes, aabbs, mesh, ray, intersection, max))
				++hits;

			for (unsigned int i = 0; i < n; ++i) {
				Ray ray;
				ray.position = p;
				ray.direction = hemisphere_sampler_sample(hemi);
				// ray.direction = ray.direction.normalized(); // is already normalized

				Intersection intersection;
				if (!scene_intersect(nodes, aabbs, mesh, ray, intersection, max))
					continue;

				++hits;
			}
			return 1.0f - ((float) hits / (float) n);
		}
		return 0.0f;
	}

	void intersect(const unsigned int x, const unsigned int y, Mesh & mesh, std::vector<unsigned int> &nodes, std::vector<Vec3f> &aabbs, float * tmp) {


		unsigned int ssX = 0;
		unsigned int ssY = 0;
		unsigned int n = 1;

		unsigned int index = y * WIDTH + x;

		// calculate the ray
		Vec3f cameraPosition = Vec3f(0.0f, 0.0f, 2.0f);

		float a = FOCALLENGTH * std::max(WIDTH, HEIGHT);

		Ray ray;
		ray.position = cameraPosition;
		ray.direction = Vec3f(
			((float) x + ((0.5f + (float) ssX) / (float) n)) / a - WIDTH / (2.0f * a),
			-( ((float) y + ((0.5f + (float) ssY) / (float) n)) / a - HEIGHT / (2.0f * a) ),
			-1.0f);

		ray.direction = ray.direction.normalized();

		float maxDistance = 100000.0f;

// 		Mesh mesh;
// 		mesh.vertices = vertices;
// 		mesh.faces = faces;
// 		mesh.vnormals = vnormals;

		Intersection intersection;
		intersection.distance = std::numeric_limits<float>::max();

		bool isIntersecting = scene_intersect(nodes, aabbs, mesh, ray, intersection, maxDistance);

		float value;
		if (!isIntersecting) {
			value = 0.0f;
		}
		else {
			Vec3f normal = get_smooth_normal(&intersection);

			if (SHADING)
				value = shading(ray, normal);
			else
				value = 1.0f;

// 		std::cout << "AO" << AO << ":"<<AO_NUMSAMPLES << std::endl;
			if (AO && AO_NUMSAMPLES > 0)
				value *= ambient_occlusion(nodes, aabbs, mesh, intersection.position, normal, index);
		}

// 		std::cout << x << ":" << y << ":" << index << ":" << WIDTH * HEIGHT << std::endl;
		tmp[index] = value;
// 		std::cout << "done" << std::endl;
	}

	void iterate(Mesh & mesh, std::vector<unsigned int> &nodes, std::vector<Vec3f> &aabbs, float * tmp) {
// 		std::cout << WIDTH << "|" << HEIGHT << "|" << FOCALLENGTH << "|" << NSUPERSAMPLES << "|" << SHADING << "|" << AO << "|" << AO_MAXDISTANCE << "|" << AO_NUMSAMPLES << "|" << (int) AO_METHOD  << "|" << AO_ALPHA_MIN  << "|" << AO_ALPHA_MAX << "|" << std::endl;

// 		#pragma omp parallel for
// 		for (unsigned int t = 0; t < 4; ++t) {
// 		}

// 		#pragma omp parallel
// { 
		#pragma omp parallel for
		for (unsigned int y = 0; y < HEIGHT; ++y) {
// 	std::cout << "THIS:" << 0 << "|" << y << "|" << faces[31398] << std::endl;
			for (unsigned int x = 0; x < WIDTH; ++x) {
				intersect(x, y, mesh, nodes, aabbs, tmp);
// 		std::cout << "done2" << std::endl;
			}
		}
// }
	}
};
