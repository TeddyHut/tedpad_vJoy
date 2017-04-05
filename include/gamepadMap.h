#pragma once

#include <string>
#include <mutex>
#include <thread>
#include <atomic>
#include "../tedpad/include/tedpad.h"

namespace gamepad_map {
	namespace attribute_vjoy {
		enum class VjoyType {
			Axis,
			Button,
			Effect,
			Invalid,
		};
		enum class VjoyAxisType {
			LX,
			LY,
			LZ,
			RX,
			RY,
			RZ,
			Invalid,
		};
		enum class VjoyEffectType {
			Constant,
			Invalid,
		};

		struct VjoyGeneric {
			VjoyType type;
			virtual void tedpadConversion(tedpad::Gamepad &gamepad, std::string const &attribute) = 0;
			VjoyGeneric(VjoyType const type = VjoyType::Invalid);
		};

		//Might be more C++y to specialise these with templates, eg instead of VjoyButton, do VjoyAttribute<bool>
		struct VjoyButton : public VjoyGeneric {
			//Value from 1 to 128
			uint8_t id;
			bool state = false;

			void tedpadConversion(tedpad::Gamepad &gamepad, std::string const &attribute) override;
			VjoyButton(uint8_t const id = 0);
		};

		struct VjoyAxis : public VjoyGeneric {
			static constexpr uint16_t min_value = 0x1;
			static constexpr uint16_t max_value = 0x8000;
			VjoyAxisType id;
			uint16_t state = min_value;

			void tedpadConversion(tedpad::Gamepad &gamepad, std::string const &attribute) override;
			VjoyAxis(VjoyAxisType const id = VjoyAxisType::Invalid);
		};

		struct VjoyEffect : public VjoyGeneric {
			//The type of effect. Only constant is supported at the moment.
			VjoyEffectType id;
			VjoyEffect(VjoyEffectType const id = VjoyEffectType::Invalid);
		};

		struct VjoyEffect_Constant : public VjoyEffect {
			//If the tedpad attribute is analogue, what the effect "on" value should be
			uint8_t onValue_analogue = UINT8_MAX;
			//If the tedpad attribute is an axis
			int16_t onValue_axis = INT16_MAX;

			//Use this to assign a new duration to the thread. In a function so that the thread knows when a new duration is set.
			void assign_duration(uint16_t const duration);

			void tedpadConversion(tedpad::Gamepad &gamepad, std::string const &attribute) override;
			VjoyEffect_Constant();
		private:
			//Used by a thread to check wheather is had been invalidated
			struct ThreadInfo {
				bool invalidated = false;
				std::thread::id threadId;
			};
			uint16_t duration;
			void set_attribute(tedpad::Gamepad &gamepad, std::string const &attribute, bool const state) const;

			void thread_main(uint16_t const duration);

			//Used by threads to check whether they have been invalidated
			std::vector<ThreadInfo> vec_threadInfo;
			std::mutex x_vec_threadInfo;

			//Will be true when a thread has completed and has not been invalidated.
			std::atomic<bool> completed = false;
			std::atomic<bool> new_duration = false;
		};
	}

	struct AttributeMap {
		std::string tedpad_attribute;
		std::unique_ptr<attribute_vjoy::VjoyGeneric> vjoy_attribute;

		AttributeMap &operator=(AttributeMap const &p0);
		AttributeMap &operator=(AttributeMap &&p0) = default;
		AttributeMap(AttributeMap const &p0);
		AttributeMap(AttributeMap &&p0) = default;
		AttributeMap() = default;
	private:
		void op_assign(AttributeMap const &p0);
	};

	class CompleteMap {
	public:
		std::vector<AttributeMap> attribute;
		std::string gamepadName;
	};
}
