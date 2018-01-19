#ifndef M_PI
#define M_PI 3.14159265358979323846f
#define M_PI_2 1.57079632679489661923f
#endif
#define AO_METHOD_UNIFORM 0
#define AO_METHOD_RANDOM 1
typedef struct Intersection {
	uint faceID;
	float4 bary;
	float4 position;
	float distance;
} Intersection;
typedef struct HemisphereSampler {
	float4 basisX;
	float4 basisY;
	float4 basisZ;
	uint4 ran;
} HemisphereSampler;
// Smits ray-box intersection test using slabs
// http://www.cs.utah.edu/~awilliam/box/box.pdf
inline bool aabb_intersect(__global const float4 *bb, float4 raypos, float4 raydir, float maxDistance) {
	float tmin, tmax, tymin, tymax, tzmin, tzmax;
	float div = 1.0f / raydir.x;
	if (div >= 0) {
		tmin = (bb[0].x - raypos.x) * div;
		tmax = (bb[1].x - raypos.x) * div;
	}
	else {
		tmin = (bb[1].x - raypos.x) * div;
		tmax = (bb[0].x - raypos.x) * div;
	}
	div = 1 / raydir.y;
	if (div >= 0) {
		tymin = (bb[0].y - raypos.y) * div;
		tymax = (bb[1].y - raypos.y) * div;
	}
	else {
		tymin = (bb[1].y - raypos.y) * div;
		tymax = (bb[0].y - raypos.y) * div;
	}
	if (tmin > tymax || tymin > tmax) {
		return false;
	}
	tmin = max(tmin, tymin);
	tmax = min(tmax, tymax);
	div = 1 / raydir.z;
	if (div >= 0) {
		tzmin = (bb[0].z - raypos.z) * div;
		tzmax = (bb[1].z - raypos.z) * div;
	}
	else {
		tzmin = (bb[1].z - raypos.z) * div;
		tzmax = (bb[0].z - raypos.z) * div;
	}
	if (tmin > tzmax || tzmin > tmax) {
		return false;
	}
	tmin = max(tmin, tzmin);
	tmax = min(tmax, tzmax);
	return tmin < maxDistance && tmax > 0;
}
/*
* Src: http://geomalgorithms.com/a06-_intersect-2.html
*/
inline bool triangle_intersect(float4 ta, float4 tb, float4 tc, uint faceID, float4 raypos, float4 raydir, Intersection *intersection) {
	const float EPSILON2 = 0.000001f;
	// Get triangle edge vectors and plane normal
	const float4 u = tb - ta;
	const float4 v = tc - ta;
	const float4 n = cross(u, v);
	const float4 w0 = raypos - ta;
	const float a = -dot(n, w0);
	const float b = dot(n, raydir);
	// Check if ray is parallel to triangle plane.
	if (fabs(b) < EPSILON2) {
		return false;
	}
	// Get intersect point of ray with triangle plane
	const float r = a / b;
	if (r < 0.0) {
		// ray goes away from triangle
		return false;
	}
	// Intersect point of ray and plane
	const float4 intersectionPoint = raypos + r * raydir;
	// Is I inside T?
	const float uu = dot(u, u);
	const float uv = dot(u, v);
	const float vv = dot(v, v);
	const float4 w = intersectionPoint - ta;
	const float wu = dot(u, w);
	const float wv = dot(w, v);
	const float D = uv * uv - uu * vv;
	// Get and test parametric coordinates
	const float s = (uv * wv - vv * wu) / D;
	if (s < -0.00001f || s > 1.00001) {
		// I is outside T
		return false;
	}
	const float t = (uv * wu - uu * wv) / D;
	if (t < -0.00001f || (s + t) > 1.00001) {
		// I is outside T
		return false;
	}
	// Intersection looks good. Fill result.
	const float distance = length(intersectionPoint - raypos);
	if (intersection->distance > distance) {
		intersection->faceID = faceID;
		intersection->bary = (float4)(1.0f - s - t, s, t, 0);
		intersection->position = intersectionPoint;
		intersection->distance = distance;
	}
	return true;
}
inline float shade(const float4 raydir, const float4 normal) {
	return clamp(-dot(normal, raydir), 0.f, 1.f);
}
inline float4 get_smooth_normal(const __global uint *faces, const __global float4 *vertices, const __global float4 *normals, Intersection intersection) {
	const uint v0 = faces[intersection.faceID + 0];
	const uint v1 = faces[intersection.faceID + 1];
	const uint v2 = faces[intersection.faceID + 2];
	return normalize(
		normals[v0] * (float4)(intersection.bary.x) +
		normals[v1] * (float4)(intersection.bary.y) +
		normals[v2] * (float4)(intersection.bary.z)
	);
}
inline unsigned int random_int(uint4 *ran) {
	unsigned int t;
	t = (*ran).x ^ ((*ran).x << 11u);
	(*ran).x = (*ran).y;
	(*ran).y = (*ran).z;
	(*ran).z = (*ran).w;
	return (*ran).w = (*ran).w ^ ((*ran).w >> 19u) ^ (t ^ (t >> 8u));
}
inline void random_initialize_seed(uint4 *ran, uint seed) {
	(*ran).x = (123456789u ^ seed) * 88675123u;
	(*ran).y = (362436069u ^ seed) * 123456789u;
	(*ran).z = (521288629u ^ seed) * 362436069u;
	(*ran).w = (88675123u ^ seed) * 521288629u;
	random_int(ran);
}
inline void random_initialize(uint4 * ran, uint a, uint b, uint c, uint d) {
	(*ran).x = a;
	(*ran).y = b;
	(*ran).z = c;
	(*ran).w = d;
	random_int(ran);
}
inline float random_float(uint4 *ran) {
	return 2.32830643653869629E-10f * random_int(ran);
}
inline void hemisphere_sampler(HemisphereSampler *hemi, float4 normal, uint index) {
	hemi->basisY = normalize(normal);
	float4 h = hemi->basisY;
	if (fabs(h.x) <= fabs(h.y) && fabs(h.x) <= fabs(h.z)) {
		h.x = 1.0;
	}
	else if (fabs(h.y) <= fabs(h.x) && fabs(h.y) <= fabs(h.z)) {
		h.y = 1.0;
	}
	else if (fabs(h.z) <= fabs(h.x) && fabs(h.z) <= fabs(h.y)) {
		h.z = 1.0;
	}
	hemi->basisX = cross(h, hemi->basisY);
	hemi->basisX = normalize(hemi->basisX);
	hemi->basisZ = cross(hemi->basisX, hemi->basisY);
	hemi->basisZ = normalize(hemi->basisZ);
	random_initialize_seed(&hemi->ran, 536870923u * index);
}
inline float4 hemisphere_sampler_sample(HemisphereSampler *hemi) {
	/* use better random generator */
	float xi1 = random_float(&hemi->ran);
	float xi2 = random_float(&hemi->ran);
	float theta = acos(sqrt(1.0f - xi1));
	// removed PI here and changed sin to sinpi and cos to cospi
	float phi = 2.0 * xi2;
	float xs = sin(theta) * cospi(phi);
	float ys = cos(theta);
	float zs = sin(theta) * sinpi(phi);
	float4 direction = hemi->basisX * xs + hemi->basisY * ys + hemi->basisZ * zs;
	return normalize(direction);
}
inline bool scene_intersect(__global const uint *nodes, __global const float4 *aabbs, const __global uint *faces, const __global float4 *vertices, const __global float4 *normals, float4 raypos, float4 raydir, Intersection *intersection, float maxDistance) {
	bool isIntersecting = false;
	uint triIndex = 0;
	for (uint i = 0; i < nodes[0];) {
		uint const nodeCount = nodes[i];
		if (!aabb_intersect(aabbs + (i << 1), raypos, raydir, maxDistance)) {
			// Skip this node and all its children
			triIndex += (nodeCount + 1) >> 1;
			i += nodeCount;
		}
		else {
			if (nodeCount == 1) {
				// If node_count is 1, it's a leaf
				const uint faceID = triIndex * 3;
				isIntersecting |= triangle_intersect(
					vertices[faces[faceID + 0]],
					vertices[faces[faceID + 1]],
					vertices[faces[faceID + 2]],
					faceID,
					raypos,
					raydir,
					intersection
				);
				++triIndex;
			}
			++i;
		}
	}
	return isIntersecting;
}
inline float ambient_occlusion(__global const uint *nodes, __global const float4 *aabbs, const __global uint *faces, const __global float4 *vertices, const __global float4 *normals, float4 point, float4 normal, int index) {
	const float4 p = point + (normal * (1.0f / 100000.0f));
	uint hits = 0;
	float maxDistance = AO_MAX_DISTANCE;
#if AO_METHOD == AO_METHOD_UNIFORM
	uint n = 0;
	const uint circleCount = AO_NUM_SAMPLES;
	const float degrees = M_PI / 180;
	const float alpha_min = (float) AO_ALPHA_MIN * degrees;
	const float alpha_max = (float) AO_ALPHA_MAX * degrees; // don't change this
	const float4 basisY = normal; // already normalized
	const float4 h = basisY;
	if (fabs(h.x) <= fabs(h.y) && fabs(h.x) <= fabs(h.z)) {
		h.x = 1.0;
	}
	else if (fabs(h.y) <= fabs(h.x) && fabs(h.y) <= fabs(h.z)) {
		h.y = 1.0;
	}
	else if (fabs(h.z) <= fabs(h.x) && fabs(h.z) <= fabs(h.y)) {
		h.z = 1.0;
	}
	const float4 basisX = normalize(cross(h, basisY));
	const float4 basisZ = normalize(cross(basisX, basisY));
	for (uint currentCircle = 0; currentCircle < circleCount; ++currentCircle) {
		const float stepAngleRad = alpha_max / circleCount; // the angle of each step
		const float angleRad = (stepAngleRad * currentCircle) + alpha_min; // the "horizontal" angle
		const uint rayCount = (uint) ((2.0f * M_PI * cos(angleRad)) / stepAngleRad);
		const float theta = M_PI_2 - angleRad;
		for (uint currentRay = 0; currentRay <= rayCount; ++currentRay) {
			const float phi = (2.0f * M_PI * currentRay) / rayCount;
			const float xs = sin(theta) * cospi(phi);
			const float ys = cos(theta);
			const float zs = sin(theta) * sinpi(phi);
			const float4 raydir = basisX * xs + basisY * ys + basisZ * zs; // is normalized
			Intersection intersection;
			++n;
			if (scene_intersect(nodes, aabbs, faces, vertices, normals, p, raydir, &intersection, maxDistance)) {
				++hits;
			}
		}
	}
	return 1.0f - ((float) hits / (float) n);
#elif AO_METHOD == AO_METHOD_RANDOM
	HemisphereSampler hemi;
	hemisphere_sampler(&hemi, normal, index);
	uint n = AO_NUM_SAMPLES;
	// intersect normal
	Intersection intersection;
	++n;
	if (scene_intersect(nodes, aabbs, faces, vertices, normals, p, normal, &intersection, maxDistance)) {
		++hits;
	}
	for (uint i = 0; i < n; ++i) {
		const float4 raydir = hemisphere_sampler_sample(&hemi);
		Intersection intersection;
		if (!scene_intersect(nodes, aabbs, faces, vertices, normals, p, raydir, &intersection, maxDistance)) {
			continue;
		}
		++hits;
	}
	return 1.0f - ((float) hits / (float) n);
#endif
}
__kernel void intersect(__global const uint *faces, __global const uint *nodes, __global const float4 *aabbs, __global const float4 *vertices, __global const float4 *normals, __global float *image) {
	const uint x = get_global_id(0);
	const uint y = get_global_id(1);
	const uint index = y * WIDTH + x;
	const int2 pos = (int2) (x, y);
	// calculate the ray
	const float4 cameraPosition = (float4)(0.0f, 0.0f, 2.0f, 0.0f);
	const float a = FOCAL_LENGTH * max(WIDTH, HEIGHT);
	const float4 raydir = normalize((float4)(
		((float) x + 0.5f) / a - WIDTH / (2.0f * a),
		-(((float) y + 0.5f) / a - HEIGHT / (2.0f * a)),
		-1.0f,
		0.0f
	));
	const float maxDistance = 100000.0f;
	Intersection intersection;
	intersection.distance = INFINITY;
	bool isIntersecting = scene_intersect(nodes, aabbs, faces, vertices, normals, cameraPosition, raydir, &intersection, maxDistance);
	float value = 1.0f;
	if (!isIntersecting) {
		value = 0.0f;
	}
	else {
		const float4 normal = get_smooth_normal(faces, vertices, normals, intersection);
#ifdef SHADING_ENABLE
		value = shade(raydir, normal);
#endif
#if defined(AO_ENABLE) && AO_NUM_SAMPLES > 0
		value *= ambient_occlusion(nodes, aabbs, faces, vertices, normals, intersection.position, normal, index);
#endif
	}
	image[index] = value;
}