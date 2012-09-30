#!/bin/sh

urpmi rpm-build kdenetwork4-devel cmake
# TODO: make archive
rpmbuild -bb package-scripts/mandriva.spec
