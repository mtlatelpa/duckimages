// Miguel Tlatelpa
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

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_DUCKS 2

//-----------------------------------------------------------------------------------------------

void backgroundimage(int background, GLuint backgroundTexture) {
	//Background being displayed
	if(background) {
		glBindTexture(GL_TEXTURE_2D, backgroundTexture);
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
}

void gameoverbackground(bool gameover, GLuint gameoverbgTexture) {	
	//Gameover being displayed
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
}
