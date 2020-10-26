#include <iostream>
#include <vector>

#include "TMath.h"
#include "TLorentzVector.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"
#include "ROOT/RDF/RInterface.hxx"

//#include "core/column_definition.hxx"
#include "core/variable_axis.hxx"
#include "core/sample_wrapper.hxx"
#include "core/sample_collection.hxx"
#include "core/region_collection.hxx"
#include "core/plot_collection.hxx"

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
//ColumnDefinition<int*(unsigned int const&, ROOT::VecOps::RVec<unsigned int>const &)> el_n("el_n",&fn_el_n,{"lep_n","lep_type"});
std::vector<std::string> el_n_args = {"lep_n","lep_type"};

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
//ColumnDefinition<float*(unsigned int const &, ROOT::VecOps::RVec<unsigned int> const &, ROOT::VecOps::RVec<float> const &, ROOT::VecOps::RVec<float> const &, ROOT::VecOps::RVec<float> const &, float const &, float const &) wcand_mt("wcand_mt",&fn_wcand_mt,{"lep_n","lep_type","lep_pt","lep_eta","lep_phi","met_et","met_phi"});
std::vector<std::string> wcand_mt_args = {"lep_n","lep_type","lep_pt","lep_eta","lep_phi","met_et","met_phi"};

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
//ColumnDefinition<float*(unsigned int const &, ROOT::VecOps::RVec<unsigned int> const &, ROOT::VecOps::RVec<float> const &, ROOT::VecOps::RVec<float> const &, ROOT::VecOps::RVec<float> const &) zcand_m("zcand_m",&fn_zcand_m,{"lep_n","lep_type","lep_pt","lep_eta","lep_phi"});
std::vector<std::string> zcand_m_args = {"lep_n","lep_type","lep_pt","lep_eta","lep_phi"};


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
//ColumnDefinition<float*(unsigned int const &, ROOT::VecOps::RVec<unsigned int> const &, ROOT::VecOps::RVec<float> const &)> max_el_pt("max_el_pt",&fn_max_el_pt,{"lep_n","lep_type","lep_pt"});
std::vector<std::string> max_el_pt_args = {"lep_n","lep_type","lep_pt"};


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
//ColumnDefinition<float*(unsigned int const &, ROOT::VecOps::RVec<unsigned int> const &, ROOT::VecOps::RVec<float> const &, ROOT::VecOps::RVec<float> const &)> max_el_abs_eta("max_el_abs_eta",&fn_max_el_abs_eta,{"lep_n","lep_type","lep_pt","lep_eta"});
std::vector<std::string> max_el_abs_eta_args = {"lep_n","lep_type","lep_pt","lep_eta"};

int main() {
	//enable multi-threading
	//ROOT::EnableImplicitMT();
	//defined samples and add to a sample collection
	std::cout << "Initializing samples" << std::endl;
	SampleWrapper *wz1 = new SampleWrapper("wz1",{"/homes/oshiro/trees/mc_105987.WZ.root"},kRed,"wz1",35.6,false,"mini");
	SampleWrapper *wz2 = new SampleWrapper("wz2",{"/homes/oshiro/trees/mc_105987.WZ.root"},kBlue,"wz2",35.6,false,"mini");
	SampleWrapper *wz_data = new SampleWrapper("wz_data",{"/homes/oshiro/trees/mc_105987.WZ.root"},kBlack,"wz_data",35.6,true,"mini");
	SampleCollection *samples = new SampleCollection();
	samples->add(wz1);
	samples->add(wz2);
	samples->add(wz_data);
	//add a filter and define new columns
	std::cout << "Defining variables and adding filters" << std::endl;
	samples->filter("lep_n>0","N_{l}>0");
	//samples.define("wcand_mt",wcand_mt,{"lep_n","lep_type","lep_pt","lep_eta","lep_phi","met_et","met_phi"});
	samples->define("wcand_mt",wcand_mt,wcand_mt_args);
	//samples.define("zcand_m",zcand_m,{"lep_n","lep_type","lep_pt","lep_eta","lep_phi"});
	samples->define("zcand_m",zcand_m,zcand_m_args);
	//define a region collection in which to make plots
	RegionCollection * regions = new RegionCollection();
	regions->add("nl3","lep_n==3","N_{l} = 3");
	std::cout << "Booking plots and tables" << std::endl;
	//book histograms and set luminosity to scale MC
	PlotCollection* w_histogram = samples->book_1d_histogram(VariableAxis("wcand_mt","W Candidate m_{T}",60,0,120000,"MeV"),"mcWeight",regions);
	PlotCollection* z_histogram = samples->book_1d_histogram(VariableAxis("zcand_m","Z Candidate m",60,0,120000,"MeV"),"mcWeight",regions);
	//generate several different types of plots
	//generate cutflow
	samples->filter("lep_n>=3","N_{l} #geq 3");
	samples->filter("zcand_m>81000&&zcand_m<101000","Z mass cut");
	TableCollection* cutflow = samples->book_cutflow_table();
	//booked histograms are generated upon calling any of the following methods, so it is good to book everything first
	std::cout << "Drawing plots and tables" << std::endl;
	w_histogram->set_luminosity(0.5);
	z_histogram->set_luminosity(0.5);
	w_histogram->draw_together();
	z_histogram->draw_together();
	w_histogram->set_plot_combine_style(PlotCombineStyle::stack)->draw_together();
	z_histogram->set_plot_combine_style(PlotCombineStyle::stack)->draw_together();
	w_histogram->set_bottom_style(BottomStyle::ratio)->draw_together();
	z_histogram->set_bottom_style(BottomStyle::ratio)->draw_together();
	cutflow->print();
	delete wz1;
        delete wz2;
	delete wz_data;
	delete regions;
	delete samples;
	delete w_histogram;
	delete z_histogram;
	delete cutflow;

	////make a new SampleCollection for efficiency plots
	//std::cout << "Running efficiency plot code." << std::endl;
	//SampleWrapper wz3("wz3","/homes/oshiro/trees/mc_105987.WZ.root",kRed,"wz3",false,"mini");
	//SampleCollection eff_samples;
	//eff_samples.add(wz3, mc_1);
	////define columns and filters
	//eff_samples.define("el_n",el_n,{"lep_n","lep_type"});
	//eff_samples.define("max_el_pt",max_el_pt,{"lep_n","lep_type","lep_pt"});
	//eff_samples.define("max_el_abs_eta",max_el_abs_eta,{"lep_n","lep_type","lep_pt","lep_eta"});
	//eff_samples.filter("el_n>1");
	////define inclusive region and book efficiency plot
	//RegionCollection region_full;
	//region_full.add("full","1","full");
	//std::cout << "Booking efficiency plot" << std::endl;
	//HistogramCollection hist_el_pt_eff = eff_samples.book_1d_efficiency_plot(region_full,10,10000.,110000.,"max_el_pt","Leading e p_{T}","mcWeight","trigE","El Trigger");
	//HistogramCollection hist_el_pt_eta_eff = eff_samples.book_2d_efficiency_plot(region_full,10,10000.,110000.,"max_el_pt","Leading e p_{T}",3,0,2.5,"max_el_abs_eta","Leading e |#eta|","mcWeight","trigE","El Trigger");
	////draw output
	//std::cout << "Drawing" << std::endl;
	//hist_el_pt_eff.draweach_histograms();
	//hist_el_pt_eta_eff.draweach_histograms();
	return 0;
}
