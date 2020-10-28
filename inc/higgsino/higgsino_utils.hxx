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

template <class C>
using RVec = ROOT::VecOps::RVec<C>;


//constants
const float TTJets_SingleLept_cross_section = 178.7;


//nano column definitions
//-----------------------------------------------------------------------
//                             Electrons
//-----------------------------------------------------------------------
RVec<bool> Electron_isInPico(unsigned int const & nElectron, RVec<float> const & Electron_pt, RVec<float> const & Electron_eCorr, RVec<float> const & Electron_eta, RVec<int> const & Electron_vidNestedWPBitmap, RVec<float> const & Electron_dz, RVec<float> const & Electron_dxy);
const std::vector<std::string> Electron_isInPico_args = {"nElectron","Electron_pt","Electron_eCorr","Electron_eta","Electron_vidNestedWPBitmap","Electron_dz","Electron_dxy"};


RVec<bool> Electron_isVeto(RVec<bool> const & Electron_isInPico, RVec<float> const & Electron_miniPFRelIso_all);
const std::vector<std::string> Electron_isVeto_args = {"Electron_isInPico","Electron_miniPFRelIso_all"};


RVec<bool> Electron_sig(RVec<bool> const & Electron_isVeto, RVec<int> const & Electron_vidNestedWPBitmap, RVec<float> const & Electron_pt, RVec<float> const & Electron_eCorr);
const std::vector<std::string> Electron_sig_args = {"Electron_isVeto","Electron_vidNestedWPBitmap","Electron_pt","Electron_eCorr"};


unsigned int nPicoElectron(RVec<bool> Electron_isInPico);
const std::vector<std::string> nPicoElectron_args = {"Electron_isInPico"};


unsigned int nVetoElectron(RVec<bool> Electron_isVeto);
const std::vector<std::string> nVetoElectron_args = {"Electron_isVeto"};


unsigned int nSigElectron(RVec<bool> Electron_sig);
const std::vector<std::string> nSigElectron_args = {"Electron_sig"};


//-----------------------------------------------------------------------
//                             Muons
//-----------------------------------------------------------------------
RVec<bool> Muon_isInPico(unsigned int const & nMuon, RVec<float> const & Muon_pt, RVec<float> const & Muon_eta, RVec<bool> const & Muon_mediumId);
const std::vector<std::string> Muon_isInPico_args = {"nMuon","Muon_pt","Muon_eta","Muon_mediumId"};


RVec<bool> Muon_isVeto(RVec<bool> const & Muon_isInPico, RVec<float> const & Muon_dz, RVec<float> const & Muon_dxy, RVec<float> const & Muon_miniPFRelIso_all);
const std::vector<std::string> Muon_isVeto_args = {"Muon_isInPico","Muon_dz","Muon_dxy","Muon_miniPFRelIso_all"};


RVec<bool> Muon_sig(RVec<bool> const & Muon_isVeto, RVec<float> const & Muon_pt);
const std::vector<std::string> Muon_sig_args = {"Muon_isVeto","Muon_pt"};


unsigned int nPicoMuon(RVec<bool> Muon_isInPico);
const std::vector<std::string> nPicoMuon_args = {"Muon_isInPico"};


unsigned int nVetoMuon(RVec<bool> Muon_isVeto);
const std::vector<std::string> nVetoMuon_args = {"Muon_isVeto"};


unsigned int nSigMuon(RVec<bool> Muon_sig);
const std::vector<std::string> nSigMuon_args = {"Muon_sig"};


//-----------------------------------------------------------------------
//                          Isolated Tracks
//-----------------------------------------------------------------------
unsigned int nSigIsoTrack(float const & MET_pt, float const & MET_phi, RVec<bool> const & IsoTrack_isPFcand, RVec<bool> const & IsoTrack_isFromLostTrack, RVec<int> const & IsoTrack_pdgId, RVec<float> const & IsoTrack_pt, RVec<float> const & IsoTrack_eta, RVec<float> const & IsoTrack_phi, RVec<float> const & IsoTrack_pfRelIso03_chg, RVec<float> const & IsoTrack_dxy, RVec<float> const & IsoTrack_dz, RVec<bool> const & Electron_isPFcand, RVec<bool> const & Electron_sig, RVec<int> const & Electron_pdgId, RVec<float> const & Electron_pt, RVec<float> const & Electron_eta, RVec<float> const & Electron_phi, RVec<float> const & Electron_pfRelIso03_chg, RVec<float> const & Electron_dxy, RVec<float> const & Electron_dz, RVec<bool> const & Muon_isPFcand, RVec<bool> const & Muon_sig, RVec<int> const & Muon_pdgId, RVec<float> const & Muon_pt, RVec<float> const & Muon_eta, RVec<float> const & Muon_phi, RVec<float> const & Muon_pfRelIso03_chg, RVec<float> const & Muon_dxy, RVec<float> const & Muon_dz);
const std::vector<std::string> nSigIsoTrack_args = {"MET_pt","MET_phi","IsoTrack_isPFcand","IsoTrack_isFromLostTrack","IsoTrack_pdgId","IsoTrack_pt","IsoTrack_eta","IsoTrack_phi","IsoTrack_pfRelIso03_chg","IsoTrack_dxy","IsoTrack_dz","Electron_isPFcand","Electron_sig","Electron_pdgId","Electron_pt","Electron_eta","Electron_phi","Electron_pfRelIso03_chg","Electron_dxy","Electron_dz","Muon_isPFcand","Muon_sig","Muon_pdgId","Muon_pt","Muon_eta","Muon_phi","Muon_pfRelIso03_chg","Muon_dxy","Muon_dz"};


//-----------------------------------------------------------------------
//                             Jet/MET
//-----------------------------------------------------------------------
RVec<bool> Jet_isLep (RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<float> const & Jet_phi, RVec<bool> const & Electron_sig, RVec<float> const & Electron_pt, RVec<float> const & Electron_eta, RVec<float> const & Electron_phi, RVec<bool> const & Muon_sig, RVec<float> const & Muon_pt, RVec<float> const & Muon_eta, RVec<float> const & Muon_phi);
const std::vector<std::string> Jet_isLep_args = {"Jet_pt","Jet_eta","Jet_phi","Electron_sig","Electron_pt","Electron_eta","Electron_phi","Muon_sig","Muon_pt","Muon_eta","Muon_phi"};
//const std::vector<std::string> Jet_isLep_args = {"Jet_pt","Jet_eta","Jet_phi","Electron_sig","Electron_pt","Electron_eCorr","Electron_eta","Electron_phi","Muon_sig","Muon_pt","Muon_eta","Muon_phi"};


unsigned int nPicoJet(RVec<float> const & Jet_pt);
const std::vector<std::string> nPicoJet_args = {"Jet_pt"};


unsigned int nSigJet(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<bool> const & Jet_isLep);
const std::vector<std::string> nSigJet_args = {"Jet_pt","Jet_eta","Jet_isLep"};


unsigned int nTightbJet_2016(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<bool> const & Jet_isLep, RVec<float> const & Jet_btagDeepB);
const std::vector<std::string> nTightbJet_2016_args = {"Jet_pt","Jet_eta","Jet_isLep","Jet_btagDeepB"};


unsigned int nTightbJet_2017(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<bool> const & Jet_isLep, RVec<float> const & Jet_btagDeepB);
const std::vector<std::string> nTightbJet_2017_args = {"Jet_pt","Jet_eta","Jet_isLep","Jet_btagDeepB"};


unsigned int nTightbJet_2018(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<bool> const & Jet_isLep, RVec<float> const & Jet_btagDeepB);
const std::vector<std::string> nTightbJet_2018_args = {"Jet_pt","Jet_eta","Jet_isLep","Jet_btagDeepB"};


unsigned int nMediumbJet_2016(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<bool> const & Jet_isLep, RVec<float> const & Jet_btagDeepB);
const std::vector<std::string> nMediumbJet_2016_args = {"Jet_pt","Jet_eta","Jet_isLep","Jet_btagDeepB"};


unsigned int nMediumbJet_2017(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<bool> const & Jet_isLep, RVec<float> const & Jet_btagDeepB);
const std::vector<std::string> nMediumbJet_2017_args = {"Jet_pt","Jet_eta","Jet_isLep","Jet_btagDeepB"};


unsigned int nMediumbJet_2018(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<bool> const & Jet_isLep, RVec<float> const & Jet_btagDeepB);
const std::vector<std::string> nMediumbJet_2018_args = {"Jet_pt","Jet_eta","Jet_isLep","Jet_btagDeepB"};


unsigned int nLoosebJet_2016(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<bool> const & Jet_isLep, RVec<float> const & Jet_btagDeepB);
const std::vector<std::string> nLoosebJet_2016_args = {"Jet_pt","Jet_eta","Jet_isLep","Jet_btagDeepB"};


unsigned int nLoosebJet_2017(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<bool> const & Jet_isLep, RVec<float> const & Jet_btagDeepB);
const std::vector<std::string> nLoosebJet_2017_args = {"Jet_pt","Jet_eta","Jet_isLep","Jet_btagDeepB"};


unsigned int nLoosebJet_2018(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<bool> const & Jet_isLep, RVec<float> const & Jet_btagDeepB);
const std::vector<std::string> nLoosebJet_2018_args = {"Jet_pt","Jet_eta","Jet_isLep","Jet_btagDeepB"};


float MHT_pt(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<float> const & Jet_phi, RVec<float> const & Jet_mass);
const std::vector<std::string> MHT_pt_args = {"Jet_pt","Jet_eta","Jet_phi","Jet_mass"};


float MHT_phi(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<float> const & Jet_phi, RVec<float> const & Jet_mass);
const std::vector<std::string> MHT_phi_args = {"Jet_pt","Jet_eta","Jet_phi","Jet_mass"};


float HT_pt(RVec<bool> const & Jet_isLep, RVec<float> const & Jet_pt, RVec<float> const & Jet_eta);
const std::vector<std::string> HT_pt_args = {"Jet_isLep","Jet_pt","Jet_eta"};


float HT5_pt(RVec<bool> const & Jet_isLep, RVec<float> const & Jet_pt, RVec<float> const & Jet_eta);
const std::vector<std::string> HT5_pt_args = {"Jet_isLep","Jet_pt","Jet_eta"};


//-----------------------------------------------------------------------
//                             Event Info
//-----------------------------------------------------------------------
bool EventInGoldenJson_2016(unsigned int const & run, unsigned int const & luminosityBlock);
const std::vector<std::string> EventInGoldenJson_2016_args = {"run","luminosityBlock"};


bool EventInGoldenJson_2017(unsigned int const & run, unsigned int const & luminosityBlock);
const std::vector<std::string> EventInGoldenJson_2017_args = {"run","luminosityBlock"};


bool EventInGoldenJson_2018(unsigned int const & run, unsigned int const & luminosityBlock);
const std::vector<std::string> EventInGoldenJson_2018_args = {"run","luminosityBlock"};


bool Flag_EcalNoiseJetFilter(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<float> const & Jet_phi, float const & MET_phi);
const std::vector<std::string> Flag_EcalNoiseJetFilter_args = {"Jet_pt","Jet_eta","Jet_phi","MET_phi"};


bool Flag_MuonJetFilter(RVec<bool> const & Jet_isLep, RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<float> const & Jet_phi, RVec<float> const & Jet_muEF, float const & MET_phi);
const std::vector<std::string> Flag_MuonJetFilter_args = {"Jet_isLep","Jet_pt","Jet_eta","Jet_phi","Jet_muEF","MET_phi"};


bool Flag_LowNeutralJetFilter(RVec<float> const & Jet_phi, RVec<float> const & Jet_neEmEF, float const & MET_phi);
const std::vector<std::string> Flag_LowNeutralJetFilter_args = {"Jet_phi","Jet_neEmEF","MET_phi"};


bool Flag_HTRatioDPhiTightFilter(RVec<float> const & Jet_phi, float const & MET_phi, float const & HT_pt, float const & HT5_pt);
const std::vector<std::string> Flag_HTRatioDPhiTightFilter_args = {"Jet_phi","MET_phi","HT_pt","HT5_pt"};


bool Flag_HEMDPhiVetoFilter(RVec<bool> const & Electron_isVeto, RVec<float> const & Electron_eta, RVec<float> const & Electron_phi, RVec<float> const & Electron_miniPFRelIso_all, RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<float> const & Jet_phi, float const & MET_phi);
const std::vector<std::string> Flag_HEMDPhiVetoFilter_args = {"Electron_isVeto","Electron_eta","Electron_phi","Electron_miniPFRelIso_all","Jet_pt","Jet_eta","Jet_phi","MET_phi"};


bool Flag_JetID(RVec<float> const & Jet_pt, RVec<int> const & Jet_jetId);
const std::vector<std::string> Flag_JetID_args = {"Jet_pt","Jet_jetId"};


//-----------------------------------------------------------------------
//                             Trigger
//-----------------------------------------------------------------------
float MET_TriggerEff2016(float const & HT_pt, float const & MET_pt);
const std::vector<std::string> MET_TriggerEff2016_args = {"HT_pt","MET_pt"};
