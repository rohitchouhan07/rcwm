/* rc's window manager */

#include <stdio.h>
#include <X11/xlib.h>
#include <signal.h>


// Taken from DWM. Many thanks. https://git.suckless.org/dwm
#define mod_clean(mask) (mask & ~(numlock|LockMask) & \
        (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
        
//variables
static display *disp;
static int screen;
static Window root;
static int sw, sh, numlock = 0;

//events array
static void (*events[LASTEvent])(XEvent *e) = {
    [ConfigureRequest] = configure_request,
    [KeyPress]         = key_press,
    [MapRequest]       = map_request,
    [MappingNotify]    = mapping_notify,
    [DestroyNotify]    = notify_destroy,
    [EnterNotify]      = notify_enter,
    [MotionNotify]     = notify_motion
};

//functions

//keypress event handler
void key_press(XEvent *e) {
    KeySym keysym = XkbKeycodeToKeysym(d, e->xkey.keycode, 0, 0);

    for (unsigned int i=0; i < sizeof(keys)/sizeof(*keys); ++i)
        if (keys[i].keysym == keysym &&
            mod_clean(keys[i].mod) == mod_clean(e->xkey.state))
            keys[i].function(keys[i].arg);
}

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

