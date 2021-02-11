#pragma once

#include "wnd_base_interface.h"
#include "../common/reference_wrapper.h"
#include "../common/list_iterator.h"
#include "../geometry/region.h"

#include <list>
#include <memory>


BEGIN_NAMESPACE(WndDesign)

using std::list;
using std::unique_ptr;

class Layer;


class WndBase : public IWndBase, public Uncopyable {
	//////////////////////////////////////////////////////////
	////                  Initialization                  ////
	//////////////////////////////////////////////////////////
protected:
	WndObject& _object;

public:
	WndBase(WndObject& object);
	~WndBase();

	friend class DesktopBase;
	friend class DesktopWndFrame;


	///////////////////////////////////////////////////////////
	////                   Parent Window                   ////
	///////////////////////////////////////////////////////////
private:
	ref_ptr<WndBase> _parent;
	list<ref_ptr<WndBase>>::iterator _index_on_parent;
public:
	bool HasParent() const { return _parent != nullptr; }
private:
	/* called by new parent window */
	void SetParent(ref_ptr<WndBase> parent, list<ref_ptr<WndBase>>::iterator index_on_parent);
	/* called by old parent window */
	void ClearParent();
	/* called by myself */
	void DetachFromParent();


	//// window depth ////
	// Depth is used to determine the redraw queue priority. 
	// For Desktop, depth is 0; for desktop window and its descendants, depth is 1, 2, 3, ...
	//   and for windows who are not descendant of Desktop, depth is -1.
private:
	uint _depth;
private:
	bool IsDepthValid() const { return _depth != -1; }
	uint GetChildDepth() const { return _depth == -1 ? -1 : _depth + 1; }
public:
	void SetDepth(uint depth);
	uint GetDepth() const { return _depth; }


	///////////////////////////////////////////////////////////
	////                   Child Windows                   ////
	///////////////////////////////////////////////////////////
private:
	list<ref_ptr<WndBase>> _child_wnds;
public:
	virtual void AddChild(IWndBase& child_wnd) override;
	virtual void RemoveChild(IWndBase& child_wnd) override;


	//////////////////////////////////////////////////////////
	////                      Region                      ////
	//////////////////////////////////////////////////////////
private:
	// The entire region of the window.
	Rect _accessible_region;
	// The display region's offset, in my coordinates, size is determined by parent window.
	// Rect(display_region_offset, display_region.size) must be contained in accessible_region.
	Point _display_offset;
	// The region shown on parent window, in parent's coordinates.
	Rect _region_on_parent;
	// The region, in my coordinates, that is cached on parent layer and must be composited.
	//   (Parent's cached region relative to child is child's visible region.)
	Rect _visible_region;

public:
	virtual const Rect GetAccessibleRegion() const override { return _accessible_region; }
	virtual const Rect GetDisplayRegion() const override { return Rect(_display_offset, _region_on_parent.size); }
	virtual const Rect GetRegionOnParent() const override { return _region_on_parent; }
	virtual void SetAccessibleRegion(Rect accessible_region) override;
	virtual const Vector SetDisplayOffset(Point display_offset) override;  // returns the display_offset change.
	virtual void SetRegionOnParent(Rect region_on_parent) override;

public:
	const Vector GetDisplayOffset() const { return _display_offset - point_zero; }
	// point_on_parent + offset_from_parent = point_on_myself
	const Vector OffsetFromParent() const { return _display_offset - _region_on_parent.point; }


	//// reflow queue ////
private:
	friend class ReflowQueue;
	list_iterator<ref_ptr<WndBase>> _reflow_queue_index;
private:
	void JoinReflowQueue();
	void LeaveReflowQueue();
	virtual void InvalidateLayout() override { JoinReflowQueue(); }


	//// invalid layout ////
public:
	void MayRegionOnParentChange();
	void UpdateInvalidLayout();


	///////////////////////////////////////////////////////////
	////                    Composition                    ////
	///////////////////////////////////////////////////////////

	//// background ////
private:
	reference_wrapper<const Background> _background;
public:
	virtual void SetBackground(const Background& background) override { _background = background; }


	//// layer management ////
private:
	unique_ptr<Layer> _layer;
private:
	bool HasLayer() const { return _layer != nullptr; }
public:
	virtual void AllocateLayer() override;

private:
	virtual const Rect GetCachedRegion() const;
	void ResetVisibleRegion() { SetVisibleRegion(HasParent() ? _parent->GetCachedRegion() : region_empty); }
	void SetVisibleRegion(Rect parent_cached_region);


	//// redraw queue ////
private:
	friend class RedrawQueue;
	list_iterator<ref_ptr<WndBase>> _redraw_queue_index;
private:
	void JoinRedrawQueue();
	void LeaveRedrawQueue();


	//// invalid region ////
private:
	Region _invalid_region;
private:
	/* called by child window when child has updated invalid region */
	virtual void InvalidateChild(WndBase& child, Region& child_invalid_region);
	virtual void InvalidateChild(IWndBase& child, Rect child_invalid_region) override;
public:
	virtual void Invalidate(Rect region) override;
	/* called by redraw queue at commit time */
	void UpdateInvalidRegion();
	/* called by parent window (WndObject) , the coordinate space of figure_queue now is parent's client region */
	virtual void Composite(FigureQueue& figure_queue, Vector client_offset, Rect parent_invalid_region) const override;


	///////////////////////////////////////////////////////////
	////                      Message                      ////
	///////////////////////////////////////////////////////////
private:
	ref_ptr<WndBase> _capture_child;	   // child, this, or nullptr
	ref_ptr<WndBase> _focus_child;		   // child, this, or nullptr
	ref_ptr<WndBase> _last_tracked_child;  // child or nullptr
private:
	void ChildLoseCapture();
	void ChildLoseFocus();
	void ChildLoseTrack();
private:
	virtual void SetChildCapture(WndBase& child);
	virtual void SetChildFocus(WndBase& child);
public:
	virtual void SetCapture() override { SetChildCapture(*this); }
	virtual void SetFocus() override { SetChildFocus(*this); }
	virtual void ReleaseCapture() override;
	virtual void ReleaseFocus() override;
private:
	virtual bool SendChildMessage(IWndBase& child_wnd, Msg msg, Para para) override;
	bool HandleMessage(Msg msg, Para para);
	bool DispatchMessage(Msg msg, Para para);
};


END_NAMESPACE(WndDesign)