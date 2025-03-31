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
#include "types.h"
#include <argon.hpp>

namespace deluge::dsp {

/// @brief A base class for generators that process a stream of samples.
/// @tparam T The type of the samples to process.
template <typename T>
struct BlockProcessor {
	/// @brief The type of the samples to process.
	using value_type = T;

	virtual ~BlockProcessor() = default;

	/// @brief Process a block of samples.
	/// @param input The input buffer of samples to process.
	/// @param output The output buffer to fill with processed samples.
	virtual void renderBlock(Buffer<T> input, Buffer<T> output) = 0;
};

/// @brief A base class for processors that are able to process a single sample at a time.
/// @tparam T The type of the samples to process.
template <typename T>
struct Processor : BlockProcessor<T> {
	/// @brief Process a single sample of type T.
	/// @param sample The input sample to process.
	/// @return The processed sample.
	virtual T render(T sample) = 0;

	/// @brief Process a block of samples by calling render() for each sample.
	/// @param input The input buffer of samples to process.
	/// @param output The output buffer to fill with processed samples.
	void renderBlock(Buffer<T> input, Buffer<T> output) final {
		for (size_t i = 0; i < input.size(); ++i) {
			output[i] = render(input[i]); // Call the process function for each sample
		}
	}
};

/// @brief A base class for processors that are able to process a vector of samples using SIMD operations.
/// @tparam T The type of the samples to process.
template <typename T>
struct SIMDProcessor : BlockProcessor<T> {
	/// @brief Process a block of samples using SIMD operations.
	/// @param samples The input buffer of samples to process.
	/// @return The processed buffer of samples.
	virtual Argon<T> render(Argon<T> sample) = 0;

	/// @brief Process a block of samples by calling render() for each sample.
	/// @param input The input buffer of samples to process.
	/// @param output The output buffer to fill with processed samples.
	void renderBlock(Buffer<T> input, Buffer<T> output) final {
		auto input_view = input | std::views::chunk(Argon<T>::lanes);
		auto output_view = output | std::views::chunk(Argon<T>::lanes);

		auto input_it = input_view.begin();
		auto output_it = output_view.begin();
		for (; input_it != input_view.end() && output_it != output_view.end(); ++input_it, ++output_it) {
			auto [input_chunk_start, _input_chunk_end] = *input_it;
			auto [output_chunk_start, _output_chunk_end] = *output_it;

			Argon<T> input = Argon<T>::Load(*input_chunk_start);
			Argon<T> output = render(input);
			output.StoreTo(*output_chunk_start);
		}
	};
};

} // namespace deluge::dsp
