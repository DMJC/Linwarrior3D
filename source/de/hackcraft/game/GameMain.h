/**
 * File:     main.h
 * Project:  LinWarrior 3D
 * Home:     hackcraft.de
 *
 * Created on March 28, 2008, 21:25 PM (1999)
 */

#ifndef _GAMEMAIN_H
#define _GAMEMAIN_H

#include "de/hackcraft/Main.h"

#include <queue>
#include <string>
#include <sstream>
#include <vector>

#include <SDL/SDL.h>

class Entity;
class GameConfig;
class GapBuffer;
class GLFramebuffer;
class Logger;
class GameMission;
class Pad;
class World;

/**
 * Encapsulates low level system-, startup- and io-code.
 * Actually it is just a collection of what would be functions otherwise.
 */
class GameMain : public Main {
private:
    static Logger* logger;
public:
    /** Points to the main instance, necessary because of c-callbacks/threads. */
    static GameMain* instance;
    
private:
    /** Frame-buffer for initial 3d to 2d rendering. */
    std::vector<GLFramebuffer*>* renderbuffer;

    /** Frame-buffer for 2d deferred-/post-processing and effect application. */
    std::vector<GLFramebuffer*>* effectbuffer;

    /** Frame-buffer for final display if the effect-buffer wasn't the final. */
    GLFramebuffer* screenbuffer;
    
    /** Current game configuration. */
    GameConfig* config;
    
    /** Current mission/game controller. */
    GameMission *mission;

    /** Instance of the world we are on. */
    World *world;

    /** Seeing the world through which object's eyes? */
    Entity* camera;
    
    /** Keyboard/Joystick inputs sent to which Virtual-Gamepad? */
    Pad* pad1;
    
    /** Low level joystick handle. */
    void* joy0;

    /** SDL doesn't count mouse wheel movement. */
    int mouseWheel;

    /** Old keystates */
    Uint8 keystate_[512];

    /** stdout is redirected to this stringstream. */
    std::stringstream oss;
    
    /** Old stdout used to restore stdout. */
    std::streambuf* stdout_;

    /** Program output log. */
    GapBuffer *log;

    /** Console terminal program output. */
    GapBuffer *console;

    /** Commandline input buffer for console. */
    GapBuffer *cmdline;

    /** Command & Control Overlay enabled => redirect keyboard */
    bool overlayEnabled;
    
    /** Used to signal forked working threads (log, BGM) to terminate. */
    bool done;

private:
    /** Apply post-processing filter right after drawing frame. */
    void applyEffectFilter(GLFramebuffer* source);
    
    /** Apply optional projection filter after optional post-processing. */
    void applyProjectionFilter(GLFramebuffer* sourceLeft, GLFramebuffer* sourceRight);
    
private:
    /** Initializes called at the start of the run method. */
    int init(int argc, char** args);
    
    /** Initializes called at the end of the run method. */
    void deinit();
    
    /** Sets initial OpenGL mode parameters (dis-/enables and values). */
    void initGL(int width, int height);

    /**
     * Starts and initializes Mission according to the current game attributes.
     */
    void initMission();
    
    /** Handles "special" keys for things like rendering options. */
    void updateKey(Uint8 keysym);

    /** Reads joystick/keyboard input and maps it to to a Gamepad structure. */
    void updatePad(Pad* pad, SDL_Joystick* joy);

    /** Updating the world for the given delta time. */
    void updateFrame(int elapsed_msec);

    /** Draws a single frame for the whole screen. */
    void drawFrame();
    
    /** Draws a single frame for one eye. */
    void drawFramelet(int eye, float shift);

public:
    /** Constructor sets instance pointer among other things. */
    GameMain();
    
    ~GameMain();

    /** Called directly by the main entry point. */
    int run(int argc, char** args);
    
    /** Forked from run/init to shovel output within a concurrent thread. */
    void runOutputJob();
    
    /** Forked from run/init to shovel BGM within a concurrent thread. */
    void runBGMJob();

    /** Enables and disables OpenAL Audio-System. */
    int alEnableSystem(bool en);

    /** Experiment. */
    void drawLog();

    /** Experiment. */
    void drawPlaque();

    /** Experiment. */
    void updateLog();
};


#endif
