#pragma once

#include <map>
#include <string>
#include <vector>
#include <functional>
#include "../tedpad/engine/include/eg/eg_engine.h"
#include "gamepadMap.h"

//TODO: Learn regexes and use them here

namespace Filetype_tpm_dec {
	enum class Token {
		Invalid,
		Quotation,
		StrPlaceholder,
		Assign,
		GamepadName,
		MapBegin,
		MapEnd,
		effect,
		constant,
		onValueAnalogue,
		onValueAxis,
		tedpad,
		vjoy,
		button,
		axis,
		lx,
		ly,
		lz,
		rx,
		ry,
		rz,
	};

	extern std::map<Token const, std::string const> const tokenMap;

	class Filetype_tpm : public eg::util::TextParse<Token, tokenMap> {
	public:
		operator gamepad_map::CompleteMap() const;
		gamepad_map::CompleteMap generate_completeMap();

		Filetype_tpm(std::string const &fileContents);
	private:
		gamepad_map::CompleteMap completeMap;

		gamepad_map::AttributeMap generate_attributeMap(std::vector<Token>::const_iterator begin, std::vector<Token>::const_iterator const end) const;
		std::string const &get_stringAtPlaceholder(std::vector<Token>::const_iterator const itr) const;
		std::string get_stringBetweenQuotes(std::vector<Token>::const_iterator firstQuoteItr) const;
		std::string get_stringAtPos(std::vector<Token>::const_iterator const itr) const;
	};
}
