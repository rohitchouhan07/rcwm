/* rc's window manager */

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

#include "rcwm.h"

// Taken from DWM. Many thanks. https://git.suckless.org/dwm
#define mod_clean(mask) (mask & ~(numlock|LockMask) & \
        (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
        
#define TABLENGTH(X)    (sizeof(X)/sizeof(*X))

//structs
typedef union {
    const char** com;
    const int i;
} Arg;

struct key {
    unsigned int mod;
    KeySym keysym;
    void (*function)(const Arg arg);
    const Arg arg;
};

typedef struct client{
	client *next;
	client *prev;
	
	Window win;
}
       
typedef struct desktop{
	client *tail;
	client *current;
}desktop;


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

//add window
void add_window(Window w) {
    client *c,*t;

    if(!(c = (client *)calloc(1,sizeof(client)))){
        fprintf(stdout, "rcwm: cannot open display!!\n");
		exit(1);
	}

    if(head == NULL) {
        c->next = NULL;
        c->prev = NULL;
        c->win = w;
        head = c;
    }
    else {
        for(t=head;t->next;t=t->next);

        c->next = NULL;
        c->prev = t;
        c->win = w;

        t->next = c;
    }

    current = c;
}

void change_desktop(const Arg arg) {
    client *c;

    if(arg.i == current_desktop)
        return;

    // Unmap all window
    if(head != NULL)
        for(c=head;c;c=c->next)
            XUnmapWindow(disp,c->win);

    // Save current "properties"
    save_desktop(current_desktop);

    // Take "properties" from the new desktop
    select_desktop(arg.i);


    // Map all windows
    if(head != NULL)
        for(c=head;c;c=c->next)
            XMapWindow(disp,c->win);

    tile();
    update_current();
}

void client_to_desktop(const Arg arg) {
    client *tmp = current;
    int tmp2 = current_desktop;
    
    if(arg.i == current_desktop || current == NULL)
        return;

    // Add client to desktop
    select_desktop(arg.i);
    add_window(tmp->win);
    save_desktop(arg.i);

    // Remove client from current desktop
    select_desktop(tmp2);
    remove_window(current->win);

    tile();
    update_current();
}

void next_desktop() {
    int tmp = current_desktop;
    if(tmp== 9)
        tmp = 0;
    else
        tmp++;

    Arg a = {.i = tmp};
    change_desktop(a);
}


void save_desktop(int i) {
    desktops[i].master_size = master_size;
    desktops[i].mode = mode;
    desktops[i].head = head;
    desktops[i].current = current;
}

void select_desktop(int i) {
    head = desktops[i].head;
    current = desktops[i].current;
    master_size = desktops[i].master_size;
    mode = desktops[i].mode;
    current_desktop = i;
}


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

//spawn function
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

void tile() {
    client *c;
    int n = 0;
    int y = 0;

    // If only one window
    if(head != NULL && head->next == NULL) {
        XMoveResizeWindow(dis,head->win,0,0,sw-2,sh-2);
    }
    else if(head != NULL) {
        switch(mode) {
            case 0:
                // Master window
                XMoveResizeWindow(dis,head->win,0,0,master_size-2,sh-2);

                // Stack
                for(c=head->next;c;c=c->next) ++n;
                for(c=head->next;c;c=c->next) {
                    XMoveResizeWindow(dis,c->win,master_size,y,sw-master_size-2,(sh/n)-2);
                    y += sh/n;
                }
                break;
            case 1:
                for(c=head;c;c=c->next) {
                    XMoveResizeWindow(dis,c->win,0,0,sw,sh);
                }
                break;
            default:
                break;
        }
    }
}

void update_current() {
    client *c;

    for(c=head;c;c=c->next)
        if(current == c) {
            // "Enable" current window
            XSetWindowBorderWidth(dis,c->win,1);
            XSetWindowBorder(dis,c->win,win_focus);
            XSetInputFocus(dis,c->win,RevertToParent,CurrentTime);
            XRaiseWindow(dis,c->win);
        }
        else
            XSetWindowBorder(dis,c->win,win_unfocus);
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
	
	int master_size = sw*MASTER_SIZE;
	
	//grabbing input
	XSelectInput(disp, root, SubstructureRedirectMask);
	grabkeys(root);
	
	// List of client
    tail = NULL;
    current = NULL;

    // Set up all desktop
    int i;
    for(i=0;i<TABLENGTH(desktops);++i) {
        desktops[i].tail = head;
        desktops[i].current = current;
    }
    
	//event loop
	while (1 && !XNextEvent(d, &ev)) // 1 && will forever be here.
		if (events[ev.type]) events[ev.type](&ev);
	
	//close the display
	XCloseDisplay(disp);
	
}

