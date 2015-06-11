#ifndef MBGL_MAP_TILE_WORKER
#define MBGL_MAP_TILE_WORKER

#include <mbgl/util/variant.hpp>
#include <mbgl/map/tile_data.hpp>
#include <mbgl/geometry/elements_buffer.hpp>
#include <mbgl/geometry/fill_buffer.hpp>
#include <mbgl/geometry/line_buffer.hpp>
#include <mbgl/util/noncopyable.hpp>
#include <mbgl/style/filter_expression.hpp>

#include <string>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace mbgl {

class CollisionTile;
class GeometryTile;
class Style;
class Bucket;
class StyleBucket;
class GeometryTileLayer;

using TileParseResult = mapbox::util::variant<
    TileData::State,     // success
    std::exception_ptr>; // error

class TileWorker : public util::noncopyable {
public:
    TileWorker(const TileID&,
               Style&,
               const uint16_t maxZoom,
               const std::atomic<TileData::State>&,
               std::unique_ptr<CollisionTile>);
    ~TileWorker();

    Bucket* getBucket(const StyleLayer&) const;
    size_t countBuckets() const;

    TileParseResult parse(const GeometryTile&);
    void redoPlacement(float angle, bool collisionDebug);

    Style& style;

private:
    void parseLayer(const StyleLayer&, const GeometryTile&);

    std::unique_ptr<Bucket> createFillBucket(const GeometryTileLayer&, const StyleBucket&);
    std::unique_ptr<Bucket> createLineBucket(const GeometryTileLayer&, const StyleBucket&);
    std::unique_ptr<Bucket> createSymbolBucket(const GeometryTileLayer&, const StyleBucket&);

    template <class Bucket>
    void addBucketGeometries(Bucket&, const GeometryTileLayer&, const FilterExpression&);

    const TileID id;
    const uint16_t maxZoom;
    const std::atomic<TileData::State>& state;

    bool partialParse = false;

    FillVertexBuffer fillVertexBuffer;
    LineVertexBuffer lineVertexBuffer;

    TriangleElementsBuffer triangleElementsBuffer;
    LineElementsBuffer lineElementsBuffer;

    std::unique_ptr<CollisionTile> collision;

    // Contains all the Bucket objects for the tile. Buckets are render
    // objects and they get added to this map as they get processed.
    // Tiles partially parsed can get new buckets at any moment but are
    // also fit for rendering. That said, access to this list needs locking
    // unless the tile is completely parsed.
    std::unordered_map<std::string, std::unique_ptr<Bucket>> buckets;
    mutable std::mutex bucketsMutex;
};

}

#endif
