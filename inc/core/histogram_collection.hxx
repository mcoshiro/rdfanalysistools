#ifndef H_HISTOGRAM_COLLECTION
#define H_HISTOGRAM_COLLECTION

#include <string>
#include <vector>

#include "TCanvas.h"
#include "TH1.h"
#include "ROOT/RResultPtr.hxx"
#include "ROOT/RDF/InterfaceUtils.hxx"

#include "../../inc/core/sample_wrapper.hxx"
#include "../../inc/core/region_collection.hxx"

/**
 * struct to be used when sorting histograms
 */
struct HistogramAndStyle {
	ROOT::RDF::RResultPtr<TH1D> histogram;
	std::string description;
	short color;
	bool is_data;
};

/**
 * sorting function to sort histograms by integrated area
 */
bool sort_by_integral(HistogramAndStyle hist_a, HistogramAndStyle hist_b);

/**
 * sorting function to sort histograms based on their maximum value
 */
bool sort_by_maximum(HistogramAndStyle hist_a, HistogramAndStyle hist_b);

/**
 * class to hold a collection of booked histograms
 * equipped with methods for drawing said histograms
 */
class HistogramCollection {
	private:
		std::string name;
		std::string description;
		std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> histograms;
		std::vector<SampleWrapper*> samples;
		RegionCollection regions;
		float luminosity;

	public:
		/**
		 * constructor to generate collection from a vector of vectors
		 */
		HistogramCollection(std::string i_name, std::string i_description, std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> i_histograms, std::vector<SampleWrapper*> i_samples, RegionCollection i_regions);

		/**
		 * function to set luminosity
		 */
		void set_luminosity(float i_luminosity);

		/**
		 * function to draw several TH1's on top of each other
		 */
		void overlay_1d_histograms();
};

#endif
