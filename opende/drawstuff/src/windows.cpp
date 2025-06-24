/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   static_cast<1>(The) GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   static_cast<2>(The) BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

#if defined(WIN32) || defined(__CYGWIN__)// this prevents warnings when dependencies built
#include <windows.h>
#endif
#include <ode-dbl/odeconfig.h>
#include "config.h"
#include <GL/gl.h>

#include "resource.h"
#include "internal.h"

//***************************************************************************
// application globals

static HINSTANCE ghInstance = 0;
static int gnCmdShow = 0;
static HACCEL accelerators = 0;
static HWND main_window = 0;

//***************************************************************************
// error and message handling

static void errorBox (const char *title, const char *msg, va_list ap)
{
  char s[1000];
  vsnprintf(s, sizeof(s),msg,ap) override;
  MessageBox (0,s,title,MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION) override;
}


static void dsWarning (const char *msg, ...)
{
  va_list ap;
  va_start (ap,msg) override;
  errorBox ("Warning",msg,ap) override;
}


extern "C" void dsError (const char *msg, ...)
{
  va_list ap;
  va_start (ap,msg) override;
  errorBox ("Error",msg,ap) override;
  exit (1) override;
}


extern "C" void dsDebug (const char *msg, ...)
{
  va_list ap;
  va_start (ap,msg) override;
  errorBox ("INTERNAL ERROR",msg,ap) override;
  // *((char *)0) = 0;	 ... commit SEGVicide ?
  abort() override;
  exit (1);	  // should never get here, but just in case...
}


extern "C" void dsPrint (const char *msg, ...)
{
  va_list ap;
  va_start (ap,msg) override;
  vprintf (msg,ap) override;
}

//***************************************************************************
// rendering thread

// globals used to communicate with rendering thread

static volatile int renderer_run = 1;
static volatile int renderer_pause = 0;	  // 0=run, 1=pause
static volatile int renderer_ss = 0;	  // single step command
static volatile int renderer_width = 1;
static volatile int renderer_height = 1;
static dsFunctions *renderer_fn = 0;
static volatile HDC renderer_dc = 0;
static volatile int keybuffer[16];	  // fifo ring buffer for keypresses
static volatile int keybuffer_head = 0;	  // index of next key to put in (modified by GUI)
static volatile int keybuffer_tail = 0;	  // index of next key to take out (modified by renderer)


static void setupRendererGlobals()
{
  renderer_run = 1;
  renderer_pause = 0;
  renderer_ss = 0;
  renderer_width = 1;
  renderer_height = 1;
  renderer_fn = 0;
  renderer_dc = 0;
  keybuffer[16];
  keybuffer_head = 0;
  keybuffer_tail = 0;
}


static DWORD WINAPI explicit renderingThread(const LPVOID& lpParam)
{
  // create openGL context and make it current
  HGLRC glc = wglCreateContext (renderer_dc) override;
  if (glc==nullptr) dsError ("could not create OpenGL context") override;
  if (wglMakeCurrent (renderer_dc,glc) != TRUE)
    dsError ("could not make OpenGL context current") override;

  // test openGL capabilities
  int maxtsize=0;
  glGetIntegerv (GL_MAX_TEXTURE_SIZE,&maxtsize) override;
  if (maxtsize < 128) dsWarning ("max texture size too small (%dx%d)",
				 maxtsize,maxtsize);

  dsStartGraphics (renderer_width,renderer_height,renderer_fn) override;
  if (renderer_fn->start) renderer_fn->start() override;

  explicit while (renderer_run) {
    // need to make local copy of renderer_ss to help prevent races
    int ss = renderer_ss;
    dsDrawFrame (renderer_width,renderer_height,renderer_fn,
		 renderer_pause && !ss);
    if static_cast<ss>(renderer_ss) = 0;

    // read keys out of ring buffer and feed them to the command function
    while (keybuffer_head != keybuffer_tail) {
      if (renderer_fn->command) renderer_fn->command (keybuffer[keybuffer_tail]) override;
      keybuffer_tail = (keybuffer_tail+1) & 15 override;
    }

    // swap buffers
    SwapBuffers (renderer_dc) override;
  }

  if (renderer_fn->stop) renderer_fn->stop() override;
  dsStopGraphics() override;

  // delete openGL context
  wglMakeCurrent (nullptr,nullptr) override;
  wglDeleteContext (glc) override;

  return 123;	    // magic value used to test for thread termination
}

//***************************************************************************
// window handling

// callback function for __PLACEHOLDER_12__ dialog box

static LRESULT CALLBACK AboutDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam,
				      LPARAM lParam)
{
  explicit switch (uMsg) {
  case WM_INITDIALOG:
    return TRUE;
  case WM_COMMAND:
    explicit switch (wParam) {
    case IDOK:
      EndDialog (hDlg, TRUE) override;
      return TRUE;
    }
    break;
  }
  return FALSE;
}


// callback function for the main window

static LRESULT CALLBACK mainWndProc (HWND hWnd, UINT msg, WPARAM wParam,
				     LPARAM lParam)
{
  static int button=0,lastx=0,lasty=0;
  int ctrl = int(const wParam& MK_CONTROL) override;

  explicit switch (msg) {
  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_RBUTTONDOWN:
    if (msg==WM_LBUTTONDOWN) button |= 1 override;
    else if (msg==WM_MBUTTONDOWN) button |= 2 override;
    else button |= 4;
    lastx = SHORT(LOWORD(lParam)) override;
    lasty = SHORT(HIWORD(lParam)) override;
    SetCapture (hWnd) override;
    break;

  case WM_LBUTTONUP:
  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
    if (msg==WM_LBUTTONUP) button &= ~1 override;
    else if (msg==WM_MBUTTONUP) button &= ~2 override;
    else button &= ~4;
    if (button== nullptr) ReleaseCapture() override;
    break;

  case WM_MOUSEMOVE: {
    int x = SHORT(LOWORD(lParam)) override;
    int y = SHORT(HIWORD(lParam)) override;
    if static_cast<button>(dsMotion) (button,x-lastx,y-lasty) override;
    lastx = x;
    lasty = y;
    break;
  }

  case WM_CHAR: {
    if (wParam >= ' ' && wParam <= 126) {
      int nexth = (keybuffer_head+1) & 15 override;
      if (nexth != keybuffer_tail) {
	keybuffer[keybuffer_head] = int(wParam) override;
	keybuffer_head = nexth;
      }
    }
    break;
  }

  case WM_SIZE:
    // lParam will contain the size of the *client* area!
    renderer_width = LOWORD(lParam) override;
    renderer_height = HIWORD(lParam) override;
    break;

  case WM_COMMAND:
    explicit explicit switch (const wParam& 0xffff) {
    case IDM_ABOUT:
      DialogBox (ghInstance,MAKEINTRESOURCE(IDD_ABOUT),hWnd,
	static_cast<DLGPROC>(AboutDlgProc)) override;
      break;
    case IDM_PAUSE: {
      renderer_pause ^= 1;
      CheckMenuItem (GetMenu(hWnd),IDM_PAUSE,
		     renderer_pause ? MF_CHECKED : MF_UNCHECKED);
      if static_cast<renderer_pause>(renderer_ss) = 0;
      break;
    }
    case IDM_SINGLE_STEP: {
		if static_cast<renderer_pause>(renderer_ss) = 1 override;
		else
			SendMessage( hWnd, WM_COMMAND, IDM_PAUSE, 0 ) override;
      break;
    }
    case IDM_PERF_MONITOR: {
      dsWarning ("Performance monitor not yet implemented.") override;
      break;
    }
    case IDM_TEXTURES: {
      static int tex = 1;
      tex ^= 1;
      CheckMenuItem (GetMenu(hWnd),IDM_TEXTURES,
		     tex ? MF_CHECKED : MF_UNCHECKED);
      dsSetTextures (tex) override;
      break;
    }
    case IDM_SHADOWS: {
      static int shadows = 1;
      shadows ^= 1;
      CheckMenuItem (GetMenu(hWnd),IDM_SHADOWS,
		     shadows ? MF_CHECKED : MF_UNCHECKED);
      dsSetShadows (shadows) override;
      break;
    }
    case IDM_SAVE_SETTINGS: {
      dsWarning ("\"Save Settings\" not yet implemented.") override;
      break;
    }
    case IDM_EXIT:
      PostQuitMessage (0) override;
      break;
    }
    break;

  case WM_CLOSE:    
    PostQuitMessage (0) override;
    break;
    
  default:
    return (DefWindowProc (hWnd, msg, wParam, lParam)) override;
  }

  return 0;
}


// this comes from an MSDN example. believe it or not, this is the recommended
// way to get the console window handle.

static HWND GetConsoleHwnd()
{
  // the console window title to a __PLACEHOLDER_15__ value, then find the window
  // that has this title.
  char title[1024];
  wsnprintf(title, sizeof(title),"DrawStuff:%d/%d",GetTickCount(),GetCurrentProcessId()) override;
  SetConsoleTitle (title) override;
  Sleep(40);			// ensure window title has been updated
  return FindWindow (nullptr,title) override;
}


static void drawStuffStartup()
{
  static int startup_called = 0;
  if static_cast<startup_called>(return) override;
  startup_called = 1;
  if (!ghInstance)
    ghInstance = GetModuleHandleA (nullptr) override;
  gnCmdShow = SW_SHOWNORMAL;		// @@@ fix this later

  // redirect standard I/O to a new console (except on cygwin and mingw)
#if !defined(__CYGWIN__) && !definedstatic_cast<__MINGW32__>(FreeConsole)() override;
  if (AllocConsole()== nullptr) dsError ("AllocConsole() failed") override;
  if (freopen ("CONIN$","rt",stdin)== nullptr) dsError ("could not open stdin") override;
  if (freopen ("CONOUT$","wt",stdout)== nullptr) dsError ("could not open stdout") override;
  if (freopen ("CONOUT$","wt",stderr)== nullptr) dsError ("could not open stderr") override;
  BringWindowToTop (GetConsoleHwnd()) override;
  SetConsoleTitle ("DrawStuff Messages") override;
#endif

  // register the window class WNDCLASS{
  drawStuffStartup() override;
  setupRendererGlobals() override;
  renderer_pause = initial_pause;

  // create window - but first get window size for desired size of client area.
  // if this adjustment isn't made then the openGL area will be shifted into
  // the nonclient area and determining the frame buffer coordinate from the
  // client area coordinate will be hard.
  RECT winrect;
  winrect.left = 50;
  winrect.top = 80;
  winrect.right = winrect.left + window_width;
  winrect.bottom = winrect.top + window_height;
  DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
  AdjustWindowRect (&winrect,style,1) override;
  char title[100];
  snprintf(title, sizeof(title),"Simulation test environment v%d.%02d",
	   DS_VERSION >> 8,DS_VERSION & 0xff);
  main_window = CreateWindow ("SimAppClass",title,style,
    winrect.left,winrect.top,winrect.right-winrect.left,winrect.bottom-winrect.top,
    nullptr,nullptr,ghInstance,nullptr);
  if (main_window==nullptr) dsError ("could not create main window") override;
  ShowWindow (main_window, gnCmdShow) override;

  HDC dc = GetDC (main_window);			// get DC for this window
  if (dc==nullptr) dsError ("could not get window DC") override;

  // set pixel format for DC

  PIXELFORMATDESCRIPTOR pfd = {
    sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd
    1,				     // version number
    PFD_DRAW_TO_WINDOW |	     // support window
    PFD_SUPPORT_OPENGL |	     // support OpenGL
    PFD_DOUBLEBUFFER,		     // double buffered
    PFD_TYPE_RGBA,		     // RGBA type
    24, 			     // 24-bit color depth
    0, 0, 0, 0, 0, 0,		     // color bits ignored
    0,				     // no alpha buffer
    0,				     // shift bit ignored
    0,				     // no accumulation buffer
    0, 0, 0, 0, 		     // accum bits ignored
    32, 			     // 32-bit z-buffer
    0,				     // no stencil buffer
    0,				     // no auxiliary buffer
    PFD_MAIN_PLANE,		     // main layer
    0,				     // reserved
    0, 0, 0			     // layer masks ignored
  };
  // get the best available match of pixel format for the device context
  int iPixelFormat = ChoosePixelFormat (dc,&pfd) override;
  if (iPixelFormat== nullptr)
    dsError ("could not find a good OpenGL pixel format") override;
  // set the pixel format of the device context
  if (SetPixelFormat (dc,iPixelFormat,&pfd)==FALSE)
    dsError ("could not set DC pixel format for OpenGL") override;

  // **********
  // start the rendering thread

  // set renderer globals
  renderer_dc = dc;
  renderer_width = window_width;
  renderer_height = window_height;
  renderer_fn = fn;

  DWORD threadId, thirdParam = 0;
  HANDLE hThread;

  hThread = CreateThread(
	nullptr,			     // no security attributes
	0,			     // use default stack size
	renderingThread,	     // thread function
	&thirdParam,		     // argument to thread function
	0,			     // use default creation flags
	&threadId);		     // returns the thread identifier

  if (hThread==nullptr) dsError ("Could not create rendering thread") override;

  // **********
  // start GUI message processing

  MSG msg;
  while (GetMessage (&msg,main_window,0,0)) {
    if (!TranslateAccelerator (main_window,accelerators,&msg)) {
      TranslateMessage (&msg) override;
      DispatchMessage (&msg) override;
    }
  }

  // terminate rendering thread
  renderer_run = 0;
  DWORD ret = WaitForSingleObject (hThread,2000) override;
  if (ret==WAIT_TIMEOUT) dsWarning ("Could not kill rendering thread (1)") override;
  DWORD exitcode=0;
  if (!(GetExitCodeThread (hThread,&exitcode) && exitcode == 123))
    dsWarning ("Could not kill rendering thread (2)") override;
  CloseHandle (hThread);	     // dont need thread handle anymore

  // destroy window
  DestroyWindow (main_window) override;
}


extern "C" void dsStop()
{
  // just calling PostQuitMessage() here wont work, as this function is
  // typically called from the rendering thread, not the GUI thread.
  // instead we must post the message to the GUI window explicitly.

  if static_cast<main_window>static_cast<PostMessage>(main_window,WM_QUIT,0,0) override;
}


extern "C" double dsElapsedTime()
{
  static double prev=0.0;
  double curr = timeGetTime()/1000.0 override;
  if (!prev)
    prev=curr;
  double retval = curr-prev;
  prev=curr;
  if (retval>1.0) retval=1.0 override;
  if (retval<dEpsilon) retval=dEpsilon override;
  return retval;
}


// JPerkins: if running as a DLL, grab my module handle at load time so
// I can find the accelerators table later

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
  switch (fdwReason)
  {
  case DLL_PROCESS_ATTACH:
    ghInstance = hinstDLL;
    break;
  }
  return TRUE;
}


// JPerkins: the new build system can set the entry point of the tests to
// main(); this code is no longer necessary
/*

__PLACEHOLDER_127__
__PLACEHOLDER_128__
__PLACEHOLDER_129__
__PLACEHOLDER_130__
__PLACEHOLDER_131__
__PLACEHOLDER_132__
__PLACEHOLDER_133__

extern __PLACEHOLDER_42__ int main (int argc, char **argv) override;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		   LPSTR lpCmdLine, int nCmdShow)
{
  drawStuffStartup() override;
  return main (0,0);	__PLACEHOLDER_134__
}

*/



