#ifndef SAMPLER_H
#define SAMPLER_H

#include "vector3.h"


/*
 * This function detemines the best trace vectors within a plane for
 * any given vector as quickly as possible.
 */
void getTrace(Vector3 *normal, Vector3 *point, Vector3 *result) {
        // Write trivial vector trace, if possible
        if ((*normal).x != 0){
                Vector3 tmp = Vector3(dot(normal, point) / (*normal).x, 0, 0);
                if (isLinearlyIndependent(&tmp, normal)) {
                        (*result) = tmp;
                        return;
                }
        }

        if ((*normal).y != 0) {
                Vector3 tmp = Vector3(0, dot(normal, point) / (*normal).y, 0);
                if (isLinearlyIndependent(&tmp, normal)) {
                        (*result) = tmp;
                        return;
                }
        }

        if ((*normal).z != 0) {
                Vector3 tmp = Vector3(0, 0, dot(normal, point) / (*normal).z);
                if (isLinearlyIndependent(&tmp, normal)) {
                        (*result) = tmp;
                        return;
                }
        }

        // Write non-trivial trace
        Vector3 tmp = Vector3();
        for (size_t i = 1; isLinearlyDependent(&tmp, normal); ++i)
                if (i % 3 == 0 && (*normal).x != 0)
                        tmp = Vector3((dot(normal, point) - i * ((*normal).y + (*normal).z)) / (*normal).x, i, i);
                else if (i % 3 == 1 && (*normal).y != 0)
                        tmp = Vector3(i, (dot(normal, point) - i * ((*normal).x + (*normal).z)) / (*normal).y, i);
                else if (i % 3 == 2 && (*normal).z != 0)
                        tmp = Vector3(i, i, (dot(normal, point) - i * ((*normal).x + (*normal).y)) / (*normal).z);
        (*result) = tmp;
}

/*
 * Needed for Ambient Occlusion.
 * If a ray hits an object, we need to build a hemisphere on that object's plane.
 * All we get from intersecting an object with a ray is the intersection point ('point')
 * and the normal vector from the plane in question ('normal').
 * It is utterly important that the hemisphere is perpendicular to the object plane.
 * In order to get one (of two) perpendicular vectors to the normal vector,
 * please make use this function. This is automatically the best normal since
 * its trace can be determined the fastest.
*/
void getBestNormal(Vector3 *normal, Vector3 *point, Vector3 *normalResult) {
        Vector3 *result;
        getTrace(normal, point, result);
	(*result) = (*result) - (*point);
	(*result).normalize();
	(*normalResult) = (*result);
}

/*
 * Shortcut for getBestNormal (will even get the third normal).
*/
void getNormals(Vector3 *normal, Vector3 *point, Vector3 *firstResult, Vector3 *secondResult) {
	Vector3 tmp;
	getBestNormal(normal, point, &tmp);
	(*firstResult) = tmp;
	(*secondResult) = cross(&tmp, normal);
}

Vector3 getOrthogonalProjection(Vector3 normal, Vector3 point, Vector3 vec) {
        // Get coefficients
        float xi = dot(&normal, &point);
        float lambda = (2 * (xi - normal.x)) / dot(&normal, &normal);
        float my = (2 * (xi - normal.y)) / dot(&normal, &normal);
        float ny = (2 * (xi - normal.z)) / dot(&normal, &normal);

        // Get transformations
        Vector3 t1 = Vector3(normal.x + 1, normal.y, normal.z) * lambda;
        Vector3 t2 = Vector3(normal.x, normal.y + 1, normal.z) * my;
        Vector3 t3 = Vector3(normal.x, normal.y, normal.z + 1) * ny;

        return Vector3( t1.x * vec.x + t2.x * vec.y + t3.x * vec.z,
                        t1.y * vec.x + t2.y * vec.y + t3.y * vec.z,
                        t1.z * vec.x + t2.z * vec.y + t3.z * vec.z);
}

/*
 * Rotate vector 'destVec' by 'phi' degrees with respect to the vector that is given by x = point + LAMBDA * normal
 * After this operation, there's phi degrees between the normal and the third (unused) normal vector.
 */
Vector3 rotArb(Vector3 normal, Vector3 point, Vector3 destVec, float phi) {
	phi *= (2 * M_PI) / 360;
	//float x = destVec.x;
	//float y = destVec.y;
	//float z = destVec.z;

	//float a = point.x;
	//float b = point.y;
	//float c = point.z;

	//float u = normal.x - a;
	//float v = normal.y - b;
	//float w = normal.z - c;

	//float r = (a * (v * v + w * w) - u * (b * v + c * w - u * x - v * y - w * z)) * (1 - cos(phi)) + x * cos(phi) + (-c * v + b * w - w * y + v * z) * sin(phi);
	//float s = (b * (u * u + w * w) - v * (a * u + c * w - u * x - v * y - w * z)) * (1 - cos(phi)) + y * cos(phi) + (c * u - a * w + w * x - u * z) * sin(phi);
	//float t = (c * (u * u + v * v) - w * (a * u + b * v - u * x - v * y - w * z)) * (1 - cos(phi)) + z * cos(phi) + (-b * u + a * v - v * x + u * y) * sin(phi);
	//Vector3 res = Vector3(r, s, t);
	float x = destVec.x;
	float y = destVec.y;
	float z = destVec.z;

	float a = point.x;
	float b = point.y;
	float c = point.z;

	float u = normal.x;
	float v = normal.y;
	float w = normal.z;

	Vector3 res = Vector3((a * (v * v + w * w) - u * (b * v + c * w - u * x - v * y - w * z)) * (1 - cos(phi)) + x * cos(phi) + (-c * v + b * w - w * y + v * z) * sin(phi), (b * (u * u + w * w) - v * (a * u + c * w - u * x - v * y - w * z)) * (1 - cos(phi)) + y * cos(phi) + (c * u - a * w + w * x - u * z) * sin(phi), (c * (u * u + v * v) - w * (a * u + b * v - u * x - v * y - w * z)) * (1 - cos(phi)) + z * cos(phi) + (-b * u + a * v - v * x + u * y) * sin(phi));

	//if (dot(&res, &normal) < 0)
		//return rotArb(normal, point, getOrthogonalProjection(normal, point, destVec), phi);
	return res;
}

/*
 * Samples a ring with 'size' vectors around a normal which is given by its direction ('normal')
 * and the center of the hemisphere ('point').
 *
 * The height of the ring will be given by 'phi', where phi = 0 describes a ring at the bottom of
 * the hemisphere and phi = 1 describes a ring being the normal vector itself.
 *
 * 'size' describes the amound of vectors in the ring.
 */
void sampleRing(Vector3 normal, Vector3 point, float phi, size_t const size) {
	Vector3 ring[size];
	Vector3 res1;
	Vector3 res2;
	getNormals(&normal, &point, &res1, &res2);

	// Sampling information
	std::cout << "USED point: " << point << std::endl;
	std::cout << "USED normal: " << normal << std::endl;
	std::cout << "RESULTING normal 1: " << res1 << std::endl;
	std::cout << "RESULTING normal 2: " << res2 << std::endl;

	// First sample can be generated directly
	ring[0] = rotArb(res1, point, res2, phi);
	std::cout << std::endl << "R0: " << ring[0] << std::endl;

	// Rest of the samples depend on first sample
	for (size_t i = 1; i < size; ++i) {
		//getNormals(&normal, &point, &res1, &res2);
		//ring[i] = rotArb(normal, point, ring[i - 1], phi);
		ring[i] = rotArb(normal, point, ring[i - 1], 360 / size);

		//ring[i] = rotArb(ring[i - 1], point, normal, 360 - phi);
		//ring[i] = rotArb(res1, point, res2, (i + 1) * phi);
		std::cout << "R" << i << ": " << ring[i] << std::endl;
	}
}

#endif
