proc generateLightSetup {} {

set cut_paste_input [stack 0]
version 4.6000
push $cut_paste_input
z2Pos {
 fov 35
 name z2Pos1
 selected true
 xpos -118
 ypos -251
}
set N1427238 [stack 0]
push $N1427238
pos2Normal {
 "FLIP X\n" true
 "FLIP Y\n" true
 "FLIP Z\n" true
 name pos2Normal1
 selected true
 xpos -262
 ypos -251
}
joLight {
 inputs 2
 name joLight1
 selected true
 xpos -189
 ypos -174
}


}

