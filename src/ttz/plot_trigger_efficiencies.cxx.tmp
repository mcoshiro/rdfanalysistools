#include <iostream>
#include <vector>

#include "TMath.h"
#include "TLorentzVector.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"
#include "ROOT/RDF/RInterface.hxx"

#include "../../inc/core/sample_wrapper.hxx"
#include "../../inc/core/sample_collection.hxx"
#include "../../inc/core/region_collection.hxx"
#include "../../inc/core/histogram_collection.hxx"
#include "../../inc/ttz/ttz_pico_utils.hxx"

//helper functions

enum flags {
	mc = 1,
	data = 2
};

int main() {
	ROOT::EnableImplicitMT();
	std::cout << "Setting up and filtering." << std::endl;
	SampleCollection samples;
	samples.add(SampleWrapper("data","/net/cms29/cms29r0/pico/NanoAODv5/ttz_cordellbankv3/2016/data/skim_3l/raw_pico_3l_MET*.root",kBlack,"data",true,"tree"), data);
	//samples.add(SampleWrapper("ttbar","/net/cms29/cms29r0/pico/NanoAODv5/ttz_cordellbankv3/2016/mc/skim_3l/pico_3l_1l_TTJets*.root",kTeal,"t#bar{t}",false,"tree"), mc);
	
	samples.filter("nlep>=1&&(HLT_PFMET110_PFMHT110_IDTight||HLT_PFMETNoMu110_PFMHTNoMu110_IDTight||HLT_PFMET120_PFMHT120_IDTight||HLT_PFMETNoMu120_PFMHTNoMu120_IDTight)","N_{l}#geq 1, MET110 or MET120 Trigger");
	samples.define("lep_pt0","lep_pt[0]");
	samples.define("lep_eta0","TMath::Abs(lep_eta[0])");

	RegionCollection regions;
	regions.add("leadinge","(lep_pdgid[0]==11 || lep_pdgid[0]==-11)","Leading lepton e");
	regions.add("leadingmu","(lep_pdgid[0]==13 || lep_pdgid[0]==-13)","Leading lepton #mu");

	std::cout << "Booking histograms." << std::endl;
	HistogramCollection ptl1 = samples.book_1d_efficiency_plot(regions,14,20,180,"lep_pt0","Leading lepton p_{T}","weight","(HLT_Ele27_WPTight_Gsf||HLT_IsoMu24||HLT_IsoTkMu24)","Isolated Single Lepton Trigger");
	HistogramCollection etal1 = samples.book_1d_efficiency_plot(regions,10,0,2.5,"lep_eta0","Leading lepton |#eta|","weight","(HLT_Ele27_WPTight_Gsf||HLT_IsoMu24||HLT_IsoTkMu24)","Isolated Single Lepton Trigger");
	HistogramCollection ptetal1 = samples.book_2d_efficiency_plot(regions,14,20,180,"lep_pt0","Leading lepton p_{T}",10,0,2.5,"lep_eta0","Leading lepton |#eta|","weight","(HLT_Ele27_WPTight_Gsf||HLT_IsoMu24||HLT_IsoTkMu24)","Isolated Single Lepton Trigger");

	std::cout << "Drawing histograms." << std::endl;
	ptl1.draweach_histograms();
	etal1.draweach_histograms();
	ptetal1.draweach_histograms();
	return 0;
}
