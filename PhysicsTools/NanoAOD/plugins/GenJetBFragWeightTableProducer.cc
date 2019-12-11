// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "SimDataFormats/JetMatching/interface/JetFlavourInfo.h"

#include "DataFormats/NanoAOD/interface/FlatTable.h"

#include "CommonTools/Utils/interface/StringCutObjectSelector.h"
#include "CommonTools/Utils/interface/StringObjectFunction.h"

class GenJetBFragWeightTableProducer : public edm::stream::EDProducer<> {
public:
  explicit GenJetBFragWeightTableProducer(const edm::ParameterSet& iConfig)
      : name_(iConfig.getParameter<std::string>("name")),
        jetsToken_(consumes<std::vector<reco::GenJet>>(iConfig.getParameter<edm::InputTag>("genJets"))),
        jetsWithNuToken_(consumes<std::vector<reco::GenJet>>(iConfig.getParameter<edm::InputTag>("genJetsWithNu"))),
        cut_(iConfig.getParameter<std::string>("cut"), true),
        deltaR_(iConfig.getParameter<double>("deltaR")),
        precision_(iConfig.getParameter<int>("precision")),
        debug_(iConfig.getUntrackedParameter<bool>("debug", false)) {
    for (const auto& tag : iConfig.getParameter<std::vector<edm::InputTag>>("weightSrc")) {
      weightsToken_.emplace(tag.instance(), consumes<edm::ValueMap<float>>(tag));
    }
    produces<nanoaod::FlatTable>();
  }

  ~GenJetBFragWeightTableProducer() override{};

private:
  void produce(edm::Event&, edm::EventSetup const&) override;

  std::string name_;
  edm::EDGetTokenT<std::vector<reco::GenJet>> jetsToken_;
  edm::EDGetTokenT<std::vector<reco::GenJet>> jetsWithNuToken_;
  const StringCutObjectSelector<reco::GenJet> cut_;
  const double deltaR_;
  const int precision_;
  const bool debug_;
  std::map<std::string, edm::EDGetTokenT<edm::ValueMap<float>>> weightsToken_;
};

// ------------ method called to produce the data  ------------
void GenJetBFragWeightTableProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  edm::Handle<reco::GenJetCollection> jets;
  iEvent.getByToken(jetsToken_, jets);

  edm::Handle<reco::GenJetCollection> jetsWithNu;
  iEvent.getByToken(jetsWithNuToken_, jetsWithNu);

  std::map<std::string, edm::Handle<edm::ValueMap<float>>> weightHandles;
  std::map<std::string, std::vector<float>> outWeights;

  for (auto& wgt_token: weightsToken_) {
    edm::Handle<edm::ValueMap<float>> handle;
    iEvent.getByToken(wgt_token.second, handle);
    weightHandles.emplace(wgt_token.first, handle);
    outWeights.emplace(wgt_token.first, std::vector<float>());
  }
  if (debug_) {
    outWeights.emplace("matchDR", std::vector<float>());
    outWeights.emplace("matchDpT", std::vector<float>());
    outWeights.emplace("nMatch", std::vector<float>());
  }

  unsigned int ncand = 0;

  for (const reco::GenJet& jet: *jets) {
    if (!cut_(jet)) {
      continue;
    }
    ++ncand;
    std::map<std::string, float> theseWeights;
    for (const auto& wgt: outWeights) {
      theseWeights[wgt.first] = 1.;
    }

    int nMatch = 0;
    int bestJetIdx = -1;
    float bestDR = 999.;

    for (const reco::GenJet& jetWithNu: *jetsWithNu) {
      edm::Ref<std::vector<reco::GenJet>> genJetRef(jetsWithNu, &jetWithNu - &(*jetsWithNu->begin()));

      float matchDR = deltaR(jet.p4(), jetWithNu.p4());
      if (matchDR < deltaR_) {
        nMatch++;
      }

      if (matchDR < bestDR) {
        bestDR = matchDR;
        bestJetIdx = &jetWithNu - &(*jetsWithNu->begin());
      }
    }  // end loop on genJetsWithNu

    if (debug_) {
      theseWeights["matchDR"] = bestDR;
      theseWeights["nMatch"] = nMatch;
    }
    // found a matched genJetWithNu?
    if (bestJetIdx >= 0) {
      const reco::GenJet& bestJet = jetsWithNu->at(bestJetIdx);
      if (debug_) {
        theseWeights["matchDpT"] = bestJet.p4().Pt() - jet.p4().Pt();
      }

      edm::Ref<std::vector<reco::GenJet>> genJetRef(jetsWithNu, bestJetIdx);

      // only store weights if best genJetWithNu is close enough
      if (bestDR < deltaR_) {
        for (const auto& wgt_handle: weightHandles) {
          theseWeights[wgt_handle.first] = (*wgt_handle.second)[genJetRef];
        }
      }
    }

    for (const auto& wgt_table: outWeights) {
      outWeights[wgt_table.first].push_back(theseWeights[wgt_table.first]);
    }
  }  // end loop on genJets

  auto tab = std::make_unique<nanoaod::FlatTable>(ncand, name_, false, true);
  for (const auto& wgt_table: outWeights) {
    tab->addColumn<float>(wgt_table.first,
                          wgt_table.second,
                          "jet fragmentation weight: " + wgt_table.first,
                          nanoaod::FlatTable::FloatColumn,
                          precision_);
  }

  iEvent.put(std::move(tab));
}

#include "FWCore/Framework/interface/MakerMacros.h"
//define this as a plug-in
DEFINE_FWK_MODULE(GenJetBFragWeightTableProducer);
