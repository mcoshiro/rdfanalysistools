#ifndef H_PLOT_COLLECTION
#define H_PLOT_COLLECTION

#include <string>
#include <vector>

#include "TCanvas.h"
#include "TH1.h"
#include "ROOT/RResultPtr.hxx"
#include "ROOT/RDF/InterfaceUtils.hxx"

#include "../../inc/core/sample_wrapper.hxx"
#include "../../inc/core/variable_axis.hxx"
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
class PlotCollection {
	private:
		//internal variables
		std::string name;
		std::string yname;
		std::string description;
		std::string ydescription;
		std::string file_extension;
		std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> histograms;
		std::vector<std::vector<ROOT::RDF::RResultPtr<TH2D>>> twodim_histograms;
		std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> denominator_histograms;
		std::vector<std::vector<ROOT::RDF::RResultPtr<TH2D>>> twodim_denominator_histograms;
		std::vector<SampleWrapper*> samples;
		RegionCollection* regions;
		float luminosity;
		bool draw_log;
		bool is_2d;
		bool is_efficiency;
		bool save_as_root;

		/**
		 * function to draw several TH1's separately
		 */
		void draweach_1d_histograms();

		/**
		 * function to draw several TH2's separately
		 */
		void draweach_2d_histograms();

	public:
		/**
		 * constructor to generate collection from a vector of vectors, for 1d histograms
		 */
		PlotCollection(VariableAxis axis, std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> i_histograms, std::vector<SampleWrapper*> i_samples, RegionCollection* i_regions);

		/**
		 * constructor to generate collection from a vector of vectors, for 1d efficiencies
		 */
		PlotCollection(VariableAxis axis, std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> i_histograms, std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> i_denominator_histograms, std::vector<SampleWrapper*> i_samples, std::string numerator_description, RegionCollection* i_regions);

		/**
		 * constructor to generate collection from a vector of vectors for 2d histograms
		 */
		PlotCollection(VariableAxis x_axis, VariableAxis y_axis, std::vector<std::vector<ROOT::RDF::RResultPtr<TH2D>>> i_twodim_histograms, std::vector<SampleWrapper*> i_samples, RegionCollection* i_regions);
		/**
		 * constructor to generate collection from a vector of vectors, for 2d efficiencies
		 */
		PlotCollection(VariableAxis x_axis, VariableAxis y_axis, std::vector<std::vector<ROOT::RDF::RResultPtr<TH2D>>> i_twodim_histograms, std::vector<std::vector<ROOT::RDF::RResultPtr<TH2D>>> i_twodim_denominator_histograms, std::vector<SampleWrapper*> i_samples, std::string numerator_description, RegionCollection* i_regions);

		/**
		 * function to save root file
		 */
		void set_save_root_file(bool i_set_save_root_file);

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
		void set_draw_log(bool i_draw_log=true);

		/**
		 * function to set file extension
		 */
		void set_file_extension(std::string i_file_extension="pdf");

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
		 * function to draw several histograms separately
		 */
		void draweach_histograms();

};

#endif