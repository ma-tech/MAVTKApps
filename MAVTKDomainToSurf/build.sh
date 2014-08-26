export MA=/opt/MouseAtlas

cmake -D WOOLZ_DIR=/opt/MouseAtlas -D VTK_DIR=/opt/vis .

# For some reason cmake splits the lines in the linker file which
# causes the linking to fail, if so these lines will fix the file.
mv CMakeFiles/MAVTKDomainToSurf.dir/link.txt link.txt
awk 'ORS=" "' < link.txt > CMakeFiles/MAVTKDomainToSurf.dir/link.txt
rm -f link.txt

make VERBOSE=1

cp MAVTKDomainToSurf $MA/bin
chmod 755 $MA/bin/MAVTKDomainToSurf

