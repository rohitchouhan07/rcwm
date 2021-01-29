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


// Shortcuts
static struct key keys[] = {
    // MOD              KEY                         FUNCTION        ARGS
    {  MOD,             XK_h,                       decrease,       	{NULL}},
    {  MOD,             XK_l,                       increase,       	{NULL}},
    {  MOD,             XK_w,                       kill_client,    	{NULL}},
    {  MOD,             XK_k,                       prev_win,       	{NULL}},
    {  MOD,             XK_c,                       spawn,          	{.com = lockcmd}},
    {  0,               XF86XK_AudioNext,           spawn,          	{.com = next}},
    {  0,               XF86XK_AudioPrev,           spawn,          	{.com = prev}},
    {  0,               XF86XK_AudioPlay,           spawn,          	{.com = toggle}},
    {  0,               XF86XK_AudioLowerVolume,    spawn,          	{.com = voldown}},
    {  0,               XF86XK_AudioRaiseVolume,    spawn,          	{.com = volup}},
    {  MOD,             XK_p,                       spawn,          	{.com = dmenucmd}},
    {  MOD,			    XK_Return,                  spawn,          	{.com = termcmd}},
    {  MOD,             XK_Right,                   next_desktop,   	{NULL}},
    {  MOD,             XK_0,                       change_desktop, 	{.i = 0}},
    {  MOD|ShiftMask,   XK_0,                       client_to_desktop,  {.i = 0}},
    {  MOD,             XK_1,                       change_desktop, 	{.i = 1}},
    {  MOD|ShiftMask,   XK_1,                       client_to_desktop,  {.i = 1}},
    {  MOD,             XK_2,                       change_desktop, 	{.i = 2}},
    {  MOD|ShiftMask,   XK_2,                       client_to_desktop,  {.i = 2}},
    {  MOD,             XK_3,                       change_desktop, 	{.i = 3}},
    {  MOD|ShiftMask,   XK_3,                       client_to_desktop,  {.i = 3}},
    {  MOD,             XK_4,                       change_desktop, 	{.i = 4}},
    {  MOD|ShiftMask,   XK_4,                       client_to_desktop,  {.i = 4}},
    {  MOD,             XK_5,                       change_desktop, 	{.i = 5}},
    {  MOD|ShiftMask,   XK_5,                       client_to_desktop,  {.i = 5}},
    {  MOD,             XK_6,                       change_desktop, 	{.i = 6}},
    {  MOD|ShiftMask,   XK_6,                       client_to_desktop,  {.i = 6}},
    {  MOD,             XK_7,                       change_desktop, 	{.i = 7}},
    {  MOD|ShiftMask,   XK_7,                       client_to_desktop,  {.i = 7}},
    {  MOD,             XK_8,                       change_desktop, 	{.i = 8}},
    {  MOD|ShiftMask,   XK_8,                       client_to_desktop,  {.i = 8}},
    {  MOD,             XK_9,                       change_desktop, 	{.i = 9}},
    {  MOD|ShiftMask,   XK_9,                       client_to_desktop,  {.i = 9}},
    
};
#endif
