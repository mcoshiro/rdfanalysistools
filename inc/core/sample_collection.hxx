#ifndef H_SAMPLE_COLLECTION
#define H_SAMPLE_COLLECTION

#include <iostream>
#include <string_view>
#include <string>
#include <vector>

#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDF/HistoModels.hxx"

#include "../../inc/core/variable_axis.hxx"
#include "../../inc/core/sample_wrapper.hxx"
#include "../../inc/core/region_collection.hxx"
#include "../../inc/core/plot_collection.hxx"
//#include "../../inc/core/column_definition.hxx"

//class to hold a collection of samples which can be used to make histograms, tables, etc.

class SampleCollection {
  private:
    std::vector<SampleWrapper*> samples;

    ROOT::RDF::TH1DModel get_1d_histogram_model(VariableAxis axis, unsigned int sample_idx, RegionCollection* regions=nullptr, unsigned int region_idx=0);
    ROOT::RDF::TH2DModel get_2d_histogram_model(VariableAxis x_axis, VariableAxis y_axis, unsigned int sample_idx, RegionCollection* regions=nullptr, unsigned int region_idx=0);
  
  public:
    /**
     * default constructor
     */
    SampleCollection();
    
    /**
     * method to add a sample to the collection
     */
    void add(SampleWrapper* sample);
    
    /**
     * method to define data frame columns, see RInterface::Define
     * flags argument can be used to only define colums for certain samples
     */
    void define(const char* name, const char* expression, std::vector<std::string> flags={});
    
    /**
     * method to define data frame columns, see RInterface::Define
     * flags argument can be used to only define colums for certain samples
     */
    template<typename F>
    void define(const char* name, F expression, std::vector<std::string> columns, std::vector<std::string> flags={});

    ///**
    // * method to define data frame columns, see RInterface::Define
    // * flags argument can be used to only define colums for certain samples
    // */
    //template<typename F>
    //void define(ColumnDefinition<F> column_definition, std::vector<std::string> flags={});
    
    /**
     * method to filter data frames, see RInterface::Filter
     * flags argument can be used to only filter certain samples
     */
    void filter(std::string expression, std::string filter_description="", std::vector<std::string> flags={});
    
    /**
     * method to make 1d histograms of variable with weight weight in each region specified by regions, see RInterface::Histo1D
     */
    PlotCollection* book_1d_histogram(VariableAxis axis, std::string_view weight, RegionCollection* regions=nullptr);
    
    /**
     * method to make 1d efficiency plots of variable with weight weight in each region specified by regions, see RInterface::Histo1D
     */
    PlotCollection* book_1d_efficiency_plot(VariableAxis axis, std::string_view weight, std::string numerator_cut, std::string numerator_description, RegionCollection* regions=nullptr);
    
    /**
     * method to make 2d histograms of variable with weight weight in each region specified by regions, see RInterface::Histo2D
     */
    PlotCollection* book_2d_histogram(VariableAxis x_axis, VariableAxis y_axis, std::string_view weight, RegionCollection* regions=nullptr);
    
    /**
     * method to make 2d efficiency plots of variable with weight weight in each region specified by regions, see RInterface::Histo2D
     */
    PlotCollection* book_2d_efficiency_plot(VariableAxis x_axis, VariableAxis y_axis, std::string_view weight, std::string numerator_cut, std::string numerator_description, RegionCollection* regions=nullptr);
    
    /**
     * method to make cutflow table
     */
    //CutflowTable make_cutflow_table();
};

#include "../../src/core/sample_collection.tpp"

#endif
