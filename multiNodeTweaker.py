# multi node tweaker v0.1 by Aleksandar Djordjevic
#
# this script creates a panel that enables the user to manipulate
# several knobs inside all selected read and write nodes
# you can select all nodes but it is going to change values only on reads and writes\

import nuke
import os

def multiNodeTweaker():

	test = 0
	origFileName = None
	replaceInFileName = None
	booleanCheckBox = None
	chanVal = 'rgb rgba alpha depth'
	cspace = 'default linear sRGB rec709 Cineon Gamma1.8 Gamma2.2 Panalog REDlog ViperLog REDSpace'
	sn = nuke.selectedNodes()

# first checkpoint - is anything selected?
	if (len(sn) == 0):
		nuke.message("Select one or more Read or Write nodes")
		return

# second checkpoint - I will work only on valid node classes
	for i in sn:
		if i.Class() != 'Read' or 'Write':
			nuke.message("No Read or Write nodes selected.")
			return

	o = nuke.Panel("Multi Node Tweaker")
	o.addSingleLineInput('Find:', origFileName)
	o.addSingleLineInput('Replace:', replaceInFileName)
	o.addEnumerationPulldown('Color Space',cspace)
	o.addButton("Cancel")
	o.addButton("Ok")

# If selected nodes are of Write class, add parameter to mess with the channels
	for i in sn:
	    if i.Class() == 'Write':
	        test = 1
	if test == 1:
	    o.addEnumerationPulldown('Channels:',chanVal)

	o.show()

# grab new values
	origFileName = o.value("Find:")
	replaceInFileName = o.value("Replace:")
	cspace = o.value("Color Space")
	chanVal = o.value("Channels:")

	for n in sn:
	    filename = n['file'].value()
	    newFileName = filename.replace(origFileName,replaceInFileName)
	    n.knob('file').setValue(newFileName)
	    n.knob('colorspace').setValue(cspace)
	    if n.Class() == 'Write':
	            n.knob('channels').setValue(chanVal)
