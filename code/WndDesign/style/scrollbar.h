#pragma once

#include "../geometry/geometry.h"
#include "../geometry/margin.h"
#include "../message/msg_base.h"

#include <memory>


BEGIN_NAMESPACE(WndDesign)

using std::unique_ptr;

class FigureQueue;


class Scrollbar {
public:
	Scrollbar() {}
	virtual ~Scrollbar() {}
	virtual bool IsAuto() const { return false; }
	virtual const Margin GetMargin() const { return {}; }
	virtual bool Update(Rect accessible_region, Rect display_region) { return false; }

	virtual bool HitTest(Point point, Size display_size) { return false; }
	virtual bool Handler(Msg msg, Para para) { return false; }
	virtual bool IsVisible() const { return false; }
	virtual void OnPaint(FigureQueue& figure_queue, Size display_size) {}
};


unique_ptr<Scrollbar> CreateDefaultScrollbar();


END_NAMESPACE(WndDesign)