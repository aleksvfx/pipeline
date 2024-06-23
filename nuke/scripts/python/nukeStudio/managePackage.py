#  Copyright (c) Chris Bankoff.  All Rights Reserved
#
# 
import os
import nuke
import time

class managePackage:
  'Tools used to manage package development within a Nuke Studio Custom'
  
  def __init__ (self, NUKE_STUDIO_CUSTOM = ''):
    if NUKE_STUDIO_CUSTOM == '':
      NUKE_STUDIO_CUSTOM = '/S2/3d/farm/nuke/studio_custom'
    
    self.__DEFAULT_LOCATION = NUKE_STUDIO_CUSTOM
    self.__ACHIVE_LOCATION = self.__DEFAULT_LOCATION + "/archive-packages/"
    self.__DISTRIBUTE_LOCATION =  self.__DEFAULT_LOCATION + "/studio-packages/"
    self.__BETA_LOCATION = self.__DEFAULT_LOCATION + '/beta-packages/'
    
    self.packageLoc = ''
    self.packageName = ''
    self.__iniName = 'packageInfo.ini'
    self.__iniFile = ''
    
    self.user = 'userName'
    self.version = '0.0.0'
    self.build = 0
    self.date = ''
    self.comments = ''
  
  def debugInfo(self):
    print 'packageLoc: ' + str(self.packageLoc)
    print 'packageName: ' + str(self.packageName)
    print '__iniName: ' + str(self.__iniName)
    print '__iniFile: ' + str(self.__iniFile)
    
    print 'user: ' + str(self.user)
    print 'version: ' + str(self.version)
    print 'build: ' + str(self.build)
    print 'date: ' + str(self.date)
    print 'comments: ' + str(self.comments)
    return

  def __setTime(self):
    'sets the time'
    import time
    self.date = time.asctime()
    return

  def __findPackageName(self, location):
    'grabs the package name from a location string'
    dirList = location.split('/')
    while True:
      dirName = dirList.pop()
      #if not empty
      if dirName != '': break
    
    #remove .### tails from the file name
    if dirName.count('.') != 0:
      dirName = dirName.split('.')[0]
    
    return dirName

  def __verifyPackage (self, testLoc):
    'test to see if the package is a module and returns a boolean'
    import os
    
    #Looks for an init file to varify that it is a modual 
    initFile = testLoc + '__init__.py'

    #returns a boolean if it exists
    return os.path.exists(initFile)
  
  ### INI Files ###
  def __iniExists(self):
    'boolen checks to see if the ini file exists 1 if it exists and 0 if it does not exists'
    import os.path as path
    return path.exists(self.__iniFile)
  
  def __iniSet(self, file=''):
    'sets the location of the ini file if empty sets to the default location'
    if file == '':
      self.__iniFile = self.packageLoc + self.__iniName
    else:
      self.__iniFile = file
    
    if self.__iniExists():
      self.__iniLoad()

    return

  def __iniLoad(self):
    'load the package info ini file'
    import ConfigParser
    import os
    
    #creates the config object
    config = ConfigParser.ConfigParser()
    
    #load data from file if it already exists
    if self.__iniExists():
      try:
        config.read(self.__iniFile)
      except:
        print "ERROR(read): " + self.__iniFile
    
    if config.has_section(self.packageName) == True:
      self.version = config.get(self.packageName, 'version')
      self.build = config.get(self.packageName, 'build')
      self.date = config.get(self.packageName, 'date')
      self.user = config.get(self.packageName, 'user')
      self.comments = config.get(self.packageName, 'comments')
    return
  
  def __iniSave(self):
    'save the package info ini file'
    import ConfigParser
    import os
    
    #creates the config object
    config = ConfigParser.ConfigParser()
    
    #load data from file if it already exists
    if self.__iniExists():
      #print 'File Exists'
      try:
        config.read(self.__iniFile)
      except:
        print "ERROR(read): " + self.__iniFile
    
    #update the config information
    if config.has_section(self.packageName) == False:
      config.add_section(self.packageName)
    config.set(self.packageName, "version", self.version)
    config.set(self.packageName, "build", self.build)
    config.set(self.packageName, "user", self.user)
    config.set(self.packageName, "date", self.date)
    config.set(self.packageName, "comments", self.comments) 
    
    #output file
    fd = open(self.__iniFile, 'w')
    config.write(fd)
    fd.close()
    return
    
    
  ### INI Files ###
  
  def __disPanel(self):
    'opens a nuke panel and alows the editing of package info'
    import nuke
  
    #Create Panel Options
    p = nuke.Panel(self.packageName)
    p.addSingleLineInput("Version:",self.version)
    p.addSingleLineInput("Build:",self.build)
    p.addNotepad("Comments:",self.comments)
    p.addSingleLineInput("User:",self.user)
    
    
    p.addButton("Cancel") # returns 0
    p.addButton("Ok") # returns 1
    p.setWidth(250)
  
    #Show Panelself.__iniFile
    try: 
      result = p.show()
    except:
      result = False
    
    #Crashes if value() is called and the panel was canceled 
    if result:
      self.version = p.value("Version:")
      self.build = p.value("Build:")
      self.user = p.value("User:")
      self.comments = p.value("Comments:")
      self.__setTime()
    return result
  
  def __backupPackage(self, packageLocation = ''):
    'creates or overwrites a backup of a package'
    import shutil
    import os
    
    if packageLocation == '':
        packageLocation = self.packageLoc
    
    #if self.__verifyPackage(packageLocation) == 0:
      #print "ERROR: Not a package" 
      #return 
    
    dirList = packageLocation.split('/')
    while True:
      dirName = dirList.pop()
      #if not empty
      if dirName != '': break
    
    backupName = '~.'+ dirName
    
    path = ''
    for d in dirList:
      path +=  str(d) +"/"
  
    packageBackupLocation = path + backupName
    
    #print packageBackupLocation
    if os.path.exists(packageBackupLocation):
      shutil.rmtree(packageBackupLocation)
    
    shutil.copytree(packageLocation, packageBackupLocation)
    
    return

  def setPackage (self, packageLocation):
    'sets the package location returns a boolean; 0 if set and 1 if error setting'
    if self.__verifyPackage(packageLocation):
      self.packageLoc = packageLocation
      self.packageName = self.__findPackageName(packageLocation)
      self.__iniSet()
      return 0
    else:
      return 1
    return

  def selectPackage (self, packageLocation=''):
    'promt to select a package location'
    import nuke
    
    if packageLocation == '':
      packageLocation = self.__BETA_LOCATION
    
    while True:
      #Prompt User to select a Package
      rawInput = nuke.getFilename('Select Package', '/', packageLocation)
      
      if rawInput == None: 
        return 1
      if self.setPackage(rawInput) == 0:
        break
    return

  def updatePackageInfo(self, buildIncr = 1, verionIncr = 0, releaseIncr = 0):
    'update the package info file'
    #if self.promtSelect() == 1 : return
    
    #Load from File
    #self.iniLoad()
    
    
    verID = self.version
    rls = verID.split('.')[0]
    ver = verID.split('.')[1]
    sbv = verID.split('.')[2]
    
    rls = int(rls)
    ver = int(ver)
    sbv = int(sbv)
    
    #incr release number
    rls = eval('rls+ releaseIncr')
    
    #if incr relealse then 0 out version number
    if releaseIncr != 0:
      ver= 0
    else: 
      ver = eval('ver+ verionIncr')
    
    #if incr version then 0 out subVersion number
    if verionIncr != 0:
      sbv= 0
    else:
      sbv = eval('sbv+ buildIncr')

    #create new version Id
    self.version = str(rls) + '.' + str(ver) +'.' + str(sbv) 
    
    #incr build number
    self.build =int(self.build)
    self.build = eval('self.build + buildIncr')
    if self.__disPanel() == 0 : return
    
    #Save the Changes the INI file
    self.__iniSave()
    return

  def archivePackage(self, archiveLocation = ''):
    'create an achive of the current package'
    import shutil 
    
    if archiveLocation == '':
      archiveLocation = self.__ACHIVE_LOCATION

    #Set up Archive Package Name and Desination
    archivePackageName = self.packageName + "." + '%05d' % int(self.build)
    destLoc = archiveLocation + archivePackageName
    
    #Controled Copy of the Package
    try:
      shutil.copytree(self.packageLoc, destLoc)
    except:
      raise NameError, 'ERROR(copytree): Package Exists'
    return

  def distributePackage(self, distributedLocation = '' ):
    'distibute the current package to the live location'
    import shutil
    import os
    
    #set the distribute source
    distributeSrc = self.packageLoc
    
    #set the destination location
    if distributedLocation == '':
        distributeDst = self.__DISTRIBUTE_LOCATION + self.packageName + "/"
    else:
        distributeDst = distributedLocation
    
    print "distributeSrc: " + distributeSrc
    print "distributeDst: " + distributeDst
     
    #create a temp backup of the current package
    if os.path.exists(distributeDst):
      self.__backupPackage(distributeDst)
      print "remove: " + distributeDst
      shutil.rmtree(distributeDst)
    
    try:
      shutil.copytree(distributeSrc, distributeDst)
    except:
      raise NameError, 'ERROR(copytree): Package Exists'
    return


  def distributeVersion(self, packageLocation = '', distributedLocation = ''): 
    'Distribute New Version of a package'

    #set the distribute source promt the user to select if not alreay set or given
    
    if packageLocation == '':
      if self.packageLoc == '':
        self.selectPackage()
    else:
      if self.packageLoc == '':
        self.setPackage(packageLocation)
    
    
    #set the destination location
    if distributedLocation == '':
        distributeDst = self.__DISTRIBUTE_LOCATION + self.packageName + "/"
    else:
        distributeDst = distributedLocation

    self.updatePackageInfo(1, 1)
    #self.debugInfo()
    self.archivePackage()
    self.distributePackage(distributeDst)
    
    return

  def archiveBuild(self, packageLocation = ''): 
    'Distribute New Version of a package'

    #set the distribute source promt the user to select if not alreay set or given
    if packageLocation == '':
      if self.packageLoc == '':
        self.selectPackage()
    else:
      if self.packageLoc == '':
        self.setPackage(packageLocation)

    self.updatePackageInfo(1)
    self.archivePackage()
    
    return
  
  def archiveVersion(self, packageLocation = ''): 
    'Distribute New Version of a package'

    #set the distribute source promt the user to select if not alreay set or given
    if packageLocation == '':
      if self.packageLoc == '':
        self.selectPackage()
    else:
      if self.packageLoc == '':
        self.setPackage(packageLocation)

    self.updatePackageInfo(1, 1)
    self.archivePackage()
    
    return

  def archiveRelease(self, packageLocation = ''): 
    'Distribute New Version of a package'

    #set the distribute source promt the user to select if not alreay set or given
    if packageLocation == '':
      if self.packageLoc == '':
        self.selectPackage()
    else:
      if self.packageLoc == '':
        self.setPackage(packageLocation)

    self.updatePackageInfo(1,1,1)
    self.archivePackage()
    
    return





