# camera with aim for Nuke v0.1

import nuke

def aimcam():

	n = nuke.nodes.Camera2()

	p = 'set lookAt [value lookObject]\nputs $lookAt\nset xX "degrees(atan2($lookAt.translate.y-translate.y,sqrt(pow($lookAt.translate.x-translate.x,2)+pow($lookAt.translate.z-translate.z,2))))"\nset yX "$lookAt.translate.z-this.translate.z >= 0 ? 180+degrees(atan2($lookAt.translate.x-translate.x,$lookAt.translate.z-translate.z)):180+degrees(atan2($lookAt.translate.x-translate.x,$lookAt.translate.z-translate.z))"\nin this.rotate.x {set_expression $xX}\nin this.rotate.y {set_expression $yX}\n'

	tab = nuke.Tab_Knob("Look","Camera Aim")
	n.addKnob(tab)

	k = nuke.Script_Knob("knob", "look at")
	n.addKnob(k)
	n.knob("knob").setValue(p)
	k.setTooltip('Press this button after you type in the aim object\'s name')

	m = nuke.String_Knob("lookObject", "")
	n.addKnob(m)
	m.setTooltip('Type your aim object node name here')
