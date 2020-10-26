#include <iostream>
#include <string>
#include <vector>

#include "ROOT/RResultPtr.hxx"
#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDF/RCutFlowReport.hxx"

#include "core/table_collection.hxx"

TableCollection::TableCollection(std::vector<ROOT::RDF::RResultPtr<ROOT::RDF::RCutFlowReport>> i_cutflows, std::vector<SampleWrapper*> i_samples) {
  cutflows = i_cutflows;
  samples = i_samples;
  luminosity = 1.0;
}

TableCollection* TableCollection::set_luminosity(float i_luminosity) {
  luminosity = i_luminosity;
  return this;
}

void TableCollection::print() {
  for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
    std::cout << samples[sample_idx]->sample_description << std::endl;
    std::cout << "Default print: \n";
    cutflows[sample_idx]->Print();
    std::cout << "Custom print: \n";
    for (unsigned int cut_idx = 0; cut_idx < samples[sample_idx]->cuts.size(); cut_idx++) {
      std::string cut_name = samples[sample_idx]->cuts[cut_idx];
      std::cout << cut_name << ": ";
      std::cout << cutflows[sample_idx]->At(cut_name).GetPass() << ": " << cutflows[sample_idx]->At(cut_name).GetEff() << std::endl;
    }
  }
}
