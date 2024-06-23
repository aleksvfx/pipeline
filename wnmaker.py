# script that attaches a Write node with default values to selected Read nodes
# and sticks an expression that reads first and last frames from the read notes

import nuke
import os

sn = nuke.selectedNodes()

# build panel
p = nuke.Panel("IMAGE SEQUENCE WRITE NODE MAKER ")

p.addEnumerationPulldown('Channels:','all none rgb rgba alpha depth motion forward backward mask')
p.addEnumerationPulldown('Colorspace:', 'default linear sRGB rec709 Cineon Gamma1.8 Gamma2.2 Panalog REDlog ViperLog REDSpace')
p.addEnumerationPulldown('File format:', 'cin dpx exr hdr jpg pic png sgi tga tif xpm yuv')

# show panel
p.show()

# pick up values from panel
chanVal = p.value('Channels:')
cspace = p.value('Colorspace:')
fmt = p.value("File format:")

# function that creates a write node with previously defined value
def wnmake():

	for n in sn:

	    first = n.firstFrame()
	    last = n.lastFrame()
		nodeName = n['name'].value()
	    fileName = n['file'].value()
	    (shortname, extension) = os.path.splitext(n['file'].value())  
	    fileNameNew = fileName.replace(extension,"."+fmt)
		writeThis = fileNameNew

	    wr = nuke.nodes.Write(name="Write_"+fmt,file=writeThis, colorspace=cspace)
		wr['file_type'].setValue(fmt)

		if fmt == 'jpg':
			wr['_jpeg_quality'].setValue([1])

	    wr['disable'].setExpression("((frame < "+nodeName+".first)?true:false)||((frame >" +nodeName+".last)?true:false)")
		wr.knob('channels').setValue(chanVal)
	    wr.setInput(0,n)
