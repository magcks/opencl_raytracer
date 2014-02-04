#include "vector3.h"
#include "ray.h"
#include "sampler.h"
#include <iostream>

int main() {
	// Create vector
	//Vector3 v = Vector3(1, 0, 0);
	//std::cout << "Vector\t\t\t" << v << " created." << std::endl;

	// Rotation
	//float deg = 90.0f;
	//std::cout << "Rotating\t\t" << v << " by " << deg << "° in y-direction..." << std::endl;
	//v.rotZ(90);

	// Result
	//std::cout << "Rotation result is:\t" << v << std::endl;

	// Get trace vector of n = (0, 0, 1) and p = (2, 3, 1)
	//Vector3 n = Vector3(0, 0, 1);
	//Vector3 p = Vector3(0, 24, 1);
	//Vector3 result;
	//getTrace(&n, &p, &result);
	//std::cout << "Trace is:\t\t" << result << std::endl;

	//Vector3 n1 = Vector3(11.3, 4.2, 6.2);
	//Vector3 n2 = Vector3(11.3, 4.2, 6.3);
	//std::cout << std::endl << "Are\t\t\t" << n1 << " and " << n2 << " linearly dependent? " << isLinearlyDependent(&n1, &n2) << std::endl;

	//std::cout << std::endl << "n2, normalized:\t\t" << (*n2.normalized()) << std::endl << std::endl;

	//Vector3 normalResult;
	//getBestNormal(&n, &p, &normalResult);

	//Vector3 firstResult;
	//Vector3 secondResult;

	//getNormals(&n, &p, &firstResult, &secondResult);
	//std::cout << "OC normal:\t\t" << n << std::endl;
	//std::cout << "Normal 2:\t\t" << firstResult << std::endl;
	//std::cout << "Normal 3:\t\t" << secondResult << std::endl;

	Vector3 normal = Vector3(0, 0, 1);
	Vector3 point = Vector3(0, 0, 0);
	Vector3 res1, res2;
	getNormals(&normal, &point, &res1, &res2);
	float phi = 30;
	float size = 10;

	//sampleRing(normal, point, phi, 20);

	sampleRing(normal, point, phi, size);

	//Vector3 destVec = Vector3(0.123, -0.456, -0.789);
	//Vector3 rot = rotArb(normal, point, destVec, phi);

	//std::cout << rot << std::endl;
}
