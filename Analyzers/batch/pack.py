#!/usr/bin/env python

"""
Based on CERN CRAB3 UserTarball.py and CMSSWConfig.py
"""

import os
import glob
import tarfile

import sys
import imp
import pickle

class UserTarball(object):

    def __init__(self, name='default.tgz', mode='w:gz'):
        self.name = name
        self.mode = mode
        self.sendPythonFolder = False
        self.scriptExe = None

    def add_files(self, userFiles=None, excludeFiles=None):
        """
        Add the necessary files to the tarball
        """
        directories = ['lib', 'biglib', 'module', 'bin']
        if self.sendPythonFolder:
            directories.append('python')
            directories.append('cfipython')
        # Note that dataDirs are only looked-for and added under the src/ folder.
        # /data/ subdirs contain data files needed by the code
        # /interface/ subdirs contain C++ header files needed e.g. by ROOT6
        dataDirs    = ['data','interface']
        userFiles = userFiles or []
        def filter_fn(tarinfo):
            #print tarinfo.name
            if os.path.split(tarinfo.name)[1] in excludeFiles:
                return None
            else:
                return tarinfo

        with tarfile.open(self.name, self.mode) as tar:

            # Tar up whole directories
            for directory in directories:
                fullPath = os.path.join(os.environ['CMSSW_BASE'], directory)
                if os.path.exists(fullPath):
                    tar.add(fullPath, directory, recursive=True, filter=filter_fn)

            # Search for and tar up "data" directories in src/
            srcPath = os.path.join(os.environ['CMSSW_BASE'], 'src')
            for root, dirs, files in os.walk(srcPath):
                if os.path.basename(root) in dataDirs:
                    directory = root.replace(srcPath,'src')
                    tar.add(root, directory, recursive=True, filter=filter_fn)

            # Tar up extra files the user needs
            for globName in userFiles:
                fileNames = glob.glob(globName)
                if not fileNames:
                    raise Exception("The input file '%s' cannot be found." % globName)
                for filename in fileNames:
                    directory = os.path.basename(filename)
                    tar.add(filename, directory, recursive=True, filter=filter_fn)

            if self.scriptExe:
                tar.add(self.scriptExe, arcname=os.path.basename(self.scriptExe))


class CMSSWConfig(object):
    """
    Class to handle CMSSW _cfg.py file
    """
    def __init__(self, userConfig='pset.py', pyCfgParams=[], pklFileName='PSet.pkl', auxFileName='PSet.py'):
        self.userConfig = userConfig
        self.pyCfgParams = pyCfgParams
        self.pklFileName = pklFileName
        self.auxFileName = auxFileName

        self.fullConfig = None
        self.outputFile = None

        if userConfig:
            cfgBaseName = os.path.basename(userConfig).replace(".py", "")
            cfgDirName = os.path.dirname(os.path.abspath(userConfig))

            if not os.path.isfile(userConfig):
                msg = "Cannot find CMSSW configuration file %s in %s" % (userConfig, os.getcwd())
                raise Exception(msg)

            originalArgv = sys.argv
            sys.argv = [self.userConfig]
            if self.pyCfgParams:
                sys.argv.extend(self.pyCfgParams)
            file, pathname, description = imp.find_module(cfgBaseName, [cfgDirName])

            sys.path.append(os.getcwd())
            try:
                self.fullConfig = imp.load_module(cfgBaseName, file, pathname, description)
            finally:
                file.close()
            sys.argv = originalArgv

    def write_files(self):
        #saving the process object as a pickle
        with open(self.pklFileName, "wb") as f:
            pickle.dump(self.fullConfig.process, f)

        #create the auxiliary file
        with open(self.auxFileName, "wb") as f:
            f.write("import FWCore.ParameterSet.Config as cms\n")
            f.write("import pickle\n")
            f.write("process = pickle.load(open('%s', 'rb'))\n" % self.pklFileName)


# ______________________________________________________________________________
def main():

    print('[INFO   ] Save pset.py as pickle ...')
    pkl = CMSSWConfig()
    pkl.write_files()
    print('[INFO   ] %s%s is created (%iM).%s' % ('\033[92m', pkl.pklFileName, os.stat(pkl.pklFileName).st_size >> 20, '\033[0m'))

    print('[INFO   ] Packing tarball ...')
    print('[INFO   ] Using CMSSW_BASE: %s' % (os.environ['CMSSW_BASE']))
    print('[INFO   ] Using CMSSW_VERSION: %s' % (os.environ['CMSSW_VERSION']))
    tb = UserTarball()
    tb.add_files(userFiles=[pkl.pklFileName, pkl.auxFileName], excludeFiles=['LUT_AndrewFix_25July16.dat'])
    print('[INFO   ] %s%s is created (%iM).%s' % ('\033[92m', tb.name, os.stat(tb.name).st_size >> 20, '\033[0m'))

    def add_check():
        # Make a blank file
        open('.checkfile.txt', 'w').close()
    add_check()


# ______________________________________________________________________________
if __name__ == '__main__':

    main()

