import os, sys
import re
from subprocess import Popen, PIPE
import argparse

parser = argparse.ArgumentParser(description='Command line parser of plotting options')

parser.add_argument('--tag', dest='tag', help='process tag',     default=None)
parser.add_argument('--out', dest='out', help='output file',     default=None)
parser.add_argument('--add-xrootd-serv', dest='addxrootdserv', help='add xrootd serv to LFN', action='store_true', default=False)

args = parser.parse_args()

if not args.tag:
    print "please provide the tag production name"
    sys.exit()

# def saveToFile (lista, filename):
#     f = open (filename, "w")

#     # if not areEnrichedMiniAOD:
#     for elem in lista:
#         f.write("%s\n" % elem) #vintage
    
#     # else:
#     #     f.write("FILELIST = cms.untracked.vstring()\n")
#     #     f.write("FILELIST.extend ([\n")
#     #     for elem in lista:
#     #         elem = elem.replace ("root://polgrid4.in2p3.fr//dpm/in2p3.fr/home/cms/trivcat", "")
#     #         f.write ("    '")
#     #         f.write("%s',\n" % elem) #vintage  
#     #     f.write("])\n")
 
#     f.close()


############################################################################################

tag = args.tag
if tag.startswith('jobs_'):
    tag = tag.replace('jobs_', '', 1) ## remove first occurrence of jobs_ in the folder name
print tag

match = 'ntuple_*.root'

############################################################################################

proto = '/eos/uscms/store/user/lcadamur/EMTF_ntuples/{0}/output/{1}'.format(tag, match)
command = 'ls %s' % proto

pipe = Popen(command, shell=True, stdout=PIPE)
files = []
for line in pipe.stdout:
    line = line.strip()
    if not line:
        continue
    files.append(line)

############################################################################################

if args.addxrootdserv:
    xrootdServer = 'root://cmsxrootd.fnal.gov/'
    files = [xrootdServer + f.replace('/eos/uscms', '') for f in files]

############################################################################################

if args.out:
    fout = open (args.out, "w")
    for f in files:
        fout.write("%s\n" % f)
else:
    for f in files:
        print f
