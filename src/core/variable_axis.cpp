#include <string>
#include <vector>

#include "core/generic_utils.hxx"
#include "core/variable_axis.hxx"

/**
 * uniform bin constructor
 */
VariableAxis::VariableAxis(std::string i_variable_name, std::string i_variable_description, int i_nbins, double i_low, double i_high, std::string i_units)
  : bins(nullptr)
{
  variable_name = i_variable_name;
  variable_description = i_variable_description;
  units = i_units;
  nbins = i_nbins;
  low = i_low;
  high = i_high;
  uniform_bins = true;
}

/**
 * variable bin constructor
 */
VariableAxis::VariableAxis(std::string i_variable_name, std::string i_variable_description, int i_nbins, const double* i_bins, std::string i_units) 
  : bins(i_bins)
{
  variable_name = i_variable_name;
  variable_description = i_variable_description;
  units = i_units;
  nbins = i_nbins;
  low = -1;
  high = -1;
  bins = i_bins;
  uniform_bins = false;
}

/**
 * return string describing units as it should be applied to axes
 */
std::string VariableAxis::formatted_units() {
  if (units != "")
    return ("["+units+"]");
  return "";
}

/**
 * return string describing bin size
 */
std::string VariableAxis::bin_size() {
  if (units != "" && uniform_bins)
    return (float_to_string((high-low)/static_cast<float>(nbins),4)+" "+units);
  return "bin";
}
