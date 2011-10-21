/**
 * This example is based on ODE's demo_boxstack: Copyright (c) Russell L. Smith
 * Licensed under BSD.
 */

#include <unistd.h>
#include <limits.h>
#include <ode/ode.h>
#include <ode/odemath.h>
#include <drawstuff/drawstuff.h>
#include <fermat/cd.h>
#include <fermat/cd-ode.h>
#include <fermat/timer.h>
#include <fermat/opts.h>
#include <fermat/dbg.h>

#define DRAWSTUFF_TEXTURE_PATH "textures"
#ifdef _MSC_VER
#pragma warning(disable:4244 4305)  // for VC++, no precision loss complaints
#endif


// select correct drawing functions

#ifdef dDOUBLE
#define dsDrawBox dsDrawBoxD
#define dsDrawSphere dsDrawSphereD
#define dsDrawCylinder dsDrawCylinderD
#define dsDrawCapsule dsDrawCapsuleD
#define dsDrawConvex dsDrawConvexD
#endif


// some constants

#define NUM 1000000			// max number of objects
#define DENSITY (5.0)		// density of all objects
#define GPB 3			// maximum number of geometries per body
#define MAX_CONTACTS 8          // maximum number of contact points per body
#define MAX_FEEDBACKNUM 20
#define GRAVITY         REAL(0.5)
#define USE_GEOM_OFFSET 1

static fer_cd_t *cd = NULL;
static int pargc;
static char **pargv;
static fer_cd_params_t params;
static fer_timer_t loop_timer;

static dWorldID world;
static dSpaceID space;
static dJointGroupID contactgroup;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static int no_win = 0;
static int no_sleep = 0;
static int show_contacts = 0;
static int show_obb = 0;
static int grid_level = 1;
static int paus = 0;
static int alpha = 1;
static long num_steps = LONG_MAX;

static void _drawG(fer_cd_geom_t *g, fer_cd_obb_t *obb)
{
    dReal pos[4];
    dReal rot[12];
    const fer_vec3_t *tr;
    const fer_mat3_t *r;
    dVector3 sides;
    fer_list_t *item;
    fer_cd_obb_t *o;
    dReal v[9];
    fer_cd_tri_t *t;
    fer_cd_box_t *box;
    fer_cd_sphere_t *sphere;
    fer_cd_cap_t *cap;
    fer_cd_cyl_t *cyl;

    dsSetColorAlpha (1,1,0,0.7 + alpha * 0.3);
    dsSetTexture (DS_WOOD);

    tr = ferCDGeomTr(cd, g);
    r  = ferCDGeomRot(cd, g);
    pos[0] = ferVec3X(tr);
    pos[1] = ferVec3Y(tr);
    pos[2] = ferVec3Z(tr);
    rot[0] = ferMat3Get(r, 0, 0);
    rot[1] = ferMat3Get(r, 0, 1);
    rot[2] = ferMat3Get(r, 0, 2);
    rot[4] = ferMat3Get(r, 1, 0);
    rot[5] = ferMat3Get(r, 1, 1);
    rot[6] = ferMat3Get(r, 1, 2);
    rot[8] = ferMat3Get(r, 2, 0);
    rot[9] = ferMat3Get(r, 2, 1);
    rot[10] = ferMat3Get(r, 2, 2);

    if (ferListEmpty(&obb->obbs)){
        if (obb->shape->cl->type == FER_CD_SHAPE_TRI
                || obb->shape->cl->type == FER_CD_SHAPE_TRIMESH_TRI){
            t = (fer_cd_tri_t *)obb->shape;
            v[0] = ferVec3X(t->p[0]);
            v[1] = ferVec3Y(t->p[0]);
            v[2] = ferVec3Z(t->p[0]);
            v[3] = ferVec3X(t->p[1]);
            v[4] = ferVec3Y(t->p[1]);
            v[5] = ferVec3Z(t->p[1]);
            v[6] = ferVec3X(t->p[2]);
            v[7] = ferVec3Y(t->p[2]);
            v[8] = ferVec3Z(t->p[2]);

            dsDrawTriangle(pos, rot, v, v + 3, v + 6, 0);

        }else if (obb->shape->cl->type == FER_CD_SHAPE_BOX){
            box = (fer_cd_box_t *)obb->shape;
            sides[0] = 2. * ferVec3X(box->half_extents);
            sides[1] = 2. * ferVec3Y(box->half_extents);
            sides[2] = 2. * ferVec3Z(box->half_extents);
            dsDrawBox(pos, rot, sides);

        }else if (obb->shape->cl->type == FER_CD_SHAPE_SPHERE){
            sphere = (fer_cd_sphere_t *)obb->shape;
            dsDrawSphere(pos, rot, sphere->radius);

        }else if (obb->shape->cl->type == FER_CD_SHAPE_CAP){
            cap = (fer_cd_cap_t *)obb->shape;
            dsDrawCapsule(pos, rot, 2 * cap->half_height, cap->radius);

        }else if (obb->shape->cl->type == FER_CD_SHAPE_CYL){
            cyl = (fer_cd_cyl_t *)obb->shape;
            dsDrawCylinder(pos, rot, 2 * cyl->half_height, cyl->radius);
        }

        if (show_obb){
            dReal x, y, z;
            dReal pos2[4];
            dReal rot2[12];

            sides[0] = 2. * ferVec3X(&obb->half_extents);
            sides[1] = 2. * ferVec3Y(&obb->half_extents);
            sides[2] = 2. * ferVec3Z(&obb->half_extents);
            x = ferVec3X(&obb->center);
            y = ferVec3Y(&obb->center);
            z = ferVec3Z(&obb->center);
            pos2[0] = x * rot[0] + y * rot[1] + z * rot[2] + pos[0];
            pos2[1] = x * rot[4] + y * rot[5] + z * rot[6] + pos[1];
            pos2[2] = x * rot[8] + y * rot[9] + z * rot[10] + pos[2];
            rot2[0] = rot[0] * ferVec3X(&obb->axis[0])
                        + rot[1] * ferVec3Y(&obb->axis[0])
                        + rot[2] * ferVec3Z(&obb->axis[0]);
            rot2[1] = rot[0] * ferVec3X(&obb->axis[1])
                        + rot[1] * ferVec3Y(&obb->axis[1])
                        + rot[2] * ferVec3Z(&obb->axis[1]);
            rot2[2] = rot[0] * ferVec3X(&obb->axis[2])
                        + rot[1] * ferVec3Y(&obb->axis[2])
                        + rot[2] * ferVec3Z(&obb->axis[2]);
            rot2[4] = rot[4] * ferVec3X(&obb->axis[0])
                        + rot[5] * ferVec3Y(&obb->axis[0])
                        + rot[6] * ferVec3Z(&obb->axis[0]);
            rot2[5] = rot[4] * ferVec3X(&obb->axis[1])
                        + rot[5] * ferVec3Y(&obb->axis[1])
                        + rot[6] * ferVec3Z(&obb->axis[1]);
            rot2[6] = rot[4] * ferVec3X(&obb->axis[2])
                        + rot[5] * ferVec3Y(&obb->axis[2])
                        + rot[6] * ferVec3Z(&obb->axis[2]);
            rot2[8] = rot[8] * ferVec3X(&obb->axis[0])
                        + rot[9] * ferVec3Y(&obb->axis[0])
                        + rot[10] * ferVec3Z(&obb->axis[0]);
            rot2[9] = rot[8] * ferVec3X(&obb->axis[1])
                        + rot[9] * ferVec3Y(&obb->axis[1])
                        + rot[10] * ferVec3Z(&obb->axis[1]);
            rot2[10] = rot[8] * ferVec3X(&obb->axis[2])
                        + rot[9] * ferVec3Y(&obb->axis[2])
                        + rot[10] * ferVec3Z(&obb->axis[2]);

            dsSetColorAlpha (1,0,0,0.3);
            dsDrawBox(pos2, rot2, sides);
        }
    }else{
        FER_LIST_FOR_EACH(&obb->obbs, item){
            o = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
            _drawG(g, o);
        }
    }
}

static void drawG(fer_cd_geom_t *g)
{
    fer_list_t *item;
    fer_cd_obb_t *obb;

    FER_LIST_FOR_EACH(&g->obbs, item){
        obb = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
        _drawG(g, obb);
    }
}


static int sepCB(const fer_cd_t *cd,
                 const fer_cd_geom_t *g1, const fer_cd_geom_t *g2,
                 const fer_cd_contacts_t *con,
                 void *data)
{
    size_t i;
    dContact contact;
    dBodyID b1, b2;
   
    b1 = (dBodyID)ferCDGeomData(g1);
    b2 = (dBodyID)ferCDGeomData(g2);

    for (i = 0; i < con->num; i++){
        if (con->depth[i] < FER_ZERO)
            continue;

        contact.geom.pos[0] = ferVec3X(&con->pos[i]);
        contact.geom.pos[1] = ferVec3Y(&con->pos[i]);
        contact.geom.pos[2] = ferVec3Z(&con->pos[i]);
        contact.geom.normal[0] = -ferVec3X(&con->dir[i]);
        contact.geom.normal[1] = -ferVec3Y(&con->dir[i]);
        contact.geom.normal[2] = -ferVec3Z(&con->dir[i]);
        contact.geom.depth = con->depth[i];
        contact.geom.g1 = 0;
        contact.geom.g2 = 0;
        contact.surface.mode = dContactBounce | dContactSoftCFM;
        contact.surface.mu = dInfinity;
        contact.surface.mu2 = 0;
        contact.surface.bounce = 0.1;
        contact.surface.bounce_vel = 0.1;
        contact.surface.soft_cfm = 0.01;

        // global lock in case multi-threaded version is used
        pthread_mutex_lock(&lock);
        dJointID c = dJointCreateContact(world, contactgroup, &contact);
        dJointAttach(c, b1, b2);
        pthread_mutex_unlock(&lock);

        if (!no_win && show_contacts){
            dMatrix3 RI;
            dRSetIdentity (RI);
            const dReal ss[3] = {0.02,0.02,0.02};

            dsDrawBox(contact.geom.pos, RI, ss);
        }
    }

    return 0;
}


static void start(void)
{
    dAllocateODEDataForThread(dAllocateMaskAll);

    //static float xyz[3] = {3.1640f,-0.3079f,1.7600f};
    //static float hpr[3] = {125.5000f,-17.0000f,0.0000f};
    //static float xyz[3] = {1.2192,0.8433,0.4700};
    //static float hpr[3] = {125.5000,-17.0000,0.0000};
    static float xyz[3] = { 5.518125, -3.677142, 2.340000 };
    static float hpr[3] = { 125.500000, -17.000000, 0.000000 };
    dsSetViewpoint (xyz,hpr);
    printf ("To drop another object, press:\n");
    printf ("   b for box.\n");
    printf ("   s for sphere.\n");
    printf ("   c for capsule.\n");
    printf ("   y for cylinder.\n");
    printf ("   m for bunny.\n");
    printf ("To print xyz, hpr viewpoint, press v.\n");
    printf ("To toggle showing the geom OBBs, press o.\n");
    printf ("To toggle showing the contact points, press t.\n");
}

static void command (int cmd);

static void __drawG(fer_cd_t *cd, fer_cd_geom_t *g, void *data)
{
    drawG(g);
}

static void simLoop(int _pause)
{
    static long i = 0;

    if (!no_sleep)
        usleep(10000);

    if (!no_win)
        dsSetColor (0,0,2);

    if (!_pause && !paus){
        // remove all contact joints
        dJointGroupEmpty(contactgroup);

        // determine contact points and handle them in sepCB() callback
        ferTimerStart(&loop_timer);
        ferCDSeparate(cd, sepCB, NULL);
        ferTimerStop(&loop_timer);
        fprintf(stderr, "Step[%06ld]: %lu us [%d geoms]\n",
                i++, ferTimerElapsedInUs(&loop_timer),
                (int)ferCDNumGeoms(cd));

        // perform physical simulation
        dWorldQuickStep (world,0.02);
    }


    if (!no_win){
        dsSetColor (1,1,0);
        dsSetTexture (DS_WOOD);

        ferCDEachGeom(cd, __drawG, NULL);
    }
}

static void command (int cmd)
{
    dBodyID body;
    fer_cd_geom_t *g;
    dReal sides[3];
    dMass m;
    dMatrix3 R;
    size_t i;
    static float xyz[3];
    static float hpr[3];

    if (cmd == 'b' || cmd == 's' || cmd == 'y' || cmd == 'c' || cmd == 'm'){
        body = dBodyCreate(world);
        dBodySetPosition(body, dRandReal()*2-1, dRandReal()*2-1, dRandReal()+2);
        dRFromAxisAndAngle(R, dRandReal()*2.0-1.0,dRandReal()*2.0-1.0,
                dRandReal()*2.0-1.0,dRandReal()*10.0-5.0);
        dBodySetRotation(body, R);

        for (i = 0; i < 3; i++)
            sides[i] = dRandReal() * 0.5 + 0.1;

        g = ferCDGeomNew(cd);

        if (cmd == 'b'){
            dMassSetBox(&m, DENSITY, sides[0], sides[1], sides[2]);
            ferCDGeomAddBox(cd, g, sides[0], sides[1], sides[2]);
        }else if (cmd == 's'){
            dMassSetSphere(&m, DENSITY, sides[0]);
            ferCDGeomAddSphere(cd, g, sides[0]);
        }else if (cmd == 'y'){
            dMassSetCylinder (&m,DENSITY,3,sides[0],sides[1]);
            ferCDGeomAddCyl(cd, g, sides[0], sides[1]);
        }else if (cmd == 'c'){
            sides[0] *= 0.5;
            dMassSetCapsule(&m,DENSITY,3,sides[0],sides[1]);
            ferCDGeomAddCap(cd, g, sides[0], sides[1]);
        }else if (cmd == 'm'){
            dMassSetCylinder(&m,DENSITY,3,sides[0],sides[1]);
            ferCDGeomAddTrisFromRawScale(cd, g, "bugtrap-2.raw", 0.1);
            ferCDGeomBuild(cd, g);
            /*
            ferCDGeomAddTriMesh(cd, g, bunny_coords, bunny_ids, bunny_tri_len);
            */
        }

        dBodySetMass(body, &m);

        // assign body to geom
        ferCDGeomSetBody(cd, g, body);
        // store pointer to body into geom
        ferCDGeomSetData(g, (void *)body);

    }else if (cmd == 'q'){
        exit(-1);

    }else if (cmd == 'o') {
        show_obb ^= 1;
    }else if (cmd == 't') {
        show_contacts ^= 1;
    }else if (cmd == 'v'){
        dsGetViewpoint(xyz, hpr);
        printf("xyz: %f %f %f\nhpr: %f %f %f\n",
               xyz[0], xyz[1], xyz[2],
               hpr[0], hpr[1], hpr[2]);
        fflush(stdout);
    }else if (cmd == 'p'){
        paus ^= 1;
    }else if (cmd == 'a'){
        alpha ^= 1;
    }
}

static void boxGrid(int x, int y)
{
    size_t i;
    dReal sides[3];
    dMatrix3 R;
    dMass m;
    dBodyID body;
    fer_cd_geom_t *g;

    for (i = 0; i < x * y; i++){
        body = dBodyCreate(world);
        sides[0] = 0.1;
        sides[1] = 0.2;
        sides[2] = 0.3;

        dBodySetPosition(body, 0.4 * (i / x),
                                      0.4 * (i % x), 0.5 + grid_level * 0.5);
        dRFromAxisAndAngle(R, dRandReal()*2.0-1.0,dRandReal()*2.0-1.0,
                              dRandReal()*2.0-1.0,dRandReal()*10.0-5.0);
        dBodySetRotation(body,R);
        dMassSetBox(&m,DENSITY,sides[0],sides[1],sides[2]);
        dBodySetMass(body, &m);

        g = ferCDGeomNew(cd);
        ferCDGeomAddBox(cd, g, sides[0], sides[1], sides[2]);

        // assign body to geom
        ferCDGeomSetBody(cd, g, body);
        // store pointer to body into geom
        ferCDGeomSetData(g, (void *)body);
    }
}

static void sphereGrid(int x, int y)
{
    size_t i;
    dMass m;
    dBodyID body;
    fer_cd_geom_t *g;

    for (i = 0; i < x * y; i++){
        body = dBodyCreate(world);

        dBodySetPosition(body, 0.4 * (i / x),
                               0.4 * (i % x), 0.5 + grid_level * 0.5);

        g = ferCDGeomNew(cd);
        ferCDGeomAddSphere(cd, g, 0.1);

        dMassSetSphere(&m,DENSITY, 0.1);
        dBodySetMass(body, &m);

        ferCDGeomSetBody(cd, g, body);
        ferCDGeomSetData(g, (void *)body);
    }
}

static void capGrid(int x, int y)
{
    size_t i;
    dReal sides[3];
    dMatrix3 R;
    dMass m;
    dBodyID body;
    fer_cd_geom_t *g;

    for (i = 0; i < x * y; i++){
        body = dBodyCreate(world);
        sides[0] = 0.1;
        sides[1] = 0.2;
        sides[2] = 0.3;

        dBodySetPosition(body, 0.4 * (i / x),
                               0.4 * (i % x), 0.5 + grid_level * 0.5);
        dRFromAxisAndAngle(R, dRandReal()*2.0-1.0,dRandReal()*2.0-1.0,
                              dRandReal()*2.0-1.0,dRandReal()*10.0-5.0);
        dBodySetRotation(body,R);
        dMassSetCapsule(&m,DENSITY,3, sides[0],sides[1]);
        dBodySetMass(body, &m);

        g = ferCDGeomNew(cd);
        ferCDGeomAddCap(cd, g, sides[0], sides[1]);


        ferCDGeomSetBody(cd, g, body);
        ferCDGeomSetData(g, (void *)body);
    }
}

static void cylGrid(int x, int y)
{
    size_t i;
    dReal sides[3];
    dMatrix3 R;
    dMass m;
    dBodyID body;
    fer_cd_geom_t *g;

    for (i = 0; i < x * y; i++){
        body = dBodyCreate(world);
        sides[0] = 0.1;
        sides[1] = 0.2;
        sides[2] = 0.3;

        dBodySetPosition(body, 0.4 * (i / x),
                               0.4 * (i % x), 0.5 + grid_level * 0.5);
        dRFromAxisAndAngle(R, dRandReal()*2.0-1.0,dRandReal()*2.0-1.0,
                              dRandReal()*2.0-1.0,dRandReal()*10.0-5.0);
        dBodySetRotation(body,R);
        dMassSetCylinder(&m,DENSITY,3, sides[0],sides[1]);
        dBodySetMass(body, &m);

        g = ferCDGeomNew(cd);
        ferCDGeomAddCyl(cd, g, sides[0], sides[1]);
        ferCDGeomContactPersistence(cd, g, 4);


        ferCDGeomSetBody(cd, g, body);
        ferCDGeomSetData(g, (void *)body);
    }
}


static void optGrid(const char *l, char s, const fer_vec2_t *v)
{
    if (!cd)
        cd = ferCDNew(&params);

    if (strcmp(l, "box-grid") == 0){
        boxGrid(ferVec2X(v), ferVec2Y(v));
    }else if (strcmp(l, "sphere-grid") == 0){
        sphereGrid(ferVec2X(v), ferVec2Y(v));
    }else if (strcmp(l, "cap-grid") == 0){
        capGrid(ferVec2X(v), ferVec2Y(v));
    }else if (strcmp(l, "cyl-grid") == 0){
        cylGrid(ferVec2X(v), ferVec2Y(v));
    }

    grid_level++;
}

static void opt(void)
{
    ferCDParamsInit(&params);

    params.use_sap = 1;
    params.sap_hashsize = 1023 * 1023 * 10 + 1;
    params.use_cp = 1;
    params.max_contacts = 50;

    ferOptsAdd("use-gpu", 0, FER_OPTS_NONE, &params.sap_gpu, NULL);
    ferOptsAdd("use-cp", 0, FER_OPTS_NONE, &params.use_cp, NULL);
    ferOptsAdd("threads", 't', FER_OPTS_SIZE_T, &params.num_threads, NULL);
    ferOptsAdd("max-contacts", 0, FER_OPTS_SIZE_T, &params.max_contacts, NULL);
    ferOptsAdd("no-win", 0, FER_OPTS_NONE, &no_win, NULL);
    ferOptsAdd("no-sleep", 0, FER_OPTS_NONE, &no_sleep, NULL);
    ferOptsAdd("box-grid", 0, FER_OPTS_V2, NULL, FER_OPTS_CB(optGrid));
    ferOptsAdd("sphere-grid", 0, FER_OPTS_V2, NULL, FER_OPTS_CB(optGrid));
    ferOptsAdd("cap-grid", 0, FER_OPTS_V2, NULL, FER_OPTS_CB(optGrid));
    ferOptsAdd("cyl-grid", 0, FER_OPTS_V2, NULL, FER_OPTS_CB(optGrid));
    ferOptsAdd("pause", 0, FER_OPTS_NONE, &paus, NULL);
    ferOptsAdd("num-steps", 0, FER_OPTS_LONG, &num_steps, NULL);

    if (ferOpts(&pargc, pargv) != 0)
        exit(-1);
}

int main (int argc, char **argv)
{
    long i;
    fer_cd_geom_t *plane;

    // setup pointers to drawstuff callback functions
    dsFunctions fn;
    fn.version = DS_VERSION;
    fn.start = &start;
    fn.step = &simLoop;
    fn.command = &command;
    fn.stop = 0;
    fn.path_to_textures = DRAWSTUFF_TEXTURE_PATH;

    // create world
    dInitODE2(0);
    world = dWorldCreate();
    space = dHashSpaceCreate(0);
    //space = dSimpleSpaceCreate(0);
    contactgroup = dJointGroupCreate(0);
    dWorldSetGravity(world, 0, 0, -GRAVITY);
    dWorldSetCFM(world, 1e-5);
    dWorldSetAutoDisableFlag(world, 1);
    dWorldSetAutoDisableAverageSamplesCount(world, 10);

    dWorldSetLinearDamping(world, 0.00001);
    dWorldSetAngularDamping(world, 0.005);
    dWorldSetMaxAngularSpeed(world, 200);

    dWorldSetContactMaxCorrectingVel(world, 0.1);
    dWorldSetContactSurfaceLayer(world, 0.001);

    pargc = argc;
    pargv = argv;
    opt();

    // init collide library
    if (!cd)
        cd = ferCDNew(&params);


    plane = ferCDGeomNew(cd);
    ferCDGeomAddPlane(cd, plane);


    // run simulation
    if (no_win){
        for (i = 0; i < num_steps; i++){
            simLoop(0);
        }
    }else{
        dsSimulationLoop(argc, argv, 800, 600, &fn);
    }

    dJointGroupDestroy(contactgroup);
    dSpaceDestroy(space);
    dWorldDestroy(world);
    dCloseODE();

    ferCDDel(cd);

    ferOptsClear();

    return 0;
}