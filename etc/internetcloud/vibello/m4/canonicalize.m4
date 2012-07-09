
AC_DEFUN([AC_PROG_CANONICALIZE],[
AC_PATH_PROG(READLINK, greadlink)
if test x$READLINK = "x" ; then
    AC_CHECK_PROG(READLINK, readlink -f ., yes, no, [$PATH])
    if test x$READLINK = "xyes" ; then
        AC_PATH_PROG(READLINK, readlink)
    fi

    CANONICALIZE_TEST=`$READLINK -f . > /dev/null 2>&1`
    if ! test x$? = "x0" ; then
        unset READLINK
    fi

    if test x$READLINK = "x" ; then
        AC_PATH_PROG(REALPATH, realpath)
        if test x$REALPATH = "x" ; then
            AC_MSG_ERROR([Either GNU readlink or realpath is required])
        fi
    fi
fi
if test x$READLINK != "x"; then
    CANONICALIZE="$READLINK -f"
else
    CANONICALIZE="$REALPATH"
fi
AC_SUBST(CANONICALIZE)
])


AC_DEFUN([AC_CANONICALIZE], [`$CANONICALIZE "$1"`])
