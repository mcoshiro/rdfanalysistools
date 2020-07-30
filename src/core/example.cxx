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

//helper functions
//function that returns the number of electrons
int el_n(unsigned int const &lep_n, ROOT::VecOps::RVec<unsigned int> const &lep_type) {
	int r_el_n = 0;
	for (unsigned int lep_idx = 0; lep_idx < lep_n; lep_idx++) {
		if (lep_type[lep_idx]==11) {
			r_el_n++;
		}
	}
	return r_el_n;
}

//function to get W boson mt in WZ->llln events
float wcand_mt(unsigned int const &lep_n, ROOT::VecOps::RVec<unsigned int> const &lep_type, ROOT::VecOps::RVec<float> const &lep_pt, ROOT::VecOps::RVec<float> const &lep_eta, ROOT::VecOps::RVec<float> const &lep_phi, float const &met_et, float const &met_phi) {
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

//function to get Z boson mass in WZ->llln events
float zcand_m(unsigned int const &lep_n, ROOT::VecOps::RVec<unsigned int> const &lep_type, ROOT::VecOps::RVec<float> const &lep_pt, ROOT::VecOps::RVec<float> const &lep_eta, ROOT::VecOps::RVec<float> const &lep_phi) {
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

//return pt of highest pt electron
float max_el_pt(unsigned int const &lep_n, ROOT::VecOps::RVec<unsigned int> const &lep_type, ROOT::VecOps::RVec<float> const &lep_pt) {
	float r_max_el_pt = -999;
	for (unsigned int lep_idx = 0; lep_idx < lep_n; lep_idx++) {
		if (lep_type[lep_idx] == 11) {
			r_max_el_pt = lep_pt[lep_idx] > r_max_el_pt ? lep_pt[lep_idx] : r_max_el_pt;
		}
	}
	return r_max_el_pt;
}

//return |eta| of highest pt electron
float max_el_abs_eta(unsigned int const &lep_n, ROOT::VecOps::RVec<unsigned int> const &lep_type, ROOT::VecOps::RVec<float> const &lep_pt, ROOT::VecOps::RVec<float> const &lep_eta) {
	float max_el_pt = -999;
	float r_max_el_eta = -999;
	for (unsigned int lep_idx = 0; lep_idx < lep_n; lep_idx++) {
		if (lep_type[lep_idx] == 11) {
			r_max_el_eta = lep_pt[lep_idx] > max_el_pt ? lep_eta[lep_idx] : r_max_el_eta;
			max_el_pt = lep_pt[lep_idx] > max_el_pt ? lep_pt[lep_idx] : max_el_pt;
		}
	}
	return TMath::Abs(r_max_el_eta);
}

//flag enum
enum Flag {
	mc_1 = 1,
	mc_2 = 2,
	data = 3
};

int main() {
	//enable multi-threading
	//ROOT::EnableImplicitMT();
	//defined samples and add to a sample collection
	SampleWrapper wz1("wz1","/homes/oshiro/trees/mc_105987.WZ.root",kRed,"wz1",false,"mini");
	SampleWrapper wz2("wz2","/homes/oshiro/trees/mc_105987.WZ.root",kBlue,"wz2",false,"mini");
	SampleWrapper wz_data("wz_data","/homes/oshiro/trees/mc_105987.WZ.root",kBlack,"wz_data",true,"mini");
	SampleCollection samples;
	samples.add(wz1, mc_1);
	samples.add(wz2, mc_2);
	samples.add(wz_data, data);
	//add a filter and define new columns
	samples.filter("lep_n>0","N_{l}>0");
	samples.define("wcand_mt",wcand_mt,{"lep_n","lep_type","lep_pt","lep_eta","lep_phi","met_et","met_phi"});
	samples.define("zcand_m",zcand_m,{"lep_n","lep_type","lep_pt","lep_eta","lep_phi"});
	//define a region collection in which to make plots
	RegionCollection regions;
	regions.add("nl3","lep_n==3","N_{l} = 3");
	//book histograms and set luminosity to scale MC
	HistogramCollection w_histogram = samples.book_1d_histogram(regions,60,0,120000,"wcand_mt","W Candidate m_{T} (MeV)","mcWeight");
	HistogramCollection z_histogram = samples.book_1d_histogram(regions,60,0,120000,"zcand_m","Z Candidate m (MeV)","mcWeight");
	//generate several different types of plots
	//booked histograms are generated upon calling any of the following methods, so it is good to book everything first
	w_histogram.set_luminosity(0.5);
	z_histogram.set_luminosity(0.5);
	w_histogram.overlay_1d_histograms();
	z_histogram.overlay_1d_histograms();
	w_histogram.stack_1d_histograms();
	z_histogram.stack_1d_histograms();
	w_histogram.stack_ratio_1d_histograms();
	z_histogram.stack_ratio_1d_histograms();

	//make a new SampleCollection for efficiency plots
	std::cout << "Running efficiency plot code." << std::endl;
	SampleWrapper wz3("wz3","/homes/oshiro/trees/mc_105987.WZ.root",kRed,"wz3",false,"mini");
	SampleCollection eff_samples;
	eff_samples.add(wz3, mc_1);
	//define columns and filters
	eff_samples.define("el_n",el_n,{"lep_n","lep_type"});
	eff_samples.define("max_el_pt",max_el_pt,{"lep_n","lep_type","lep_pt"});
	eff_samples.define("max_el_abs_eta",max_el_abs_eta,{"lep_n","lep_type","lep_pt","lep_eta"});
	eff_samples.filter("el_n>1");
	//define inclusive region and book efficiency plot
	RegionCollection region_full;
	region_full.add("full","1","full");
	std::cout << "Booking efficiency plot" << std::endl;
	HistogramCollection hist_el_pt_eff = eff_samples.book_1d_efficiency_plot(region_full,10,10000.,110000.,"max_el_pt","Leading e p_{T}","mcWeight","trigE","El Trigger");
	HistogramCollection hist_el_pt_eta_eff = eff_samples.book_2d_efficiency_plot(region_full,10,10000.,110000.,"max_el_pt","Leading e p_{T}",3,0,2.5,"max_el_abs_eta","Leading e |#eta|","mcWeight","trigE","El Trigger");
	//draw output
	std::cout << "Drawing" << std::endl;
	hist_el_pt_eff.draweach_histograms();
	hist_el_pt_eta_eff.draweach_histograms();
	return 0;
}
