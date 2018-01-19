#pragma once
#include <cmath>
#include <ostream>
#include <algorithm>
constexpr std::size_t X = 0, Y = 1, Z = 2;
template <typename T> class Vec3 {
	public:
		Vec3() {
			std::fill(v, v + 3, T(0));
			fourth = (T) 0;
		}
		explicit Vec3(T value) {
			std::fill(v, v + 3, value);
		}
		Vec3(T const &nx, T const &ny, T const &nz) {
			v[X] = nx;
			v[Y] = ny;
			v[Z] = nz;
		}
		Vec3(Vec3<T> const &src) {
			std::copy(src.v, src.v + 3, v);
		}
		Vec3<T> operator=(const Vec3<T> rhs) {
			std::copy(rhs.v, rhs.v + 3, v);
			return *this;
		}
		T dot(Vec3<T> const &rhs) const {
			return v[X] * rhs.v[X] + v[Y] * rhs.v[Y] + v[Z] * rhs.v[Z];
		}
		Vec3<T> cross(Vec3<T> const &rhs) const {
			return Vec3<T>(
				v[Y] * rhs.v[Z] - rhs.v[Y] * v[Z],
				v[Z] * rhs.v[X] - rhs.v[Z] * v[X],
				v[X] * rhs.v[Y] - rhs.v[X] * v[Y]
			);
		}
		Vec3<T> operator+(T rhs) const {
			return Vec3<T>(v[X] + rhs, v[Y] + rhs, v[Z] + rhs);
		}
		Vec3<T> operator+(Vec3<T> const &rhs) const {
			return Vec3<T>(v[X] + rhs.v[X], v[Y] + rhs.v[Y], v[Z] + rhs.v[Z]);
		}
		Vec3<T> &operator+=(Vec3<T> const &rhs) {
			v[X] += rhs.v[X];
			v[Y] += rhs.v[Y];
			v[Z] += rhs.v[Z];
			return *this;
		}
		Vec3<T> operator-(Vec3<T> const &rhs) const {
			return Vec3<T>(v[X] - rhs.v[X], v[Y] - rhs.v[Y], v[Z] - rhs.v[Z]);
		}
		Vec3<T> operator*(T rhs) const {
			return Vec3<T>(v[X] * rhs, v[Y] * rhs, v[Z] * rhs);
		}
		Vec3<T> operator/(T rhs) const {
			return Vec3<T>(v[X] / rhs, v[Y] / rhs, v[Z] / rhs);
		}
		Vec3<T> &operator/=(T rhs) {
			v[X] /= rhs;
			v[Y] /= rhs;
			v[Z] /= rhs;
			return *this;
		}
		Vec3<T> operator-(T rhs) const {
			return Vec3<T>(v[X] - rhs, v[Y] - rhs, v[Z] - rhs);
		}
		T squareLength() const {
			return v[X] * v[X] + v[Y] * v[Y] + v[Z] * v[Z];
		}
		T length() const {
			return (T) std::sqrt(squareLength());
		}
		Vec3<T> normalized() const {
			return (*this) / length();
		}
		friend std::ostream &operator<<(std::ostream &os, const Vec3<T> vec) {
			os << vec.v[X] << " " << vec.v[Y] << " " << vec.v[Z];
			return os;
		}
		bool isSimilar(const Vec3<T> &vec, T epsilon) const {
			return
				std::fabs(v[X] - vec.v[X]) < epsilon &&
				std::fabs(v[Y] - vec.v[Y]) < epsilon &&
				std::fabs(v[Z] - vec.v[Z]) < epsilon;
		}
		T const &operator[](const unsigned int index) const {
			return v[index];
		}
		T &operator[](const unsigned int index) {
			return v[index];
		}
	private:
		T v[3];
		T fourth;
};
typedef Vec3<float> Vec3f;