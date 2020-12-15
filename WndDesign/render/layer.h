#pragma once

#include "tile_cache.h"
#include "composite_style.h"

#include <list>


BEGIN_NAMESPACE(WndDesign)

using std::list;

class WndBase;


class Layer {
	//////////////////////////////////////////////////////////
	////                  Initialization                  ////
	//////////////////////////////////////////////////////////
private:
	WndBase& _parent;
	Rect _composite_region;
	Point _current_point;
	bool _is_redirected;

public:
	Layer(WndBase& parent, Rect composite_region);


	//////////////////////////////////////////////////////////
	////                      Region                      ////
	//////////////////////////////////////////////////////////
public:
	const Rect GetCachedRegion();


	///////////////////////////////////////////////////////////
	////                   Child Windows                   ////
	///////////////////////////////////////////////////////////
public:
	struct ChildWndWrapper {
		WndBase& wnd;
		Rect region;
	};
public:
	void AddChild(WndBase& wnd, Rect region);
	void RemoveChild()

	void DrawChildWnd(ChildWndWrapper& child_wnd);




	//////////////////////////////////////////////////////////
	////                      Render                      ////
	//////////////////////////////////////////////////////////
private:
	// Redraw background and all windows intersecting the region.
	void UpdateRegion(Rect region);


	///////////////////////////////////////////////////////////
	////                     Composite                     ////
	///////////////////////////////////////////////////////////
public:
	[[LayerFigure]]
	void DrawOn(Rect region, DeviceContext& target, Vector offset);
};


class SingleWndLayer : public Layer {
	//////////////////////////////////////////////////////////
	////                  Initialization                  ////
	//////////////////////////////////////////////////////////
public:
	SingleWndLayer(WndBase& child, Rect region, CompositeStyle style);

	CompositeStyle _style;


	///////////////////////////////////////////////////////////
	////                   Child Windows                   ////
	///////////////////////////////////////////////////////////
private:
	WndBase& _child;

};


class MultipleWndLayer : public Layer {
	//////////////////////////////////////////////////////////
	////                  Initialization                  ////
	//////////////////////////////////////////////////////////



	///////////////////////////////////////////////////////////
	////                   Child Windows                   ////
	///////////////////////////////////////////////////////////
private:
	list<ChildWndWrapper> _child_wnds;
};


END_NAMESPACE(WndDesign)