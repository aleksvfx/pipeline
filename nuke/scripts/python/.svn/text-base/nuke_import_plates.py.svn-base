""" Utility to examine filesystem for plate image seequences
    Intended to be usable outside of nuke. So that maya
    or houd or command line tools can obtain image seqeucnes
    for plates.

    FYI function that can used outside of nuke have a prefix of
    "fs_"
"""
import sys
import os
import os.path

kMethodToolsRoot = "METHOD_TOOLS_ROOT"
gToolsRoot       = "/tools/release"
gMethPyPath      = os.path.join( gToolsRoot, "lib/python" )   
if kMethodToolsRoot in os.environ:
    gToolsRoot  = os.environ[kMethodToolsRoot]
    gMethPyPath = os.path.join( gToolsRoot, "lib/python" )
if (not gMethPyPath in sys.path):
    sys.path.insert (0, gMethPyPath)

import facility.get_seqs as get_seqs

g_have_nuke = False
try:
    import nuke
    g_have_nuke = True
except:
    pass

# Menu.py calls this - import plates for current shot
def nuke_importPlates ():
    # Import for the current setshot
    if (not fs_checkEnvVar()):
        nuke.message( "Please use Setshot to set shot first." )
        return
    shot = os.environ ["SHOT_"]
    return importPlatesWithUi (shot)

# Menu.py calls this - ask user for shot name to import plates from
def nuke_importPlatesAskUser ():
    # Ask user for the name of the plate that they'd to import
    if (not fs_checkEnvVar()):
        nuke.message( "Please use Setshot to set shot first." )
        return

    p = nuke.Panel ("Enter name of shot to import (e.g. blr240)")
    p.addSingleLineInput("Shot name (e.g. blr240):", "")
    p.addButton("Cancel")

    p.addButton("OK")
    result = p.show()
    if (result):
        shot_name = p.value("Shot name (e.g. blr240):")
    else:
        return
    
    if (not shot_name):
        return
    if (len (shot_name) < 4):
        msg = "You need to enter a shot name, like blr240"
        nuke.message (msg)
        return
    seq = shot_name [:3]
    importPlatesWithUi (shot_name)
    return


def importPlatesWithUi (shot):
    plate_dir = fs_getPlateDir (shot)
    if (not plate_dir):
        msg = "No images/plates directory for %s" % (shot)
        if (g_have_nuke):
            nuke.message (msg)
        print msg
        return None
    img_seqs, bad_stuff = get_seqs.getAllImgSeqsThisDir (plate_dir)
    if (len (img_seqs) == 0):
        msg = "The images/plates directory %s has no plates" % (plate_dir)
        if (g_have_nuke):
            nuke.message (msg)
        print msg
        return None

    # NYI - eventually we should support base_name and when multiple
    # verison of same plate, import the latest version
    if (len (img_seqs) == 1):
        # Create read node directly
        plate_seq = img_seqs [0]
        nuke_createRead (plate_seq)
    elif (len (img_seqs) > 1):
        # dialog shows user name and frame range of eachplate
        # and import checkbox
        # default of return imports all
        nuke_AskWhichImgs (shot, img_seqs)
            
    return

def nuke_AskWhichImgs (shot, img_seqs):
    # @key name of knob to show to user in panel
    # @value img_seq obj
    knob_names_dict = {}
    for img_seq in img_seqs:
        name = img_seq.getDisplayName ()
        first = img_seq.first
        last = img_seq.last
        knob_name = "%s (%d-%d)" % (name, first, last)
        knob_names_dict [knob_name] = img_seq

    if (not g_have_nuke):
        return
    p = nuke.Panel ("Which plates do you want to import for %s" % (shot))
    for knob_name in knob_names_dict.keys ():
        p.addBooleanCheckBox(knob_name, True)
    p.addButton("Cancel")

    p.addButton("OK")
    result = p.show()
    selected_seqs = []
    if (result):
        for knob_name in knob_names_dict.keys ():
            should_import = p.value (knob_name)
            if (should_import):
                selected_seqs.append (knob_names_dict [knob_name])
    # Now actually create read nodes
    for img_seq in selected_seqs:
        plate_seq = img_seq
        plate_node = nuke_createRead (plate_seq)
        x_pos = plate_node.knob ("xpos").getValue ()
        y_pos = plate_node.knob ("ypos").getValue ()
        plate_node.setXpos (int (x_pos + 200))
        # Set display name /name of read node plate?
        # possiblly Import grade here....
    return

def nuke_createRead (plate_seq):
    plate_node = None
    if (g_have_nuke):
        plate_node = nuke.createNode ("Read", "file {%s %d-%d}" % (
            plate_seq.getPath (), plate_seq.start, plate_seq.end), inpanel = False)
    return plate_node

# ##### Handy Utilties ################

def fs_checkEnvVar():
    # ---- Check ENV Var ----- #
    envKeys = os.environ.keys()
    if (not 'CAMPAIGN_' in envKeys ) or \
    (not 'JOB_' in envKeys ) or \
    (not 'SEQUENCE_' in envKeys ) or\
    (not 'SHOT_' in envKeys ):
        return False
    return True

def fs_getPlateDir (shot):
    # This will only work for normal shot folders, not
    # lookdev or previz or common
    seq = shot [:3]
    seq_dir = os.path.join (os.environ ["JOB_PATH_"], "sequences", seq)
    if (not os.path.isdir (seq_dir)):
        return None
    plate_dir = os.path.join (seq_dir, shot, "images", "plates")
    if (not os.path.isdir (plate_dir)):
        return None
    return plate_dir

if __name__ == "__main__":
    if (not fs_checkEnvVar()):
        msg =  "Please use Setshot to set shot first."
        print msg
    #shot = os.environ ["SHOT_"]
    #print fs_getPlateDir (shot)
    #print importPlatesWithUi (shot)

