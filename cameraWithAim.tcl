set cut_paste_input [stack 0]
version 6.0 v1
push $cut_paste_input
Camera2 {
 translate {25.25 13.25 0}
 name Camera1
 selected true
 xpos 2256
 ypos 1783
}

{
    addUserKnob node [stack 0] 20 Look
    addUserKnob node [stack 0] 32 "" l "look at" T "set lookAt \[value lookObject]\nputs \$lookAt\nset xX \"degrees(atan2(\$lookAt.translate.y-translate.y,sqrt(pow(\$lookAt.translate.x-translate.x,2)+pow(\$lookAt.translate.z-translate.z,2))))\"\nset yX \"\$lookAt.translate.z-this.translate.z >= 0 ? 180+degrees(atan2(\$lookAt.translate.x-translate.x,\$lookAt.translate.z-translate.z)):180+degrees(atan2(\$lookAt.translate.x-translate.x,\$lookAt.translate.z-translate.z))\"\nin this.rotate.x \{set_expression \$xX\}\nin this.rotate.y \{set_expression \$yX\}"
    addUserKnob node [stack 0] 1 lookObject l ""
    }


m == nuke.selectedNode()
knob = nuke.String_Knob("Name","Label", True)
knob2 = nuke.Boolean_Knob
m.addKnob(knob)
m.addKnob(knob2)
