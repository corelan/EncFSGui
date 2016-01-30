#/bin/sh
echo Incrementing version number
major=`cat version.txt | cut -d. -f1`
minor=`cat version.txt | cut -d. -f2`
revision=`cat version.txt | cut -d. -f3`
revision=`expr $revision + 1`
echo $major.$minor.$revision > version.txt
echo "static const wxString g_encfsguiversion = "\"$major.$minor.$revision\"";" > version.h 
echo Current version:
cat version.txt