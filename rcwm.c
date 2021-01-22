/* rc's window manager */

#include <stdio.h>
#include <X11/xlib.h>
#include <signal.h>

//variables
static display *disp;
static int screen;
static Window root;
static int sw, sh, numlock = 0;

//functions
void grabkeys(Window root){
	unsigned int i, j, modifiers[] = {0, LockMask, numlock, numlock|LockMask};
    XModifierKeymap *modmap = XGetModifierMapping(disp);
    KeyCode code;

    for (i = 0; i < 8; i++)
        for (int k = 0; k < modmap->max_keypermod; k++)
            if (modmap->modifiermap[i * modmap->max_keypermod + k]
                == XKeysymToKeycode(d, 0xff7f))
                numlock = (1 << i);

    XUngrabKey(disp, AnyKey, AnyModifier, root);

    for (i = 0; i < sizeof(keys)/sizeof(*keys); i++)
        if ((code = XKeysymToKeycode(disp, keys[i].keysym)))
            for (j = 0; j < sizeof(modifiers)/sizeof(*modifiers); j++)
                XGrabKey(disp, code, keys[i].mod | modifiers[j], root,
                        True, GrabModeAsync, GrabModeAsync);

    for (i = 1; i < 4; i += 2)
        for (j = 0; j < sizeof(modifiers)/sizeof(*modifiers); j++)
            XGrabButton(d, i, MOD | modifiers[j], root, True,
                ButtonPressMask|ButtonReleaseMask|PointerMotionMask,
                GrabModeAsync, GrabModeAsync, 0, 0);

    XFreeModifiermap(modmap);
}

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
	
	//grabbing input
	XSelectInput(disp, root, SubstructureRedirectMask);
	grabkeys(root);
	
	//event loop
	while (1 && !XNextEvent(d, &ev)) // 1 && will forever be here.
		if (events[ev.type]) events[ev.type](&ev);
	
	//close the display
	XCloseDisplay(disp);
	
}

