proc generateGlassSetup {} {

set cut_paste_input [stack 0]
version 4.6000
push 0
push 0
z2Pos {
 fov 35
 name z2Pos1
 selected true
 xpos -202
 ypos 80
}
set N14949a8 [stack 0]
push $N14949a8
pos2Normal {
 "FLIP X\n" true
 "FLIP Y\n" true
 "FLIP Z\n" true
 name pos2Normal1
 selected true
 xpos -323
 ypos 80
}
set N1495bc8 [stack 0]
joRefract {
 inputs 3
 "Index of refraction" 35
 name joRefract1
 selected true
 xpos -323
 ypos 163
}
set N1495d20 [stack 0]
push $N14949a8
push $N1495bc8
push $N1495d20
vec2st {
 "Flip s with t" true
 "Flip s" true
 "Flip t" true
 name vec2st3
 selected true
 xpos -323
 ypos 234
}
push $cut_paste_input
SphericalTransform {
 input "Lat Long map"
 name SphericalTransform1
 selected true
 xpos -510
 ypos 212
}
set N14977d0 [stack 0]
STMap {
 inputs 2
 uv rgb
 name STMap4
 selected true
 xpos -323
 ypos 304
}
push 0
push $N14949a8
push $N1495bc8
joReflect {
 inputs 3
 "Index of refraction" 20
 name joReflect1
 selected true
 xpos -202
 ypos 168
}
vec2st {
 "Flip s with t" true
 "Flip s" true
 "Flip t" true
 name vec2st1
 selected true
 xpos -202
 ypos 235
}
push $N14977d0
STMap {
 inputs 2
 uv rgb
 name STMap2
 selected true
 xpos -202
 ypos 304
}
joFresnel {
 inputs 5
 "Index of refraction" 1.66
 name joFresnel1
 selected true
 xpos -267
 ypos 403
}


}

