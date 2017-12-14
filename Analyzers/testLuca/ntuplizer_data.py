import FWCore.ParameterSet.Config as cms
import pset_SingleMuon_PositiveEndCap_data as pset
import FWCore.ParameterSet.VarParsing as VarParsing
process = pset.process
process.setName_('NTUPLIZER')


# ______________________________________________________________________________
# Modify source
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring('file:dummy.root'),
    # secondaryFileNames = cms.untracked.vstring(),
    # inputCommands = cms.untracked.vstring('keep *', 'drop *_simEmtfDigis_*_*', 'drop *_simGmtStage2Digis_*_*', 'drop *_simGtStage2Digis_*_*'),
)
process.maxEvents.input = cms.untracked.int32(-1)

# fileNames = [
#     "/store/user/jiafulow/L1MuonTrigger/P2_9_2_0/SingleMuon_PositiveEndCap/ParticleGuns/CRAB3/170613_001230/0000/SingleMuon_PositiveEndCap_71.root",
#     "/store/user/jiafulow/L1MuonTrigger/P2_9_2_0/SingleMuon_PositiveEndCap/ParticleGuns/CRAB3/170613_001230/0000/SingleMuon_PositiveEndCap_72.root",
#     "/store/user/jiafulow/L1MuonTrigger/P2_9_2_0/SingleMuon_PositiveEndCap/ParticleGuns/CRAB3/170613_001230/0000/SingleMuon_PositiveEndCap_73.root",
#     "/store/user/jiafulow/L1MuonTrigger/P2_9_2_0/SingleMuon_PositiveEndCap/ParticleGuns/CRAB3/170613_001230/0000/SingleMuon_PositiveEndCap_74.root",
#     "/store/user/jiafulow/L1MuonTrigger/P2_9_2_0/SingleMuon_PositiveEndCap/ParticleGuns/CRAB3/170613_001230/0000/SingleMuon_PositiveEndCap_75.root",
#     "/store/user/jiafulow/L1MuonTrigger/P2_9_2_0/SingleMuon_PositiveEndCap/ParticleGuns/CRAB3/170613_001230/0000/SingleMuon_PositiveEndCap_76.root",
#     "/store/user/jiafulow/L1MuonTrigger/P2_9_2_0/SingleMuon_PositiveEndCap/ParticleGuns/CRAB3/170613_001230/0000/SingleMuon_PositiveEndCap_77.root",
#     "/store/user/jiafulow/L1MuonTrigger/P2_9_2_0/SingleMuon_PositiveEndCap/ParticleGuns/CRAB3/170613_001230/0000/SingleMuon_PositiveEndCap_78.root",
#     "/store/user/jiafulow/L1MuonTrigger/P2_9_2_0/SingleMuon_PositiveEndCap/ParticleGuns/CRAB3/170613_001230/0000/SingleMuon_PositiveEndCap_79.root",
#     "/store/user/jiafulow/L1MuonTrigger/P2_9_2_0/SingleMuon_PositiveEndCap/ParticleGuns/CRAB3/170613_001230/0000/SingleMuon_PositiveEndCap_80.root",
# ]
# process.source.fileNames = cms.untracked.vstring(fileNames)


options = VarParsing.VarParsing ('analysis')
options.inputFiles = []
options.outputFile = 'ntuple_SingleMuon_PositiveEndCap.root'
options.parseArguments()

# if True:
# from L1TMuonSimulations.Configuration.tools import *
# txt = 'L1TMuonSimulations/Configuration/data/input_SingleMuon_PositiveEndCap.txt'
# txt = os.path.join(os.environ['CMSSW_BASE'], 'src', txt)
# fileNames_txt = loadFromFile(txt, fmt='root://cmsxrootd.fnal.gov/%s')
# process.source.fileNames = fileNames_txt

# for samples without ME0 info (PU140-200)
process.simEmtfDigis.ME0Enable = cms.bool(False)

process.load('L1TMuonSimulations.Analyzers.rpcintegration_data_cfi')
process.ntupler.outFileName = options.outputFile
process.ntupler.docString   = 'SingleMuon_PositiveEndCap'
process.ntupler.verbosity   = 0
process.TFileService        = cms.Service('TFileService', fileName = cms.string(process.ntupler.outFileName.value()))
#
process.ntuple_step = cms.Path(process.ntupler)
process.step1       = cms.Path(process.simEmtfDigis)
# process.schedule    = cms.Schedule(process.step1, process.ntuple_step)
process.schedule    = cms.Schedule(process.ntuple_step)
#
process.maxEvents.input = cms.untracked.int32(-1)
# process.MessageLogger.cerr.FwkReport.reportEvery = 100

if options.inputFiles:
  process.source.fileNames = cms.untracked.vstring(options.inputFiles)
else:
  # process.source.fileNames = cms.untracked.vstring("/store/data/Run2017H/ZeroBias/RAW/v1/000/307/048/00000/A84CDBBB-D7D0-E711-857B-02163E01A24E.root")
  process.source.fileNames = cms.untracked.vstring("file:A84CDBBB-D7D0-E711-857B-02163E01A24E.root")


# Configure framework report and summary
process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(True))

if options.inputFiles: # likely a cluster job
    process.MessageLogger.cerr.FwkReport.reportEvery = 1000
else: # likely a local test
    process.MessageLogger.cerr.FwkReport.reportEvery = 1

# Run in unscheduled mode
process.options.allowUnscheduled = cms.untracked.bool(True)

# Dump the full python config
# with open("dump.py", "w") as f:
#     f.write(process.dumpPython())

