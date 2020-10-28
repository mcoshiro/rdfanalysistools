#include <iostream>
#include <string_view>
#include <string>
#include <set>
#include <vector>

#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDF/HistoModels.hxx"
#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDF/RCutFlowReport.hxx"

#include "core/variable_axis.hxx"
#include "core/sample_wrapper.hxx"
#include "core/sample_collection.hxx"
#include "core/plot_collection.hxx"
#include "core/table_collection.hxx"

/**
 * default constructor
 */
SampleCollection::SampleCollection() {
  //do nothing
}


/**
 * method to add a sample to the collection
 */
SampleCollection* SampleCollection::add(SampleWrapper* sample) {
  samples.push_back(sample);
  return this;
}

/**
 * method to define data frame columns, see RInterface::Define
 * flags argument can be used to only define colums for certain samples
 */
SampleCollection* SampleCollection::define(const char* name, const char* expression, std::vector<std::string> flags) {
  if (flags.size() > 0) {
    //if flags provided, define only for samples matching flag
    for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
      for (std::string flag : flags) {
        if (samples[sample_idx]->check_flag(flag)) {
          samples[sample_idx]->data_frame() = samples[sample_idx]->data_frame().Define(name, expression);
          break;
        }
      }
    }
  }
  else {
    //if no flags provided, define for all samples
    for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
      samples[sample_idx]->data_frame() = samples[sample_idx]->data_frame().Define(name, expression);
    }
  }
  return this;
}

/**
 * method to filter data frames, see RInterface::Filter
 * flags argument can be used to only filter certain samples
 */
SampleCollection* SampleCollection::filter(std::string expression, std::string filter_description, std::vector<std::string> flags) {
  if (flags.size() > 0) {
    //if flags provided, filter only samples matching flag
    for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
      for (std::string flag : flags) {
        if (samples[sample_idx]->check_flag(flag)) {
          samples[sample_idx]->filter(expression, filter_description);
          break;
        }
      }
    }
  }
  else {
    //if no flags provided, filter all samples
    for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
      samples[sample_idx]->filter(expression, filter_description);
    }
  }
  return this;
}


/**
 * method to set luminosity
 */
SampleCollection* SampleCollection::set_luminosity(float luminosity, std::vector<std::string> flags) {
  if (flags.size() > 0) {
    //if flags provided, set only for samples matching flag
    for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
      for (std::string flag : flags) {
        if (samples[sample_idx]->check_flag(flag)) {
          if (!samples[sample_idx]->is_data)
            samples[sample_idx]->set_luminosity(luminosity);
          break;
        }
      }
    }
  }
  else {
    //if no flags provided, set for all samples
    for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
      if (!samples[sample_idx]->is_data)
        samples[sample_idx]->set_luminosity(luminosity);
    }
  }
  return this;
}


/**
 * method that sets weights to use when making tables/plots
 * lumi_weight_column_name is the name of the column to use when normalizing weights i.e. this is the weight that does NOT model inefficiency
 * full_weight_column_name is the name of the column to use when weighting event; if unspecified, lumi_weight_column_name will be used
 */
SampleCollection* SampleCollection::set_weight_branches(std::string lumi_weight_column_name, std::string full_weight_column_name, std::vector<std::string> flags) {
  if (flags.size() > 0) {
    //if flags provided, set only for samples matching flag
    for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
      for (std::string flag : flags) {
        if (samples[sample_idx]->check_flag(flag)) {
          samples[sample_idx]->set_weight_branches(lumi_weight_column_name,full_weight_column_name);
          break;
        }
      }
    }
  }
  else {
    //if no flags provided, set for all samples
    for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
      samples[sample_idx]->set_weight_branches(lumi_weight_column_name,full_weight_column_name);
    }
  }
  return this;
}


ROOT::RDF::TH1DModel SampleCollection::get_1d_histogram_model(VariableAxis axis, unsigned int sample_idx, RegionCollection* regions, unsigned int region_idx) {
  std::string hist_name = "hist_"+axis.variable_name+"_"+samples[sample_idx]->sample_name;
  std::string hist_description = axis.variable_description+" "+samples[sample_idx]->selection_string();
  if (regions != nullptr) {
    hist_name = hist_name + "_" + regions->get_name(region_idx);
    hist_description = hist_description+", "+regions->get_description(region_idx);
  }
  hist_description = hist_description+"; "+axis.variable_description+" "+axis.formatted_units();

  if (axis.uniform_bins) {
    return ROOT::RDF::TH1DModel(hist_name.c_str(),hist_description.c_str(),axis.nbins,axis.low,axis.high);
  }
  return ROOT::RDF::TH1DModel(hist_name.c_str(),hist_description.c_str(),axis.nbins,axis.bins);
}

/**
 * method to make 1d histograms of variable with weight weight in each region specified by regions, see RInterface::Histo1D
 */
PlotCollection* SampleCollection::book_1d_histogram(VariableAxis axis, RegionCollection* regions) {
  std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> histograms;
  //loop over samples
  for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
    histograms.push_back(std::vector<ROOT::RDF::RResultPtr<TH1D>>());
    if (regions != nullptr) {
      //loop over regions
      for (unsigned int region_idx = 0; region_idx < regions->size(); region_idx++) {
        //filter sample to region
        ROOT::RDF::RInterface<ROOT::Detail::RDF::RJittedFilter, void> region_data_frame = samples[sample_idx]->data_frame().Filter(regions->get_cuts(region_idx, samples[sample_idx]));
	if (samples[sample_idx]->weighted_sample) {
          histograms[sample_idx].push_back(region_data_frame.Histo1D(
            get_1d_histogram_model(axis,sample_idx,regions,region_idx),axis.variable_name,samples[sample_idx]->weight_column));
        }
	else {
          histograms[sample_idx].push_back(region_data_frame.Histo1D(
            get_1d_histogram_model(axis,sample_idx,regions,region_idx),axis.variable_name));
	}
      }  
    }
    else {
      //no regions
      if (samples[sample_idx]->weighted_sample) {
        histograms[sample_idx].push_back(samples[sample_idx]->data_frame().Histo1D(
          get_1d_histogram_model(axis,sample_idx),axis.variable_name,samples[sample_idx]->weight_column));
      }
      else {
        histograms[sample_idx].push_back(samples[sample_idx]->data_frame().Histo1D(
          get_1d_histogram_model(axis,sample_idx),axis.variable_name));
      }
    }
  }
  return new PlotCollection(axis, histograms, samples, regions);
}

/**
* method to make 1d efficiency plots of variable with weight weight in each region specified by regions, see RInterface::Histo1D
*/
PlotCollection* SampleCollection::book_1d_efficiency_plot(VariableAxis axis, std::string numerator_cut, std::string numerator_description, RegionCollection* regions) {
  std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> histograms;
  std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> denominator_histograms;
  //loop over samples
  for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
    histograms.push_back(std::vector<ROOT::RDF::RResultPtr<TH1D>>());
    denominator_histograms.push_back(std::vector<ROOT::RDF::RResultPtr<TH1D>>());
    if (regions != nullptr) {
      //loop over regions
      for (unsigned int region_idx = 0; region_idx < regions->size(); region_idx++) {
        //filter sample to region
        ROOT::RDF::RInterface<ROOT::Detail::RDF::RJittedFilter, void> region_data_frame = samples[sample_idx]->data_frame().Filter(regions->get_cuts(region_idx, samples[sample_idx]));
	if (samples[sample_idx]->weighted_sample) {
          denominator_histograms[sample_idx].push_back(region_data_frame.Histo1D(
            get_1d_histogram_model(axis,sample_idx,regions,region_idx),axis.variable_name,samples[sample_idx]->weight_column));
          region_data_frame = region_data_frame.Filter(numerator_cut);
          histograms[sample_idx].push_back(region_data_frame.Histo1D(
            get_1d_histogram_model(axis,sample_idx,regions,region_idx),axis.variable_name,samples[sample_idx]->weight_column));
	}
	else {
          denominator_histograms[sample_idx].push_back(region_data_frame.Histo1D(
            get_1d_histogram_model(axis,sample_idx,regions,region_idx),axis.variable_name));
          region_data_frame = region_data_frame.Filter(numerator_cut);
          histograms[sample_idx].push_back(region_data_frame.Histo1D(
           get_1d_histogram_model(axis,sample_idx,regions,region_idx),axis.variable_name));
	}
      }  
    }
    else {
      //no regions
      if (samples[sample_idx]->weighted_sample) {
        denominator_histograms[sample_idx].push_back(samples[sample_idx]->data_frame().Histo1D(
          get_1d_histogram_model(axis,sample_idx),axis.variable_name,samples[sample_idx]->weight_column));
        ROOT::RDF::RInterface<ROOT::Detail::RDF::RJittedFilter, void> numerator_data_frame = samples[sample_idx]->data_frame().Filter(numerator_cut);
        histograms[sample_idx].push_back(numerator_data_frame.Histo1D(
          get_1d_histogram_model(axis,sample_idx),axis.variable_name,samples[sample_idx]->weight_column));
      }
      else {
      }
        denominator_histograms[sample_idx].push_back(samples[sample_idx]->data_frame().Histo1D(
          get_1d_histogram_model(axis,sample_idx),axis.variable_name));
        ROOT::RDF::RInterface<ROOT::Detail::RDF::RJittedFilter, void> numerator_data_frame = samples[sample_idx]->data_frame().Filter(numerator_cut);
        histograms[sample_idx].push_back(numerator_data_frame.Histo1D(
          get_1d_histogram_model(axis,sample_idx),axis.variable_name));
    }
  }
  return new PlotCollection(axis, histograms, denominator_histograms, samples, numerator_description, regions);
}

///**
// * method to make 1d efficiency plots of variable with weight weight in each region specified by regions, see RInterface::Histo1D
// */
//HistogramCollection SampleCollection::book_1d_efficiency_plot(RegionCollection regions, int nbins, double xlow, double xhigh, std::string variable, std::string description, std::string_view weight, std::string numerator_cut, std::string numerator_description) {
//	std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> histograms;
//	std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> denominator_histograms;
//	std::vector<SampleWrapper*> sample_pointers;
//	std::string internal_description = description;
//	if (description=="") internal_description = variable;
//	//loop over samples
//	for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
//		histograms.push_back(std::vector<ROOT::RDF::RResultPtr<TH1D>>());
//		denominator_histograms.push_back(std::vector<ROOT::RDF::RResultPtr<TH1D>>());
//		sample_pointers.push_back(&samples[sample_idx]);
//		//loop over regions
//		for (unsigned int region_idx = 0; region_idx < regions->size(); region_idx++) {
//			//filter sample to region
//			ROOT::RDF::RInterface<ROOT::Detail::RDF::RJittedFilter, void> region_data_frame = samples[sample_idx].data_frame().Filter(regions->get_cuts(region_idx, sample_flags[sample_idx]));
//			denominator_histograms[sample_idx].push_back(region_data_frame.Histo1D({
//				("hist_"+variable+"_"+samples[sample_idx].sample_name+"_"+regions->get_name(region_idx)).c_str(),
//				(internal_description+" "+baseline_selection_descriptions[sample_idx]+", "+regions->get_description(region_idx)+"; "+internal_description).c_str(),
//				nbins,xlow,xhigh},variable,weight));
//			region_data_frame = region_data_frame.Filter(numerator_cut);
//			histograms[sample_idx].push_back(region_data_frame.Histo1D({
//				("hist_"+variable+"_"+samples[sample_idx].sample_name+"_"+regions->get_name(region_idx)).c_str(),
//				(internal_description+" "+baseline_selection_descriptions[sample_idx]+", "+regions->get_description(region_idx)+"; "+internal_description+"; Efficiency "+numerator_description).c_str(),
//				nbins,xlow,xhigh},variable,weight));
//		}
//	}
//	return HistogramCollection(variable, internal_description, histograms, denominator_histograms, sample_pointers, regions);
//}

ROOT::RDF::TH2DModel SampleCollection::get_2d_histogram_model(VariableAxis x_axis, VariableAxis y_axis, unsigned int sample_idx, RegionCollection* regions, unsigned int region_idx) {
  std::string hist_name = "hist_"+y_axis.variable_name+"_"+x_axis.variable_name+"_"
    +samples[sample_idx]->sample_name;
  std::string hist_description = y_axis.variable_description+" vs "+x_axis.variable_description+", "
    +samples[sample_idx]->selection_string();
  if (regions != nullptr) {
    hist_name = hist_name + "_" + regions->get_name(region_idx);
    hist_description = hist_description+", "+regions->get_description(region_idx);
  }
  hist_description = hist_description+"; "+x_axis.variable_description+" "+x_axis.formatted_units()
    +y_axis.variable_description+" "+y_axis.formatted_units()+"; Events/"+x_axis.bin_size()+"/"+y_axis.bin_size();
  if (x_axis.uniform_bins) {
    if (y_axis.uniform_bins) {
      return ROOT::RDF::TH2DModel(hist_name.c_str(),hist_description.c_str(),x_axis.nbins,x_axis.low,x_axis.high,
        y_axis.nbins,y_axis.low,y_axis.high);
    }
    return ROOT::RDF::TH2DModel(hist_name.c_str(),hist_description.c_str(),x_axis.nbins,x_axis.low,x_axis.high,
      y_axis.nbins,y_axis.bins);
  }
  if (y_axis.uniform_bins) {
    return ROOT::RDF::TH2DModel(hist_name.c_str(),hist_description.c_str(),x_axis.nbins,x_axis.bins,
      y_axis.nbins,y_axis.low,y_axis.high);
  }
  return ROOT::RDF::TH2DModel(hist_name.c_str(),hist_description.c_str(),x_axis.nbins,x_axis.bins,y_axis.nbins,y_axis.bins);
}

/**
 * method to make 2d histograms of variable with weight weight in each region specified by regions, see RInterface::Histo2D
 */
PlotCollection* SampleCollection::book_2d_histogram(VariableAxis x_axis, VariableAxis y_axis, RegionCollection* regions) {
  std::vector<std::vector<ROOT::RDF::RResultPtr<TH2D>>> histograms;
  //loop over samples
  for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
    histograms.push_back(std::vector<ROOT::RDF::RResultPtr<TH2D>>());
    if (regions != nullptr) {
      //loop over regions
      for (unsigned int region_idx = 0; region_idx < regions->size(); region_idx++) {
        //filter sample to region
        ROOT::RDF::RInterface<ROOT::Detail::RDF::RJittedFilter, void> region_data_frame = samples[sample_idx]->data_frame().Filter(regions->get_cuts(region_idx, samples[sample_idx]));
	if (samples[sample_idx]->weighted_sample) {
          histograms[sample_idx].push_back(region_data_frame.Histo2D(
            get_2d_histogram_model(x_axis,y_axis,sample_idx,regions,region_idx),x_axis.variable_name,y_axis.variable_name,samples[sample_idx]->weight_column));
	}
	else {
          histograms[sample_idx].push_back(region_data_frame.Histo2D(
            get_2d_histogram_model(x_axis,y_axis,sample_idx,regions,region_idx),x_axis.variable_name,y_axis.variable_name));
	}
      }  
    }
    else {
      //no regions
      if (samples[sample_idx]->weighted_sample) {
        histograms[sample_idx].push_back(samples[sample_idx]->data_frame().Histo2D(
          get_2d_histogram_model(x_axis,y_axis,sample_idx),x_axis.variable_name,y_axis.variable_name,samples[sample_idx]->weight_column));
      }
      else {
        histograms[sample_idx].push_back(samples[sample_idx]->data_frame().Histo2D(
          get_2d_histogram_model(x_axis,y_axis,sample_idx),x_axis.variable_name,y_axis.variable_name));
      }
    }
  }
  return new PlotCollection(x_axis, y_axis, histograms, samples, regions);
}

/**
 * method to make 2d efficiency plots of variable with weight weight in each region specified by regions, see RInterface::Histo2D
 */
PlotCollection* SampleCollection::book_2d_efficiency_plot(VariableAxis x_axis, VariableAxis y_axis, std::string numerator_cut, std::string numerator_description, RegionCollection* regions) {
  std::vector<std::vector<ROOT::RDF::RResultPtr<TH2D>>> histograms;
  std::vector<std::vector<ROOT::RDF::RResultPtr<TH2D>>> denominator_histograms;
  //loop over samples
  for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
    histograms.push_back(std::vector<ROOT::RDF::RResultPtr<TH2D>>());
    denominator_histograms.push_back(std::vector<ROOT::RDF::RResultPtr<TH2D>>());
    if (regions != nullptr) {
      //loop over regions
      for (unsigned int region_idx = 0; region_idx < regions->size(); region_idx++) {
        //filter sample to region
        ROOT::RDF::RInterface<ROOT::Detail::RDF::RJittedFilter, void> region_data_frame = samples[sample_idx]->data_frame().Filter(regions->get_cuts(region_idx, samples[sample_idx]));
	if (samples[sample_idx]->weighted_sample) {
          denominator_histograms[sample_idx].push_back(region_data_frame.Histo2D(
            get_2d_histogram_model(x_axis,y_axis,sample_idx,regions,region_idx),x_axis.variable_name,y_axis.variable_name,samples[sample_idx]->weight_column));
          region_data_frame = region_data_frame.Filter(numerator_cut);
          histograms[sample_idx].push_back(region_data_frame.Histo2D(
            get_2d_histogram_model(x_axis,y_axis,sample_idx,regions,region_idx),x_axis.variable_name,y_axis.variable_name,samples[sample_idx]->weight_column));
	}
	else {
          denominator_histograms[sample_idx].push_back(region_data_frame.Histo2D(
            get_2d_histogram_model(x_axis,y_axis,sample_idx,regions,region_idx),x_axis.variable_name,y_axis.variable_name));
          region_data_frame = region_data_frame.Filter(numerator_cut);
          histograms[sample_idx].push_back(region_data_frame.Histo2D(
            get_2d_histogram_model(x_axis,y_axis,sample_idx,regions,region_idx),x_axis.variable_name,y_axis.variable_name));
	}
      }  
    }
    else {
      //no regions
      if (samples[sample_idx]->weighted_sample) {
        denominator_histograms[sample_idx].push_back(samples[sample_idx]->data_frame().Histo2D(
          get_2d_histogram_model(x_axis,y_axis,sample_idx),x_axis.variable_name,y_axis.variable_name,samples[sample_idx]->weight_column));
        ROOT::RDF::RInterface<ROOT::Detail::RDF::RJittedFilter, void> numerator_data_frame = samples[sample_idx]->data_frame().Filter(numerator_cut);
        histograms[sample_idx].push_back(numerator_data_frame.Histo2D(
          get_2d_histogram_model(x_axis,y_axis,sample_idx),x_axis.variable_name,y_axis.variable_name,samples[sample_idx]->weight_column));
      }
      else {
        denominator_histograms[sample_idx].push_back(samples[sample_idx]->data_frame().Histo2D(
          get_2d_histogram_model(x_axis,y_axis,sample_idx),x_axis.variable_name,y_axis.variable_name));
        ROOT::RDF::RInterface<ROOT::Detail::RDF::RJittedFilter, void> numerator_data_frame = samples[sample_idx]->data_frame().Filter(numerator_cut);
        histograms[sample_idx].push_back(numerator_data_frame.Histo2D(
          get_2d_histogram_model(x_axis,y_axis,sample_idx),x_axis.variable_name,y_axis.variable_name));
      }
    }
  }
  return new PlotCollection(x_axis, y_axis, histograms, denominator_histograms, samples, 
    numerator_description, regions);
}

///**
// * method to make 2d efficiency plots of variable with weight weight in each region specified by regions, see RInterface::Histo2D
// */
//HistogramCollection SampleCollection::book_2d_efficiency_plot(RegionCollection regions, int xnbins, double* xbins, std::string xvariable, std::string xdescription, int ynbins, double* ybins, std::string yvariable, std::string ydescription, std::string_view weight, std::string numerator_cut, std::string numerator_description) {
//	std::vector<std::vector<ROOT::RDF::RResultPtr<TH2D>>> histograms;
//	std::vector<std::vector<ROOT::RDF::RResultPtr<TH2D>>> denominator_histograms;
//	std::vector<SampleWrapper*> sample_pointers;
//	std::string x_internal_description = xdescription;
//	std::string y_internal_description = ydescription;
//	if (xdescription=="") x_internal_description = xvariable;
//	if (ydescription=="") y_internal_description = yvariable;
//	//loop over samples
//	for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
//		histograms.push_back(std::vector<ROOT::RDF::RResultPtr<TH2D>>());
//		denominator_histograms.push_back(std::vector<ROOT::RDF::RResultPtr<TH2D>>());
//		sample_pointers.push_back(&samples[sample_idx]);
//		//loop over regions
//		for (unsigned int region_idx = 0; region_idx < regions->size(); region_idx++) {
//			//filter sample to region
//			ROOT::RDF::RInterface<ROOT::Detail::RDF::RJittedFilter, void> region_data_frame = samples[sample_idx].data_frame().Filter(regions->get_cuts(region_idx, sample_flags[sample_idx]));
//			denominator_histograms[sample_idx].push_back(region_data_frame.Histo2D({
//				("hist_"+xvariable+"_"+yvariable+"_"+samples[sample_idx].sample_name+"_"+regions->get_name(region_idx)).c_str(),
//				(y_internal_description+" vs. "+x_internal_description+" "+baseline_selection_descriptions[sample_idx]+", "+regions->get_description(region_idx)+"; "+x_internal_description+"; "+y_internal_description).c_str(),
//				xnbins,xbins,ynbins,ybins},xvariable,yvariable,weight));
//			region_data_frame = region_data_frame.Filter(numerator_cut);
//			histograms[sample_idx].push_back(region_data_frame.Histo2D({
//				("hist_"+xvariable+"_"+yvariable+"_"+samples[sample_idx].sample_name+"_"+regions->get_name(region_idx)).c_str(),
//				("Efficiency "+numerator_description+" "+y_internal_description+" vs. "+x_internal_description+" "+baseline_selection_descriptions[sample_idx]+", "+regions->get_description(region_idx)+"; "+x_internal_description+"; "+y_internal_description).c_str(),
//				xnbins,xbins,ynbins,ybins},xvariable,yvariable,weight));
//		}
//	}
//	return HistogramCollection(xvariable, x_internal_description, yvariable, y_internal_description, histograms, denominator_histograms, sample_pointers, regions);
//}

/**
 * method to make cutflow table, see RInterface::RCutFlowReport
 */
TableCollection* SampleCollection::book_cutflow_table() {
  std::vector<ROOT::RDF::RResultPtr<ROOT::RDF::RCutFlowReport>> tables;
  for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
    tables.push_back(samples[sample_idx]->data_frame().Report());
  }
  return new TableCollection(tables, samples);
}
