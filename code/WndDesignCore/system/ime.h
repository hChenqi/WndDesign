#pragma once

#include "../common/core.h"
#include "../figure/color.h"
#include "../geometry/geometry.h"

#include <string>
#include <vector>


BEGIN_NAMESPACE(WndDesign)

using std::wstring;
using std::vector;


// The status of an ongoing ime composition.
struct ImeComposition {
	struct UnderlineStyle {
		uint begin = 0;
		uint end = 0;
		Color color = ColorSet::White;
		bool thick = false;
	};

	uint begin = 0;	// The start position of the selection range.
	uint end = 0;	// The end of the selection range.
	enum class Type : uint { None = 0, CompositionString = 0x0008, ResultString = 0x0800 } type = Type::None;
	wstring string;					   // The string retrieved.
	vector<UnderlineStyle> underlines; // The underline information of the composition string.
};


class WndObject;

WNDDESIGNCORE_API void EnableIME(WndObject& wnd);
WNDDESIGNCORE_API void DisableIME(WndObject& wnd);
WNDDESIGNCORE_API void CancelIME(WndObject& wnd);
WNDDESIGNCORE_API void MoveImeWindow(WndObject& wnd, Rect caret_region);


END_NAMESPACE(WndDesign)