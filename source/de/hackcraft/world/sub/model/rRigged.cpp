#include "rRigged.h"

#include "de/hackcraft/io/Filesystem.h"

#include "de/hackcraft/log/Logger.h"

#include "de/hackcraft/opengl/GL.h"

#include "de/hackcraft/proc/Solid.h"

#include "de/hackcraft/psi3d/GLS.h"
#include "de/hackcraft/psi3d/Primitive.h"
#include "de/hackcraft/psi3d/GLF.h"


#include <sstream>
#include <string>


#define DRAWJOINTS !true

#define MECHDETAIL 0


Logger* rRigged::logger = Logger::getLogger("de.hackcraft.world.sub.model.rRigged");

std::string rRigged::cname = "RIGGED";
unsigned int rRigged::cid = 4900;

std::map<std::string,unsigned long> rRigged::materials;


rRigged::rRigged(Entity* obj) : scale(1.0f), seconds(0.0f), grounded(0.0f), jetting(0.0f), basetexture3d(0), joints(NULL), height(0.1f), radius(0.1f), model(NULL) {
    object = obj;
    vector_zero(pos0);
    vector_zero(vel);
    quat_zero(ori0);
    initMaterials();
    for (int i = 0; i < MAX_JOINTPOINTS; i++) {
        rotators[i][0] = rotators[i][1] = rotators[i][2] = 0;
        rotatorsFactors[i][0] = rotatorsFactors[i][1] = rotatorsFactors[i][2] = 1;
    }
}


rRigged::~rRigged() {
    delete model;
    delete[] joints;
}


std::string rRigged::getJointname(unsigned int num) {
    const char* names[] = {
        "EYE", "HEADPITCH", "HEADYAW",
        "CTMOUNT", "LAMOUNT", "RAMOUNT", "LSMOUNT", "RSMOUNT", "BKMOUNT",
        "JET0", "JET1", "JET2", "JET3", "JET4",
        "YAW", "PITCH", "LEFTLEG", "RIGHTLEG", "LEFTCALF", "RIGHTCALF", "LEFTFOOT", "RIGHTFOOT"
    };
    if (num >= MAX_JOINTPOINTS) return std::string("");
    return std::string(names[num]);
}


int rRigged::getMountpoint(const char* point) {
    int jp = 0;
    if (strcmp(point, "LTorsor") == 0) jp = LSMOUNT;
    else if (strcmp(point, "LUpArm") == 0) jp = LSMOUNT;
    else if (strcmp(point, "LLoArm") == 0) jp = LAMOUNT;
    else if (strcmp(point, "RTorsor") == 0) jp = RSMOUNT;
    else if (strcmp(point, "RUpArm") == 0) jp = RSMOUNT;
    else if (strcmp(point, "RLoArm") == 0) jp = RAMOUNT;
    else if (point[0] == 'L') jp = LSMOUNT;
    else if (point[0] == 'R') jp = RSMOUNT;
    else if (point[0] == 'C') jp = CTMOUNT;
    else jp = BKMOUNT;
    return jointpoints[jp];
}


std::string rRigged::resolveFilename(std::string modelname) {
    std::map<std::string,std::string> m2f;
    
    m2f["frogger"] = "/base/model/wanzer/frogger/frogger.md5mesh";
    m2f["gorilla_ii"] = "/base/model/wanzer/gorilla/gorilla_ii.md5mesh";
    m2f["lemur"] = "/base/model/wanzer/lemur/lemur.md5mesh";
    m2f["kibitz"] = "/base/model/wanzer/kibitz/kibitz.md5mesh";

    m2f["pod"] = "/base/model/turret/pod/pod.md5mesh";

    m2f["bug"] = "/base/model/tank/bug/bug.md5mesh";
    m2f["ant"] = "/base/model/tank/ant/ant.md5mesh";
    m2f["warbuggy"] = "/base/model/tank/warbuggy/warbuggy.md5mesh";

    m2f["flopsy"] = "/com/blendswap/model/flopsy/flopsy.md5mesh";

    m2f["scorpion"] = "/org/opengameart/model/scorpion/scorpion.md5mesh";
    m2f["thunderbird"] = "/org/opengameart/model/thunderbird/thunderbird.md5mesh";

    m2f["gausscan"] = "/org/opengameart/model/gausscan/gausscan.md5mesh";
    m2f["twinblaster"] = "/org/opengameart/model/twinblaster/twinblaster.md5mesh";
    m2f["reactor"] = "/org/opengameart/model/reactor/reactor.md5mesh";

    m2f["soldier"] = "/media/44EA-7693/workspaces/mm3d/soldier/soldier.md5mesh";
    
    // if the model name isn't a shortcut then assume it's a path+filename.
    if (m2f.find(modelname) == m2f.end()) {
        return modelname;
    }

    return m2f[modelname];
}


void rRigged::loadModel(std::string filename) {
    model = MD5Format::mapMD5Mesh(filename.c_str());

    if (!true) logger->debug() << MD5Format::getModelStats(model) << "\n";

    // Make joints local for later animation and transformation to global.
    MD5Format::joint* joints_ = MD5Format::getJoints(model);
    MD5Format::toLocalJoints(model->numJoints, joints_, joints_);

    // Rotate around X to swap Y/Z.
    quat convaxes;
    vec3 xaxis = {1, 0, 0};
    quat_rotaxis(convaxes, (-0.5f * M_PI), xaxis);
    quat_mul(joints_[0].q, joints_[0].q, convaxes);

    // FIXME: Rotate yaw 180 to make frontfacing.
    if (true) {
        quat convfacing;
        vec3 yaxis = {0, 1, 0};
        quat_rotaxis(convfacing, M_PI, yaxis);
        quat_mul(joints_[0].q, joints_[0].q, convfacing);
    }

    // Allocate space for animated global joints.
    joints = new MD5Format::joint[model->numJoints];
    memcpy(joints, joints_, sizeof (MD5Format::joint) * model->numJoints);

    loopi(MAX_JOINTPOINTS) {
        jointpoints[i] = MD5Format::findJoint(model, getJointname(i).c_str());
        //if (jointpoints[i] < 0) jointpoints[i] = MD5Format::findJoint(model, getJointname2(i).c_str());
    }

    // Generate base vertices and normals for 3d-tex-coords:

    if (baseverts.empty()) {
        MD5Format::mesh* msh = MD5Format::getFirstMesh(model);
        MD5Format::joint* staticjoints = MD5Format::getJoints(model);

        loopi(model->numMeshes) {
            float* cur_baseverts = new float[msh->numverts * 3];
            float* cur_basenorms = new float[msh->numverts * 3];
            MD5Format::animatedMeshVertices(msh, staticjoints, cur_baseverts, cur_basenorms);
            baseverts[i] = cur_baseverts;
            basenorms[i] = cur_basenorms;
            // Recalculate normals, invert and set as normal-weights.
            {
                MD5Format::tri* tris = MD5Format::getTris(msh);
                // TODO: Normals need to be averaged/smoothed.
                for (int j = 0; j < msh->numtris; j++) {
                    float* a = &cur_baseverts[3 * tris[j].a];
                    float* b = &cur_baseverts[3 * tris[j].b];
                    float* c = &cur_baseverts[3 * tris[j].c];
                    float ab[3];
                    float bc[3];
                    float n[3];
                    vector_sub(ab, b, a);
                    vector_sub(bc, c, b);
                    vector_cross(n, bc, ab);
                    vector_norm(n, n);
                    vector_cpy(&cur_basenorms[3 * tris[j].a], n);
                    vector_cpy(&cur_basenorms[3 * tris[j].b], n);
                    vector_cpy(&cur_basenorms[3 * tris[j].c], n);
                }
                MD5Format::unanimatedMeshNormals(msh, staticjoints, cur_basenorms);
            }
            msh = MD5Format::getNextMesh(msh);
        }
    }
}


void rRigged::animate(float spf) {
    if (!active) return;
    if (model == NULL) return;
    if (grounded > 0.15f) {
        poseRunning(spf);
    } else {
        poseJumping(spf);
    }
    transformJoints();
}


void rRigged::transform() {
    if (!active) return;
    if (model == NULL) return;
    //transformJoints();
    //transformMounts();
}


void rRigged::drawSolid() {
    if (!active) return;
    if (model == NULL) return;
    GL::glPushMatrix();
    {
        GL::glTranslatef(pos0[0], pos0[1], pos0[2]);
        GLS::glRotateq(ori0);
        //cPrimitives::glAxis(3.0f);
        drawMeshes();
    }
    GL::glPopMatrix();
}


void rRigged::drawEffect() {
    if (!active) return;
    if (model == NULL) return;
    if (DRAWJOINTS) {
        GL::glPushMatrix();
        {
            GL::glTranslatef(pos0[0], pos0[1], pos0[2]);
            GLS::glRotateq(ori0);
            drawBones();
        }
        GL::glPopMatrix();
    }
    // Draw jumpjet exaust if jet is somewhat on.
    if (jetting > 0.3f) {
        GL::glPushAttrib(GL_ALL_ATTRIB_BITS);
        {
            GLS::glUseProgram_fgaddcolor();

            //std::map<int, int>& jointpoints = rigged->jointpoints;
            int jet[5];
            jet[0] = jointpoints[JET0];
            jet[1] = jointpoints[JET1];
            jet[2] = jointpoints[JET2];
            jet[3] = jointpoints[JET3];
            jet[4] = jointpoints[JET4];

            loopi(5) {
                if (jet[i] >= 0) {
                    float* v = joints[jet[i]].v;
                    GL::glPushMatrix();
                    {
                        float f = jetting * 0.5f;

                        GL::glTranslatef(pos0[0], pos0[1], pos0[2]);
                        GLS::glRotateq(ori0);
                        GL::glTranslatef(v[0], v[1], v[2]);

                        float n[16];
                        GLS::glGetTransposeInverseRotationMatrix(n);
                        GL::glMultMatrixf(n);

                        GL::glColor4f(1, 1, 0.3, 0.6);
                        Primitive::glDisk(7, f + 0.0003 * (rand() % 100));
                        GL::glColor4f(1, 0.5, 0.3, 0.7);
                        Primitive::glDisk(7, f * 0.6 + 0.001 * (rand() % 100));
                        GL::glColor4f(1, 1, 1, 0.8);
                        Primitive::glDisk(7, f * 0.3 + 0.001 * (rand() % 100));
                    }
                    GL::glPopMatrix();
                } // if
            } // loopi
        }
        GL::glPopAttrib();
    }
}


unsigned int rRigged::loadMaterial() {
    static bool fail = false;
    static GL::GLenum prog = 0;

    if (prog == 0 && !fail) {
        char* vtx = Filesystem::loadTextFile("/base/material/base.vert");
        if (vtx) logger->debug() << "--- Vertex-Program Begin ---\n" << vtx << "\n--- Vertex-Program End ---\n";
        char* fgm = Filesystem::loadTextFile("/base/material/base3d.frag");
        if (fgm) logger->debug() << "--- Fragment-Program Begin ---\n" << fgm << "\n--- Fragment-Program End ---\n";
        fail = (vtx == NULL || fgm == NULL) || (vtx[0] == 0 && fgm[0] == 0);
        if (!fail) {
            std::stringstream str;
            prog = GLS::glCompileProgram(vtx, fgm, str);
            logger->error() << str.str() << "\n";
        }
        delete[] vtx;
        delete[] fgm;
    }

    if (fail) return 0;
    return prog;
}


void rRigged::initMaterials() {
    if (materials.empty()) {
        if (1) {

            logger->info() << "Generating Camoflage..." << "\n";

            const int SIZE = 1 << (7 + MECHDETAIL);
            unsigned char* texels = new unsigned char[SIZE * SIZE * SIZE * 3];

            enum {
                WOOD,
                RUSTY,
                URBAN,
                DESERT,
                SNOW,
                CAMO,
                GLASS,
                RUBBER,
                STEEL,
                WARN,
                MAX_TEX
            };

            const char* names[] = {
                "wood",
                "rusty",
                "urban",
                "desert",
                "snow",
                "camo",
                "glass",
                "rubber",
                "steel",
                "warn"
            };

            for (int l = 0; l < MAX_TEX; l++) {
                long t = 0;

                loopijk(SIZE, SIZE, SIZE) {
                    float color[16];
                    const float f = 0.25f * 0.25f * 64.0f / SIZE;
                    float x = f*i, y = f*j, z = f*k;
                    switch (l) {
                        case WOOD: Solid::camo_wood(x, y, z, color); 
                            break;
                        case RUSTY: Solid::camo_rust(x, y, z, color);
                            break;
                        case URBAN: Solid::camo_urban(x, y, z, color);
                            break;
                        case DESERT: Solid::camo_desert(x, y, z, color);
                            break;
                        case SNOW: Solid::camo_snow(x, y, z, color);
                            break;
                        case CAMO: Solid::camo_desert(x, y, z, color); // camo
                            break;
                        case GLASS: Solid::metal_damast(x, y, z, color); // glass
                            break;
                        case RUBBER: //Solid::stone_lava(x, y, z, color); // rubber
                            color[0] = color[1] = color[2] = 0.2f;
                            color[3] = 1.0f;
                            break;
                        case STEEL: Solid::metal_sheets(x, y, z, color); // steel
                            break;
                        case WARN: Solid::pattern_warning(x, y, z, color); // warn
                            break;
                        default:
                            Solid::camo_rust(x, y, z, color);
                    }
                    texels[t++] = 255.0f * color[0];
                    texels[t++] = 255.0f * color[1];
                    texels[t++] = 255.0f * color[2];
                }
                unsigned int texname = GLS::glBindTexture3D(0, true, true, true, true, true, SIZE, SIZE, SIZE, texels);
                materials[std::string(names[l])] = texname;
            }
            delete[] texels;
        }
    }
}


void rRigged::drawBones() {
    GL::glPushAttrib(GL_ALL_ATTRIB_BITS);
    {
        GLS::glUseProgram_fgplaincolor();

        loopi(model->numJoints) {
            GL::glPushMatrix();
            {
                GL::glTranslatef(joints[i].v[0], joints[i].v[1], joints[i].v[2]);
                GLS::glRotateq(joints[i].q);
                Primitive::glAxis(0.7);
            }
            GL::glPopMatrix();
            GL::glColor3f(0.5, 0.5, 0.5);
            GL::glBegin(GL_LINES);
            {
                int j = joints[i].parent;
                if (j >= 0) {
                    GL::glVertex3fv(joints[i].v);
                    GL::glVertex3fv(joints[j].v);
                }
            }
            GL::glEnd();
        }
    }
    GL::glPopAttrib();
}


void rRigged::drawMeshes() {
    GL::glPushAttrib(GL_ALL_ATTRIB_BITS);
    {
        GLS::glUseProgram_fglittexture3d();
        GL::glUseProgramObjectARB(loadMaterial());
        GL::glColor4f(1, 1, 1, 1);
        GL::glFrontFace(GL_CW);

        // Get first mesh of model to iterate through.
        MD5Format::mesh* msh = MD5Format::getFirstMesh(model);

        // Bounding box min and max corner.
        // Bounding box dimensions are model orientation dependent...
        /*
        const float e = 100000.0f;
        vec3 mins = { +e, +e, +e };
        vec3 maxs = { -e, -e, -e };
         */
        // Reset cylindrical dimensions.
        radius = 0;
        height = 0;

        /*
        std::map<char,float> colors;
        colors['r'] = 0.10;
        colors['c'] = 0.50;
        colors['s'] = 0.80;
        colors['g'] = 1.00;
        */

        loopi(model->numMeshes) {
            //logger->trace() << "Shader:" << msh->shader << "\n";
            //float co = colors[msh->shader[0]];
            //GL::glColor3f(co,co,co);
            
            std::string shader = std::string(msh->shader);
            if (shader.compare("camo") == 0) {
                switch(basetexture3d) {
                    case 0: shader = "wood"; break;
                    case 1: shader = "rusty"; break;
                    case 2: shader = "urban"; break;
                    case 3: shader = "snow"; break;
                    default: break;
                }
            }
            GL::glBindTexture(GL_TEXTURE_3D, materials[shader]);

            //logger->trace() << curr->numverts << " " << curr->numtris << " " << curr->numweights << "\n";
            float* vtx = new float[msh->numverts * 3];
            float* nrm = new float[msh->numverts * 3];
            MD5Format::animatedMeshVertices(msh, joints, vtx, nrm);
            MD5Format::tri* tris = MD5Format::getTris(msh);
            //MD5Format::vert* verts = MD5Format::getVerts(msh);
            // For 3d texturing.
            float* vox = baseverts[i];
            //
            GL::glBegin(GL_TRIANGLES);
            const float s = 0.25f;
            for (int j = 0; j < msh->numtris; j++) {
                int k = tris[j].a;
                //glTexCoord2fv(verts[k].tmap);
                //GL::glTexCoord3fv(&vox[3 * k]);
                GL::glNormal3fv(&nrm[3 * k]);
                GL::glTexCoord3f(vox[3 * k + 0] * s, vox[3 * k + 1] * s, vox[3 * k + 2] * s);
                GL::glVertex3fv(&vtx[3 * k]);
                k = tris[j].b;
                //glTexCoord2fv(verts[k].tmap);
                //GL::glTexCoord3fv(&vox[3 * k]);
                GL::glNormal3fv(&nrm[3 * k]);
                GL::glTexCoord3f(vox[3 * k + 0] * s, vox[3 * k + 1] * s, vox[3 * k + 2] * s);
                GL::glVertex3fv(&vtx[3 * k]);
                k = tris[j].c;
                //glTexCoord2fv(verts[k].tmap);
                //GL::glTexCoord3fv(&vox[3 * k]);
                GL::glNormal3fv(&nrm[3 * k]);
                GL::glTexCoord3f(vox[3 * k + 0] * s, vox[3 * k + 1] * s, vox[3 * k + 2] * s);
                GL::glVertex3fv(&vtx[3 * k]);
            }
            GL::glEnd();
            // Calculate boundaries of current pose.
            {
                int nvtx = msh->numverts;

                loopj(nvtx) {
                    float* v = &vtx[3 * j];
                    /*
                    loopk(3) {
                        mins[k] = (v[k] < mins[k]) ? v[k] : mins[k];
                        maxs[k] = (v[k] > maxs[k]) ? v[k] : maxs[k];
                    }
                     */
                    float r = v[0] * v[0] + v[2] * v[2];
                    radius = (r > radius) ? r : radius;
                    height = v[1] > height ? v[1] : height;
                }
            }
            //
            delete[] vtx;
            delete[] nrm;
            // Process next mesh of model in next iteration.
            msh = MD5Format::getNextMesh(msh);
        }
        radius = sqrt(radius);
        //logger->trace() << " Model-Dimensions: r = " << radius << " h = " << height << "\n";
        //logger->trace() << " Model-Dimensions: (" << mins[0] << " " << mins[1] << " " << mins[2] << ") (" << maxs[0] << " " << maxs[1] << " " << maxs[2] << ")\n";
        GL::glUseProgramObjectARB(0);
    }
    GL::glPopAttrib();
}


void rRigged::poseJumping(float spf) {
    // Hanging Legs
    float one = PI_OVER_180;
    const float a = -30.0f * one;
    const float b = -60.0f * one;
    const float s = +30.0f * one;
    const float f = b / a;

    if (rotators[LEFTLEG][0] > -a) rotators[LEFTLEG][0] -= s * spf;
    if (rotators[LEFTLEG][0] < -a + one) rotators[LEFTLEG][0] += s * spf;

    if (rotators[LEFTCALF][0] > +b) rotators[LEFTCALF][0] -= f * s * spf;
    if (rotators[LEFTCALF][0] < +b + one) rotators[LEFTCALF][0] += f * s * spf;

    if (rotators[RIGHTLEG][0] > -a) rotators[RIGHTLEG][0] -= s * spf;
    if (rotators[RIGHTLEG][0] < -a + one) rotators[RIGHTLEG][0] += s * spf;

    if (rotators[RIGHTCALF][0] > +b) rotators[RIGHTCALF][0] -= f * s * spf;
    if (rotators[RIGHTCALF][0] < +b + one) rotators[RIGHTCALF][0] += f * s * spf;
}


void rRigged::poseRunning(float spf) {
    // Animate legs according to real forward velocity.
    vec3 v;
    if (vel != NULL && ori0 != NULL) {
        quat ori_inv;
        quat_cpy(ori_inv, ori0);
        quat_conj(ori_inv);
        quat_apply(v, ori_inv, vel);
    } else if (vel != NULL) {
        vector_cpy(v, vel);
    } else {
        vector_zero(v);
    }

    float fwdvel = copysign(sqrtf(v[0] * v[0] + v[2] * v[2]), v[2]) * -0.16f;
    if (fwdvel > 2.8) fwdvel = 2.8;
    if (fwdvel < -1.0) fwdvel = -1.0;

    // This is full of hand crafted magic numbers and magic code don't ask...
    const float e = 0.017453;
    float o = e * -60;
    seconds += e * spf * 180.0f * (1.0f + 0.2 * fabs(fwdvel));
    float t = 1.4f * seconds;

    float l1 = 25 * +sin(t) * fwdvel;
    float l2 = 15 * (+sin(copysign(t, fwdvel) + o) + 1) * fabs(fwdvel);
    float l3 = 10 * +cos(t) * fwdvel;

    float r1 = 20 * -sin(t) * fwdvel;
    float r2 = 20 * (-sin(copysign(t, fwdvel) + o) + 1) * fabs(fwdvel);
    float r3 = 15 * -cos(t) * fwdvel;

    bool bird = false;
    if (model != NULL) {
        if (MD5Format::findJoint(model, "BIRD") >= 0) {
            bird = true;
        }
    }
    float bf = bird ? -1.0f : +1.0f;

    float l1scale = 0.65;
    if (bf * l1 > 0) l1 *= l1scale;
    if (bf * r1 > 0) r1 *= l1scale;

    float l2scale = 0.45;
    if (bf * l2 < 0) l2 *= l2scale;
    if (bf * r2 < 0) r2 *= l2scale;

    float l3scale = 0.25;
    if (bf * l3 > 0) l3 *= l3scale;
    if (bf * r3 > 0) r3 *= l3scale;

    float s = PI_OVER_180;
    rotators[LEFTLEG][0] = -l1 * s;
    rotators[LEFTCALF][0] = -l2 * s;
    rotators[LEFTFOOT][0] = -l3 * s;
    rotators[RIGHTLEG][0] = -r1 * s;
    rotators[RIGHTCALF][0] = -r2 * s;
    rotators[RIGHTFOOT][0] = -r3 * s;
}


void rRigged::transformJoints() {
    MD5Format::joint* manipulators = new MD5Format::joint[model->numJoints];
    assert(manipulators != NULL);

    loopi(model->numJoints) {
        quat_set(manipulators[i].q, 0, 0, 0, -1);
        vector_set(manipulators[i].v, 0, 0, 0);
    }

    { // set joint manipulators
        float xaxis[] = {-1, 0, 0};
        float yaxis[] = {0, -1, 0};
        //float zaxis[] = {0, 0, -1};

        int yaw_idx = jointpoints[YAW];
        int pitch_idx = jointpoints[PITCH];
        if (yaw_idx < 0) yaw_idx = pitch_idx;

        if (0) {
            quat qy;
            quat_rotaxis(qy, +1 * M_PI, yaxis);
            quat_cpy(manipulators[0].q, qy);
        }

        if (yaw_idx == pitch_idx && pitch_idx >= 0) {
            quat qy;
            quat_rotaxis(qy, rotators[YAW][1]*rotatorsFactors[YAW][1], yaxis);
            quat qx;
            quat_rotaxis(qx, rotators[PITCH][0]*rotatorsFactors[PITCH][0], xaxis);
            quat q;
            quat_mul(q, qy, qx);
            quat_cpy(manipulators[pitch_idx].q, q);
        } else {
            if (yaw_idx >= 0) {
                quat qy;
                quat_rotaxis(qy, rotators[YAW][1]*rotatorsFactors[YAW][1], yaxis);
                quat_cpy(manipulators[yaw_idx].q, qy);
            }
            if (pitch_idx >= 0) {
                quat qx;
                quat_rotaxis(qx, rotators[PITCH][0]*rotatorsFactors[PITCH][0], xaxis);
                quat_cpy(manipulators[pitch_idx].q, qx);
            }
        }

        if (true) {
            quat qx;
            quat_rotaxis(qx, rotators[HEADPITCH][0]*rotatorsFactors[HEADPITCH][0], xaxis);
            int jidx = jointpoints[HEADPITCH];
            if (jidx >= 0) quat_cpy(manipulators[jidx].q, qx);
        }

        bool left = true;
        if (left) {
            quat qx;
            quat_rotaxis(qx, rotators[LEFTLEG][0], xaxis);
            int jidx = jointpoints[LEFTLEG];
            if (jidx >= 0) quat_cpy(manipulators[jidx].q, qx);
        }
        if (left) {
            quat qx;
            quat_rotaxis(qx, rotators[LEFTCALF][0], xaxis);
            int jidx = jointpoints[LEFTCALF];
            if (jidx >= 0) quat_cpy(manipulators[jidx].q, qx);
        }
        if (left) {
            quat qx;
            quat_rotaxis(qx, rotators[LEFTFOOT][0], xaxis);
            int jidx = jointpoints[LEFTFOOT];
            if (jidx >= 0) quat_cpy(manipulators[jidx].q, qx);
        }
        bool right = true;
        if (right) {
            quat qx;
            quat_rotaxis(qx, rotators[RIGHTLEG][0], xaxis);
            int jidx = jointpoints[RIGHTLEG];
            if (jidx >= 0) quat_cpy(manipulators[jidx].q, qx);
        }
        if (right) {
            quat qx;
            quat_rotaxis(qx, rotators[RIGHTCALF][0], xaxis);
            int jidx = jointpoints[RIGHTCALF];
            if (jidx >= 0) quat_cpy(manipulators[jidx].q, qx);
        }
        if (right) {
            quat qx;
            quat_rotaxis(qx, rotators[RIGHTFOOT][0], xaxis);
            int jidx = jointpoints[RIGHTFOOT];
            if (jidx >= 0) quat_cpy(manipulators[jidx].q, qx);
        }
    } // set joint manipulators

    // Transform local skeleton using manipulators to global skeleton.
    MD5Format::joint* joints_orig = MD5Format::getJoints(model);
    MD5Format::toGlobalJoints(model->numJoints, joints_orig, joints, manipulators);
    delete[] manipulators;
}

