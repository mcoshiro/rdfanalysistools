#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "ROOT/RResultPtr.hxx"
#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDF/RCutFlowReport.hxx"

#include "core/table_collection.hxx"

/**
 * constructor to generate collection from a vector of vectors
 */
TableCollection::TableCollection(std::vector<ROOT::RDF::RResultPtr<ROOT::RDF::RCutFlowReport>> i_cutflows, std::vector<SampleWrapper*> i_samples) {
  cutflows = i_cutflows;
  samples = i_samples;
  luminosity = 1.0;
}

/**
 * function to set luminosity
 * FIXME: use sample lumis?
 */
TableCollection* TableCollection::set_luminosity(float i_luminosity) {
  luminosity = i_luminosity;
  return this;
}

/**
 * function to print table to terminal
 */
void TableCollection::print() {
  for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
    std::cout << samples[sample_idx]->sample_description << std::endl;
    std::cout << "Default print: \n";
    cutflows[sample_idx]->Print();
    std::cout << "Custom print: \n";
    float previous_cut_yield = -1;
    float current_cut_yield = -1;
    for (unsigned int cut_idx = 0; cut_idx < samples[sample_idx]->cuts.size(); cut_idx++) {
      std::string cut_name = samples[sample_idx]->cuts[cut_idx];
      std::cout << cut_name << ": ";
      if (samples[sample_idx]->weighted_sample) {
        if (cut_idx == 0) {
	  current_cut_yield = *(samples[sample_idx]->cut_yields[cut_idx]);
	  if (!samples[sample_idx]->is_data) current_cut_yield = current_cut_yield*samples[sample_idx]->scale_weight();
          std::cout << current_cut_yield << " : -" << std::endl;
	  previous_cut_yield = current_cut_yield;
	}
	else {
	  current_cut_yield = *(samples[sample_idx]->cut_yields[cut_idx]);
	  if (!samples[sample_idx]->is_data) current_cut_yield = current_cut_yield*samples[sample_idx]->scale_weight();
          std::cout << current_cut_yield << " : " << current_cut_yield/previous_cut_yield << std::endl;
	  previous_cut_yield = current_cut_yield;
	}
      }
      else
        std::cout << cutflows[sample_idx]->At(cut_name).GetPass() << " : " << cutflows[sample_idx]->At(cut_name).GetEff() << std::endl;
    }
  }
}

/**
 * function to save table to tex file
 */
void TableCollection::save(std::string filename) {
  if (samples.size() == 0) {
    std::cout << "ERROR: No tables booked." << std::endl; 
    return;
    //currently, all samples need to have the same number of cuts
    unsigned int num_cuts = samples[0]->cuts.size();
    for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
      if (samples[sample_idx]->cuts.size() != num_cuts) {
	std::cout << "ERROR: Currently, save only supports samples with equal numbers of cuts." << std::endl;
	return;
      }
    }

  }
  std::ofstream output_file;
  output_file.open(("tables/"+filename).c_str());
  output_file << "\\documentclass[10pt,oneside]{report}\n";
  output_file << "\\usepackage{graphicx,xspace,amssymb,amsmath,colordvi,colortbl,verbatim,multicol}\n";
  output_file << "\\usepackage{multirow, rotating}\n";
  output_file << "\\usepackage[active,tightpage]{preview}\n";
  output_file << "\\usepackage{siunitx}\n";
  output_file << "\\sisetup{round-mode = figures, round-precision=2}\n";
  output_file << "\\renewcommand{\\arraystretch}{1.1}\n\n";
  output_file << "\\begin{document}\n";
  output_file << "\\begin{preview}\n";
  output_file << "\\begin{tabular}{l";
  for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) output_file << "|r|r";
  output_file << "}\\hline\\hline\n";
  output_file << "Cut ";
  for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) output_file << "& " << samples[sample_idx]->sample_description << "& Eff.";
  output_file << "\\\\ \\hline\n";
  std::vector<float> previous_cut_yield;
  std::vector<float> current_cut_yield;
  for (unsigned int cut_idx = 0; cut_idx < samples[0]->cuts.size(); cut_idx++) {
    output_file << samples[0]->cuts[cut_idx];
    for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
      std::string cut_name = samples[sample_idx]->cuts[cut_idx];
      if (samples[sample_idx]->weighted_sample) {
	if (cut_idx == 0) {
	  current_cut_yield.push_back(*(samples[sample_idx]->cut_yields[cut_idx]));
	  if (!samples[sample_idx]->is_data) current_cut_yield[sample_idx] = current_cut_yield[sample_idx]*samples[sample_idx]->scale_weight();
          output_file << "& " << current_cut_yield[sample_idx] << "& -";
	  previous_cut_yield.push_back(current_cut_yield[sample_idx]);
	}
	else {
	  current_cut_yield[sample_idx] = *(samples[sample_idx]->cut_yields[cut_idx]);
	  if (!samples[sample_idx]->is_data) current_cut_yield[sample_idx] = current_cut_yield[sample_idx]*samples[sample_idx]->scale_weight();
          output_file << "& " << current_cut_yield[sample_idx] << "& " << current_cut_yield[sample_idx]/previous_cut_yield[sample_idx];
	  previous_cut_yield[sample_idx] = current_cut_yield[sample_idx];
	}
      }
      else {
        output_file << "& " << cutflows[sample_idx]->At(cut_name).GetPass() << "& " << cutflows[sample_idx]->At(cut_name).GetEff();
      }
    }
    output_file << "\\\\ \n";
  }
  output_file << "\\hline\\hline \n";
  output_file << "\\end{tabular}\n";
  output_file << "\\end{preview}\n";
  output_file << "\\end{document}\n";
  output_file.close();
}
