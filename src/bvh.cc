#include <iomanip>
#include <iostream>
#include <limits>
#include "aabb.h"
#include "bvh.h"
#include "mesh.h"
#include "triangle.h"
/*
* Generates the bounding box and the bounding box of the centroids of the triangles.
*/
inline void getBBAndBBCentroid(const Mesh &mesh, std::vector<unsigned int> &faceIDs, AABB &bb, AABB &bbCentroid) {
	for (auto i = 0u; i < faceIDs.size(); ++i) {
		/* Get the triangle from the mesh */
		Triangle tri(&mesh, faceIDs[i]);
		/* Get the triangle's centroid and add it */
		Vec3f centroid = tri.getCentroid();
		/* to the BV (it might increase in size) */
		bbCentroid.merge(centroid);
		/* Merge the AABBs so the tree rebuilds */
		bb.merge(tri.getAABB());
	}
}
/*
* Generates the bounding box.
*/
inline void getBB(const Mesh &mesh, std::vector<unsigned int> &faceIDs, AABB &bb) {
	for (auto i = 0u; i < faceIDs.size(); ++i) {
		/* Get the triangle from the mesh */
		Triangle tri(&mesh, faceIDs[i]);
		/* Merge the AABBs so the tree rebuilds */
		bb.merge(tri.getAABB());
	}
}
/*
* Returns the surface area of a bounding box.
*/
inline float getSurfaceArea(AABB &bb) {
	float bbWidth  = bb.max[0] - bb.min[0];
	float bbHeight = bb.max[1] - bb.min[1];
	float bbDepth  = bb.max[2] - bb.min[2];
	return 2.0 * (bbWidth * bbHeight + bbHeight * bbDepth + bbDepth * bbWidth);
}
/*
* Cuts the face IDs into two pieces.
*/
void BVH::cutFaces(const Mesh &mesh, std::vector<unsigned int> &faceIDs, std::vector<unsigned int> &leftIDs, std::vector<unsigned int> &rightIDs, AABB &bb) {
	switch (method) {
		case BVH::Method::CUT_LONGEST_AXIS:
			cutFacesLongestAxis(mesh, faceIDs, leftIDs, rightIDs, bb);
			break;
		case BVH::Method::SURFACE_AREA_HEURISTIC:
			cutFacesSAH(mesh, faceIDs, leftIDs, rightIDs, bb);
			break;
	}
}
/*
* Cuts the face IDs along the longest axis.
*/
void BVH::cutFacesLongestAxis(const Mesh &mesh, std::vector<unsigned int> &faceIDs, std::vector<unsigned int> &leftIDs, std::vector<unsigned int> &rightIDs, AABB &bb) {
//	std::cout << "\rCutting..." << std::flush;
	/* The problem states that we need to create a bounding box of all the centroids as well. */
	AABB bbCentroid;
	getBBAndBBCentroid(mesh, faceIDs, bb, bbCentroid);
	/* Now we need to take care of the longest axis and its bisection. */
	/* Get the longest axis */
	int longestAxis = bbCentroid.getLongestAxis();
	//maxEntentPoint[longestAxis] = maxEntentPoint[longestAxis] - (maxEntentPoint[longestAxis] - bbCentroid.getAABBMin()[longestAxis]) / 2;
	// this is the same:
	bbCentroid.max[longestAxis] = (bbCentroid.max[longestAxis] + bbCentroid.min[longestAxis]) / 2;
	/* Set the centroid BV's new maximum extent */
	/* Since we need to split up the IDs into two lists, we'll just go ahead and create those here.*/
	for (auto i = 0u; i < faceIDs.size(); ++i) {
		/* Get the triangle from the mesh */
		Triangle tri(&mesh, faceIDs[i]);
		/* Get the current centroid */
		Vec3f centroid = tri.getCentroid();
		if (bbCentroid.inside(centroid)) {
			leftIDs.push_back(faceIDs[i]);
		}
		else {
			rightIDs.push_back(faceIDs[i]);
		}
	}
	// ensure left is not (I think this won't happen)
	if (leftIDs.size() == 0) {
		leftIDs.push_back(rightIDs[rightIDs.size() - 1]);
		rightIDs.pop_back();
	}
	// ensure right is not empty
	if (rightIDs.size() == 0) {
		rightIDs.push_back(leftIDs[leftIDs.size() - 1]);
		leftIDs.pop_back();
	}
}
/*
* The "bootstrap" for building the BVH.
*/
void BVH::buildBVH(const Mesh &mesh) {
	std::size_t size = mesh.faces.size() / 3;
	std::vector<uint32_t> faceIDs(size);
	for (auto i = 0u; i < size; ++i) {
		faceIDs[i] = i;
	}
	triangles.reserve(size);
	nodes = std::vector<uint32_t>(size * 2 - 1);
	aabbs = std::vector<Vec3f>((size * 2 - 1) * 2);
	std::size_t i = 0;
	build(mesh, faceIDs, i);
	nodes.resize(nodes[0]);
	aabbs.resize(nodes[0] * 2);
}
/*
* Builds an node of the BVH and returns the count of nodes (incl. the current node)
*/
unsigned int BVH::build(const Mesh &mesh, std::vector<unsigned int> &faceIDs, std::size_t &i) {
	uint32_t & node = nodes.at(i);
	AABB bb;
	if (faceIDs.size() <= /*MAX_LEAF_TRIANGLES*/1) {
// 		node.triangles_offset = triangles.size() * 3;
// 		node.triangles_length = faceIDs.size();
		/* Push all of our triangles into the node. */
		for (auto k = 0u; k < faceIDs.size(); ++k) {
			Triangle tri(&mesh, faceIDs[k]);    /* Get the triangle from the mesh */
			bb.merge(tri.getAABB());
			triangles.push_back(tri.getFaceID());
		}
		aabbs.at(i * 2) = bb.min;
		aabbs.at(i * 2 + 1) = bb.max;
		node = 1;
	}
	else {
		std::vector<unsigned int> leftIDs;
		leftIDs.reserve(faceIDs.size());
		std::vector<unsigned int> rightIDs;
		rightIDs.reserve(faceIDs.size());
		cutFaces(mesh, faceIDs, leftIDs, rightIDs, bb);
		if (leftIDs.size() == 0 && rightIDs.size() == 0) {
			std::cout << "ERROR: invalid cut left/right" << std::endl;
			std::exit(1);
		}
		if (leftIDs.size() == 0) {
			std::cout << "ERROR: invalid left cut" << std::endl;
			std::exit(1);
		}
		if (rightIDs.size() == 0) {
			std::cout << "ERROR: invalid right cut" << std::endl;
			std::exit(1);
		}
		aabbs.at(i * 2) = bb.min;
		aabbs.at(i * 2 + 1) = bb.max;
// 		node.triangles_offset = 0;
// 		node.triangles_length = 0;
		++i;
		node = build(mesh, leftIDs, i);
		leftIDs.clear();
		++i;
		node += build(mesh, rightIDs, i) + 1;
		rightIDs.clear();
	}
	//std::cout << nodes.at(i).node_count << std::endl;
	return node;
}
class sortByAxis {
	public:
		sortByAxis(const Mesh *mesh, std::size_t *axis) {
			this->mesh = mesh;
			this->axis = axis;
		}
		bool operator()(std::size_t i, std::size_t j) {
			Triangle tri1(mesh, i);
			Triangle tri2(mesh, j);
			return tri1.getCentroid()[*(axis)] > tri2.getCentroid()[*(axis)];
		}
	private:
		const Mesh *mesh;
		std::size_t *axis;
};
void BVH::cutFacesSAH(const Mesh &mesh, std::vector<unsigned int> &faceIDs, std::vector<unsigned int> &leftIDs, std::vector<unsigned int> &rightIDs, AABB &bb) {
	getBB(mesh, faceIDs, bb);
	// traversal_cost
	float cBV2 = 1.0;
	// intersection_cost
	float cObj = 1.0;
	// SA of the current node
	float SACurrent = getSurfaceArea(bb);
	float SALeft, SARight;
	std::size_t bestAxis = 0;
	auto bestPos = 1u;
	AABB emptyBB;
	float minCosts = std::numeric_limits<float>::max();
	float currentCosts;
	std::size_t countFaceIDs = faceIDs.size();
	for (std::size_t axis = 0; axis < 3; ++axis) {
		// And sort the array by the current axis
		std::sort(faceIDs.begin(), faceIDs.end(), sortByAxis(&mesh, &axis));
		AABB bbLeft = Triangle(&mesh, faceIDs[0]).getAABB(), bbRight;
		double countLeft, countRight;
		countLeft = 1;
		countRight = countFaceIDs - 1;
		// Iterate from the second to the next to last element, so that a subnode isn't empty
		for (auto i = 1u; i < countFaceIDs - 1; ++i) {
			std::cout << "\rCutting " << countFaceIDs << " face IDs into two parts... Processing (Axis: " << (int) axis << ") " << std::setw(5) << std::setfill('0') << std::left << (10000 * (i + axis * countFaceIDs) / (countFaceIDs * 3)) / 100.0f << "%..." << std::flush;
			// Compute right bounding box
			for (decltype(i) j = i; j < countFaceIDs; ++j) {
				bbRight.merge(Triangle(&mesh, faceIDs[j]).getAABB());
			}
			// Compute the surface area
			SALeft = getSurfaceArea(bbLeft);
			SARight = getSurfaceArea(bbRight);
			// Compute the costs
			currentCosts = cBV2 + (SALeft / SACurrent) * countLeft * cObj + (SARight / SACurrent) * countRight * cObj;
			// Check if this costs are the best costs we found
			if (currentCosts < minCosts) {
				minCosts = currentCosts;
				bestPos = i;
				bestAxis = axis;
			}
			// Push to left bb
			bbLeft.merge(Triangle(&mesh, faceIDs[i]).getAABB());
			// Clear right bb
			bbRight = emptyBB;
			// inc left, dec right
			++countLeft;
			--countRight;
		}
	}
// 	std::cout << std::endl;
// 	std::cout << "Best axis: " << (int) bestAxis << ", best pos: " << bestPos << std::endl;
	// Sort (again...) along the best axis
	// We don't have to sort if the best axis is '3', because we've sorted it in the last iteration already.
	if (bestAxis < 2)
		std::sort(faceIDs.begin(), faceIDs.end(), sortByAxis(&mesh, &bestAxis));
// std::cout << bestPos << std::endl;
	// Split
	leftIDs.assign(faceIDs.begin(), faceIDs.begin() + bestPos);
	rightIDs.assign(faceIDs.begin() + bestPos, faceIDs.end());
}