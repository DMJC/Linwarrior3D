#include "rWeaponMachinegun.h"

#include "de/hackcraft/psi3d/GLF.h"
#include "de/hackcraft/psi3d/GLS.h"
#include "de/hackcraft/psi3d/Primitive.h"

#include <cassert>


std::string rWeaponMachinegun::cname = "MACHINEGUN";
unsigned int rWeaponMachinegun::cid = 7743;


rWeaponMachinegun::rWeaponMachinegun(Entity* obj) {
    
    object = obj;

    //cout << "cMachineGun()\n";
    clipSize = 75;
    depotSize = 2;
    remainingAmmo = clipSize;
    remainingClips = depotSize;
    if (WEAPONSOUND) {
        loadSource("/org/freesound/weapon/machinegun.wav", 1.0f, 1.5f);
    }
}


void rWeaponMachinegun::fire() {
    
    if (!ready()) return;
    
    triggered = true;

    if (remainingAmmo > 0) {
        remainingAmmo--;
        if (remainingAmmo == 0 && remainingClips != 0) {
            timeReloading = 2.5;
        } else {
            timeReloading = 0.5;
        }
    }

    float* source = weaponPosef;
    float* pos = &source[12];

    {
        Particle* s = new Particle();
        assert(s != NULL);
        s->fuel = 4;

        vector_cpy(s->pos, pos);

        float nrm[3];
        float pos2[] = {0, +3, 0};
        matrix_apply2(source, pos2);
        vector_sub(nrm, pos2, s->pos);

        vector_scale(s->vel, nrm, 1.4);
        const float variation = 0.01;
        loopi(3) s->vel[i] += (50 - (rand() % 100)) * variation;
        castoffParticles.push_back(s);
    }


    {
        Particle* s = new Particle();
        assert(s != NULL);
        s->fuel = 1.0f;

        vector_cpy(s->pos, pos);

        float nrm[3];
        float pos2[] = {0, 0, -1};
        matrix_apply2(source, pos2);
        vector_sub(nrm, pos2, s->pos);

        vector_scale(s->vel, nrm, 200);
        const float variation = 0.03;
        loopi(3) s->vel[i] += (50 - (rand() % 100)) * variation;
        shrapnelParticles.push_back(s);
    }

    playSourceIfNotPlaying();
}


void rWeaponMachinegun::animate(float spf) {
    
    triggereded = triggered;
    if (trigger) fire();
    trigger = false;

    for (std::list<Particle*>::iterator i = shrapnelParticles.begin(); i != shrapnelParticles.end();) {
        float damage = 10;
        Particle* s = *i++;
        float dt = spf * 0.2f;
        int damaged = 0;

        loopi(5) {
            vector_muladd(s->pos, s->pos, s->vel, dt);
            damaged = this->damageByParticle(s->pos, 0.2f, 0, damage);
            if (damaged != 0) break;
        }
        s->fuel -= spf;
        if (s->fuel < 0 || damaged != 0) {
            shrapnelParticles.remove(s);
            delete s;
        }
    }
    
    float gravity = -9.80665f;

    foreachNoInc(i, castoffParticles) {
        Particle* s = *i++;
        s->vel[1] += gravity * spf;
        vector_muladd(s->pos, s->pos, s->vel, spf);
        s->fuel -= spf;
        if (s->fuel < 0) {
            castoffParticles.remove(s);
            delete s;
        }
    }

    foreachNoInc(i, damageParticles) {
        Particle* s = *i++;
        vector_muladd(s->pos, s->pos, s->vel, spf);
        s->fuel -= spf;
        if (s->fuel < 0) {
            damageParticles.remove(s);
            delete s;
        }
    }

    timeReloading -= spf;
    if (timeReloading < 0) {
        timeReloading = 0.0f;
        if (remainingAmmo == 0 && remainingClips != 0) {
            remainingAmmo = clipSize;
            if (remainingClips > 0) remainingClips--;
        }
    }
}


void rWeaponMachinegun::drawSolid() {
    
    if (drawWeapon) {
        GL::glPushAttrib(GL_ALL_ATTRIB_BITS);
        {
            GLS::glUseProgram_fglitcolor();

            GL::glPushMatrix();
            {
                transformTo();
                GL::glRotatef(-90, 1, 0, 0);

                // Scale
                float scale = weaponScale;
                GL::glScalef(scale, scale, scale);

                GL::glColor4f(0.2, 0.3, 0.2, 1.0);
                GL::glPushMatrix();
                {
                    GL::glScalef(0.1, 0.14, 0.12);
                    Primitive::glCenterUnitBlock();
                }
                GL::glPopMatrix();

                if (ready() == 0 && remainingAmmo != 0 && object != NULL) {
                    GL::glRotatef(object->seconds * 8 * 90, 0, 1, 0);
                }

                GL::glColor4f(0.3, 0.3, 0.3, 1.0);
                GL::glPushMatrix();
                {
                    GL::glTranslatef(0, 0.75, 0);
                    GL::glScalef(0.065, 0.75, 0.065);
                    Primitive::glCenterUnitCylinder(7);

                }
                GL::glPopMatrix();

                GL::glColor4f(0.1, 0.1, 0.1, 1.0);
                int n = 3;

                loopi(n) {
                    GL::glPushMatrix();
                    {
                        GL::glRotatef(i * 360 / n, 0, 1, 0);
                        GL::glTranslatef(0.075, 0.8, 0);
                        GL::glScalef(0.03, 0.8, 0.03);
                        Primitive::glCenterUnitCylinder(7);
                    }
                    GL::glPopMatrix();
                }
            }
            GL::glPopMatrix();
        }
        GL::glPopAttrib();
    }
}


void rWeaponMachinegun::drawEffect() {
    
    if (shrapnelParticles.empty() && castoffParticles.empty() && damageParticles.empty()) return;

    GL::glPushAttrib(GL_ALL_ATTRIB_BITS);
    {
        GLS::glUseProgram_fgaddcolor();

        GL::glLineWidth(2);

        foreachNoInc(i, shrapnelParticles) {
            Particle* s = *i++;
            const float len1 = 0.01;
            const float len2 = 0.02;
            GL::glBegin(GL_LINE_STRIP);
            GL::glColor4f(1, 1, 1, 1);
            GL::glVertex3fv(s->pos);
            GL::glColor4f(1, 1, 0, 1);
            GL::glVertex3f(s->pos[0] - s->vel[0] * len1, s->pos[1] - s->vel[1] * len1, s->pos[2] - s->vel[2] * len1);
            GL::glColor4f(1, 0, 0, 0.3);
            GL::glVertex3f(s->pos[0] - s->vel[0] * len2, s->pos[1] - s->vel[1] * len2, s->pos[2] - s->vel[2] * len2);
            GL::glEnd();
        }

        foreachNoInc(i, castoffParticles) {
            Particle* s = *i++;
            const float len2 = 0.02;
            GL::glBegin(GL_LINE_STRIP);
            GL::glColor4f(1, 1, 1, 1);
            GL::glVertex3fv(s->pos);
            //GL::glColor4f(1, 1, 0, 1);
            //GL::glVertex3f(s->pos[0] - s->vel[0] * len1, s->pos[1] - s->vel[1] * len1, s->pos[2] - s->vel[2] * len1);
            GL::glColor4f(1, 0, 0, 0.3);
            GL::glVertex3f(s->pos[0] - s->vel[0] * len2, s->pos[1] - s->vel[1] * len2, s->pos[2] - s->vel[2] * len2);
            GL::glEnd();
        }

        float n[16];
        GLS::glGetTransposeInverseRotationMatrix(n);

        foreachNoInc(i, damageParticles) {
            Particle* s = *i++;
            GL::glPushMatrix();
            {
                GL::glTranslatef(s->pos[0], s->pos[1], s->pos[2]);
                GL::glScalef(0.6, 0.6, 0.8);
                GL::glMultMatrixf(n);
                GL::glColor4f(1, 0, 0, 0.7);
                GLF::glprint(((std::string*)s->data)->c_str());
            }
            GL::glPopMatrix();
        }

    }
    GL::glPopAttrib();
}


void rWeaponMachinegun::drawHUD() {
    
    float a = 0.9;
    float b = 0.6;

    // Ammo
    float i = 0.1;
    float j = 0.9;
    float r = i + (remainingAmmo / (float) clipSize) * (j - i);
    GL::glBegin(GL_QUADS);
    GL::glColor4f(1, 1, 0, b);
    GL::glVertex3f(i, j, 0.0);
    GL::glVertex3f(i, i, 0.0);
    GL::glColor4f(1, 0, 0, b);
    GL::glVertex3f(r, i, 0.0);
    GL::glVertex3f(r, j, 0.0);
    GL::glEnd();

    // Clips
    float l = 0.1;
    float h = 0.15;
    r = i + (remainingClips / (float) depotSize) * (j - i);
    GL::glBegin(GL_QUADS);
    GL::glColor4f(0, 0, 0, a);
    GL::glVertex3f(l, i + h, 0.0);
    GL::glVertex3f(l, i, 0.0);
    GL::glColor4f(1, 1, 1, a);
    GL::glVertex3f(r, i, 0.0);
    GL::glVertex3f(r, i + h, 0.0);
    GL::glEnd();

    GL::glBegin(GL_LINES);
    GL::glColor4f(1, 1, 0, b);
    GL::glVertex3f(0.25, 0.5, 0.0);
    GL::glColor4f(1, 1, 1, a);
    GL::glVertex3f(0.35, 0.5, 0.0);
    GL::glColor4f(1, 1, 0, b);
    GL::glVertex3f(0.45, 0.5, 0.0);
    GL::glColor4f(1, 1, 1, a);
    GL::glVertex3f(0.55, 0.5, 0.0);
    GL::glColor4f(1, 1, 0, b);
    GL::glVertex3f(0.65, 0.5, 0.0);
    GL::glColor4f(1, 1, 1, a);
    GL::glVertex3f(0.75, 0.5, 0.0);
    GL::glEnd();
}

