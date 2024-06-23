# script that attaches a Furnace DeNoise node
# with default values to selected nodes,
# and then adds a write node to that Furnace DeNoise node

sn = nuke.selectedNodes()

for n in sn:

    first = n.firstFrame()
    last = n.lastFrame()
    a = n['file'].value()
    b = n['name'].value()
    c = a.replace('.%04d.dpx','_denoise.%04d.exr')
    d = "DeNoise_"+b

    dn=nuke.nodes.F_DeNoise (name=d, plateSize="Pal Or NTSC")
    dn.setInput(0,n)
    nuke.toNode(d).knob('selected').setValue(True)

    wr = nuke.nodes.Write (name="Write_EXR",file=c, colorspace="linear")
    wr['disable'].setExpression("((frame < "+b+".first)?true:false)||((frame >" +b+".last)?true:false)")
    wr.setInput(0,dn)

-------------

# script that attaches a Write node
# with default values to selected Read nodes,

sn = nuke.selectedNodes()

for n in sn:

    first = n.firstFrame()
    last = n.lastFrame()
    a = n['file'].value()
    b = n['name'].value()
    c = a.replace('.%04d.sgi','_crop.%04d.sgi')
    d = "DeNoise_"+b

    wr = nuke.nodes.Write (name="Write_EXR",file=c, colorspace="sRGB")
    wr['disable'].setExpression("((frame < "+b+".first)?true:false)||((frame >" +b+".last)?true:false)")
    wr.setInput(0,n)

-------------

#script that changes a single or multiple parameters in all selected nodes

sn = nuke.selectedNodes()
for n in sn:
        n.knob('channels').setValue("rgba")
        n.knob('colorspace').setValue("sRGB")

#script that adds a text node with the value from the input node.

sn = nuke.selectedNodes()

for i in sn:
	txt = nuke.nodes.Text (font ="/usr/share/fonts/bitstream-vera/Vera.ttf", message = "[file tail [knob input0.file]]", size = "35")
	txt.knob[transform.box].setValue(0,0,1920,1080)
	txt.setInput(0,i)
