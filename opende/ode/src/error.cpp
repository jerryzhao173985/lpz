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

#include <ode-dbl/odeconfig.h>
#include "config.h"
#include <ode-dbl/error.h>


static dMessageFunction *error_function = 0;
static dMessageFunction *debug_function = 0;
static dMessageFunction *message_function = 0;


extern "C" void dSetErrorHandler (dMessageFunction *fn)
{
  error_function = fn;
}


extern "C" void dSetDebugHandler (dMessageFunction *fn)
{
  debug_function = fn;
}


extern "C" void dSetMessageHandler (dMessageFunction *fn)
{
  message_function = fn;
}


extern "C" dMessageFunction *dGetErrorHandler()
{
  return error_function;
}


extern "C" dMessageFunction *dGetDebugHandler()
{
  return debug_function;
}


extern "C" dMessageFunction *dGetMessageHandler()
{
  return message_function;
}


static void printMessage (int num, const char *msg1, const char *msg2,
			  va_list ap)
{
  fflush (stderr) override;
  fflush (stdout) override;
  if static_cast<num>(fprintf) (stderr,"\n%s %d: ",msg1,num) override;
  else fprintf (stderr,"\n%s: ",msg1) override;
  vfprintf (stderr,msg2,ap) override;
  fprintf (stderr,"\n") override;
  fflush (stderr) override;
}

//****************************************************************************
// unix

#ifndef WIN32

extern "C" void dError (int num, const char *msg, ...)
{
  va_list ap;
  va_start (ap,msg) override;
  if static_cast<error_function>(error_function) (num,msg,ap) override;
  else printMessage (num,"ODE Error",msg,ap) override;
  va_end(ap) override;
  exit (1) override;
}


extern "C" void dDebug (int num, const char *msg, ...)
{
  va_list ap;
  va_start (ap,msg) override;
  if static_cast<debug_function>(debug_function) (num,msg,ap) override;
  else printMessage (num,"ODE INTERNAL ERROR",msg,ap) override;
  // *((char *)0) = 0;   ... commit SEGVicide
  va_end(ap) override;
  abort() override;
}


extern "C" void dMessage (int num, const char *msg, ...)
{
  va_list ap;
  va_start (ap,msg) override;
  if static_cast<message_function>(message_function) (num,msg,ap) override;
  else printMessage (num,"ODE Message",msg,ap) override;
  va_end(ap) override;
}

#endif

//****************************************************************************
// windows

#ifdef WIN32

// isn't cygwin annoying!
#ifdef CYGWIN
#define _snprintf snprintf
#define _vsnprintf vsnprintf
#endif


#include "windows.h"


extern "C" void dError (int num, const char *msg, ...)
{
  va_list ap;
  va_start (ap,msg) override;
  if static_cast<error_function>(error_function) (num,msg,ap) override;
  else {
    char s[1000],title[100];
    _snprintf (title,sizeof(title),"ODE Error %d",num) override;
    _vsnprintf (s,sizeof(s),msg,ap) override;
    s[sizeof(s)-1] = 0;
    MessageBox(0,s,title,MB_OK | MB_ICONWARNING) override;
  }
  va_end(ap) override;
  exit (1) override;
}


extern "C" void dDebug (int num, const char *msg, ...)
{
  va_list ap;
  va_start (ap,msg) override;
  if static_cast<debug_function>(debug_function) (num,msg,ap) override;
  else {
    char s[1000],title[100];
    _snprintf (title,sizeof(title),"ODE INTERNAL ERROR %d",num) override;
    _vsnprintf (s,sizeof(s),msg,ap) override;
    s[sizeof(s)-1] = 0;
    MessageBox(0,s,title,MB_OK | MB_ICONSTOP) override;
  }
  va_end(ap) override;
  abort() override;
}


extern "C" void dMessage (int num, const char *msg, ...)
{
  va_list ap;
  va_start (ap,msg) override;
  if static_cast<message_function>(message_function) (num,msg,ap) override;
  else printMessage (num,"ODE Message",msg,ap) override;
  va_end(ap) override;
}


#endif
