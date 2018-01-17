#pragma once
#include <vector>
#include "mesh.h"
#include "triangle.h"
#include "ray.h"
#include "aabb.h"
// Bounding Volume Hierarchy (BVH) Interface.
class BVH {
	public:
		static const char METHOD_CUT_LONGEST_AXIS = 0;
		static const char METHOD_SAH = 2;
		BVH(void);
		BVH(char method);
		~BVH(void);
		// Constructs the BVH from the given mesh.
		void buildBVH(Mesh const &mesh);
		unsigned int const build(Mesh const &mesh, std::vector<unsigned int> &faceIDs, unsigned int &ind);
		std::vector<uint32_t> triangles;
		std::vector<uint32_t> nodes;
		std::vector<Vec3f> aabbs;
	private:
		void cutFaces(const Mesh &mesh, std::vector<unsigned int> &faceIDs, std::vector<unsigned int> &leftIDs, std::vector<unsigned int> &rightIDs, AABB &bb);
		void cutFacesLongestAxis(const Mesh &mesh, std::vector<unsigned int> &faceIDs, std::vector<unsigned int> &leftIDs, std::vector<unsigned int> &rightIDs, AABB &bb);
		void cutFacesMedianCut(const Mesh &mesh, std::vector<unsigned int> &faceIDs, std::vector<unsigned int> &leftIDs, std::vector<unsigned int> &rightIDs, AABB &bb);
		void cutFacesSAH(const Mesh &mesh, std::vector<unsigned int> &faceIDs, std::vector<unsigned int> &leftIDs, std::vector<unsigned int> &rightIDs, AABB &bb);
		char method;
};
inline BVH::BVH(void) : method(BVH::METHOD_CUT_LONGEST_AXIS) {}
inline BVH::BVH(char _method) : method(_method) {}
inline BVH::~BVH() {}
