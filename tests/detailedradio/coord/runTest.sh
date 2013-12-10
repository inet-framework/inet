#!/bin/bash

lPATH='.'
LIBSREF=( )
lINETPath='../../../src'
for lP in '../../../src' \
          '../../../src/base' \
          '../../../src/modules' \
          '../testUtils' \
          "$lINETPath"; do
    for pr in 'inet'; do
        if [ -d "$lP" ] && [ -f "${lP}/lib${pr}$(basename $lP).so" -o -f "${lP}/lib${pr}$(basename $lP).dll" ]; then
            lPATH="${lP}:$lPATH"
            LIBSREF=( '-l' "${lP}/${pr}$(basename $lP)" "${LIBSREF[@]}" )
        elif [ -d "$lP" ] && [ -f "${lP}/lib${pr}.so" -o -f "${lP}/lib${pr}.dll" ]; then
            lPATH="${lP}:$lPATH"
            LIBSREF=( '-l' "${lP}/${pr}" "${LIBSREF[@]}" )
        fi
    done
done
PATH="${PATH}:${lPATH}" #needed for windows
LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${lPATH}"
NEDPATH="../../../src:.."
NEDPATH="${NEDPATH}:$lINETPath"
export PATH
export NEDPATH
export LD_LIBRARY_PATH

lCombined='detailedradiotests'
lSingle='coord'
lIsComb=0
if [ ! -e ${lSingle} -a ! -e ${lSingle}.exe ]; then
    if [ -e ../${lCombined}.exe ]; then
        ln -s ../${lCombined}.exe ${lSingle}.exe
        lIsComb=1
    elif [ -e ../${lCombined} ]; then
        ln -s ../${lCombined}     ${lSingle}
        lIsComb=1
    fi
fi

./${lSingle} "${LIBSREF[@]}">  out.tmp 2>  err.tmp

[ x$lIsComb = x1 ] && rm -f ${lSingle} ${lSingle}.exe >/dev/null 2>&1
diff -I '^Assigned runID=' \
     -I '^Loading NED files from' \
     -I '^OMNeT++ Discrete Event Simulation' \
     -I '^Version: ' \
     -I '^     Speed:' \
     -I '^** Event #' \
     -w exp-output out.tmp >diff.log 2>/dev/null

if [ -s diff.log ]; then
    echo "FAILED counted $(( 1 + $(grep -c -e '^---$' diff.log) )) differences where #<=$(grep -c -e '^<' diff.log) and #>=$(grep -c -e '^>' diff.log); see $(basename $(cd $(dirname $0);pwd) )/diff.log"
    [ "$1" = "update-exp-output" ] && \
        cat out.tmp >exp-output
    exit 1
else
    echo "PASSED $(basename $(cd $(dirname $0);pwd) )"
    rm -f out.tmp diff.log err.tmp
fi
exit 0
