/* 
 * File:     rCamera.h
 * Project:  LinWarrior 3D
 * Home:     hackcraft.de
 *
 * Created on April 3, 2011, 8:33 PM
 */

#ifndef RCAMERA_H
#define	RCAMERA_H

struct rCamera;

#include "de/hackcraft/psi3d/math3d.h"

#include "de/hackcraft/world/Component.h"

#include <string>

class Entity;

/**
 * Camera component.
 */
/// @Entity(table="world_camera")
struct rCamera : public Component {
public: // SYSTEM
    /** Identifier for this component (all uppercase letters without leading "r"). */
    static std::string cname;
    /** A unique random number (0-9999) to identify this component. */
    static unsigned int cid;
public: // INPUT
    /** Base orientation (hook i). */
    /// @Column(name="orientation_zero")
    quat ori0;
    /** Base position (hook i). */
    vec3 pos0;
    /** Orientation relative to base position and orientation (hook i). */
    quat ori1;
    /** Position relative to base position and orientation (hook i). */
    vec3 pos1;
    /** Switch camera state one step when greater zero. (hook i) */
    float cameraswitch;
    /** Strength of camera shaking [0,1], scaled internally. (hook i) */
    float camerashake;
public: // OUTPUT
    /** True when camerastate is in first perspective mode. (hook o) */
    bool firstperson;
protected: // INTERNALS
    /** Current Camera mode, negative number is indicating transition. */
    int camerastate;
public:
    /** Constructor */
    rCamera(Entity * obj);
    
    /** Mult in camera matrix. */
    virtual void camera();

    /** Performs camera mode switching (see camera() for random shaking). */
    virtual void animate(float spf);
    
    /** Approximately Gaussian random number. */
    float grand();
};

#endif	/* RCAMERA_H */

