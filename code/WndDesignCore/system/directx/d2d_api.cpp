#include "../../layer/figure_queue.h"

#include "directx_helper.h"
#include "d2d_api.h"
#include "dxgi_api.h"

#pragma comment(lib, "d2d1.lib" )


BEGIN_NAMESPACE(WndDesign)


//////////////////////////////////////////////////////////
////           Device Independent Resources           ////
//////////////////////////////////////////////////////////

BEGIN_NAMESPACE(Anonymous)

class D2DFactory {
private:
    ID2D1Factory1* factory;
    D2DFactory();
    ~D2DFactory();
public:
    static ID2D1Factory1& Get();
};

D2DFactory::D2DFactory() :factory(nullptr) {
	hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		&factory
	);
}

D2DFactory::~D2DFactory() {
	SafeRelease(&factory);
}

ID2D1Factory1& D2DFactory::Get() {
	static D2DFactory factory;
	return *factory.factory;
}

END_NAMESPACE(Anonymous)

WNDDESIGNCORE_API ID2D1Factory1& GetD2DFactory() { return D2DFactory::Get(); }


//////////////////////////////////////////////////////////
////            Device Dependent Resources            ////
//////////////////////////////////////////////////////////


BEGIN_NAMESPACE(Anonymous)

class D2DDeviceContext {
private:
    D2DDeviceContext();
    ~D2DDeviceContext();
public:
    ID2D1DeviceContext* device_context;
    static D2DDeviceContext& Get();
};

D2DDeviceContext::D2DDeviceContext() :device_context(nullptr) {
	IDXGIDevice1* dxgi_device = nullptr;
	hr = GetD3DDevice().QueryInterface(IID_PPV_ARGS(&dxgi_device));

	ID2D1Device* d2d_device = nullptr;
	hr = D2DFactory::Get().CreateDevice(dxgi_device, &d2d_device);
	SafeRelease(&dxgi_device);

	hr = d2d_device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &device_context);
	SafeRelease(&d2d_device);
}

D2DDeviceContext::~D2DDeviceContext() {
	SafeRelease(&device_context);
}

D2DDeviceContext& D2DDeviceContext::Get() {
	static D2DDeviceContext device_context;
	return device_context;
}


inline ID2D1Bitmap1* D2DCreateBitmap(Size size) {
    ID2D1Bitmap1* bitmap;
    D2D1_BITMAP_PROPERTIES1 bitmap_properties = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
    );
    hr = GetD2DDeviceContext().CreateBitmap(
        D2D1::SizeU(size.width, size.height),
        nullptr, 0,
        &bitmap_properties,
        &bitmap
    );
    return bitmap;
}


END_NAMESPACE(Anonymous)


WNDDESIGNCORE_API ID2D1DeviceContext& GetD2DDeviceContext() {
    return *D2DDeviceContext::Get().device_context; 
}


void BeginDraw() {
    auto& device_context = GetD2DDeviceContext();
    device_context.BeginDraw();
}

void EndDraw() {
    auto& device_context = GetD2DDeviceContext();
    hr = device_context.EndDraw();
#pragma message(Remark"If a target was freed after being drawn, EndDraw should fail!")
#pragma message(Remark"And D2DERR_RECREATE_TARGET should be handled.")
    device_context.SetTarget(nullptr);
}


Target::Target(Size size) : bitmap(D2DCreateBitmap(size)) {}
Target::~Target() { SafeRelease(&bitmap); }

void Target::DrawFigureQueue(const FigureQueue& figure_queue, Vector offset, Rect clip_region) {
    ID2D1DeviceContext& device_context = GetD2DDeviceContext(); device_context.SetTarget(&GetBitmap());
    auto& groups = figure_queue.GetFigureGroups();
    auto& figures = figure_queue.GetFigures();
    uint figure_index = 0;
    clip_region = clip_region.Intersect(Rect(point_zero, SIZE2Size(bitmap->GetSize())));
    for (uint group_index = 0; group_index < groups.size(); ++group_index) {
        auto& group = groups[group_index];
        for (; figure_index < group.figure_index; ++figure_index) {
            Vector figure_offset = figures[figure_index].offset + offset;
        #pragma message(Remark"The region of the figure could be cached in the figure queue when it is appended.")
            Rect figure_region = figures[figure_index].figure->GetRegion() + figure_offset;
            if (figure_region.Intersect(clip_region).IsEmpty()) { continue; }
            figures[figure_index].figure->DrawOn(static_cast<RenderTarget&>(device_context), figure_offset);
        }
        if (group.IsBegin()) {
            auto& group_end = groups[group.group_end_index];
            // Calculate new offset and clip region.
            Vector new_offset = offset + group.coordinate_offset;
            Rect new_clip_region = clip_region.Intersect(group.bounding_region + offset);
            // Jump to group end if new clip region is empty.
            if (new_clip_region.IsEmpty()) { 
                group_index = group.group_end_index;
                figure_index = group_end.figure_index;
                continue;
            }
            // Store old offset and clip region to group end.
            group_end.prev_offset = offset;
            group_end.prev_clip_region = clip_region;
            // Set new offset and clip region.
            offset = new_offset;
            clip_region = new_clip_region;
            device_context.PushAxisAlignedClip(Rect2RECT(clip_region), D2D1_ANTIALIAS_MODE_ALIASED);
        } else {
            // Restore to previous offset and clip region.
            offset = group.prev_offset;
            clip_region = group.prev_clip_region;
            device_context.PopAxisAlignedClip();
        }
    }
}


END_NAMESPACE(WndDesign)