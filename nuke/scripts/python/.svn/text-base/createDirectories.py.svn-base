#  Copyright (c) Chris Bankoff.  All Rights Reserved
#
# 
import os
import sys
import os.path
from os import system
from os import makedirs
import nuke

this_filename = sys._getframe().f_code.co_filename
print('Loading '+this_filename)

def write_mkdir(nodes = ''):
    'Make directories from write nodes'

    n = nuke.allNodes()

    for i in n:
        if i.Class() == "Write":
            i.knob("selected").setValue(True)

    snodes = nuke.selectedNodes()
    if len(snodes) == 0:
        nuke.message('ERROR: No Write nodes to work on')
        return

    for i in snodes:
        _class = i.Class()
        if _class == "Write":
            # use this to get only the filename
            path = nuke.filename(i)
            #if empty continue
            if path is None:
                continue
            #Get Directory Structure with out file name
            dirPath = os.path.dirname(path)
            if os.path.isdir (dirPath):
#               nuke.message('Directory already exists:\n%s' % (dirPath))
            	continue
            if os.path.isfile (dirPath):
                # The folder exists as a file
                msg = "The directory:\n%s\nexists as a file. Delete it and create folder?" % (dirPath)
                delete_existing_file = nuke.ask(msg)
                if (delete_existing_file):
                    os.unlink (dirPath)
                else:
                    return
                #Create directory
            try:
                makedirs(dirPath,0775)
                nuke.message('Created:\n%s' % (dirPath))
            except:
                if nuke.ask('Create Path: '+ dirPath):
#                    makedirs(dirPath,0775)
		     continue
                else:
                    return
        else:
#            nuke.message('ERROR: Skipping non-Write node.')
            continue

    return
