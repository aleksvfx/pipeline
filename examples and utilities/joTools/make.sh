#!/bin/sh
echo COMPILING...
cd joLight
scons
mv libjoLight.linuxfc4_intel ~/NUKE/joLight.linuxfc4_intel

cd ../pos2Normal
scons
mv libpos2Normal.linuxfc4_intel ~/NUKE/pos2Normal.linuxfc4_intel

cd ../z2Pos
scons
mv libz2Pos.linuxfc4_intel ~/NUKE/z2Pos.linuxfc4_intel

cd ../vec2st
scons
mv libvec2st.linuxfc4_intel ~/NUKE/vec2st.linuxfc4_intel

cd ../joReflect
scons
mv libjoReflect.linuxfc4_intel ~/NUKE/joReflect.linuxfc4_intel

cd ../joRefract
scons
mv libjoRefract.linuxfc4_intel ~/NUKE/joRefract.linuxfc4_intel

cd ../joFresnel
scons
mv libjoFresnel.linuxfc4_intel ~/NUKE/joFresnel.linuxfc4_intel

cd ../joGradVec
scons
mv libjoGradVec.linuxfc4_intel ~/NUKE/joGradVec.linuxfc4_intel

cd ../joSpotLight
scons
mv libjoSpotLight.linuxfc4_intel ~/NUKE/joSpotLight.linuxfc4_intel

cd ../joSTMapper
scons
mv libjoSTMapper.linuxfc4_intel ~/NUKE/joSTMapper.linuxfc4_intel

cd ../joDot
scons
mv libjoDot.linuxfc4_intel ~/NUKE/joDot.linuxfc4_intel

cd ../joCross
scons
mv libjoCross.linuxfc4_intel ~/NUKE/joCross.linuxfc4_intel

cd ../joNormalize
scons
mv libjoNormalize.linuxfc4_intel ~/NUKE/joNormalize.linuxfc4_intel