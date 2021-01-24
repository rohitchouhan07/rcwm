/* rc's window manager */

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

// Taken from DWM. Many thanks. https://git.suckless.org/dwm
#define mod_clean(mask) (mask & ~(numlock|LockMask) & \
        (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
        
#define TABLENGTH(X)    (sizeof(X)/sizeof(*X))
        
typedef struct desktop{
	client *head;
	client *current;
}desktop;

typedef struct client{
	client *next;
	client *prev;
	
	Window win;
}
//variables
static display *disp;
static int screen;
static Window root;
static int sw, sh, numlock = 0;
static int master_size;
static desktop desktops[10];
static int current_desktop = 1;
static client *head;
static client *current;

//events array
static void (*events[LASTEvent])(XEvent *e) = {
    [ConfigureRequest] = configure_request,
    [KeyPress]         = key_press,
    [MapRequest]       = map_request,
    [DestroyNotify]    = notify_destroy,
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

void configure_request(XEvent *e) {
    XConfigureRequestEvent *ev = &e->xconfigurerequest;

    XConfigureWindow(d, ev->window, ev->value_mask, &(XWindowChanges) {
        .x          = ev->x,
        .y          = ev->y,
        .width      = ev->width,
        .height     = ev->height,
        .sibling    = ev->above,
        .stack_mode = ev->detail
    });
}

void spawn(const Arg arg) {
    if (fork()) return;
    if (disp) close(ConnectionNumber(disp));

    setsid();
    execvp((char*)arg.com[0], (char**)arg.com);
}

void maprequest(XEvent *e) {
    XMapRequestEvent *ev = &e->xmaprequest;

    // For fullscreen mplayer (and maybe some other program)
    client *c;
    for(c=head;c;c=c->next)
        if(ev->window == c->win) {
            XMapWindow(dis,ev->window);
            return;
        }

    add_window(ev->window);
    XMapWindow(dis,ev->window);
    tile();
    update_current();
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
	
	master_size = sw*0.6;
	
	//grabbing input
	XSelectInput(disp, root, SubstructureRedirectMask);
	grabkeys(root);
	
	 // List of client
    head = NULL;
    current = NULL;

    // Set up all desktop
    int i;
    for(i=0;i<TABLENGTH(desktops);++i) {
        desktops[i].head = head;
        desktops[i].current = current;
    }
    
	//event loop
	while (1 && !XNextEvent(d, &ev)) // 1 && will forever be here.
		if (events[ev.type]) events[ev.type](&ev);
	
	//close the display
	XCloseDisplay(disp);
	
}

