#ifndef H_PLOT_COLLECTION
#define H_PLOT_COLLECTION

#include <string>
#include <vector>

#include "TCanvas.h"
#include "TH1.h"
#include "ROOT/RResultPtr.hxx"
#include "ROOT/RDF/InterfaceUtils.hxx"

#include "core/sample_wrapper.hxx"
#include "core/variable_axis.hxx"
#include "core/region_collection.hxx"

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
 * plot option enumerations
 */
enum class PlotCombineStyle {
  overlay,
  stack
};
enum class BottomStyle {
  none,
  ratio,
  upper_cut_significance,
  lower_cut_significance
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
    bool x_log, y_log, z_log;
    bool is_2d, is_efficiency;
    bool save_as_root;
    PlotCombineStyle plot_combine_style;
    BottomStyle bottom_style;
    
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
     * function to set luminosity
     * FIXME: does this need to be modified?
     */
    void set_luminosity(float i_luminosity);

    /**
     * function to save root file
     */
    PlotCollection* set_save_root_file(bool i_set_save_root_file);
    
    /**
     * function to set log options
     */
    PlotCollection* set_log(bool i_x_log=false, bool i_y_log=false, bool i_z_log=false);
    
    /**
     * function to set file extension options
     */
    PlotCollection* set_file_extension(std::string i_file_extension="pdf");
    
    /**
     * function to set combine style option
     */
    PlotCollection* set_plot_combine_style(PlotCombineStyle i_plot_combine_style);

    /**
     * function to set bottom style option
     */
    PlotCollection* set_bottom_style(BottomStyle i_bottom_style);
    
    /**
     * function to draw several 1d plots together (stacked/overlayed)
     * for stacks, samples marked as 'data' will be drawn over the stack rather than in it
     * sort_histograms - organize non-data histograms so largest non-data histograms are on top of stack
     */
    void draw_together(bool sort_histograms=true);
    
    /**
     * function to draw several plots separately
     */
    void draw_separate();
  
};

#endif
