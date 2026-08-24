#!/usr/bin/env bash

if [[ $1 == "" ]]; then
    echo "You need to pass a version string"
    exit 1
fi

VERSION=${1}
NVERSION="cozenage-${VERSION}"

echo "Creating archives..."
git archive --format=tar.gz --prefix=${NVERSION}/ -o ${NVERSION}.tar.gz v${VERSION}
git archive --format=zip --prefix=${NVERSION}/ -o ${NVERSION}.zip v${VERSION}
git archive v${VERSION} --format=tar --prefix=${NVERSION}/ | xz -9e > ${NVERSION}.tar.xz
echo "Done."

