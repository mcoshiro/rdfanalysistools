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

template <class C>
using RVec = ROOT::VecOps::RVec<C>;

//helper functions - not visible outside this file
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

//golden json checker copied from babymaker
std::vector< std::vector<int> > MakeVRunLumi(std::string input){
  std::ifstream orgJSON;
  std::string fullpath = "";
  if(input == "golden2016"){
    fullpath = "data/json/golden_Cert_271036-284044_13TeV_23Sep2016ReReco_Collisions16.json";
  } else if(input == "golden2017"){
    fullpath = "data/json/golden_Cert_294927-306462_13TeV_EOY2017ReReco_Collisions17.json";
  } else if(input == "golden2018"){
    fullpath = "data/json/golden_Cert_314472-325175_13TeV_PromptReco_Collisions18.json";
  } else{
    fullpath = input;
  }
  orgJSON.open(fullpath.c_str());
  std::vector<int> VRunLumi;
  if(orgJSON.is_open()){
    char inChar;
    int inInt;
    std::string str;
    while(!orgJSON.eof()){
      char next = orgJSON.peek();
      if( next == '1' || next == '2' || next == '3' ||
          next == '4' || next == '5' || next == '6' ||
          next == '7' || next == '8' || next == '9' || 
          next == '0'){     
        orgJSON >>inInt;
        VRunLumi.push_back(inInt);        
      }
      else if(next == ' '){
        getline(orgJSON,str,' ');
      }
      else{
        orgJSON>>inChar;
      }
    }
  }//check if the file opened.
  else{
    std::cout<<"Invalid JSON File:"<<fullpath<<"!\n";
  }
  orgJSON.close();
  if(VRunLumi.size() == 0){
    std::cout<<"No Lumiblock found in JSON file\n";
  }
  std::vector< std::vector<int> > VVRunLumi;
  for(unsigned int i = 0; i+2 < VRunLumi.size();){
    if(VRunLumi[i] > 130000){
      std::vector<int> RunLumi;
      RunLumi.push_back(VRunLumi[i]);
      while(VRunLumi[i+1] < 130000 && i+1 < VRunLumi.size()){
        RunLumi.push_back(VRunLumi[i+1]);
        ++i;
      }
      VVRunLumi.push_back(RunLumi);
      ++i;
    }
  }
  return VVRunLumi;
}

bool inJSON(std::vector< std::vector<int> > VVRunLumi, int Run, int LS){
  bool answer = false;
  if(Run < 120000){
    answer = true;
  }
  else{
    for(unsigned int i = 0; i < VVRunLumi.size();++i){
      if(Run == VVRunLumi[i][0]){
        for(unsigned int j = 1; j+1 < VVRunLumi[i].size();j=j+2){
          if(LS >= VVRunLumi[i][j] && LS <= VVRunLumi[i][j+1]){
            answer = true;
          }
        }
      }
    }
  }
  return answer;
}

std::vector<std::vector<int>> VVRunLumi_2016 = MakeVRunLumi("golden2016");
std::vector<std::vector<int>> VVRunLumi_2017 = MakeVRunLumi("golden2017");
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


unsigned int nPicoElectron(RVec<bool> Electron_isInPico) {
  unsigned int r_nPicoElectron = 0;
  for (unsigned int el_idx = 0; el_idx < Electron_isInPico.size(); el_idx++) {
    if (Electron_isInPico[el_idx]) r_nPicoElectron++;
  }
  return r_nPicoElectron;
}


unsigned int nVetoElectron(RVec<bool> Electron_isVeto) {
  unsigned int r_nVetoElectron = 0;
  for (unsigned int el_idx = 0; el_idx < Electron_isVeto.size(); el_idx++) {
    if (Electron_isVeto[el_idx]) r_nVetoElectron++;
  }
  return r_nVetoElectron;
}


unsigned int nSigElectron(RVec<bool> Electron_sig) {
  unsigned int r_nSigElectron = 0;
  for (unsigned int el_idx = 0; el_idx < Electron_sig.size(); el_idx++) {
    if (Electron_sig[el_idx]) r_nSigElectron++;
  }
  return r_nSigElectron;
}


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


unsigned int nPicoMuon(RVec<bool> Muon_isInPico) {
  unsigned int r_nPicoMuon = 0;
  for (unsigned int mu_idx = 0; mu_idx < Muon_isInPico.size(); mu_idx++) {
    if (Muon_isInPico[mu_idx]) r_nPicoMuon++;
  }
  return r_nPicoMuon;
}


unsigned int nVetoMuon(RVec<bool> Muon_isVeto) {
  unsigned int r_nVetoMuon = 0;
  for (unsigned int mu_idx = 0; mu_idx < Muon_isVeto.size(); mu_idx++) {
    if (Muon_isVeto[mu_idx]) r_nVetoMuon++;
  }
  return r_nVetoMuon;
}


unsigned int nSigMuon(RVec<bool> Muon_sig) {
  unsigned int r_nSigMuon = 0;
  for (unsigned int mu_idx = 0; mu_idx < Muon_sig.size(); mu_idx++) {
    if (Muon_sig[mu_idx]) r_nSigMuon++;
  }
  return r_nSigMuon;
}


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


unsigned int nPicoJet(RVec<float> const & Jet_pt) {
  unsigned int r_nPicoJet = 0;
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    if (Jet_pt[jet_idx] > 30.) r_nPicoJet++;
  }
  return r_nPicoJet;
}


unsigned int nSigJet(RVec<float> const & Jet_pt, RVec<float> const & Jet_eta, RVec<bool> const & Jet_isLep) {
  unsigned int r_nSigJet = 0;
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    if (Jet_pt[jet_idx]>30. && TMath::Abs(Jet_eta[jet_idx])<2.4 && !Jet_isLep[jet_idx]) r_nSigJet++;
  }
  return r_nSigJet;
}


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


//-----------------------------------------------------------------------
//                             Event Info
//-----------------------------------------------------------------------
bool EventInGoldenJson_2016(unsigned int const & run, unsigned int const & luminosityBlock) {
  return inJSON(VVRunLumi_2016, static_cast<int>(run), static_cast<int>(luminosityBlock));
}


bool EventInGoldenJson_2017(unsigned int const & run, unsigned int const & luminosityBlock) {
  return inJSON(VVRunLumi_2017, static_cast<int>(run), static_cast<int>(luminosityBlock));
}


bool EventInGoldenJson_2018(unsigned int const & run, unsigned int const & luminosityBlock) {
  return inJSON(VVRunLumi_2018, static_cast<int>(run), static_cast<int>(luminosityBlock));
}


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


bool Flag_LowNeutralJetFilter(RVec<float> const & Jet_phi, RVec<float> const & Jet_neEmEF, float const & MET_phi) {
  bool pass_low_neutral_jet = true;
  for (unsigned int jet_idx = 0; jet_idx < Jet_phi.size(); jet_idx++) {
    //currently, no pt or eta cuts
    //if (Jet_pt[jet_idx] <= 30. || TMath::Abs(Jet_eta[jet_idx])>2.4) continue;
    if (Jet_neEmEF[jet_idx] < 0.03 && delta_phi(Jet_phi[jet_idx], MET_phi)>(TMath::Pi()-0.4))
      pass_low_neutral_jet = false;
    break;
  }
  return pass_low_neutral_jet;
}


bool Flag_HTRatioDPhiTightFilter(RVec<float> const & Jet_phi, float const & MET_phi, float const & HT_pt, float const & HT5_pt) {
  bool pass_htratio_dphi_tight = true;
  float htratio = HT5_pt/HT_pt;
  for (unsigned int jet_idx = 0; jet_idx < Jet_phi.size(); jet_idx++) {
    //currently, no pt or eta cuts
    //if (Jet_pt[jet_idx] < 30. || TMath::Abs(Jet_eta[jet_idx])>2.4) continue;
    if (htratio >= 1.2 && delta_phi(Jet_phi[jet_idx], MET_phi)<(5.3*htratio-4.78))
      pass_htratio_dphi_tight = false;
    break;
  }
  return pass_htratio_dphi_tight;
}


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


bool Flag_JetID(RVec<float> const & Jet_pt, RVec<int> const & Jet_jetId) {
  for (unsigned int jet_idx = 0; jet_idx < Jet_pt.size(); jet_idx++) {
    if (Jet_pt[jet_idx] <= 30. && Jet_jetId[jet_idx] < 1)
      return false;
  }
  return true;
}


//-----------------------------------------------------------------------
//                             Trigger
//-----------------------------------------------------------------------
float MET_TriggerEff2016(float const & HT_pt, float const & MET_pt) {
  float errup=0., errdown=0.; // Not used, but for reference
  float eff = 1., met = MET_pt, ht = HT_pt;
  errup+=errdown; //suppress unused warning
  if (ht> 0 && ht<= 200 && met> 150 && met<= 155) {eff = 0.421454; errup = 0.0122918; errdown = 0.0121974;}
  else if (ht> 200 && ht<= 600 && met> 150 && met<= 155) {eff = 0.511723; errup = 0.00553945; errdown = 0.0055423;}
  else if (ht> 600 && ht<= 800 && met> 150 && met<= 155) {eff = 0.524272; errup = 0.0229051; errdown = 0.0230034;}
  else if (ht> 800 && ht<= 1000 && met> 150 && met<= 155) {eff = 0.502959; errup = 0.0412691; errdown = 0.0413067;}
  else if (ht> 1000 && ht<= 9999 && met> 150 && met<= 155) {eff = 0.575342; errup = 0.0630717; errdown = 0.0653595;}
  else if (ht> 0 && ht<= 200 && met> 155 && met<= 160) {eff = 0.478716; errup = 0.0135556; errdown = 0.0135252;}
  else if (ht> 200 && ht<= 600 && met> 155 && met<= 160) {eff = 0.58184; errup = 0.00573815; errdown = 0.00576013;}
  else if (ht> 600 && ht<= 800 && met> 155 && met<= 160) {eff = 0.523909; errup = 0.0237337; errdown = 0.0238375;}
  else if (ht> 800 && ht<= 1000 && met> 155 && met<= 160) {eff = 0.564706; errup = 0.0404303; errdown = 0.0412473;}
  else if (ht> 1000 && ht<= 9999 && met> 155 && met<= 160) {eff = 0.5; errup = 0.0653624; errdown = 0.0653624;}
  else if (ht> 0 && ht<= 200 && met> 160 && met<= 165) {eff = 0.537445; errup = 0.0151993; errdown = 0.0152672;}
  else if (ht> 200 && ht<= 600 && met> 160 && met<= 165) {eff = 0.641288; errup = 0.00593655; errdown = 0.00597961;}
  else if (ht> 600 && ht<= 800 && met> 160 && met<= 165) {eff = 0.629712; errup = 0.023519; errdown = 0.0241214;}
  else if (ht> 800 && ht<= 1000 && met> 160 && met<= 165) {eff = 0.580645; errup = 0.0474312; errdown = 0.0488429;}
  else if (ht> 1000 && ht<= 9999 && met> 160 && met<= 165) {eff = 0.682927; errup = 0.0786376; errdown = 0.0888868;}
  else if (ht> 0 && ht<= 200 && met> 165 && met<= 170) {eff = 0.556346; errup = 0.0176245; errdown = 0.0177627;}
  else if (ht> 200 && ht<= 600 && met> 165 && met<= 170) {eff = 0.699494; errup = 0.00600351; errdown = 0.00607174;}
  else if (ht> 600 && ht<= 800 && met> 165 && met<= 170) {eff = 0.699752; errup = 0.0235238; errdown = 0.0245674;}
  else if (ht> 800 && ht<= 1000 && met> 165 && met<= 170) {eff = 0.59854; errup = 0.0445755; errdown = 0.0461321;}
  else if (ht> 1000 && ht<= 9999 && met> 165 && met<= 170) {eff = 0.680851; errup = 0.0733593; errdown = 0.0821322;}
  else if (ht> 0 && ht<= 200 && met> 170 && met<= 175) {eff = 0.638268; errup = 0.0184422; errdown = 0.0188431;}
  else if (ht> 200 && ht<= 600 && met> 170 && met<= 175) {eff = 0.749666; errup = 0.00603513; errdown = 0.00613206;}
  else if (ht> 600 && ht<= 800 && met> 170 && met<= 175) {eff = 0.733918; errup = 0.0246003; errdown = 0.0260493;}
  else if (ht> 800 && ht<= 1000 && met> 170 && met<= 175) {eff = 0.72807; errup = 0.0436702; errdown = 0.0480675;}
  else if (ht> 1000 && ht<= 9999 && met> 170 && met<= 175) {eff = 0.574468; errup = 0.0799773; errdown = 0.083562;}
  else if (ht> 0 && ht<= 200 && met> 175 && met<= 180) {eff = 0.654701; errup = 0.02022; errdown = 0.0207713;}
  else if (ht> 200 && ht<= 600 && met> 175 && met<= 180) {eff = 0.784363; errup = 0.00597389; errdown = 0.00609421;}
  else if (ht> 600 && ht<= 800 && met> 175 && met<= 180) {eff = 0.75641; errup = 0.0249947; errdown = 0.0267432;}
  else if (ht> 800 && ht<= 1000 && met> 175 && met<= 180) {eff = 0.734513; errup = 0.0434929; errdown = 0.0480594;}
  else if (ht> 1000 && ht<= 9999 && met> 175 && met<= 180) {eff = 0.634615; errup = 0.0726695; errdown = 0.0785167;}
  else if (ht> 0 && ht<= 200 && met> 180 && met<= 185) {eff = 0.741722; errup = 0.0210861; errdown = 0.022209;}
  else if (ht> 200 && ht<= 600 && met> 180 && met<= 185) {eff = 0.822244; errup = 0.00596603; errdown = 0.00612396;}
  else if (ht> 600 && ht<= 800 && met> 180 && met<= 185) {eff = 0.819672; errup = 0.0225045; errdown = 0.0247522;}
  else if (ht> 800 && ht<= 1000 && met> 180 && met<= 185) {eff = 0.798319; errup = 0.0380713; errdown = 0.0436274;}
  else if (ht> 1000 && ht<= 9999 && met> 180 && met<= 185) {eff = 0.702703; errup = 0.0810427; errdown = 0.0937327;}
  else if (ht> 0 && ht<= 200 && met> 185 && met<= 190) {eff = 0.645533; errup = 0.0266391; errdown = 0.0275234;}
  else if (ht> 200 && ht<= 600 && met> 185 && met<= 190) {eff = 0.853995; errup = 0.00578957; errdown = 0.00598182;}
  else if (ht> 600 && ht<= 800 && met> 185 && met<= 190) {eff = 0.832753; errup = 0.0224984; errdown = 0.0249952;}
  else if (ht> 800 && ht<= 1000 && met> 185 && met<= 190) {eff = 0.826531; errup = 0.0394452; errdown = 0.0469475;}
  else if (ht> 1000 && ht<= 9999 && met> 185 && met<= 190) {eff = 0.771429; errup = 0.0749545; errdown = 0.0932188;}
  else if (ht> 0 && ht<= 200 && met> 190 && met<= 195) {eff = 0.714286; errup = 0.0269016; errdown = 0.0284131;}
  else if (ht> 200 && ht<= 600 && met> 190 && met<= 195) {eff = 0.874115; errup = 0.00585518; errdown = 0.00609152;}
  else if (ht> 600 && ht<= 800 && met> 190 && met<= 195) {eff = 0.849817; errup = 0.022045; errdown = 0.0248186;}
  else if (ht> 800 && ht<= 1000 && met> 190 && met<= 195) {eff = 0.818182; errup = 0.0425143; errdown = 0.0506834;}
  else if (ht> 1000 && ht<= 9999 && met> 190 && met<= 195) {eff = 0.880952; errup = 0.0504413; errdown = 0.0725148;}
  else if (ht> 0 && ht<= 200 && met> 195 && met<= 200) {eff = 0.75; errup = 0.0307629; errdown = 0.0333008;}
  else if (ht> 200 && ht<= 600 && met> 195 && met<= 200) {eff = 0.903775; errup = 0.00541855; errdown = 0.00569667;}
  else if (ht> 600 && ht<= 800 && met> 195 && met<= 200) {eff = 0.876068; errup = 0.0218991; errdown = 0.0254228;}
  else if (ht> 800 && ht<= 1000 && met> 195 && met<= 200) {eff = 0.835443; errup = 0.042987; errdown = 0.0526851;}
  else if (ht> 1000 && ht<= 9999 && met> 195 && met<= 200) {eff = 0.870968; errup = 0.0607061; errdown = 0.0903257;}
  else if (ht> 0 && ht<= 200 && met> 200 && met<= 210) {eff = 0.794118; errup = 0.0236918; errdown = 0.0257459;}
  else if (ht> 200 && ht<= 600 && met> 200 && met<= 210) {eff = 0.926773; errup = 0.00363813; errdown = 0.00380788;}
  else if (ht> 600 && ht<= 800 && met> 200 && met<= 210) {eff = 0.915074; errup = 0.0129577; errdown = 0.0148636;}
  else if (ht> 800 && ht<= 1000 && met> 200 && met<= 210) {eff = 0.9; errup = 0.0232988; errdown = 0.0285863;}
  else if (ht> 1000 && ht<= 9999 && met> 200 && met<= 210) {eff = 0.830769; errup = 0.0480266; errdown = 0.0597643;}
  else if (ht> 0 && ht<= 200 && met> 210 && met<= 220) {eff = 0.806122; errup = 0.0290347; errdown = 0.0324296;}
  else if (ht> 200 && ht<= 600 && met> 210 && met<= 220) {eff = 0.955567; errup = 0.00326322; errdown = 0.00349935;}
  else if (ht> 600 && ht<= 800 && met> 210 && met<= 220) {eff = 0.945026; errup = 0.0116963; errdown = 0.0142838;}
  else if (ht> 800 && ht<= 1000 && met> 210 && met<= 220) {eff = 0.938356; errup = 0.0198781; errdown = 0.0268833;}
  else if (ht> 1000 && ht<= 9999 && met> 210 && met<= 220) {eff = 0.941176; errup = 0.0318434; errdown = 0.0539242;}
  else if (ht> 0 && ht<= 200 && met> 220 && met<= 230) {eff = 0.860656; errup = 0.0320534; errdown = 0.0387031;}
  else if (ht> 200 && ht<= 600 && met> 220 && met<= 230) {eff = 0.96675; errup = 0.00316544; errdown = 0.00346976;}
  else if (ht> 600 && ht<= 800 && met> 220 && met<= 230) {eff = 0.967262; errup = 0.00964044; errdown = 0.0128421;}
  else if (ht> 800 && ht<= 1000 && met> 220 && met<= 230) {eff = 0.949153; errup = 0.0199859; errdown = 0.0291361;}
  else if (ht> 1000 && ht<= 9999 && met> 220 && met<= 230) {eff = 0.934426; errup = 0.0311194; errdown = 0.0488167;}
  else if (ht> 0 && ht<= 200 && met> 230 && met<= 240) {eff = 0.844444; errup = 0.0392546; errdown = 0.0479576;}
  else if (ht> 200 && ht<= 600 && met> 230 && met<= 240) {eff = 0.976041; errup = 0.00303115; errdown = 0.00342929;}
  else if (ht> 600 && ht<= 800 && met> 230 && met<= 240) {eff = 0.964968; errup = 0.0103099; errdown = 0.0137191;}
  else if (ht> 800 && ht<= 1000 && met> 230 && met<= 240) {eff = 0.953488; errup = 0.0221284; errdown = 0.0352492;}
  else if (ht> 1000 && ht<= 9999 && met> 230 && met<= 240) {eff = 1; errup = 0; errdown = 0.0449824;}
  else if (ht> 0 && ht<= 200 && met> 240 && met<= 250) {eff = 0.857143; errup = 0.0511428; errdown = 0.0684449;}
  else if (ht> 200 && ht<= 600 && met> 240 && met<= 250) {eff = 0.982009; errup = 0.00296575; errdown = 0.00348796;}
  else if (ht> 600 && ht<= 800 && met> 240 && met<= 250) {eff = 0.978102; errup = 0.00865233; errdown = 0.0128502;}
  else if (ht> 800 && ht<= 1000 && met> 240 && met<= 250) {eff = 1; errup = 0; errdown = 0.0180628;}
  else if (ht> 1000 && ht<= 9999 && met> 240 && met<= 250) {eff = 0.964286; errup = 0.0295635; errdown = 0.077387;}
  else if (ht> 0 && ht<= 200 && met> 250 && met<= 275) {eff = 0.873239; errup = 0.0402311; errdown = 0.0525007;}
  else if (ht> 200 && ht<= 600 && met> 250 && met<= 275) {eff = 0.987697; errup = 0.00186062; errdown = 0.00216111;}
  else if (ht> 600 && ht<= 800 && met> 250 && met<= 275) {eff = 0.986667; errup = 0.00490435; errdown = 0.0071078;}
  else if (ht> 800 && ht<= 1000 && met> 250 && met<= 275) {eff = 0.995238; errup = 0.00393961; errdown = 0.0108643;}
  else if (ht> 1000 && ht<= 9999 && met> 250 && met<= 275) {eff = 1; errup = 0; errdown = 0.0239329;}
  else if (ht> 0 && ht<= 200 && met> 275 && met<= 300) {eff = 0.870968; errup = 0.0607061; errdown = 0.0903257;}
  else if (ht> 200 && ht<= 600 && met> 275 && met<= 300) {eff = 0.992795; errup = 0.00183562; errdown = 0.00237029;}
  else if (ht> 600 && ht<= 800 && met> 275 && met<= 300) {eff = 0.99505; errup = 0.00319693; errdown = 0.00649192;}
  else if (ht> 800 && ht<= 1000 && met> 275 && met<= 300) {eff = 0.984848; errup = 0.0097805; errdown = 0.0196341;}
  else if (ht> 1000 && ht<= 9999 && met> 275 && met<= 300) {eff = 0.983871; errup = 0.0133466; errdown = 0.0361115;}
  else if (ht> 0 && ht<= 200 && met> 300 && met<= 9999) {eff = 0.846154; errup = 0.0721243; errdown = 0.105033;}
  else if (ht> 200 && ht<= 600 && met> 300 && met<= 9999) {eff = 0.995964; errup = 0.00139558; errdown = 0.00198449;}
  else if (ht> 600 && ht<= 800 && met> 300 && met<= 9999) {eff = 0.998106; errup = 0.00156681; errdown = 0.00434157;}
  else if (ht> 800 && ht<= 1000 && met> 300 && met<= 9999) {eff = 0.983784; errup = 0.00881271; errdown = 0.0155222;}
  else if (ht> 1000 && ht<= 9999 && met> 300 && met<= 9999) {eff = 0.987179; errup = 0.0106082; errdown = 0.0288622;}
  return eff;
}
