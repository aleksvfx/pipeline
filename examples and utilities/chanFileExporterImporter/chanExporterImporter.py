#	The MIT License
#
#	Copyright (c) 2010 Dominic Drane
#
#	Permission is hereby granted, free of charge, to any person obtaining a copy
#	of this software and associated documentation files (the "Software"), to deal
#	in the Software without restriction, including without limitation the rights
#	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#	copies of the Software, and to permit persons to whom the Software is
#	furnished to do so, subject to the following conditions:
#
#	The above copyright notice and this permission notice shall be included in
#	all copies or substantial portions of the Software.
#
#	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#	THE SOFTWARE.
#
#	Chan File Importer/Exporter Maya Python plugin
#
#	Author: Dominic Drane
#		www.reality-debug.co.uk
#		dom@reality-debug.co.uk
#
#	Version .5a
#	Release date: 27/01/10
#
#	Usage: See readme.txt for details/limitations
#	

#todos
# add ui
# add update system framework
# close window after operation
# add menu for manual update?

		

#import locationMarker

import math, sys, string, os, urllib

import maya.OpenMaya as OpenMaya
import maya.OpenMayaMPx as OpenMayaMPx
import maya.OpenMayaAnim as OpenMayaAnim
from xml.dom import minidom

import locationMarker

import maya.cmds as cmds
import maya.mel as mel

kPluginTranslatorTypeName = "chan Export/Import"
kVersionNumber = "0.5a"

class customNodeTranslator(OpenMayaMPx.MPxFileTranslator):
	def __init__(self):
		OpenMayaMPx.MPxFileTranslator.__init__(self)
	def haveWriteMethod(self):
		return True
	def haveReadMethod(self):
		return True
	def filter(self):
		return "*.chan"
	def defaultExtension(self):
		return "chan"
	def writer( self, fileObject, optionString, accessMode ):
		try:
			fullName = fileObject.fullName()
			fileHandle = open(fullName,"w")

			selectList = OpenMaya.MSelectionList()
	
			OpenMaya.MGlobal.getActiveSelectionList(selectList)
			node = OpenMaya.MObject()
			depFn = OpenMaya.MFnDependencyNode()
			path = OpenMaya.MDagPath()
			iterator = OpenMaya.MItSelectionList(selectList)
			
			animationTime = OpenMayaAnim.MAnimControl()
			
			maxTime = int(animationTime.maxTime().value())
			minTime = int(animationTime.minTime().value())
			
			
			while (iterator.isDone() == 0):
				
				
				iterator.getDependNode(node)
				
				depFn.setObject(node)
				
				iterator.getDagPath(path, node)
				
				cameraObject = OpenMaya.MFnCamera(path)
				
				transform = OpenMaya.MFnTransform(path)
				
				chanMe = ChanFileExporter(transform, minTime, maxTime, cameraObject)
				
				for all in chanMe():
					fileHandle.write(all)
					
				iterator.next()
			
			fileHandle.close()
			
			
			

			
		except:
			sys.stderr.write( "Failed to write file information\n")
			raise
	
	def processLine( self, lineStr ):

		self.importTheChan.writeFrameData(lineStr)
		
	def reader(self, fileObject, optionString, accessMode):
		self.initialWindow()


		try:
			fullName = fileObject.fullName()
		
			self.fileHandle = open(fullName,"r")

		except:
			sys.stderr.write( "Failed to read file information\n")
			raise

		
		self.fileObject = fileObject
		self.optionString = optionString
		self.accessMode = accessMode
		
		cmds.select('pointCloud')
		cmds.select('camera1', add=True)
		cmds.group(n="cameraTrack")
		cmds.scale(10, 10, 10)

			
	def initialWindow(self, *args):
		rotationOrder = True
		self.window = cmds.window(title="Import .chan", menuBar=True, iconName='.chanFileImport', widthHeight=(335, 221), sizeable=False)
		
		
		
		cmds.menu( label='Help')
		cmds.menuItem(label='About', command=self.aboutDialog)
		cmds.menuItem(label='Check for updates', command=self.aboutDialog)
		cmds.menuItem(label='Help', command=self.openHelp)
		
		cmds.columnLayout(adjustableColumn=True)

		form = cmds.formLayout(numberOfDivisions=100)
		
		
		self.rotationOrderControl = cmds.optionMenuGrp(label='Rotation Order', columnWidth=(2, 80))
		cmds.menuItem(label='XYZ')
		cmds.menuItem(label='XZY')
		cmds.menuItem(label='YXZ')
		cmds.menuItem(label='YZX')
		cmds.menuItem(label='ZXY')
		cmds.menuItem(label='ZYX')
		
		windowSeparator = cmds.separator(width=334)
		
		self.cloudCheckbox = cmds.checkBox(label='Import point cloud data', onCommand=self.switchObjImportOn, offCommand=self.switchObjImportOff)
		
		self.fileLabel = cmds.text(label=".obj file")
		self.waveFileAddress = cmds.textField(fileName="", enable=False, width=230)
		self.browseForObj = cmds.symbolButton(image="navButtonBrowse.xpm", command=self.getObjFile, enable=False)
		
		
		importButton = cmds.button(label='Import', command=self.readFileIn, width=150, height=35)
		cancelButton = cmds.button( label='Cancel', command=self.closeWindow, width=150, height=35)
		
		
		cmds.formLayout(form, edit=True, attachForm=[\
		(self.rotationOrderControl, 'top', 18),\
		(self.rotationOrderControl, 'left', 50),\
		(self.cloudCheckbox, 'top', 70),\
		(self.cloudCheckbox, 'left', 90),\
		(self.fileLabel, 'top', 105),\
		(self.fileLabel, 'left', 10),\
		(self.waveFileAddress, 'top', 100),\
		(self.waveFileAddress, 'left', 70),\
		(self.browseForObj, 'top', 103),\
		(self.browseForObj, 'left', 300),\
		(importButton, 'left', 10),\
		(importButton, 'top', 140),\
		(cancelButton, 'right', 10),\
		(cancelButton, 'top', 140),\
		(windowSeparator, 'top', 50)])
		cmds.showWindow(self.window)
		
	def closeWindow(self, *args):
		cmds.deleteUI(self.window, window=True)
		
	def checkForUpdates(self, *args):
		#goGetUpdate.userQueriesUpdate()
		pass
		
	def aboutDialog(self, *args):
		cmds.confirmDialog(title="About", message=".chan File Exporter/Importer\n Dominic Drane (c) 2010")
		
	def openHelp(self, *args):
		cmds.launch(web="http://www.reality-debug.co.uk/wiki/index.php?title=.chan_File_Exporter")
	
	def getObjFile(self, *args):
		
		cmds.fileBrowserDialog( m=0, fc=self.importImage, ft='OBJ', an='Import', om='Import' )
	
	def importImage(self, fileName, fileType):
		cmds.textField(self.waveFileAddress, edit=True, text=fileName)
		return 1
		
	def getObjPath(self, *args):
		pass
		
	def switchObjImportOn(self, *args):
		cmds.symbolButton(self.browseForObj, edit=True, enable=True)
		cmds.textField(self.waveFileAddress, edit=True, enable=True)
		cmds.symbolButton(self.browseForObj, edit=True, enable=True)
		
	def switchObjImportOff(self, *args):
		cmds.symbolButton(self.browseForObj, edit=True, enable=False)
		cmds.textField(self.waveFileAddress, edit=True, enable=False)
		cmds.symbolButton(self.browseForObj, edit=True, enable=False)
		
	def readFileIn(self, *args):
		
		importCloudCheck = cmds.checkBox(self.cloudCheckbox, query=True, value=True)
		if(importCloudCheck == 1):
			
			finalObjPath = cmds.textField(self.waveFileAddress, query=True, text=True)
			pointCloud = readObjCloud(finalObjPath)
			
		chanRotationOrder = cmds.optionMenuGrp(self.rotationOrderControl, value=True, query=True)

		self.importTheChan = ChanFileImporter(chanRotationOrder)
		
		try:
			for line in self.fileHandle:
				self.processLine(line)
			self.fileHandle.close()
			
			self.closeWindow()
		
		except:
			sys.stderr.write( "Failed to read file information\n")
			raise

class readObjCloud():
	""" read in vertex data from nuke 3d camera track """
	def __init__(self, objPath):
		try:
			#fullName = fileObject.fullName()
		
			fileHandle = open(objPath,"r")
			
			for line in fileHandle:
				self.processLine(line)
			fileHandle.close()

		except:
			sys.stderr.write( "Failed to read file information\n")
			raise
			
		arrayOfPoints = cmds.select("locator*")
		
		
		cmds.group(n="pointCloud")
		cmds.xform(os=True, piv=(0,0,0))
		cmds.scale(0.01, 0.01, 0.01)
		cmds.delete(constructionHistory=True)
		

		
	def processLine(self, line):
		
		if(line[0] == "v"):
			
			brokenString = string.split(line)

			arrayOfPoints = cmds.spaceLocator(absolute=True, position=(\
			float(brokenString[1])*100\
			, float(brokenString[2])*100\
			, float(brokenString[3])*100))
			cmds.CenterPivot()
			cmds.scale(0.5, 0.5, 0.5)

class ChanFileImporter():
	""" module for importing chan files from nuke """
	def __init__(self, rotationOrder):
		
		self.rotationOrder = rotationOrder
		# create the camera
		self.cameraName = cmds.camera()
		self.cameraShape = self.cameraName[1]
		cmds.select(self.cameraShape)
		cmds.scale(0.5, 0.5, 0.5)
		
	
	def writeFrameData(self, lineStr):

		brokenString = string.split(lineStr, "\t")
		
		cmds.currentTime(int(brokenString[0]), edit=True, update=False)
		
		# transX
		cmds.setKeyframe(self.cameraName, attribute='translateX', v=float(brokenString[1]))

		# transY
		cmds.setKeyframe(self.cameraName, attribute='translateY', v=float(brokenString[2]))
		
		# transZ
		cmds.setKeyframe(self.cameraName, attribute='translateZ', v=float(brokenString[3]))
		
		
		if(self.rotationOrder == "XYZ"):
			# rotationX
			cmds.setKeyframe(self.cameraName, attribute='rotateX', v=float(brokenString[4]))
		
			# rotationY
			cmds.setKeyframe(self.cameraName, attribute='rotateY', v=float(brokenString[5]))
		
			# rotationZ
			cmds.setKeyframe(self.cameraName, attribute='rotateZ', v=float(brokenString[6]))
		
		
		elif(self.rotationOrder == "XZY"):
			
			# rotationX
			cmds.setKeyframe(self.cameraName, attribute='rotateX', v=float(brokenString[4]))
		
			# rotationZ
			cmds.setKeyframe(self.cameraName, attribute='rotateZ', v=float(brokenString[5]))
		
			# rotationY
			cmds.setKeyframe(self.cameraName, attribute='rotateY', v=float(brokenString[6]))

		
		elif(self.rotationOrder == "YXZ"):
			
			# rotationX
			cmds.setKeyframe(self.cameraName, attribute='rotateY', v=float(brokenString[4]))
		
			# rotationZ
			cmds.setKeyframe(self.cameraName, attribute='rotateX', v=float(brokenString[5]))
		
			# rotationY
			cmds.setKeyframe(self.cameraName, attribute='rotateZ', v=float(brokenString[6]))
			

		elif(self.rotationOrder == "YZX"):
			
			# rotationX
			cmds.setKeyframe(self.cameraName, attribute='rotateY', v=float(brokenString[4]))
		
			# rotationZ
			cmds.setKeyframe(self.cameraName, attribute='rotateZ', v=float(brokenString[5]))
		
			# rotationY
			cmds.setKeyframe(self.cameraName, attribute='rotateX', v=float(brokenString[6]))
			

		elif(self.rotationOrder == "ZXY"):
			
			# rotationX
			cmds.setKeyframe(self.cameraName, attribute='rotateZ', v=float(brokenString[4]))
		
			# rotationZ
			cmds.setKeyframe(self.cameraName, attribute='rotateX', v=float(brokenString[5]))
		
			# rotationY
			cmds.setKeyframe(self.cameraName, attribute='rotateY', v=float(brokenString[6]))


		elif(self.rotationOrder == "ZYX"):
			
			# rotationX
			cmds.setKeyframe(self.cameraName, attribute='rotateZ', v=float(brokenString[4]))
		
			# rotationZ
			cmds.setKeyframe(self.cameraName, attribute='rotateY', v=float(brokenString[5]))
		
			# rotationY
			cmds.setKeyframe(self.cameraName, attribute='rotateX', v=float(brokenString[6]))
		

		#focal. this was a cunt to do.
		cameraVerticalApp = cmds.getAttr((self.cameraShape + ".verticalFilmAperture"))
		focalFromChan = mel.eval("tand(" + brokenString[7] + "/2)")
		
		newFocalLength = cameraVerticalApp * 25.4 / (2 * focalFromChan)
		
		
		cmds.setKeyframe(self.cameraShape, attribute='focalLength', v=newFocalLength)
		

		
		
class ChanFileExporter():
	""" module for exporting chan files from application. arguments: object, startFrame, endFrame """

	fileExport = []

	def __init__(self, transform, startAnimation, endAnimation, cameraObj):

		mayaGlobal = OpenMaya.MGlobal()
		mayaGlobal.viewFrame(OpenMaya.MTime(1))

		for i in range(startAnimation, (endAnimation + 1)):

			focalLength = cameraObj.focalLength()
			
			vFilmApp = cameraObj.verticalFilmAperture()

			focalOut = 2* math.degrees(math.atan(vFilmApp * 25.4/ (2* focalLength)))

			myEuler = OpenMaya.MEulerRotation()
			spc = OpenMaya.MSpace.kWorld

			trans = transform.getTranslation(spc)

			rotation = transform.getRotation(myEuler)
			rotVector = OpenMaya.MVector(myEuler.asVector())

			self.fileExport.append((str(i) + '\t' + str(trans[0]) + "\t" + str(trans[1]) + "\t" + str(trans[2]) + "\t" + str(math.degrees(rotVector[0])) + "\t" + str(math.degrees(rotVector[1])) + "\t" + str(math.degrees(rotVector[2])) + "\t" + str(focalOut) + "\n"))

			mayaGlobal.viewFrame(OpenMaya.MTime(i))

	def __call__(self):
		return self.fileExport


	def radianToDegree(self, radians):
		outDegrees = 0.0
		outDegrees = (float(radians) / (math.pi))*180
		return outDegrees
		


class MayaUpdateGUI:
	"""main maya GUI class containing interface"""
	#class variables

	def __init__(self, latestVersionNumber, currentRunningVersion, productName, bugFixes, newFeatures, downloadURL):

		# set the preference file name 
		self.preferenceFileName = "mayaUpdatePrefs.prefs"

		#instanate download class system
		self.fileDownloader = updateDownloadSystem()

		# get the file name from the main URL
		self.base = downloadURL[downloadURL.rindex('/')+1:]

		#nicely done
		self.downloadURL = downloadURL

		# main string for scrollfield
		self.updateInformationString = ("Bug fixes:" + "\n\t" + bugFixes + "\n \nNew features:" + newFeatures)

		#main window
		self.updateInformationWindow = cmds.window(title="There is an update available", iconName='Update', widthHeight=(460, 335), sizeable=True)

		#layout stuffs
		cmds.columnLayout(adjustableColumn=True)

		form = cmds.formLayout(numberOfDivisions=100)



		informationPanel = cmds.scrollField(text=self.updateInformationString, height= 160, width=438, editable=False, wordWrap=True, font="boldLabelFont", insertionPosition=1)

		informationText = cmds.text(label=(productName + " " + latestVersionNumber + " is now available, you're currently running version " + currentRunningVersion + ", would you like to download it now?"), align='left', width=450)

		askAgainCheck = cmds.checkBox(label='Do not ask again.', onCommand=self.turnOnAutoUpdate, offCommand=self.turnOffAutoUpdate)

		updateButton = cmds.button(label='Update', command=self.callDownloadCommand, width=213, height=40)

		cancelButton = cmds.button(label='Not now', command=('cmds.deleteUI(\"' + self.updateInformationWindow + '\", window=True)'), width=213, height=40)

		cmds.formLayout(form, edit=True, attachForm=[(informationText, 'top', 17), (informationText, 'left', 15), (informationPanel, 'left', 12), (informationPanel, 'top', 57), (askAgainCheck, 'top', 222), (askAgainCheck, 'left', 173), (updateButton, 'top', 250), (updateButton, 'right', 15), (cancelButton, 'top', 250), (cancelButton, 'left', 15)])

		cmds.showWindow(self.updateInformationWindow)

	def callDownloadCommand(self, *args):


		self.fileSaveDialog()

		self.fileDownloader.geturl(self.downloadURL, self.downloadPath)

		cmds.deleteUI(self.updateInformationWindow, window=True)

	def fileSaveDialog(self, *args):

		#!!! look into this
		#there are some problems here and i'll fix it eventually, they are most likely only OSX based issues

		saveFileName = self.base.strip()

		# this maybe a workaround that I have yet to fully investigate
		#self.downloadPath = tkFileDialog.asksaveasfile(mode='w', initialfile=str(saveFileName))
		self.downloadPath = cmds.fileDialog(mode=1, defaultFileName=str(saveFileName))

	def turnOffAutoUpdate(self, *args):
		# turn off auto updating in the prefs
		defaultPrefContents = "autoUpdateCheck = FALSE"

		prefFile = open((self.currentLocation() + "/" + self.preferenceFileName),"w")
		prefFile.write(defaultPrefContents)
		prefFile.close()

	def turnOnAutoUpdate(self, *args):
		defaultPrefContents = "autoUpdateCheck = TRUE"

		prefFile = open((self.currentLocation() + "/" + self.preferenceFileName),"w")
		prefFile.write(defaultPrefContents)
		prefFile.close()

	def currentLocation(self):
		#yes its horrible, I know
		currentDir = os.path.dirname(locationMarker.__file__)
		return currentDir

class updateDownloadSystem():

	def reporthook(self, numblocks, blocksize, filesize, url=None):


		fileName = os.path.basename(url)

		percent = min((numblocks*blocksize*100)/filesize, 100)

		cmds.progressWindow(title='Downloading', progress=percent, status='Downloading: ', isInterruptable=True)

		if(percent == 100):
			cmds.progressWindow(endProgress=True)

		else:
			cmds.progressWindow(edit=True, title='Downloaded', progress=percent, status=('Downloading: ' + str(percent) + '%' ))

	def geturl(self, url, dst):

		urllib.urlretrieve(url, dst,
							lambda nb, bs, fs, url=url: self.reporthook(nb,bs,fs,url))

class MayaToolAutoUpdater:

	""" checks online for updated version from XML file """

	currentRunningVersion = ""
	feedPath = ""
	latestVersionNumber = ""
	bugFixList = ""
	newFeaturesList = ""
	newFileURL = ""
	productName = ""

	def __init__(self, productName, currentVersion, xmlPath):

		self.setRunningVersion(currentVersion)

		self.setProductName(productName)

		self.setXMLPath(xmlPath)

		if(self.checkPreferenceFile()):

			self.getUpdateInformation()

		else:
			pass



	def checkPreferenceFile(self, *args):
		preferenceFileName = "mayaUpdatePrefs.prefs"

		if(os.path.exists(self.currentLocation() + "/" + preferenceFileName)):
			readPrefs = open((self.currentLocation() + "/" + preferenceFileName),"r")
			updatePreference = readPrefs.read()
			readPrefs.close()

			if(updatePreference.count("autoUpdateCheck = TRUE")):
				updateStatus = True
			else:
				updateStatus = False


		else:

			defaultPrefContents = "autoUpdateCheck = TRUE"

			prefFile = open((self.currentLocation() + "/" + preferenceFileName),"w")
			prefFile.write(defaultPrefContents)
			prefFile.close()

		return updateStatus

	def userQueriesUpdate(self):
		if (str(self.currentRunningVersion.strip()) != str(self.latestVersionNumber.strip())):
			self.mayaGUI = MayaUpdateGUI(self.latestVersionNumber, self.productName, self.bugFixList, self.newFeaturesList, self.newFileURL)
		else:
			cmds.confirmDialog(title="wah wah wah", message="there is no update available.",button="*sighs*")

	def setRunningVersion(self, runningVersion):
		""" sets the current running version of the script """
		self.currentRunningVersion = runningVersion

	def setXMLPath(self, xmlPath):
		""" set the path to the update information xml file """
		self.feedPath = xmlPath

	def setProductName(self, productNameIn):
		self.productName = productNameIn

	def getUpdateInformation(self):

		try:

			self.updateFeed = urllib.urlopen(self.feedPath)

			self.parseFeed()

		except IOError:

			sys.stderr.write("IO Error: Cannot to connect to 'tinternet.")

	def currentLocation(self):
		currentDir = os.path.dirname(locationMarker.__file__)
		return currentDir

	def parseFeed(self):
		""" parses the XML feed """

		domObj = minidom.parseString(self.updateFeed.read())
		versionNumberResult = domObj.getElementsByTagName("versionNumber")
		bitref = versionNumberResult[0]
		self.latestVersionNumber = bitref.childNodes[0].nodeValue

		versionNumberResult = domObj.getElementsByTagName("bugFixes")
		bitref = versionNumberResult[0]
		self.bugFixList = bitref.childNodes[0].nodeValue

		versionNumberResult = domObj.getElementsByTagName("newFeatures")
		bitref = versionNumberResult[0]
		self.newFeaturesList = bitref.childNodes[0].nodeValue

		versionNumberResult = domObj.getElementsByTagName("fileURL")
		bitref = versionNumberResult[0]
		self.newFileURL = bitref.childNodes[0].nodeValue

		if (str(self.currentRunningVersion.strip()) != str(self.latestVersionNumber.strip())):
			self.mayaGUI = MayaUpdateGUI(str(self.latestVersionNumber.strip()), str(self.currentRunningVersion.strip()), str(self.productName.strip()), str(self.bugFixList.strip()), str(self.newFeaturesList), self.newFileURL)



# creator
def translatorCreator():
	return OpenMayaMPx.asMPxPtr( customNodeTranslator() )

# initialize the script plug-in
def initializePlugin(mobject):
	mplugin = OpenMayaMPx.MFnPlugin(mobject)
	goGetUpdate = MayaToolAutoUpdater(".chan File Exporter/Importer", "0.5","http://update.reality-debug.co.uk/chanFileExtractor.xml")
	try:
		mplugin.registerFileTranslator(kPluginTranslatorTypeName, None, translatorCreator)
	except:
		sys.stderr.write( "Failed to register translator: %s" % kPluginTranslatorTypeName )
		raise

# uninitialize the script plug-in
def uninitializePlugin(mobject):
	mplugin = OpenMayaMPx.MFnPlugin(mobject)
	try:
		mplugin.deregisterFileTranslator( kPluginTranslatorTypeName )
	except:
		sys.stderr.write( "Failed to deregister translator: %s" % kPluginTranslatorTypeName )
		raise
	
