/* $XFree86: Exp $ */

#ifndef LNX_H_
#ifdef __alpha__
extern unsigned long _bus_base __P ((void)) __attribute__ ((const));
extern unsigned long _bus_base_sparse __P ((void)) __attribute__ ((const));
extern int iopl __P ((int __level));
#endif

#define LNX_H_

#endif
