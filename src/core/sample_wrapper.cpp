#include <string>
#include <vector>

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDF/RInterface.hxx"

#include "../../inc/core/sample_wrapper.hxx"

/**
 * constructor for adding multiple filenames
 */
SampleWrapper::SampleWrapper(std::string i_sample_name, std::vector<std::string> i_sample_filenames, short i_sample_color, std::string i_sample_description, bool i_is_data, const char* tree_name)
	: sample_data_frame(ROOT::RDataFrame(tree_name,i_sample_filenames).Filter("1"))
{
	sample_name = i_sample_name;
	if (i_sample_description == "") {
		sample_description = i_sample_name;
	}
	else {
		sample_description = i_sample_description;
	}
	sample_filenames = i_sample_filenames;
	sample_color = i_sample_color;
	is_data = i_is_data;
	is_composite_sample = false;
}

/**
 * constructor for samples with a single name
 */
SampleWrapper::SampleWrapper(std::string i_sample_name, std::string i_sample_filename, short i_sample_color, std::string i_sample_description, bool i_is_data, const char* tree_name)
	: sample_data_frame(ROOT::RDataFrame(tree_name,i_sample_filename).Filter("1"))
{
	sample_name = i_sample_name;
	if (i_sample_description == "") {
		sample_description = i_sample_name;
	}
	else {
		sample_description = i_sample_description;
	}
	sample_filenames = std::vector<std::string>(1,i_sample_filename);
	sample_color = i_sample_color;
	is_data = i_is_data;
	is_composite_sample = false;
}

///**
// * constructor for combining other SampleWrappers
// */
//SampleWrapper::SampleWrapper(std::string i_sample_name, std::vector<SampleWrapper*> i_sub_samples, short i_sample_color, std::string i_sample_description, bool i_is_data) {
//	sample_name = i_sample_name;
//	if (i_sample_description == "") {
//		sample_description = i_sample_name;
//	}
//	else {
//		sample_description = i_sample_description;
//	}
//	sub_samples = i_sub_samples;
//	sample_color = i_sample_color;
//	is_data = i_is_data;
//	is_composite_sample = true;
//	sample_data_frame = nullptr;
//}

/**
 * internal RDataFrame object
 */
ROOT::RDF::RInterface<ROOT::Detail::RDF::RJittedFilter, void> &SampleWrapper::data_frame() {
	//error when composite, perhaps there is a better way to do this
	if (is_composite_sample) {
		std::cout << "Error: attempting to access RDataFrame for composite SampleWrapper." << std::endl;
		throw;
	}
	return sample_data_frame;
}

