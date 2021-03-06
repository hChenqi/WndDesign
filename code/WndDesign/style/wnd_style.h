#pragma once

#include "value_tag.h"
#include "scrollbar.h"
#include "border_resizer.h"
#include "../figure/color.h"
#include "../figure/background_types.h"
#include "../system/cursor.h"


BEGIN_NAMESPACE(WndDesign)


struct WndStyle {

	////
	//// styles related with region on parent window
	//// 
	//// (size and position determines the display region of the window on parent window)

	// The size of the window (may be in fixed size or relative to parent window).
	struct LengthStyle {
	public:
		ValueTag _normal = length_auto;
		ValueTag _min = length_min_tag;
		ValueTag _max = length_max_tag;
		constexpr LengthStyle& normal(ValueTag normal) { _normal = normal; return *this; }
		constexpr LengthStyle& min(ValueTag min) { _min = min; return *this; }
		constexpr LengthStyle& max(ValueTag max) { _max = max; return *this; }
		constexpr void setFixed(ValueTag length) { _normal = _min = _max = length; }
	};
	LengthStyle width;
	LengthStyle height;


	// The position of the window, or the margin between the window and the parent window's client region.
	// (relative to the inner edges of parent window)
	struct PositionStyle {
	public:
		ValueTag _left = position_auto;
		ValueTag _top = position_auto;
		ValueTag _right = position_auto;
		ValueTag _bottom = position_auto;
	public:
		constexpr PositionStyle& left(ValueTag left) { _left = left; return *this; }
		constexpr PositionStyle& top(ValueTag top) { _top = top; return *this; }
		constexpr PositionStyle& right(ValueTag right) { _right = right; return *this; }
		constexpr PositionStyle& bottom(ValueTag bottom) { _bottom = bottom; return *this; }
		constexpr void set(ValueTag left, ValueTag top, ValueTag right, ValueTag bottom) { _left = left; _top = top; _right = right; _bottom = bottom; }
		constexpr void setAll(ValueTag all) { _left = _top = _right = _bottom = all; }
		constexpr PositionStyle& setHorizontalCenter() { _left = _right = position_center; return *this; }
		constexpr PositionStyle& setVerticalCenter() { _top = _bottom = position_center; return *this; }
	}position;


	// The composite effect of the window.
	struct CompositeStyle : public CompositeEffect {
	public:
		constexpr CompositeStyle& opacity(uchar opacity) { _opacity = opacity; return *this; }
		constexpr CompositeStyle& blur_radius(uchar blur_radius) { _blur_radius = blur_radius; return *this; }
		constexpr CompositeStyle& z_index(char z_index) { _z_index = z_index; return *this; }
		constexpr CompositeStyle& mouse_penetrate(bool mouse_penetrate) { _mouse_penetrate = mouse_penetrate; return *this; }
	}composite;


	////
	//// styles related with non-client region
	//// 
	//// (border, scrollbar and padding determines the non-client region that is relative to the display region)

	// The border of the window.
	struct BorderStyle {
	public:
		uint _width = 0;
		Color _color = ColorSet::Black;
		uint _radius = 0;
		uint _saved_width = (uint)-1;  // for internal use
		unique_ptr<BorderResizer> _resizer = CreateEmptyBorderResizer();
	public:
		constexpr BorderStyle& width(uint width) { _width = width; return *this; }
		constexpr BorderStyle& color(Color color) { _color = color; return *this; }
		constexpr BorderStyle& radius(uint radius) { _radius = radius; return *this; }
		BorderStyle& setResizer(unique_ptr<BorderResizer> resizer) { _resizer = std::move(resizer); return *this; }
	}border;


	// The scrollbar resource.
	struct ScrollbarStyle {
	public:
		unique_ptr<Scrollbar> _resource = CreateEmptyScrollbar();
	public:
		void set(unique_ptr<Scrollbar> scrollbar_resource) { _resource = std::move(scrollbar_resource); }
	}scrollbar;


	// The padding of the window.
	// (relative to display region, and is extended from the inner side of the border and scroll bar)
	struct PaddingStyle {
	public:
		ValueTag _left = 0px;
		ValueTag _top = 0px;
		ValueTag _right = 0px;
		ValueTag _bottom = 0px;
	public:
		constexpr PaddingStyle& left(ValueTag left) { _left = left; return *this; }
		constexpr PaddingStyle& top(ValueTag top) { _top = top; return *this; }
		constexpr PaddingStyle& right(ValueTag right) { _right = right; return *this; }
		constexpr PaddingStyle& bottom(ValueTag bottom) { _bottom = bottom; return *this; }
		constexpr void set(ValueTag left, ValueTag top, ValueTag right, ValueTag bottom) { _left = left; _top = top; _right = right; _bottom = bottom; }
		constexpr void setAll(ValueTag all) { _left = _top = _right = _bottom = all; }
	}padding;


	////
	//// styles related with client region
	//// 
	//// (client region is relative to displayed-client-region, which is calculated by display region 
	////    subtracts non-client region and determine the size of child windows, or auto fit the size of 
	////    child windows or contents, and the actual accessible region must be larger than client region)
	//// (if accessible region overflows the display region, scroll bar can be drawn)

	// The client region of the window.
	struct ClientStyle {
	public:
		ValueTag _left = 0px;
		ValueTag _top = 0px;
	public:
		LengthStyle width;
		LengthStyle height;
		constexpr ClientStyle& left(ValueTag left) { _left = left; return *this; }
		constexpr ClientStyle& top(ValueTag top) { _top = top; return *this; }
	public:
		ClientStyle() { width.setFixed(100pct); height.setFixed(100pct); }
	}client;


	////
	//// other window styles
	//// 

	// The background of the window.
	struct BackgroundStyle {
	public:
		shared_ptr<Background> _resource = GetNullBackground();
		const Background& Get() const { return *_resource; }
	public:
		void setColor(Color color) { _resource.reset(new SolidColorBackground(color)); }
		void setImage(const wstring& image_file, uchar opacity = 0xFF, Vector offset_on_image = vector_zero) {
			_resource.reset(new ImageRepeatBackground(Image(image_file), opacity, offset_on_image));
		}
		void setImage(const Image& image, uchar opacity = 0xFF, Vector offset_on_image = vector_zero) {
			_resource.reset(new ImageRepeatBackground(image, opacity, offset_on_image));
		}
		void set(shared_ptr<Background> resource) { _resource = resource; }
	}background;


	// cursor style.
	struct CursorStyle {
	public:
		Cursor _cursor = Cursor::Default;
	public:
		void set(Cursor cursor) { _cursor = cursor; }
	}cursor;


	virtual ~WndStyle() {}  // style may contain allocated resources like background or title.
};


END_NAMESPACE(WndDesign)