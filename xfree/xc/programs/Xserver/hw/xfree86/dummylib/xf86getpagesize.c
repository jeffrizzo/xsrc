/* $XFree86$ */

#include "X.h"
#include "os.h"
#include "xf86.h"
#include "xf86Priv.h"

/*
 * Utility functions required by libxf86_os. 
 */

int xf86getpagesize(void);

int
xf86getpagesize(void)
{
    return 4096;	/* not used */
}

