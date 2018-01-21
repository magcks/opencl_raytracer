#ifndef M_PI
#define M_PI 3.14159265358979323846f
#define M_PI_2 1.57079632679489661923f
#endif
#define AO_METHOD_UNIFORM 0
#define AO_METHOD_RANDOM 1
typedef struct Intersection {
	uint face_id;
	float4 barycentric;
	float4 position;
	float distance;
} Intersection;
typedef struct HemisphereSampler {
	float4 basis_x;
	float4 basis_y;
	float4 basis_z;
	uint4 direction;
} HemisphereSampler;
// Smits ray-box intersection test using slabs
// http://www.cs.utah.edu/~awilliam/box/box.pdf
inline bool aabb_intersect(__global const float4 *bb, float4 ray_pos, float4 ray_dir, float max_distance) {
	float t_min, t_max, ty_min, ty_max, tz_min, tz_max;
	float div = 1.0f / ray_dir.x;
	if (div >= 0) {
		t_min = (bb[0].x - ray_pos.x) * div;
		t_max = (bb[1].x - ray_pos.x) * div;
	}
	else {
		t_min = (bb[1].x - ray_pos.x) * div;
		t_max = (bb[0].x - ray_pos.x) * div;
	}
	div = 1 / ray_dir.y;
	if (div >= 0) {
		ty_min = (bb[0].y - ray_pos.y) * div;
		ty_max = (bb[1].y - ray_pos.y) * div;
	}
	else {
		ty_min = (bb[1].y - ray_pos.y) * div;
		ty_max = (bb[0].y - ray_pos.y) * div;
	}
	if (t_min > ty_max || ty_min > t_max) {
		return false;
	}
	t_min = max(t_min, ty_min);
	t_max = min(t_max, ty_max);
	div = 1 / ray_dir.z;
	if (div >= 0) {
		tz_min = (bb[0].z - ray_pos.z) * div;
		tz_max = (bb[1].z - ray_pos.z) * div;
	}
	else {
		tz_min = (bb[1].z - ray_pos.z) * div;
		tz_max = (bb[0].z - ray_pos.z) * div;
	}
	if (t_min > tz_max || tz_min > t_max) {
		return false;
	}
	t_min = max(t_min, tz_min);
	t_max = min(t_max, tz_max);
	return t_min < max_distance && t_max > 0;
}
/*
* Src: http://geomalgorithms.com/a06-_intersect-2.html
*/
inline bool triangle_intersect(float4 ta, float4 tb, float4 tc, uint face_id, float4 ray_pos, float4 ray_dir, Intersection *intersection) {
	const float EPSILON2 = 0.000001f;
	// Get triangle edge vectors and plane normal
	const float4 u = tb - ta;
	const float4 v = tc - ta;
	const float4 n = cross(u, v);
	const float4 w0 = ray_pos - ta;
	const float a = -dot(n, w0);
	const float b = dot(n, ray_dir);
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
	const float4 intersection_point = ray_pos + r * ray_dir;
	// Is I inside T?
	const float uu = dot(u, u);
	const float uv = dot(u, v);
	const float vv = dot(v, v);
	const float4 w = intersection_point - ta;
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
	const float distance = length(intersection_point - ray_pos);
	if (intersection->distance > distance) {
		intersection->face_id = face_id;
		intersection->barycentric = (float4) (1.0f - s - t, s, t, 0);
		intersection->position = intersection_point;
		intersection->distance = distance;
	}
	return true;
}
inline float shade(const float4 ray_dir, const float4 normal) {
	return clamp(-dot(normal, ray_dir), 0.f, 1.f);
}
inline float4 get_smooth_normal(const __global uint *faces, const __global float4 *vertices, const __global float4 *normals, Intersection intersection) {
	const uint v0 = faces[intersection.face_id + 0];
	const uint v1 = faces[intersection.face_id + 1];
	const uint v2 = faces[intersection.face_id + 2];
	return normalize(
		normals[v0] * (float4) (intersection.barycentric.x) +
		normals[v1] * (float4) (intersection.barycentric.y) +
		normals[v2] * (float4) (intersection.barycentric.z)
	);
}
inline unsigned int random_int(uint4 *v) {
	unsigned int t;
	t = (*v).x ^ ((*v).x << 11u);
	(*v).x = (*v).y;
	(*v).y = (*v).z;
	(*v).z = (*v).w;
	return (*v).w = (*v).w ^ ((*v).w >> 19u) ^ (t ^ (t >> 8u));
}
inline void random_initialize_seed(uint4 *v, uint seed) {
	(*v).x = (123456789u ^ seed) * 88675123u;
	(*v).y = (362436069u ^ seed) * 123456789u;
	(*v).z = (521288629u ^ seed) * 362436069u;
	(*v).w = (88675123u ^ seed) * 521288629u;
	random_int(v);
}
inline void random_initialize(uint4 *v, uint a, uint b, uint c, uint d) {
	(*v).x = a;
	(*v).y = b;
	(*v).z = c;
	(*v).w = d;
	random_int(v);
}
inline float random_float(uint4 *v) {
	return 2.32830643653869629E-10f * random_int(v);
}
inline void hemisphere_sampler(HemisphereSampler *hemi, float4 normal, uint index) {
	hemi->basis_y = normalize(normal);
	float4 h = hemi->basis_y;
	if (fabs(h.x) <= fabs(h.y) && fabs(h.x) <= fabs(h.z)) {
		h.x = 1.0;
	}
	else if (fabs(h.y) <= fabs(h.x) && fabs(h.y) <= fabs(h.z)) {
		h.y = 1.0;
	}
	else if (fabs(h.z) <= fabs(h.x) && fabs(h.z) <= fabs(h.y)) {
		h.z = 1.0;
	}
	hemi->basis_x = cross(h, hemi->basis_y);
	hemi->basis_x = normalize(hemi->basis_x);
	hemi->basis_z = cross(hemi->basis_x, hemi->basis_y);
	hemi->basis_z = normalize(hemi->basis_z);
	random_initialize_seed(&hemi->direction, 536870923u * index);
}
inline float4 hemisphere_sampler_sample(HemisphereSampler *hemi) {
	/* use better random generator */
	float xi1 = random_float(&hemi->direction);
	float xi2 = random_float(&hemi->direction);
	float theta = acos(sqrt(1.0f - xi1));
	// removed PI here and changed sin to sinpi and cos to cospi
	float phi = 2.0 * xi2;
	float xs = sin(theta) * cospi(phi);
	float ys = cos(theta);
	float zs = sin(theta) * sinpi(phi);
	float4 direction = hemi->basis_x * xs + hemi->basis_y * ys + hemi->basis_z * zs;
	return normalize(direction);
}
inline bool scene_intersect(__global const uint *nodes, __global const float4 *aabbs, const __global uint *faces, const __global float4 *vertices, const __global float4 *normals, float4 ray_pos, float4 ray_dir, Intersection *intersection, float max_distance) {
	bool is_intersecting = false;
	uint triangle_idex = 0;
	for (uint i = 0; i < nodes[0];) {
		uint const node_count = nodes[i];
		if (!aabb_intersect(aabbs + (i << 1), ray_pos, ray_dir, max_distance)) {
			// Skip this node and all its children
			triangle_idex += (node_count + 1) >> 1;
			i += node_count;
		}
		else {
			if (node_count == 1) {
				// If node_count is 1, it's a leaf
				const uint face_id = triangle_idex * 3;
				is_intersecting |= triangle_intersect(
					vertices[faces[face_id + 0]],
					vertices[faces[face_id + 1]],
					vertices[faces[face_id + 2]],
					face_id,
					ray_pos,
					ray_dir,
					intersection
				);
				++triangle_idex;
			}
			++i;
		}
	}
	return is_intersecting;
}
inline float ambient_occlusion(__global const uint *nodes, __global const float4 *aabbs, const __global uint *faces, const __global float4 *vertices, const __global float4 *normals, float4 point, float4 normal, int index) {
	const float4 p = point + (normal * (1.0f / 100000.0f));
	uint hits = 0;
	float max_distance = AO_MAX_DISTANCE;
#if AO_METHOD == AO_METHOD_UNIFORM
	uint n = 0;
	const uint circle_count = AO_NUM_SAMPLES;
	const float degrees = M_PI / 180;
	const float alpha_min = (float) AO_ALPHA_MIN * degrees;
	const float alpha_max = (float) AO_ALPHA_MAX * degrees; // don't change this
	const float4 basis_y = normal; // already normalized
	const float4 h = basis_y;
	if (fabs(h.x) <= fabs(h.y) && fabs(h.x) <= fabs(h.z)) {
		h.x = 1.0;
	}
	else if (fabs(h.y) <= fabs(h.x) && fabs(h.y) <= fabs(h.z)) {
		h.y = 1.0;
	}
	else if (fabs(h.z) <= fabs(h.x) && fabs(h.z) <= fabs(h.y)) {
		h.z = 1.0;
	}
	const float4 basis_x = normalize(cross(h, basis_y));
	const float4 basis_z = normalize(cross(basis_x, basis_y));
	for (uint current_circle = 0; current_circle < circle_count; ++current_circle) {
		const float step_angle_radians = alpha_max / circle_count; // the angle of each step
		const float angle_radians = (step_angle_radians * current_circle) + alpha_min; // the "horizontal" angle
		const uint ray_count = (uint) ((2.0f * M_PI * cos(angle_radians)) / step_angle_radians);
		const float theta = M_PI_2 - angle_radians;
		for (uint current_ray = 0; current_ray <= ray_count; ++current_ray) {
			const float phi = (2.0f * M_PI * current_ray) / ray_count;
			const float xs = sin(theta) * cospi(phi);
			const float ys = cos(theta);
			const float zs = sin(theta) * sinpi(phi);
			// is normalized
			const float4 ray_dir = basis_x * xs + basis_y * ys + basis_z * zs;
			Intersection intersection;
			++n;
			if (scene_intersect(nodes, aabbs, faces, vertices, normals, p, ray_dir, &intersection, max_distance)) {
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
	if (scene_intersect(nodes, aabbs, faces, vertices, normals, p, normal, &intersection, max_distance)) {
		++hits;
	}
	for (uint i = 0; i < n; ++i) {
		const float4 ray_dir = hemisphere_sampler_sample(&hemi);
		Intersection intersection;
		if (!scene_intersect(nodes, aabbs, faces, vertices, normals, p, ray_dir, &intersection, max_distance)) {
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
	const float4 camera_position = (float4) (0.0f, 0.0f, 2.0f, 0.0f);
	const float a = FOCAL_LENGTH * max(WIDTH, HEIGHT);
	const float4 ray_dir = normalize((float4) (
		((float) x + 0.5f) / a - WIDTH / (2.0f * a),
		-(((float) y + 0.5f) / a - HEIGHT / (2.0f * a)),
		-1.0f,
		0.0f
	));
	const float max_distance = 100000.0f;
	Intersection intersection;
	intersection.distance = INFINITY;
	bool is_intersecting = scene_intersect(nodes, aabbs, faces, vertices, normals, camera_position, ray_dir, &intersection, max_distance);
	float value = 1.0f;
	if (!is_intersecting) {
		value = 0.0f;
	}
	else {
		const float4 normal = get_smooth_normal(faces, vertices, normals, intersection);
#ifdef SHADING_ENABLE
		value = shade(ray_dir, normal);
#endif
#if defined(AO_ENABLE) && AO_NUM_SAMPLES > 0
		value *= ambient_occlusion(nodes, aabbs, faces, vertices, normals, intersection.position, normal, index);
#endif
	}
	image[index] = value;
}