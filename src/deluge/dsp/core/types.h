/**
 * Copyright (c) 2025 Katherine Whitlock
 *
 * This file is part of The Synthstrom Audible Deluge Firmware.
 *
 * The Synthstrom Audible Deluge Firmware is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once
#include "util/fixedpoint.h"
#include <span>
#include <string_view>

namespace deluge::dsp {

template <typename T>
using Sample = T;

template <typename T>
using Buffer = std::span<T>;

template <typename T>
using Signal = std::span<const T>;

template <typename T>
struct StereoSample {
	Sample<T> l;
	Sample<T> r;

	[[gnu::always_inline]] static constexpr StereoSample fromMono(Sample<T> sample) { return {sample, sample}; }

	bool operator==(const StereoSample& other) const { return l == other.l && r == other.r; }
	bool operator!=(const StereoSample& other) const { return !(*this == other); }
	StereoSample operator+(const StereoSample& other) const { return {l + other.l, r + other.r}; }
	StereoSample operator-(const StereoSample& other) const { return {l - other.l, r - other.r}; }
	StereoSample operator*(T scalar) const { return {l * scalar, r * scalar}; }
};

template <typename T>
using StereoBuffer = Buffer<StereoSample<T>>;

template <typename T>
using StereoSignal = Signal<StereoSample<T>>;

namespace fixed_point {
using Sample = Sample<FixedPoint<31>>;
using Buffer = Buffer<FixedPoint<31>>;
using Signal = Signal<FixedPoint<31>>;
using StereoSample = StereoSample<FixedPoint<31>>;
using StereoBuffer = StereoBuffer<FixedPoint<31>>;
using StereoSignal = StereoSignal<FixedPoint<31>>;
} // namespace fixed_point

namespace floating_point {
using Sample = Sample<float>;
using Buffer = Buffer<float>;
using Signal = Signal<float>;
using StereoSample = StereoSample<float>;
using StereoBuffer = StereoBuffer<float>;
using StereoSignal = StereoSignal<float>;
} // namespace floating_point

/// @brief A simple class to represent a value with a specific unit.
/// @tparam T The type of the value.
template <typename T>
struct Value {
	T value = 0;

	Value(T value) : value{value} {}
	constexpr Value() = default;
	constexpr operator T() { return value; }
	constexpr auto operator<=>(const Value<T>& o) const = default;
	constexpr bool operator==(const Value<T>& o) const = default;
};

struct Frequency : Value<float> {
	Frequency(float value) : Value{value} {}

	auto operator<=>(const Frequency& o) const { return this->value <=> o.value; };
	constexpr operator float() const { return this->value; }

	// std::string_view unit() const { return "Hz"; }
	static constexpr std::string_view unit = "Hz";
};

/// @brief A class to represent a percentage value.
struct Percentage : Value<float> {
	using Value::Value;
	Percentage(float value, float lower_bound, float upper_bound)
	    : Value<float>{value}, lower_bound{lower_bound}, upper_bound{upper_bound} {}

	bool operator==(const Percentage& o) const {
		if (this->lower_bound == o.lower_bound && this->upper_bound == o.upper_bound) {
			return this->value == o.value;
		}

		float diff = upper_bound - lower_bound;
		float val_scaled = (this->value - lower_bound) / diff;

		float o_diff = o.upper_bound - o.lower_bound;
		float o_val_scaled = (o.value - o.lower_bound) / diff;

		return (val_scaled + lower_bound) == (o.value + o.lower_bound);
	}

	float lower_bound = 0.f;
	float upper_bound = 1.f;

	static constexpr std::string_view unit = "%";
};

/// @brief A class to represent the Q-factor of a filter.
/// @tparam T The type of the value.
using QFactor = Value<float>;

struct Gain : Value<float> {
	using Value::Value;
	// std::string_view unit() const { return "dB"; }
};

} // namespace deluge::dsp
