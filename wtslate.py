# writeDPX, slate and writeJPG generator v0.1 alpha
# 1. delete the old write_DPX, slate and write_Latest nodes
# 2. click on the last node in your comp (select it)
# 3. execute script

a="[file dirname [value root.name]]/../../images/comp/[file rootname [file tail [value root.name]]]/[file rootname [file tail [value root.name]]].%04d.dpx"
b="[file rootname [file tail [value root.name]]].[frame].dpx"
c="[file dirname [value root.name]]/../../images/comp/latest/[lindex [split [value root.name] /] 4]_comp_latest.%04d.jpg"

wdpx=nuke.nodes.Write(name="Write_DPX", file=a, colorspace="rec709")
wdpx['file_type'].setValue("dpx")
wdpx.setInput(0,nuke.selectedNode())

nuke.toNode('Write_DPX').knob('selected').setValue(True)

wslate = nuke.nodes.Text (name="Slate", font="/usr/share/fonts/bitstream-vera/Vera.ttf", yjustify = "center")
wslate['box'].setValue([0,0,2048,1168])
wslate['translate'].setValue([50, -550])
wslate['size'].setValue(25)
wslate['message'].setValue(b)
wslate.setInput(0,nuke.selectedNode())

nuke.toNode('Slate').knob('selected').setValue(True)

wjpg=nuke.nodes.Write (name="Write_Latest", file=c, colorspace="rec709")
wjpg['file_type'].setValue("jpg")
wjpg['_jpeg_quality'].setValue([1])
wjpg.setInput(0,nuke.selectedNode())
