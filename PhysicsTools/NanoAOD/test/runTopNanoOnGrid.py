#! /usr/bin/env python

from CRABAPI.RawCommand import crabCommand
import CRABClient.UserUtilities as crab

import json
import copy
import os
import argparse
import sys

CMSSW_ROOT = os.path.join(os.environ['CMSSW_BASE'], 'src')
NANO_ROOT = os.path.join(os.environ['CMSSW_BASE'], 'src', 'PhysicsTools', 'NanoAOD')
PROD_TAG = "v6p1"

def get_options():
    """
    Parse and return the arguments provided by the user.
    """
    parser = argparse.ArgumentParser(description='Generate crab config files for multiple datasets.')

    parser.add_argument('-e', '--era', required=False, choices=['2016', '2017', '2018'], help='Choose era (year). If not specified, run on all eras')
    parser.add_argument('-d', '--datasets', nargs='*', help='Json file(s) with dataset list')
    parser.add_argument('-s', '--site', required=True, help='Site to which to write the output')
    parser.add_argument('-o', '--output', default='./', help='Folder in which to write the config files')

    return parser.parse_args()


def create_default_config():
    config = crab.config()

    config.General.workArea = 'tasks'
    config.General.transferOutputs = True
    config.General.transferLogs = True
    config.JobType.allowUndistributedCMSSW = True # for slc7

    config.JobType.pluginName = 'Analysis'
    config.JobType.maxMemoryMB = 2500

    config.Data.inputDBS = 'global'
    config.Data.splitting = 'EventAwareLumiBased'
    config.Data.unitsPerJob = 800000
    config.Data.publication = True

    return config


def findPSet(pset):
    c = pset
    if not os.path.isfile(c):
        # Try to find the psetName file
        filename = os.path.basename(c)
        path = NANO_ROOT
        c = None
        for root, dirs, files in os.walk(path):
            if filename in files:
                c = os.path.join(root, filename)
                break
        if c is None:
            raise IOError('Configuration file %r not found' % filename)
    return os.path.abspath(c)


def writeCrabConfig(pset, dataset, metadata, era, crab_config, site, output):
    c = copy.deepcopy(crab_config)

    c.JobType.psetName = pset

    name = metadata['name']

    c.General.requestName = "TopNanoAOD{}_{}__{}".format(PROD_TAG, name, era)
    
    c.Data.outputDatasetTag = "TopNanoAOD{}_{}".format(PROD_TAG, era)
    c.Data.inputDataset = dataset
    c.Data.outLFNDirBase = '/store/user/{user}/topNanoAOD/{tag}/{era}/'.format(user=os.getenv('USER'), tag=PROD_TAG, era=era)
    c.Site.storageSite = site

    # Some jobs may request more memory
    if 'memory' in metadata:
        c.JobType.maxMemoryMB = metadata['memory']
    if 'inputDBS' in metadata:
        c.Data.inputDBS = metadata["inputDBS"]
    if 'splitting' in metadata:
        c.Data.splitting = metadata["splitting"]
    if 'unitsPerJob' in metadata:
        c.Data.unitsPerJob = metadata["unitsPerJob"]
    if "max_runtime" in metadata:
        c.JobType.maxJobRuntimeMin = metadata["max_runtime"]

    print("Creating new task {}".format(c.General.requestName))
    print("\tDataset: ".format(dataset))

    # Create output file
    crab_config_file = os.path.join(output, 'crab_' + c.General.requestName + '.py')
    with open(crab_config_file, 'w') as f:
        f.write(str(c))
    
    print('Configuration file saved as %r' % (crab_config_file))



if __name__ == "__main__":

    options = get_options()
    
    if not os.path.isdir(options.output):
        os.makedirs(options.output)

    # Load datasets
    datasets = {}
    for dataset in options.datasets:
        with open(dataset) as f:
            datasets.update(json.load(f))

    crab_config = create_default_config()

    for era, era_datasets in datasets.items():
        if options.era and era != options.era:
            continue

        for dataset, metadata in era_datasets.items():
            pset = findPSet("topNano_{}_{}_cfg.py".format(PROD_TAG, era))
            writeCrabConfig(pset, dataset, metadata, era, crab_config, options.site, options.output)
