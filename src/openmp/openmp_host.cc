#include "openmp_host.h"


bool OpenMPHost::operator()(Mesh &mesh, std::vector<unsigned int> &nodes, std::vector<Vec3f> &aabbs, float * tmp) {
	this->kernel.iterate(mesh, nodes, aabbs, tmp);

	return true; // TODO
}