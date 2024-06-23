#  Copyright (c) Chris Bankoff.  All Rights Reserved
#
# 

##############################################################################
##                              Knob Defaults                               ##
## These set a knob's initial value when a node is created.                 ##
##############################################################################

#import nuke
from nuke import knobDefault
#from nuke import defaultFontPathname

#This is work around
def studioFontLocation():
  FONT_LOC = '/S2/3d/farm/nuke/studio_custom/font/Vera.ttf'
  return FONT_LOC

#the init also replace nuke's defaultFontPathname function with a studioFontLocation function 
#knobDefault('Text.font', studioFontLocation())


#Mask to unchecked rbg.alpha and mask from to imput
############################
__nodeList = ('ColorCorrect', 'CCorrect', 'Clamp', 'ColorLookup', 'CCrosstalk', 'EXPTool', 'Grade', 'Histogram', 'HueCorrect', 'HueShift', 'Invert', 'Posterize', 'RolloffContrast', 'Saturation', 'Add', 'Multiply', 'Gamma', 'ClipTest', 'Expression', 'Merge', 'Merge2', 'Blur', 'Bezier', 'Grid', 'Ramp', 'Rectangle', 'Copy', 'TransformMasked', 'ZDefocus')
for __node in __nodeList:
  knobDefault(__node+'.maskChannel', '-rgba.alpha')
  knobDefault(__node+'.maskFrom', 'input') 

#Default Channel to rgba
############################
__nodeList = ('Blur', 'Convolve', 'Defocus', 'DegrainSimple', 'DirBlurWrapper', 'EdgeBlur', 'Emboss', 'Dilate', 'FilterErode', 'Glow', 'GodRays', 'Laplacian', 'Median', 'Sharpen', 'Soften', 'Dither', 'Glint', 'TransformMasked' )
for __node in __nodeList:
  knobDefault(__node+'.channels', 'rgba')
knobDefault('Glow.nonlinear', 'true')

#UnPreMult to unchecked rgb.alpha
############################
__nodeList = ('ColorCorrect', 'CCorrect', 'Clamp', 'ColorLookup', 'CCrosstalk', 'EXPTool', 'Grade', 'Histogram', 'HueCorrect', 'HueShift', 'Invert', 'Posterize', 'RolloffContrast', 'Saturation', 'Add', 'Multiply', 'Gamma', 'ClipTest', 'Expression')
for __node in __nodeList:
  knobDefault(__node+'.unpremult', '-rgba.alpha')



#Misc NODES
############################





##OLD SETTINGS
#############################
##set font default
#knob_default Text.font S:/Production_Tools/6_Nuke/sway_custom/fonts/ARIAL.TTF

#knob_default Write._exr_compression	None
#knob_default Write.quality 1

#knob_default Glow.nonlinear	true


## set layers to rgba instead of all default
#knob_default Blur.channels rgba
#knob_default Defocus.channels rgba
#knob_default IDistort.channels rgba
#knob_default Emboss.channels rgba
#knob_default Multiply.channels rgba
#knob_default Clamp.channels rgba
#knob_default Add.channels rgba
#knob_default Invert.channels rgba
#knob_default EdgeBlur.channels rgba
#knob_default VectorBlur.channels rgba

#knob_default IDistort.uv uv
#knob_default VectorBlur.uv uv

## Set default mask and unpremult options 
#knob_default Correct.unpremult -rgba.alpha
#knob_default Correct.mask -rgba.alpha
#knob_default CCorrect.unpremult -rgba.alpha
#knob_default CCorrect.mask -rgba.alpha
#knob_default ColorCorrect.unpremult -rgba.alpha
#knob_default ColorCorrect.mask -rgba.alpha
#knob_default Grade.unpremult -rgba.alpha
#knob_default Grade.mask -rgba.alpha
#knob_default HueCorrect.unpremult -rgba.alpha
#knob_default HueCorrect.mask -rgba.alpha
#knob_default Lookup.unpremult -rgba.alpha
#knob_default Lookup.mask -rgba.alpha
#knob_default Saturation.unpremult -rgba.alpha
#knob_default Saturation.mask -rgba.alpha
#knob_default HueShift.unpremult -rgba.alpha
#knob_default HueShift.mask -rgba.alpha
#knob_default Histogram.unpremult -rgba.alpha
#knob_default Histogram.mask -rgba.alpha
#knob_default Crosstalk.unpremult -rgba.alpha
#knob_default Crosstalk.mask -rgba.alpha 
#knob_default Clamp.unpremult -rgba.alpha
#knob_default Clamp.mask -rgba.alpha 
#knob_default Invert.unpremult -rgba.alpha
#knob_default Invert.mask -rgba.alpha
#knob_default EXPTool.unpremult -rgba.alpha
#knob_default EXPTool.mask -rgba.alpha

#############################
##OLD SETTINGS
