#include "../include/filetype_tpm.h"

std::map<Filetype_tpm_dec::Token const, std::string const> const Filetype_tpm_dec::tokenMap {
	{ Filetype_tpm_dec::Token::Quotation, "\""},
	{ Filetype_tpm_dec::Token::Assign, "="},
	{ Filetype_tpm_dec::Token::GamepadName, "gamepadName" },
	{ Filetype_tpm_dec::Token::MapBegin, "map_begin"},
	{ Filetype_tpm_dec::Token::MapEnd, "map_end"},
	{ Filetype_tpm_dec::Token::effect, "effect"},
	{ Filetype_tpm_dec::Token::constant, "constant"},
	{ Filetype_tpm_dec::Token::onValueAnalogue, "onValueAnalogue"},
	{ Filetype_tpm_dec::Token::onValueAxis, "onValueAxis"},
	{ Filetype_tpm_dec::Token::tedpad, "tedpad"},
	{ Filetype_tpm_dec::Token::vjoy, "vjoy"},
	{ Filetype_tpm_dec::Token::button, "button"},
	{ Filetype_tpm_dec::Token::axis, "axis"},
	{ Filetype_tpm_dec::Token::lx, "lx"},
	{ Filetype_tpm_dec::Token::ly, "ly"},
	{ Filetype_tpm_dec::Token::lz, "lz"},
	{ Filetype_tpm_dec::Token::rx, "rx"},
	{ Filetype_tpm_dec::Token::ry, "ry"},
	{ Filetype_tpm_dec::Token::rz, "rz"}
};

Filetype_tpm_dec::Filetype_tpm::operator gamepad_map::CompleteMap() const
{
	return(completeMap);
}

gamepad_map::CompleteMap Filetype_tpm_dec::Filetype_tpm::generate_completeMap()
{
	gamepad_map::CompleteMap rtrn;
	auto assign_gamepadName = [&](std::vector<Token>::const_iterator &itr) {
		rtrn.gamepadName = get_stringAtPos(++itr);
	};

	std::function<void(std::vector<Token>::const_iterator &)> assignFunction;
	for (auto itr = vec_token.begin(); itr != vec_token.end(); itr++) {
		switch (*itr) {
		case Token::GamepadName:
			assignFunction = assign_gamepadName;
			break;
		case Token::MapBegin:
		{
			auto attributeEndItr = std::find(itr, vec_token.end(), Token::MapEnd);
			rtrn.attribute.push_back(generate_attributeMap(++itr, attributeEndItr));
			itr = attributeEndItr;
			break;
		}
		case Token::Assign:
			assignFunction(itr);
			break;
		default:
			break;
		}
	}
	return(rtrn);
}

Filetype_tpm_dec::Filetype_tpm::Filetype_tpm(std::string const & fileContents)
{
	generate_tokens(fileContents);
	completeMap = generate_completeMap();
}

//TOOD: Remake this so that the vjoy attribute doesn't have to be before all the others

gamepad_map::AttributeMap Filetype_tpm_dec::Filetype_tpm::generate_attributeMap(std::vector<Token>::const_iterator begin, std::vector<Token>::const_iterator const end) const
{
	gamepad_map::AttributeMap rtrn;
	//TODO: Determine whether there is something better than a pointer to a function object here.
	std::function<void()> assignFunction;

	auto assign_tedpad = [&]() {
		rtrn.tedpad_attribute = get_stringAtPos(++begin);
	};

	auto assign_vjoy = [&]() {
		begin++;
		if (*begin == Token::button) {
			//Consider using assignment operator instead of reset here
			rtrn.vjoy_attribute.reset(new gamepad_map::attribute_vjoy::VjoyButton);
			std::stringstream ss_id(get_stringAtPos(++begin));
			//TODO: Determine whether there is a way to use some sort of method other than operator >> to get the numberic value out of this
			uint16_t id;	//Probably needs to be uint16_t since a uint8_t is basically a char
			ss_id >> id;
			dynamic_cast<gamepad_map::attribute_vjoy::VjoyButton *>(rtrn.vjoy_attribute.get())->id = static_cast<uint8_t>(id);
		}
		else if (*begin == Token::axis) {
			rtrn.vjoy_attribute.reset(new gamepad_map::attribute_vjoy::VjoyAxis);
			std::map<Token, gamepad_map::attribute_vjoy::VjoyAxisType> map_axisType{
				{ Token::lx, gamepad_map::attribute_vjoy::VjoyAxisType::LX },
				{ Token::ly, gamepad_map::attribute_vjoy::VjoyAxisType::LY },
				{ Token::lz, gamepad_map::attribute_vjoy::VjoyAxisType::LZ },
				{ Token::rx, gamepad_map::attribute_vjoy::VjoyAxisType::RX },
				{ Token::ry, gamepad_map::attribute_vjoy::VjoyAxisType::RY },
				{ Token::rz, gamepad_map::attribute_vjoy::VjoyAxisType::RZ }
			};
			dynamic_cast<gamepad_map::attribute_vjoy::VjoyAxis *>(rtrn.vjoy_attribute.get())->id = map_axisType.at(*(++begin));
		}
		else if (*begin == Token::effect) {
			if (*(++begin) == Token::constant) {
				rtrn.vjoy_attribute.reset(new gamepad_map::attribute_vjoy::VjoyEffect_Constant);
			}
			/*
			else {
				//Throw an exception here
			}
			*/
		}
		/*
		else {
			//Throw an exception here
		}
		*/
	};

	auto assign_onValueAnalogue = [&]() {
		std::stringstream ss_onValueAnalogue(get_stringAtPos(++begin));
		uint16_t val;
		ss_onValueAnalogue >> val;
		dynamic_cast<gamepad_map::attribute_vjoy::VjoyEffect_Constant *>(rtrn.vjoy_attribute.get())->onValue_analogue = static_cast<uint8_t>(val);
	};
	auto assign_onValueAxis = [&]() {
		std::stringstream ss_onValueAxis(get_stringAtPos(++begin));
		int16_t val;
		ss_onValueAxis >> val;
		dynamic_cast<gamepad_map::attribute_vjoy::VjoyEffect_Constant *>(rtrn.vjoy_attribute.get())->onValue_axis = val;
	};

	for (; begin != end; begin++) {
		switch (*begin) {
		case Token::tedpad:
			assignFunction = assign_tedpad;
			break;
		case Token::vjoy:
			assignFunction = assign_vjoy;
			break;
		case Token::onValueAnalogue:
			assignFunction = assign_onValueAnalogue;
			break;
		case Token::onValueAxis:
			assignFunction = assign_onValueAxis;
			break;
		case Token::MapEnd:
			begin = end;
			break;
		case Token::Assign:
			assignFunction();
			break;
		default:
			break;
		}
	}
	return(rtrn);
}

std::string const &Filetype_tpm_dec::Filetype_tpm::get_stringAtPlaceholder(std::vector<Token>::const_iterator const itr) const
{
	return(*(vec_str.begin() + std::count(vec_token.begin(), itr, Token::StrPlaceholder)));
}

std::string Filetype_tpm_dec::Filetype_tpm::get_stringBetweenQuotes(std::vector<Token>::const_iterator firstQuoteItr) const
{
	std::string rtrn;
	//Increment this by 1 so that it should point to the first strplaceholder
	firstQuoteItr++;
	//If there is a closing quotation before any token that isn't a strplaceholder
	auto lastQuoteItr = std::find(firstQuoteItr , vec_token.end(), Token::Quotation);
	if (lastQuoteItr < std::find_if(firstQuoteItr, vec_token.end(), [](Token const p0) {return((p0 != Token::StrPlaceholder) && (p0 != Token::Quotation)); })) {
		for (; firstQuoteItr != lastQuoteItr; firstQuoteItr++)
			rtrn += get_stringAtPlaceholder(firstQuoteItr);
	}
	return(rtrn);
}

std::string Filetype_tpm_dec::Filetype_tpm::get_stringAtPos(std::vector<Token>::const_iterator const itr) const
{
	return(*itr == Token::Quotation ? get_stringBetweenQuotes(itr) : (*itr == Token::StrPlaceholder ? get_stringAtPlaceholder(itr) : std::string()));
}
