/* 
 * File:     rAlert.h
 * Project:  LinWarrior 3D
 * Home:     hackcraft.de
 *
 * Created on September 24, 2011, 2:57 PM
 */

#ifndef RALERT_H
#define	RALERT_H

class rAlert;

#include "de/hackcraft/lang/OID.h"

#include "de/hackcraft/world/Component.h"

#include <hash_set>
#include <set>
#include <string>

class Entity;
class rTrigger;

/**
 * Checks a defined area for certain intruding trigger-objects
 * (most likely the player). When such an intrusion is
 * detected a message is sent to a given group of objects.
 * Those receivers may then react on that message.
 *
 * Some ideas:
 * - Enemies powering up as soon as you enter a zone (pop WAIT state).
 * - Enemies collectively attacking the intruder (push ATTACK target).
 * - Allied backup moving in after you (push GOTO destination).
 * - Joining you when reaching a roundevous point (push FOLLOW target).
 * - Warning and mission abort when leaving mission area.
 *
 * Some further ideas:
 * Message aggregation and processing objects for and, or, xor, forwarding,
 * would allow extended mission scripting.
 * Likewise switching alerts on and off would present opportunities.
 *
 * A cone or pyramid for detection would make a nice surveilance camera.
 *
 */
class rAlert : public Component {
public: // SYSTEM
    /** Identifier for this component (all uppercase letters without leading "r"). */
    static std::string cname;
    /** A unique random number (0-9999) to identify this component. */
    static unsigned int cid;
    /** Draw zones for debugging. */
    static bool sDrawzone;
    /** Id */
    OID id;
public: // INPUT
    /** The group of receivers. */
    OID receiver;
    /** Use positive shape (true) or negative of shape (false) as zone? */
    bool positive;
    /** Fire message when entering (true) or exiting (false) the zone? */
    bool posedge;
    /** Fire message only once (true) or always when conditions are true? */
    bool once;
    /** Delay for the delivery of the notification message. */
    OID fusedelay;
    /** Reacting only on triggers with such Tags: */
    std::set<OID> inc_sense;
    /** But not reacting on triggers with such Tags: */
    std::set<OID> exc_sense;
    /** Type of the message to be sent when triggered. */
    std::string msgtype;
    /** Text of the message to be sent when triggered. */
    std::string msgtext;
public: // OUTPUT
    /** Overall radius of this alert zone for clustering. */
    float radius;
    /** Set true when the message was fired at least once - initially false. */
    bool fired;
    /** Lists object currently inside the tracked zone. */
    std::set<OID> intruders;
public:
    struct rShape {

        enum Shapes {
            CYLINDER, BOX, SPHERE, CONE, MAX_SHAPES
        };
        int type;
        float center[3];
        float range[3];
    };
protected: // INTERNALS
    rShape shape;
public:

    rAlert(Entity* obj, float* center, float* range, int shapetype, std::string msgtype, std::string msgtext, OID receiver, std::set<OID>* include, std::set<OID>* exclude, bool positive = true, bool posedge = true, bool once = false, OID fusedelay = 0);

    float detect(rTrigger* trigger);
    virtual void drawSystemEffect();    
};

#endif	/* RALERT_H */

