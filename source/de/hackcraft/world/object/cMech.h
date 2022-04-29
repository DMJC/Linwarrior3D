/**
 * File:     cMech.h
 * Project:  LinWarrior 3D
 * Home:     hackcraft.de
 *
 * Created on March 28, 2008, 10:23 PM (Dez1999)
 */

#ifndef _CMECH_H
#define	_CMECH_H

class cMech;

#include "de/hackcraft/world/Entity.h"

#include <string>
#include <vector>
#include <map>

struct rChatMember;
struct rComcom;
struct rTarcom;
struct rWepcom;
struct rForcom;
struct rNavcom;

struct rBillboard;
struct rTarget;
struct rTraceable;
struct rController;
struct rCamera;
struct rMobile;
struct rRigged;
struct rCollider;

struct rWeapon;
struct rWeaponExplosion;

class rSoundsource;
class rLightsource;
class rInputsource;

class Logger;
class Propmap;

// Gamepad Mapping
#define MECH_CHASSIS_LR_AXIS  AX_LR1
#define MECH_TURRET_LR_AXIS   AX_LR2
#define MECH_TURRET_UD_AXIS   AX_UD2
#define MECH_THROTTLE_AXIS    AX_UD1
#define MECH_FIRE_BUTTON1     BT_R1
#define MECH_FIRE_BUTTON2     BT_R2
#define MECH_JET_BUTTON1      BT_PD
#define MECH_JET_BUTTON2      BT_L2
#define MECH_CAMERA_BUTTON    BT_PU
#define MECH_NEXT_BUTTON      BT_PR
#define MECH_PREV_BUTTON      BT_PL


/**
 * Models Mechlike Objects.
 */
class cMech : public Entity {
private:
    static Logger* logger;
protected:

    /** Instance counter. */
    static int sInstances;

    enum {
        TEXTURE_DESERT,
        TEXTURE_WOOD,
        TEXTURE_SNOW,
        TEXTURE_URBAN
    };

    /** Texture Binds shared between instances. */
    static std::map<int, long> sTextures;

public:

    rChatMember* chatMember;

    // COMPUTERs
    rComcom* comcom;
    rTarcom* tarcom;
    rWepcom* wepcom;
    rForcom* forcom;
    rNavcom* navcom;

    rBillboard* nameable;
    rTarget* target;
    rTraceable* traceable;
    rController* controller;
    rCamera* camra;
    rMobile* mobile;
    rRigged* rigged;
    rCollider* collider;

    // WEAPON: EXPLOSION
    rWeaponExplosion* explosion;

    /** List of WEAPONs. */
    std::vector<rWeapon*> weapons;
    
    rSoundsource* soundsource;
    rLightsource* lightsource;
    rInputsource* inputsource;

public:
    cMech(Propmap* props);
    cMech(float* pos, float* rot, std::string modelName);
    ~cMech();
    
    void init(float* pos, float* rot, std::string modelName);

    // Events
    virtual void spawn();

    // Camera and Headphone
    void camera();
    void listener();

    // Weapons
    void mountWeapon(const char* point, rWeapon *weapon, bool add = true);

    // World Step
    //virtual void message(Message* message); // Conditionally/repeatedly called.
    virtual void animate(float spf);
    virtual void transform();
    virtual void drawSolid();
    virtual void drawEffect();
    virtual void drawHUD();

    // Particle constraining
    virtual void damage(float* localpos, float damage, Entity* enactor = NULL);
    virtual float constrain(float* worldpos, float radius = 0.0f, float* localpos = NULL, Entity* enactor = NULL);
};


#endif	/* _CMECH_H */

