#
# xsd2html - XSD documentation tool
# Author: Andras Varga, 2003
#

GIFTRANS=giftrans
XSDDOC_XSL=`dirname $0`/xsd2html.xsl
XSDDOC_PL=`dirname $0`/proc.pl
DOT=dot

# process command-line options
outdir=html

if [ -n $DOT ]; then
    have_dot=yes
fi

while [ $# -gt 0 ]; do
    case "$1" in
    -o)       shift; outdir=$1 ;;
    -x)       have_dot=no ;;
    --no-diagrams)  have_dot=no ;;
    -h)       do_help=y ;;
    --help)   do_help=y ;;
    -*)       echo "opp_neddoc: ${1}: invalid option, try -h for help" >&2
              exit 2 ;;
    *)        break ;;
    esac
    shift
done

if [ -n "$do_help" ]; then
    echo "xsd2html - XSD documentation tool, part of IPv6Suite"
    echo "(c) 2003 Andras Varga"
    echo ""
    echo "Usage: xsd2html [-o <dir>] xsdfile"
    echo " -o <dir>     output directory, defaults to ./html"
    echo " -x, --no-diagrams"
    echo "              do not generate usage and interitance diagrams"
    echo " -h, --help   displays this help text"
    echo ""
    exit
fi

if [ $# -ne 1 ]; then
        echo "xsd2html: no input file specified or more than one"
        exit 2
fi
xsdfile=$*


rm -fr $outdir
mkdir $outdir

echo "Applying XSLT stylesheet to XSD $xsdfile XSD..."

xsltproc --stringparam outputdir "$outdir" \
         --stringparam have_dot "$have-dot" \
         $XSDDOC_XSL $xsdfile > /dev/null || exit 1

if [ "$have_dot" = "yes" ]; then
    echo 'generating diagrams via '$DOT'...'
    for i in $outdir/*.dot; do
       #echo -n "$i "
       echo -n '#'
       gif=`echo $i | sed 's/dot$/gif/'`
       map=`echo $i | sed 's/dot$/map/'`
       $DOT -Tgif  <$i >$gif 2>>$outdir/dot.err || exit 2
       $DOT -Tcmap <$i >$map 2>>$outdir/dot.err || exit 3
       if [ -n "$GIFTRANS" ]; then
           mv $gif $gif.tmp || exit 4
           $GIFTRANS -t white $gif.tmp >$gif || exit 5
           rm $gif.tmp || exit 6
       fi
       rm $i
    done
    if [ -s $outdir/dot.err ]; then
	echo
	echo 'see warnings from '$DOT' in '$outdir'/dot.err';
    else
	rm $outdir/dot.err
    fi
fi

perl $XSDDOC_PL $outdir
