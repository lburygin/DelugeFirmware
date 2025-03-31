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
#include "dsp/core/converter.h"
#include "dsp/core/processor.h"
#include "util.h"
#include <type_traits>

/// @file An adapter is a specialization of a processor that wraps another processor, converting between an
/// "inner" type and an "outer" type. This allows for floating-point processors in fixed-point pipelines
/// and vice versa.

namespace deluge::dsp {
/// @brief A class that adapts a processor to work in a pipeline of a different type.
/// @tparam T The type of the input samples.
/// @tparam ProcessorType The type of the processor to be wrapped by the adapter, either a pointer or a non-pointer
/// type.
/// @details Adapter acts as a converter between an outer type `T` and an inner type which is the value type of the
/// processor.
template <typename T, typename ProcessorType>
class Adapter : public Converter<T, typename std::remove_pointer_t<ProcessorType>::value_type>, public Processor<T> {
public:
	using from_type = T;
	using to_type = typename std::remove_pointer_t<ProcessorType>::value_type;

	static_assert(!std::is_same_v<from_type, to_type>, "Adapter requires different types for source and destination.");

	/// @brief Constructor for the BlockAdapter class.
	/// @param processor The processor to be contained in the adapter.
	Adapter(ProcessorType processor) : processor_(processor) {}

	/// @brief Destructor for the BlockAdapter class.
	virtual ~Adapter() = default;

	[[gnu::always_inline]] T render(T sample) final {
		auto converted = converter_.render(sample);
		auto output = util::dereference(processor_).render(converted);
		return converter_.render(output);
	}

private:
	ProcessorType processor_;                 // The contained processor (can be a pointer)
	Converter<from_type, to_type> converter_; // The converter
};

/// @copydoc Adapter
template <typename T, typename ProcessorType>
class SIMDAdapter : public Converter<Argon<T>, Argon<typename std::remove_pointer_t<ProcessorType>::value_type>>,
                    public SIMDProcessor<T> {
public:
	using from_type = Argon<T>;
	using to_type = Argon<typename std::remove_pointer_t<ProcessorType>::value_type>;

	static_assert(!std::is_same_v<from_type, to_type>, "Adapter requires different types for source and destination.");

	/// @brief Constructor for the BlockAdapter class.
	/// @param processor The processor to be contained in the adapter.
	SIMDAdapter(ProcessorType processor) : processor_{processor} {}

	/// @brief Destructor for the BlockAdapter class.
	virtual ~SIMDAdapter() = default;

	[[gnu::always_inline]] Argon<T> render(Argon<T> sample) final {
		auto converted = converter_.render(sample);
		auto output = util::dereference(processor_).render(converted);
		return converter_.render(output);
	}

private:
	ProcessorType processor_;                 // The contained processor (can be a pointer)
	Converter<from_type, to_type> converter_; // The converter
};
} // namespace deluge::dsp
