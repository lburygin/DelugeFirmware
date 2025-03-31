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
#include "definitions_cxx.hpp"
#include "dsp/core/types.h"
#include "util/containers.h"
#include <argon.hpp>
#include <bitset>

namespace deluge::dsp {
template <typename ProcessorType, typename T, size_t N, typename ParallelType>
class ParallelProcessor {
public:
	using value_type = T;
	using parallel_type = ParallelType;

	constexpr static size_t num_slots = N;
	virtual ~ParallelProcessor() = default;

	/// The render function is called for each sample in the input buffer.
	virtual ParallelType render(ParallelType input) = 0;

	struct Slot {
		ProcessorType& processor;
		const size_t slot;

		virtual ~Slot() { release(); }

	private:
		/// @brief Release the slot for the processor (called in the destructor)
		constexpr void release() {
			processor.releaseSlot(slot); // Release the slot
		}
	};

	template <typename... Args>
	static Slot acquire(Args... args) {
		for (auto& instance : instances) {
			if (instance.hasAvailableSlot()) {
				return {instance.acquireSlot(), instance}; // Return the slot and the processor
			}
		}

		// No slots available, create a new processor
		instances.emplace_back(std::forward<Args>(args)...);
		auto instance = instances.back();
		return {instance.acquireSlot(), instance}; // Return the slot and the processor
	}

	/// @brief Remove all processors that have all slots available
	static void cleanup() {
		for (auto it = instances.begin(); it != instances.end();) {
			if (it->slots_.all()) {
				it = instances.erase(it); // Remove the processor if all slots are available
			}
			else {
				++it;
			}
		}
	}

	/// @brief Acquire a slot for the processor
	[[nodiscard]] size_t acquireSlot() const {
		for (size_t i = 0; i < N; ++i) {
			if (slots_[i]) {
				slots_[i] = false; // Mark the slot as occupied
				return i;
			}
		}
		return N; // No available slot
	}

	/// @brief Release a slot for the processor
	constexpr void releaseSlot(size_t slot) {
		slots_[slot] = true; // Mark the slot as available
	}

protected:
	std::bitset<N>& slots() const { return slots_; }

private:
	std::bitset<N> slots_{};                         // Bitset to track available slots
	static std::vector<ParallelProcessor> instances; // Vector to store all instances of the processor
};

} // namespace deluge::dsp
