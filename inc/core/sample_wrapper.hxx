#ifndef H_SAMPLE_WRAPPER
#define H_SAMPLE_WRAPPER

#include <string>
#include <vector>

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDF/RInterface.hxx"

/**
 * class representing a certain category of samples 
 * ex. ZJets, QCDMultijet, ttbar, GluGluHToGammaGamma
 */

class SampleWrapper {
  private:
    std::vector<std::string> sample_filenames;
    std::vector<std::string> flags;
    std::vector<std::string> cuts;
    ROOT::RDF::RInterface<ROOT::Detail::RDF::RJittedFilter, void> sample_data_frame;
  
  public:
    short sample_color;
    float luminosity;
    bool is_data;
    std::string sample_name;
    std::string sample_description;
    
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
    SampleWrapper(std::string i_sample_name, std::vector<std::string> i_sample_filenames, short i_sample_color, std::string i_sample_description="", float i_luminosity=1, bool i_is_data=false, const char *tree_name="tree");
    
    ///**
    // * constructor for samples with a single name
    // */
    //SampleWrapper(std::string i_sample_name, std::string i_sample_filename, short i_sample_color, std::string i_sample_description="", float i_luminosity=1, bool i_is_data=false, const char* tree_name="tree");
    
    /**
     * method for adding a flag
     */
    SampleWrapper* add_flag(std::string flag);
    
    /**
     * method for checking a flag
     */
    bool check_flag(std::string flag);

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
