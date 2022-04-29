#include "MobileSystem.h"

#include "de/hackcraft/log/Logger.h"

#include "de/hackcraft/world/World.h"

#include "de/hackcraft/world/sub/mobile/rMobile.h"


Logger* MobileSystem::logger = Logger::getLogger("de.hackcraft.world.sub.mobile.MobileSystem");

MobileSystem* MobileSystem::instance = NULL;


MobileSystem* MobileSystem::getInstance() {
    return instance;
}


MobileSystem::MobileSystem() {
    instance = this;
}


MobileSystem::~MobileSystem() {
}


void MobileSystem::add(rMobile* mobile) {
    mobiles.push_back(mobile);
    mobilesIndex[mobile->getId()] = mobile;
}


void MobileSystem::animateObjects() {
    
    float spf = World::getInstance()->getTiming()->getSPF();
    
    for(rMobile* component : mobiles) {
        component->prebind();
        component->animate(spf);
        component->postbind();
    }
}

