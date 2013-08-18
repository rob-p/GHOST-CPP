#/bin/bash

HOST=
LIBPATH=GHOST-Signature-Extractor/lib

if [ `uname` == "Darwin" ]
then
    HOST=Mac
    LIBS=`otool -L build/src/ComputeSubgraphSignatures | awk '{ if(NR>1) {print \$1}}'`
    EXTRACTCMD="DYLD_LIBRARY_PATH=lib:\$DYLD_LIBRARY_PATH bin/ComputeSubgraphSignatures \$@"
fi

echo $LIBS

if [ `uname` == "Linux" ]
then
    HOST=Linux
    LIBS=`ldd build/src/ComputeSubgraphSignatures | grep "=>" | awk '{ if(NF>3) {print $3} }'`
fi

mkdir -p GHOST-Signature-Extractor/lib
mkdir -p GHOST-Signature-Extractor/bin

LOCALLIBS=GHOST-Signature-Extractor/lib

for slib in $LIBS
do
    cp $slib $LOCALLIBS
done

cp build/src/ComputeSubgraphSignatures GHOST-Signature-Extractor/bin
echo "#/bin/bash" > GHOST-Signature-Extractor/ExtractSignatures.sh
echo $EXTRACTCMD >> GHOST-Signature-Extractor/ExtractSignatures.sh


