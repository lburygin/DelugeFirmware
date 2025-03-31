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
#include "dsp/core/generator.h"
#include "dsp/core/types.h"

namespace deluge::dsp {

/// @brief A class that chains together multiple block processors for use high-level (multiple effects) audio pipeline.
/// @tparam T The type of the samples.
template <typename T>
class Chain : public BlockProcessor<T> {
	std::vector<BlockProcessor<T>*> processors_;

public:
	using value_type = T;

	Chain(std::vector<BlockProcessor<T>*> processors) : processors_{processors} {}

	/// @brief Renders a block of audio samples by processing the input through each processor in the chain.
	[[gnu::always_inline]] void renderBlock(Signal<T> input, Buffer<T> output) final {
		for (auto& processor : processors_) {
			processor->renderBlock(input, output); // Call the process function for each sample
		}
	}

	/// @brief Get the processors in the chain.
	std::vector<BlockProcessor<T>*>& processors() { return processors_; }
};

} // namespace deluge::dsp
