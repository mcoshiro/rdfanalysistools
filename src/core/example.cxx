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

enum Flag {
	mc_1 = 1,
	mc_2 = 2
};

//helper functions
int el_n(unsigned int lep_n, ROOT::VecOps::RVec<int> lep_type) {
	int r_el_n = 0;
	for (unsigned int lep_idx = 0; lep_idx < lep_n; lep_idx++) {
		if (lep_type[lep_idx]==11) {
			r_el_n++;
		}
	}
	return r_el_n;
}

float wcand_mt(unsigned int lep_n, ROOT::VecOps::RVec<unsigned int> lep_type, ROOT::VecOps::RVec<float> lep_pt, ROOT::VecOps::RVec<float> lep_eta, ROOT::VecOps::RVec<float> lep_phi, float met_et, float met_phi) {
	float r_wcand_mt = -999;
	if (lep_n != 3) return r_wcand_mt;
	//find odd lepton out
	int el_n = 0;
	int mu_n = 0;
	std::vector<TLorentzVector> lep_p; 
	for (unsigned int lep_idx = 0; lep_idx < lep_n; lep_idx++) {
		float lep_m = 0;
		if (lep_type[lep_idx]==11) {
			el_n++;
			lep_m = 0.511;
		}
		else {
			mu_n++;
			lep_m = 106.;
		}
		TLorentzVector thislep_p(0,0,0,0);
		thislep_p.SetPtEtaPhiM(lep_pt[lep_idx],lep_eta[lep_idx],lep_phi[lep_idx],lep_m);
		lep_p.push_back(thislep_p);
	}
	if (el_n == 3 || el_n==0) {
		//all leptons are same flavor, must find Z candidate to get W candidate
		float zcand1_m = (lep_p[0]+lep_p[1]).M();
		float zcand2_m = (lep_p[1]+lep_p[2]).M();
		float zcand3_m = (lep_p[2]+lep_p[0]).M();
		if (TMath::Abs(zcand1_m-91000.0) <= TMath::Abs(zcand2_m-91000.0) && TMath::Abs(zcand1_m-91000.0) <= TMath::Abs(zcand3_m-91000.0)) {
			//third lepton is from W
			r_wcand_mt = TMath::Sqrt(2.0*lep_pt[2]*met_et*(1.0-TMath::Cos(lep_phi[2]-met_phi)));
		}
		else if (TMath::Abs(zcand2_m-91000.0) <= TMath::Abs(zcand1_m-91000.0) && TMath::Abs(zcand2_m-91000.0) <= TMath::Abs(zcand3_m-91000.0)) {
			//first lepton is from W
			r_wcand_mt = TMath::Sqrt(2.0*lep_pt[0]*met_et*(1.0-TMath::Cos(lep_phi[0]-met_phi)));
		}
		else {
			//second lepton is from W
			r_wcand_mt = TMath::Sqrt(2.0*lep_pt[1]*met_et*(1.0-TMath::Cos(lep_phi[1]-met_phi)));
		}

	}
	else if (el_n == 2) {
		//one muon, this is W candidate lepton
		for (unsigned int lep_idx = 0; lep_idx < lep_n; lep_idx++) {
			if (lep_type[lep_idx]==13) {
				r_wcand_mt = TMath::Sqrt(2.0*lep_pt[lep_idx]*met_et*(1.0-TMath::Cos(lep_phi[lep_idx]-met_phi)));
			}
		}
	}
	else {
		//one electron, this is W candidate lepton
		for (unsigned int lep_idx = 0; lep_idx < lep_n; lep_idx++) {
			if (lep_type[lep_idx]==11) {
				r_wcand_mt = TMath::Sqrt(2.0*lep_pt[lep_idx]*met_et*(1.0-TMath::Cos(lep_phi[lep_idx]-met_phi)));
			}
		}
	}
	return r_wcand_mt;
}

float zcand_m(unsigned int lep_n, ROOT::VecOps::RVec<unsigned int> lep_type, ROOT::VecOps::RVec<float> lep_pt, ROOT::VecOps::RVec<float> lep_eta, ROOT::VecOps::RVec<float> lep_phi) {
	float r_zcand_m = -999;
	if (lep_n != 3) return r_zcand_m;
	//find odd lepton out
	int el_n = 0;
	int mu_n = 0;
	std::vector<TLorentzVector> lep_p; 
	for (unsigned int lep_idx = 0; lep_idx < lep_n; lep_idx++) {
		float lep_m = 0;
		if (lep_type[lep_idx]==11) {
			el_n++;
			lep_m = 0.511;
		}
		else {
			mu_n++;
			lep_m = 106.;
		}
		TLorentzVector thislep_p(0,0,0,0);
		thislep_p.SetPtEtaPhiM(lep_pt[lep_idx],lep_eta[lep_idx],lep_phi[lep_idx],lep_m);
		lep_p.push_back(thislep_p);
	}
	if (el_n == 3 || el_n==0) {
		//all leptons are same flavor, must find Z candidate to get W candidate
		float zcand1_m = (lep_p[0]+lep_p[1]).M();
		float zcand2_m = (lep_p[1]+lep_p[2]).M();
		float zcand3_m = (lep_p[2]+lep_p[0]).M();
		if (TMath::Abs(zcand1_m-91000.0) <= TMath::Abs(zcand2_m-91000.0) && TMath::Abs(zcand1_m-91000.0) <= TMath::Abs(zcand3_m-91000.0)) {
			//first two leptons form Z
			return zcand1_m;
		}
		else if (TMath::Abs(zcand2_m-91000.0) <= TMath::Abs(zcand1_m-91000.0) && TMath::Abs(zcand2_m-91000.0) <= TMath::Abs(zcand3_m-91000.0)) {
			//latter two leptons form Z
			return zcand2_m;
		}
		else {
			//first and third lepton form Z
			return zcand3_m;
		}

	}
	else if (el_n == 2) {
		//two electrons form Z
		TLorentzVector z_p(0,0,0,0);
		for (unsigned int lep_idx = 0; lep_idx < lep_n; lep_idx++) {
			if (lep_type[lep_idx]==11) {
				z_p += lep_p[lep_idx];
			}
		}
		return z_p.M();
	}
	else {
		//two muons form Z
		TLorentzVector z_p(0,0,0,0);
		for (unsigned int lep_idx = 0; lep_idx < lep_n; lep_idx++) {
			if (lep_type[lep_idx]==13) {
				z_p += lep_p[lep_idx];
			}
		}
		return z_p.M();
	}
	return r_zcand_m;
}

int main() {
	//ROOT::EnableImplicitMT();
	std::cout << "DEBUG: starting" << std::endl;
	SampleWrapper wz1("wz1","/homes/oshiro/trees/mc_105987.WZ.root",kRed,"wz1",false,"mini");
	SampleWrapper wz2("wz2","/homes/oshiro/trees/mc_105987.WZ.root",kBlue,"wz2",false,"mini");
	std::cout << "DEBUG: making SampleCollection" << std::endl;
	SampleCollection samples;
	samples.add(wz1, 0);
	samples.add(wz2, 0);
	//samples.add(wz2, 1);
	std::cout << "DEBUG: adding filter" << std::endl;
	//samples.filter("lep_n==0");
	std::cout << "DEBUG: defining variables" << std::endl;
	samples.define("wcand_mt",wcand_mt,{"lep_n","lep_type","lep_pt","lep_eta","lep_phi","met_et","met_phi"});
	samples.define("zcand_m",zcand_m,{"lep_n","lep_type","lep_pt","lep_eta","lep_phi"});
	std::cout << "DEBUG: making RegionCollection" << std::endl;
	RegionCollection regions;
	regions.add("nl3","lep_n==3","N_{l} = 3");
	std::cout << "DEBUG: adding histograms" << std::endl;
	HistogramCollection w_histogram = samples.book_histogram(regions,60,0,120000,"wcand_mt","mcWeight","W Candidate m_{T} (MeV)");
	HistogramCollection z_histogram = samples.book_histogram(regions,60,0,120000,"zcand_m","mcWeight","Z Candidate m (MeV)");
	w_histogram.set_luminosity(35.9);
	z_histogram.set_luminosity(35.9);
	std::cout << "DEBUG: overlaying histograms" << std::endl;
	w_histogram.overlay_1d_histograms();
	z_histogram.overlay_1d_histograms();
	return 0;
}
