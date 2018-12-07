import os
import time

print "\njobMaker Test Version 0.02 \"Oban\" by Aleksandar Djordjevic."
print "With great power comes great responsibility, use this for test purposes only.\n"
print "Go Giants!\n"

serverFolder = ""
jobName = raw_input("Enter job name: ")
jobDate = time.strftime("%Y%m")
jobID = raw_input("Enter three numeric global job ID (001 - 999): ")
job = jobDate+jobID+"_"+jobName
os.mkdir(job)

# at this point the same job should be made on shotgun, so let's name this
# shotgun subroutine

#archive
os.mkdir(job+'/archive/')
os.mkdir(job+'/archive/autodesk')
os.mkdir(job+'/archive/davinci')

#assets
os.mkdir(job+'/assets')
os.mkdir(job+'/assets/2D')
os.mkdir(job+'/assets/3D')

#config
os.mkdir(job+'/config')
os.mkdir(job+'/config/aftereffects')
os.mkdir(job+'/config/houdini')
os.mkdir(job+'/config/maya')
os.mkdir(job+'/config/nuke')
os.mkdir(job+'/config/pipeline')

#design
os.mkdir(job+'/design')
os.mkdir(job+'/design/aftereffects')
os.mkdir(job+'/design/c4d')
os.mkdir(job+'/design/photoshop')

#docs
os.mkdir(job+'/docs')
os.mkdir(job+'/docs/boards')
os.mkdir(job+'/docs/notes')
os.mkdir(job+'/docs/notes/01_internal')
os.mkdir(job+'/docs/notes/02_client')
os.mkdir(job+'/docs/production')
os.mkdir(job+'/docs/production/estimates')
os.mkdir(job+'/docs/production/schedules')
os.mkdir(job+'/docs/production/resources')
os.mkdir(job+'/docs/reference')
os.mkdir(job+'/docs/setData')

#library
os.mkdir(job+'/library')
os.mkdir(job+'/library/audio')
os.mkdir(job+'/library/edl')
os.mkdir(job+'/library/export')
os.mkdir(job+'/library/export/approval')
os.mkdir(job+'/library/export/delivery')
os.mkdir(job+'/library/export/delivery/generic')
os.mkdir(job+'/library/export/delivery/titled')
os.mkdir(job+'/library/offlines')
os.mkdir(job+'/library/rushes')
os.mkdir(job+'/library/selects')
os.mkdir(job+'/library/selects/flat')
os.mkdir(job+'/library/selects/graded')

#pitch
os.mkdir(job+'/pitch')
os.mkdir(job+'/pitch/docs')
os.mkdir(job+'/pitch/research')
os.mkdir(job+'/pitch/development')
os.mkdir(job+'/pitch/treatment')
os.mkdir(job+'/pitch/postings')

#sequences
os.mkdir(job+'/sequences')

#temp
os.mkdir(job+'/temp')
