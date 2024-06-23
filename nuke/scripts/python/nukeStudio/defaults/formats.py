#  Copyright (c) Chris Bankoff.  All Rights Reserved
#
# 

##############################################################################
##                              Format Defaults                             ##
## These set a knob's initial value when a node is created.                 ##
##############################################################################

#import nuke
from nuke import addFormat
from nuke import knobDefault
import nuke

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
# 4:3 video formats
# In Nuke <= 5.1 we define video formats
old_nuke = False
if (nuke.NUKE_VERSION_MAJOR < 5):
    old_nuke = True
if (nuke.NUKE_VERSION_MAJOR == 5 and nuke.NUKE_VERSION_MINOR == 1):
    old_nuke = True

addFormat(" 720  540                    1.0   NTSC_Square")
addFormat(" 640  480                    1.0   PC_Video")
if (old_nuke):
    addFormat(" 720  486                    0.9   NTSC")
    addFormat(" 720  576                    1.067 PAL")
    addFormat(" 720  486                    1.2   NTSC_16:9")
    addFormat(" 720  576                    1.422 PAL_16:9")
# that makes PAL exactly 4:3, some docs indicate pixel aspect is 59/54

# 16:9 video formats
addFormat("1920 1080                    1.0   HD_1080")

# FILM FORMATSnuke.showInfo('Root')
addFormat("1024 778                     1.0   1K_Super_35(full-ap)")
addFormat("914 778                      2.0   1K_Cinemascope")

addFormat("2048 1556                    1.0   2K_Super_35(full-ap)")
addFormat("1828 1556                    2.0   2K_Cinemascope")

addFormat("4096 3112                    1.0   4K_Super_35(full-ap)")
addFormat("3656 3112                    2.0   4K_Cinemascope")

addFormat("2048 1558 220 284 2048 1272  1.0   2K_Academy_aperture")

#SQUARE FORMATS
addFormat(" 256  256                    1.0   square_256")
addFormat(" 512  512                    1.0   square_512")
addFormat("1024 1024                    1.0   square_1K")
addFormat("2048 2048                    1.0   square_2K")


## Script Defaults
#knobDefault('Root.proxy_type', 'format')
knobDefault('Root.format', ' 720  540                    1.0   NTSC_Square')
knobDefault('Root.proxy_format', ' 720  540                    1.0   NTSC_Square')

#knobDefault('Root.proxy_type', 'scale')
#knobDefault('Root.proxy_scale', '.5')
  
##OLD SETTINGS
#############################


## Setup any special formats:
##              W    H   x   y    r    t   pa   name
##---------------------------------------------------------------------------
#add_format "2048 1556   0   0 2048 1556  1.0   2k"
#add_format " 720  486   0   0  720  486  0.9   NTSC_video"
#add_format " 720  540   0   0  720  540  1   NTSC_Square"
#add_format " 1920  1080   0   0  1920 1080  1   HD_1080"
#add_format " 1280  720   0   0  1280  720  1   HD_720"
##add_format " 1920  817   0   0  1920 817  1   HD_2.35"
##add_format "720 486 0 68 720 418 0.9 NTSC_LeterBox_1.85"


#############################
##OLD SETTINGS
