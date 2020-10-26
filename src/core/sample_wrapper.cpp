#include <string>
#include <vector>

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDF/RInterface.hxx"

#include "core/sample_wrapper.hxx"

/**
 * SampleWrapper constructor
 * i_sample_name - name as used in file names, etc.
 * i_sample_filenames - names of root files for SampleWrapper
 * i_sample_color - color used to draw this sample in histograms
 * i_sample_description - name of sample as displayed in plots
 * i_luminosity - luminosity with which to scale sample
 * i_is_data - bool if sample is data
 * tree_name - name of TTree to read from files
 */
SampleWrapper::SampleWrapper(std::string i_sample_name, std::vector<std::string> i_sample_filenames, short i_sample_color, std::string i_sample_description, float i_luminosity, bool i_is_data, const char* tree_name)
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
  luminosity = i_luminosity;
}

///**
// * constructor for samples with a single name
// */
//SampleWrapper::SampleWrapper(std::string i_sample_name, std::string i_sample_filename, short i_sample_color, std::string i_sample_description, float, i_luminosity, bool i_is_data, const char* tree_name)
//	: sample_data_frame(ROOT::RDataFrame(tree_name,i_sample_filename).Filter("1"))
//{
//	sample_name = i_sample_name;
//	if (i_sample_description == "") {
//		sample_description = i_sample_name;
//	}
//	else {
//		sample_description = i_sample_description;
//	}
//	sample_filenames = std::vector<std::string>(1,i_sample_filename);
//	sample_color = i_sample_color;
//	is_data = i_is_data;
//	luminosity = i_luminosity;
//}

/**
 * method for adding a flag
 */
SampleWrapper* SampleWrapper::add_flag(std::string flag) {
  flags.push_back(flag);
  return this;
}

/**
 * method for checking a flag
 */
bool SampleWrapper::check_flag(std::string flag) {
  for (std::string sample_flag : flags) {
    if (sample_flag == flag) return true;
  }
  return false;
}

/**
 * method for filtering sample
 */
SampleWrapper* SampleWrapper::filter(std::string expression, std::string filter_description) {
  std::string internal_description = filter_description;
  if (internal_description == "") internal_description = expression;
  sample_data_frame = sample_data_frame.Filter(expression, internal_description);
  cuts.push_back(internal_description);
  return this;
}

/**
 * method for getting formatted string of all cuts applied
 */
std::string SampleWrapper::selection_string() {
  std::string selection_string;
  bool first = true;
  for (std::string cut : cuts) {
    if (!first) selection_string = selection_string + ", ";
    selection_string = selection_string + cut;
    if (first) first = false;
  }
  return selection_string;
}

/**
 * internal RDataFrame object
 */
ROOT::RDF::RInterface<ROOT::Detail::RDF::RJittedFilter, void> &SampleWrapper::data_frame() {
  return sample_data_frame;
}
