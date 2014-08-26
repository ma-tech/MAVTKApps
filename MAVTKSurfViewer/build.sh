#!/bin/sh
# This script will install the MAVTKSurfViewer application. Uncomment
# the appropriate configure command lines for the build you want. The
# easiest way to use this script is probably to copy it to mybuild.sh and
# the edit that script.

set -x
#export MA=$HOME
#export MA=$HOME/MouseAtlas/Build/
export MA=/opt/MouseAtlas

cp MAVTKSurfViewer $MA/bin
chmod 755 $MA/bin/MAVTKSurfViewer
