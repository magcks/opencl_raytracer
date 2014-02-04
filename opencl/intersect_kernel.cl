// #pragma OPENCL EXTENSION cl_amd_printf : enable


typedef struct Mesh {
	const __global float4 * vertices;
	const __global uint * faces;
	const __global float4 * vnormals;
} Mesh;

typedef struct Triangle {
	Mesh const* mesh;
	uint faceID;
} Triangle;


typedef struct Ray {
	float4 position;
	float4 direction;
// 	float4 inverse;
// 	char4 sign;
} Ray;


typedef struct Intersection {
	Mesh const* mesh;
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
inline bool aabb_intersect(__global const float4 * bb, Ray * ray, float * maxDistance) {
	float tmin, tmax, tymin, tymax, tzmin, tzmax;
	float div = 1.0f / ray->direction.x;
	if (div >= 0) {
		tmin = (bb[0].x - ray->position.x) * div;
		tmax = (bb[1].x - ray->position.x) * div;
	}
	else {
		tmin = (bb[1].x - ray->position.x) * div;
		tmax = (bb[0].x - ray->position.x) * div;
	}

	div = 1 / ray->direction.y;
	if (div >= 0) {
		tymin = (bb[0].y - ray->position.y) * div;
		tymax = (bb[1].y - ray->position.y) * div;
	}
	else {
		tymin = (bb[1].y - ray->position.y) * div;
		tymax = (bb[0].y - ray->position.y) * div;
	}

	if (tmin > tymax || tymin > tmax)
		return false;

	tmin = max(tmin, tymin);
	tmax = min(tmax, tymax);

	div = 1 / ray->direction.z;
	if (div >= 0) {
		tzmin = (bb[0].z - ray->position.z) * div;
		tzmax = (bb[1].z - ray->position.z) * div;
	}
	else {
		tzmin = (bb[1].z - ray->position.z) * div;
		tzmax = (bb[0].z - ray->position.z) * div;
	}

	if (tmin > tzmax || tzmin > tmax)
		return false;

	tmin = max(tmin, tzmin);
	tmax = min(tmax, tzmax);

	//printf("AABB%f\n", tmin);

	return tmin < (*maxDistance) && tmax > 0.0f;
// this is slower:
// 	float tmin, tmax, tymin, tymax, tzmin, tzmax;
// 
// 	tmin = (bb[ray->sign.x].x - ray->position.x) * ray->inverse.x;
// 	tmax = (bb[1-ray->sign.x].x - ray->position.x) * ray->inverse.x;
// 	tymin = (bb[ray->sign.y].y - ray->position.y) * ray->inverse.y;
// 	tymax = (bb[1-ray->sign.y].y - ray->position.y) * ray->inverse.y;
// 	if ( (tmin > tymax) || (tymin > tmax) )
// 	return false;
// 	if (tymin > tmin)
// 	tmin = tymin;
// 	if (tymax < tmax)
// 	tmax = tymax;
// 	tzmin = (bb[ray->sign.z].z - ray->position.z) * ray->inverse.z;
// 	tzmax = (bb[1-ray->sign.z].z - ray->position.z) * ray->inverse.z;
// 	if ( (tmin > tzmax) || (tzmin > tmax) )
// 	return false;
// 	if (tzmin > tmin)
// 	tmin = tzmin;
// 	if (tzmax < tmax)
// 	tmax = tzmax;
// 	return ( (tmin < (*maxDistance)) && (tmax > 0.0f) );

}

inline float4 triangle_get_element(Triangle * tri, uint index) {
	return tri->mesh->vertices[tri->mesh->faces[tri->faceID + index]];
}

/*
 * Src: http://geomalgorithms.com/a06-_intersect-2.html
 */
inline bool triangle_intersect(Triangle * tri, Ray * ray, Intersection * intersection) {
	float const EPSILON2 = 0.000001;

	// Get triangle edge vectors and plane normal
	float4 u = triangle_get_element(tri, 1) - triangle_get_element(tri, 0);
	float4 v = triangle_get_element(tri, 2) - triangle_get_element(tri, 0);
	float4 n = cross(u, v);

	float4 w0 = ray->position - triangle_get_element(tri, 0);
	float a = -dot(n, w0);
	float b = dot(n, ray->direction);

	// Check if ray is parallel to triangle plane.
	if (fabs(b) < EPSILON2)
		return false;

	// Get intersect point of ray with triangle plane
	float r = a / b;
	if (r < 0.0) // ray goes away from triangle
		return false;

	// Intersect point of ray and plane
	float4 intersectionPoint = ray->position + r * ray->direction;

	// Is I inside T?
	float uu = dot(u, u);
	float uv = dot(u, v);
	float vv = dot(v, v);
	float4 w = intersectionPoint - triangle_get_element(tri, 0);
	float wu = dot(u, w);
	float wv = dot(w, v);
	float D = uv * uv - uu * vv;

	// Get and test parametric coords
	float s = (uv * wv - vv * wu) / D;
	if (s < -0.00001f || s > 1.00001)		// I is outside T
		return false;
	float t = (uv * wu - uu * wv) / D;
	if (t < -0.00001f || (s + t) > 1.00001)	// I is outside T
		return false;

	// Intersection looks good. Fill result.
	float const distance = length(intersectionPoint - ray->position);
	if (intersection->distance > distance) {
		intersection->mesh = tri->mesh;
		intersection->faceID = tri->faceID;
		intersection->bary = (float4)(1.0f - s - t, s, t, 0);
		intersection->position = intersectionPoint;
		intersection->distance = distance;
	}
	return true;
}

inline float shading(Ray * ray, float4 * normal) {
	return min(1.0f, fabs(dot((*normal), ray->direction)));
}

inline float4 get_smooth_normal(Intersection const * intersection) {
	uint v0 = intersection->mesh->faces[intersection->faceID + 0];
	uint v1 = intersection->mesh->faces[intersection->faceID + 1];
	uint v2 = intersection->mesh->faces[intersection->faceID + 2];
	return normalize(intersection->mesh->vnormals[v0] * (float4) (intersection->bary.x)
			+ intersection->mesh->vnormals[v1] * (float4) (intersection->bary.y)
			+ intersection->mesh->vnormals[v2] * (float4) (intersection->bary.z));
}

// RANDOM
inline unsigned int random_int(uint4 * ran) {
	unsigned int t;

	t = (*ran).x ^ ((*ran).x << 11u);
	(*ran).x = (*ran).y;
	(*ran).y = (*ran).z;
	(*ran).z = (*ran).w;

	return (*ran).w = (*ran).w ^ ((*ran).w >> 19u) ^ (t ^ (t >> 8u));
}
inline void random_initialize_seed(uint4 * ran, uint seed) {
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

inline float random_float(uint4 * ran) {
	return 2.32830643653869629E-10f * random_int(ran);
}

// HEMISPHERE SAMPLER
inline void hemisphere_sampler(HemisphereSampler * hemi, float4 normal, uint index) {
	hemi->basisY = normalize(normal);
	float4 h = hemi->basisY;

	if (fabs(h.x) <= fabs(h.y) && fabs(h.x) <= fabs(h.z))
		h.x = 1.0;
	else if (fabs(h.y) <= fabs(h.x) && fabs(h.y) <= fabs(h.z))
		h.y = 1.0;
	else
		h.z = 1.0;

	hemi->basisX = cross(h, hemi->basisY);
	hemi->basisX = normalize(hemi->basisX);
	hemi->basisZ = cross(hemi->basisX, hemi->basisY);
	hemi->basisZ = normalize(hemi->basisZ);

	random_initialize_seed(&hemi->ran, 536870923u * index);
}
inline float4 hemisphere_sampler_sample(HemisphereSampler * hemi) {
	/* use better random generator */
	float xi1 = random_float(&hemi->ran);
	float xi2 = random_float(&hemi->ran);

	float theta = acos(sqrt(1.0 - xi1));
	float phi = 2.0 * xi2; // removed PI here and changed sin to sinpi and cos to cospi

	float xs = sin(theta) * cospi(phi);
	float ys = cos(theta);
	float zs = sin(theta) * sinpi(phi);

	float4 direction = hemi->basisX * xs + hemi->basisY * ys + hemi->basisZ * zs;
	return normalize(direction);
}

inline bool scene_intersect(__global const uint * nodes, __global const float4 * aabbs, Mesh * mesh, Ray * ray, Intersection * intersection, float * max) {
	bool isIntersecting = false;
	uint triIndex = 0;
	for (uint i = 0; i < nodes[0];) {
		uint const nodeCount = nodes[i];
		if (!aabb_intersect(aabbs + (i << 1), ray, max)) {
			// Skip this node and all its children
			triIndex += (nodeCount + 1) >> 1;

			i += nodeCount;
		}
		else {
			if (nodeCount == 1) {
				// If node_count is 1 it's a leaf
				uint faceID = triIndex * 3;
				Triangle tri;
				tri.mesh = mesh;
				tri.faceID = faceID;
				isIntersecting |= triangle_intersect(&tri, ray, intersection);

				++triIndex;
			}

			++i;
		}
	}

	return isIntersecting;
}


#define AO_METHOD_PERFECT 0
#define AO_METHOD_RANDOM 1

// AMBIENT OCCULUTION
inline float ambient_occlusion(__global const uint * nodes, __global const float4 * aabbs, Mesh * mesh, float4 const point, float4 const normal, int const index) {
	float4 p = point + (normal * (1.0f / 100000.0f));

	uint hits = 0;
	float max = AO_MAXDISTANCE;

#if AO_METHOD == AO_METHOD_PERFECT
	uint n = 0;

	uint circleCount = AO_NUMSAMPLES;

	float degrees = M_PI / 180;

	float alpha_min = (float) AO_ALPHA_MIN * degrees;
	float alpha_max = (float) AO_ALPHA_MAX * degrees; // don't change this


	float4 basisY = normal; // already normalized
	float4 h = basisY;

	if (fabs(h.x) <= fabs(h.y) && fabs(h.x) <= fabs(h.z))
		h.x = 1.0;
	else if (fabs(h.y) <= fabs(h.x) && fabs(h.y) <= fabs(h.z))
		h.y = 1.0;
	else
		h.z = 1.0;

	float4 basisX = cross(h, basisY);
	basisX = normalize(basisX);
	float4 basisZ = cross(basisX, basisY);
	basisZ = normalize(basisZ);

	for (uint currentCircle = 0; currentCircle < circleCount; ++currentCircle) {
		float const stepAngleRad = alpha_max / circleCount; // the angle of each step

		float const angleRad = (stepAngleRad * currentCircle) + alpha_min; // the "horizontal" angle

		uint const rayCount = (uint) ((2.0f * M_PI * cos(angleRad)) / stepAngleRad);

		float const theta = M_PI_2 - angleRad;

		for (uint currentRay = 0; currentRay <= rayCount; ++currentRay) {
			float const phi = (2.0 * M_PI * currentRay) / rayCount;

			float xs = sin(theta) * cospi(phi);
			float ys = cos(theta);
			float zs = sin(theta) * sinpi(phi);

			Ray ray;
			ray.position = p;
			ray.direction = basisX * xs + basisY * ys + basisZ * zs; // is normalized

			Intersection intersection;

			++n;

			if (scene_intersect(nodes, aabbs, mesh, &ray, &intersection, &max))
				++hits;
		}
	}

	return 1.0f - ((float) hits / (float) n);
#elif AO_METHOD == AO_METHOD_RANDOM
	HemisphereSampler hemi;
	hemisphere_sampler(&hemi, normal, index);

	uint n = AO_NUMSAMPLES;

	// intersect normal
	Ray ray;
	ray.position = p;
	ray.direction = normal;

	Intersection intersection;

	++n;

	if (scene_intersect(nodes, aabbs, mesh, &ray, &intersection, &max))
		++hits;

	for (uint i = 0; i < n; ++i) {
		Ray ray;
		ray.position = p;
		ray.direction = hemisphere_sampler_sample(&hemi);
		// ray.direction = ray.direction.normalized(); // is already normalized

		Intersection intersection;
		if (!scene_intersect(nodes, aabbs, mesh, &ray, &intersection, &max))
			continue;

		++hits;
	}
	return 1.0f - ((float) hits / (float) n);
#endif

}


__kernel void intersect(__global const uint * faces, __global const uint * nodes, __global const float4 * aabbs, __global const float4 * vertices, __global const float4 * vnormals, __write_only image2d_t image) {
	uint x = get_global_id(0);
	uint y = get_global_id(1);

	uint index = y * WIDTH + x;
	const int2 pos = (int2)(x, y);

	// calculate the ray
	float4 cameraPosition = (float4)(0.0f, 0.0f, 2.0f, 0.0f);

	float a = FOCALLENGTH * max(WIDTH, HEIGHT);

	Ray ray;
	ray.position = cameraPosition;
	ray.direction = (float4)(
		((float) x + 0.5f) / a - WIDTH / (2.0f * a),
		-( ((float) y + 0.5f) / a - HEIGHT / (2.0f * a) ),
		-1.0f,
		0.0f);

	ray.direction = normalize(ray.direction);

	float maxDistance = 100000.0f;

	Mesh mesh;
	mesh.vertices = vertices;
	mesh.faces = faces;
	mesh.vnormals = vnormals;

	Intersection intersection;
	intersection.distance = INFINITY;

	bool isIntersecting = scene_intersect(nodes, aabbs, &mesh, &ray, &intersection, &maxDistance);

	float value;
	if (!isIntersecting) {
		value = 0.0f;
	}
	else {
		float4 normal = get_smooth_normal(&intersection);
#ifdef SHADING
		value = shading(&ray, &normal);
#else
		value = 1.0f;
#endif // SHADING

#ifdef AO
#if AO_NUMSAMPLES > 0
		value *= ambient_occlusion(nodes, aabbs, &mesh, intersection.position, normal, index);
#endif
#endif // AO
	}

	write_imagef(image, pos, (float4) (value, value, value, value));
}