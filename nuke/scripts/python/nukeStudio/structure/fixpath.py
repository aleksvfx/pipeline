#  Copyright (c) Chris Bankoff.  All Rights Reserved
#
# 

#########################################################
#    Fix path structures for cross platform support
#########################################################
#Depended on a fileStructure class from nukeStudio


####MOVE ME LATER################
#This fixes paths and makes nuke platform independed
#This is will be depricated with a pythond verison of filename_fix

if platform.system() == 'Linux':
  nuke.tcl('''
proc filename_fix {arg} {
  if {[string range $arg 0 11]=="/Volumes/s2/"} {
    return "/S2/3d/[string range $arg 12 end]"
  }
  if {[string range $arg 0 13]=="/Volumes/s2-1/"} {
    return "/S2/pers/[string range $arg 14 end]"
  }
  return $arg
}
''')
elif platform.system() == 'Darwin':
  nuke.tcl('''
proc filename_fix {arg} {
  if {[string range $arg 0 6]=="/S2/3d/"} {
    return "/Volumes/s2/[string range $arg 7 end]"
  }
  if {[string range $arg 0 8]=="/S2/pers/"} {
    return "/Volumes/s2-1/[string range $arg 9 end]"
  }
  return $arg
}
''')
#################################


def fixpath (path, platform = ''):
  'trys to fix the path structure form different platforms'
  
  import platform
  from nukeStudio.structure import fileStructure

  #the file strcuture the we are try to conform to
  replaceStructure = fileStructure(platform) 
  
  endsearch = 0
  
  for testPlatform in replaceStructure.platformTypes:
    
    #Don't test to see if already matches the file structure we are trying to confrom to
    #move on an test the next platform type
    if testPlatform == platform:
      print 'testPlatform Bypass: ' + testPlatform
      continue
    
    #create a test strcture based on a suported platform
    tempStructure = fileStructure(testPlatform)
    print 'testing: ' + testPlatform
  
    #Look for a match based on root mnt points
    for mnt in  tempStructure.rootList:
      regP = '^' + tempStructure.get(mnt)
      if re.search(regP, path):
        print 'Match: ' + mnt
        endsearch = 1
        break
  
    #completely break out of search
    if endsearch == 1:
      break
  
  print 'the End'
  
  regS = replaceStructure.get(mnt)
  
  path =  re.sub(regP, regS, path)
  

  return path




