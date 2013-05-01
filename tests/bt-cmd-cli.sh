#!/bin/sh
# run buzztrax-cmd commandline options

cd tests
. ./bt-cfg.sh

# test the output a little
echo "testing output"
$LIBTOOL $BUZZTRAX_CMD --help | grep >/dev/null -- "--help-bt-core"
if [ $? -ne 0 ]; then exit 1; fi

$LIBTOOL $BUZZTRAX_CMD --version | grep >/dev/null -- "buzztrax-cmd from buzztrax"
if [ $? -ne 0 ]; then exit 1; fi

# here we test that these don't crash
echo "testing options"
$LIBTOOL $BUZZTRAX_CMD >/dev/null 2>&1 --quiet
$LIBTOOL $BUZZTRAX_CMD >/dev/null 2>&1 --command
$LIBTOOL $BUZZTRAX_CMD >/dev/null 2>&1 --command=info
$LIBTOOL $BUZZTRAX_CMD >/dev/null 2>&1 --command=play
$LIBTOOL $BUZZTRAX_CMD >/dev/null 2>&1 --command=convert
$LIBTOOL $BUZZTRAX_CMD >/dev/null 2>&1 --command=encode
$LIBTOOL $BUZZTRAX_CMD >/dev/null 2>&1 --command=does_not_exist
$LIBTOOL $BUZZTRAX_CMD >/dev/null 2>&1 --input-file
$LIBTOOL $BUZZTRAX_CMD >/dev/null 2>&1 --input-file=$TESTSONGDIR
$LIBTOOL $BUZZTRAX_CMD >/dev/null 2>&1 --output-file
$LIBTOOL $BUZZTRAX_CMD >/dev/null 2>&1 --output-file=$TESTRESULTDIR

# do something real
echo "testing scenarios"
echo "... play"
$LIBTOOL $BUZZTRAX_CMD >/dev/null 2>&1 --command=play --input-file=$TESTSONGDIR/test-simple1.xml
$LIBTOOL $BUZZTRAX_CMD >/dev/null 2>&1 --command=play -q --input-file=$TESTSONGDIR/simple1.xml
$LIBTOOL $BUZZTRAX_CMD >/dev/null 2>&1 --command=play -q --input-file=$TESTSONGDIR/simple6.xml

$LIBTOOL $BUZZTRAX_CMD >/dev/null 2>&1 --command=convert -q --input-file=$TESTSONGDIR/test-simple1.xml --output-file=$TESTRESULTDIR/test-simple1.out.xml
echo "... convert"
if [ ! -r $TESTRESULTDIR/test-simple1.out.xml ]; then exit 1; fi
rm -f $TESTRESULTDIR/test-simple1.out.xml

echo "... encode"
$LIBTOOL $BUZZTRAX_CMD >/dev/null 2>&1 --command=encode -q --input-file=$TESTSONGDIR/test-simple1.xml --output-file=$TESTRESULTDIR/test-simple1.ogg
if [ ! -r $TESTRESULTDIR/test-simple1.ogg ]; then exit 1; fi
rm -f $TESTRESULTDIR/test-simple1.ogg
$LIBTOOL $BUZZTRAX_CMD >/dev/null 2>&1 --command=encode -q --input-file=$TESTSONGDIR/test-simple1.xml --output-file=$TESTRESULTDIR/test-simple1
if [ ! -r $TESTRESULTDIR/test-simple1.ogg ]; then exit 1; fi
rm -f $TESTRESULTDIR/test-simple1.ogg


# check what happens when we face a broken setup
#GST_PLUGIN_SYSTEM_PATH=/tmp GST_PLUGIN_PATH=/tmp ./src/ui/cmd/buzztrax-cmd --command=play --input-file=tests/songs/melo3.xml
