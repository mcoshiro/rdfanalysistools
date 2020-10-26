#ifndef H_TABLE_COLLECTION
#define H_TABLE_COLLECTION

#include <string>
#include <vector>

#include "ROOT/RResultPtr.hxx"
#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDF/RCutFlowReport.hxx"

#include "core/sample_wrapper.hxx"
#include "core/region_collection.hxx"

/**
 * class to hold a collection of booked CutFlowReports
 * equipped with methods for comparing them and writing results to .tex file
 */
class TableCollection {
  private:
    //internal variables
    std::vector<ROOT::RDF::RResultPtr<ROOT::RDF::RCutFlowReport>> cutflows;
    std::vector<SampleWrapper*> samples;
    float luminosity;
  
  public:
    /**
     * constructor to generate collection from a vector of vectors
     */
    TableCollection(std::vector<ROOT::RDF::RResultPtr<ROOT::RDF::RCutFlowReport>> i_cutflows, std::vector<SampleWrapper*> i_samples);
    
    /**
     * function to set luminosity
     * FIXME: use sample lumis?
     */
    TableCollection* set_luminosity(float i_luminosity);
    
    /**
     * function to print table to terminal
     */
    void print();
  
};

#endif
