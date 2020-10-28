#include <iostream>
#include <string>
#include <vector>

#include "TMath.h"
#include "TLorentzVector.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"
#include "ROOT/RDF/RInterface.hxx"

#include "core/generic_utils.hxx"
#include "core/sample_wrapper.hxx"
#include "core/sample_collection.hxx"
#include "core/region_collection.hxx"
#include "core/plot_collection.hxx"
#include "higgsino/higgsino_utils.hxx"

//helper functions

//function that returns true if there is a jet closely aligned with MET
bool low_dphi_met(unsigned int const & nJet, ROOT::VecOps::RVec<float> const & Jet_pt, ROOT::VecOps::RVec<float> const & Jet_eta, ROOT::VecOps::RVec<float> const & Jet_phi, float const & MET_phi) {
  int signal_jet_idx = 0;
  //does not clean jets for lepton candidates => ONLY USE THIS FUNCTION WITH Nl=0 REQUIREMENT
  for (unsigned int jet_idx = 0; jet_idx < nJet; jet_idx++) {
    if (Jet_pt[jet_idx] <= 30) continue;
    if (TMath::Abs(Jet_eta[jet_idx]) > 2.4) continue;
    float dphi_cut = signal_jet_idx <= 1 ? 0.5 : 0.3;
    if (delta_phi(Jet_phi[jet_idx],MET_phi)<dphi_cut) {
      return true;
    }
    signal_jet_idx += 1;
  }
  return false;
}

enum flags {
	mc = 1,
	data = 2
};

int main() {
	ROOT::EnableImplicitMT();
	std::cout << "Initializing." << std::endl;
	SampleWrapper* ttbar = (new SampleWrapper("ttbar",{"/net/cms25/cms25r5/pico/NanoAODv5/nano/2016/mc/TTJets_SingleLeptFromT_TuneCUETP8M1_13TeV-madgraphMLM-pythia8__RunIISummer16NanoAODv5__PUMoriond17_Nano1June2019_102X_mcRun2_asymptotic_v7-v1__100000*.root"},kBlue,"t#bar{t} 1l",false,"Events"))->set_cross_section(TTJets_SingleLept_cross_section);
	//SampleWrapper* ttbar = (new SampleWrapper("ttbar",{"/net/cms25/cms25r5/pico/NanoAODv5/nano/2016/mc/TTJets_SingleLeptFromT_Tune*.root"},kBlue,"t#bar{t} 1l",35.9,false,"Events");
	//SampleWrapper* signal_mchi900 = (new SampleWrapper("ttbar",{"/net/cms25/cms25r5/pico/NanoAODv5/nano/2016/*.root"},kRed,"TChiHH",35.9,false,"Events");
	SampleCollection* samples = new SampleCollection;
	samples->add(ttbar);
	//samples->add(signal_mchi900);

	samples->define("HT_pt",HT_pt,HT_pt_args);
	samples->define("MET_TriggerEff2016",MET_TriggerEff2016,MET_TriggerEff2016_args);
	samples->define("Weight","MET_TriggerEff2016*Generator_weight");
	samples->define("Electron_isInPico",Electron_isInPico,Electron_isInPico_args);
	samples->define("Electron_isVeto",Electron_isVeto,Electron_isVeto_args);
	samples->define("Electron_sig",Electron_sig,Electron_sig_args);
	samples->define("nVetoElectron",nVetoElectron,nVetoElectron_args);
	samples->define("nSigElectron",nSigElectron,nSigElectron_args);
	samples->define("Muon_isInPico",Muon_isInPico,Muon_isInPico_args);
	samples->define("Muon_isVeto",Muon_isVeto,Muon_isVeto_args);
	samples->define("Muon_sig",Muon_sig,Muon_sig_args);
	samples->define("nVetoMuon",nVetoMuon,nVetoMuon_args);
	samples->define("nSigMuon",nSigMuon,nSigMuon_args);
	samples->define("nSigIsoTrack",nSigIsoTrack,nSigIsoTrack_args);
	samples->define("Jet_isLep",Jet_isLep,Jet_isLep_args);
	samples->define("nPicoJet",nPicoJet,nPicoJet_args);
	samples->define("nSigJet",nSigJet,nSigJet_args);
	samples->define("nTightbJet",nTightbJet_2016,nTightbJet_2016_args);

	samples->set_weight_branches("Generator_weight","Weight");
	samples->set_luminosity(35.6);

	//baseline selection
	samples->filter("MET_pt>150","MET>150 GeV");
	samples->filter("nVetoElectron==0&&nVetoMuon==0","nVetoLepton=0");
	samples->filter("nSigJet>=4&&nSigJet<=5","4<=nJet<=5");
	samples->filter("nTightbJet>=2","nTightbJet>=2");
	samples->filter("nSigIsoTrack==0","nSigIsoTrack=0");

	//make cutflow
	std::cout << "Booking histograms and tables." << std::endl;
	TableCollection* cutflow = samples->book_cutflow_table();
	cutflow->print();
	cutflow->save("varstudies_sync_cutflow.tex");

	delete samples;
	delete ttbar;
	//delete signal_mchi900;
	delete cutflow;
	return 0;
}
