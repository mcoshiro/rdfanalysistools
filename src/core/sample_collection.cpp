#include <iostream>
#include <string_view>
#include <string>
#include <set>
#include <vector>

#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDF/HistoModels.hxx"
#include "ROOT/RDF/InterfaceUtils.hxx"

#include "../../inc/core/sample_wrapper.hxx"
#include "../../inc/core/sample_collection.hxx"
#include "../../inc/core/histogram_collection.hxx"


/**
 * default constructor
 */
SampleCollection::SampleCollection() {
	//do nothing
}


/**
 * method to add a sample to the collection
 */
void SampleCollection::add(SampleWrapper sample, int flag) {
	samples.push_back(sample);
	sample_flags.push_back(flag);
	baseline_selection_descriptions.push_back("");
}

/**
 * method to define data frame columns, see RInterface::Define
 * flags argument can be used to only define colums for certain samples
 */
//void SampleCollection::define(const char* name, const char* expression, std::set<int> flags) {
//	for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
//		if (flags.size() == 0 || flags.count(sample_flags[sample_idx])>0) {
//			samples[sample_idx].data_frame() = samples[sample_idx].data_frame().Define(name, expression);
//		}
//	}
//}

/**
 * method to filter data frames, see RInterface::Filter
 * flags argument can be used to only filter certain samples
 */
void SampleCollection::filter(std::string expression, std::string selection_description, std::set<int> flags) {
	for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
		if (flags.size() == 0 || flags.count(sample_flags[sample_idx])>0) {
			samples[sample_idx].data_frame() = samples[sample_idx].data_frame().Filter(expression);
			if (selection_description.size()==0) {
				baseline_selection_descriptions[sample_idx] = baseline_selection_descriptions[sample_idx] + ", " + expression;
			}
			else {
				baseline_selection_descriptions[sample_idx] = baseline_selection_descriptions[sample_idx] + ", " + selection_description;
			}
		}
	}
}

/**
 * method to make 1d histograms of variable with weight weight in each region specified by regions, see RInterface::Histo1D
 */
HistogramCollection SampleCollection::book_1d_histogram(RegionCollection regions, int nbins, double xlow, double xhigh, std::string variable, std::string description, std::string_view weight) {
	std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> histograms;
	std::vector<SampleWrapper*> sample_pointers;
	std::string internal_description = description;
	if (description=="") internal_description = variable;
	//loop over samples
	for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
		histograms.push_back(std::vector<ROOT::RDF::RResultPtr<TH1D>>());
		sample_pointers.push_back(&samples[sample_idx]);
		//loop over regions
		for (unsigned int region_idx = 0; region_idx < regions.size(); region_idx++) {
			//filter sample to region
			ROOT::RDF::RInterface<ROOT::Detail::RDF::RJittedFilter, void> region_data_frame = samples[sample_idx].data_frame().Filter(regions.get_cuts(region_idx, sample_flags[sample_idx]));
			histograms[sample_idx].push_back(region_data_frame.Histo1D({
				("hist_"+variable+"_"+samples[sample_idx].sample_name+"_"+regions.get_name(region_idx)).c_str(),
				(internal_description+" "+baseline_selection_descriptions[sample_idx]+", "+regions.get_description(region_idx)+"; "+internal_description).c_str(),
				nbins,xlow,xhigh},variable,weight));
			//debug printouts
			if (false) {
				std::cout << "DEBUG: histograms[" << sample_idx << "].push_back(region_data_frame.Histo1D({"
				<< "(hist_"+variable+"_"+samples[sample_idx].sample_name+"_"+regions.get_name(region_idx)+").c_str(),"
				<< "("+internal_description+baseline_selection_descriptions[sample_idx]+", "+regions.get_description(region_idx)+"; "+internal_description+").c_str(),"
				<< nbins << "," << xlow << "," << xhigh << "}," << variable << "," << weight << "));" << std::endl;
			}
		}
	}
	return HistogramCollection(variable, internal_description, histograms, sample_pointers, regions);
}

/**
 * method to make 1d efficiency plots of variable with weight weight in each region specified by regions, see RInterface::Histo1D
 */
HistogramCollection SampleCollection::book_1d_efficiency_plot(RegionCollection regions, int nbins, double xlow, double xhigh, std::string variable, std::string description, std::string_view weight, std::string numerator_cut, std::string numerator_description) {
	std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> histograms;
	std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> denominator_histograms;
	std::vector<SampleWrapper*> sample_pointers;
	std::string internal_description = description;
	if (description=="") internal_description = variable;
	//loop over samples
	for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
		histograms.push_back(std::vector<ROOT::RDF::RResultPtr<TH1D>>());
		denominator_histograms.push_back(std::vector<ROOT::RDF::RResultPtr<TH1D>>());
		sample_pointers.push_back(&samples[sample_idx]);
		//loop over regions
		for (unsigned int region_idx = 0; region_idx < regions.size(); region_idx++) {
			//filter sample to region
			ROOT::RDF::RInterface<ROOT::Detail::RDF::RJittedFilter, void> region_data_frame = samples[sample_idx].data_frame().Filter(regions.get_cuts(region_idx, sample_flags[sample_idx]));
			denominator_histograms[sample_idx].push_back(region_data_frame.Histo1D({
				("hist_"+variable+"_"+samples[sample_idx].sample_name+"_"+regions.get_name(region_idx)).c_str(),
				(internal_description+" "+baseline_selection_descriptions[sample_idx]+", "+regions.get_description(region_idx)+"; "+internal_description).c_str(),
				nbins,xlow,xhigh},variable,weight));
			region_data_frame = region_data_frame.Filter(numerator_cut);
			histograms[sample_idx].push_back(region_data_frame.Histo1D({
				("hist_"+variable+"_"+samples[sample_idx].sample_name+"_"+regions.get_name(region_idx)).c_str(),
				(internal_description+" "+baseline_selection_descriptions[sample_idx]+", "+regions.get_description(region_idx)+"; "+internal_description+"; Efficiency "+numerator_description).c_str(),
				nbins,xlow,xhigh},variable,weight));
		}
	}
	return HistogramCollection(variable, internal_description, histograms, denominator_histograms, sample_pointers, regions);
}

/**
 * method to make 2d histograms of variable with weight weight in each region specified by regions, see RInterface::Histo2D
 */
HistogramCollection SampleCollection::book_2d_histogram(RegionCollection regions, int xnbins, double xlow, double xhigh, std::string xvariable, std::string xdescription, int ynbins, double ylow, double yhigh, std::string yvariable, std::string ydescription, std::string_view weight) {
	std::vector<std::vector<ROOT::RDF::RResultPtr<TH2D>>> histograms;
	std::vector<SampleWrapper*> sample_pointers;
	std::string x_internal_description = xdescription;
	std::string y_internal_description = ydescription;
	if (xdescription=="") x_internal_description = xvariable;
	if (ydescription=="") y_internal_description = yvariable;
	//loop over samples
	for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
		histograms.push_back(std::vector<ROOT::RDF::RResultPtr<TH2D>>());
		sample_pointers.push_back(&samples[sample_idx]);
		//loop over regions
		for (unsigned int region_idx = 0; region_idx < regions.size(); region_idx++) {
			//filter sample to region
			ROOT::RDF::RInterface<ROOT::Detail::RDF::RJittedFilter, void> region_data_frame = samples[sample_idx].data_frame().Filter(regions.get_cuts(region_idx, sample_flags[sample_idx]));
			histograms[sample_idx].push_back(region_data_frame.Histo2D({
				("hist_"+xvariable+"_"+yvariable+"_"+samples[sample_idx].sample_name+"_"+regions.get_name(region_idx)).c_str(),
				(y_internal_description+" vs. "+x_internal_description+" "+baseline_selection_descriptions[sample_idx]+", "+regions.get_description(region_idx)+"; "+x_internal_description+"; "+y_internal_description).c_str(),
				xnbins,xlow,xhigh,ynbins,ylow,yhigh},xvariable,yvariable,weight));
		}
	}
	return HistogramCollection(xvariable, x_internal_description, yvariable, y_internal_description, histograms, sample_pointers, regions);
}

/**
 * method to make 2d efficiency plots of variable with weight weight in each region specified by regions, see RInterface::Histo2D
 */
HistogramCollection SampleCollection::book_2d_efficiency_plot(RegionCollection regions, int xnbins, double xlow, double xhigh, std::string xvariable, std::string xdescription, int ynbins, double ylow, double yhigh, std::string yvariable, std::string ydescription, std::string_view weight, std::string numerator_cut, std::string numerator_description) {
	std::vector<std::vector<ROOT::RDF::RResultPtr<TH2D>>> histograms;
	std::vector<std::vector<ROOT::RDF::RResultPtr<TH2D>>> denominator_histograms;
	std::vector<SampleWrapper*> sample_pointers;
	std::string x_internal_description = xdescription;
	std::string y_internal_description = ydescription;
	if (xdescription=="") x_internal_description = xvariable;
	if (ydescription=="") y_internal_description = yvariable;
	//loop over samples
	for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
		histograms.push_back(std::vector<ROOT::RDF::RResultPtr<TH2D>>());
		denominator_histograms.push_back(std::vector<ROOT::RDF::RResultPtr<TH2D>>());
		sample_pointers.push_back(&samples[sample_idx]);
		//loop over regions
		for (unsigned int region_idx = 0; region_idx < regions.size(); region_idx++) {
			//filter sample to region
			ROOT::RDF::RInterface<ROOT::Detail::RDF::RJittedFilter, void> region_data_frame = samples[sample_idx].data_frame().Filter(regions.get_cuts(region_idx, sample_flags[sample_idx]));
			denominator_histograms[sample_idx].push_back(region_data_frame.Histo2D({
				("hist_"+xvariable+"_"+yvariable+"_"+samples[sample_idx].sample_name+"_"+regions.get_name(region_idx)).c_str(),
				(y_internal_description+" vs. "+x_internal_description+" "+baseline_selection_descriptions[sample_idx]+", "+regions.get_description(region_idx)+"; "+x_internal_description+"; "+y_internal_description).c_str(),
				xnbins,xlow,xhigh,ynbins,ylow,yhigh},xvariable,yvariable,weight));
			region_data_frame.Filter(numerator_cut);
			histograms[sample_idx].push_back(region_data_frame.Histo2D({
				("hist_"+xvariable+"_"+yvariable+"_"+samples[sample_idx].sample_name+"_"+regions.get_name(region_idx)).c_str(),
				("Efficiency "+numerator_description+" "+y_internal_description+" vs. "+x_internal_description+" "+baseline_selection_descriptions[sample_idx]+", "+regions.get_description(region_idx)+"; "+x_internal_description+"; "+y_internal_description).c_str(),
				xnbins,xlow,xhigh,ynbins,ylow,yhigh},xvariable,yvariable,weight));
		}
	}
	return HistogramCollection(xvariable, x_internal_description, yvariable, y_internal_description, histograms, denominator_histograms, sample_pointers, regions);
}
