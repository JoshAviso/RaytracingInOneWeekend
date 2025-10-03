#pragma once

#include <cmath>
#include <limits>
#include <memory>
// Usings
using std::shared_ptr;
using std::make_shared;
using std::sqrt;
// Constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;
// Utility Functions
inline double degrees_to_radians(double degrees) {
	return degrees * pi / 180.0;
}
inline double clamp(double x, double min, double max) {
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

// Random Utils
#include <cstdlib>
inline double random_double() {
	// Returns a random real in [0,1).
	return rand() / (RAND_MAX + 1.0);
}
inline double random_double(double min, double max) {
	// Returns a random real in [min,max).
	return min + (max - min) * random_double();
}
#include <random>
#include <thread>
// Thread-local mt19937 generator
inline double threadsafe_random_double() {
	static thread_local std::mt19937 generator(std::random_device{}() + std::hash<std::thread::id>{}(std::this_thread::get_id()));
	static thread_local std::uniform_real_distribution<double> distribution(0.0, 1.0);
	return distribution(generator);
}
inline double threadsafe_random_double(double min, double max) {
	// Returns a random real in [min,max).
	return min + (max - min) * threadsafe_random_double();
}

// Common Headers
#include "ray.h"
#include "Vec3.h"