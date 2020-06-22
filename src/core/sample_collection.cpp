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
void SampleCollection::define(std::string_view name, std::string_view expression, std::set<int> flags) {
	for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
		if (flags.size() == 0 || flags.count(sample_flags[sample_idx])>0) {
			samples[sample_idx].data_frame() = samples[sample_idx].data_frame().Define(name, expression);
		}
	}
}

/**
 * method to define data frame columns, see RInterface::Define
 * flags argument can be used to only define colums for certain samples
 */
template<typename F>
void SampleCollection::define(std::string_view name, F expression, const ROOT::Detail::RDF::ColumnNames_t & columns, std::set<int> flags) {
	for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
		if (flags.size() == 0 || flags.count(sample_flags[sample_idx])>0) {
			samples[sample_idx].data_frame() = samples[sample_idx].data_frame().Define(name, expression, columns);
		}
	}
}

/**
 * method to filter data frames, see RInterface::Filter
 * flags argument can be used to only filter certain samples
 */
void SampleCollection::filter(std::string expression, std::set<int> flags, std::string selection_description) {
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
HistogramCollection SampleCollection::book_histogram(RegionCollection regions, int nbins, double xlow, double xhigh, std::string variable, std::string_view weight, std::string description) {
	std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> histograms;
	std::string internal_description = description;
	if (description=="") internal_description = variable;
	//loop over samples
	for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
		histograms.push_back(std::vector<ROOT::RDF::RResultPtr<TH1D>>());
		//loop over regions
		for (unsigned int region_idx = 0; region_idx < regions.size(); region_idx++) {
			//filter sample to region
			ROOT::RDF::RInterface<ROOT::Detail::RDF::RJittedFilter, void> region_data_frame = samples[sample_idx].data_frame().Filter(regions.get_cuts(region_idx, sample_flags[sample_idx]));
			histograms[sample_idx].push_back(region_data_frame.Histo1D({
				("hist_"+variable+"_"+samples[sample_idx].sample_name+"_"+regions.get_name(region_idx)).c_str(),
				(description+baseline_selection_descriptions[sample_idx]+", "+regions.get_description(region_idx)+"; "+description).c_str(),
				nbins,xlow,xhigh},variable,weight));
		}
	}
	return HistogramCollection(variable, description, histograms);
}
