
import nuke, os


# customising the creation of nodes
#
# override nuke.createNode to return the custom write node
#
# it is possible to override nuke.nodes. as well.  See the user manual for an example.

nukeOriginalCreateNode = nuke.createNode

def attachCustomCreateNode():
  nuke.createNode = customCreateNode

def customCreateNode(node, knobs = "", inpanel = True):
  if node == "Write":
    writeNode = nukeOriginalCreateNode( node, knobs, inpanel )
    ## attach our custom tab
    createJobSystemTab( writeNode )
    return writeNode
  else:
    return nukeOriginalCreateNode( node, knobs, inpanel )
    
########################################################################

def createJobSystemTab(node):
    #### create knobs
    tabKnob = nuke.Tab_Knob('JobSystem')
    seqKnob = nuke.EvalString_Knob('seq')
    planKnob = nuke.EvalString_Knob('plan')
    versionKnob = nuke.Int_Knob('version')
    takeKnob = nuke.Int_Knob('take')
    labelKnob = nuke.EvalString_Knob('usr_label', 'label')
    extKnob = nuke.EvalString_Knob('ext')
    buttonKnob = nuke.PyScript_Knob('Create Ouptut')

    #### set some defaults for the knobs
    seqKnob.setValue(os.environ.get('SEQ'))
    planKnob.setValue(os.environ.get('PLAN'))
    versionKnob.setValue(1)
    takeKnob.setValue(1)
    labelKnob.setValue('look')
    extKnob.setValue('exr')

  #### the python script to run when the user presses the 'Create dirs' button
    script = """
seq = nuke.thisNode()['seq'].value()
plan = nuke.thisNode()['plan'].value()
version = nuke.thisNode()['version'].value()
take = nuke.thisNode()['take'].value()
label = nuke.thisNode()['usr_label'].value()
ext = nuke.thisNode()['ext'].value()
user = os.environ.get('USER')

#### build up the shot name
seqName = 's%s_p%s' % (seq, plan)
versionName = 'v%02d'% (int(version))

#### grab base render directory from environment
baseDir = os.environ.get('PIC')
if baseDir == None:
  baseDir = 'I:/Images/Project/Compo'
fullPath = os.path.join(baseDir,seqName,versionName)

try:
    os.makedirs(fullPath)
    nuke.message('Created dir %s' % fullPath)
except OSError:
    nuke.message('WARNING: err creating dir %s' % dir)
    pass

fileName = '%s_%s.%s.%s' % (seqName,versionName, '%04d', ext)
fname = os.path.join(baseDir,seqName,versionName,fileName)

#### set the file knob to the new file name
nuke.thisNode()['file'].setValue(fname)
"""
    buttonKnob.setValue(script)

    #### add knobs to node
    for k in [tabKnob, seqKnob, planKnob, versionKnob, takeKnob,labelKnob, buttonKnob, extKnob]:
        node.addKnob(k)
        
