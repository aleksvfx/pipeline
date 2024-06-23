#  Copyright (c) Chris Bankoff.  All Rights Reserved
#
# 

import nuke

if nuke.GUI:
  #Load the shot navigator in to the menu stystem
  #Had to load during init not during menu
  nukeStudio.tools.shotNavigator()
