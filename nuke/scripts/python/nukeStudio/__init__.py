#  Copyright (c) Chris Bankoff.  All Rights Reserved
#
# 
'This is the studio package extension designed for nuke'

#########################################################
#    INFO Panel
#########################################################

class infoPanel:

  def __init__(self):
    self.__infoText = ''
    return

  def append(self, appendText):
    self.__infoText += appendText
    return

  def displayInfo(self):
    'use a nuke message box to display the information'
    from nuke import message
    message(self.__infoText)
    return
  
  def printInfo(self):
    'print info to the standard out'
    print self.infoText
    return

#########################################################
#    locate_nukeStudio
#########################################################

def locate_nukeStudio():
  'returns the location of nukeStudio'
  
  import nukeStudio
  import os.path

  location = os.path.dirname(nukeStudio.__file__)

  return location 

#########################################################
#    END: Built-in function
#########################################################

nukeStudioInfo = infoPanel()
nukeStudioInfo.append("nukeStudio(__init__):\n-----------------------------------------\n")


#CONFIGURE: Set the static name of the studio
STUDIO_NAME = "Method"
nukeStudioInfo.append("STUDIO_NAME: " + STUDIO_NAME + '\n')

#Set variable with the package location
NUKESTUDIO_PACKAGE_LOCATION = locate_nukeStudio()
nukeStudioInfo.append("NUKESTUDIO_PACKAGE_LOCATION: " + NUKESTUDIO_PACKAGE_LOCATION + '\n')


#########################################################
#    Add Packages and Modules
#########################################################



#Sub-Packages
import defaults
import structure
import tools
import qube

#Modules
from helloWorld import *
from localUserSettings import *
from managePackage import *
from nuke import message


#########################################################
#    USER SETTINGS
#########################################################

userSettings = localUserSettings()
userSettings.fileLoad()


#########################################################
#    nukeStudio STRUCTURE
#########################################################

#FIX ME: need to properly tie into pipeline tools

#Job Name
JOB_NAME = userSettings.job

#Job Directory name
JOB_DIR = userSettings.job

#Job Number
JOB_NUMBER = ''

#User Name
USER_NAME = userSettings.user

#Sets TD privileges in nukeStudio
TD = userSettings.td

#Set Current Shot Number
#FIX ME: not fully supported
SHOT = ''

#init the the directory structure
structure.currentStructure.setJob(JOB_DIR)
structure.currentStructure.setUser(USER_NAME)


#Get the Location of the Job Custom from Structure
JOB_CUSTOM_PATH = structure.currentStructure.get('JOB_CUSTOM_PATH')

#Try to load Job Custom if fail then load default
from jobCustom import *

#DEBUGING: for the develment of the stock Job Defaults
JOB_CUSTOM_PATH = DEFAULT_JOB_LOC

#Load job custom
jobCustomInit(JOB_CUSTOM_PATH)


#########################################################
#    Addtional Plugings
#########################################################

#Add Menu System
nuke.pluginAddPath(NUKESTUDIO_PACKAGE_LOCATION + "/menu")

#Add gizmos
nuke.pluginAddPath(NUKESTUDIO_PACKAGE_LOCATION + "/gizmos")

#Add plugins
nuke.pluginAddPath(NUKESTUDIO_PACKAGE_LOCATION + "/plugins")


