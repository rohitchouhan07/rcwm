/* rc's window manager */

#include <stdio.h>
#include <X11/xlib.h>
#include <signal.h>

//variables
static display *disp;
static int screen;
static Window root;
static int sw,sh;

int main(void){
	
	//try to open the display
	if(!disp = XOpenDisplay(0)){
		fprintf(stdout, "cannot open display!!\n");
		exit(1);
	}
	
	//avoid zombie processes
	signal(SIGCHLD, SIG_IGN);
	
	//getting screen and root window
	screen = DefaultScreen(disp);
	root = RootWindow(disp, screen);
	sh = XDisplayHeight(disp, screen);
	sw = XDisplayWidth(disp, screen);
	XSelectInput(disp, root, SubstructureRedirectMask);

	
}

