#pragma once
#include <cmath>
#include <ostream>
#include <algorithm>
#include <CL/cl.h>
template <typename T> class Vec3;
typedef Vec3<cl_float> Vec3f;
typedef Vec3<double> Vec3d;
typedef Vec3<int> Vec3i;
// Representation of a 3-vector with common functionality.
template <typename T> class Vec3 {
	public:
		Vec3(void) {
			std::fill(v, v + 3, T(0));
			this->fourth = (T) 0;
		}
		explicit Vec3(T value) {
			std::fill(v, v + 3, value);
		}
		Vec3(T const &nx, T const &ny, T const &nz) {
			v[0] = nx;
			v[1] = ny;
			v[2] = nz;
		}
		Vec3(Vec3<T> const &src) {
			std::copy(src.v, src.v + 3, v);
		}
		Vec3<T> operator=(const Vec3<T> rhs) {
			std::copy(rhs.v, rhs.v + 3, v);
			return *this;
		}
		T dot(Vec3<T> const &rhs) const {
			return v[0] * rhs.v[0] + v[1] * rhs.v[1] + v[2] * rhs.v[2];
		}
		Vec3<T> cross(Vec3<T> const &rhs) const {
			return Vec3<T>(v[1] * rhs.v[2] - rhs.v[1] * v[2],
			               v[2] * rhs.v[0] - rhs.v[2] * v[0],
			               v[0] * rhs.v[1] - rhs.v[0] * v[1]);
		}
		Vec3<T> operator+(T rhs) const {
			return Vec3<T>(v[0] + rhs, v[1] + rhs, v[2] + rhs);
		}
		Vec3<T> operator+(Vec3<T> const &rhs) const {
			return Vec3<T>(v[0] + rhs.v[0], v[1] + rhs.v[1], v[2] + rhs.v[2]);
		}
		Vec3<T> &operator+=(Vec3<T> const &rhs) {
			v[0] += rhs.v[0];
			v[1] += rhs.v[1];
			v[2] += rhs.v[2];
			return *this;
		}
		Vec3<T> operator-(Vec3<T> const &rhs) const {
			return Vec3<T>(v[0] - rhs.v[0], v[1] - rhs.v[1], v[2] - rhs.v[2]);
		}
		Vec3<T> operator*(T rhs) const {
			return Vec3<T>(v[0] * rhs, v[1] * rhs, v[2] * rhs);
		}
		Vec3<T> operator/(T rhs) const {
			return Vec3<T>(v[0] / rhs, v[1] / rhs, v[2] / rhs);
		}
		Vec3<T> &operator/=(T rhs) {
			v[0] /= rhs;
			v[1] /= rhs;
			v[2] /= rhs;
			return *this;
		}
		Vec3<T> operator-(T rhs) const {
			return Vec3<T>(v[0] - rhs, v[1] - rhs, v[2] - rhs);
		}
		T squareLength(void) const {
			return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
		}
		T length(void) const {
			return (T)std::sqrt(this->squareLength());
		}
		Vec3<T> normalized(void) const {
			return (*this) / this->length();
		}
		friend std::ostream &operator<<(std::ostream &lhs, const Vec3<T> rhs) {
			lhs << rhs.v[0] << " " << rhs.v[1] << " " << rhs.v[2];
			return lhs;
		}
		bool isSimilar(const Vec3<T> &rhs, T epsilon) const {
			return std::fabs(v[0] - rhs.v[0]) < epsilon
			       && std::fabs(v[1] - rhs.v[1]) < epsilon
			       && std::fabs(v[2] - rhs.v[2]) < epsilon;
		}
		T const &operator[](const unsigned int index) const {
			return v[index];
		}
		T &operator[](const unsigned int index) {
			return v[index];
		}
		T v[3];
	private:
		T fourth;
};