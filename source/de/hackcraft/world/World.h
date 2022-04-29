/**
 * File:     World.h
 * Project:  LinWarrior 3D
 * Home:     hackcraft.de
 *
 * Created on March 28, 2008, 10:23 PM (1999)
 */

#ifndef _CWORLD_H
#define _CWORLD_H

class World;

#include "de/hackcraft/lang/OID.h"

#include "de/hackcraft/log/Logger.h"

#include "de/hackcraft/util/Geomap.h"
#include "de/hackcraft/util/Timing.h"

#include "de/hackcraft/psi3d/macros.h"

#include "de/hackcraft/world/Entity.h"
#include "de/hackcraft/world/Subsystem.h"

#include <list>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

class Entity;

/**
 * The Game-Object-Managing World-Database-Class.
 * Cares about spawning, indexing, searching, messaging, updating, rendering.
 */
class World : public Subsystem {
private:
    static Logger* logger;
public:
    /** The current World instance (singleton) other objects may */
    /** only affect that current instance. */
    static World* instance;

    /** List of subsystems which serve components to model slices of the world. */
    std::list<Subsystem*> subsystems;
    
private:
    /** Not yet dispatched messages (because they are not yet overdue). */
    //std::priority_queue<Message*, std::vector<Message*>, Message> mMessages;

    /** Messages already delivered to group participants. */
    //std::list<Message*> mDispatchedMessages;

    // Not Yet, may be merged with Background:
    // Overlays weather effects like rain, snow or dust.
    //cWeather mWeather;

    /** Object-ID (OID) to Object-Reference (non-fragged). */
    std::map<OID, Entity*> mIndex;

    /** Contains non-fragged objects. */
    std::list<Entity*> mObjects;

    /** Contains fragged objects. */
    std::list<Entity*> mCorpses;

    /** Allows searching the world in a structured manner. */
    Geomap<Entity*> mGeomap;

    /** Non-Positional (NaN) and some oversize objects go here for clustering. */
    std::list<Entity*> mUncluster;

    /** Render only objects that far away. */
    float mViewdistance;
    
    /** Rendering origin or camera position. */
    float mVisorigin[3];
    
    /** Visible objects for next rendering - (re-)set in setupView. */
    std::list<Entity*>* mVisobjects;

    /** Timing, simulation stepping, deltacycles, date, fps, etc. */
    Timing mTiming;
    
    /** Groups objects together for searching, messaging, tagging, ... */
    struct Group {
        /** Group id used for reference (later: use for tags) */
        OID gid;
        /** Group id of parent group if not zero. */
        //OID parent;
        /** Name of the group (use qualified names like: "world/enemies/squads/alpha" or "state/dead") */
        std::string name;
        /** Objects in this group. */
        std::list<OID> members;
    };
    
    /** Groups of objects */
    std::map<OID,Group*> mGroupIndex;

public: // Constructor, Object-Management and Drawing:

    World();

public: // Accessors
    
    static World* getInstance();

    /** Returns the Object ID valid for this instant's time & deltacycle. */
    OID getOID();

    Timing* getTiming();

    Entity* getObject(OID oid);

    void setViewdistance(float viewdistance);

    float getViewdistance();
    
public: // Grouping

    /** Finds and returns a group or creates a new one. */
    OID getGroup(std::string name);

    /** Adds a new member to a existing group or creates a new one, returns gid. */
    void addToGroup(OID gid, Entity* member);

public: // Messaging

    /**
     * Now defunct:
     * Sent messages are currently collected with an attached timestamp.
     * @param delay time offset for sending see getOID() for calculation.
     * @param sender a value of 0 currently defines a broadcast.
     * @param recvid is the receiver the message is sent to - 0 null/def receiver.
     * @param text the message subject/text to send, will be copied to string.
     * @param blob binary data, you give away ownership may be deleted by world.
     */
    void sendMessage(OID delay, OID sender /* = 0 */, OID recvid /* = 0 */, std::string type, std::string text, void* blob = NULL);

public: // Spawning, Fragging, Garbage Collection

    /**
     * Pastes object into active world (mObjects & mIndex),
     * assigns serial OID,
     * increments delta cycle,
     * finally calls object->onSpawn().
     */
    void spawnObject(Entity *object);

    /**
     * Removes object from active world (mObjects & mIndex),
     * adds object to Corpse-List,
     * adds rRole::FRAGGED role to object,
     * finally object->onFragged() is called.
     * Note that FRAGGED != DEAD objects:
     * Dead objects are still part of the world
     * but fragged objects are removed from the perceptible world.
     * Fragged == removed and scheduled for deletion.
     */
    void fragObject(Entity *object);

    /**
     * Deletes fragged objects from Corpse-List.
     * Possibly called very rarely within a Mission at transitional moments.
     * This is like garbage collection on demand.
     */
    void bagFragged();

public: // Simulation Step - Call every frame in order to update the world/mission and draw a frame.

    /** Advance simulation time for one frame. */
    void advanceTime(int deltamsec);
    /** Re-build spatial clustering of objects. */
    void clusterObjects();
    /** Deliver overdue messages to objects. */
    void dispatchMessages();
    /** Let all objects process input, adjust pose and calculate physics. */
    void animateObjects();
    /** Let all objects calculate transformation matrices etc. */
    void transformObjects();
    /** Setup structures for rendering */
    void setupView(float* pos, float* ori);
    /** Draws background (skybox) by calling mBackground drawing method. */
    bool drawBack();
    /** Draw all Object's solid surfaces (calls their drawSolid method). */
    void drawSolid();
    /** Draw all Object's effects (calls their drawEffect method). */
    void drawEffect();

public: // World-Filtering, World-Scanning, World-Sense for objects.

    /**
     * Find objects within 2d area using location hashkeys.
     * 
     * @param min2f First corner of square area (smaller values).
     * @param max2f Second corner of square area (larger values).
     * @param addunclustered Return global unclustered objects, too.
     * @return List of found objects.
     */
    std::list<Entity*>* getGeoInterval(float* min2f, float* max2f, bool addunclustered = false);

    /**
     *
     * @param worldpos The particle position to be constrained given in world coordinates.
     * @param radius The size of the particle in terms of a radius.
     * @return Zero if there was no collision, else the maximum impact depth.
     */
    float constrainParticle(Entity* ex, float* worldpos, float radius);
};

#endif
