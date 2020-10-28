#ifndef H_SAMPLE_WRAPPER
#define H_SAMPLE_WRAPPER

#include <string>
#include <vector>

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RResultPtr.hxx"

/**
 * class representing a certain category of samples 
 * ex. ZJets, QCDMultijet, ttbar, GluGluHToGammaGamma
 */

class SampleWrapper {
  private:
    std::vector<std::string> sample_filenames;
    std::vector<std::string> flags;
    std::string lumi_weight_column;
    ROOT::RDF::RInterface<ROOT::Detail::RDF::RJittedFilter, void> sample_data_frame;
    ROOT::RDF::RResultPtr<ROOT::Detail::RDF::SumReturnType_t<double>> total_yield;
    float cross_section;
    float normed_luminosity;
    float luminosity;
  
  public:
    short sample_color;
    bool is_data;
    bool weighted_sample;
    std::string weight_column;
    std::string sample_name;
    std::string sample_description;
    std::vector<std::string> cuts;
    std::vector<ROOT::RDF::RResultPtr<ROOT::Detail::RDF::SumReturnType_t<double>>> cut_yields;
    
    /**
     * SampleWrapper constructor
     * i_sample_name - name as used in file names, etc.
     * i_sample_filenames - names of root files for SampleWrapper
     * i_sample_color - color used to draw this sample in histograms
     * i_sample_description - name of sample as displayed in plots
     * i_is_data - bool if sample is data
     * tree_name - name of TTree to read from files
     */
    SampleWrapper(std::string i_sample_name, std::vector<std::string> i_sample_filenames, short i_sample_color, std::string i_sample_description="", bool i_is_data=false, const char *tree_name="tree");
    
    /**
     * method for adding a flag
     */
    SampleWrapper* add_flag(std::string flag);

    /**
     * method that sets weights to use when making tables/plots
     * note that for "data", these weights are used in plotting but not scaling
     * lumi_weight_column_name is the name of the column to use when normalizing weights i.e. this is the weight that does NOT model inefficiency
     * full_weight_column_name is the name of the column to use when weighting event; if unspecified, lumi_weight_column_name will be used
     */
    SampleWrapper* set_weight_branches(std::string lumi_weight_column_name, std::string full_weight_column_name="");
    
    /**
     * method for checking a flag
     */
    bool check_flag(std::string flag);

    /**
     * method for setting cross section of process in pb. Used for rescaling events appropriately for luminosity
     */
    SampleWrapper* set_cross_section(float i_cross_section);

    /**
     * method for setting luminosity for rescaling
     */
    SampleWrapper* set_luminosity(float i_luminosity);

    /**
     * get scaling weight based on assigned luminosity and cross section
     */
    float scale_weight();

    /**
     * method for filtering sample
     */
    SampleWrapper* filter(std::string expression, std::string filter_description="");

    /**
     * method for getting formatted string of all cuts applied
     */
    std::string selection_string();
    
    /**
     * internal RDataFrame object
     */
    ROOT::RDF::RInterface<ROOT::Detail::RDF::RJittedFilter, void> & data_frame();
};

#endif
