#include <string>
#include <vector>

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RResultPtr.hxx"
#include "ROOT/RDF/RInterface.hxx"

#include "core/sample_wrapper.hxx"

/**
 * SampleWrapper constructor
 * i_sample_name - name as used in file names, etc.
 * i_sample_filenames - names of root files for SampleWrapper
 * i_sample_color - color used to draw this sample in histograms
 * i_sample_description - name of sample as displayed in plots
 * i_is_data - bool if sample is data
 * tree_name - name of TTree to read from files
 */
SampleWrapper::SampleWrapper(std::string i_sample_name, std::vector<std::string> i_sample_filenames, short i_sample_color, std::string i_sample_description, bool i_is_data, const char* tree_name)
  : sample_data_frame(ROOT::RDataFrame(tree_name,i_sample_filenames).Filter("1"))
{
  sample_name = i_sample_name;
  if (i_sample_description == "")
    sample_description = i_sample_name;
  else
    sample_description = i_sample_description;
  sample_filenames = i_sample_filenames;
  sample_color = i_sample_color;
  is_data = i_is_data;
  weighted_sample = false;
  weight_column = "";
  lumi_weight_column = "";
  luminosity = 1.;
  cross_section = 1.;
}

/**
 * method for adding a flag
 */
SampleWrapper* SampleWrapper::add_flag(std::string flag) {
  flags.push_back(flag);
  return this;
}

/**
 * method that sets weights to use when making tables/plots
 * lumi_weight_column_name is the name of the column to use when normalizing weights i.e. this is the weight that does NOT model inefficiency
 * full_weight_column_name is the name of the column to use when weighting event; if unspecified, lumi_weight_column_name will be used
 */
SampleWrapper* SampleWrapper::set_weight_branches(std::string lumi_weight_column_name, std::string full_weight_column_name) {
  weighted_sample = true;
  lumi_weight_column = lumi_weight_column_name;
  if (full_weight_column_name=="")
    weight_column = lumi_weight_column_name;
  else
    weight_column = full_weight_column_name;
  total_yield = sample_data_frame.Sum(lumi_weight_column);
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
 * get scaling weight based on assigned luminosity and cross section
 */
float SampleWrapper::scale_weight() {
  if (weighted_sample)
    return cross_section*luminosity*1000./(*(total_yield));
  return 1.;
}

/**
 * method for setting cross section of process. Used for rescaling events appropriately for luminosity
 */
SampleWrapper* SampleWrapper::set_cross_section(float i_cross_section) {
  cross_section = i_cross_section;
  return this;
}

/**
 * method for setting luminosity for rescaling
 */
SampleWrapper* SampleWrapper::set_luminosity(float i_luminosity) {
  luminosity = i_luminosity;
  return this;
}

/**
 * method for filtering sample
 */
SampleWrapper* SampleWrapper::filter(std::string expression, std::string filter_description) {
  std::string internal_description = filter_description;
  if (internal_description == "") internal_description = expression;
  sample_data_frame = sample_data_frame.Filter(expression, internal_description);
  cuts.push_back(internal_description);
  if (weighted_sample)
    cut_yields.push_back(sample_data_frame.Sum(weight_column));
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
