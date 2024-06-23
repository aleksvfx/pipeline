# writeDPX, slate and writeJPG generator v0.1 alpha
# 1. delete the old write_DPX, slate and write_Latest nodes
# 2. click on the last node in your comp (select it)
# 3. execute script

import os
import nuke
import string
import sys

this_filename = sys._getframe().f_code.co_filename
print('Loading '+this_filename)

def jobForCurrentFile():
	curfile=nuke.knob('root.name')
	if curfile != '':
		parts=curfile.split('/')
		return parts[2]

def shotForCurrentFile():
	curfile=nuke.knob('root.name')
	if curfile != '':
		parts=curfile.split('/')
		return parts[4]

def rootNameOfCurrentFile():
	curfile=nuke.knob('root.name')
	if curfile != '':
		parts=curfile.split('/')
		length=len(parts)
		tmp=parts[length-1]
		root=tmp.split('.')
		return root[0]

def wtslate():

	imageresolution = 'misc 2ksq 1ksq 512sq 4kfa 2kfa 1kfa tvfa 2kafa 2kan 4kufa 2kufa tvufa 2k3p hd hhd tvhd'
	dpximagecolorspace = 'lg10 lg16 lnf ln16 vdf8 vdf16 vdb8 vdb16 vd8 vd16 nc8 nc16 ncf dt8'
	jpgimagecolorspace = 'lg10 lg16 lnf ln16 vdf8 vdf16 vdb8 vdb16 vd8 vd16 nc8 nc16 ncf dt8'
	imgres = ""
	dpximgcspace = ""
	jpgimgcspace = ""
	dpxenumerator = ""
	jpgenumerator = ""

	root=rootNameOfCurrentFile()
	job=jobForCurrentFile()
	shot=shotForCurrentFile()

	if root == None:
		raise RuntimeError(this_filename+"\nwtslate() : Save your nuke script first!")
	if job == None:
		raise RuntimeError(this_filename+"\nwtslate() : can't figure out job!")
	if shot == None:
		raise RuntimeError(this_filename+"\nwtslate() : can't figure out shot!")

#	comp_dir='/jobs/'+job+'/shots/'+shot+'/images/comp/'+root
#	if not os.path.exists(comp_dir):
#		os.mkdir(comp_dir)

	o = nuke.Panel("write nodes resolution & color space enumerator")
	o.addEnumerationPulldown('resolution:',imageresolution)
	o.addEnumerationPulldown('DPX writer color space:',dpximagecolorspace)
	o.addEnumerationPulldown('JPG writer color space:',jpgimagecolorspace)
	o.show()

	imgres = str(o.value("resolution:"))
	dpximgcspace = str(o.value("DPX writer color space:"))
	jpgimgcspace = str(o.value("JPG writer color space:"))
	dpxenumerator = imgres + "_"+ dpximgcspace
	jpgenumerator = imgres + "_"+ jpgimgcspace

	a="[file dirname [value root.name]]/../../images/comp/[file rootname [file tail [value root.name]]]/" + dpxenumerator + "_dpx/[file rootname [file tail [value root.name]]]_" + dpxenumerator + ".%04d.dpx"

#	b="[file rootname [file tail [value root.name]]].[frame].dpx"

	c="[file dirname [value root.name]]/../../images/comp/[file rootname [file tail [value root.name]]]/" + jpgenumerator + "_jpg/[file rootname [file tail [value root.name]]]_" + jpgenumerator + ".%04d.jpg"

	nuke.nodes.Write(name="Write_DPX",file=a,colorspace="default").setInput(0,nuke.selectedNode())
	nuke.toNode('Write_DPX').knob('selected').setValue(True)

#	tn = nuke.nodes.Text (name="Slate", font="/usr/share/fonts/bitstream-vera/Vera.ttf", yjustify = "center")
#	tn['box'].setValue([0,0,2048,1168])
#	tn['translate'].setValue([50, -550])
#	tn['size'].setValue(25)
#	tn['message'].setValue(b)
#	tn.setInput(0,nuke.selectedNode())

	nuke.nodes.Write(name="Write_JPG",file=c,colorspace="sRGB").setInput(0,nuke.selectedNode())

#	nuke.toNode('Write_JPG').knob('file_type').setValue("jpeg")
#	nuke.toNode('Write_Latest').knob('_jpeg_quality').setValue("1")

	nuke.toNode('Write_DPX').knob('selected').setValue(True)
