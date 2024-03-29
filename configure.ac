# -*- Autoconf -*-
#
# Copyright (c) 2010 Sandia Corporation
#

## --------------------- ##
## Autoconf Requirements ##
## --------------------- ##

AC_PREREQ(2.60)

## ---------------------------------- ##
## Autoconf / Automake Initialization ##
## ---------------------------------- ##
AC_INIT([portals4], [1.0a1], [portals4-devel@googlegroups.com])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([config])

dnl Automake's silent rules were implemented in the same version that
dnl color-tests was implemented, so we can use one to detect the other.
dnl This nasty, dirty, unreliable trick is strongly discouraged by its author:
dnl http://blog.flameeyes.eu/trackbacks?article_id=5155
m4_ifdef([AM_SILENT_RULES],
           [m4_define([ptl_color_tests], [color-tests])],
           [m4_define([ptl_color_tests], [])])
AM_INIT_AUTOMAKE([foreign check-news subdir-objects parallel-tests dist-bzip2 no-define tar-ustar 1.7 -Wall -Werror ]ptl_color_tests)
dnl If Automake supports silent rules, enable them
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# clean some extra things...
CLEANFILES="*~ .\#* .gdb_history"
AC_SUBST(CLEANFILES)

## -------------------------- ##
## Information on the package ##
## -------------------------- ##
AC_CANONICAL_HOST

## ----------------- ##
## Configure Options ##
## ----------------- ##

AC_ARG_ENABLE([picky],
  [AS_HELP_STRING([--enable-picky],
  [Enable extra compiler warnings (for developers of portals)])])
AS_IF([test "x$enable_picky" == x],
  [AS_IF([test -d "${srcdir}/.svn"],
     [echo "--> developer override: enable picky compiler by default"
      enable_picky=yes])])

AC_ARG_ENABLE([fast],
  [AS_HELP_STRING([--enable-fast],
     [Sets the available configuration options to run as quickly as possible. This makes assumptions and CAN result in bad things happening.])])
AS_IF([test "x$enable_fast" == xyes],
  [CPPFLAGS="$CPPFLAGS -DNDEBUG"
   CFLAGS="-O3 $CFLAGS"
   enable_arg_checking=no
   enable_strict_uid=no
   enable_hard_polling=yes
   enable_register_on_bind=yes])

AC_ARG_ENABLE([arg-checking],
  [AS_HELP_STRING([--disable-arg-checking],
    [Disable argument checking in Portals library.])])
AS_IF([test "x$enable_arg_checking" == xno],
  [AC_DEFINE([NO_ARG_VALIDATION], [1], [Define to disable argument checking])])

AC_ARG_WITH([cacheline-width],
  [AS_HELP_STRING([--with-cacheline-width=bytes],
    [Specify the cacheline width for the target machine. Defaults to 64.])],
  [],
  [with_cacheline_width=64])
AC_MSG_CHECKING([cacheline width])
AC_MSG_RESULT([$with_cacheline_width])
AC_DEFINE_UNQUOTED([CACHELINE_WIDTH], [$with_cacheline_width], [The cacheline width])

AS_IF([test "x$enable_register_on_bind" == xyes],
	  [AC_DEFINE([REGISTER_ON_BIND], [1], [Define that makes XFE memory registration happen at MDBind time, rather than at data movement time.])])

AC_ARG_ENABLE([zeromrs],
  [AS_HELP_STRING([--enable-zero-mrs],
    [Enable this when using MOFED V2.2+ or Qlogic InfiniPath Hardware of IB communication. (default: no)])])
AS_IF([test "x$enable_zero_mrs" == "xyes"],
  [AC_DEFINE([WITH_ZERO_MRS], [1], [Define to enable MOFED V2.2 or greater or Qlogic])])
AM_CONDITIONAL(WITH_ZERO_MRS, test "x$enable_zero_mrs" == xyes)

AC_ARG_ENABLE([trigmeops],
  [AS_HELP_STRING([--enable-me-triggered],
    [Enable extended (non-standard) triggered operations. Experimental. (default: no)])])
AS_IF([test "x$enable_me_triggered" == "xyes"],
  [AC_DEFINE([WITH_TRIG_ME_OPS], [1], [Define to enable triggered match list entry operations])])
AM_CONDITIONAL(WITH_TRIG_ME_OPS, test "x$enable_me_triggered" == xyes)

AC_ARG_ENABLE([transport-shmem],
  [AS_HELP_STRING([--enable-transport-shmem],
    [Use Shared memory for on-node communication. This is currently experimental and should be avoided. (default: off)])])

AC_ARG_ENABLE([transport-ib],
  [AS_HELP_STRING([--enable-transport-ib],
    [Use IB for remote communication.  (default: auto-detect)])])

AC_ARG_ENABLE([transport-udp],
  [AS_HELP_STRING([--enable-transport-udp],
    [Use UDP for remote communication.  Will only be chosen if IB is not built. Experimental. (default: auto-detect)])])

AC_ARG_ENABLE([reliable-udp],
  [AS_HELP_STRING([--enable-reliable-udp],
    [Use reliable UDP for remote communication. Must select this in addition to --enable-transport-udp. Experimental. (default: off)])])
AS_IF([test "x$enable_reliable_udp" == "xyes"],
  [AC_DEFINE([WITH_RUDP], [1], [Define to enable RUDP])
    reliable_udp=yes],
    [reliable_udp=no])
AM_CONDITIONAL(WITH_RUDP, test "x$enable_reliable_udp" == xyes)


AC_ARG_ENABLE([ib-shmem],
  [AS_HELP_STRING([--enable-ib-shmem],
    [Backward compatibility option for --enable-transport-shmem.])],
  [enable_transport_shmem=${enable_ib_shmem}])

AC_ARG_ENABLE([ib-ib],
  [AS_HELP_STRING([--enable-ib-ib],
    [Backward compatibility option for --enable-transport-ib.])],
  [enable_transport_ib=${enable_ib_ib}])

AC_ARG_ENABLE([ib-udp],
  [AS_HELP_STRING([--enable-ib-udp],
    [Backward compatibility option for --enable-transport-udp.])],
  [enable_transport_udp=${enable_ib_udp}])

AC_ARG_ENABLE([ppe],
  [AS_HELP_STRING([--enable-ppe],
    [Enable process-offload engine for Portals.  Experimental.  (default: disabled)])])
AS_IF([test "$enable_ppe" == "yes"],
  [AC_DEFINE([WITH_PPE], [1], [Define to enable PPE])
   enable_transport_shmem=no],
  [enable_ppe=no])
AM_CONDITIONAL([WITH_PPE], [test "$enable_ppe" = "yes"])

AC_ARG_ENABLE([kitten],
  [AS_HELP_STRING([--enable-kitten],
    [Enable support for the Kitten OS (default: disabled)])],
  [AC_DEFINE([HAVE_KITTEN], [1], [Defined to 1 if kitten support requested])])
AM_CONDITIONAL([HAVE_KITTEN], [test "$enable_kitten" = "yes"])

AC_ARG_ENABLE([libtool-wrapper],
  [AS_HELP_STRING([--disable-libtool-wrapper],
    [Disable use of libtool executable wrappers for tests])])
AS_IF([test "$enable_libtool_wrapper" = "no"], [LIBTOOL_WRAPPER_LDFLAGS="-no-install"], [LIBTOOL_WRAPPER_LDFLAGS=])
AC_SUBST([LIBTOOL_WRAPPER_LDFLAGS])

## ------------------- ##
## Checks for programs ##
## ------------------- ##
AC_PROG_AWK
AC_PROG_CC
AC_USE_SYSTEM_EXTENSIONS
m4_ifdef([AM_PROG_AR],[AM_PROG_AR])
AC_PROG_CC_C99
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AM_PROG_CC_C_O

LT_PREREQ([2.2])
LT_INIT([])

AC_CACHE_SAVE

## ----------------------- ##
## Checks for header files ##
## ----------------------- ##
AC_HEADER_STDC
AC_SYS_LARGEFILE
AC_CHECK_HEADERS([fcntl.h stddef.h stdint.h malloc.h sys/time.h limits.h endian.h])
AC_CHECK_HEADERS([sys/posix_shm.h], [], [],
[#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
])
AC_CHECK_HEADERS([arpa/inet.h limits.h netinet/in.h stddef.h \
        stdint.h stdlib.h string.h sys/file.h sys/socket.h \
        unistd.h syscall.h])

AM_PATH_XML2([2.6.0], [have_libxml=1], [have_libxml=0])
AM_CONDITIONAL([HAVE_LIBXML], [test "$have_libxml" = "1"])

## --------------- ##
## Check for types ##
## --------------- ##
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_PID_T
AC_CHECK_TYPES([uint_fast8_t, uint_fast32_t, uint_fast64_t])

## ---------------------------------- ##
## Check for compiler characteristics ##
## ---------------------------------- ##
SANDIA_DETECT_COMPILER_TYPE
AC_C_CONST
AC_C_INLINE
AC_C_VOLATILE
SANDIA_ALIGNED_ATTRIBUTE
SANDIA_UNUSED_ATTRIBUTE
SANDIA_NORETURN_ATTRIBUTE
SANDIA_BUILTIN_UNREACHABLE
# Find out if we need the -restrict flag
RESTRICT_CXXFLAGS=""
AS_IF([test "x$sandia_cv_cxx_compiler_type" = "xIntel"],
	  [CXXFLAGS="-restrict $CXXFLAGS"])

AX_C_RESTRICT
AS_IF([test "x$enable_picky" == xyes],
  [AS_CASE([$sandia_cv_c_compiler_type],
		   [GNU], [CFLAGS="-Wall -Wno-strict-aliasing -Wmissing-prototypes -Wstrict-prototypes $CFLAGS"],
		   [Intel], [CFLAGS="-Wall -wd981 $CFLAGS"],
		   [SunStudio], [], dnl This compiler defaults to enabling all warnings
		   [PortlandGroup], [CFLAGS="-Minform=inform $CFLAGS"],
		   [CFLAGS="-Wall $CFLAGS"])
   AS_CASE([$sandia_cv_cxx_compiler_type],
		   [GNU], [CXXFLAGS="-Wall $CXXFLAGS"],
		   [Intel], [CXXFLAGS="-Wall -wd981 $CXXFLAGS"],
		   [SunStudio], [], dnl This compiler defaults to enabling all warnings
		   [PortlandGroup], [CXXFLAGS="-Minform=inform $CXXFLAGS"],
		   [CXXFLAGS="-Wall $CXXFLAGS"])
   ])

SANDIA_CHECK_ATOMICS([],[AC_MSG_ERROR([Atomics are not implemented portably, consider upgrading to a newer compiler that supports builtin atomics])])
SANDIA_CHECK_BITFIELDS

AC_CACHE_SAVE

## --------------------------- ##
## Check for library functions ##
## --------------------------- ##

AC_FUNC_FORK
AC_FUNC_STRERROR_R
AC_FUNC_MMAP
AC_CHECK_FUNCS([syscall __munmap __mmap])
AC_CHECK_FUNCS([munmap]) # how absurd is this?
AC_CHECK_FUNCS([memalign posix_memalign], [break]) # first win
AC_CHECK_FUNCS([getpagesize tdestroy linux/ioctl.h]) # not mandatory
AC_CHECK_FUNCS([ftruncate getpagesize inet_ntoa memset select socket strerror strtol strtoul])
AC_CHECK_LIB([dl], [dlsym])
AC_CHECK_FUNCS([dlsym])

AC_SEARCH_LIBS([pthread_spin_init],[pthread],
			   [AC_DEFINE([HAVE_PTHREAD_SPIN_INIT], [1], [Define if pthread supports spinlocks])]) # not mandatory either, just faster
AC_CHECK_FUNCS([ftruncate memset setenv strtol strtoul select inet_ntoa socket strerror], [],
			   [AC_MSG_ERROR([required function not found])])
AC_CHECK_LIB([bsd-compat],[main]) # potentially important because we use BSD functions like ftruncate
AC_SEARCH_LIBS([shm_open],
	       [rt], [],
	       [AC_MSG_ERROR([Kick the lazy developer and make him support normal file mmaping])])
AC_SEARCH_LIBS([hwloc_topology_init], [hwloc],
			   [AC_DEFINE([HAVE_HWLOC],[1],[Define if hwloc is available])])
SANDIA_CHOOSE_TIMER

AS_IF([test "$enable_kitten" != "yes"],
  [SANDIA_CHECK_EV([], [AC_MSG_ERROR([libev not found.])])])

AS_IF([test "$enable_ppe" = "yes"],
  [SANDIA_CHECK_XPMEM([], [AC_MSG_ERROR([libxpmem not found.])])])

active_remote_transport=""

AS_IF([test "$active_remote_transport" == ""], 
  [AS_IF([test "$enable_transport_ib" != "no"],
         [SANDIA_CHECK_OFED([active_remote_transport="ib"], 
           [AS_IF([test "$enable_transport_ib" != ""], [AC_MSG_ERROR([OFED not found.])])])])])

AS_IF([test "$active_remote_transport" == ""], 
  [AS_IF([test "$enable_transport_udp" != "no"],
         [active_remote_transport="udp"])])

SANDIA_DETECT_KNEM()
AS_IF([test "x$enable_transport_shmem" = "xyes" -a "x$enable_ppe" != "xyes"],
      [AC_DEFINE([WITH_TRANSPORT_SHMEM], [1], [Define to enable shared memory support])
       transport_shmem="yes"],
      [transport_shmem="no"])
AM_CONDITIONAL(WITH_TRANSPORT_SHMEM, test "x$transport_shmem" == "xyes")

AS_IF([test "$active_remote_transport" = "" -a "$enable_ppe" = "no"],
  [AC_MSG_ERROR([No transport found.])])

AS_IF([test "$active_remote_transport" == "ib"],
  [AC_DEFINE([WITH_TRANSPORT_IB], [1], [Define to enable IB support])
   transport_ib="yes"],
  [transport_ib="no"])
AM_CONDITIONAL([WITH_TRANSPORT_IB], [test "$active_remote_transport" == "ib"])

AS_IF([test "$active_remote_transport" == "udp"],
  [AC_DEFINE([WITH_TRANSPORT_UDP], [1], [Define to enable UDP support])
   transport_udp="yes"],
  [transport_udp="no"])

AM_CONDITIONAL([WITH_TRANSPORT_UDP], [test "$active_remote_transport" == "udp"])

# figure out all the runtime stuff
AS_IF([test "$with_pmi" = "" -o "$with_pmi" = "no"],
  [want_runtime=1],
  [AS_IF([test "$enable_kitten" = "yes"], 
         [want_runtime=0
	  pmi_CPPFLAGS="-I${with_pmi}/include"
	  pmi_LDFLAGS="-L${with_pmi}/lib"
	  pmi_LIBS="-lpmi"],
	 [ORTE_CHECK_PMI([pmi], [want_runtime=0], [want_runtime=1])])])
AS_IF([test "$want_runtime" = "1"],
  [pmi_CPPFLAGS='-I$(top_srcdir)/src/runtime/portals4'
   pmi_LDFLAGS=
   pmi_LIBS='$(top_builddir)/src/runtime/libportals_runtime.la'
   TEST_RUNNER='$(top_builddir)/src/runtime/hydra/yod.hydra -np $(NPROCS)'],
  [TEST_RUNNER='yod -np $(NPROCS)'])
AC_SUBST(pmi_CPPFLAGS)
AC_SUBST(pmi_LDFLAGS)
AC_SUBST(pmi_LIBS)
AC_SUBST(TEST_RUNNER)
AM_CONDITIONAL([WANT_RUNTIME], [test "$want_runtime" = "1"])


## -------------------------- ##
## Check for library behavior ##
## -------------------------- ##
SANDIA_CHECK_8ALIGNED_MALLOC
SANDIA_CHECK_8ALIGNED_CALLOC
SANDIA_CHECK_16ALIGNED_MALLOC
SANDIA_CHECK_16ALIGNED_CALLOC
SANDIA_CHECK_WORKING_VALLOC
AS_IF([test "x$enable_hard_polling" != xyes],
	  [SANDIA_CHECK_PTHREAD_PROCESS_SHARED([AC_DEFINE([HAVE_PTHREAD_SHMEM_LOCKS],
 	   	  [1],
		  [Define if PTHREAD_PROCESS_SHARED attribute on mutexes and cond variables works])])])

AC_CACHE_SAVE

# Only export the library symbols 
AC_CACHE_CHECK(whether ld accepts --version-script, ac_cv_version_script,
    [AS_IF([test -n "`$LD --help < /dev/null 2>/dev/null | grep version-script`"],
           [LD_VERSION_SCRIPT='-Wl,--version-script=$(srcdir)/ib/portals4.map'],
		   [LD_VERSION_SCRIPT=''])])
AC_SUBST(LD_VERSION_SCRIPT)

## --------------- ##
## Output and done ##
## --------------- ##

DISTCHECK_CONFIGURE_FLAGS=
AS_IF([test -n "$enable_transport_shmem"],
  [DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --enable-transport-shmem=${enable_transport_shmem}"])
AS_IF([test -n "$enable_transport_udp"],
  [DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --enable-transport-udp=${enable_transport_udp}"])
AS_IF([test -n "$enable_transport_ib"],
  [DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --enable-transport-ib=${enable_transport_ib}"])
AS_IF([test -n "$with_xpmem"],
  [DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-xpmem=${with_xpmem}"])
AS_IF([test -n "$with_ev"],
  [DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-ev=${with_ev}"])
AS_IF([test -n "$with_ofed"],
  [DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-ofed=${with_ofed}"])
AC_SUBST(DISTCHECK_CONFIGURE_FLAGS)

AS_IF([test "$want_runtime" = "1"], [AC_CONFIG_SUBDIRS([src/runtime/hydra])])

AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_FILES([Makefile
                 doc/Makefile
                 include/Makefile
                 src/Makefile
		 src/ib/Makefile
		 src/runtime/Makefile
                 test/Makefile
		 test/basic/Makefile
		 test/benchmarks/Makefile
                 test/sfw/Makefile
                 test/sfw/test_n1/Makefile
                 test/sfw/test_n2/Makefile])
AC_OUTPUT

AS_IF([test "$enable_ppe" = "yes"],
      [progress_thread="no"
       progress_process="yes"],
      [progress_thread="yes"
       progress_process="no"])
AS_IF([test "$enable_qlogic" = "yes"],
      [ib_type="Qlogic"],
      [ib_type="Mellanox"])
echo ""
echo "  InfiniBand Device: $ib_type"
echo ""
echo "  Transport Support:"
echo "       InfiniBand: $transport_ib"
echo "              UDP: $transport_udp"
echo "     Reliable UDP: $enable_reliable_udp"
echo "    Shared memory: $transport_shmem"
echo "             KNEM: $knem_happy"
echo ""
echo "  Progress Support:"
echo "           Thread: $progress_thread"
echo "          Process: $progress_process"
echo ""
