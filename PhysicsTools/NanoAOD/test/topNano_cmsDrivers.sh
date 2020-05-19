function runDriver () {
    cmsDriver.py NANO --python_filename topNano_v6p1_$1_cfg.py --fileout file:tree.root -s NANO --mc --conditions $2 --era $3 --eventcontent NANOAODSIM --datatier NANOAODSIM --customise_commands="process.add_(cms.Service('InitRootHandlers', EnableIMT = cms.untracked.bool(False)));process.MessageLogger.cerr.FwkReport.reportEvery=1000;process.NANOAODSIMoutput.fakeNameForCrab=cms.untracked.bool(True)" --nThreads 2 -n -1 --no_exec
}

runDriver 2016 102X_mcRun2_asymptotic_v7      Run2_2016,run2_nanoAOD_94X2016
runDriver 2017 102X_mc2017_realistic_v7       Run2_2017,run2_nanoAOD_94XMiniAODv2
runDriver 2018 102X_upgrade2018_realistic_v20 Run2_2018,run2_nanoAOD_102Xv1
