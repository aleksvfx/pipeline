#  Copyright (c) Chris Bankoff.  All Rights Reserved
#
# 
import os

class localUserSettings:
  'User Local Settings'
  
  def __init__(self, file = ''):
    'sets up the default values for the localUserSettings'
    import os
    
    if file == '':
      self.__iniFile = os.environ['HOME'] + '/.nuke/localUserSettings.ini'
    else:
      self.__iniFile = file
    
    self.__iniSection = 'localUserSettings'
    #self.iniFile = file
    self.user = os.environ ["USER"]
    self.job = 'jobName'
    self.td = False
    #self.dev = False

  def disPanel(self):
    'opens a nuke panel and alows the editing of localUserSettings'
    import nuke
    
    try:
      from nukeStudio.structure import jobs
      jobList = jobs()
    except:
      jobList = []

    try:
      from nukeStudio.structure import users
      userList = users()
    except:
      userList = []

    
    #Build Options
    booleanOptsT = 'True False'
    booleanOptsF = 'False True'

    #Make the list into a string with spaces
    if self.job == '':
      jobOpts = ' defaultJob'
    else: 
      jobOpts = self.job + " defaultJob "
    for j in jobList:
      if j != self.job:
        jobOpts += j +" "

    
    #Make the list into a string with spaces
    if self.user == '':
      userOpts = 'defaultUser '
    else: 
      userOpts = self.user + " defaultUser "
    for j in userList:
      if j != self.user:
        userOpts += j +" "
    
    
    #Create Panel Options
    p = nuke.Panel("User Local Settings:")
    
    #If no list of users then enter name else use drop down choice
    if userOpts == '': 
        p.addSingleLineInput("USER:", self.user)
    else: 
        
        p.addEnumerationPulldown("USER:",userOpts)
    
    #If no list of jobs then enter name else use drop down choice
    if jobOpts == '': 
        p.addSingleLineInput("JOB:", self.job)
    else: 
        p.addEnumerationPulldown("JOB:",jobOpts)

    if self.td == 'True':  #FIX ME: Look up the correct value
      p.addEnumerationPulldown("TD:",booleanOptsT)
    else:
      p.addEnumerationPulldown("TD:",booleanOptsF)
    
    #one has to be a TD to add the dev enviroment
    #if self.td:
    #  p.addEnumerationPulldown("DEV:",booleanOpts)
    
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
      self.user = p.value("USER:")
      self.job = p.value("JOB:")
      #if self.td:
        #self.dev = p.value("DEV:")
      self.td = p.value("TD:")
      return True
    else:
      return False

  def file(self):
    'return the ini file location'
    return self.__iniFile
  
  def fileExists(self):
    'boolen checks to see if the ini file exists'
    import os.path as path
    return path.exists(self.__iniFile)

  def fileSet(self, file):
    'sets the location of the ini file'
    self.__iniFile = file
    return self.__iniFile

  def fileSave(self):
    'save the localUserSettings to an ini file'
    import ConfigParser
    import os
    
    #creates the config object
    config = ConfigParser.ConfigParser()
    
    #load data from file if it already exists
    if self.fileExists():
      #print 'File Exists'
      try:
        config.read(self.__iniFile)
      except:
        print "ERROR(read): " + self.__iniFile
    
    #update the config information
    if config.has_section(self.__iniSection) == False:
      config.add_section(self.__iniSection)
    config.set(self.__iniSection, "user", self.user)
    config.set(self.__iniSection, "td", self.td)
    config.set(self.__iniSection, "job", self.job)  
    #config.set(self.__iniSection, "dev", self.dev)
    
    #output file
    fd = open(self.__iniFile, 'w')
    config.write(fd)
    fd.close()
    return

  def fileLoad(self):
    'load the localUserSettings from an ini file'
    import ConfigParser
    import os
    
    #creates the config object
    config = ConfigParser.ConfigParser()
    
    #load data from file if it already exists
    if self.fileExists():
      #print 'File Exists'
      try:
        config.read(self.__iniFile)
      except:
        print "ERROR(read): " + self.__iniFile
    
    if config.has_section(self.__iniSection) == True:
      self.user = config.get(self.__iniSection, 'user')
      self.td = config.get(self.__iniSection, 'td')
      self.job = config.get(self.__iniSection, 'job')
      #try:
      #  self.dev = config.get(self.__iniSection, 'dev')
      #except:
      #  self.dev = False
    
    return
    
  def loadPrompt(self):
    'Prompt and load from a specified ini file'
    import nuke
    file = ""
    try: 
      file = nuke.getFilename('prompt', '*.ini', os.environ['HOME']+'/.nuke/')
      self.iniFile = file
      self.fileLoad()
    except:
      print "ERROR(file): " + file
    return
  
  def modifyPrompt(self):
    'Prompt the settings panel and save to a file'
    from nuke import ask
    from nuke import scriptNew
    
    if self.disPanel() == True:
      self.fileSave()
      if [ask('Do you want to open a new script with updated settings?')]:
        scriptNew()
      
    return

#foo = nukeDev.localUserSettings()
#foo.disPanel()

#dir('localUserSettings')
#localUserSettings.disPanel()
