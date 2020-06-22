#ifndef H_SAMPLE_WRAPPER
#define H_SAMPLE_WRAPPER

#include <string>
#include <vector>

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDF/RInterface.hxx"

//class representing a certain category of samples ex. ZJets, QCDMultijet, ttbar, GluGluHToGammaGamma...
//may contain multiple samples ex. VVV might include WWW, WWZ, WZZ, and ZZZ
//can combine multiple sub-samplewrappers into one in order to plot together despite different cuts on the different subsamples (useful for certain stitches)

class SampleWrapper {
	private:
		std::vector<std::string> sample_filenames;
		std::vector<SampleWrapper*> sub_samples;
		ROOT::RDF::RInterface<ROOT::Detail::RDF::RJittedFilter, void> *sample_data_frame;
	public:
		bool is_composite_sample;
		short sample_color;
		bool is_data;
		std::string sample_name;
		std::string sample_description;

		/**
		 * constructor for adding multiple filenames
		 */
		SampleWrapper(std::string i_sample_name, std::vector<std::string> i_sample_filenames, short i_sample_color, std::string i_sample_description="", bool i_is_data=false);

		/**
		 * constructor for samples with a single name
		 */
		SampleWrapper(std::string i_sample_name, std::string i_sample_filename, short i_sample_color, std::string i_sample_description="", bool i_is_data=false);

		/**
		 * constructor for combining other SampleWrappers
		 */
		SampleWrapper(std::string i_sample_name, std::vector<SampleWrapper*> i_sub_samples, short i_sample_color, std::string i_sample_description="", bool i_is_data=false);

		/**
		 * destructor
		 */
		~SampleWrapper();

		/**
		 * internal RDataFrame object
		 */
		ROOT::RDF::RInterface<ROOT::Detail::RDF::RJittedFilter, void> & data_frame();
};

#endif
