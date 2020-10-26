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
#include "higgsino/in_json.hxx"

template <class C>
using RVec = ROOT::VecOps::RVec<C>;

//helper functions
bool idElectron_noIso(int bitmap, int level){
  // decision for each cut represented by 3 bits (0:fail, 1:veto, 2:loose, 3:medium, 4:tight)
  // Electron_vidNestedWPBitmap
  //0 - MinPtCut
  //1 - GsfEleSCEtaMultiRangeCut
  //2 - GsfEleDEtaInSeedCut
  //3 - GsfEleDPhiInCut
  //4 - GsfEleFull5x5SigmaIEtaIEtaCut
  //5 - GsfEleHadronicOverEMEnergyScaledCut
  //6 - GsfEleEInverseMinusPInverseCut
  //7 - GsfEleRelPFIsoScaledCut
  //8 - GsfEleConversionVetoCut
  //9 - GsfEleMissingHitsCut
  bool pass = true;
  // cout<<std::bitset<8*sizeof(bitmap)>(bitmap)<<endl;
  for (int i(0); i<10; i++){
    if (i==7) continue;
    if ( ((bitmap >> i*3) & 0x7) < level) pass = false;
  }
  return pass;
}

bool is_good_track(int pdgid, float pt, float eta, float phi, float reliso_chg, float dxy, float dz, float met, float met_phi){
  
  // re-applying cuts used in NanoAOD so that they would also apply to "tracks" in the lepton collections
  // ((pt>5 && (abs(pdgId) == 11 || abs(pdgId) == 13)) || pt > 10) && 
  // (abs(pdgId) < 15 || abs(eta) < 2.5) && 
  // abs(dxy) < 0.2 && 
  // abs(dz) < 0.1 && 
  // ((pfIsolationDR03().chargedHadronIso < 5 && pt < 25) || pfIsolationDR03().chargedHadronIso/pt < 0.2)
  
  pdgid = TMath::Abs(pdgid);

  if (pdgid!=11 && pdgid!=13 && pdgid!=211) return false; //must be electron, muon, or pion
  
  if (pdgid==11 || pdgid==13) {
    if (pt < 5) {
      return false;
    } else if (pt < 25) { // fail both relative and absolute isolation! (N.B. in this pT range, absolute is always looser...)
      if (reliso_chg >= 0.2 && reliso_chg*pt >= 5) return false; 
    } else {
      if (reliso_chg >= 0.2) return false;
    }
  } else {
    if (pt < 10) {
      return false;
    } else if (pt < 25) {
      if (reliso_chg >= 0.1 && reliso_chg*pt >= 5) return false;
    } else {
      if (reliso_chg >= 0.1) return false;
    }
  }

  if (fabs(eta) > 2.5) return false; // not applied to all tracks in Nano
  if (fabs(dxy)  > 0.2) return false; //applied to tracks but not leptons in Nano
  if (fabs(dz)  > 0.1) return false; //applied to tracks but not leptons in Nano
  if (mt(met,met_phi,pt,phi) > 100) return false; // we should revisit whether this is useful

  return true;
}

std::vector<std::vector<int>> VVRunLumi_2016 = MakeVRunLumi("golden2016");
std::vector<std::vector<int>> VVRunLumi_2018 = MakeVRunLumi("golden2018");

//column definitions
//-----------------------------------------------------------------------
//                             Electrons
//-----------------------------------------------------------------------
RVec<bool> Electron_isInPico(unsigned int const & nElectron, RVec<float> const & Electron_pt, RVec<float> const & Electron_eCorr, RVec<float> const & Electron_eta, RVec<int> const & Electron_vidNestedWPBitmap, RVec<float> const & Electron_dz, RVec<float> const & Electron_dxy) {
  std::vector<bool> in_pico_list;
  for (unsigned int el_idx = 0; el_idx < nElectron; el_idx++) {
    bool is_in_pico = false;
    float el_pt = Electron_pt[el_idx]/Electron_eCorr[el_idx];
    bool is_barrel = abs(Electron_eta[el_idx]) <= 1.479;
    if (el_pt > 10) {
      if (TMath::Abs(Electron_eta[el_idx]) <= 2.5) {
        if (idElectron_noIso(Electron_vidNestedWPBitmap[el_idx], 1)) {
          if ((is_barrel && TMath::Abs(Electron_dz[el_idx])<0.1) || (!is_barrel && TMath::Abs(Electron_dz[el_idx])<0.2)) {
            if ((is_barrel && TMath::Abs(Electron_dxy[el_idx])<0.05) || (!is_barrel && TMath::Abs(Electron_dxy[el_idx])<0.1)) {
	      is_in_pico = true;
	    }
	  }
	}
      }
    }
    in_pico_list.push_back(is_in_pico);
  }
  return in_pico_list;
}
std::vector<std::string> Electron_isInPico_args = {"nElectron","Electron_pt","Electron_eCorr","Electron_eta","Electron_vidNestedWPBitmap","Electron_dz","Electron_dxy"};


RVec<bool> Electron_isVeto(RVec<bool> const & Electron_isInPico, RVec<float> const & Electron_miniPFRelIso_all) {
  std::vector<bool> veto_list;
  for (unsigned int el_idx = 0; el_idx < Electron_isInPico.size(); el_idx++) {
    bool is_veto = false;
    if (Electron_isInPico[el_idx] && Electron_miniPFRelIso_all[el_idx]<0.1) {
      is_veto = true;
    }
    veto_list.push_back(is_veto);
  }
  return veto_list;
}
std::vector<std::string> Electron_isVeto_args = {"Electron_isInPico","Electron_miniPFRelIso_all"};


RVec<bool> Electron_sig(RVec<bool> const & Electron_isVeto, RVec<int> const & Electron_vidNestedWPBitmap, RVec<float> const & Electron_pt, RVec<float> const & Electron_eCorr) {
  std::vector<bool> sig_list;
  for (unsigned int el_idx = 0; el_idx < Electron_isVeto.size(); el_idx++) {
    float el_pt = Electron_pt[el_idx]/Electron_eCorr[el_idx];
    bool is_signal = false;
    if (Electron_isVeto[el_idx] && el_pt>20. && idElectron_noIso(Electron_vidNestedWPBitmap[el_idx],3)) {
      is_signal = true;
    }
    sig_list.push_back(is_signal);
  }
  return sig_list;
}
std::vector<std::string> Electron_sig_args = {"Electron_isVeto","Electron_vidNestedWPBitmap","Electron_pt","Electron_eCorr"};


unsigned int nPicoElectron(RVec<bool> Electron_isInPico) {
  unsigned int r_nPicoElectron = 0;
  for (unsigned int el_idx = 0; el_idx < Electron_isInPico.size(); el_idx++) {
    if (Electron_isInPico[el_idx]) r_nPicoElectron++;
  }
  return r_nPicoElectron;
}
std::vector<std::string> nPicoElectron_args = {"Electron_isInPico"};


unsigned int nVetoElectron(RVec<bool> Electron_isVeto) {
  unsigned int r_nVetoElectron = 0;
  for (unsigned int el_idx = 0; el_idx < Electron_isVeto.size(); el_idx++) {
    if (Electron_isVeto[el_idx]) r_nVetoElectron++;
  }
  return r_nVetoElectron;
}
std::vector<std::string> nVetoElectron_args = {"Electron_isVeto"};


unsigned int nSigElectron(RVec<bool> Electron_sig) {
  unsigned int r_nSigElectron = 0;
  for (unsigned int el_idx = 0; el_idx < Electron_sig.size(); el_idx++) {
    if (Electron_sig[el_idx]) r_nSigElectron++;
  }
  return r_nSigElectron;
}
std::vector<std::string> nSigElectron_args = {"Electron_sig"};


//-----------------------------------------------------------------------
//                             Muons
//-----------------------------------------------------------------------
RVec<bool> Muon_isInPico(unsigned int const & nMuon, RVec<float> const & Muon_pt, RVec<float> const & Muon_eta, RVec<bool> const & Muon_mediumId) {
  std::vector<bool> pico_list;
  for (unsigned int mu_idx = 0; mu_idx < nMuon; mu_idx++) {
    bool is_pico = false;
    if (Muon_pt[mu_idx] > 10.) {
      if (TMath::Abs(Muon_eta[mu_idx]) < 2.4) {
        if (Muon_mediumId[mu_idx]) {
	  is_pico = true;
	}
      }
    }
    pico_list.push_back(is_pico);
  }
  return pico_list;
}
std::vector<std::string> Muon_isInPico_args = {"nMuon","Muon_pt","Muon_eta","Muon_mediumId"};


RVec<bool> Muon_isVeto(RVec<bool> const & Muon_isInPico, RVec<float> const & Muon_dz, RVec<float> const & Muon_dxy, RVec<float> const & Muon_miniPFRelIso_all) {
  std::vector<bool> veto_list;
  for (unsigned int mu_idx = 0; mu_idx < Muon_isInPico.size(); mu_idx++) {
    bool is_veto = false;
    if (Muon_isInPico[mu_idx]) {
      if (TMath::Abs(Muon_dz[mu_idx])<0.5) {
        if (TMath::Abs(Muon_dxy[mu_idx])<0.2) {
          if (Muon_miniPFRelIso_all[mu_idx] < 0.2) {
            is_veto = true;
          }
        }
      }
    }
    veto_list.push_back(is_veto);
  }
  return veto_list;
}
std::vector<std::string> Muon_isVeto_args = {"Muon_isInPico","Muon_dz","Muon_dxy","Muon_miniPFRelIso_all"};


RVec<bool> Muon_sig(RVec<bool> const & Muon_isVeto, RVec<float> const & Muon_pt) {
  std::vector<bool> signal_list;
  for (unsigned int mu_idx = 0; mu_idx < Muon_isVeto.size(); mu_idx++) {
    bool is_signal = false;
    if (Muon_isVeto[mu_idx] && Muon_pt[mu_idx] > 20.) {
      is_signal = true;
    }
    signal_list.push_back(is_signal);
  }
  return signal_list;
}
std::vector<std::string> Muon_sig_args = {"Muon_isVeto","Muon_pt"};


unsigned int nPicoMuon(RVec<bool> Muon_isInPico) {
  unsigned int r_nPicoMuon = 0;
  for (unsigned int mu_idx = 0; mu_idx < Muon_isInPico.size(); mu_idx++) {
    if (Muon_isInPico[mu_idx]) r_nPicoMuon++;
  }
  return r_nPicoMuon;
}
std::vector<std::string> nPicoMuon_args = {"Muon_isInPico"};


unsigned int nVetoMuon(RVec<bool> Muon_isVeto) {
  unsigned int r_nVetoMuon = 0;
  for (unsigned int mu_idx = 0; mu_idx < Muon_isVeto.size(); mu_idx++) {
    if (Muon_isVeto[mu_idx]) r_nVetoMuon++;
  }
  return r_nVetoMuon;
}
std::vector<std::string> nVetoMuon_args = {"Muon_isVeto"};


unsigned int nSigMuon(RVec<bool> Muon_sig) {
  unsigned int r_nSigMuon = 0;
  for (unsigned int mu_idx = 0; mu_idx < Muon_sig.size(); mu_idx++) {
    if (Muon_sig[mu_idx]) r_nSigMuon++;
  }
  return r_nSigMuon;
}
std::vector<std::string> nSigMuon_args = {"Muon_sig"};


//-----------------------------------------------------------------------
//                          Isolated Tracks
//-----------------------------------------------------------------------
unsigned int nSigIsoTrack(float const & MET_pt, float const & MET_phi, RVec<bool> const & IsoTrack_isPFcand, RVec<bool> const & IsoTrack_isFromLostTrack, RVec<int> const & IsoTrack_pdgId, RVec<float> const & IsoTrack_pt, RVec<float> const & IsoTrack_eta, RVec<float> const & IsoTrack_phi, RVec<float> const & IsoTrack_pfRelIso03_chg, RVec<float> const & IsoTrack_dxy, RVec<float> const & IsoTrack_dz, RVec<bool> const & Electron_isPFcand, RVec<bool> const & Electron_sig, RVec<int> const & Electron_pdgId, RVec<float> const & Electron_pt, RVec<float> const & Electron_eta, RVec<float> const & Electron_phi, RVec<float> const & Electron_pfRelIso03_chg, RVec<float> const & Electron_dxy, RVec<float> const & Electron_dz, RVec<bool> const & Muon_isPFcand, RVec<bool> const & Muon_sig, RVec<int> const & Muon_pdgId, RVec<float> const & Muon_pt, RVec<float> const & Muon_eta, RVec<float> const & Muon_phi, RVec<float> const & Muon_pfRelIso03_chg, RVec<float> const & Muon_dxy, RVec<float> const & Muon_dz) {
  unsigned int r_nSigIsoTrack = 0;
  for (unsigned int tk_idx = 0; tk_idx < IsoTrack_pt.size(); tk_idx++) {
    if (!IsoTrack_isPFcand[tk_idx] || IsoTrack_isFromLostTrack[tk_idx]) continue;
    if (is_good_track(IsoTrack_pdgId[tk_idx], IsoTrack_pt[tk_idx], IsoTrack_eta[tk_idx], IsoTrack_phi[tk_idx], IsoTrack_pfRelIso03_chg[tk_idx], IsoTrack_dxy[tk_idx], IsoTrack_dz[tk_idx], MET_pt, MET_phi))
      r_nSigIsoTrack++;
  }
  for (unsigned int el_idx = 0; el_idx < Electron_pt.size(); el_idx++) {
    if (!Electron_isPFcand[el_idx] || Electron_sig[el_idx]) continue;
    if (is_good_track(Electron_pdgId[el_idx], Electron_pt[el_idx], Electron_eta[el_idx], Electron_phi[el_idx], Electron_pfRelIso03_chg[el_idx], Electron_dxy[el_idx], Electron_dz[el_idx], MET_pt, MET_phi))
      r_nSigIsoTrack++;
  }
  for (unsigned int mu_idx = 0; mu_idx < Muon_pt.size(); mu_idx++) {
    if (!Muon_isPFcand[mu_idx] || Muon_sig[mu_idx]) continue;
    if (is_good_track(Muon_pdgId[mu_idx], Muon_pt[mu_idx], Muon_eta[mu_idx], Muon_phi[mu_idx], Muon_pfRelIso03_chg[mu_idx], Muon_dxy[mu_idx], Muon_dz[mu_idx], MET_pt, MET_phi))
      r_nSigIsoTrack++;
  }
  return r_nSigIsoTrack;
}
std::vector<std::string> nSigIsoTk_args = {"MET_pt","MET_phi","IsoTrack_isPFcand","IsoTrack_isFromLostTrack","IsoTrack_pdgId","IsoTrack_pt","IsoTrack_eta","IsoTrack_phi","IsoTrack_pfRelIso03_chg","IsoTrack_dxy","IsoTrack_dz","Electron_isPFcand","Electron_sig","Electron_pdgId","Electron_pt","Electron_eta","Electron_phi","Electron_pfRelIso03_chg","Electron_dxy","Electron_dz","Muon_isPFcand","Muon_sig","Muon_pdgId","Muon_pt","Muon_eta","Muon_phi","Muon_pfRelIso03_chg","Muon_dxy","Muon_dz"};


//-----------------------------------------------------------------------
//                             Jet/MET
//-----------------------------------------------------------------------
RVec<bool> Jet_isLep (RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<float> const & Jet_phi, RVec<bool> const & Electron_sig, RVec<float> const & Electron_pt, RVec<float> const & Electron_eta, RVec<float> const & Electron_phi, RVec<bool> const & Muon_sig, RVec<float> const & Muon_pt, RVec<float> const & Muon_eta, RVec<float> const & Muon_phi) {
  std::vector<bool> is_lep;
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    //remove leptons
    bool found_lep = false;
    for (unsigned int el_idx = 0; el_idx < Electron_sig.size(); el_idx++) {
      if (Electron_sig[el_idx]) {
	float el_pt = Electron_pt[el_idx]; ///Electron_eCorr[el_idx];
	if (delta_r(Electron_eta[el_idx], Electron_phi[el_idx], Jet_eta[jet_idx], Jet_phi[jet_idx])<0.4 && TMath::Abs(Jet_pt[jet_idx]-el_pt)/el_pt < 1.) {
	  found_lep = true;
	  break;
	}
      }
    }
    for (unsigned int mu_idx = 0; mu_idx < Muon_sig.size(); mu_idx++) {
      if (Muon_sig[mu_idx]) { 
	if (delta_r(Muon_eta[mu_idx], Muon_phi[mu_idx], Jet_eta[jet_idx], Jet_phi[jet_idx])<0.4 && TMath::Abs(Jet_pt[jet_idx]-Muon_pt[mu_idx])/Muon_pt[mu_idx] < 1.) {
	  found_lep = true;
	  break;
	}
      }
    }
    is_lep.push_back(found_lep);
  }
  return is_lep;
}
std::vector<std::string> Jet_isLep_args = {"Jet_pt","Jet_eta","Jet_phi","Electron_sig","Electron_pt","Electron_eta","Electron_phi","Muon_sig","Muon_pt","Muon_eta","Muon_phi"};
//std::vector<std::string> Jet_isLep_args = {"Jet_pt","Jet_eta","Jet_phi","Electron_sig","Electron_pt","Electron_eCorr","Electron_eta","Electron_phi","Muon_sig","Muon_pt","Muon_eta","Muon_phi"};


unsigned int nPicoJet(RVec<float> const & Jet_pt) {
  unsigned int r_nPicoJet = 0;
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    if (Jet_pt[jet_idx] > 30.) r_nPicoJet++;
  }
  return r_nPicoJet;
}
std::vector<std::string> nPicoJet_args = {"Jet_pt"};


unsigned int nSigJet(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<bool> const & Jet_isLep) {
  unsigned int r_nSigJet = 0;
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    if (Jet_pt[jet_idx]>30. && TMath::Abs(Jet_eta[jet_idx])<2.4 && !Jet_isLep[jet_idx]) r_nSigJet++;
  }
  return r_nSigJet;
}
std::vector<std::string> nSigJet_args = {"Jet_pt","Jet_eta","Jet_isLep"};


unsigned int nTightbJet_2016(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<bool> const & Jet_isLep, RVec<float> const & Jet_btagDeepB) {
  unsigned int r_nTightbJet = 0;
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    if (Jet_pt[jet_idx]>30. && TMath::Abs(Jet_eta[jet_idx])<2.4 && !Jet_isLep[jet_idx]) {
      if (Jet_btagDeepB[jet_idx] > 0.8953) //2016
        r_nTightbJet++;
    }
  }
  return r_nTightbJet;
}
std::vector<std::string> nTightbJet_2016_args = {"Jet_pt","Jet_eta","Jet_isLep","Jet_btagDeepB"};


unsigned int nTightbJet_2017(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<bool> const & Jet_isLep, RVec<float> const & Jet_btagDeepB) {
  unsigned int r_nTightbJet = 0;
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    if (Jet_pt[jet_idx]>30. && TMath::Abs(Jet_eta[jet_idx])<2.4 && !Jet_isLep[jet_idx]) {
      if (Jet_btagDeepB[jet_idx] > 0.8001) //2017
        r_nTightbJet++;
    }
  }
  return r_nTightbJet;
}
std::vector<std::string> nTightbJet_2017_args = {"Jet_pt","Jet_eta","Jet_isLep","Jet_btagDeepB"};


unsigned int nTightbJet_2018(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<bool> const & Jet_isLep, RVec<float> const & Jet_btagDeepB) {
  unsigned int r_nTightbJet = 0;
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    if (Jet_pt[jet_idx]>30. && TMath::Abs(Jet_eta[jet_idx])<2.4 && !Jet_isLep[jet_idx]) {
      if (Jet_btagDeepB[jet_idx] > 0.7527) //2018
        r_nTightbJet++;
    }
  }
  return r_nTightbJet;
}
std::vector<std::string> nTightbJet_2018_args = {"Jet_pt","Jet_eta","Jet_isLep","Jet_btagDeepB"};


unsigned int nMediumbJet_2016(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<bool> const & Jet_isLep, RVec<float> const & Jet_btagDeepB) {
  unsigned int r_nMediumbJet = 0;
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    if (Jet_pt[jet_idx]>30. && TMath::Abs(Jet_eta[jet_idx])<2.4 && !Jet_isLep[jet_idx]) {
      if (Jet_btagDeepB[jet_idx] > 0.6321) //2016
        r_nMediumbJet++;
    }
  }
  return r_nMediumbJet;
}
std::vector<std::string> nMediumbJet_2016_args = {"Jet_pt","Jet_eta","Jet_isLep","Jet_btagDeepB"};


unsigned int nMediumbJet_2017(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<bool> const & Jet_isLep, RVec<float> const & Jet_btagDeepB) {
  unsigned int r_nMediumbJet = 0;
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    if (Jet_pt[jet_idx]>30. && TMath::Abs(Jet_eta[jet_idx])<2.4 && !Jet_isLep[jet_idx]) {
      if (Jet_btagDeepB[jet_idx] > 0.4941) //2017
        r_nMediumbJet++;
    }
  }
  return r_nMediumbJet;
}
std::vector<std::string> nMediumbJet_2017_args = {"Jet_pt","Jet_eta","Jet_isLep","Jet_btagDeepB"};


unsigned int nMediumbJet_2018(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<bool> const & Jet_isLep, RVec<float> const & Jet_btagDeepB) {
  unsigned int r_nMediumbJet = 0;
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    if (Jet_pt[jet_idx]>30. && TMath::Abs(Jet_eta[jet_idx])<2.4 && !Jet_isLep[jet_idx]) {
      if (Jet_btagDeepB[jet_idx] > 0.4184) //2018
        r_nMediumbJet++;
    }
  }
  return r_nMediumbJet;
}
std::vector<std::string> nMediumbJet_2018_args = {"Jet_pt","Jet_eta","Jet_isLep","Jet_btagDeepB"};


unsigned int nLoosebJet_2016(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<bool> const & Jet_isLep, RVec<float> const & Jet_btagDeepB) {
  unsigned int r_nLoosebJet = 0;
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    if (Jet_pt[jet_idx]>30. && TMath::Abs(Jet_eta[jet_idx])<2.4 && !Jet_isLep[jet_idx]) {
      if (Jet_btagDeepB[jet_idx] > 0.2217) //2016
        r_nLoosebJet++;
    }
  }
  return r_nLoosebJet;
}
std::vector<std::string> nLoosebJet_2016_args = {"Jet_pt","Jet_eta","Jet_isLep","Jet_btagDeepB"};


unsigned int nLoosebJet_2017(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<bool> const & Jet_isLep, RVec<float> const & Jet_btagDeepB) {
  unsigned int r_nLoosebJet = 0;
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    if (Jet_pt[jet_idx]>30. && TMath::Abs(Jet_eta[jet_idx])<2.4 && !Jet_isLep[jet_idx]) {
      if (Jet_btagDeepB[jet_idx] > 0.1522) //2017
        r_nLoosebJet++;
    }
  }
  return r_nLoosebJet;
}
std::vector<std::string> nLoosebJet_2017_args = {"Jet_pt","Jet_eta","Jet_isLep","Jet_btagDeepB"};


unsigned int nLoosebJet_2018(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<bool> const & Jet_isLep, RVec<float> const & Jet_btagDeepB) {
  unsigned int r_nLoosebJet = 0;
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    if (Jet_pt[jet_idx]>30. && TMath::Abs(Jet_eta[jet_idx])<2.4 && !Jet_isLep[jet_idx]) {
      if (Jet_btagDeepB[jet_idx] > 0.0494) //2018
        r_nLoosebJet++;
    }
  }
  return r_nLoosebJet;
}
std::vector<std::string> nLoosebJet_2018_args = {"Jet_pt","Jet_eta","Jet_isLep","Jet_btagDeepB"};


float MHT_pt(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<float> const & Jet_phi, RVec<float> const & Jet_mass) {
  TLorentzVector mht_vec;
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    if (Jet_pt[jet_idx] < 30.) continue;
    TLorentzVector ijet_v4;
    ijet_v4.SetPtEtaPhiM(Jet_pt[jet_idx], Jet_eta[jet_idx], Jet_phi[jet_idx], Jet_mass[jet_idx]);
    mht_vec -= ijet_v4;
  }
  return mht_vec.Pt();
}
std::vector<std::string> MHT_pt_args = {"Jet_pt","Jet_eta","Jet_phi","Jet_mass"};


float MHT_phi(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<float> const & Jet_phi, RVec<float> const & Jet_mass) {
  TLorentzVector mht_vec;
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    if (Jet_pt[jet_idx] < 30.) continue;
    TLorentzVector ijet_v4;
    ijet_v4.SetPtEtaPhiM(Jet_pt[jet_idx], Jet_eta[jet_idx], Jet_phi[jet_idx], Jet_mass[jet_idx]);
    mht_vec -= ijet_v4;
  }
  return mht_vec.Phi();
}
std::vector<std::string> MHT_phi_args = {"Jet_pt","Jet_eta","Jet_phi","Jet_mass"};


float HT_pt(RVec<bool> const & Jet_isLep, RVec<float> const & Jet_pt, RVec<float> const & Jet_eta) {
  float ht = 0.;
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    if (Jet_pt[jet_idx] <= 30.) continue;
    if (Jet_isLep[jet_idx]) continue;
    if (TMath::Abs(Jet_eta[jet_idx])>2.4) continue;
    ht += Jet_pt[jet_idx];
  }
  return ht;
}
std::vector<std::string> HT_pt_args = {"Jet_isLep","Jet_pt","Jet_eta"};


float HT5_pt(RVec<bool> const & Jet_isLep, RVec<float> const & Jet_pt, RVec<float> const & Jet_eta) {
  float ht = 0.;
  //random stuff to avoid unused variable error
  unsigned int temp_eta_size = Jet_eta.size();
  temp_eta_size += 1;
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    if (Jet_pt[jet_idx] <= 30.) continue;
    if (Jet_isLep[jet_idx]) continue;
    //if (TMath::Abs(Jet_eta[jet_idx])>5.0) continue;
    ht += Jet_pt[jet_idx];
  }
  return ht;
}
std::vector<std::string> HT5_pt_args = {"Jet_isLep","Jet_pt","Jet_eta"};


//-----------------------------------------------------------------------
//                             Event Info
//-----------------------------------------------------------------------
bool EventInGoldenJson_2016(unsigned int const & run, unsigned int const & luminosityBlock) {
  return inJSON(VVRunLumi_2016, static_cast<int>(run), static_cast<int>(luminosityBlock));
}
std::vector<std::string> EventInGoldenJson_2016_args = {"run","luminosityBlock"};


bool EventInGoldenJson_2018(unsigned int const & run, unsigned int const & luminosityBlock) {
  return inJSON(VVRunLumi_2018, static_cast<int>(run), static_cast<int>(luminosityBlock));
}
std::vector<std::string> EventInGoldenJson_2018_args = {"run","luminosityBlock"};


bool Flag_EcalNoiseJetFilter(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<float> const & Jet_phi, float const & MET_phi) {
  int counter = 0;
  bool pass_ecalnoisejet;
  bool goodjet[2] = {true, true};
  double dphi = 0.;
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    if (counter >= 2) break;
    float Jet_abs_eta = TMath::Abs(Jet_eta[jet_idx]);
    if (Jet_pt[jet_idx]>30 && Jet_abs_eta > 2.4 && Jet_abs_eta < 5.0) {
      dphi = delta_phi(Jet_phi[jet_idx],MET_phi);
      if (Jet_pt[jet_idx]>250 && (dphi > 2.6 || dphi < 0.1)) goodjet[counter] = false;
      ++counter;
    }
  }
  pass_ecalnoisejet = goodjet[0] && goodjet[1];
  return pass_ecalnoisejet;
}
std::vector<std::string> Flag_EcalNoiseJetFilter_args = {"Jet_pt","Jet_eta","Jet_phi","MET_phi"};


bool Flag_MuonJetFilter(RVec<bool> const & Jet_isLep, RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<float> const & Jet_phi, RVec<float> const & Jet_muEF, float const & MET_phi) {
  bool pass_muonjet = true;
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    if (TMath::Abs(Jet_eta[jet_idx]) > 2.4) continue;
    if (Jet_pt[jet_idx] <= 200.) continue;
    if (Jet_muEF[jet_idx] <= 0.5) continue;
    if (Jet_isLep[jet_idx]) continue;
    if (delta_phi(Jet_phi[jet_idx], MET_phi)<(TMath::Pi()-0.4)) continue;
    pass_muonjet = false;
  }
  return pass_muonjet;
}
std::vector<std::string> Flag_MuonJetFilter_args = {"Jet_isLep","Jet_pt","Jet_eta","Jet_phi","Jet_muEF","MET_phi"};


bool Flag_LowNeutralJetFilter(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<float> const & Jet_phi, RVec<float> const & Jet_neEmEF, float const & MET_phi) {
  bool pass_low_neutral_jet = true;
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    if (Jet_pt[jet_idx] <= 30. || TMath::Abs(Jet_eta[jet_idx])>2.4) continue;
    if (Jet_neEmEF[jet_idx] < 0.03 && delta_phi(Jet_phi[jet_idx], MET_phi)>(TMath::Pi()-0.4))
      pass_low_neutral_jet = false;
    break;
  }
  return pass_low_neutral_jet;
}
std::vector<std::string> Flag_LowNeutralJetFilter_args = {"Jet_pt","Jet_eta","Jet_phi","Jet_neEmEF","MET_phi"};


bool Flag_HTRatioDPhiTightFilter(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<float> const & Jet_phi, float const & MET_phi, float const & HT_pt, float const & HT5_pt) {
  bool pass_htratio_dphi_tight = true;
  float htratio = HT5_pt/HT_pt;
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    if (Jet_pt[jet_idx] < 30. || TMath::Abs(Jet_eta[jet_idx])>2.4) continue;
    if (htratio >= 1.2 && delta_phi(Jet_phi[jet_idx], MET_phi)<(5.3*htratio-4.78))
      pass_htratio_dphi_tight = false;
    break;
  }
  return pass_htratio_dphi_tight;
}
std::vector<std::string> Flag_HTRatioDPhiTightFilter_args = {"Jet_pt","Jet_eta","Jet_phi","MET_phi","HT_pt","HT5_pt"};


bool Flag_HEMDPhiVetoFilter(RVec<bool> const & Electron_isVeto, RVec<float> const & Electron_eta, RVec<float> const & Electron_phi, RVec<float> const & Electron_miniPFRelIso_all, RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<float> const & Jet_phi, float const & MET_phi) {
  //currently, use veto electrons
  for (unsigned int el_idx = 0; el_idx < Electron_eta.size(); el_idx++) {
    if (Electron_isVeto[el_idx] && Electron_miniPFRelIso_all[el_idx] < 0.1 && -3.0 < Electron_eta[el_idx] && Electron_eta[el_idx] < -1.4 && -1.57 < Electron_phi[el_idx] && Electron_phi[el_idx] < -0.87) {
      return false;
    }
  }
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    if (Jet_pt[jet_idx] > 30. && -3.2 < Jet_eta[jet_idx] && Jet_eta[jet_idx] < -1.2 && -1.77 < Jet_phi[jet_idx] && Jet_phi[jet_idx] < -0.67) {
      double dphi = delta_phi(Jet_phi[jet_idx],MET_phi);
      if (dphi < 0.5) {
        return false;
      }
    }
  }
  return true;
}
std::vector<std::string> Flag_HEMDPhiVetoFilter_args = {"Electron_isVeto","Electron_eta","Electron_phi","Electron_miniPFRelIso_all","Jet_pt","Jet_eta","Jet_phi","MET_phi"};


bool Flag_JetID(RVec<float> const & Jet_pt, RVec<int> const & Jet_jetId) {
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    if (Jet_pt[jet_idx] <= 30. && Jet_jetId[jet_idx] < 1)
      return false;
  }
  return true;
}
std::vector<std::string> Flag_JetID_args = {"Jet_pt","Jet_jetId"};


//main function
int main() {
	ROOT::EnableImplicitMT();
	//SampleWrapper *met2018d = (new SampleWrapper("met__run2018d",{"/net/cms25/cms25r5/pico/NanoAODv5/nano/2018/data/MET__Run2018D__Nano1June2019_ver2-v1__30000__*"},kBlack,"MET 2018D",1.0,true,"Events"))->add_flag("2018");
	SampleWrapper *met2016c = (new SampleWrapper("met__run2016c",{"/net/cms25/cms25r5/pico/NanoAODv5/nano/2016/data/MET__Run2016C*"},kBlack,"MET 2016C",1.0,true,"Events"))->add_flag("2016");
	SampleWrapper *met2018d = (new SampleWrapper("met__run2018d",{"/net/cms25/cms25r5/pico/NanoAODv7/nano/2018/data/MET__Run2018D*"},kBlack,"MET 2018D",1.0,true,"Events"))->add_flag("2018");
	SampleCollection* samples = new SampleCollection;
	samples->add(met2018d);

	samples->define("Electron_isInPico",Electron_isInPico,Electron_isInPico_args);
	samples->define("Electron_isVeto",Electron_isVeto,Electron_isVeto_args);
	samples->define("Electron_sig",Electron_sig,Electron_sig_args);
	samples->define("nPicoElectron",nPicoElectron,nPicoElectron_args);
	samples->define("nVetoElectron",nVetoElectron,nVetoElectron_args);
	samples->define("nSigElectron",nSigElectron,nSigElectron_args);
	samples->define("Muon_isInPico",Muon_isInPico,Muon_isInPico_args);
	samples->define("Muon_isVeto",Muon_isVeto,Muon_isVeto_args);
	samples->define("Muon_sig",Muon_sig,Muon_sig_args);
	samples->define("nPicoMuon",nPicoMuon,nPicoMuon_args);
	samples->define("nVetoMuon",nVetoMuon,nVetoMuon_args);
	samples->define("nSigMuon",nSigMuon,nSigMuon_args);
	samples->define("Jet_isLep",Jet_isLep,Jet_isLep_args);
	samples->define("nPicoJet",nPicoJet,nPicoJet_args);
	samples->define("nSigJet",nSigJet,nSigJet_args);
	samples->define("MHT_pt",MHT_pt,MHT_pt_args);
	samples->define("MHT_phi",MHT_phi,MHT_phi_args);
	samples->define("HT_pt",HT_pt,HT_pt_args);
	samples->define("HT5_pt",HT5_pt,HT5_pt_args);
	samples->define("EventInGoldenJson",EventInGoldenJson_2016,EventInGoldenJson_2016_args,{"2016"});
	samples->define("EventInGoldenJson",EventInGoldenJson_2018,EventInGoldenJson_2018_args,{"2018"});
	samples->define("Flag_EcalNoiseJetFilter",Flag_EcalNoiseJetFilter,Flag_EcalNoiseJetFilter_args);
	samples->define("Flag_MuonJetFilter",Flag_MuonJetFilter,Flag_MuonJetFilter_args);
	samples->define("Flag_LowNeutralJetFilter",Flag_LowNeutralJetFilter,Flag_LowNeutralJetFilter_args);
	samples->define("Flag_HTRatioDPhiTightFilter",Flag_HTRatioDPhiTightFilter,Flag_HTRatioDPhiTightFilter_args);
	samples->define("Flag_HEMDPhiVetoFilter",Flag_HEMDPhiVetoFilter,Flag_HEMDPhiVetoFilter_args);
	samples->define("Flag_JetID",Flag_JetID,Flag_JetID_args);

	samples->filter("EventInGoldenJson");
	samples->filter("HLT_PFMET120_PFMHT120_IDTight");
	samples->filter("Flag_goodVertices","GoodVertexFilter");
	samples->filter("Flag_globalSuperTightHalo2016Filter","globalSuperTightHalo2016Filter");
	samples->filter("Flag_HBHENoiseFilter","HBHENoiseFilter");
	samples->filter("Flag_HBHENoiseIsoFilter","HBHENoiseIsoFilter");
	samples->filter("Flag_EcalDeadCellTriggerPrimitiveFilter","EcalDeadCellTriggerPrimitiveFilter");
	samples->filter("Flag_BadPFMuonFilter","BadPFMuonFilter");
	samples->filter("Flag_eeBadScFilter","eeBadScFilter");
	////RA2b filters
	samples->filter("Flag_EcalNoiseJetFilter","EcalNoiseJetFilter",{"2018"});
	samples->filter("Flag_MuonJetFilter","MuonJetFilter");
	samples->filter("MET_pt/CaloMET_pt<2.0","MET/METCalo<2");
	samples->filter("MET_pt/MHT_pt<2.0","MET/MHT<2");
	samples->filter("Flag_LowNeutralJetFilter","LowNeutralJetFilter");
	samples->filter("Flag_HTRatioDPhiTightFilter","HTRatioDphiTightFilter");
	samples->filter("Flag_HEMDPhiVetoFilter","HEMDPhiVetoFilter",{"2018"});
	samples->filter("Flag_JetID","Jet ID");

	std::cout << "Booking histograms and tables." << std::endl;
	TableCollection* cutflow = samples->book_cutflow_table();
	cutflow->print();
	delete samples;
	delete met2018d;
	delete met2016c;
	delete cutflow;
	return 0;
}
