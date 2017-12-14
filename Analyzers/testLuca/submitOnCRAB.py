# TEMPLATE used for automatic script submission of multiple datasets

## crab submit -c crab3_template.py \
# General.requestName=GluGluToRSGravitonToHHTo2B2Tau_M-300_narrow_13TeV-madgraph_1 General.workArea=crab3_MC_gravitonsRS_29Apr2017
# Data.inputDataset=/GluGluToRSGravitonToHHTo2B2Tau_M-300_narrow_13TeV-madgraph/RunIISummer16MiniAODv2-PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/MINIAODSIM \
# Data.outLFNDirBase=/store/user/lcadamur/HHNtuples/MC_gravitonsRS_29Apr2017/1_GluGluToRSGravitonToHHTo2B2Tau_M-300_narrow_13TeV-madgraph__RunIISummer16MiniAODv2-PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1 \
# Data.outputDatasetTag=GluGluToRSGravitonToHHTo2B2Tau_M-300_narrow_13TeV-madgraph_MC_gravitonsRS_29Apr2017_1 \
# Data.publication=False \
# JobType.maxJobRuntimeMin=2500

work_area    = 'TestingSingleMuonCRAB'
request_name = 'SingleMu_NoPU_27Nov2017_FNALwhitelist'
input_filelist = '/uscms/home/lcadamur/nobackup/EMTF_Emulator/CMSSW_9_2_14_patch2/src/L1TMuonSimulations/Configuration/data/input_SingleMuon_PositiveEndCap.txt'

# input_SingleMuon_PositiveEndCap_PU140.txt
# input_SingleMuon_PositiveEndCap_PU200.txt

########################################################

from WMCore.Configuration import Configuration
config = Configuration()

config.section_("General")
config.General.requestName = request_name
config.General.workArea = work_area

config.section_("JobType")
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'ntuple_SingleMuon_PositiveEndCap.py' # to produce LLR ntuples or EnrichedMiniAOD according to the RunNtuplizer bool
# config.JobType.sendExternalFolder = True #Needed until the PR including the Spring16 ele MVA ID is integrated in CMSSW/cms-data.

config.section_("Data")
# config.Data.inputDataset = '/my/precious/dataset'
config.Data.userInputFiles = open(input_filelist).readlines()
# config.Data.inputDBS = 'global'
# config.Data.splitting = 'EventAwareLumiBased'
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 2
# config.Data.totalUnits = -1 #number of event
config.Data.totalUnits = 4 #number of event
config.Data.outLFNDirBase = '/store/user/lcadamur/EMTF_trigger/%s' % request_name
config.Data.publication = False
config.Data.outputDatasetTag = request_name

config.section_("Site")
config.Site.storageSite = 'T3_US_FNALLPC'
config.Site.whitelist = ['T3_US_FNALLPC']