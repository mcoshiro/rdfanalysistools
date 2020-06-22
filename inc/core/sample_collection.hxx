#ifndef H_SAMPLE_COLLECTION
#define H_SAMPLE_COLLECTION

#include <string_view>
#include <string>
#include <set>
#include <vector>

#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDF/HistoModels.hxx"

#include "../../inc/core/sample_wrapper.hxx"
#include "../../inc/core/region_collection.hxx"
#include "../../inc/core/histogram_collection.hxx"

//class to hold a collection of samples which can be used to make histograms, tables, etc.

class SampleCollection {
	private:
		std::vector<SampleWrapper> samples;
		std::vector<int> sample_flags;
		std::vector<std::string> baseline_selection_descriptions;

	public:
		/**
		 * default constructor
		 */
		SampleCollection();

		/**
		 * method to add a sample to the collection
		 */
		void add(SampleWrapper sample, int flag=0);

		/**
		 * method to define data frame columns, see RInterface::Define
		 * flags argument can be used to only define colums for certain samples
		 */
		void define(std::string_view name, std::string_view expression, std::set<int> flags={});

		/**
		 * method to define data frame columns, see RInterface::Define
		 * flags argument can be used to only define colums for certain samples
		 */
		template<typename F>
		void define(std::string_view name, F expression, const ROOT::Detail::RDF::ColumnNames_t & columns={}, std::set<int> flags={});

		/**
		 * method to filter data frames, see RInterface::Filter
		 * flags argument can be used to only filter certain samples
		 */
		void filter(std::string expression, std::set<int> flags={}, std::string baseline_selection_description="");

		/**
		 * method to make 1d histograms of variable with weight weight in each region specified by regions, see RInterface::Histo1D
		 */
		HistogramCollection book_histogram(RegionCollection regions, int nbins, double xlow, double xhigh, std::string variable, std::string_view weight, std::string description="");
};

#endif
