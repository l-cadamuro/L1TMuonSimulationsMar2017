import os, sys
import re
from subprocess import Popen, PIPE
import argparse

# tag = "SingleMuon_PositiveEndCap"
# outFolder = './'

areEnrichedMiniAOD = False; # if true:  add a header and the /store.. etc to run ntuplizer on Tier3 on CMSSW
                                 # if false: only add the polgrid server to run the skim and submit on root
# ====================================================================

def formatName (name, pathToRem, addServ=True):
    name.strip(pathToRem)
    name = "root://polgrid4.in2p3.fr/" + name if addServ else name
    return name

def saveToFile (lista, filename, areEnrichedMiniAOD):
    f = open (filename, "w")

    if not areEnrichedMiniAOD:
        for elem in lista:
            f.write("%s\n" % elem) #vintage
    
    else:
        f.write("FILELIST = cms.untracked.vstring()\n")
        f.write("FILELIST.extend ([\n")
        for elem in lista:
            elem = elem.replace ("root://polgrid4.in2p3.fr//dpm/in2p3.fr/home/cms/trivcat", "")
            f.write ("    '")
            f.write("%s',\n" % elem) #vintage  
        f.write("])\n")
 
    f.close()

def atoi(text):
    return int(text) if text.isdigit() else text

def natural_keys(text):
    '''
    alist.sort(key=natural_keys) sorts in human order
    http://nedbatchelder.com/blog/200712/human_sorting.html
    (See Toothy's implementation in the comments)
    '''
    return [ atoi(c) for c in re.split('(\d+)', text) ]

# ====================================================================

parser = argparse.ArgumentParser(description='Command line parser of plotting options')

parser.add_argument('--tag', dest='tag', help='process tag',     default=None)
parser.add_argument('--out', dest='out', help='output file',     default=None)
parser.add_argument('--add-xrootd-serv', dest='addxrootdserv', help='add xrootd serv to LFN', action='store_true', default=False)

args = parser.parse_args()

if not args.tag:
    print "please provide the tag production name"
    sys.exit()
# if not args.out:
#     print "please provide output name"
#     sys.exit()

tag = args.tag
if tag.startswith('jobs_'):
    tag = tag.replace('jobs_', '', 1) ## remove first occurrence of jobs_ in the folder name

useOnly = [] #empty list to do list fot all the folders
print useOnly

# outFolder = ''

# dpmhome = "/dpm/in2p3.fr/home/cms/trivcat"
# partialPath = "/store/user/lcadamur/HHNtuples/" #folder contenente la produzione
dpmhome = ""
# partialPath = "/store/group/l1upgrades/L1MuonTrigger/P2_9_2_3_patch1/" #folder contenente la produzione
# partialPath = "/store/user/jiafulow/L1MuonTrigger/P2_9_2_0/"
partialPath = "/store/user/lcadamur/EMTF_ntuples/"

# partialPath = "/store/user/salerno/HHNtuples/"
#partialPath = "/store/user/davignon/EnrichedMiniAOD/"
#partialPath = "/store/user/gortona/HHNtuples/"

path = dpmhome + partialPath + tag
# if outFolder[-1] != "/": outFolder += '/'

command = "eos root://cmseos.fnal.gov ls %s" % path
# print command
pipe = Popen(command, shell=True, stdout=PIPE)

allLists = {} #dictionary

for line in pipe.stdout:
    
    #print line
    if useOnly:
        if not (line.strip()) in useOnly:
            continue
    samplesPath = (path + "/" + line).strip()    
    sampleName = line
    allLists[sampleName] = []
    print sampleName.strip()
    for level in range(0, 2): #sampleName, tag, hash subfolders
        # comm = "/usr/bin/rfdir %s" % samplesPath.strip()
        comm = "eos root://cmseos.fnal.gov ls %s" % samplesPath.strip()
        #print "comm: ==> " , comm
        pipeNested = Popen (comm, shell=True, stdout=PIPE)
        out = pipeNested.stdout.readlines()
        numLines = len (out)
        if numLines > 0 :
            if numLines > 1 : print "  *** WARNING: In %s too many subfolders, using last one (most recent submission)" % samplesPath        
            samplesPath = samplesPath + "/" + out[-1].strip()
    # print samplesPath
    # now I have to loop over the folders 0000, 1000, etc...
    comm = "eos root://cmseos.fnal.gov ls %s" % samplesPath.strip()
    pipeNested = Popen (comm, shell=True, stdout=PIPE)
    out = pipeNested.stdout.readlines()
    for subfol in out:
        finalDir = samplesPath + "/" + subfol.strip()
        getFilesComm = ""
        if areEnrichedMiniAOD :
            getFilesComm = "eos root://cmseos.fnal.gov ls %s | grep Enriched_miniAOD" % finalDir.strip()
        else :
            getFilesComm = "eos root://cmseos.fnal.gov ls %s | grep \.root" % finalDir.strip()
            #print getFilesComm
        pipeGetFiles = Popen (getFilesComm, shell=True, stdout=PIPE)
        outGetFiles = pipeGetFiles.stdout.readlines()
        for filename in outGetFiles:
            name = formatName (finalDir + "/" + filename.strip(), dpmhome, args.addxrootdserv)
            allLists[sampleName].append (name)

# now I have all file lists, save them
for sample, lista in allLists.iteritems():
    if lista:
        lista.sort(key=natural_keys)
        # outName = outFolder + sample.strip()+".txt"
        if args.out:
            outName = args.out
            if areEnrichedMiniAOD : outName = outFolder + sample.strip()+".py"
            saveToFile (lista, outName, areEnrichedMiniAOD)
        else:
            for l in lista: print l
    else:
        print "  *** WARNING: Folder for dataset %s is empty" % sample

