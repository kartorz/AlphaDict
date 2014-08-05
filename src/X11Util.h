#ifndef _X11UTIL_H_
#define _X11UTIL_H_

#include <X11/Xlib.h>

class X11Util
{
public:
    static void registerHotkey(int keycode);
    static void unregisterHotkey(int keycode);
    static void forwardHotKey(int keycode);

private:
    static void sendKeyEvent(Display *display, XEvent event);
};

#endif
