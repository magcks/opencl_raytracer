#include <iostream>
#include <limits>
#include <stdexcept>
#include <list>
#include <iomanip>
#include "bvh.h"
#include "aabb.h"
#include "triangle.h"
/*
* Generates the bounding box and the bounding box of the centroids of the triangles.
*/
inline void getBBAndBBCentroid(Mesh const &mesh, std::vector<unsigned int> &faceIDs, AABB &bb, AABB &bbCentroid) {
	for (size_t i = 0; i < faceIDs.size(); ++i) {
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
inline void getBB(Mesh const &mesh, std::vector<unsigned int> &faceIDs, AABB &bb) {
	for (size_t i = 0; i < faceIDs.size(); ++i) {
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
* Cuts the face IDs into two pieces. The method to cut it is specified by the "this->method"-char.
*/
void BVH::cutFaces(const Mesh &mesh, std::vector<unsigned int> &faceIDs, std::vector<unsigned int> &leftIDs, std::vector<unsigned int> &rightIDs, AABB &bb) {
	switch (this->method) {
		case BVH::Method::CUT_LONGEST_AXIS:
			this->cutFacesLongestAxis(mesh, faceIDs, leftIDs, rightIDs, bb);
			break;
		case BVH::Method::SURFACE_AREA_HEURISTIC:
			this->cutFacesSAH(mesh, faceIDs, leftIDs, rightIDs, bb);
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
	char longestAxis = bbCentroid.getLongestAxis();
	//maxEntentPoint[longestAxis] = maxEntentPoint[longestAxis] - (maxEntentPoint[longestAxis] - bbCentroid.getAABBMin()[longestAxis]) / 2;
	// this is the same:
	bbCentroid.max[longestAxis] = (bbCentroid.max[longestAxis] + bbCentroid.min[longestAxis]) / 2;
	/* Set the centroid BV's new maximum extent */
	/* Since we need to split up the IDs into two lists, we'll just go ahead and create those here.*/
	for (size_t i = 0; i < faceIDs.size(); ++i) {
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
void BVH::buildBVH(Mesh const &mesh) {
	std::size_t size = mesh.faces.size() / 3;
	std::vector<uint32_t> faceIDs(size);
// 	std::cout << "LIMIT: " << faceIDs.max_size() << std::endl;
// 	std::cout << "CAPA: " << faceIDs.capacity() << std::endl;
	for (std::size_t i = 0; i < size; ++i) {
// 		std::cout << i << std::endl;
		faceIDs[i] = i;
	}
// 	std::cout << "ACTUAL SIZE: " << faceIDs.size() << std::endl;
	this->triangles.reserve(size);
// 	std::cout << "LIMIT: " << this->triangles.max_size() << std::endl;
// 	this->triangles.reserve(size);
	this->nodes = std::vector<uint32_t>(size * 2 - 1);
	this->aabbs = std::vector<Vec3f>((size * 2 - 1) * 2);
// 	std::cout << "LIMIT: " << this->nodes.max_size() << " WANNA HAVE: " << size * 2 - 1 << " N: " << size << std::endl;
// 	this->nodes.reserve(size * 2 - 1);
// 	std::cout << "YEAH!" << std::endl;
	std::size_t i = 0;
// 	std::cout << std::endl; // because of the flush
	this->build(mesh, faceIDs, i);
// 	std::cout << std::endl; // because of the flush
	this->nodes.resize(this->nodes[0]);
	this->aabbs.resize(this->nodes[0] * 2);
// 	printf("\n");
// 	printf("NODE: %d AABB: %d\n", sizeof(Node), sizeof(AABB));
// 	printf("X1 %f\n", (*this->nodes)[1].bb.min[0]);
// 	printf("UINT: %d\n", sizeof(uint32_t));
// 	hexdump(&(*this->nodes)[0], 500);
// 	std::cout << "SWAG!!!" << (*this->nodes)[0].node_count << std::endl;
}
/*
* Builds an node of the BVH and returns the count of nodes (incl. the current node)
*/
unsigned int const BVH::build(const Mesh &mesh, std::vector< unsigned int > &faceIDs, std::size_t &ind) {
// 	size_t i = this->nodes.size();
// 	Node n;
// 	this->nodes.push_back(n);
//
// 	std::cout << ind << std::endl;
// 	uint32_t n;
// 	this->nodes.at(ind) = n;
	uint32_t & node = this->nodes.at(ind);
	AABB bb;
	if (faceIDs.size() <= /*MAX_LEAF_TRIANGLES*/1) {
// 		node.triangles_offset = this->triangles.size() * 3;
// 		node.triangles_length = faceIDs.size();
		/* Push all of our triangles into the node. */
		for (size_t i = 0; i < faceIDs.size(); ++i) {
			Triangle tri(&mesh, faceIDs[i]);    /* Get the triangle from the mesh */
			bb.merge(tri.getAABB());
			this->triangles.push_back(tri.getFaceID());
		}
		this->aabbs.at(ind * 2) = bb.min;
		this->aabbs.at(ind * 2 + 1) = bb.max;
		node = 1;
	}
	else {
		std::vector<unsigned int> leftIDs;
		leftIDs.reserve(faceIDs.size());
		std::vector<unsigned int> rightIDs;
		rightIDs.reserve(faceIDs.size());
		this->cutFaces(mesh, faceIDs, leftIDs, rightIDs, bb);
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
		this->aabbs.at(ind * 2) = bb.min;
		this->aabbs.at(ind * 2 + 1) = bb.max;
// 		node.triangles_offset = 0;
// 		node.triangles_length = 0;
		++ind;
		node = this->build(mesh, leftIDs, ind);
		leftIDs.clear();
		++ind;
		node += this->build(mesh, rightIDs, ind) + 1;
		rightIDs.clear();
	}
	//std::cout << this->nodes.at(i).node_count << std::endl;
	return node;
}
class sortByAxis {
	public:
		sortByAxis(const Mesh *mesh, char *axis) {
			this->mesh = mesh;
			this->axis = axis;
		}
		bool operator()(unsigned int i, unsigned int j) {
			Triangle tri1(this->mesh, i);
			Triangle tri2(this->mesh, j);
			return tri1.getCentroid()[*(this->axis)] > tri2.getCentroid()[*(this->axis)];
		}
	private:
		const Mesh *mesh;
		char *axis;
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
	char bestAxis = 0;
	size_t bestPos = 1;
	AABB _emptyBB;
	float minCosts = std::numeric_limits<float>::max();
	float currentCosts;
	size_t countFaceIDs = faceIDs.size();
	for (char axis = 0; axis < 3; ++axis) {
		std::sort(faceIDs.begin(), faceIDs.end(), sortByAxis(&mesh, &axis)); // And sort the array by the current axis
		AABB bbLeft = Triangle(&mesh, faceIDs[0]).getAABB(), bbRight;
		double countLeft, countRight;
		countLeft = 1;
		countRight = countFaceIDs - 1;
		for (size_t i = 1; i < countFaceIDs - 1; ++i) { // Vom 2. bis zum vorletzten, damit ein subnode nicht leer ist
			std::cout << "\rCutting " << countFaceIDs << " face IDs into two parts... Processing (Axis: " << (int) axis << ") " << std::setw(5) << std::setfill('0') << std::left << (10000 * (i + axis * countFaceIDs) / (countFaceIDs * 3)) / 100.0f << "%..." << std::flush;
			// Compute right bb
			for (size_t j = i; j < countFaceIDs; ++j) {
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
			bbRight = _emptyBB;
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