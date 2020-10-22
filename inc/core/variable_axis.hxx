#ifndef H_VARIABLE_AXIS
#define H_VARIABLE_AXIS

#include <string>
#include <vector>

/***
 * class to hold information used in construction an axis for a plot
 */
class VariableAxis {
  private:

  public:
    std::string variable_name, variable_description, units;
    int nbins;
    double low, high;
    const double* bins;
    bool uniform_bins;
    
    /**
     * uniform bin constructor
     */
    VariableAxis(std::string i_variable_name, std::string i_variable_description, int i_nbins, double i_low, double i_high, std::string i_units="");

    /**
     * variable bin constructor
     */
    VariableAxis(std::string i_variable_name, std::string i_variable_description, int i_nbins, const double* i_bins, std::string i_units="");

    /**
     * return string describing units as it should be applied to axes
     */
    std::string formatted_units();
    
    /**
     * return string describing bin size
     */
    std::string bin_size();
};

#endif
