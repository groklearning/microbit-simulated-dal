INPUT=../microbit-micropython/source/microbit/microbitconstimage.cpp
cat $INPUT | awk '/^IMAGE_T/ { name=$2; data=""; } /^   / { data=data $1 } /^);/ { print name " = [" data "]" }' | sed 's/\([,[]\)1/\19/g' | sed -r 's/microbit_const_image_([^ ]*)/\1/g' | sed 's/_obj//g' | tr 'a-z' 'A-Z' | sed 's/^/    self./g' | sed 's/,/, /g'
