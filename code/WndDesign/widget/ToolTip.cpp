#include "ToolTip.h"
#include "../wnd/DesktopObject.h"
#include "../system/win32.h"


BEGIN_NAMESPACE(WndDesign)


ToolTip& ToolTip::Get() {
	static ToolTip tool_tip;
	return tool_tip;
}


struct ToolTipStyle : public TextBox::Style {
	ToolTipStyle() {
		width.max(200px);
		height.max(500px);
		position.left(0px).top(0px);
		composite.z_index(127).mouse_penetrate(true);  // always topmost, and mouse transparent
		border.width(1).color(ColorSet::Silver);
		padding.setAll(3px);
		background.setColor(0xF1F2F7);
		paragraph.line_height(length_auto);
		font.size(14);
	}
};

static constexpr uint delay_time = 500;
static constexpr uint fade_in_time = 100;
static constexpr uint expire_time = 5000;
static constexpr uint fade_out_time = 200;

/*

										 (expire_time)
				         (fade_in_time)  -------------------------  (fade_out_time)
	(delay_time)         ----------------						  ------------------
	---------------------									 					    --
	^			         ^															^
MouseEnter 	        start_fade_in (show)									       hide

*/


ToolTip::ToolTip() :
	TextBox(std::make_unique<ToolTipStyle>(), L""), 
	timer([]() {}), fade_animation(*this, []() {}) {
}

ToolTip::~ToolTip() {}

void ToolTip::FadeInBegin() {
	timer.Stop();
	Point position = GetCursorPosition();
	GetStyle().position.left(px(position.x - 10)).top(px(position.y + 10));
	GetStyle().composite._opacity = 0;
	fade_animation.callback = std::bind(&ToolTip::FadeInEnd, this);
	fade_animation.Set(fade_in_time, 0xFF);
	desktop.AddChild(*this, [](HANDLE hWnd) { HideWndFromTaskbar(hWnd); });
}

void ToolTip::FadeInEnd() {
	timer.callback = std::bind(&ToolTip::FadeOutBegin, this);
	timer.Set(expire_time);
}

void ToolTip::FadeOutBegin() {
	timer.Stop(); 
	fade_animation.callback = std::bind(&ToolTip::Hide, this);
	fade_animation.Set(fade_out_time, 0);
}

void ToolTip::Hide() {
	timer.Stop(); fade_animation.Stop();
	desktop.RemoveChild(*this);
}


void ToolTip::OnMouseEnter(const wchar text[]) {
	Hide(); SetText(text);
	timer.Set(delay_time); timer.callback = std::bind(&ToolTip::FadeInBegin, this);
}


END_NAMESPACE(WndDesign)