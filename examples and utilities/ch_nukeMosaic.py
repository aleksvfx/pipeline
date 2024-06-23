#################################################################################################################################################


# NUKE MOSAIC - ch_nukeMosaic.py
# Nile Hylton - http://www.chunkified.com
# CURRENT DEV FILE : nuke_mosaic_v01_02.py

# Creates a picture mosaic out of nuke nodes


# *************************************************************************************************************************************************


import math


class Mosaic( nukescripts.PythonPanel ):
                     
     # INIT
    def __init__ (self):
        
        nukescripts.PythonPanel.__init__( self, 'ch_nukeMosaic v01', 'com.chunkified.Mosaic' )
                
        # width and height of a node inside nuke
        self.nW = 80
        self.nH = 70
        self.warning = False
        
        self.mosaicUI()


    
    def mosaicUI(self):
        
        # * WINDOW PROPERTIES    
        self.setMinimumSize ( 500, 200 )
        self.setMaximumSize ( 500, 200 )
        
        self.titleText = nuke.Text_Knob("", "")
        self.titleText.setValue("ch_nukeMosaic :)\nSelect a node, set the divisions and press 'Build' ")
        
        self.spaceText = nuke.Text_Knob("", " ")
        self.spaceText.setTooltip("")
        
        self.widthKnob = nuke.Int_Knob("Width divisions", "Width divisions")
        self.widthKnob.setValue(50)
        
        self.heightKnob = nuke.Int_Knob("Height divisions", "Height divisions")
        self.heightKnob.setValue(50)
        
        self.totalKnob = nuke.Int_Knob("Total nodes", "Total nodes")
        self.totalKnob.setValue( self.widthKnob.value() * self.heightKnob.value() )
        self.totalKnob.setEnabled( False )
        
        self.buildKnob = nuke.PyScript_Knob('Click to build mosaic', ' Build ')
        self.buildKnob.setFlag (0x1000)
        
        self.warnKnob = nuke.Text_Knob("", "")
        self.warnKnob.setValue("\nYou may want to lower your division \namount as you're current node count is: " + str( self.totalKnob.value() ) )
        
        self.addKnob( self.titleText )
        self.addKnob( self.spaceText )
        self.addKnob( self.widthKnob )
        self.addKnob( self.heightKnob )
        self.addKnob( self.totalKnob )
        self.addKnob( self.buildKnob )
        
        self.showModalDialog()

    
    
    
    # BUILD
    # this is where the mosaic is built.    
    def build(self, divW, divH):              
              
        self.loop = divW * divH # total number of tiles and how many times the loop below will run
        
        self.image = nuke.selectedNode()
        self.iW = self.image.width()
        self.iH = self.image.height()
        
        # sample width and height (space between each sampled pixel in the image)
        self.sW = int( self.iW / divW )
        self.sH = int( self.iH / divH )


        for i in range(self.loop):
            
            # sampR, sampG and sampB are where the pixel data from the image is stored temporarily
            self.sampR = nuke.sample( self.image, 'red', ( i % divW ) * self.sW, math.floor( i / divH ) * self.sH )
            self.sampG = nuke.sample( self.image, 'green', ( i % divW ) * self.sW, math.floor( i / divH ) * self.sH )
            self.sampB = nuke.sample( self.image, 'blue', ( i % divW ) * self.sW, math.floor( i / divH ) * self.sH )
        
            # sampled colours are then converted into a hex colour string which is then converted into an RGB integer.
            # because the sampled values range from 0 to 1 they need to be multiplied by 255 to get to an RGB 255 value.
            # the colour hex string will be in the following format: 0xRRGGBBAA
            self.hexchars = "0123456789ABCDEF"
            self.hexc = "0x" + self.hexchars[int((self.sampR*256)) / 16] + self.hexchars[int((self.sampR*256)) % 16] + self.hexchars[int((self.sampG*256)) / 16] + self.hexchars[int((self.sampG*256)) % 16] + self.hexchars[int((self.sampB *256)) / 16] + self.hexchars[int((self.sampB *256)) % 16] + "FF"
            self.tc = int(self.hexc, 0)
        
            # the sampled values are stored in array which are then passed into the color knob of a Constant
            self.cc = []
            self.cc = [self.sampR, self.sampG, self.sampB, 0]
            self.cons = nuke.nodes.Constant()
            self.cons.knob('color').setValue(self.cc)
            
            # the tile_color and gl_color knobs have there values set to the RGB integer
            self.cons.knob('tile_color').setValue(self.tc)
            self.cons.knob('gl_color').setValue(self.tc)
            
            # position of the constant is according to the pixels in the image 
            self.cons.setXYpos( ( i % divW ) * self.nW, -math.floor( i / divH ) * self.nH )
            
    
    
    def knobChanged(self, k):
        
            if k == self.buildKnob:                
                self.build( int( self.widthKnob.value() ), int( self.heightKnob.value()) )
                
            
            if k == self.widthKnob or k == self.heightKnob:
                self.totalKnob.setValue( self.widthKnob.value() * self.heightKnob.value() )
                if self.totalKnob.value() > 10000 and self.warning == False:
                    self.warnKnob.setValue("\nYou may want to lower your division \namount as you're current node count is: " + str( self.totalKnob.value() ) )
                    self.addKnob(self.warnKnob)
                    self.warning = True
                    self.setMinimumSize ( 500, 230 )
                    self.setMaximumSize ( 500, 230 )
                    
                elif self.totalKnob.value() <= 10000 and self.warning == True:
                    self.removeKnob(self.warnKnob)
                    self.warning = False
                    self.setMinimumSize ( 500, 200 )
                    self.setMaximumSize ( 500, 200 )
          
        
        
      
    def showModalDialog( self ):
        
        self.__modalResult = None;
        self.showModal()
        return self.__modalResult
    
    
    
mo = Mosaic()
