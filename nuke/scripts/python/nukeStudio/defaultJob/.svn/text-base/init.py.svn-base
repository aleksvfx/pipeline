#  Copyright (c) Chris Bankoff.  All Rights Reserved
#
# 



#########################################################
#    JOB CUSTOM INIT
######################################################### 



#Add Menu System
nuke.pluginAddPath("./menu")

#Add gizmos
nuke.pluginAddPath("./gizmos")

#Add plugins
nuke.pluginAddPath("./plugins")




#nuke.message('jobCustom:LOADED')


# Formats let Nuke assign a name, pixel aspect ratio, and possibly
# a cropped image area, to any size of input image. You can make
# more than one format for a given size but the user will have to
# pick the other ones from the chooser in the file reader.
#
#       W = Total image width in pixels
#       H = Total image height in pixels
#       x = left edge of active region
#       y = bottom edge of active region
#       r = right edge of active region
#       t = top edge of active region
#      pa = pixel-aspect ratio (width/height of a pixel)
#    name = the name displayed in the menus
#
# xyrt may be omitted to set them to 0 0 W H.
# if xyrt are omitted, you can also omit pa and it is set to 1.0
#---------------------------------------------------------------------------

#             W    H   x   y    r    t   pa   name
#---------------------------------------------------------------------------
# Job formats
#addFormat(" 720  540                    1.0   NTSC_Square")
#addFormat(" 720  486                    0.9   NTSC")


## Script Defaults
#knobDefault('Root.proxy_type', 'format')

nuke.knobDefault('Root.format', "1920 1080                    1.0   HD_1080")
nuke.knobDefault('Root.proxy_format', "1920 1080                    1.0   HD_1080")

#nuke.knobDefault('Root.format', " 720  540                    1.0   NTSC_Square")
#nuke.knobDefault('Root.proxy_format', " 720  540                    1.0   NTSC_Square")


#DEBUG:
if nukeStudio.JOB_CUSTOM_PATH == nukeStudio.DEFAULT_JOB_LOC:
  nuke.message("DEFUALT_CUSTOM_LOADED: " + nukeStudio.JOB_NAME + "\n" + nukeStudio.JOB_CUSTOM_PATH)

