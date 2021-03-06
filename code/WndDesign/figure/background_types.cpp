#include "background_types.h"
#include "figure_types.h"
#include "../geometry/geometry_helper.h"
#include "../geometry/rect_point_iterator.h"


BEGIN_NAMESPACE(WndDesign)


shared_ptr<Background> GetNullBackground() {
	static shared_ptr<Background> null_background(new SolidColorBackground(ColorSet::White));
	return null_background;
}


void SolidColorBackground::DrawOn(Rect region, RenderTarget& target, Vector offset) const {
	Rectangle(region.size, color).DrawOn(target, offset);
}


void ImageRepeatBackground::DrawOn(Rect region, RenderTarget& target, Vector offset) const {
	using RepeatRange = Rect;

	// Calculate the image range that overlaps with the tile region.
	Size image_size = image.GetSize();
	Size target_size = GetTargetSize(target);
	Rect region_on_target = Rect(point_zero, target_size).Intersect(Rect(point_zero + offset, region.size));
	Vector target_to_image_space_conversion = (region.point - point_zero + offset_on_image) - offset;
	Rect region_on_image_space = region_on_target + target_to_image_space_conversion;
	RepeatRange repeat_range = RegionToOverlappingTileRange(region_on_image_space, image_size);
	if (repeat_range.IsEmpty()) { return; }

	// Draw every single image tile on the target.
	ImageFigure image_figure(image, opacity);
	for (RectPointIterator it(repeat_range); !it.Finished(); ++it) {
		Vector tile_offset = ScalePointBySize(it.Item(), image_size) - point_zero;
		Rect drawing_region_on_image_space = Rect(point_zero + tile_offset, image_size).Intersect(region_on_image_space);
		image_figure.region_on_image = drawing_region_on_image_space - tile_offset;
		image_figure.DrawOn(target, drawing_region_on_image_space.point - point_zero - target_to_image_space_conversion);
	}
}


END_NAMESPACE(WndDesign)