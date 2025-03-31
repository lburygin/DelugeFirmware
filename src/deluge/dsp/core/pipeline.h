// Copyright (c) 2025 Katherine Whitlock
//
// This file is part of The Synthstrom Audible Deluge Firmware.
//
// The Synthstrom Audible Deluge Firmware is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.
// If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include <dsp/core/processor.h>
#include <queue>

/// A DSP pipeline is a collection of processors that are executed concurrently (whether threaded or SIMD). This is done
/// by introducing a single sample delay. For N processors there are N stored samples. The first processor is fed the
/// input sample, the second processor is fed the previous output sample of the first processor, and so on.

namespace deluge::dsp {

template <typename T>
class ParallelPipeline {
	std::queue<T> samples_;
	std::vector<SIMDProcessor<T>> processors_;

public:
};
} // namespace deluge::dsp
