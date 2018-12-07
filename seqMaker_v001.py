import os
import time

print "\nseqMaker Test Version 0.01 \"Caol Ila\" by Aleksandar Djordjevic."
print "Use this script to make a sequence with N number of shots in a project."
print "This being a very rudimentary early prototype this script has to be put into your job's sequence folder."
print "Brief manual:"
print "1. cd into your project in the terminal"
print "2. cd into your sequence folder in the job"
print "3. type in the name of the sequence and the number of shots you want to create"
print "\nThe script will create the shots and general subfolders.\n"
print "With great power comes great responsibility, use this for test purposes only.\n"
print "Go Giants!"

seqName = raw_input("Enter sequence name: ")
shotNumber = int(raw_input("How many shots do you want to make?: "))
shotNumber = shotNumber + 1
totalNumber = list()

os.mkdir(seqName)

for i in range(int(shotNumber)):
    if shotNumber <= 99:
        shotName = seqName+"_"+str(i).zfill(2)+"0"
    elif shotNumber > 99:
        shotName = seqName+"_"+str(i).zfill(3)+"0"
    os.mkdir(seqName+"/"+shotName)
    os.mkdir(seqName+"/"+shotName+'/elements')
    os.mkdir(seqName+"/"+shotName+'/elements/2D_renders')
    os.mkdir(seqName+"/"+shotName+'/elements/3D_renders')
    os.mkdir(seqName+"/"+shotName+'/elements/data')
    os.mkdir(seqName+"/"+shotName+'/elements/plates')
    os.mkdir(seqName+"/"+shotName+'/elements/plates/flat')
    os.mkdir(seqName+"/"+shotName+'/elements/plates/graded')
    os.mkdir(seqName+"/"+shotName+'/elements/plates/offline')
    os.mkdir(seqName+"/"+shotName+'/flame')
    os.mkdir(seqName+"/"+shotName+'/nuke')
    os.mkdir(seqName+"/"+shotName+'/photoshop')
    os.mkdir(seqName+"/"+shotName+'/maya')
    os.mkdir(seqName+"/"+shotName+'/houdini')
