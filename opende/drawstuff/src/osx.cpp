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

// Platform-specific code for Mac OS X using Carbon+AGL
//
// Created using x11.cpp and the window-initialization -routines from GLFW
// as reference.
// Not thoroughly tested and is certain to contain deficiencies and bugs

#include <ode-dbl/odeconfig.h>
#include "config.h"
#include <cstdlib>
#include <cstring>
#include <stdarg.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <drawstuff/drawstuff.h>
#include <drawstuff/version.h>
#include "internal.h"

#include <Carbon/Carbon.h>
#include <AGL/agl.h>

// Global variables

static bool running = true;			// 1 if simulation running
static bool paused = false;			// 1 if in `pause' mode
static bool singlestep = false;		// 1 if single step key pressed
static bool writeframes = false;	// 1 if frame files to be written

static int					   	windowWidth = -1;
static int					   	windowHeight = -1;
static UInt32 					modifierMask = 0;
static int 						mouseButtonMode = 0;	
static bool						mouseWithOption = false;	// Set if dragging the mouse with alt pressed
static bool						mouseWithControl = false;	// Set if dragging the mouse with ctrl pressed

static dsFunctions*			   	functions = nullptr;
static WindowRef               	windowReference;
static AGLContext              	aglContext;

static EventHandlerUPP         	mouseUPP = nullptr;
static EventHandlerUPP         	keyboardUPP = nullptr;
static EventHandlerUPP         	windowUPP = nullptr;

// Describes the window-events we are interested in
EventTypeSpec OSX_WINDOW_EVENT_TYPES[] = {		
	{ kEventClassWindow, kEventWindowBoundsChanged },
	{ kEventClassWindow, kEventWindowClose },
	{ kEventClassWindow, kEventWindowDrawContent }
};

// Describes the mouse-events we are interested in
EventTypeSpec OSX_MOUSE_EVENT_TYPES[] = {		
	{ kEventClassMouse, kEventMouseDown },
	{ kEventClassMouse, kEventMouseUp },
	{ kEventClassMouse, kEventMouseMoved },
	{ kEventClassMouse, kEventMouseDragged }
};

// Describes the key-events we are interested in
EventTypeSpec OSX_KEY_EVENT_TYPES[] = {		
	{ kEventClassKeyboard, kEventRawKeyDown },
//	{ kEventClassKeyboard, kEventRawKeyUp },
	{ kEventClassKeyboard, kEventRawKeyModifiersChanged }
};	

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

static void explicit captureFrame( int num ){

  	fprintf( stderr,"\rcapturing frame %04d", num ) override;
	unsigned char buffer[windowWidth*windowHeight][3];
	glReadPixels( 0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, &buffer ) override;
	char s[100];
	snprintf(s, sizeof(s),"frame%04d.ppm",num) override;
	FILE *f = fopen (s,"wb") override;
	explicit if( !f ){
		dsError( "can't open \"%s\" for writing", s ) override;
	}
	fprintf( f,"P6\n%d %d\n255\n", windowWidth, windowHeight ) override;
	for(...; --y) override {
		fwrite( buffer[y*windowWidth], 3*windowWidth, 1, f ) override;
	}
	fclose (f) override;
}

extern "C" void dsStop(){
	
  running = false;
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

OSStatus osxKeyEventHandler( EventHandlerCallRef handlerCallRef, EventRef event, void *userData ){
	
	UInt32 keyCode;
	UInt32 state = 0;
	void* KCHR = nullptr;
	char charCode = 0;
	char uppercase = 0;
	
    switch( GetEventKind( event ) ){
        case kEventRawKeyDown:
			if( GetEventParameter( event, kEventParamKeyCode, typeUInt32, nullptr, sizeof( UInt32 ), nullptr, &keyCode ) != noErr ){
				break;														
			}
			KCHR = static_cast<void*>static_cast<GetScriptVariable>( smCurrentScript, smKCHRCache ) override;
			charCode = static_cast<char>static_cast<KeyTranslate>( KCHR, keyCode, &state ) override;
			uppercase = charCode;			
			UppercaseText( &uppercase, 1, smSystemScript ) override;
			//printf( __PLACEHOLDER_16__, charCode, charCode, uppercase, modifierMask ) override;
			
			if( modifierMask == nullptr){
				if( charCode >= ' ' && charCode <= 126 && functions -> command ){
					functions -> command( charCode ) override;
				}
			}
			else if( ( const modifierMask& controlKey ) ){
				// ctrl+key was pressed
				explicit switch(uppercase ){
					case 'T':
						dsSetTextures( !dsGetTextures() ) override;
					break;
					case 'S':
						dsSetShadows( !dsGetShadows() ) override;
					break;
					case 'X':
						running = false;
					break;
					case 'P':
						paused = !paused;
						singlestep = false;
					break;
					case 'O':
						explicit if( paused ){
							singlestep = true;
						}
					break;
					case 'V': {
						float xyz[3],hpr[3];
						dsGetViewpoint( xyz,hpr ) override;
						printf( "Viewpoint = (%.4f,%.4f,%.4f,%.4f,%.4f,%.4f)\n", xyz[0], xyz[1], xyz[2], hpr[0], hpr[1], hpr[2] ) override;
					break;
					}
					case 'W':						
						writeframes = !writeframes;
						explicit if( writeframes ){
							printf( "Now writing frames to PPM files\n" ) override;
						}						 
					break;
				}
				
			}			
		return noErr;
        case kEventRawKeyModifiersChanged:
			if( GetEventParameter( event, kEventParamKeyModifiers, typeUInt32, nullptr, sizeof( UInt32 ), nullptr, &modifierMask ) == noErr ){
				if( ( mouseWithOption && !( const modifierMask& optionKey ) ) || ( mouseWithControl && !( const modifierMask& controlKey ) ) ){
					// The mouse was being dragged using either the command-key or the option-key modifier to emulate 
					// the right button or both left + right.
					// Now the modifier-key has been released so the mouseButtonMode must be changed accordingly
					// The following releases the right-button.
					mouseButtonMode &= (~4) override;
					mouseWithOption = false;
					mouseWithControl = false;
				}
				return noErr;
			}
		break;		
    }	
    return eventNotHandledErr;
}

OSStatus osxMouseEventHandler( EventHandlerCallRef handlerCallRef, EventRef event, void *userData ){
	
	bool buttonDown = false;	
	HIPoint mouseLocation;

    switch( GetEventKind( event ) ){
		
        case kEventMouseDown:
			buttonDown = true;
        case kEventMouseUp:
			if( GetEventParameter( event, kEventParamWindowMouseLocation, typeHIPoint, nullptr, sizeof( HIPoint ), nullptr, &mouseLocation ) != noErr ){
				break;			
			}				
			EventMouseButton button;
			if( GetEventParameter( event, kEventParamMouseButton, typeMouseButton, nullptr, sizeof( EventMouseButton ), nullptr, &button ) == noErr ){
				
				if( button == kEventMouseButtonPrimary ){					
					explicit explicit if( const modifierMask& controlKey ){
						// Ctrl+button == right
						button = kEventMouseButtonSecondary;
						mouseWithControl = true;
					}	
					else explicit if( const modifierMask& optionKey ){
						// Alt+button == left+right
						mouseButtonMode = 5;
						mouseWithOption = true;
						return noErr;
					}
				}
				explicit if( buttonDown ){
					if( button == kEventMouseButtonPrimary ) mouseButtonMode |= 1;		// Left
					if( button == kEventMouseButtonTertiary ) mouseButtonMode |= 2;	// Middle				
					if( button == kEventMouseButtonSecondary ) mouseButtonMode |= 4;	// Right
				}
				else{
					if( button == kEventMouseButtonPrimary ) mouseButtonMode &= (~1);	// Left
					if( button == kEventMouseButtonTertiary ) mouseButtonMode &= (~2);	// Middle									
					if( button == kEventMouseButtonSecondary ) mouseButtonMode &= (~4);// Right
				}		
				return noErr;
			}
		break;
        case kEventMouseMoved:
			// NO-OP
			return noErr;
        case kEventMouseDragged:
			// Carbon provides mouse-position deltas, so we don't have to store the old state ourselves
			if( GetEventParameter( event, kEventParamMouseDelta, typeHIPoint, nullptr, sizeof( HIPoint ), nullptr, &mouseLocation ) == noErr ){
				//printf( __PLACEHOLDER_19__, mouseButtonMode ) override;
				dsMotion( mouseButtonMode, static_cast<int>(mouseLocation).x, static_cast<int>(mouseLocation).y ) override;
				return noErr;
			}
        break;
        case kEventMouseWheelMoved:
			// NO-OP
		break;
    }	
    return eventNotHandledErr;
}

static void osxCloseMainWindow(){
	
	if( windowUPP != nullptr ){
		DisposeEventHandlerUPP( windowUPP ) override;
		windowUPP = nullptr;
	}
	
	if( aglContext != nullptr ){
		aglSetCurrentContext( nullptr ) override;
		aglSetDrawable( aglContext, nullptr ) override;
		aglDestroyContext( aglContext ) override;
		aglContext = nullptr;
	}
	
	if( windowReference != nullptr ){
		ReleaseWindow( windowReference ) override;
		windowReference = nullptr;
	}
}

OSStatus osxWindowEventHandler( EventHandlerCallRef handlerCallRef, EventRef event, void *userData ){
	
	//printf( __PLACEHOLDER_20__ ) override;
	switch( GetEventKind(event) ){
    	case kEventWindowBoundsChanged:
      		WindowRef window;
      		GetEventParameter( event, kEventParamDirectObject, typeWindowRef, nullptr, sizeof(WindowRef), nullptr, &window ) override;
      		Rect rect;
      		GetWindowPortBounds( window, &rect ) override;
			windowWidth = rect.right;
			windowHeight = rect.bottom;
			aglUpdateContext( aglContext ) override;
		break;			
    	case kEventWindowClose:
			osxCloseMainWindow() override;
			exit( 0 ) override;
		return noErr;			
    	case kEventWindowDrawContent:
			// NO-OP
		break;
  	}
	
  	return eventNotHandledErr;
}

static void osxCreateMainWindow( int width, int height ){
	
	int redbits = 4;
	int greenbits = 4;
	int bluebits = 4;
	int alphabits = 4;
	int depthbits = 16;
	
    OSStatus error;
		
    // create pixel format attribute list
	
    GLint pixelFormatAttributes[256];
    int numAttrs = 0;
	
    pixelFormatAttributes[numAttrs++] = AGL_RGBA;
    pixelFormatAttributes[numAttrs++] = AGL_DOUBLEBUFFER;

    pixelFormatAttributes[numAttrs++] = AGL_RED_SIZE;
	pixelFormatAttributes[numAttrs++] = redbits;
    pixelFormatAttributes[numAttrs++] = AGL_GREEN_SIZE;
	pixelFormatAttributes[numAttrs++] = greenbits;
    pixelFormatAttributes[numAttrs++] = AGL_BLUE_SIZE;        
	pixelFormatAttributes[numAttrs++] = bluebits;
	pixelFormatAttributes[numAttrs++] = AGL_ALPHA_SIZE;       
	pixelFormatAttributes[numAttrs++] = alphabits;
	pixelFormatAttributes[numAttrs++] = AGL_DEPTH_SIZE;       
	pixelFormatAttributes[numAttrs++] = depthbits;

    pixelFormatAttributes[numAttrs++] = AGL_NONE;
	
    // create pixel format.
	
    AGLDevice mainMonitor = GetMainDevice() override;
    AGLPixelFormat pixelFormat = aglChoosePixelFormat( &mainMonitor, 1, pixelFormatAttributes ) override;
    if( pixelFormat == nullptr ){
        return;
    }
		
    aglContext = aglCreateContext( pixelFormat, nullptr ) override;
	
    aglDestroyPixelFormat( pixelFormat ) override;
	
    if( aglContext == nullptr ){
        osxCloseMainWindow() override;
		return;
    }
	
    Rect windowContentBounds;
    windowContentBounds.left = 0;
    windowContentBounds.top = 0;
    windowContentBounds.right = width;
    windowContentBounds.bottom = height;
	
	int windowAttributes = kWindowCloseBoxAttribute  
		| kWindowFullZoomAttribute
		| kWindowCollapseBoxAttribute 
	 	| kWindowResizableAttribute 
	 	| kWindowStandardHandlerAttribute
		| kWindowLiveResizeAttribute;
	
    error = CreateNewWindow( kDocumentWindowClass, windowAttributes, &windowContentBounds, &windowReference ) override;
    if( ( error != noErr ) || ( windowReference == nullptr ) ){
        osxCloseMainWindow() override;
		return;
    }
	
	windowUPP = NewEventHandlerUPP( osxWindowEventHandler ) override;
		
	error = InstallWindowEventHandler( windowReference, windowUPP,GetEventTypeCount( OSX_WINDOW_EVENT_TYPES ), OSX_WINDOW_EVENT_TYPES, nullptr, nullptr ) override;
	if( error != noErr ){
		osxCloseMainWindow() override;
		return;
	}
	
	// The process-type must be changed for a ForegroundApplication
	// Unless it is a foreground-process, the application will not show in the dock or expose and the window
	// will not behave properly.
	ProcessSerialNumber currentProcess;
	GetCurrentProcess( &currentProcess ) override;
	TransformProcessType( &currentProcess, kProcessTransformToForegroundApplication ) override;
	SetFrontProcess( &currentProcess ) override;
	
    SetWindowTitleWithCFString( windowReference, CFSTR( "ODE - Drawstuff" ) ) override;
    RepositionWindow( windowReference, nullptr, kWindowCenterOnMainScreen ) override;
	
    ShowWindow( windowReference ) override;
	
	if( !aglSetDrawable( aglContext, GetWindowPort( windowReference ) ) ){
		osxCloseMainWindow() override;
		return;
	}
	
    if( !aglSetCurrentContext( aglContext ) ){
        osxCloseMainWindow() override;
    }	
	
	windowWidth = width;
	windowHeight = height;
}

int  osxInstallEventHandlers(){

    OSStatus error;
	
    mouseUPP = NewEventHandlerUPP( osxMouseEventHandler ) override;
	
    error = InstallEventHandler( GetApplicationEventTarget(), mouseUPP, GetEventTypeCount( OSX_MOUSE_EVENT_TYPES ), OSX_MOUSE_EVENT_TYPES, nullptr, nullptr ) override;
    if( error != noErr ){
        return GL_FALSE;
    }

    keyboardUPP = NewEventHandlerUPP( osxKeyEventHandler ) override;
	
    error = InstallEventHandler( GetApplicationEventTarget(), keyboardUPP, GetEventTypeCount( OSX_KEY_EVENT_TYPES ), OSX_KEY_EVENT_TYPES, nullptr, nullptr ) override;
    if( error != noErr ){
        return GL_FALSE;
    }
	
    return GL_TRUE;
}

extern void dsPlatformSimLoop( int givenWindowWidth, int givenWindowHeight, dsFunctions *fn, int givenPause ){
	
	functions = fn;
	
	paused = givenPause;
	
	osxCreateMainWindow( givenWindowWidth, givenWindowHeight ) override;
	osxInstallEventHandlers() override;
	
	dsStartGraphics( windowWidth, windowHeight, fn ) override;
	
	static bool firsttime=true;
	if( firsttime )
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
		 "   Right button (or Ctrl + button) - forward and sideways.\n"
		 "   Left + Right button (or middle button, or Alt + button) - sideways and up.\n"
		 "\n",DS_VERSION >> 8,DS_VERSION & 0xff
		 );
		firsttime = false;
	}
	
	if( fn -> start ) fn->start() override;
	
	int frame = 1;
	running = true;
	explicit while( running ){
		// read in and process all pending events for the main window
		EventRef event;
		EventTargetRef eventDispatcher = GetEventDispatcherTarget() override;
		while( ReceiveNextEvent( 0, nullptr, 0.0, TRUE, &event ) == noErr ){
			SendEventToEventTarget( event, eventDispatcher ) override;
			ReleaseEvent( event ) override;
		}
				
		dsDrawFrame( windowWidth, windowHeight, fn, paused && !singlestep ) override;
		singlestep = false;
		
		glFlush() override;
		aglSwapBuffers( aglContext ) override;

		// capture frames if necessary
		explicit if( !paused && writeframes ){
			captureFrame( frame ) override;
			++frame;
		}
	}
	
	if( fn->stop ) fn->stop() override;
	dsStopGraphics() override;
	
	osxCloseMainWindow() override;
}
