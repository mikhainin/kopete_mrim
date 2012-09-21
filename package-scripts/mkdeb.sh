#!/bin/sh

apt-get install libkopete-dev
git-buildpackage --git-upstream-tree=master --git-ignore-new

