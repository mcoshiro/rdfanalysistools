#ifndef H_HISTOGRAM_COLLECTION
#define H_HISTOGRAM_COLLECTION

#include <string>
#include <vector>

#include "TCanvas.h"
#include "TH1.h"
#include "ROOT/RResultPtr.hxx"
#include "ROOT/RDF/InterfaceUtils.hxx"

//class to hold a collection of booked histograms

class HistogramCollection {
	private:
		std::string name;
		std::string description;
		std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> histograms;

	public:
		/**
		 * constructor to generate collection from a vector of vectors
		 */
		HistogramCollection(std::string i_name, std::string i_description, std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> i_histograms);

};

#endif
