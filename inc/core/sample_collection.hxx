#ifndef H_SAMPLE_COLLECTION
#define H_SAMPLE_COLLECTION

#include <iostream>
#include <string_view>
#include <string>
#include <vector>

#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDF/HistoModels.hxx"

#include "core/variable_axis.hxx"
#include "core/sample_wrapper.hxx"
#include "core/region_collection.hxx"
#include "core/plot_collection.hxx"
#include "core/table_collection.hxx"
//#include "core/column_definition.hxx"

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
    SampleCollection* add(SampleWrapper* sample);
    
    /**
     * method to define data frame columns, see RInterface::Define
     * flags argument can be used to only define colums for certain samples
     */
    SampleCollection* define(const char* name, const char* expression, std::vector<std::string> flags={});
    
    /**
     * method to define data frame columns, see RInterface::Define
     * flags argument can be used to only define colums for certain samples
     */
    template<typename F>
    SampleCollection* define(const char* name, F expression, const std::vector<std::string> columns, std::vector<std::string> flags={});

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
    SampleCollection* filter(std::string expression, std::string filter_description="", std::vector<std::string> flags={});

    /**
     * method to set luminosity
     */
    SampleCollection* set_luminosity(float luminosity, std::vector<std::string> flags={});

    /**
     * method that sets weights to use when making tables/plots
     * lumi_weight_column_name is the name of the column to use when normalizing weights i.e. this is the weight that does NOT model inefficiency
     * full_weight_column_name is the name of the column to use when weighting event; if unspecified, lumi_weight_column_name will be used
     */
    SampleCollection* set_weight_branches(std::string lumi_weight_column_name, std::string full_weight_column_name="", std::vector<std::string> flags={});
    
    /**
     * method to make 1d histograms of variable with weight weight in each region specified by regions, see RInterface::Histo1D
     */
    PlotCollection* book_1d_histogram(VariableAxis axis, RegionCollection* regions=nullptr);
    
    /**
     * method to make 1d efficiency plots of variable with weight weight in each region specified by regions, see RInterface::Histo1D
     */
    PlotCollection* book_1d_efficiency_plot(VariableAxis axis, std::string numerator_cut, std::string numerator_description, RegionCollection* regions=nullptr);
    
    /**
     * method to make 2d histograms of variable with weight weight in each region specified by regions, see RInterface::Histo2D
     */
    PlotCollection* book_2d_histogram(VariableAxis x_axis, VariableAxis y_axis, RegionCollection* regions=nullptr);
    
    /**
     * method to make 2d efficiency plots of variable with weight weight in each region specified by regions, see RInterface::Histo2D
     */
    PlotCollection* book_2d_efficiency_plot(VariableAxis x_axis, VariableAxis y_axis, std::string numerator_cut, std::string numerator_description, RegionCollection* regions=nullptr);
    
    /**
     * method to make cutflow table
     */
    TableCollection* book_cutflow_table();
};

#include "../../src/core/sample_collection.tpp"

#endif
