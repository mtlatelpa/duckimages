// Jason Thai
// Gerardo Peregrina
// Duck Hunt

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "ppm.h"
#include <stdio.h>
#include <unistd.h> //for sleep function

//800, 600
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_DUCKS 2

#define rnd() (float)rand() / (float)RAND_MAX

extern "C"
{
#include "fonts.h"
}


#define USE_SOUND
#ifdef USE_SOUND
#include <FMOD/fmod.h>
#include <FMOD/wincompat.h>
#include "fmod.h"
#endif //USE_SOUND

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

struct timespec timeStart, timeCurrent;
struct timespec timePause;
double movementCountdown = 0.0;
double timeSpan = 0.0;
const double oobillion = 1.0 / 1e9;
const double movementRate = 1.0 / 60.0;
double timeDiff(struct timespec *start, struct timespec *end)
{
	return (double)(end->tv_sec - start->tv_sec) + (double)(end->tv_nsec - start->tv_nsec) * oobillion;
}
void timeCopy(struct timespec *dest, struct timespec *source)
{
	memcpy(dest, source, sizeof(struct timespec));
}


//Structures
struct Vec {
	float x, y, z;
};

struct Shape {
	float width, height;
	float radius;
	Vec center;
};

//duck sprite
typedef double Arr[3];
struct Sprite {
	Arr pos;
	Arr vel;
};
//First Duck Sprite
Sprite duck_sprite;
Ppmimage *duckImage=NULL;
GLuint duckTexture;
GLuint duckSil;
//Second Duck Sprite
Sprite duck_sprite2;
Ppmimage *duckImage2=NULL;
GLuint duckTexture2;
GLuint duckSil2;
int show_duck = 0;
int silhouette = 1;
//Bullet Sprite
Sprite bullet_sprite;
Ppmimage *bulletImage=NULL;
GLuint bulletTexture;
//White duck Sprite
Sprite duckscore_sprite;
Ppmimage *duckscoreImage=NULL;
GLuint duckscoreTexture;
//Red duck sprite
Sprite duckscore_sprite2;
Ppmimage *duckscoreImage2=NULL;
GLuint duckscoreTexture2;

struct Duck
{
	Shape s;
	Vec velocity;
	struct timespec time;
	struct Duck *prev;
	struct Duck *next;
	//bool shot;
	Duck()
	{
		prev = NULL;
		next = NULL;
	}
};
struct deadDuck
{
    Shape s;
    Vec velocity;
    struct timespec time;
    struct deadDuck *prev;
    struct deadDuck *next;
    bool points;
    deadDuck()
    {
    prev = NULL;
    next = NULL;
    }
};
struct freeDuck
{
    Shape s;
    Vec velocity;
    //struct timespec time;
    struct freeDuck *prev;
    struct freeDuck *next;
    freeDuck()
    {
    prev = NULL;
    next = NULL;
    }
};


////////////////////////////////////////////////
struct Dog
{
    Shape s;
    Vec velocity;
    struct timespec time;
    struct Dog *prev;
    struct Dog *next;
    Dog()
    {
        prev = NULL;
        next = NULL;
    }
};
struct happyDog
{
    Shape s;
    Vec velocity;
    struct timespec time;
    struct happyDog *prev;
    struct happyDog *next;
    happyDog()
    {
        prev = NULL;
        next = NULL;
    }
};
struct laughingDog
{
    Shape s;
    Vec velocity;
    struct timespec time;
    struct laughingDog *prev;
    struct laughingDog *next;
    laughingDog()
    {
        prev = NULL;
        next = NULL;
    }
};
//////////////////////////////////////////////////

struct Game {
    int bullets, n, rounds, score, duckCount, duckShot, onScreen, duckCaptured;  // <-- duckCaptured
    Duck *duck;
    deadDuck *deadD;
    freeDuck *freeD;
    Dog *dog;    //////////////////////
    happyDog *hdog;  ////////////////////
    laughingDog *ldog;   //////////////////
    float floor;
    struct timespec duckTimer, dogTimer; // <-- dogTimer
    Shape box[10];
    bool oneDuck, twoDuck, animateDog, dogGone, afterDog, waitForDog; // <-- animateDog,dogGone,afterDog,waitForDog
    bool menutest;
	~Game()
    {
        delete duck;
        delete deadD;
        delete freeD;
        delete dog;  ///////////////
        delete hdog;  ///////////////
        delete ldog;  /////////////////
    }
    Game()
    {
        duck = NULL;
        deadD = NULL;
        freeD = NULL;
        dog = NULL;  //////////////////
        hdog = NULL;  /////////////////
        ldog = NULL;  //////////////////
        bullets = 0;
        n = 0;
        floor = WINDOW_HEIGHT / 5.0;
        rounds = 1;
        score = 0;
        duckCount = 0;
        duckShot = 0;
        onScreen = 0;
        duckCaptured = 0;  ///////////////////
        oneDuck = false;
        twoDuck = false;
        animateDog = false;   //////////////////////
        dogGone = true;  /////////////////////
        afterDog = false;  ////////////////////////
        waitForDog = true;  ///////////////////////
		
		menutest = true;

        //bullet
        box[0].width = 45;
        box[0].height = 35;
        box[0].center.x = (WINDOW_WIDTH / 10) - (WINDOW_WIDTH / 20);
        box[0].center.y = WINDOW_HEIGHT - (WINDOW_HEIGHT - floor) - (floor / 1.1);
        box[0].center.z = 0;

        //count
        box[1].width = 100;
        box[1].height = 35;
        box[1].center.x = WINDOW_WIDTH / 4;
        box[1].center.y = WINDOW_HEIGHT - (WINDOW_HEIGHT - floor) - (floor / 1.1);
        box[1].center.z = 0;

        //score
        box[2].width = 45;
        box[2].height = 35;
        box[2].center.x = (WINDOW_WIDTH / 2) + (WINDOW_WIDTH / 4);
        box[2].center.y = WINDOW_HEIGHT - (WINDOW_HEIGHT - floor) - (floor / 1.1);
        box[2].center.z = 0;

        //round
        box[3].width = 45;
        box[3].height = 35;
        box[3].center.x = (WINDOW_WIDTH / 10) - (WINDOW_WIDTH / 70);
        box[3].center.y = WINDOW_HEIGHT - (WINDOW_HEIGHT - floor) - (floor / 1.5);
        box[3].center.z = 0;

        //score on shot
        box[4].width = 45;
        box[4].height = 35;
        box[4].center.x = 0;
        box[4].center.y = 0;
        box[4].center.z = 0;
		
        box[5].width = 100;
        box[5].height = 80;
        box[5].center.x = 110;
        box[5].center.y = 500;
        box[5].center.z = 0;

		box[6].width = 100;
        box[6].height = 80;
        box[6].center.x = 400;
        box[6].center.y = 500;
        box[6].center.z = 0;

		box[7].width = 100;
        box[7].height = 80;
        box[7].center.x = 690;
        box[7].center.y = 500;
        box[7].center.z = 0;



    }
};


//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);
void makeDuck(Game *game);
//Added from Jason's File
void makeDeadDuck(Game *game);
void makeFreeDuck(Game *game);
void makeDog(Game *game);   //////////////////////////////
void makeHappyDog(Game *game);  //////////////////////////////
void makeLaughingDog(Game *game);  ///////////////////////////////
void deleteDuck(Game *game, Duck *duck);
void deleteDeadDuck(Game *game, deadDuck *deadD);
void deleteFreeDuck(Game *game, freeDuck *freeD);
void deleteDog(Game *game, Dog *dog);
void deleteHappyDog(Game *game, happyDog *dog);  ////////////////////////////
void deleteLaughingDog(Game *game, laughingDog *dog);  ///////////////////////////

void check_resize(XEvent *e);
void init_sounds(void);

Ppmimage *backgroundImage = NULL;
GLuint backgroundTexture;
int background = 1;
//Transparent background
Ppmimage *backgroundTransImage = NULL;
Ppmimage *gameoverbgImage = NULL;
GLuint backgroundTransTexture;
GLuint gameoverbgTexture;
int trees = 1;
bool gameover = false;

int main(void)
{
	int done=0;
	srand(time(NULL));
	initXWindows();
	init_opengl();
	init_sounds();

	clock_gettime(CLOCK_REALTIME, &timePause);
	clock_gettime(CLOCK_REALTIME, &timeStart);
	//declare game object
	Game game;

	//start animation
	while(!done) {
		while(XPending(dpy)) {
			XEvent e;
			XNextEvent(dpy, &e);
			check_resize(&e);
			check_mouse(&e, &game);
			done = check_keys(&e, &game);
		}
		clock_gettime(CLOCK_REALTIME, &timeCurrent);
		timeSpan = timeDiff(&timeStart, &timeCurrent);
		timeCopy(&timeStart, &timeCurrent);
		movementCountdown += timeSpan;
		while(movementCountdown >= movementRate)
		{
			movement(&game);
			movementCountdown -= movementRate;
		}
		render(&game);
		glXSwapBuffers(dpy, win);
	}
	cleanupXWindows();
	cleanup_fonts();
	#ifdef USE_SOUND
	fmod_cleanup();
	#endif //USE_SOUND
	return 0;
}

void set_title(void)
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "Duck Hunt");
}

void cleanupXWindows(void) {
	//do not change
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

void initXWindows(void) {
	//do not change
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;

	XSetWindowAttributes swa;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		std::cout << "\n\tcannot connect to X server\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if(vi == NULL) {
		std::cout << "\n\tno appropriate visual found\n" << std::endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	//XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
		ButtonPress | ButtonReleaseMask |
		PointerMotionMask |
		StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
			InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void reshape_window(int width, int height)
{
	glViewport(0, 0, (GLint)width, (GLint)height);
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
	set_title();
}

unsigned char *buildAlphaData(Ppmimage *img) {
	// add 4th component to RGB stream...
	int a,b,c;
	unsigned char *newdata, *ptr;
	unsigned char *data = (unsigned char *)img->data;
	//newdata = (unsigned char *)malloc(img->width * img->height * 4);
	newdata = new unsigned char[img->width * img->height * 4];
	ptr = newdata;
	for (int i=0; i<img->width * img->height * 3; i+=3) {
		a = *(data+0);
		b = *(data+1);
		c = *(data+2);
		*(ptr+0) = a;
		*(ptr+1) = b;
		*(ptr+2) = c;
		*(ptr+3) = (a|b|c);
		ptr += 4;
		data += 3;
	}
	return newdata;
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);

	//added for background
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);
	glDisable(GL_CULL_FACE);
	//clear the screen
	glClearColor(1.0, 1.0, 1.0, 1.0);
	backgroundImage = ppm6GetImage("./images/background.ppm");
	backgroundTransImage = ppm6GetImage("./images/backgroundTrans.ppm");
	gameoverbgImage = ppm6GetImage("./images/gameoverbg.ppm");
	


	//-------------------------------------------------------------------
	//bullet
	glGenTextures(1, &bulletTexture);
	bulletImage = ppm6GetImage("./images/bullet.ppm");
	int w3 = bulletImage->width;
	int h3 = bulletImage->height;
	glBindTexture(GL_TEXTURE_2D, bulletTexture);
	//
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, w3, h3, 0, GL_RGB, GL_UNSIGNED_BYTE, bulletImage->data);
	//-------------------------------------------------------------------
	
	//-------------------------------------------------------------------
	//White duck score sprite	
	glGenTextures(1, &duckscoreTexture);
	duckscoreImage = ppm6GetImage("./images/duck_score_1.ppm");
	int w4 = duckscoreImage->width;
	int h4 = duckscoreImage->height;
	glBindTexture(GL_TEXTURE_2D, duckscoreTexture);
	//
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, w4, h4, 0, GL_RGB, GL_UNSIGNED_BYTE, duckscoreImage->data);
	//-------------------------------------------------------------------
	
	//-------------------------------------------------------------------
	//Red duck score sprite	
	glGenTextures(1, &duckscoreTexture2);
	duckscoreImage2 = ppm6GetImage("./images/duck_score_2.ppm");
	int w5 = duckscoreImage2->width;
	int h5 = duckscoreImage2->height;
	glBindTexture(GL_TEXTURE_2D, duckscoreTexture2);
	//
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, w5, h5, 0, GL_RGB, GL_UNSIGNED_BYTE, duckscoreImage2->data);
	//-------------------------------------------------------------------
	
	//-------------------------------------------------------------------
	//duck sprite
	glGenTextures(1, &duckTexture);
	glGenTextures(1, &duckSil);
	duckImage = ppm6GetImage("./images/duck.ppm");
	int w = duckImage->width;
	int h = duckImage->height;
	glBindTexture(GL_TEXTURE_2D, duckTexture);
	//
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, duckImage->data);
	//-------------------------------------------------------------------
	
	//-------------------------------------------------------------------
	//duck sprite 2
	glGenTextures(1, &duckTexture2);
	glGenTextures(1, &duckSil2);
	duckImage2 = ppm6GetImage("./images/duck2.ppm");
	int w2 = duckImage2->width;
	int h2 = duckImage2->height;
	glBindTexture(GL_TEXTURE_2D, duckTexture2);
	//
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, w2, h2, 0, GL_RGB, GL_UNSIGNED_BYTE, duckImage2->data);
	//-------------------------------------------------------------------

	//-------------------------------------------------------------------
	//duck silhouette 
	glBindTexture(GL_TEXTURE_2D, duckSil);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	////must build a new set of data...
	unsigned char *silhouetteData = buildAlphaData(duckImage);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
	GL_RGBA, GL_UNSIGNED_BYTE, silhouetteData);
	delete [] silhouetteData;
	//-------------------------------------------------------------------
	
	//-------------------------------------------------------------------
	//duck silhouette 2 
	glBindTexture(GL_TEXTURE_2D, duckSil2);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	////must build a new set of data...
	unsigned char *silhouetteData2 = buildAlphaData(duckImage2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w2, h2, 0,
	GL_RGBA, GL_UNSIGNED_BYTE, silhouetteData2);
	delete [] silhouetteData2;
	//-------------------------------------------------------------------

	//-------------------------------------------------------------------
	//background textures
	//create opengl texture elements
	glGenTextures(1, &backgroundTexture);
	glBindTexture(GL_TEXTURE_2D, backgroundTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, backgroundImage->width, backgroundImage->height, 0, GL_RGB, GL_UNSIGNED_BYTE, backgroundImage->data); 
	//-------------------------------------------------------------------

	//-------------------------------------------------------------------
//No genTextures for trans image?
	//forest transparent part
	glBindTexture(GL_TEXTURE_2D, backgroundTransTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    //must build a new set of data...
    int w6 = backgroundTransImage->width;
    int h6 = backgroundTransImage->height;
    unsigned char *ftData = buildAlphaData(backgroundTransImage);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w6, h6, 0, GL_RGBA, GL_UNSIGNED_BYTE, ftData);
    delete [] ftData;
	//-------------------------------------------------------------------
    
	//-------------------------------------------------------------------
    //gameover
	glGenTextures(1, &gameoverbgTexture);
    glBindTexture(GL_TEXTURE_2D, gameoverbgTexture);
    //
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, gameoverbgImage->width, gameoverbgImage->height, 0, GL_RGB, GL_UNSIGNED_BYTE, gameoverbgImage->data);
    //-------------------------------------------------------------------

	glEnable(GL_TEXTURE_2D);
	initialize_fonts();
}

void makeDuck(Game *game, float x, float y)
{
    if(game->n >= MAX_DUCKS)
        return;
    struct timespec dt;
    clock_gettime(CLOCK_REALTIME, &dt);
    timeCopy(&game->duckTimer, &dt);
    int directionNum = rand() % 101;
    Duck *d;
    try
    {
        d = new Duck;
    }
    catch(std::bad_alloc)
    {
        return;
    }
    timeCopy(&d->time, &dt);
    d->s.center.x = x;
    d->s.center.y = y;
    d->s.center.z = 0.0;
    if(directionNum >= 50)
        d->velocity.x = 4.0 * (game->rounds * .5);
    else
        d->velocity.x = -4.0 * (game->rounds * .5);
    d->velocity.y = 4.0 * (game->rounds * .5);
    d->velocity.z = 0.0;
    d->s.width = 50.0;
    d->s.height = 50.0;
    d->next = game->duck;
    if(game->duck != NULL)
    {
        game->duck->prev = d;
    }
    game->duck = d;
    game->n++;
}


//----------------------------------------------------------------------
void makeDeadDuck(Game *game, Duck *duck)
{
    if(game->onScreen >= MAX_DUCKS)
        return;
    struct timespec dt;
    clock_gettime(CLOCK_REALTIME, &dt);
    timeCopy(&game->duckTimer, &dt);
    deadDuck *dd;
    try
    {
        dd = new deadDuck;
    }
    catch(std::bad_alloc)
    {
        return;
    }
    timeCopy(&dd->time, &dt);
    dd->s.center.x = duck->s.center.x;
    dd->s.center.y = duck->s.center.y;
    dd->s.center.z = 0.0;
    dd->velocity.x = 0.0;
    //dd->velocity.y = -3.5;
    dd->velocity.z = 0.0;
    dd->s.width = 50.0;
    dd->s.height = 50.0;
    dd->next = game->deadD;
    if(game->deadD != NULL)
    {
        game->deadD->prev = dd;
    }
    game->deadD = dd;
    game->onScreen++;
}


void makeFreeDuck(Game *game, Duck *duck)
{
    if(game->onScreen >= MAX_DUCKS)
        return;
    //struct timespec dt;
    //clock_gettime(CLOCK_REALTIME, &dt);
    //timeCopy(&game->duckTimer, &dt);
    freeDuck *fd;
    try
    {
        fd = new freeDuck;
    }
    catch(std::bad_alloc)
    {
        return;
    }
    //timeCopy(&dd->time, &dt);
    fd->s.center.x = duck->s.center.x;
    fd->s.center.y = duck->s.center.y;
    fd->s.center.z = 0.0;
    fd->velocity.x = duck->velocity.x;
    fd->velocity.y = duck->velocity.y;
    fd->velocity.z = 0.0;
    fd->s.width = 50.0;
    fd->s.height = 50.0;
    fd->next = game->freeD;
    if(game->freeD != NULL)
    {
        game->freeD->prev = fd;
    }
    game->freeD = fd;
    game->onScreen++;
}

void makeDog(Game *game, float x, float y)
{
    //if(!game->dogGone)
    //  return;
    struct timespec dt;
    clock_gettime(CLOCK_REALTIME, &dt);
    timeCopy(&game->dogTimer, &dt);
    Dog *doge;
    try
    {
        doge = new Dog;
    }
    catch(std::bad_alloc)
    {
        return;
    }
    timeCopy(&doge->time, &dt);
    doge->s.center.x = x;
    doge->s.center.y = y;
    doge->s.center.z = 0.0;
    doge->velocity.x = 1.0;
    doge->velocity.y = 0.0;
    doge->velocity.z = 0.0;
    doge->s.width = 50.0;
    doge->s.height = 50.0;
    doge->next = game->dog;
    if(game->dog != NULL)
    {
        game->dog->prev = doge;
    }
    game->dog = doge;
    game->animateDog = true;
    //game->dogGone = false;
}

void makeHappyDog(Game *game, float x, float y)
{
    //if(!game->afterDog)
    //  return;
    struct timespec dt;
    clock_gettime(CLOCK_REALTIME, &dt);
    timeCopy(&game->dogTimer, &dt);
    happyDog *hdoge;
    try
    {
        hdoge = new happyDog;
    }
    catch(std::bad_alloc)
    {
        return;
    }
    timeCopy(&hdoge->time, &dt);
    hdoge->s.center.x = x;
    hdoge->s.center.y = y;
    hdoge->s.center.z = 0.0;
    hdoge->velocity.x = 0.0;
    hdoge->velocity.y = 2.0;
    hdoge->velocity.z = 0.0;
    hdoge->s.width = 50.0;
    hdoge->s.height = 50.0;
    hdoge->next = game->hdog;
    if(game->hdog != NULL)
    {
        game->hdog->prev = hdoge;
    }
    game->hdog = hdoge;
    //game->animateDog = true;
    game->afterDog = true;
    game->waitForDog = true;
}

void makeLaughingDog(Game *game, float x, float y)
{
    //if(!game->afterDog)
    //  return;
    struct timespec dt;
    clock_gettime(CLOCK_REALTIME, &dt);
    timeCopy(&game->dogTimer, &dt);
    laughingDog *ldoge;
    try
    {
        ldoge = new laughingDog;
    }
    catch(std::bad_alloc)
    {
        return;
    }
    timeCopy(&ldoge->time, &dt);
    ldoge->s.center.x = x;
    ldoge->s.center.y = y;
    ldoge->s.center.z = 0.0;
    ldoge->velocity.x = 0.0;
    ldoge->velocity.y = 2.0;
    ldoge->velocity.z = 0.0;
    ldoge->s.width = 50.0;
    ldoge->s.height = 50.0;
    ldoge->next = game->ldog;
    if(game->ldog != NULL)
    {
        game->ldog->prev = ldoge;
    }
    game->ldog = ldoge;
    //game->animateDog = true;
    game->afterDog = true;
    game->waitForDog = true;
}
//----------------------------------------------------------------------

void check_resize(XEvent *e)
{
	if(e->type != ConfigureNotify)
		return;
	XConfigureEvent xce = e->xconfigure;
	if(xce.width != WINDOW_WIDTH || xce.height != WINDOW_HEIGHT)
	{
		reshape_window(xce.width, xce.height);
	}
}


//----------------------------------------------------------------------
void check_mouse(XEvent *e, Game *game)
{
    int y = WINDOW_HEIGHT - e->xbutton.y;

    Duck *d = game->duck;
    Duck *saved = new Duck;
    struct timespec dt;
    clock_gettime(CLOCK_REALTIME, &dt);
    double ts;

    if (e->type == ButtonRelease) {
        return;
    }
    if (e->type == ButtonPress) {
        if (e->xbutton.button==1) {
                #ifdef USE_SOUND
                fmod_playsound(0);
            //Left button was pressed
            while(d)
            {
                if(e->xbutton.x >= d->s.center.x - d->s.width &&
                        e->xbutton.x <= d->s.center.x + d->s.width &&
                        y <= d->s.center.y + d->s.height &&
                        y >= d->s.center.y - d->s.height)
                {
                fmod_playsound(1);
                    makeDeadDuck(game, d);
                    ts = timeDiff(&d->time, &dt);
                    if(ts < 1.5)
                    {
                        game->deadD->points = true;  /////////////////////////////////
                        game->score += 200;
                    }
                    else
                    {
                        game->deadD->points = false;  ///////////////////////////////////
                        game->score += 100;
                    }
                    saved = d->next;
                    deleteDuck(game, d);
                    d = saved;
                    game->bullets--;
                    game->duckShot++;
                    game->duckCaptured++;
                    if(game->bullets < 1)
                    {
                        if(game->n == 1)
                        {
                            makeFreeDuck(game, d);
                            saved = d->next;
                            deleteDuck(game, d);
                            d = saved;
                        }
                        return;
                    }
                    return;
                }
		 if(game->n == 2)
                {
                    d = d->next;
                    if(e->xbutton.x >= d->s.center.x - d->s.width &&
                            e->xbutton.x <= d->s.center.x + d->s.width &&
                            y <= d->s.center.y + d->s.height &&
                            y >= d->s.center.y - d->s.height)
                    {
                        makeDeadDuck(game, d);
                        ts = timeDiff(&d->time, &dt);
                        if(ts < 1.5)
                        {
                            game->deadD->points = true;  /////////////////////////////////////
                            game->score += 200;
                        }
                        else
                        {
                            game->deadD->points = false;  /////////////////////////////////////
                            game->score += 100;
                        }
                        saved = d->prev;
                        deleteDuck(game, d);
                        d = saved;
                        game->bullets--;
                        game->duckShot++;
                        game->duckCaptured++;
                        if(game->bullets < 1)
                        {
                            if(game->n == 1)
                            {
                                makeFreeDuck(game, d);
                                saved = d->next;
                                deleteDuck(game, d);
                                d = saved;
                            }
                            return;
                        }
                        return;
                    }
                }
		 if(game->bullets <= 1)
                {
                    if(game->n == 2)
                    {
                        d = d->prev;
                    }
                    makeFreeDuck(game, d);
                    saved = d->next;
                    deleteDuck(game, d);
                    d = saved;
                    game->bullets--;
                    if(game->n == 1)
                    {
                        makeFreeDuck(game, d);
                        saved = d->next;
                        deleteDuck(game, d);
                        d = saved;
                    }
                    return;
                }
                game->bullets--;
                d = d->next;
            }
        }
        #endif
    }
    if (e->xbutton.button==3) {
        //Right button was pressed
        return;
    }
}


int check_keys(XEvent *e, Game *game)
{
    Duck *d = game->duck;
    deadDuck *dd = game->deadD;
    freeDuck *fd = game->freeD;
    Dog *doge = game->dog;
    happyDog *hdoge = game->hdog;
    laughingDog *ldoge = game->ldog;

    //Was there input from the keyboard?
    if (e->type == KeyPress) {
        int key = XLookupKeysym(&e->xkey, 0);
        if (key == XK_Escape) {
            return 1;
        }
        //You may check other keys here.
        if(key == XK_1)
        {
            game->menutest = false;
			while(d)
            {
                deleteDuck(game, d);
                d = d->next;
            }
            while(dd)
            {
                deleteDeadDuck(game, dd);
                dd = dd->next;
            }
            while(fd)
            {
                deleteFreeDuck(game, fd);
                fd = fd->next;
            }
            while(doge)
            {
                deleteDog(game, doge);
                doge = doge->next;
            }
            while(hdoge)
            {
                deleteHappyDog(game, hdoge);
                hdoge = hdoge->next;
            }
            while(ldoge)
            {
                deleteLaughingDog(game, ldoge);
                ldoge = ldoge->next;
            }
            game->n = 0;
            game->onScreen = 0;
            game->rounds = 1;
            game->duckCount = 0;
            game->duckShot = 0;
            game->bullets = 3;
            game->score = 0;
            game->duckCaptured = 0;
            game->oneDuck = true;
            game->twoDuck = false;
            game->animateDog = true;
            game->dogGone = false;
            game->afterDog = true;
            game->waitForDog = false;
        }
        if(key == XK_2)
        {
            game->menutest = false;
            while(d)
            {
                deleteDuck(game, d);
                d = d->next;
            }
            while(dd)
            {
                deleteDeadDuck(game, dd);
                dd = dd->next;
            }
            while(fd)
            {
                deleteFreeDuck(game, fd);
                fd = fd->next;
            }
            while(doge)
            {
                deleteDog(game, doge);
                doge = doge->next;
            }
            while(hdoge)
            {
                deleteHappyDog(game, hdoge);
                hdoge = hdoge->next;
            }
            while(ldoge)
            {
                deleteLaughingDog(game, ldoge);
                ldoge = ldoge->next;
            }
            game->n = 0;
            game->onScreen = 0;
            game->rounds = 1;
            game->duckCount = 0;
            game->duckShot = 0;
            game->bullets = 3;
            game->score = 0;
            game->duckCaptured = 0;
            game->oneDuck = false;
            game->twoDuck = true;
            game->animateDog = true;
            game->dogGone = false;
            game->afterDog = true;
            game->waitForDog = false;
        }
    }
    return 0;
}
//------------------------------------------------------------------


void movement(Game *game)
{
    Duck *d = game->duck;
    deadDuck *dd = game->deadD;
    freeDuck *fd = game->freeD;
    Dog *doge = game->dog;
    happyDog *hdoge = game->hdog;
    laughingDog *ldoge = game->ldog;
    struct timespec dt;
    clock_gettime(CLOCK_REALTIME, &dt);
    int randDirectionNumX, randDirectionNumY;

    if (game->n < 0)
        return;

    while(d)
    {
        double ts = timeDiff(&d->time, &dt);
        randDirectionNumX = rand() % 101;
        randDirectionNumY = rand() % 101;
        if(ts > 5.0)
        {
            makeFreeDuck(game, d);
            Duck *saved = d->next;
            deleteDuck(game, d);
            d = saved;
            continue;
        }
        if(1.0 < ts && ts < 1.01)
        {
            if(randDirectionNumX >= 50)
                d->velocity.x *= -1;
            if(randDirectionNumY >= 50)
                d->velocity.y *= -1;
        }
        if(2.0 < ts && ts < 2.01)
        {
            if(randDirectionNumX >= 50)
                d->velocity.x *= -1;
            if(randDirectionNumY >= 50)
                d->velocity.y *= -1;
        }
        if(3.0 < ts && ts < 3.01)
        {
            if(randDirectionNumX >= 50)
                d->velocity.x *= -1;
            if(randDirectionNumY >= 50)
                d->velocity.y *= -1;
        }
        if(4.0 < ts && ts < 4.01)
        {
            if(randDirectionNumX >= 50)
                d->velocity.x *= -1;
            if(randDirectionNumY >= 50)
                d->velocity.y *= -1;
        }

        if(d->s.center.x - d->s.width <= 0.0)
        {
            d->s.center.x = d->s.width + 0.1;
            d->velocity.x *= -1.0;
        }
        if(d->s.center.x + d->s.width >= WINDOW_WIDTH)
        {
            d->s.center.x = WINDOW_WIDTH - d->s.width - 0.1;
            d->velocity.x *= -1.0;
        }
        if(d->s.center.y - d->s.height <= game->floor)
        {
            d->s.center.y = game->floor + d->s.height + 0.1;
            d->velocity.y *= -1.0;
        }
        if(d->s.center.y + d->s.height >= WINDOW_HEIGHT)
        {
            d->s.center.y = WINDOW_HEIGHT - d->s.height - 0.1;
            d->velocity.y *= -1.0;
        }

        d->s.center.x += d->velocity.x;
        d->s.center.y += d->velocity.y;

        d = d->next;
    }
	while(dd)
    {
        double ts = timeDiff(&dd->time, &dt);
        float velocity = -4.0;
        if(ts < 0.3)
            dd->velocity.y = 0.0;
        if(ts > 0.3)
            dd->velocity.y = velocity;
        if(dd->s.center.y - dd->s.height <= game->floor)
        {
            game->waitForDog = true;
            deleteDeadDuck(game, dd);
            if(game->n == 0)
            {
                game->afterDog = true;
                return;
            }
        }
        dd->s.center.y += dd->velocity.y;
        dd = dd->next;
    }

    while(fd)
    {
        if(fd->s.center.x + fd->s.width <= 0.0)
        {
            game->waitForDog = true;
            deleteFreeDuck(game, fd);
            if(game->n == 0)
            {
                game->afterDog = true;
                return;
            }
        }
        if(fd->s.center.x - fd->s.width >= WINDOW_WIDTH)
        {
            game->waitForDog = true;
            deleteFreeDuck(game, fd);
            if(game->n == 0)
            {
                game->afterDog = true;
                return;
            }
        }
        if(fd->velocity.y < 0.0)
            fd->velocity.y *= -1.0;
        if(fd->s.center.y - fd->s.height >= WINDOW_HEIGHT)
        {
            game->waitForDog = true;
            deleteFreeDuck(game, fd);
            if(game->n == 0)
            {
                game->afterDog = true;
                return;
            }
        }
        fd->s.center.x += fd->velocity.x;
        fd->s.center.y += fd->velocity.y;

        fd = fd->next;
    }
     while(doge)
    {
        double ts = timeDiff(&doge->time, &dt);
        if(2.0 < ts && ts < 2.1)
            doge->velocity.x = 0.0;
        if(3.0 < ts && ts < 3.1)
            doge->velocity.x = 1.0;
        if(5.0 < ts && ts < 5.1)
            doge->velocity.x = 0.0;
        if(6.0 < ts && ts < 6.1)
        {
            doge->velocity.x = 1.5;
            doge->velocity.y = 2.0;
        }
        if(6.5 < ts && ts < 6.6)
        {
            doge->velocity.x = 0.5;
            doge->velocity.y = -2.0;
        }
        if(doge->s.center.y - doge->s.height <= game->floor)
        {
            deleteDog(game, doge);
            //game->dogGone = true;
            return;
        }
        doge->s.center.x += doge->velocity.x;
        doge->s.center.y += doge->velocity.y;

        doge = doge->next;
    }

    while(hdoge)
    {
        double ts = timeDiff(&hdoge->time, &dt);
        if(0.5 < ts && ts < 0.6)
            hdoge->velocity.y = 0.0;
        if(1.5 < ts && ts < 1.6)
            hdoge->velocity.y = -2.0;
        if(hdoge->s.center.y - hdoge->s.height <= game->floor)
        {
            deleteHappyDog(game, hdoge);
            //game->afterDog = false;
            return;
        }
        hdoge->s.center.y += hdoge->velocity.y;

        hdoge = hdoge->next;
    }

    while(ldoge)
    {
        double ts = timeDiff(&ldoge->time, &dt);
        if(0.5 < ts && ts < 0.6)
            ldoge->velocity.y = 0.0;
        if(1.5 < ts && ts < 1.6)
            ldoge->velocity.y = -2.0;
        if(ldoge->s.center.y - ldoge->s.height <= game->floor)
        {
            deleteLaughingDog(game, ldoge);
            //game->afterDog = false;
            return;
        }
        ldoge->s.center.y += ldoge->velocity.y;

        ldoge = ldoge->next;
    }
}

//=====================================================================================
//The Display of the game 
void render(Game *game)
{
	float w, h, x, y;
	Duck *d = game->duck;
	deadDuck *dd = game->deadD;
	freeDuck *fd = game->freeD;
    Dog *doge = game->dog;
    happyDog *hdoge = game->hdog;
    laughingDog *ldoge = game->ldog;
    struct timespec dt;
    clock_gettime(CLOCK_REALTIME, &dt);
	
	glColor3ub(255, 255, 255);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

//Background and Gameover being displayed
	if(background) {
		glBindTexture(GL_TEXTURE_2D, backgroundTexture);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 0);
		glTexCoord2f(0.0f, 0.0f); glVertex2i(0, WINDOW_HEIGHT);
		glTexCoord2f(1.0f, 0.0f); glVertex2i(WINDOW_WIDTH, WINDOW_HEIGHT);
		glTexCoord2f(1.0f, 1.0f); glVertex2i(WINDOW_WIDTH, 0);
		glEnd();
	}
	if(gameover) {
        glBindTexture(GL_TEXTURE_2D, gameoverbgTexture);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 0);
        glTexCoord2f(0.0f, 0.0f); glVertex2i(0, WINDOW_HEIGHT);
        glTexCoord2f(1.0f, 0.0f); glVertex2i(WINDOW_WIDTH, WINDOW_HEIGHT);
        glTexCoord2f(1.0f, 1.0f); glVertex2i(WINDOW_WIDTH, 0);
        glEnd();
    }
	glDisable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
//----------------------------------------

	//Drawing Shapes
	glColor3ub(255, 255, 255);
	glBegin(GL_LINES);
	glVertex2f(0.0, game->floor);
	glVertex2f(WINDOW_WIDTH, game->floor);
	glEnd();

	Rect r;
	r.bot = WINDOW_HEIGHT - 550;
	r.left = WINDOW_WIDTH - 715;
	r.center = 0;

//-------------------------------------------------------------------
//Drawing Boxes
	Shape *s;

	if (game->menutest == true) {
       const char* text[3] = {"One Duck Hunt", "Two Duct Hunt", "        Exit"}; // the Text need fixing to look better.
		for(int i=5; i<8; i++) {
			glColor3ub(90, 140, 90);
			s = &game->box[i];
			glPushMatrix();
			glTranslatef(s->center.x, s->center.y, s->center.z);
			w = s->width;
			h = s->height;
			r.bot = s->height - 75;
			r.left = s->width - 170;
			glBegin(GL_QUADS);
			glVertex2i(-w,-h);
			glVertex2i(-w, h);
			glVertex2i( w, h);
			glVertex2i( w,-h);
			glEnd();
			if (i == 5)
				ggprint16(&r, 35, 0x00ffffff, text[0]);
			if (i == 6)
				ggprint16(&r, 35, 0x00ffffff, text[1]);
			if (i == 7)
				ggprint16(&r, 35, 0x00ffffff, text[2]);
			r.bot = s->height - 120;
			r.left = s->width - 170;
			if (i == 5)
				ggprint16(&r, 35, 0x00ffffff, "   Key \" 1 \"");
			if (i == 6)
				ggprint16(&r, 35, 0x00ffffff, "   Key \" 2 \"");
			if (i == 7)
				ggprint16(&r, 35, 0x00ffffff, "   Key \" Esc \"");
			glPopMatrix();
		}
	}




	
	//Displaying bullets
	glColor3ub(90, 140, 90);
	s = &game->box[0];
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);
	w = s->width;
	h = s->height;
	int num = 0, dist = 0;
	if (game->bullets == 3) {
		num = 3, dist = 80;
	}
	if (game->bullets == 2) {
		num = 2, dist = 60;
	}
	if (game->bullets == 1) {
		num = 1, dist = 40;
	}
	if (game->bullets == 0) {
		num = 0, dist = 0;
	}
	if (game->bullets != 0) {
		for (int i=0;i<num;i++) {
			bullet_sprite.pos[0] = dist - (i * 20);
			bullet_sprite.pos[1] = 40;
			bullet_sprite.pos[2] = 0;
			float wid = 10.0f;
			glPushMatrix();
			glTranslatef(bullet_sprite.pos[0], bullet_sprite.pos[1], bullet_sprite.pos[2]);
			glBindTexture(GL_TEXTURE_2D, bulletTexture);
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0.0f);
			glColor4ub(255,255,255,255);
			glBegin(GL_QUADS);
			if (bullet_sprite.vel[0] > 0.0) {
				glTexCoord2f(0.0f, 1.0f); glVertex2i(-wid,-wid);
				glTexCoord2f(0.0f, 0.0f); glVertex2i(-wid, wid);
				glTexCoord2f(1.0f, 0.0f); glVertex2i( wid, wid);
				glTexCoord2f(1.0f, 1.0f); glVertex2i( wid,-wid);
			} else {
				glTexCoord2f(1.0f, 1.0f); glVertex2i(-wid,-wid);
				glTexCoord2f(1.0f, 0.0f); glVertex2i(-wid, wid);
				glTexCoord2f(0.0f, 0.0f); glVertex2i( wid, wid);
				glTexCoord2f(0.0f, 1.0f); glVertex2i( wid,-wid);
			}
			glEnd();
			glPopMatrix();
			glDisable(GL_ALPHA_TEST);
		}
	}
	r.bot = s->height;
	r.left = s->width;
	glVertex2i(-w, -h);
	glVertex2i(-w, h);
	glVertex2i(w, h);
	glVertex2i(w, -h);
	glEnd();
	//ggprint16(&r , 16, 0x00ffffff, "%i", game->bullets);
	glPopMatrix();

	//-------------------------------------------------------------------
	//Displaying duck score sprites
	glColor3ub(90, 140, 90);
	s = &game->box[1];
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);
	w = s->width;
	h = s->height;
	if (game->duckShot <= 10) {
		for (int i=0;i<=9;i++) {
			duckscore_sprite.pos[0] = 70 + (i * 25);
			duckscore_sprite.pos[1] = 42;
			duckscore_sprite.pos[2] = 0;
			float wid = 10.0f;
			glPushMatrix();
			glTranslatef(duckscore_sprite.pos[0], duckscore_sprite.pos[1], duckscore_sprite.pos[2]);
			glBindTexture(GL_TEXTURE_2D, duckscoreTexture);
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0.0f);
			glColor4ub(255,255,255,255);
			glBegin(GL_QUADS);
			if (duckscore_sprite.vel[0] > 0.0) {
				glTexCoord2f(0.0f, 1.0f); glVertex2i(-wid,-wid);
				glTexCoord2f(0.0f, 0.0f); glVertex2i(-wid, wid);
				glTexCoord2f(1.0f, 0.0f); glVertex2i( wid, wid);
				glTexCoord2f(1.0f, 1.0f); glVertex2i( wid,-wid);
			} else {
				glTexCoord2f(1.0f, 1.0f); glVertex2i(-wid,-wid);
				glTexCoord2f(1.0f, 0.0f); glVertex2i(-wid, wid);
				glTexCoord2f(0.0f, 0.0f); glVertex2i( wid, wid);
				glTexCoord2f(0.0f, 1.0f); glVertex2i( wid,-wid);
			}
			glEnd();
			glPopMatrix();
			glDisable(GL_ALPHA_TEST);
		}
	}
	int loop = 0;
	if (game->duckShot == 1) loop = 1;
	if (game->duckShot == 2) loop = 2;
	if (game->duckShot == 3) loop = 3;
	if (game->duckShot == 4) loop = 4;
	if (game->duckShot == 5) loop = 5;
	if (game->duckShot == 6) loop = 6;
	if (game->duckShot == 7) loop = 7;
	if (game->duckShot == 8) loop = 8;
	if (game->duckShot == 9) loop = 9;
	if (game->duckShot == 10) loop = 10;
	if (game->duckShot <= 9) {
		for (int i=0;i<loop;i++) {
			duckscore_sprite2.pos[0] = 70 + (i * 25);
			duckscore_sprite2.pos[1] = 42;
			duckscore_sprite2.pos[2] = 0;
			float wid = 10.0f;
			glPushMatrix();
			glTranslatef(duckscore_sprite2.pos[0], duckscore_sprite2.pos[1], duckscore_sprite2.pos[2]);
			glBindTexture(GL_TEXTURE_2D, duckscoreTexture2);
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0.0f);
			glColor4ub(255,255,255,255);
			glBegin(GL_QUADS);
			if (duckscore_sprite2.vel[0] > 0.0) {
				glTexCoord2f(0.0f, 1.0f); glVertex2i(-wid,-wid);
				glTexCoord2f(0.0f, 0.0f); glVertex2i(-wid, wid);
				glTexCoord2f(1.0f, 0.0f); glVertex2i( wid, wid);
				glTexCoord2f(1.0f, 1.0f); glVertex2i( wid,-wid);
			} else {
				glTexCoord2f(1.0f, 1.0f); glVertex2i(-wid,-wid);
				glTexCoord2f(1.0f, 0.0f); glVertex2i(-wid, wid);
				glTexCoord2f(0.0f, 0.0f); glVertex2i( wid, wid);
				glTexCoord2f(0.0f, 1.0f); glVertex2i( wid,-wid);
			}
			glEnd();
			glPopMatrix();
			glDisable(GL_ALPHA_TEST);
		}
	}
	r.bot = s->height;
	r.left = s->width;
	glVertex2i(-w, -h);
	glVertex2i(-w, h);
	glVertex2i(w, h);
	glVertex2i(w, -h);
	glEnd();
	if(!d && game->duckCount >= 10 && game->duckShot < 6)
    {
        ggprint16(&r , 16, 0x00ffffff, "GAME OVER");
    }
    //ggprint16(&r , 16, 0x00ffffff, "%i / 10", game->duckShot);
	glPopMatrix();



	glColor3ub(90, 140, 90);
	s = &game->box[2];
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);
	w = s->width;
	h = s->height;
	r.bot = s->height;
	r.left = s->width;
	glVertex2i(-w, -h);
	glVertex2i(-w, h);
	glVertex2i(w, h);
	glVertex2i(w, -h);
	glEnd();
	//ggprint16(&r , 16, 0x00ffffff, "%i", game->score);
	glPopMatrix();

	glColor3ub(90, 140, 90);
	s = &game->box[3];
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);
	w = s->width;
	h = s->height;
	r.bot = s->height;
	r.left = s->width;
	glVertex2i(-w, -h);
	glVertex2i(-w, h);
	glVertex2i(w, h);
	glVertex2i(w, -h);
	glEnd();
	ggprint16(&r , 16, 0x00ffffff, "%i", game->rounds);
	glPopMatrix();


	if(game->afterDog && game->onScreen == 0 && game->dogGone && game->n == 0 && game->waitForDog)
    {
        if(game->duckCaptured >= 1)
            makeHappyDog(game, WINDOW_WIDTH / 2, game->floor + 51);
        else
            makeLaughingDog(game, WINDOW_WIDTH / 2, game->floor + 51);
        game->afterDog = false;
        game->duckCaptured = 0;
    }

    if(!d && game->duckCount >= 10 && game->duckShot >= 6 && game->dogGone && !game->waitForDog && game->onScreen == 0 && game->n == 0)
    {
        game->rounds++;
        game->duckCount = 0;
        game->duckShot = 0;
        game->animateDog = true;
        game->dogGone = false;
        game->afterDog = true;
        game->waitForDog = false;
    }

    if((game->oneDuck || game->twoDuck) && game->dogGone && !game->waitForDog)
    {
        if(!d && game->duckCount >= 10 && game->duckShot < 6)
        {
            while(d)
            {
                deleteDuck(game, d);
                d = d->next;
            }
            game->oneDuck = false;
            game->twoDuck = false;
            std::cout << "GAME OVER" << std::endl;
        }
        if(!d && game->oneDuck && game->duckCount < 10 && game->onScreen == 0 && game->n == 0)
        {
            game->bullets = 3;
            makeDuck(game, rand() % (WINDOW_WIDTH - 50 - 1) + 50 + 1, game->floor + 50 + 1);
            game->duckCount++;
        }
        if(!d && game->twoDuck && game->duckCount < 9 && game->onScreen == 0 && game->n == 0)
        {
            game->bullets = 3;
            makeDuck(game, rand() % (WINDOW_WIDTH - 50 - 1) + 50 + 1, game->floor + 50 + 1);
            makeDuck(game, rand() % (WINDOW_WIDTH - 50 - 1) + 50 + 1, game->floor + 50 + 1);
            game->duckCount += 2;
        }
    }


//Main Duck rendering
		duck_sprite.pos[0] = s->center.x;
		glColor3ub(255, 255, 255);
		while(d)
		{
			w = d->s.width;
			h = d->s.height;
			x = d->s.center.x;
			y = d->s.center.y;
			glBegin(GL_QUADS);
			glVertex2f(x-w, y+h);
			glVertex2f(x-w, y-h);
			glVertex2f(x+w, y-h);
			glVertex2f(x+w, y+h);
			glEnd();
			//This changes which duck it points to
			
			show_duck= 1;
			float wid = 50.0f;
			duck_sprite.pos[0] = x;
			duck_sprite.pos[1] = y;
			duck_sprite.pos[2] = s->center.z;
			
			d = d->next;
			duck_sprite2.pos[0] = x;
			duck_sprite2.pos[1] = y;
			duck_sprite2.pos[2] = s->center.z;
			

			if(show_duck) {
				glPushMatrix();
				glTranslatef(duck_sprite.pos[0], duck_sprite.pos[1], duck_sprite.pos[2]);
			//remove if statement and make
			//a while loop to take in both duck 
				if (silhouette) 
				{
			//glBind makes the duck texture
					glBindTexture(GL_TEXTURE_2D, duckTexture);
					glEnable(GL_ALPHA_TEST);
					glAlphaFunc(GL_GREATER, 0.0f);
					glColor4ub(255,255,255,255);
				}
				
				glBegin(GL_QUADS);
				if (duck_sprite.vel[0] > 0.0) {
					glTexCoord2f(0.0f, 1.0f); glVertex2i(-wid,-wid);
					glTexCoord2f(0.0f, 0.0f); glVertex2i(-wid, wid);
					glTexCoord2f(1.0f, 0.0f); glVertex2i( wid, wid);
					glTexCoord2f(1.0f, 1.0f); glVertex2i( wid,-wid);
				} else {
					glTexCoord2f(1.0f, 1.0f); glVertex2i(-wid,-wid);
					glTexCoord2f(1.0f, 0.0f); glVertex2i(-wid, wid);
					glTexCoord2f(0.0f, 0.0f); glVertex2i( wid, wid);
					glTexCoord2f(0.0f, 1.0f); glVertex2i( wid,-wid);
				}
				//Duck2
/*				glTranslatef(duck_sprite2.pos[0], duck_sprite2.pos[1], duck_sprite2.pos[2]);
				if (silhouette) 
				{
					glBindTexture(GL_TEXTURE_2D, duckTexture2);
					glEnable(GL_ALPHA_TEST);
					glAlphaFunc(GL_GREATER, 0.0f);
					glColor4ub(255,255,255,255);
				}
				glBegin(GL_QUADS);
				if (duck_sprite2.vel[0] > 0.0) {
					glTexCoord2f(0.0f, 1.0f); glVertex2i(-wid,-wid);
					glTexCoord2f(0.0f, 0.0f); glVertex2i(-wid, wid);
					glTexCoord2f(1.0f, 0.0f); glVertex2i( wid, wid);
					glTexCoord2f(1.0f, 1.0f); glVertex2i( wid,-wid);
				} else {
					glTexCoord2f(1.0f, 1.0f); glVertex2i(-wid,-wid);
					glTexCoord2f(1.0f, 0.0f); glVertex2i(-wid, wid);
					glTexCoord2f(0.0f, 0.0f); glVertex2i( wid, wid);
					glTexCoord2f(0.0f, 1.0f); glVertex2i( wid,-wid);
				}
*/
				glEnd();
				glPopMatrix();
				//Transparent part
				if (trees && silhouette) {
                    glBindTexture(GL_TEXTURE_2D, backgroundTransTexture);
                    glBegin(GL_QUADS);
                    glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 0);
                    glTexCoord2f(0.0f, 0.0f); glVertex2i(0, WINDOW_HEIGHT);
                    glTexCoord2f(1.0f, 0.0f); glVertex2i(WINDOW_WIDTH, WINDOW_HEIGHT);
                    glTexCoord2f(1.0f, 1.0f); glVertex2i(WINDOW_WIDTH, 0);
                    glEnd();
                }
				glDisable(GL_ALPHA_TEST);
			}
		}
		
		//----------------------------------------------------
		glColor3ub(255, 0, 0);
		while(dd)
		{
			double ts = timeDiff(&dd->time, &dt);
			w = dd->s.width;
			h = dd->s.height;
			x = dd->s.center.x;
			y = dd->s.center.y;
			glBegin(GL_QUADS);
			glVertex2f(x-w, y+h);
			glVertex2f(x-w, y-h);
			glVertex2f(x+w, y-h);
			glVertex2f(x+w, y+h);
			glEnd();
			r.bot = y + h;
			r.left = x - (w/2);
			if(ts < 0.3)
			{
				if(dd->points == true)
					ggprint16(&r , 16, 0x00ffffff, "200");
				else
					ggprint16(&r , 16, 0x00ffffff, "100");
			}
			dd = dd->next;
		}

		glColor3ub(0, 0, 255);
		while(fd)
		{
			w = fd->s.width;
			h = fd->s.height;
			x = fd->s.center.x;
			y = fd->s.center.y;
			glBegin(GL_QUADS);
			glVertex2f(x-w, y+h);
			glVertex2f(x-w, y-h);
			glVertex2f(x+w, y-h);
			glVertex2f(x+w, y+h);
			glEnd();
			fd = fd->next;
		}

		if(game->animateDog)
		{
			makeDog(game, 100, game->floor + 51);
			game->animateDog = false;
		}
		glColor3ub(0, 0, 0);
		while(doge)
		{
			w = doge->s.width;
			h = doge->s.height;
			x = doge->s.center.x;
			y = doge->s.center.y;
			glBegin(GL_QUADS);
			glVertex2f(x-w, y+h);
			glVertex2f(x-w, y-h);
			glVertex2f(x+w, y-h);
			glVertex2f(x+w, y+h);
			glEnd();
			doge = doge->next;
		}
		
		glColor3ub(100, 100, 255);
		while(hdoge)
		{
			w = hdoge->s.width;
			h = hdoge->s.height;
			x = hdoge->s.center.x;
			y = hdoge->s.center.y;
			glBegin(GL_QUADS);
			glVertex2f(x-w, y+h);
			glVertex2f(x-w, y-h);
			glVertex2f(x+w, y-h);
			glVertex2f(x+w, y+h);
			glEnd();
			hdoge = hdoge->next;
		}

		glColor3ub(255, 100, 100);
		while(ldoge)
		{
			w = ldoge->s.width;
			h = ldoge->s.height;
			x = ldoge->s.center.x;
			y = ldoge->s.center.y;
			glBegin(GL_QUADS);
			glVertex2f(x-w, y+h);
			glVertex2f(x-w, y-h);
			glVertex2f(x+w, y-h);
			glVertex2f(x+w, y+h);
			glEnd();
			ldoge = ldoge->next;
		}
}




//----------------------------------------------------------------------------------
void deleteDuck(Game *game, Duck *node)
{
    if(node->prev == NULL)
    {
        if(node->next == NULL)
        {
            game->duck = NULL;
        }
        else
        {
            node->next->prev = NULL;
            game->duck = node->next;
        }
    }
    else
    {
        if(node->next == NULL)
        {
            node->prev->next = NULL;
        }
        else
        {
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }
    }
    delete node;
    node = NULL;
    game->n--;
}
void deleteDeadDuck(Game *game, deadDuck *node)
{
    if(node->prev == NULL)
    {
        if(node->next == NULL)
        {
            game->deadD = NULL;
        }
        else
        {
            node->next->prev = NULL;
            game->deadD = node->next;
        }
    }
    else
    {
        if(node->next == NULL)
        {
            node->prev->next = NULL;
        }
        else
        {
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }
    }
    delete node;
    node = NULL;
    game->onScreen--;
}
void deleteFreeDuck(Game *game, freeDuck *node)
{
    if(node->prev == NULL)
    {
        if(node->next == NULL)
        {
            game->freeD = NULL;
        }
        else
        {
            node->next->prev = NULL;
            game->freeD = node->next;
        }
    }
    else
    {
        if(node->next == NULL)
        {
            node->prev->next = NULL;
        }
        else
        {
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }
    }
    delete node;
    node = NULL;
    game->onScreen--;
}


//----------------------------------------------------------------------------------
void deleteDog(Game *game, Dog *node)
{
    if(node->prev == NULL)
    {
        if(node->next == NULL)
        {
            game->dog = NULL;
        }
        else
        {
            node->next->prev = NULL;
            game->dog = node->next;
        }
    }
    else
    {
        if(node->next == NULL)
        {
            node->prev->next = NULL;
        }
        else
        {
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }
    }
    delete node;
    node = NULL;
    game->animateDog = false;
    game->dogGone = true;
}
void deleteHappyDog(Game *game, happyDog *node)
{
    if(node->prev == NULL)
    {
        if(node->next == NULL)
        {
            game->hdog = NULL;
        }
        else
        {
            node->next->prev = NULL;
            game->hdog = node->next;
        }
    }
    else
    {
        if(node->next == NULL)
        {
            node->prev->next = NULL;
        }
        else
        {
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }
    }
    delete node;
    node = NULL;
    game->afterDog = false;
    game->waitForDog = false;
}
void deleteLaughingDog(Game *game, laughingDog *node)
{
    if(node->prev == NULL)
    {
        if(node->next == NULL)
        {
            game->ldog = NULL;
        }
        else
        {
            node->next->prev = NULL;
            game->ldog = node->next;
        }
    }
    else
    {
        if(node->next == NULL)
        {
            node->prev->next = NULL;
        }
        else
        {
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }
    }
    delete node;
    node = NULL;
    game->afterDog = false;
    game->waitForDog = false;
}


//---------------------------------------------------
void init_sounds(void)
{
    #ifdef USE_SOUND
    //FMOD_RESULT result;
    if (fmod_init()) {
        std::cout << "ERROR - fmod_init()\n" << std::endl;
        return;
    }
    if (fmod_createsound((char *)"./sounds/gunshot.wav", 0)) {
        std::cout << "ERROR - fmod_createsound()\n" << std::endl;
        return;
    }
    if (fmod_createsound((char *)"./sounds/drip.mp3", 1)) {
        std::cout << "ERROR - fmod_createsound()\n" << std::endl;
        return;
    }
    fmod_setmode(0,FMOD_LOOP_OFF);
    //fmod_playsound(0);
    //fmod_systemupdate();
    #endif //USE_SOUND
}

