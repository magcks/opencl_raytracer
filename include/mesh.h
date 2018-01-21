#pragma once
#include <vector>
#include "vec3.h"
// Mesh data structure.
//
// A mesh contains a set of vertices, which are points in 3D. Triangles
// are formed by connecting three vertices, referenced by vertex IDs.
// Vertex IDs are given the faces list. The size of the faces list divided
// by three gives the number of faces in the mesh.
//
// Individual triangles are referenced by "face IDs". For example,
// face ID "f" references a triangle whose vertex IDs are located at
// positions "3 * f + 0", "3 * f + 1", "3 * f + 2" in the faces list.
struct Mesh {
	std::vector<Vec3f> vertices;
	std::vector<uint32_t> faces;
	std::vector<Vec3f> vnormals;
};
/* Loads a triangle mesh from an OFF model file. */
void load_off_mesh(const std::string &filename, Mesh *mesh);
/* Saves a triangle mesh to an OFF model file. */
void save_off_mesh(const Mesh &mesh, const std::string &filename);
/* Computes vertex normals for the triangle mesh. */
void compute_vertex_normals(Mesh *mesh);