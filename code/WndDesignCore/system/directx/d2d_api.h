#pragma once

#include "../../common/uncopyable.h"
#include "../../geometry/geometry.h"

#include "directx_resource.h"


struct ID2D1Bitmap1;


BEGIN_NAMESPACE(WndDesign)

class FigureQueue;


inline ID2D1Factory1& GetD2DFactory() { return *DirectXResources::Get().d2d_factory; }
inline ID2D1DeviceContext& GetD2DDeviceContext() { return *DirectXResources::Get().d2d_device_context; }
inline ID2D1SolidColorBrush& GetD2DSolidColorBrush() { return *DirectXResources::Get().d2d_solid_color_brush; }


void BeginDraw();

void EndDraw();


class Target : Uncopyable {
protected:
	// Inherited by WindowTarget to use swap-chain surface as the bitmap.
	ID2D1Bitmap1* bitmap;
public:
	Target(Size size);
	Target(nullptr_t) : bitmap(nullptr) {}
	~Target();

	bool HasBitmap() const { return bitmap != nullptr; }  // Only read-only target doesn't have bitmap.
	ID2D1Bitmap1& GetBitmap() const { assert(HasBitmap()); return *bitmap; }

	void DrawFigureQueue(const FigureQueue& figure_queue, Vector offset, Rect clip_region); // defined in figure_types.cpp
};


END_NAMESPACE(WndDesign)