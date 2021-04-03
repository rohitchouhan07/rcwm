/* rc's window manager */

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <X11/Xatom.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>

#define CLEANMASK(mask) (mask & ~(numlockmask | LockMask))
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

typedef struct client client;
struct client{
	client *next;
	client *prev;
	Window win;
};

typedef struct desktop desktop;    
struct desktop{
	client *tail;
	client *current;
	client *head;
};

// Functions
static void add_window(Window w);
static void change_desktop(const Arg arg);
static void client_to_desktop(const Arg arg);
static void configure_request(XEvent *e);
static void decrease();
static unsigned long getcolor(const char* color);
static void grabkeys();
static void increase();
static void key_press(XEvent *e);
static void kill_client();
static void map_request(XEvent *e);
static void next_desktop();
static void notify_destroy(XEvent *e);
//static void notify_enter(XEvent *e) 
static void prev_desktop();
static void prev_win();
static void remove_window(Window w);
static void save_desktop(int i);
static void select_desktop(int i);
static void	send_kill_signal(Window win);
static void spawn(const Arg arg);
static void tile();
static void toggle_desktop();
static void update_current();
static int xsendkill(Window w);

#include "config.h"

//variables
static Display *disp;
static int screen;
static Window root;
static int sw, sh;
unsigned int numlockmask;
static int master_size;
static desktop desktops[10];
static int current_desktop;
static client *head;
static client *current;
static client *tail;
static int master_size;
static int last_desktop;
static unsigned int win_focus;
static unsigned int win_unfocus;

//events array
static void (*events[LASTEvent])(XEvent *e) = {
    [ConfigureRequest] = configure_request,
    [KeyPress]         = key_press,
    [MapRequest]       = map_request,
    [DestroyNotify]    = notify_destroy,
    //[EnterNotify]      = notify_enter,
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
	tail = c;
    current = c;
}

void change_desktop(const Arg arg) {
   if(arg.i == current_desktop)
	return;

    client *c;
	//unsigned int tmp = current_desktop;

    // Save current "properties"
    save_desktop(current_desktop);
    
    // Unmap all window
    if(head != NULL){
        for(c = head; c; c = c->next)
            XUnmapWindow(disp, c->win);
	}
	last_desktop = current_desktop;
    
    // Take "properties" from the new desktop
    select_desktop(arg.i);

    // Map all windows
    if(head != NULL)
        for(c = head; c; c = c->next)
            XMapWindow(disp, c->win);

    tile();
    update_current();
    //select_desktop(tmp);
	select_desktop(arg.i);
}

void client_to_desktop(const Arg arg) {
    if(arg.i == current_desktop || current == NULL)
        return;
    client *tmp = current;
    int tmp2 = current_desktop;
    

	
	// Add client to desktop
    select_desktop(arg.i);
    add_window(tmp->win);	
    save_desktop(arg.i);
       
    select_desktop(tmp2);
    // Remove client from current desktop
    remove_window(tmp->win);
    XUnmapWindow(disp, tmp->win);
	save_desktop(tmp2);
	update_current();
    tile();
    
}

void configure_request(XEvent *e) {
    XConfigureRequestEvent *ev = &e->xconfigurerequest;

    XConfigureWindow(disp, ev->window, ev->value_mask, &(XWindowChanges) {
        .x          = ev->x,
        .y          = ev->y,
        .width      = ev->width,
        .height     = ev->height,
        .sibling    = ev->above,
        .stack_mode = ev->detail
    });
}

void decrease() {
    if(master_size > 50) {
        master_size -= 10;
        tile();
    }
}

unsigned long getcolor(const char* color) {
    XColor c;
    Colormap map = DefaultColormap(disp,screen);

    if(!XAllocNamedColor(disp,map,color,&c,&c))
        fprintf(stderr, "Error parsing color!");

    return c.pixel;
}

void grabkeys(){
	unsigned int i,j;
    KeyCode code;

    // numlock workaround
    XModifierKeymap *modmap;
    numlockmask = 0;
    modmap = XGetModifierMapping(disp);
    for (i=0;i<8;++i) {
        for (j=0;j<modmap->max_keypermod;++j) {
            if(modmap->modifiermap[i * modmap->max_keypermod + j] == XKeysymToKeycode(disp, XK_Num_Lock))
                numlockmask = (1 << i);
        }
    }
    XFreeModifiermap(modmap);

    XUngrabKey(disp, AnyKey, AnyModifier, root);
    // For each shortcuts
    for(i=0;i<TABLENGTH(keys);++i) {
        code = XKeysymToKeycode(disp,keys[i].keysym);
        XGrabKey(disp, code, keys[i].mod, root, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(disp, code, keys[i].mod | LockMask, root, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(disp, code, keys[i].mod | numlockmask, root, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(disp, code, keys[i].mod | numlockmask | LockMask, root, True, GrabModeAsync, GrabModeAsync);
    }
}

void increase() {
    if(master_size < sw-50) {
        master_size += 10;
        tile();
    }
}

//keypress event handler
void key_press(XEvent *e) {
	unsigned int i;
    KeySym keysym;
    XKeyEvent *ev = &e->xkey;

    keysym = XkbKeycodeToKeysym(disp, (KeyCode)ev->keycode, 0, 0);
    //fprintf(stderr, "pressed key %s\n", XKeysymToString(keysym));
    for(i=0;i<TABLENGTH(keys); ++i) {
        if(keysym == keys[i].keysym && CLEANMASK(keys[i].mod) == CLEANMASK(ev->state)) {
            if(keys[i].function)
                keys[i].function(keys[i].arg);
        }
    }
}

void kill_client()
{
    if(head == NULL) return;
    if (!xsendkill(current->win)){
		XGrabServer(disp);
		XSetCloseDownMode(disp, DestroyAll);
		XKillClient(disp, current->win);
		XSync(disp, False);
		XUngrabServer(disp);
	}
}

void map_request(XEvent *e) {
    XMapRequestEvent *ev = &e->xmaprequest;

    // For fullscreen mplayer (and maybe some other program)
    client *c;
    for(c=head;c;c=c->next)
        if(ev->window == c->win) {
            XMapWindow(disp,ev->window);
            return;
        }

    add_window(ev->window);
    XMapWindow(disp,ev->window);
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

void notify_destroy(XEvent *e) {
    int i=0;
    client *c;
    XDestroyWindowEvent *ev = &e->xdestroywindow;

    // Uber (and ugly) hack ;) Lets see if this works
    for(c=head;c;c=c->next)
        if(ev->window == c->win)
            i++;
    
    // End of the hack
    if(i == 0)
        return;

    remove_window(ev->window);
    update_current();
    tile();
    
}
/* write this function
void notify_enter(XEvent *e) {
    while(XCheckTypedEvent(d, EnterNotify, e));

    for win if (c->w == e->xcrossing.window) update_current();
}
* */

void prev_desktop() {
    int tmp = current_desktop;
    if(tmp == 0)
        tmp = 9;
    else
        tmp--;

    Arg a = {.i = tmp};
    change_desktop(a);
}

void prev_win() {
    client *c;

    if(current != NULL && head != NULL) {
        if(current->prev == NULL)
            for(c=head;c->next;c=c->next);
        else
            c = current->prev;

        current = c;
        update_current();
    }
}

void remove_window(Window w) {
    client *c;

    // CHANGE THIS UGLY CODE
    for(c=head;c;c=c->next) {

        if(c->win == w) {
            if(c->prev == NULL && c->next == NULL) {
                free(head);
                head = NULL;
                current = NULL;
                tail = NULL;
                return;
            }

            if(c->prev == NULL) {
                head = c->next;
                c->next->prev = NULL;
                current = c->next;
            }
            else if(c->next == NULL) {
                tail = c->prev;
                c->prev->next = NULL;
                current = c->prev;
            }
            else {
                c->prev->next = c->next;
                c->next->prev = c->prev;
                current = c->prev;
            }

            free(c);
            return;
        }
    }
}



void save_desktop(int i) {
    desktops[i].tail = tail;
    desktops[i].head = head;
    desktops[i].current = current;
}

void select_desktop(int i) {
    head = desktops[i].head;
    current = desktops[i].current;
	tail = desktops[i].tail;
    current_desktop = i;
}

void send_kill_signal(Window win)
{
	if(current)
		XKillClient(disp, win);
}

//spawn function
void spawn(const Arg arg) {
    if (fork()) return;
    if (disp) close(ConnectionNumber(disp));

    setsid();
    execvp((char*)arg.com[0], (char**)arg.com);
}


void tile() {
    client *c;
    int n = 0;
    int y = 0;

    // If only one window
    if(tail != NULL && tail->prev == NULL) {
        XMoveResizeWindow(disp,head->win,0,0,sw,sh);
    }
    else if(tail != NULL) {

                // Master window
                XMoveResizeWindow(disp,tail->win,0 + 10, 0 + 10, master_size - 15 ,sh - 2*10 );

                // Stack
                for(c=tail->prev;c;c=c->prev) ++n;
                for(c=tail->prev;c;c=c->prev) {
                    XMoveResizeWindow(disp,c->win,master_size + 5, y + 10 ,sw-master_size - 15, (sh/n) - 20);
                    y += sh/n - 5;
                }
}
   
}

void toggle_desktop(){
	Arg arg = {.i = last_desktop};
	change_desktop(arg);
}

void update_current() {
    client *c;

    for(c=head;c;c=c->next)
        if(c == current) {
            // "Enable" current window
            XSetWindowBorderWidth(disp,c->win,1);
            XSetWindowBorder(disp,c->win,win_focus);
            XSetInputFocus(disp,c->win,RevertToParent,CurrentTime);
            XRaiseWindow(disp,c->win);
        }
        else
            XSetWindowBorder(disp,c->win,win_unfocus);
}

int xsendkill(Window w){
	int n;
	Atom* protocols;
	XEvent ev;
	int exists = 0;
	Atom destproto = XInternAtom(disp, "WM_DELETE_WINDOW", False);

	if (XGetWMProtocols(disp, w, &protocols, &n)){
		while (!exists && n--)
			exists = (protocols[n] == destproto);
		XFree(protocols);
	}

	if (exists){
		ev.type = ClientMessage;
		ev.xclient.window = w;
		ev.xclient.message_type = XInternAtom(disp, "WM_PROTOCOLS", True);
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = destproto;
		ev.xclient.data.l[1] = CurrentTime;
		XSendEvent(disp, w, False, NoEventMask, &ev);
	}

	return exists;
}


int main(void){
	
	//try to open the display
	if (!(disp = XOpenDisplay(0))){
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
	XEvent ev;
	
	master_size = sw*MASTER_SIZE;
	// Select first dekstop by default
    const Arg arg = {.i = 1};
    current_desktop = arg.i;
    last_desktop = current_desktop;
    change_desktop(arg);
    
     // Colors
    win_focus = getcolor(FOCUS);
    win_unfocus = getcolor(UNFOCUS);
    
    // To catch maprequest and destroynotify (if other wm running)
	//grabbing input
	XSelectInput(disp, root, SubstructureNotifyMask|SubstructureRedirectMask);
	XDefineCursor(disp, root, XCreateFontCursor(disp, 68));
	grabkeys(root);
	
	// List of client
	head = NULL;
    tail = NULL;
    current = NULL;

    // Set up all desktop
    int i;
    for(i=0;i<TABLENGTH(desktops);++i) {
        desktops[i].head = head;
        desktops[i].current = current;
        desktops[i].tail = tail;
    }
    
	//event loop
	while (1 && !XNextEvent(disp, &ev)) // 1 && will forever be here.
		if (events[ev.type]) events[ev.type](&ev);
	
	//close the display
	XCloseDisplay(disp);
	
}

