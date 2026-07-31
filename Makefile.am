
SUFFIXES =
TESTS =
noinst_PROGRAMS =
noinst_HEADERS =
check_PROGRAMS =
dist_noinst_SCRIPTS =
DISTCLEANFILES =
pkginclude_HEADERS =
EXTRA_DIST =

ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -I$(top_srcdir)/include

lib_LTLIBRARIES = libwolfprov.la

EXTRA_DIST+=ChangeLog.md
EXTRA_DIST+=README.md
EXTRA_DIST+=IDE
EXTRA_DIST+=examples
EXTRA_DIST+=docs
EXTRA_DIST+=patches
EXTRA_DIST+=provider.conf
EXTRA_DIST+=provider-fips.conf

# EXTRA_DIST+=IDE above copies the IDE tree into the tarball RECURSIVELY, and it
# is not .gitignore-aware. The Visual Studio project (IDE/WINVS) writes its build
# output inside the repo -- OutDir is $(SolutionDir)$(Configuration)\$(Platform)\,
# following wolfSSL's own convention -- so anyone who builds in-tree on Windows
# and then runs `make dist` would silently ship .dll/.obj/.lib/.pdb to customers.
#
# Strip it here rather than relying on the working tree happening to be clean.
dist-hook:
	rm -rf $(distdir)/IDE/WINVS/Release $(distdir)/IDE/WINVS/Debug
	rm -rf $(distdir)/IDE/WINVS/.vs
	find $(distdir)/IDE -type f \( -name '*.user' -o -name '*.VC.db' \
	    -o -name '*.VC.opendb' -o -name '*.obj' -o -name '*.pdb' \
	    -o -name '*.tlog' \) -delete

include src/include.am
include include/include.am
include test/include.am
include certs/include.am
include scripts/include.am
include examples/include.am

#noinst_PROGRAMS += bench
#bench_SOURCES    = bench.c
#bench_LDADD      = libwolfprov.la
#DISTCLEANFILES  += .libs/bench

test: check
# By default, make distcheck will only add wolfProvider/ as an include directory.
# So, for example, a file like unit.h that has #include
# <wolfprovider/we_logging.h> won't be able to find wp_logging.h, because
# wolfProvider/include isn't an include directory. We add it here.
#
# The OpenSSL lib and include directories won't be visible to make distcheck
# either because make distcheck runs ./configure with no other options. If
# OpenSSL isn't installed in a standard location, this naked configure command
# will fail to find it. We tell it where to find the include and lib directory
# for OpenSSL here.
#
# The '--with-wolfssl' doesn't get propagated during a distcheck either, but it
# is necessary when they are installed somewhere other than /usr/local.
AM_DISTCHECK_CONFIGURE_FLAGS=CPPFLAGS="-I@abs_top_srcdir@/include" --with-openssl=@OPENSSL_INSTALL_DIR@ --with-wolfssl=@WOLFSSL_INSTALL_DIR@
