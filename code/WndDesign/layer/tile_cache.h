#pragma once

#include "../common/uncopyable.h"
#include "../geometry/geometry.h"

#include <unordered_map>


BEGIN_NAMESPACE(WndDesign)

using std::unordered_map;

class Target;
class Layer;

using TileID = Point;
using TileRange = Rect;

struct TileIDHasher {
	size_t operator()(TileID tile_id) const noexcept {
		static_assert(sizeof(TileID) == sizeof(size_t));   // 64bit
		return *reinterpret_cast<size_t*>(&tile_id);
	}
};


// Tile cache implementation using hash table.
class TileCache : public Uncopyable {
private:
	Size _tile_size;
	TileRange _cached_tile_range;
	Rect _cached_region;

	uint _max_capacity;
	unordered_map<TileID, Target, TileIDHasher> _cache;

	unique_ptr<Target> _tile_read_only;

private:
	void SwapOut();
	void SetMaxCapacity(uint max_capacity);

public:
	const Size GetTileSize() const { return _tile_size; }
	bool ResetTileSize(Size layer_size);

	const Rect GetCachedRegion() const { return _cached_region; }
	void SetCachedRegion(Rect accessible_region, Rect visible_region);

	void Clear() { _cache.clear(); }

	const Target& ReadTile(TileID tile_id) const;
	Target& WriteTile(TileID tile_id, Layer& layer);
};


END_NAMESPACE(WndDesign)