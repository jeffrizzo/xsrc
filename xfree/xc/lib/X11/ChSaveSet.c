/* $TOG: ChSaveSet.c /main/6 1998/02/06 17:07:53 kaleb $ */
/*

Copyright 1986, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

*/
/* $XFree86: xc/lib/X11/ChSaveSet.c,v 1.2 1999/05/09 10:49:01 dawes Exp $ */

#include "Xlibint.h"

int
XChangeSaveSet(dpy, win, mode)
register Display *dpy;
Window win;
int mode;
{
    register xChangeSaveSetReq *req;

    LockDisplay(dpy);
    GetReq(ChangeSaveSet, req);
    req->window = win;
    req->mode = mode;
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

int
XAddToSaveSet(dpy, win)
    register Display *dpy;
    Window win;
{
    return XChangeSaveSet(dpy,win,SetModeInsert);
}

int
XRemoveFromSaveSet (dpy, win)
    register Display *dpy;
    Window win;
{
    return XChangeSaveSet(dpy,win,SetModeDelete);
}
