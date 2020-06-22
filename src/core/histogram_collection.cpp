#include <string>
#include <vector>

#include "TCanvas.h"
#include "TH1.h"
#include "ROOT/RResultPtr.hxx"
#include "ROOT/RDF/InterfaceUtils.hxx"

#include "../../inc/core/histogram_collection.hxx"

/**
 * constructor to generate collection from a vector of vectors
 */
HistogramCollection::HistogramCollection(std::string i_name, std::string i_description, std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> i_histograms) {
	name = i_name;
	description = i_description;
	histograms = i_histograms;
}
