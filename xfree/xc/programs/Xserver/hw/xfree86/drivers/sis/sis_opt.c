/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/sis_opt.c,v 1.10 2002/01/17 09:57:30 eich Exp $ */

#include "xf86.h"
#include "xf86PciInfo.h"

#include "sis.h"

typedef enum {
    OPTION_SW_CURSOR,
    OPTION_HW_CURSOR,
    OPTION_PCI_RETRY,
    OPTION_RGB_BITS,
    OPTION_NOACCEL,
    OPTION_TURBOQUEUE,
    OPTION_FAST_VRAM,
    OPTION_SET_MEMCLOCK,
    OPTION_FORCE_CRT2TYPE,
	OPTION_SHADOW_FB,
    OPTION_ROTATE,
	OPTION_NOXVIDEO,
    OPTION_VESA,
    OPTION_MAXXFBMEM
} SISOpts;

static const OptionInfoRec SISOptions[] = {
    { OPTION_SW_CURSOR,         "SWcursor",     OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_HW_CURSOR,         "HWcursor",     OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_PCI_RETRY,         "PciRetry",     OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_RGB_BITS,          "rgbbits",      OPTV_INTEGER,   {0}, -1    },
    { OPTION_NOACCEL,           "NoAccel",      OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_TURBOQUEUE,        "TurboQueue",   OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_SET_MEMCLOCK,      "SetMClk",      OPTV_FREQ,      {0}, -1    },
    { OPTION_FAST_VRAM,         "FastVram",     OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_FORCE_CRT2TYPE,    "ForceCRT2Type",OPTV_ANYSTR,    {0}, FALSE },
    { OPTION_SHADOW_FB,         "ShadowFB",     OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_ROTATE,            "Rotate",       OPTV_ANYSTR,    {0}, FALSE },
    { OPTION_NOXVIDEO,          "NoXvideo",     OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_VESA,		"Vesa",		OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_MAXXFBMEM,         "MaxXFBMem",    OPTV_INTEGER,   {0}, -1 },
    { -1,                       NULL,           OPTV_NONE,      {0}, FALSE }
};

void SiSOptions(ScrnInfoPtr pScrn);
const OptionInfoRec * SISAvailableOptions(int chipid, int busid);

void
SiSOptions(ScrnInfoPtr pScrn)
{
    SISPtr  pSiS = SISPTR(pScrn);
    MessageType     from;
    double          temp;
    char *strptr;

    /* Collect all of the relevant option flags (fill in pScrn->options) */
    xf86CollectOptions(pScrn, NULL);

    /* Process the options */
    if (!(pSiS->Options = xalloc(sizeof(SISOptions))))
	return;
    memcpy(pSiS->Options, SISOptions, sizeof(SISOptions));
    xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, pSiS->Options);

    /* initalize some defaults */
    pSiS->FastVram = TRUE;
    pSiS->UsePCIRetry = TRUE;
    pSiS->TurboQueue = TRUE;
    pSiS->HWCursor = TRUE;
    pSiS->Rotate = FALSE;
    pSiS->ShadowFB = FALSE;
    pSiS->VESA = -1;
    pSiS->NoXvideo = FALSE;
    pSiS->maxxfbmem = 0;

    switch(pSiS->Chipset) {
        case PCI_CHIP_SIS530:
            pSiS->TurboQueue = FALSE; /* FIXME ? */
            break;
        default:
            break;
    }

#if 0 /* we only work with a depth greater or equal to 8 */
    if (pScrn->depth <= 8)  {
        if (xf86GetOptValInteger(pSiS->Options, OPTION_RGB_BITS,
                                &pScrn->rgbBits))
        {
            xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
                    "Bits per RGB set to %d\n", pScrn->rgbBits);
        }
    }
#endif

    /* sw/hw cursor */
    from = X_DEFAULT;
    if (xf86GetOptValBool(pSiS->Options, OPTION_HW_CURSOR, &pSiS->HWCursor)) {
        from = X_CONFIG;
    }
    if (xf86ReturnOptValBool(pSiS->Options, OPTION_SW_CURSOR, FALSE)) {
        from = X_CONFIG;
        pSiS->HWCursor = FALSE;
    }
    xf86DrvMsg(pScrn->scrnIndex, from, "Using %s cursor\n", 
                                pSiS->HWCursor ? "HW" : "SW");

    /* Accel */
    if (xf86ReturnOptValBool(pSiS->Options, OPTION_NOACCEL, FALSE)) {
        pSiS->NoAccel = TRUE;
        xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Acceleration disabled\n");
    }

    /* PCI retry */
    from = X_DEFAULT;
    if (xf86GetOptValBool(pSiS->Options, OPTION_PCI_RETRY, &pSiS->UsePCIRetry)) {
        from = X_CONFIG;
    }
    xf86DrvMsg(pScrn->scrnIndex, from, "PCI retry %s\n",
                         pSiS->UsePCIRetry ? "enabled" : "disabled");

    /* Mem clock */
    if (xf86GetOptValFreq(pSiS->Options, OPTION_SET_MEMCLOCK, OPTUNITS_MHZ,
                                                            &temp)) {
        pSiS->MemClock = (int)(temp * 1000.0);
        xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, 
                    "Memory clock set to %.3f MHz\n", pSiS->MemClock/1000.0);
    }

    /* fast VRAM */
    from = X_DEFAULT;
    if (xf86GetOptValBool(pSiS->Options, OPTION_FAST_VRAM, &pSiS->FastVram)) {
        from = X_CONFIG;
    }
    xf86DrvMsg(pScrn->scrnIndex, from, "Fast VRAM %s\n",
                         pSiS->FastVram ? "enabled" : "disabled");

    /* Turbo QUEUE */
    from = X_DEFAULT;
    if (xf86GetOptValBool(pSiS->Options, OPTION_TURBOQUEUE, &pSiS->TurboQueue)) {
        from = X_CONFIG;
    }
    xf86DrvMsg(pScrn->scrnIndex, from, "TurboQueue %s\n", 
                        pSiS->TurboQueue ? "enabled" : "disabled");

    /* CRT2 type */
    pSiS->ForceCRT2Type = CRT2_DEFAULT;
    strptr = (char *)xf86GetOptValString(pSiS->Options, OPTION_FORCE_CRT2TYPE);
    if (strptr != NULL)
    {
        if (!strcmp(strptr,"TV"))
            pSiS->ForceCRT2Type = CRT2_TV;
        if (!strcmp(strptr,"LCD"))
            pSiS->ForceCRT2Type = CRT2_LCD;
        if (!strcmp(strptr,"VGA"))
            pSiS->ForceCRT2Type = CRT2_VGA;
        if (!strcmp(strptr,"NONE"))
            pSiS->ForceCRT2Type = 0;

        if (pSiS->ForceCRT2Type != CRT2_DEFAULT)
            xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
                    "CRT2 Type set to: %s\n", strptr);
    }

    /* ShadowFB */
    from = X_DEFAULT;
    if (xf86GetOptValBool(pSiS->Options, OPTION_SHADOW_FB, &pSiS->ShadowFB)) {
        from = X_CONFIG;
    }
	if (pSiS->ShadowFB) {
		pSiS->NoAccel = TRUE;
    	xf86DrvMsg(pScrn->scrnIndex, from, 
				"Using \"Shadow Frame Buffer\" - acceleration disabled\n");
	}

    /* Rotate */
    if ((strptr = (char *)xf86GetOptValString(pSiS->Options, OPTION_ROTATE))) {
        if(!xf86NameCmp(strptr, "CW")) {
            pSiS->ShadowFB = TRUE;
            pSiS->NoAccel = TRUE;
            pSiS->HWCursor = FALSE;
            pSiS->Rotate = 1;
            xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
                "Rotating screen clockwise - acceleration disabled\n");
        } else
        if (!xf86NameCmp(strptr, "CCW")) {
            pSiS->ShadowFB = TRUE;
            pSiS->NoAccel = TRUE;
            pSiS->HWCursor = FALSE;
            pSiS->Rotate = -1;
            xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
                "Rotating screen counter clockwise - acceleration disabled\n");
        } else {
            xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
                    "\"%s\" is not a valid value for Option \"Rotate\"\n", strptr);
            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "Valid options are \"CW\" or \"CCW\"\n");
        }
    }

    /* NOXvideo */
    if (xf86ReturnOptValBool(pSiS->Options, OPTION_NOXVIDEO, FALSE)) {
        pSiS->NoXvideo = TRUE;
        xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "XVideo Extension Disabled\n");
    }

    /* VESA */
    /* 
     * This option is for overriding the default behavior.
     * By default it depends on the chipset/video bridge
     * combination if the VESA BIOS code is used to prevent
     * the "melting" screen effect due to lack of nowledge
     * about programming details.
     * You will normally not need this option.
     */
    {
	Bool val;
	
	if (xf86GetOptValBool(pSiS->Options, OPTION_VESA, &val)) {
	    if (val)
		pSiS->VESA = 1;
	    else
		pSiS->VESA = 0;

	    xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "VESA usage %s\n",
		       val ? "enabled":"disabled");
	}
    }
   /* With the option "MaxXFBMem" you can limit the amount of video memory X
    * uses for screen and off-screen buffers. This option should be used if
    * you intend to use DRI/DRM. The framebuffer driver required for DRM will
    * start its memory heap at 8MB if it detects more than that. So, if you
    * limit the amount of memory X uses, you avoid a clash between the framebuffer
    * driver and X as regards overwriting memory portions of each other.
    * The amount is to be specified in KB.
    */
    if (xf86GetOptValULong(pSiS->Options, OPTION_MAXXFBMEM,
                                &pSiS->maxxfbmem)) {
            xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
                    "Framebuffer memory limited to %d KB\n", pSiS->maxxfbmem);
	    pSiS->maxxfbmem *= 1024;
    }
}

const OptionInfoRec *
SISAvailableOptions(int chipid, int busid)
{
    return SISOptions;
}
