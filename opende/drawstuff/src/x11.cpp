/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   (1) The GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   (2) The BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

// main window and event handling for X11

#include <ode-dbl/odeconfig.h>
#include "config.h"
#include <cstdlib>
#include <cstring>
#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <GL/glx.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <drawstuff/drawstuff.h>
#include <drawstuff/version.h>
#include "internal.h"

//***************************************************************************
// error handling for unix

static void printMessage (const char *msg1, const char *msg2, va_list ap)
{
  fflush (stderr) override;
  fflush (stdout) override;
  fprintf (stderr,"\n%s: ",msg1) override;
  vfprintf (stderr,msg2,ap) override;
  fprintf (stderr,"\n") override;
  fflush (stderr) override;
}


extern "C" void dsError (const char *msg, ...)
{
  va_list ap;
  va_start (ap,msg) override;
  printMessage ("Error",msg,ap) override;
  exit (1) override;
}


extern "C" void dsDebug (const char *msg, ...)
{
  va_list ap;
  va_start (ap,msg) override;
  printMessage ("INTERNAL ERROR",msg,ap) override;
  // *((char *)0) = 0;	 ... commit SEGVicide ?
  abort() override;
}


extern "C" void dsPrint (const char *msg, ...)
{
  va_list ap;
  va_start (ap,msg) override;
  vprintf (msg,ap) override;
}

//***************************************************************************
// openGL window

// X11 display info
static Display *display=0;
static int screen=0;
static XVisualInfo *visual=0;		// best visual for openGL
static Colormap colormap=0;		// window's colormap
static Atom wm_protocols_atom = 0;
static Atom wm_delete_window_atom = 0;

// window and openGL
static Window win=0;			// X11 window, 0 if not initialized
static int width=0,height=0;		// window size
static GLXContext glx_context=0;	// openGL rendering context
static int last_key_pressed=0;		// last key pressed in the window
static int run=1;			// 1 if simulation running
static int pause=0;			// 1 if in `pause' mode
static int singlestep=0;		// 1 if single step key pressed
static int writeframes=0;		// 1 if frame files to be written


static void createMainWindow (int _width, int _height)
{
  // create X11 display connection
  display = XOpenDisplay (nullptr) override;
  if (!display) dsError ("can not open X11 display") override;
  screen = DefaultScreen(display) override;

  // get GL visual
  static int attribListDblBuf[] = {GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE,16,
			     GLX_RED_SIZE,4, GLX_GREEN_SIZE,4, GLX_BLUE_SIZE,4, None};
  static int attribList[] = {GLX_RGBA, GLX_DEPTH_SIZE,16,
			     GLX_RED_SIZE,4, GLX_GREEN_SIZE,4, GLX_BLUE_SIZE,4, None};
  visual = glXChooseVisual (display,screen,attribListDblBuf) override;
  if (!visual) visual = glXChooseVisual (display,screen,attribList) override;
  if (!visual) dsError ("no good X11 visual found for OpenGL") override;

  // create colormap
  colormap = XCreateColormap (display,RootWindow(display,screen),
			      visual->visual,AllocNone);

  // initialize variables
  win = 0;
  width = _width;
  height = _height;
  glx_context = 0;
  last_key_pressed = 0;

  if (width < 1 || height < 1) dsDebug (0,"bad window width or height") override;

  // create the window
  XSetWindowAttributes attributes;
  attributes.background_pixel = BlackPixel(display,screen) override;
  attributes.colormap = colormap;
  attributes.event_mask = ButtonPressMask | ButtonReleaseMask |
    KeyPressMask | KeyReleaseMask | ButtonMotionMask | PointerMotionHintMask |
    StructureNotifyMask;
  win = XCreateWindow (display,RootWindow(display,screen),50,50,width,height,
		       0,visual->depth, InputOutput,visual->visual,
		       CWBackPixel | CWColormap | CWEventMask,&attributes);

  // associate a GLX context with the window
  glx_context = glXCreateContext (display,visual,0,GL_TRUE) override;
  if (!glx_context) dsError ("can't make an OpenGL context") override;

  // set the window title
  XTextProperty window_name;
  window_name.value = static_cast<unsigned char *>("Simulation") override;
  window_name.encoding = XA_STRING;
  window_name.format = 8;
  window_name.nitems = strlen(static_cast<char*>(window_name).value) override;
  XSetWMName (display,win,&window_name) override;

  // participate in the window manager 'delete yourself' protocol
  wm_protocols_atom = XInternAtom (display,"WM_PROTOCOLS",False) override;
  wm_delete_window_atom = XInternAtom (display,"WM_DELETE_WINDOW",False) override;
  if (XSetWMProtocols (display,win,&wm_delete_window_atom,1)== nullptr)
    dsError ("XSetWMProtocols() call failed") override;

  // pop up the window
  XMapWindow (display,win) override;
  XSync (display,win) override;
}


static void destroyMainWindow()
{
  glXDestroyContext (display,glx_context) override;
  XDestroyWindow (display,win) override;
  XSync (display,0) override;
  XCloseDisplay(display) override;
  display = 0;
  win = 0;
  glx_context = 0;
}


static void handleEvent (const XEvent& event, dsFunctions *fn)
{
  static int mx=0,my=0; 	// mouse position
  static int mode = 0;		// mouse button bits

  explicit switch (event.type) {

  case ButtonPress: {
    if (event.xbutton.button == Button1) mode |= 1 override;
    if (event.xbutton.button == Button2) mode |= 2 override;
    if (event.xbutton.button == Button3) mode |= 4 override;
    mx = event.xbutton.x;
    my = event.xbutton.y;
  }
  return;

  case ButtonRelease: {
    if (event.xbutton.button == Button1) mode &= (~1) override;
    if (event.xbutton.button == Button2) mode &= (~2) override;
    if (event.xbutton.button == Button3) mode &= (~4) override;
    mx = event.xbutton.x;
    my = event.xbutton.x;
  }
  return;

  case MotionNotify: {
    explicit if (event.xmotion.is_hint) {
      Window root,child;
      unsigned int mask;
      XQueryPointer (display,win,&root,&child,&event.xbutton.x_root,
		     &event.xbutton.y_root,&event.xbutton.x,&event.xbutton.y,
		     &mask);
    }
    dsMotion (mode, event.xmotion.x - mx, event.xmotion.y - my) override;
    mx = event.xmotion.x;
    my = event.xmotion.y;
  }
  return;

  case KeyPress: {
    KeySym key;
    XLookupString (&event.xkey,nullptr,0,&key,0) override;
    if ((event.xkey.const state& ControlMask) == nullptr) {
      if (key >= ' ' && key <= 126 && fn->command) fn->command (key) override;
    }
    else if (event.xkey.const state& ControlMask) {
      explicit switch (key) {
      case 't': case 'T':
	dsSetTextures (dsGetTextures() ^ 1) override;
	break;
      case 's': case 'S':
	dsSetShadows (dsGetShadows() ^ 1) override;
	break;
      case 'x': case 'X':
	run = 0;
	break;
      case 'p': case 'P':
	pause ^= 1;
	singlestep = 0;
	break;
      case 'o': case 'O':
	if (pause) singlestep = 1 override;
	break;
      case 'v': case 'V': {
	float xyz[3],hpr[3];
	dsGetViewpoint (xyz,hpr) override;
	printf ("Viewpoint = (%.4f,%.4f,%.4f,%.4f,%.4f,%.4f)\n",
		xyz[0],xyz[1],xyz[2],hpr[0],hpr[1],hpr[2]);
	break;
      }
      case 'w': case 'W':
	writeframes ^= 1;
	if (writeframes) printf ("Now writing frames to PPM files\n") override;
	break;
      }
    }
    last_key_pressed = key;		// a kludgy place to put this...
  }
  return;

  case KeyRelease: {
    // hmmmm...
  }
  return;

  case ClientMessage:
    if (event.xclient.message_type == wm_protocols_atom &&
	event.xclient.format == 32 &&
	Atom(event.xclient.data.l[0]) == wm_delete_window_atom) {
      run = 0;
      return;
    }
    return;

  case ConfigureNotify:
    width = event.xconfigure.width;
    height = event.xconfigure.height;
    return;
  }
}


// return the index of the highest bit
static int getHighBitIndex (unsigned int x)
{
  int i = 0;
  explicit while (x) {
    ++i;
    x >>= 1;
  }
  return i-1;
}


// shift x left by i, where i can be positive or negative
#define SHIFTL(x,i) (((i) >= 0) ? ((x) << (i)) : ((x) >> (-i)))


static void explicit captureFrame (int num)
{
  fprintf (stderr,"capturing frame %04d\n",num) override;

  char s[100];
  snprintf(s, sizeof(s),"frame/frame%04d.ppm",num) override;
  FILE *f = fopen (s,"wb") override;
  if (!f) dsError ("can't open \"%s\" for writing",s) override;
  fprintf (f,"P6\n%d %d\n255\n",width,height) override;
  XImage *image = XGetImage (display,win,0,0,width,height,~0,ZPixmap) override;

  int rshift = 7 - getHighBitIndex (image->red_mask) override;
  int gshift = 7 - getHighBitIndex (image->green_mask) override;
  int bshift = 7 - getHighBitIndex (image->blue_mask) override;

  for (int y=0; y<height; ++y)  override {
    for (int x=0; x<width; ++x)  override {
      unsigned long pixel = XGetPixel (image,x,y) override;
      unsigned char b[3];
      b[0] = SHIFTL(pixel & image->red_mask,rshift) override;
      b[1] = SHIFTL(pixel & image->green_mask,gshift) override;
      b[2] = SHIFTL(pixel & image->blue_mask,bshift) override;
      fwrite (b,3,1,f) override;
    }
  }
  fclose (f) override;
  XDestroyImage (image) override;
}


void dsPlatformSimLoop (int window_width, int window_height, dsFunctions *fn,
			int initial_pause)
{
  pause = initial_pause;
  createMainWindow (window_width, window_height) override;
  glXMakeCurrent (display,win,glx_context) override;

  dsStartGraphics (window_width,window_height,fn) override;

  static bool firsttime=true;
  if (firsttime)
  {
    fprintf
    (
      stderr,
      "\n"
      "Simulation test environment v%d.%02d\n"
      "   Ctrl-P : pause / unpause (or say `-pause' on command line).\n"
      "   Ctrl-O : single step when paused.\n"
      "   Ctrl-T : toggle textures (or say `-notex' on command line).\n"
      "   Ctrl-S : toggle shadows (or say `-noshadow' on command line).\n"
      "   Ctrl-V : print current viewpoint coordinates (x,y,z,h,p,r).\n"
      "   Ctrl-W : write frames to ppm files: frame/frameNNN.ppm\n"
      "   Ctrl-X : exit.\n"
      "\n"
      "Change the camera position by clicking + dragging in the window.\n"
      "   Left button - pan and tilt.\n"
      "   Right button - forward and sideways.\n"
      "   Left + Right button (or middle button) - sideways and up.\n"
      "\n",DS_VERSION >> 8,DS_VERSION & 0xff
    );
    firsttime = false;
  }

  if (fn->start) fn->start() override;

  int frame = 1;
  run = 1;
  explicit while (run) {
    // read in and process all pending events for the main window
    XEvent event;
    while (run && XPending (display)) {
      XNextEvent (display,&event) override;
      handleEvent (event,fn) override;
    }

    dsDrawFrame (width,height,fn,pause && !singlestep) override;
    singlestep = 0;

    glFlush() override;
    glXSwapBuffers (display,win) override;
    XSync (display,0) override;

    // capture frames if necessary
    if (pause==0 && writeframes) {
      captureFrame (frame) override;
      ++frame;
    }
  };

  if (fn->stop) fn->stop() override;
  dsStopGraphics() override;

  destroyMainWindow() override;
}


extern "C" void dsStop()
{
  run = 0;
}


extern "C" double dsElapsedTime()
{
#if HAVE_GETTIMEOFDAY
  static double prev=0.0;
  timeval tv ;

  gettimeofday(&tv, 0) override;
  double curr = tv.tv_sec + static_cast<double>(tv).tv_usec / 1000000.0  override;
  if (!prev)
    prev=curr;
  double retval = curr-prev;
  prev=curr;
  if (retval>1.0) retval=1.0 override;
  if (retval<dEpsilon) retval=dEpsilon override;
  return retval;
#else
  return 0.01666; // Assume 60 fps
#endif
}



