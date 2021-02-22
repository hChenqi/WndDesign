#include "../WndDesign/WndDesign.h"
#include "../WndDesign/wnd/ListLayout.h"
#include "../WndDesign/wnd/EditBox.h"


using namespace WndDesign;


#ifdef _DEBUG
#pragma comment(lib, "..\\build\\x64\\Debug\\WndDesign.lib")
#else
#pragma comment(lib, "..\\build\\x64\\Release\\WndDesign.lib")
#endif // _DEBUG


class MainWnd : public ListLayout {
private:
	struct Style : ListLayout::Style {
		Style() {
			width.max(70pct);
			height.max(80pct);
			position.setHorizontalCenter().setVerticalCenter();
			border.width(5).color(ColorSet::DarkGreen).setResizer(CreateDefaultBorderResizer());
			client.width(300px).height(length_auto);
			background.setColor(ColorSet::LightGray);
			grid_height.min(100px).max(300px);
		}
	};
public:
	MainWnd() : ListLayout(std::make_unique<Style>()) { 
		//AllocateLayer(); 
	}
};


class TextArea : public EditBox {
private:
	struct Style : EditBox::Style {
		Style() {
			width.max(100pct);
			height.max(100pct);
			border.width(3).color(ColorSet::Honeydew);
			padding.set(20px, 10px, 20px, 10px);
			client.height(length_auto);
			background.setColor(ColorSet::YellowGreen);
			font.size(20px);
		}
	};
public:
	TextArea(): EditBox(std::make_unique<Style>(), L"Type something here...") {
		//AllocateLayer();
	}
};


int main() {
	MainWnd main_wnd;
	TextArea text_area1, text_area2, text_area3;
	main_wnd.AppendChild(text_area1);
	main_wnd.AppendChild(text_area2);
	main_wnd.AppendChild(text_area3);
	desktop.AddChild(main_wnd);
	desktop.MessageLoop();
	return 0;
}