# get frame range from read node (you must have only one)
# and stick into the script settings

import nuke

def setScriptRange():

#	n = nuke.allNodes()
	n = nuke.selectedNodes()

	for i in n:
	    if i.Class() == "Read":
	        i.knob("selected").setValue(True)

	first = nuke.selectedNode().knob('first').value()
	last = nuke.selectedNode().knob('last').value()

	nuke.message("Your new frame range is "+str(first)+" to "+str(last))

	nuke.Root().knob('first_frame').setValue(first)
	nuke.Root().knob('last_frame').setValue(last)
#	nuke.Root().knob('fps').setValue(25)
