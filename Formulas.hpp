#ifndef FORMULAS_HPP_
#define FORMULAS_HPP_

#include <type_traits>

/** @brief Compute the number of milliseconds per beat based on beats per minute */
template <typename FloatType>
FloatType ComputeMillisecondsPerBeat(FloatType BPM) {
	static_assert(std::is_floating_point<FloatType>::value);
	return (1000.0 * 60.0 / BPM);
}

#endif
