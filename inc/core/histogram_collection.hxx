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
 * also can be used for ratios/efficiency plots
 */
class HistogramCollection {
	private:
		std::string name;
		std::string yname;
		std::string description;
		std::string ydescription;
		std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> histograms;
		std::vector<std::vector<ROOT::RDF::RResultPtr<TH2D>>> twodim_histograms;
		std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> denominator_histograms;
		std::vector<std::vector<ROOT::RDF::RResultPtr<TH2D>>> twodim_denominator_histograms;
		std::vector<SampleWrapper*> samples;
		RegionCollection regions;
		float luminosity;
		bool draw_log;
		bool is_2d;
		bool is_efficiency;

	public:
		/**
		 * constructor to generate collection from a vector of vectors, for 1d histograms
		 */
		HistogramCollection(std::string i_name, std::string i_description, std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> i_histograms, std::vector<SampleWrapper*> i_samples, RegionCollection i_regions);

		/**
		 * constructor to generate collection from a vector of vectors for 1d efficiencies
		 */
		HistogramCollection(std::string i_name, std::string i_description, std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> i_histograms, std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> i_denominator_histograms, std::vector<SampleWrapper*> i_samples, RegionCollection i_regions);

		/**
		 * constructor to generate collection from a vector of vectors for 2d histograms
		 */
		HistogramCollection(std::string i_name, std::string i_description, std::string i_yname, std::string i_ydescription, std::vector<std::vector<ROOT::RDF::RResultPtr<TH2D>>> i_twodim_histograms, std::vector<SampleWrapper*> i_samples, RegionCollection i_regions);

		/**
		 * constructor to generate collection from a vector of vectors for 2d efficiencies
		 */
		HistogramCollection(std::string i_name, std::string i_description, std::string i_yname, std::string i_ydescription, std::vector<std::vector<ROOT::RDF::RResultPtr<TH2D>>> i_twodim_histograms, std::vector<std::vector<ROOT::RDF::RResultPtr<TH2D>>> i_twodim_denominator_histograms, std::vector<SampleWrapper*> i_samples, RegionCollection i_regions);

		/**
		 * function to set luminosity
		 */
		void set_luminosity(float i_luminosity);

		/**
		 * function to draw several TH1's overlayed on each other
		 */
		void overlay_1d_histograms();

		/**
		 * function to set log y
		 */
		void set_draw_log();

		/**
		 * function to draw several TH1's stacked on each other; samples marked as 'data' will be drawn over the stack rather than in it
		 */
		void stack_1d_histograms(bool sort_histograms=true);

		/**
		 * function to draw several TH1's stacked on each other; samples marked as 'data' will be drawn over the stack rather than in it
		 * additionally, draw a data/MC ratio plot
		 */
		void stack_ratio_1d_histograms(bool sort_histograms=true);

		/**
		 * function to draw several TH2's separately
		 */
		void draweach_2d_histograms();
};

#endif
