#!/bin/sh

urpmi rpm-build kdenetwork4-devel cmake
mkdir -p $HOME/rpmbuild/
# TODO: make archive
rpmbuild -bb package-scripts/mandriva.spec
