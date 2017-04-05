#include "../include/gamepadMap.h"

void gamepad_map::attribute_vjoy::VjoyButton::tedpadConversion(tedpad::Gamepad & gamepad, std::string const & attribute)
{
	switch (gamepad.get_attributeType(attribute)) {
	case tedpad::Gamepad::AttributeType::Digital:
		gamepad.Get_attribute(attribute, state);
		break;
	case tedpad::Gamepad::AttributeType::Analogue:
	{
		uint8_t result = 0;
		gamepad.Get_attribute(attribute, result);
		state = result ? true : false;
		break;
	}
	case tedpad::Gamepad::AttributeType::Axis:
	{
		int16_t result = 0;
		gamepad.Get_attribute(attribute, result);
		state = result ? true : false;
		break;
	}
	case tedpad::Gamepad::AttributeType::Buffer:
		std::cout << "No conversion to VjoyButton(" << static_cast<uint16_t>(id) << ") from tedpad attribute (" << attribute << ")" << std::endl;
		break;
	case tedpad::Gamepad::AttributeType::Invalid:
		std::cout << "No conversion to Vjoybutton(" << static_cast<uint16_t>(id) << ") from invalid attribute" << std::endl;
		break;
	default:
		break;
	}
}

gamepad_map::attribute_vjoy::VjoyButton::VjoyButton(uint8_t const id) : VjoyGeneric(VjoyType::Button), id(id)
{
}

void gamepad_map::attribute_vjoy::VjoyAxis::tedpadConversion(tedpad::Gamepad & gamepad, std::string const & attribute)
{
	switch (gamepad.get_attributeType(attribute)) {
	case tedpad::Gamepad::AttributeType::Digital:
	{
		bool result = false;
		gamepad.Get_attribute(attribute, result);
		state = result ? max_value : min_value;
		break;
	}
	case tedpad::Gamepad::AttributeType::Analogue:
	{
		uint8_t result = 0;
		gamepad.Get_attribute(attribute, result);
		constexpr float multiplicationRatio = (max_value - min_value) / UINT8_MAX;
		state = static_cast<uint16_t>((result * multiplicationRatio) + min_value);
		break;
	}
	case tedpad::Gamepad::AttributeType::Axis:
	{
		int16_t result = 0;
		gamepad.Get_attribute(attribute, result);
		constexpr float multiplicationRatio = (max_value - min_value) / UINT16_MAX;
		//Not sure exactly how accurate this conversion is
		state = static_cast<uint16_t>(((result + (-INT16_MIN)) * multiplicationRatio) + min_value);
		break;
	}
	//TODO: Make these error statements use strings instead of enums
	case tedpad::Gamepad::AttributeType::Buffer:
		std::cout << "No conversion to VjoyAxis(" << static_cast<uint16_t>(id) << ") from tedpad attribute (" << attribute << ")" << std::endl;
		break;
	case tedpad::Gamepad::AttributeType::Invalid:
		std::cout << "No conversion to VjoyAxis(" << static_cast<uint16_t>(id) << ") from invalid attribute" << std::endl;
		break;
	default:
		break;
	}
}

gamepad_map::attribute_vjoy::VjoyAxis::VjoyAxis(VjoyAxisType const id) : VjoyGeneric(VjoyType::Axis), id(id)
{
}

void gamepad_map::attribute_vjoy::VjoyEffect_Constant::assign_duration(uint16_t const duration)
{
	std::lock_guard<std::mutex> lx_threadInfo(x_vec_threadInfo);
	//If there are any threads, invalidate them all.
	if (vec_threadInfo.size()) {
		std::for_each(vec_threadInfo.begin(), vec_threadInfo.end(), [](ThreadInfo &p0) {p0.invalidated = true; });
	}
	new_duration = true;
	this->duration = duration;

}

void gamepad_map::attribute_vjoy::VjoyEffect_Constant::tedpadConversion(tedpad::Gamepad & gamepad, std::string const & attribute)
{
	if (new_duration) {
		//No duration
		if (duration == 0)
			set_attribute(gamepad, attribute, false);
		//Infinate duration
		else if (duration == UINT16_MAX)
			set_attribute(gamepad, attribute, true);
		//A timed duration
		else {
			set_attribute(gamepad, attribute, true);
			std::thread new_thread(&VjoyEffect_Constant::thread_main, this, duration);
			vec_threadInfo.push_back(ThreadInfo{ false, new_thread.get_id() });
			new_thread.detach();
		}
	}
	//If a thread has timed out, set the effect to false
	if (completed)
		set_attribute(gamepad, attribute, false);
}

gamepad_map::attribute_vjoy::VjoyEffect_Constant::VjoyEffect_Constant() : VjoyEffect(VjoyEffectType::Constant)
{
}

void gamepad_map::attribute_vjoy::VjoyEffect_Constant::set_attribute(tedpad::Gamepad & gamepad, std::string const & attribute, bool const state) const
{
	switch (gamepad.get_attributeType(attribute)) {
	case tedpad::Gamepad::AttributeType::Digital:
		gamepad.Set_attribute(attribute, state);
		break;
	case tedpad::Gamepad::AttributeType::Analogue:
		gamepad.Set_attribute(attribute, static_cast<uint8_t>(state ? onValue_analogue : 0));
		break;
	case tedpad::Gamepad::AttributeType::Axis:
		gamepad.Set_attribute(attribute, static_cast<int16_t>(state ? onValue_axis : 0));
		break;
	case tedpad::Gamepad::AttributeType::Buffer:
		std::cout << "No conversion from VjoyEffect(" << static_cast<uint16_t>(id) << ") to tedpad attribute (" << attribute << ")" << std::endl;
		break;
	case tedpad::Gamepad::AttributeType::Invalid:
		std::cout << "No conversion from VjoyEffect(" << static_cast<uint16_t>(id) << ") to invalid attribute" << std::endl;
		break;
	default:
		break;
	}
}

void gamepad_map::attribute_vjoy::VjoyEffect_Constant::thread_main(uint16_t const duration)
{
	//Sleep the thread for the duration
	std::this_thread::sleep_for(std::chrono::milliseconds(duration));
	x_vec_threadInfo.lock();
	auto itr = std::find_if(vec_threadInfo.begin(), vec_threadInfo.end(), [](ThreadInfo const &p0) {return(p0.threadId == std::this_thread::get_id()); });
	if (itr != vec_threadInfo.end()) {
		if (!(*itr).invalidated) {
			completed = true;
		}
		vec_threadInfo.erase(itr);
	}
	x_vec_threadInfo.unlock();
}

gamepad_map::AttributeMap & gamepad_map::AttributeMap::operator=(AttributeMap const & p0)
{
	op_assign(p0);
	return(*this);
}

gamepad_map::AttributeMap::AttributeMap(AttributeMap const & p0)
{
	op_assign(p0);
}

void gamepad_map::AttributeMap::op_assign(AttributeMap const & p0)
{
	switch (p0.vjoy_attribute->type) {
	case gamepad_map::attribute_vjoy::VjoyType::Button:
		vjoy_attribute.reset(new attribute_vjoy::VjoyButton(dynamic_cast<attribute_vjoy::VjoyButton const *>(p0.vjoy_attribute.get())->id));
		break;
	case gamepad_map::attribute_vjoy::VjoyType::Axis:
		vjoy_attribute.reset(new attribute_vjoy::VjoyAxis(dynamic_cast<attribute_vjoy::VjoyAxis const *>(p0.vjoy_attribute.get())->id));
		break;
	case gamepad_map::attribute_vjoy::VjoyType::Effect:
		switch (dynamic_cast<attribute_vjoy::VjoyEffect const *>(p0.vjoy_attribute.get())->id) {
		case attribute_vjoy::VjoyEffectType::Constant:
			vjoy_attribute.reset(new attribute_vjoy::VjoyEffect_Constant());
			break;
		case attribute_vjoy::VjoyEffectType::Invalid:
			break;
		default:
			break;
		}
	case gamepad_map::attribute_vjoy::VjoyType::Invalid:
		break;
	default:
		break;
	}
}

gamepad_map::attribute_vjoy::VjoyGeneric::VjoyGeneric(VjoyType const type) : type(type)
{
}

gamepad_map::attribute_vjoy::VjoyEffect::VjoyEffect(VjoyEffectType const id) : VjoyGeneric(VjoyType::Effect), id(id)
{
}
