#ifndef ROTATE_H
#define ROTATE_H

#include <cmath>
#include <string>
#include <sstream>
#include <iostream>

#define _USE_MATH_DEFINES


class Vector3 {
	public:
		float x, y, z;
		Vector3() : x(0), y(0), z(0) {
		}

		Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {
		}

		float getLength() {
			return sqrt(x * x + y * y + z * z);
		}

		void rotX(float phi) {
			phi *= (2 * M_PI) / 360;
			float tmpY = this->y;
			float tmpZ = this->z;
			y = cos(phi) * tmpY - sin(phi) * tmpZ;
			z = sin(phi) * tmpY + cos(phi) * tmpZ;
		}

		void rotY(float phi) {
			phi *= (2 * M_PI) / 360;
			float tmpX = this->x;
			float tmpZ = this->z;
			this->z = -sin(phi) * tmpX + cos(phi) * tmpZ;
			this->x = cos(phi) * tmpX + sin(phi) * tmpZ;
		}

		void rotZ(float phi) {
			phi *= (2 * M_PI) / 360;
			float tmpX = this->x;
			float tmpY = this->y;
			this->x = cos(phi) * tmpX - sin(phi) * tmpY;
			this->y = sin(phi) * tmpX + cos(phi) * tmpY;
		}

		float dot(Vector3 *operand) {
			return this->x * (*operand).x + this->y * (*operand).y + this->z * (*operand).z;
		}

		void normalize() {
			Vector3 normalized = (*this) / (*this).getLength();
			this->x = normalized.x;
			this->y = normalized.y;
			this->z = normalized.z;
		}

		Vector3* normalized() {
			normalize();
			return this;
		}

		Vector3 operator+ (Vector3 &operand) const {
			return Vector3(this->x + operand.x, this->y + operand.y, this->z + operand.z);
		}

		Vector3 operator- (Vector3 &operand) const {
			return Vector3(this->x - operand.x, this->y - operand.y, this->z - operand.z);
		}

		Vector3 operator/ (int num) const {
			return Vector3(this->x / (float) num, this->y / (float) num, this->z / (float) num);
		}

		Vector3 operator/ (float num) const {
			return Vector3(this->x / num, this->y / num, this->z / num);
		}

		Vector3 operator* (int num) const {
			return Vector3((float) num * this->x, (float) num * this->y, (float) num * this->z);
		}

		Vector3 operator* (float num) const {
			return Vector3(num * this->x, num * this->y, num * this->z);
		}

		bool operator == (Vector3 &operand) const {
			if (this->x == operand.x && this->y == operand.y && this->z == operand.z)
				return true;
			return false;
		}

		bool operator != (Vector3 &operand) const {
			if (this->x != operand.x || this->y != operand.y || this->z != operand.z)
				return true;
			return false;
		}

		friend std::ostream& operator<< (std::ostream &out, Vector3 &vec);

		void print() {
			std::cout.precision(2);
			std::cout << "(" << std::fixed << this->x << ", " << this->y << ", " << this->z << ")" << std::endl;
		}
};

std::ostream& operator<< (std::ostream &out, Vector3 &vec) {
	std::cout.precision(2);
	out << "(" << std::fixed << vec.x << ", " << std::fixed << vec.y << ", " << std::fixed << vec.z << ")";
	return out;
}

float dot(Vector3 *operandA, Vector3 *operandB) {
	return (*operandA).x * (*operandB).x + (*operandA).y * (*operandB).y + (*operandA).z * (*operandB).z;
}

Vector3 cross(Vector3 *operandA, Vector3 *operandB) {
	return Vector3(	(*operandA).y * (*operandB).z - (*operandA).z * (*operandB).y,
			(*operandA).z * (*operandB).x - (*operandA).x * (*operandB).z,
			(*operandA).x * (*operandB).y - (*operandA).y * (*operandB).x);
}

bool isLinearlyDependent(Vector3 *v1, Vector3 *v2) {
	float precision = 0.000001f;
	float lambda;

	// Is v1 = o || v2 = o?
	//if ((*v1).x == 0 && (*v1).y == 0 && (*v1).z == 0 || (*v2).x == 0 && (*v2).y == 0 && (*v2).z == 0)
	//	return true;

	// Determine dependency factor
	if ((*v1).x != 0)
		lambda = (*v2).x / (*v1).x;
	else if ((*v1).y != 0)
		lambda = (*v2).y / (*v1).y;
	else if ((*v1).z != 0)
		lambda = (*v2).z / (*v1).z;
	else
		return true;

	// Check componentwise dependency
	if (lambda * (*v1).y >= (*v2).y - precision && lambda * (*v1).y <= (*v2).y + precision)
		if (lambda * (*v1).z >= (*v2).z - precision && lambda * (*v1).z <= (*v2).z + precision)
			return true;
	return false;
}

bool isLinearlyIndependent(Vector3 *v1, Vector3 *v2) {
	return !isLinearlyDependent(v1, v2);
}

#endif
