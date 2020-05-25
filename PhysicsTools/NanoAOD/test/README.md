# TopNanoAOD

## Recipe: setting up (v6p1)

Based on central nanoAOD v6, CMSSW_10_2_18. On CC7/SLC7:
```bash
export SCRAM_ARCH=slc7_amd64_gcc700
cmsrel CMSSW_10_2_18
cd CMSSW_10_2_18/src/
cmsenv
git cms-init
git cms-merge-topic top-cmssw/topNanoV6p1_10_2_18
scram b -j 8
```

## Submitting crab jobs

```
voms-proxy-init --rfc --voms cms --valid 96:00
source /cvmfs/cms.cern.ch/common/crab-setup.sh
cd $CMSSW_BASE/PhysicsTools/NanoAOD/test
```

You'll need to prepare a JSON file listing all the samples you want to process. An example is given in [test/topSamples.json](./topSamples.json). You can either use the name of the miniAOD dataset you want to process, or the name of the nanoAOD dataset you want to reproduce (the script will then automatically fetch the parent miniAOD dataset). Then, run:
```
./runTopNanoOnGrid.py -d YOURLIST.json -o crab_tasks -s YOURSITE
```
Replace `YOURSITE` by a T2/T3 site at which you have write permissions. This creates the crab config files in the `crab_tasks` folder, but doesnt submit any jobs. You can then submit all or some of them using `crab submit`.

## Troubleshooting

You might need to adjust the job splitting options if jobs take too long to run (or run too quickly). Resubmitting using `crab resubmit --maxjobruntime` and `--siteblacklist` can also help.

In case you accidentally published a dataset twice under the same publication name, one can invalidate specific files of the dataset:
```
wget -O DBS3SetFileStatus.py https://twiki.cern.ch/twiki/pub/CMSPublic/Crab3DataHandling/DBS3SetFileStatus.py.txt
python DBS3SetFileStatus.py --url=https://cmsweb.cern.ch/dbs/prod/phys03/DBSWriter --status=invalid --recursive=False  --files=<LFN>
```
To invalidate a complete dataset, use this command:
```
wget -O DBS3SetDatasetStatus.py https://twiki.cern.ch/twiki/pub/CMSPublic/Crab3DataHandling/DBS3SetDatasetStatus.py.txt
python DBS3SetDatasetStatus.py --dataset=<datasetname> --url=https://cmsweb.cern.ch/dbs/prod/phys03/DBSWriter --status=INVALID --recursive=False
```
More information about publication [here](https://twiki.cern.ch/twiki/bin/view/CMSPublic/Crab3DataHandling#Changing_a_dataset_or_file_statu).

## Version history

### nanoAODv6

- **v6p1**: see report [here]()
    - TOP lepton MVA
    - Store NNPDF3.1 weights when available (instead of PDF4LHC)
    - PS weights storage fixed as in [this PR](https://github.com/cms-nanoAOD/cmssw/pull/506)


### nanoAODv5

See instructions [here](https://github.com/demuller/privNanoAOD).

List of samples [here](https://docs.google.com/spreadsheets/d/1SAtx-eRIuXwLp2WAuUw28no2wdV3dXBkQ8-a6jmYmBk/edit#gid=0).
