// Mod (Mod1 == alt) and master size
#ifndef CONFIG_H
#define CONFIG_H

#define MOD             Mod1Mask
#define MASTER_SIZE     0.6

const char* dmenucmd[] = {"dmenu_run",NULL};
const char* termcmd[] =  {"st",NULL};
const char* lockcmd[]  = {"xlock",NULL};
const char* next[]     = {"ncmpcpp","next",NULL};
const char* prev[]     = {"ncmpcpp","prev",NULL};
const char* toggle[]   = {"ncmpcpp","toggle",NULL };
const char* voldown[]  = {"amixer","set","PCM","5\%-",NULL};
const char* volup[]    = {"amixer","set","PCM","5\%+",NULL};

// Avoid multiple paste
#define DESKTOPCHANGE(K,N) \
    {  MOD,             K,                          change_desktop, {.i = N}}, \
    {  MOD|ShiftMask,   K,                          client_to_desktop, {.i = N}},

// Shortcuts
static struct key keys[] = {
    // MOD              KEY                         FUNCTION        ARGS
    {  MOD,             XK_h,                       decrease,       {NULL}},
    {  MOD,             XK_l,                       increase,       {NULL}},
    {  MOD,             XK_x,                       kill_client,    {NULL}},
    {  MOD,             XK_k,                       prev_win,       {NULL}},
    {  MOD,             XK_c,                       spawn,          {.com = lockcmd}},
    {  0,               XF86XK_AudioNext,           spawn,          {.com = next}},
    {  0,               XF86XK_AudioPrev,           spawn,          {.com = prev}},
    {  0,               XF86XK_AudioPlay,           spawn,          {.com = toggle}},
    {  0,               XF86XK_AudioLowerVolume,    spawn,          {.com = voldown}},
    {  0,               XF86XK_AudioRaiseVolume,    spawn,          {.com = volup}},
    {  MOD,             XK_p,                       spawn,          {.com = dmenucmd}},
    {  MOD,			    XK_Return,                  spawn,          {.com = termcmd}},
    {  MOD,             XK_Right,                   next_desktop,   {NULL}},
       DESKTOPCHANGE(   XK_0,                                       0)
       DESKTOPCHANGE(   XK_1,                                       1)
       DESKTOPCHANGE(   XK_2,                                       2)
       DESKTOPCHANGE(   XK_3,                                       3)
       DESKTOPCHANGE(   XK_4,                                       4)
       DESKTOPCHANGE(   XK_5,                                       5)
       DESKTOPCHANGE(   XK_6,                                       6)
       DESKTOPCHANGE(   XK_7,                                       7)
       DESKTOPCHANGE(   XK_8,                                       8)
       DESKTOPCHANGE(   XK_9,                                       9)
    
};
#endif
