/* $XConsortium: sunMouse.c,v 5.21 94/04/17 20:29:47 kaleb Exp $ */
/*-
 * Copyright (c) 1987 by the Regents of the University of California
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or X Consortium
not be used in advertising or publicity pertaining to 
distribution  of  the software  without specific prior 
written permission. Sun and X Consortium make no 
representations about the suitability of this software for 
any purpose. It is provided "as is" without any express or 
implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/
/*
 * Copyright 1991, 1992, 1993 Kaleb S. Keithley
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  Kaleb S. Keithley makes no 
 * representations about the suitability of this software for 
 * any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

#define NEED_EVENTS
#include    "alpha.h"
#include <stdio.h>

#if 0 /* XXX */
Bool alphaActiveZaphod = TRUE;
#endif /* 0 XXX */

static Bool alphaCursorOffScreen();
static void alphaCrossScreen();
static void alphaWarpCursor();

miPointerScreenFuncRec alphaPointerScreenFuncs = {
    alphaCursorOffScreen,
    alphaCrossScreen,
    alphaWarpCursor,
};

/*-
 *-----------------------------------------------------------------------
 * alphaMouseCtrl --
 *	Alter the control parameters for the mouse. Since acceleration
 *	etc. is done from the PtrCtrl record in the mouse's device record,
 *	there's nothing to do here.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
static 
#if NeedFunctionPrototypes
void alphaMouseCtrl (
    DeviceIntPtr    device,
    PtrCtrl*	    ctrl)
#else
void alphaMouseCtrl (device, ctrl)
    DeviceIntPtr    device;
    PtrCtrl*	    ctrl;
#endif
{
}

/*-
 *-----------------------------------------------------------------------
 * alphaMouseProc --
 *	Handle the initialization, etc. of a mouse
 *
 * Results:
 *	none.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
#if NeedFunctionPrototypes
int alphaMouseProc (
    DeviceIntPtr  device,
    int	    	  what)
#else
int alphaMouseProc (device, what)
    DeviceIntPtr  device;   	/* Mouse to play with */
    int	    	  what;	    	/* What to do with it */
#endif
{
    DevicePtr	  pMouse = (DevicePtr) device;
    int	    	  format;
    static int	  oformat;
    BYTE    	  map[4];
    char	  *dev;

fprintf(stderr, "alphaMouseProc (%d)\n", what);
fflush(stderr);
    switch (what) {
	case DEVICE_INIT:
fprintf(stderr, "init\n");
fflush(stderr);
	    if (pMouse != LookupPointerDevice()) {
		ErrorF ("Cannot open non-system mouse");	
		return !Success;
	    }
fprintf(stderr, "init a\n");
fflush(stderr);
	    if (alphaPtrPriv.fd == -1)
		return !Success;
fprintf(stderr, "init b\n");
fflush(stderr);
	    pMouse->devicePrivate = (pointer) &alphaPtrPriv;
	    pMouse->on = FALSE;
	    map[1] = 1;
	    map[2] = 2;
	    map[3] = 3;
	    InitPointerDeviceStruct(
		pMouse, map, 3, miPointerGetMotionEvents,
 		alphaMouseCtrl, miPointerGetMotionBufferSize());
	    break;

	case DEVICE_ON:
fprintf(stderr, "on\n");
fflush(stderr);
	    alphaPtrPriv.bmask = 0;
	    AddEnabledDevice (alphaPtrPriv.fd);
	    pMouse->on = TRUE;
fprintf(stderr, "on c\n");
fflush(stderr);
	    break;

	case DEVICE_CLOSE:
	    break;

	case DEVICE_OFF:
	    pMouse->on = FALSE;
	    RemoveEnabledDevice (alphaPtrPriv.fd);
	    break;
    }
    return Success;
}
    
/*-
 *-----------------------------------------------------------------------
 * alphaMouseGetEvents --
 *	Return the events waiting in the wings for the given mouse.
 *
 * Results:
 *	A pointer to an array of Firm_events or (Firm_event *)0 if no events
 *	The number of events contained in the array.
 *	A boolean as to whether more events might be available.
 *
 * Side Effects:
 *	None.
 *-----------------------------------------------------------------------
 */

#if NeedFunctionPrototypes
Firm_event* alphaMouseGetEvents (
    int		fd,
    int*	pNumEvents,
    Bool*	pAgain)
#else
Firm_event* alphaMouseGetEvents (fd, pNumEvents, pAgain)
    int		fd;
    int*	pNumEvents;
    Bool*	pAgain;
#endif
{
    int	    	  nBytes;	    /* number of bytes of events available. */
    static struct wscons_event evBuf[MAXEVENTS];   /* Buffer for Firm_events */

    if ((nBytes = read (fd, (char *)evBuf, sizeof(evBuf))) == -1) {
	if (errno == EWOULDBLOCK) {
	    *pNumEvents = 0;
	    *pAgain = FALSE;
	} else {
	    Error ("alphaMouseGetEvents read");
	    FatalError ("Could not read from mouse");
	}
    } else {
	*pNumEvents = nBytes / sizeof (Firm_event);
	*pAgain = (nBytes == sizeof (evBuf));
    }
    return evBuf;
}


/*-
 *-----------------------------------------------------------------------
 * MouseAccelerate --
 *	Given a delta and a mouse, return the acceleration of the delta.
 *
 * Results:
 *	The corrected delta
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
static short
MouseAccelerate (device, delta)
    DeviceIntPtr  device;
    int	    	  delta;
{
    int  sgn = sign(delta);
    PtrCtrl *pCtrl;
    short ret;

    delta = abs(delta);
    pCtrl = &device->ptrfeed->ctrl;
    if (delta > pCtrl->threshold) {
	ret = 
	    (short) sgn * 
		(pCtrl->threshold + ((delta - pCtrl->threshold) * pCtrl->num) /
		    pCtrl->den);
    } else {
	ret = (short) sgn * delta;
    }
    return ret;
}

/*-
 *-----------------------------------------------------------------------
 * alphaMouseEnqueueEvent --
 *	Given a Firm_event for a mouse, pass it off the the dix layer
 *	properly converted...
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The cursor may be redrawn...? devPrivate/x/y will be altered.
 *
 *-----------------------------------------------------------------------
 */

#if NeedFunctionPrototypes
void alphaMouseEnqueueEvent (
    DeviceIntPtr  device,
    struct wscons_event *fe)
#else
void alphaMouseEnqueueEvent (device, fe)
    DeviceIntPtr  device;   	/* Mouse from which the event came */
    struct wscons_event *fe;	    	/* Event to process */
#endif
{
    xEvent		xE;
    alphaPtrPrivPtr	pPriv;	/* Private data for pointer */
    int			bmask;	/* Temporary button mask */
    unsigned long	time;
    int			x, y;

    pPriv = (alphaPtrPrivPtr)device->public.devicePrivate;

    time = xE.u.keyButtonPointer.time = TVTOMILLI(fe->time);

    switch (fe->type) {
    case WSCONS_EVENT_MOUSE_UP:
    case WSCONS_EVENT_MOUSE_DOWN:
	/*
	 * A button changed state.  Sometimes we will get two events
	 * for a single status change.  Should we get a button event which
	 * reflects the current state of affairs, that event is discarded.
	 *
	 * Mouse buttons start at 1.
	 */
	xE.u.u.detail = fe->value + 1;
	bmask = 1 << xE.u.u.detail;
	if (fe->type == WSCONS_EVENT_MOUSE_UP) {
	    if (pPriv->bmask & bmask) {
		xE.u.u.type = ButtonRelease;
		pPriv->bmask &= ~bmask;
	    } else {
		return;
	    }
	} else {
	    if ((pPriv->bmask & bmask) == 0) {
		xE.u.u.type = ButtonPress;
		pPriv->bmask |= bmask;
	    } else {
		return;
	    }
	}
	mieqEnqueue (&xE);
	break;
    case WSCONS_EVENT_MOUSE_DELTA_X:
	miPointerDeltaCursor (MouseAccelerate(device,fe->value),0,time);
	break;
    case WSCONS_EVENT_MOUSE_DELTA_Y:
	/*
	 * For some reason, motion up generates a positive y delta
	 * and motion down a negative delta, so we must subtract
	 * here instead of add...
	 */
	miPointerDeltaCursor (0,-MouseAccelerate(device,fe->value),time);
	break;
    case WSCONS_EVENT_MOUSE_ABSOLUTE_X:
	miPointerPosition (&x, &y);
	miPointerAbsoluteCursor (fe->value, y, time);
	break;
    case WSCONS_EVENT_MOUSE_ABSOLUTE_Y:
	miPointerPosition (&x, &y);
	miPointerAbsoluteCursor (x, fe->value, time);
	break;
    default:
	FatalError ("alphaMouseEnqueueEvent: unrecognized id\n");
	break;
    }
}

/*ARGSUSED*/
static Bool
alphaCursorOffScreen (pScreen, x, y)
    ScreenPtr	*pScreen;
    int		*x, *y;
{
    int	    index, ret = FALSE;
    extern Bool PointerConfinedToScreen();

    if (PointerConfinedToScreen()) return TRUE;
#if 0 /* XXX */
    /*
     * Active Zaphod implementation:
     *    increment or decrement the current screen
     *    if the x is to the right or the left of
     *    the current screen.
     */
    if (sunActiveZaphod &&
	screenInfo.numScreens > 1 && (*x >= (*pScreen)->width || *x < 0)) {
	index = (*pScreen)->myNum;
	if (*x < 0) {
	    index = (index ? index : screenInfo.numScreens) - 1;
	    *pScreen = screenInfo.screens[index];
	    *x += (*pScreen)->width;
	} else {
	    *x -= (*pScreen)->width;
	    index = (index + 1) % screenInfo.numScreens;
	    *pScreen = screenInfo.screens[index];
	}
	ret = TRUE;
    }
#endif /* 0 XXX */
    return ret;
}

static void
alphaCrossScreen (pScreen, entering)
    ScreenPtr	pScreen;
    Bool	entering;
{
    if (alphaFbs[pScreen->myNum].EnterLeave)
	(*alphaFbs[pScreen->myNum].EnterLeave) (pScreen, entering ? 0 : 1);
}

static void
alphaWarpCursor (pScreen, x, y)
    ScreenPtr	pScreen;
    int		x, y;
{
    sigset_t newsigmask;

    (void) sigemptyset (&newsigmask);
    (void) sigaddset (&newsigmask, SIGIO);
    (void) sigprocmask (SIG_BLOCK, &newsigmask, (sigset_t *)NULL);
    miPointerWarpCursor (pScreen, x, y);
    (void) sigprocmask (SIG_UNBLOCK, &newsigmask, (sigset_t *)NULL);
}
