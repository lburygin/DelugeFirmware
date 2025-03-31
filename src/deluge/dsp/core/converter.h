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
#include "dsp/core/types.h"
#include <argon.hpp>

namespace deluge::dsp {
/// @brief A base class for adapters that process a stream of samples.
/// @tparam T The type of the input samples.
/// @tparam U The type of the output samples.
template <typename T, typename U>
struct BlockConverter {
	static_assert(!std::is_same_v<T, U>, "BlockConverter requires different types T and U.");

	/// @brief Destructor for the BlockConverter class.
	virtual ~BlockConverter() = default;

	/// @brief Convert a block of type T to type U.
	/// @param input The input buffer of type T to convert.
	/// @param output The output buffer of type U to fill with converted samples.
	virtual void renderBlock(Signal<T> input, Buffer<U> output) = 0;
};

/// @brief A base class for adapters that process a single sample at a time.
/// @tparam T The type of the input samples.
/// @tparam U The type of the output samples.
template <typename T, typename U>
struct Converter : BlockConverter<T, U>, BlockConverter<U, T> {
	/// @brief Convert a single sample of type T to type U.
	/// @param sample The input sample of type T to convert.
	/// @return The converted sample of type U.
	virtual U render(T sample) = 0;

	/// @brief Convert a block of type T to type U by calling render() for each sample.
	/// @param input The input buffer of type T to convert.
	/// @param output The output buffer of type U to fill with converted samples.
	void renderBlock(Signal<T> input, Buffer<U> output) final {
		for (size_t i = 0; i < input.size(); ++i) {
			output[i] = render(input[i]); // Call the process function for each sample
		}
	}

	/// @brief Convert a single sample of type U to type T.
	/// @param sample The input sample of type U to convert.
	/// @return The converted sample of type T.
	virtual T render(U sample) = 0;

	/// @brief Convert a block of type U to type T by calling render() for each sample.
	/// @param input The input buffer of type U to convert.
	/// @param output The output buffer of type T to fill with converted samples.
	void renderBlock(Signal<U> input, Buffer<T> output) final {
		for (size_t i = 0; i < input.size(); ++i) {
			output[i] = render(input[i]); // Call the process function for each sample
		}
	}
};

/// @brief A base class for adapters that process a vector of samples using SIMD operations.
/// @tparam T The type of the input samples.
/// @tparam U The type of the output samples.
template <typename T, typename U>
struct Converter<Argon<T>, Argon<U>> : BlockConverter<T, U>, BlockConverter<U, T> {
	/// @brief Convert a vector of samples of type T to type U using SIMD operations.
	/// @param sample The input samples of type T to convert.
	/// @return The converted samples of type U.
	virtual Argon<U> render(Argon<T> sample) = 0;

	/// @brief Convert a block of type T to type U using SIMD operations.
	/// @param input The input buffer of type T to convert.
	/// @param output The output buffer of type U to fill with converted samples.
	void renderBlock(Signal<T> input, Buffer<U> output) final {
		auto input_view = input | std::views::chunk(Argon<T>::lanes);
		auto output_view = output | std::views::chunk(Argon<U>::lanes);

		auto input_it = input_view.begin();
		auto output_it = output_view.begin();
		for (; input_it != input_view.end() && output_it != output_view.end(); ++input_it, ++output_it) {
			auto [input_chunk_start, _input_chunk_end] = *input_it;
			auto [output_chunk_start, _output_chunk_end] = *output_it;

			Argon<T> input = Argon<T>::Load(&*input_chunk_start);
			Argon<U> output = render(input);
			output.StoreTo(&*output_chunk_start);
		}
	}

	/// @brief Convert a vector of samples of type U to type T using SIMD operations.
	/// @param sample The input samples of type U to convert.
	/// @return The converted samples of type T.
	virtual Argon<T> render(Argon<U> sample) = 0;

	/// @brief Convert a block of type U to type T using SIMD operations.
	/// @param input The input buffer of type U to convert.
	/// @param output The output buffer of type T to fill with converted samples.
	void renderBlock(Signal<U> input, Buffer<T> output) final {
		auto input_view = input | std::views::chunk(Argon<U>::lanes);
		auto output_view = output | std::views::chunk(Argon<T>::lanes);

		auto input_it = input_view.begin();
		auto output_it = output_view.begin();
		for (; input_it != input_view.end() && output_it != output_view.end(); ++input_it, ++output_it) {
			auto [input_chunk_start, _input_chunk_end] = *input_it;
			auto [output_chunk_start, _output_chunk_end] = *output_it;

			Argon<U> input = Argon<U>::Load(&*input_chunk_start);
			Argon<T> output = render(input);
			output.StoreTo(&*output_chunk_start);
		}
	}
};
} // namespace deluge::dsp
