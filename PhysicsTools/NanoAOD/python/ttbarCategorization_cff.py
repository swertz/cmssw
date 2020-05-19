from Configuration.Eras.Modifier_run2_miniAOD_80XLegacy_cff import run2_miniAOD_80XLegacy


import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import *

from PhysicsTools.NanoAOD.jets_cff import genJetTable


##################### User floats producers, selectors ##########################

from PhysicsTools.JetMCAlgos.GenHFHadronMatcher_cff import matchGenBHadron
matchGenBHadron = matchGenBHadron.clone(
    genParticles = cms.InputTag("prunedGenParticles"),
    jetFlavourInfos = cms.InputTag("slimmedGenJetsFlavourInfos"),
)

from PhysicsTools.JetMCAlgos.GenHFHadronMatcher_cff import matchGenCHadron
matchGenCHadron = matchGenCHadron.clone(
    genParticles = cms.InputTag("prunedGenParticles"),
    jetFlavourInfos = cms.InputTag("slimmedGenJetsFlavourInfos"),
)

## Producer for ttbar categorisation ID
from TopQuarkAnalysis.TopTools.GenTtbarCategorizer_cfi import categorizeGenTtbar
categorizeGenTtbar = categorizeGenTtbar.clone(
    genJetPtMin = 20.,
    genJetAbsEtaMax = 2.4,
    genJets = cms.InputTag("slimmedGenJets"),
)


### Era dependent customization
run2_miniAOD_80XLegacy.toModify( matchGenBHadron, jetFlavourInfos = cms.InputTag("genJetFlavourAssociation"),)

run2_miniAOD_80XLegacy.toModify( matchGenCHadron, jetFlavourInfos = cms.InputTag("genJetFlavourAssociation"),)


##################### Tables for final output and docs ##########################
ttbarCategoryTable = cms.EDProducer("GlobalVariablesTableProducer",
                                    variables = cms.PSet(
                                        genTtbarId = ExtVar( cms.InputTag("categorizeGenTtbar:genTtbarId"), "int", doc = "ttbar categorization")
                                    )
)

ttbarBHadronOriginTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = genJetTable.src,
    cut = genJetTable.cut,
    name = genJetTable.name,
    singleton = cms.bool(False),
    extension = cms.bool(True),
    externalVariables = cms.PSet(
        nBHadFromT = ExtVar(cms.InputTag("categorizeGenTtbar:nBHadFromT"), int, doc="number of matched B hadrons with a top quark in their ancestry"),
        nBHadFromTbar = ExtVar(cms.InputTag("categorizeGenTtbar:nBHadFromTbar"), int, doc="number of matched B hadrons with an anti-top quark in their ancestry"),
        nBHadFromW = ExtVar(cms.InputTag("categorizeGenTtbar:nBHadFromW"), int, doc="number of matched B hadrons with a W boson in their ancestry"),
        nBHadOther = ExtVar(cms.InputTag("categorizeGenTtbar:nBHadOther"), int, doc="number of matched B hadrons with no W boson or top quark in their ancestry"),
        nCHadFromW = ExtVar(cms.InputTag("categorizeGenTtbar:nCHadFromW"), int, doc="number of matched prompt (no intermediate B) C hadrons with a W boson in their ancestry"),
        nCHadOther = ExtVar(cms.InputTag("categorizeGenTtbar:nCHadOther"), int, doc="number of matched prompt (no intermediate B) C hadrons with no W boson or top quark in their ancestry"),
    ),
)

ttbarCatMCProducers = cms.Sequence(matchGenBHadron + matchGenCHadron + categorizeGenTtbar + ttbarBHadronOriginTable)
