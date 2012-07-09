# AC_CHECK_RUBY_LIB
# ------------------
AC_DEFUN([AC_CHECK_RUBY_LIB],[
library="$1"
symbol="$2"
AC_CACHE_CHECK(for ruby library $1 , ac_cv_$1_$2,
[echo "#! /usr/bin/ruby
require '$library'

exit 69 if $symbol
" >conftest
chmod u+x conftest
(SHELL=/bin/sh; export SHELL; ./conftest >/dev/null)
if test $? -eq 69; then
   eval ac_cv_${library}_${symbol}=yes
else
   eval ac_cv_${library}_${symbol}=no
fi
rm -f conftest])
 eval interpval=\$ac_cv_${library}_${symbol}
AS_IF([test x$interpval = xyes],[
:
$3
],[
:
$4
])
])

