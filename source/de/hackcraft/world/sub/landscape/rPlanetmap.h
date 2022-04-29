/* 
 * File:     cPlanetmap.h
 * Project:  LinWarrior 3D
 * Home:     hackcraft.de
 *
 * Created on 18. Juli 2010, 21:39
 */

#ifndef PLANETMAP_H
#define	PLANETMAP_H

class rPlanetmap;

#include "de/hackcraft/world/Component.h"

#define USE_UNORDERED_MAP
#if defined(USE_UNORDERED_MAP)
#include <unordered_map>
#define maptype std::unordered_map
#else
#include <map>
#define maptype std::map
#endif

class Logger;
class Propmap;
struct rTree;

// Amount of cached patches.
#define PLANETMAP_CACHESIZE 512

// Size of cache patch (8 => [256 * 256])
#define PLANETMAP_TILESIZE 7

// Submetre detail (2 => 4th of a metre).
#define PLANETMAP_DIVISIONS 2

/**
 * Procedural Landscape Rendering and Collision.
 */
class rPlanetmap : public Component {
private:
    /** Growth and foliage template class. */
    struct Growth {
        enum Rendertype {
            BILLBOARD,
            CROSS,
            STAR,
            TRIANGLE,
            LEAFS
        };
        long texture;
        float size;
        Rendertype rendertype;
        Growth(long texture, float size, Rendertype rendertype) {
            this->texture = texture;
            this->size = size;
            this->rendertype = rendertype;
        }
    };
    
    /** LRU Surface Cache-Tile. */
    struct sPatch {
        // Number of accesses last frame.
        unsigned long touches;
        // Cache key - spatial index.
        unsigned long key;
        // Cached Surface Data.
        OID heightcolor[(1UL << PLANETMAP_TILESIZE)*(1UL << PLANETMAP_TILESIZE)];
        long normal[(1UL << PLANETMAP_TILESIZE)*(1UL << PLANETMAP_TILESIZE)];
    };
    
public:
    /** Surface Modification */
    struct sMod {
        float pos[3];
        float range;
        float height;
        enum Modtype {
            MODTYPE_FLATSQUARE,
            MODTYPE_SMOOTHSQUARE,
            MODTYPE_SMOOTHROUND,
            MODTYPE_CRATER
        };
        Modtype type;
        sMod() { type = MODTYPE_SMOOTHROUND; pos[0] = pos[1] = pos[2] = 0.0f; range = 10.0f; height = 0.0f; }
        float getModifiedHeight(float x, float y, float h);
    };
    
private:
    static Logger* logger;
    /** Instance counter. */
    static int sInstances;
    /** Constant for 1.0 divided by 256.0 for random number range conversion. */
    static const float oneOver256;
    /** Foliage is generated and rendered in blocks of this size. */
    static const float decalGridSize;
    /** Three times repeating permutation of the numbers 0 to 255 (inclusive). */
    static unsigned char perms[3 * 256];
    /** Instance shared ground textures. */
    static std::vector<long> sGrounds;
    /** Instance shared foliage textures. */
    static std::vector<long> sGrasses;
    /** Growth and foliage template objects. */
    static std::vector<rPlanetmap::Growth*> sGrowth;

public:
    /** Lists all effective surface modifications. */
    std::vector<sMod*> mods;
private:
    /** LRU Surface Cache. */
    maptype<unsigned long, sPatch*> patches;
    /** Tree template for drawing of all trees. */
    rTree* tree;
    /** Landscape type 0-4: grassland, desert etc. */
    int landtype;
    /** Vegetation density use small values near or below one (eg. 0.7). */
    double vegetation;
    /** Ground texture number used in rendering */
    int groundtype;
    
public:
    rPlanetmap();
    rPlanetmap(Propmap* properties);
    /** Remove cached data - enforce recalculation (enforce cache miss). */
    void invalidateCache();
    /** Calculate Height and Color of the xz position. */
    void getHeight(float x, float z, float* const color);
    /** Retrieves Height and Color of the xz position and calculates on demand. */
    void getCachedHeight(float x, float z, float* const color);
    /** Re-adjust particle position by making multiple downward-hemispherical checks. */
    virtual float constrain(float* worldpos, float radius, float* localpos, Entity* enactor);
    /** Animate foliage. */
    virtual void animate(float spf);
    /** Draw Landscape surrounding the current camera position. */
    virtual void drawSolid();
    /** Draw Decals surrounding the current camera position. */
    virtual void drawEffect();
    
protected:
    void init(Propmap* properties);
    
    unsigned int loadMaterial();
    
    float constrainGround(float* worldpos, float radius, float* localpos, Entity* enactor);
    float constrainFoliage(float* worldpos, float radius, float* localpos, Entity* enactor);
    float constrainTrees(float gridX, float gridZ, int visibletrees, unsigned int lfsr16, float* worldpos, float radius, float* localpos, Entity* enactor);
    
    void drawBillboardPlant(float x, float h, float z, float scale, float* unrotateMatrix);
    void drawStarPlant(float x, float h, float z, float scale);
    void drawTrianglePlant(float x, float h, float z, float scale);
    void drawCrossPlant(float x, float h, float z, float scale);
    void drawLeafPlant(float x, float h, float z, float scale);
    void drawStone(float x, float h, float z, float scaleX, float scaleH, float scaleZ);

    void drawStones(float gridX, float gridZ, int visiblestones, float opacity, unsigned int lfsr16);
    void drawGrass(float gridX, float gridZ, float opacity, unsigned int lfsr16);
    void drawPlants(float gridX, float gridZ, int visibleplants, float opacity, int maxplants, float plantscale, float plantdensity, float* billboardMatrix, unsigned int lfsr16);
    void drawTrees(float gridX, float gridZ, int visibletrees, float opacity, unsigned int lfsr16);

};

#endif	/* PLANETMAP_H */

