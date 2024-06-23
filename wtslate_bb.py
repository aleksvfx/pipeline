# writeDPX, slate and writeJPG generator v0.1 alpha
# 0. Make a folder for your version write node (the automatic folder creation still WIP)
# 1. delete the old write_DPX, slate and write_Latest nodes
# 2. click on the last node in your comp (select it)
# 3. execute script

a="[file dirname [value root.name]]/../../images/comp/[file rootname [file tail [value root.name]]]/[file rootname [file tail [value root.name]]].%04d.dpx"
b="[file rootname [file tail [value root.name]]].[frame].dpx"
c="[file dirname [value root.name]]/../../images/comp/latest/[lindex [split [value root.name] /] 4]_comp_latest.%04d.jpg"

nuke.nodes.Write(
name="Write_DPX",
file=a,
colorspace="rec709"
).setInput(0,nuke.selectedNode())

nuke.toNode('Write_DPX').knob('selected').setValue(True)

tn = nuke.nodes.Text (name="Slate", font="/usr/share/fonts/bitstream-vera/Vera.ttf", yjustify = "center")
tn['box'].setValue([0,0,2048,1168])
tn['translate'].setValue([50, -550])
tn['size'].setValue(25)
tn['message'].setValue(b)
tn.setInput(0,nuke.selectedNode())

nuke.toNode('Slate').knob('selected').setValue(True)

nuke.nodes.Write (
name="Write_Latest",
file=c,
colorspace="rec709"
).setInput(0,nuke.selectedNode())
