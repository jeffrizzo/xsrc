#	$NetBSD: Makefile,v 1.20 2000/11/08 06:35:41 is Exp $
#
# Targets & Variables
#
# build: Clean out xsrc, and build and install everything that goes
#	under /usr/X11R6.
#
#  DESTDIR -- Set to an alternative directory to install under.
#  UPDATE --  If set, don't make clean first, plus attempt to make
#	only the targets that are out of date.
#
# release snapshot: Same as build, plus tar up the X sets and install
#	them under the ${RELEASEDIR}/binary/sets directory.
#
#  DESTDIR -- Same as for build. Mandatory for building a release.
#  RELEASEDIR -- As explained above.
#  BUILD_DONE -- If set, assume build is already done.
#  INSTALL_DONE -- If set, assume binaries to tar up are to be found
#	in ${DESTDIR} already.
#  BSDSRCDIR -- Set to the full path to the main source tree, /usr/src
#	by default. Needed to find ./distrib/sets.
#
# cleandir distclean: Remove all generated files from under xsrc.
#
# clean: Remove object files, but keep imake generated makefiles.

.include <bsd.own.mk>

.MAIN: all
all: all-xc all-contrib all-local

all-xc:
.if exists(xc/xmakefile) && defined(UPDATE)
	@cd xc && ${MAKE} Everything
.else
	@-rm -f xc/xmakefile
	@cd xc && ${MAKE} World
.endif

all-contrib:
	@if [ ! -f contrib/Makefile ]; then \
	  cd contrib && PATH=../xc/config/imake:$$PATH \
	    sh ../xc/config/util/xmkmf -a ../xc ../contrib; \
	fi
	@cd contrib && ${MAKE}

all-local:
	@if [ ! -f local/Makefile ]; then \
	  cd local && PATH=../xc/config/imake:$$PATH \
	    sh ../xc/config/util/xmkmf -a ../xc ../local; \
	fi
	@cd local && ${MAKE}

install: install-xc install-contrib install-local

install-xc:
	@cd xc && ${MAKE} install && ${MAKE} install.man

install-contrib:
	@cd contrib && ${MAKE} install && ${MAKE} install.man

install-local:
	@cd local && ${MAKE} install && ${MAKE} install.man

clean:
	@-cd xc && ${MAKE} clean
	@-cd contrib && ${MAKE} clean
	@-cd local && ${MAKE} clean

cleandir distclean: clean
	find xc contrib local -name .depend | xargs rm
	find xc contrib local -name 'Makefile*' | \
	    xargs grep -l "Makefile generated by imake" | xargs rm
	-rmdir xc/exports
	rm -f xc/xmakefile

build:
.if defined(UPDATE)
	@${MAKE} all install
.else
	@${MAKE} cleandir all install
.endif

# release goo
#
.if !defined(DESTDIR)
release snapshot:
	@echo setenv DESTDIR before doing that!
	@false
.elif !defined(RELEASEDIR)
release snapshot:
	@echo setenv RELEASEDIR before doing that!
	@false
#
.else
#
.if defined(INSTALL_DONE)
release snapshot:
.elif defined(BUILD_DONE)
release snapshot: install
.else
release snapshot: build
#
.endif # INSTALL_DONE or BUILD_DONE
#
	${INSTALL} -d -m 755 -o root -g wheel ${RELEASEDIR}/binary/sets
.if ${MACHINE} == "amiga"
	cd xc/programs/Xserver/hw/netbsd/amiga && \
		${INSTALL} -c -m 644 INSTALL.X11 ${RELEASEDIR}
.endif
	sh ${BSDSRCDIR}/distrib/sets/maketars -x -s ${BSDSRCDIR}/distrib/sets \
		-d ${DESTDIR} -t ${RELEASEDIR}/binary/sets
	cd ${RELEASEDIR}/binary/sets && \
		cksum -o 1 *.tgz >BSDSUM && \
		cksum *.tgz >CKSUM && \
		cksum -m *.tgz >MD5 && \
		cksum -o 2 *.tgz >SYSVSUM
#
.endif # DESTDIR and RELEASEDIR check

