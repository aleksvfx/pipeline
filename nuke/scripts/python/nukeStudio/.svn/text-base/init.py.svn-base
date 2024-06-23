#  Copyright (c) Chris Bankoff.  All Rights Reserved
#
#    init.py
#Nuke Stuido Custom Extention for nuke
#Need to properly add plugins to the nukes enviroment

import sys
import os.path
#import os.uname
import platform
import nuke


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
#    INFO Panel   <----------------------END
#########################################################

#########################################################
#    Dev Enviroment Settings
#########################################################

class devSettings:
  'Controls the development Enviroment'
  def __init__(self, file = os.environ['HOME'] + '/.nuke/DevEnvironment.ini'):
    self.__iniFile = file
    self.__iniSection = 'DevEnviroment'
    self.load = False
    self.fileLoad()
    #FIX ME:  Make this smart and work on Macsys.path.append(STUDIO_PACKAGES)
    #if platform.system() != 'Linux':
      #self.load = 'True'
  
  def fileExists(self):
    'boolen checks to see if the ini file exists'
    import os.path as path
    return path.exists(self.__iniFile)

  def fileSave(self):
    'save the localUserSettings to an ini file'
    import ConfigParser
    import os
    
    #creates the config object
    config = ConfigParser.ConfigParser()
    config.add_section(self.__iniSection)
    config.set(self.__iniSection, "load", self.load)
    
    #output file
    fd = open(self.__iniFile, 'w')
    config.write(fd)
    fd.close()
    return

  def fileLoad(self):
    'load from an ini file'
    import ConfigParser
    import os
    
    #creates the config object
    config = ConfigParser.ConfigParser()
    try:
        config.read(self.__iniFile)
        self.load = config.get(self.__iniSection, 'load')
    except:
        self.load = False
    return
    
  def disPanel(self):
    'opens a nuke panel and alows the editing'
    import nuke
    
    booleanOpts = 'False True'
  
    #Create Panel Options
    p = nuke.Panel("Development environment:")
    p.addEnumerationPulldown("LOAD:",booleanOpts)
    
    #add buttons and window size
    p.addButton("Cancel") # returns 0
    p.addButton("Update") # returns 1
    p.setWidth(250)
  
    #Show Panelself.__iniFile
    try: 
      result = p.show()
    except:
      result = False
    
    #Crashes if value() is called and the panel was canceled 
    if result:
      self.load = p.value("LOAD:")
    return

  def modifyPrompt(self):
    'Prompt the settings panel and save to a file'
    self.disPanel()
    self.fileSave()
    return


#########################################################
#    Dev Enviroment Settings    <--------------------END
#########################################################


studioCustomInfo = infoPanel()
studioCustomInfo.append("Nuke Dev Message\n-----------------------------------------\n")
studioCustomInfo.append("Current OS: " + platform.system() + "\n")
studioCustomInfo.append("  system: "+ str(os.uname()[2]) + "\n")
studioCustomInfo.append("  release: "+ str(os.uname()[4]) + "\n")
studioCustomInfo.append("  init:   " + NUKE_STUDIO_CUSTOM + "\n")


#Create a Dev Setting Class to control
#if Dev or live studio packages are loaded
Dev = devSettings()

#Used for Publishing SVN Packages and Beta Development
beta_dir = 'beta-package'
__temp_dir_list = NUKE_STUDIO_CUSTOM.split('/')
__temp_dir_list[-2] = beta_dir
NUKESTUDIO_PACKAGE_BETA = '/'.join(__temp_dir_list)

live_dir = 'site-package'
__temp_dir_list = NUKE_STUDIO_CUSTOM.split('/')
__temp_dir_list[-2] = live_dir
NUKESTUDIO_PACKAGE_LIVE =  '/'.join(__temp_dir_list)


#if in development mode then append .beta 
if Dev.load == 'True':

  studioCustomInfo.append("BETA---BETA---BETA---BETA---BETA---BETA---BETA---BETA\n")
  studioCustomInfo.append("With SVN\n")

  #modify path looking for beta package
  #beta_dir = 'beta-package'
  #__temp_dir_list = NUKE_STUDIO_CUSTOM.split('/')
  #__temp_dir_list[-2] = beta_dir
  #__NUKE_STUDIO_CUSTOM_BETA =  '/'.join(__temp_dir_list)
  #studioCustomInfo.append("Beta Custom:   " + __NUKE_STUDIO_CUSTOM_BETA + "\n")
  studioCustomInfo.append("Beta Custom:   " + NUKESTUDIO_PACKAGE_BETA + "\n")

  #Test see if the beta-package exists
  if os.path.exists(NUKESTUDIO_PACKAGE_BETA):
      NUKE_STUDIO_CUSTOM = NUKESTUDIO_PACKAGE_BETA
  else:
      studioCustomInfo.append("Beta-Package not found\n")
      
  #studioCustomInfo.displayInfo()
  

#set the location of the studio site-package repository
STUDIO_PACKAGES = '/'.join(NUKE_STUDIO_CUSTOM.split('/')[0:-1])
studioCustomInfo.append("  STUDIO_PACKAGES:   " + STUDIO_PACKAGES + "\n")
studioCustomInfo.append("  NUKE_STUDIO_CUSTOM:   " + NUKE_STUDIO_CUSTOM + "\n")

#Add the site-package to the system path
try:
  sys.path.append(STUDIO_PACKAGES)
  import nukeStudio
except:
  #studioCustomInfo.append("\n\nERROR---ERROR---ERROR---ERROR---ERROR---ERROR---ERROR\n")
  studioCustomInfo.append('\n\nERROR(import): nukeStudio\n\nIf the problem persists, please contact nukesupport@methodstudios.com.')
  #studioCustomInfo.displayInfo()


#########################################################
#    Addtional Plugings
#########################################################

#Add Menu System
#nuke.pluginAddPath("./menu")

#Add gizmos
#nuke.pluginAddPath("./gizmos")

#Add plugins
#nuke.pluginAddPath("./plugins")


