#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "TStyle.h"
#include "TLegend.h"
#include "TCanvas.h"
#include "TH1.h"
#include "THStack.h"
#include "TGraphAsymmErrors.h"
#include "ROOT/RResultPtr.hxx"
#include "ROOT/RDF/InterfaceUtils.hxx"

#include "core/sample_wrapper.hxx"
#include "core/region_collection.hxx"
#include "core/plot_collection.hxx"

/**
 * sorting function to sort histograms by integrated area
 */
bool sort_by_integral(HistogramAndStyle hist_a, HistogramAndStyle hist_b) {
  if (hist_a.histogram->Integral() < hist_b.histogram->Integral())
    return true;
  return false;
}

/**
 * sorting function to sort histograms based on their maximum value
 */
bool sort_by_maximum(HistogramAndStyle hist_a, HistogramAndStyle hist_b) {
  if (hist_a.histogram->GetBinContent(hist_a.histogram->GetMaximumBin()) > hist_b.histogram->GetBinContent(hist_b.histogram->GetMaximumBin()))
    return true;
  return false;
}

/**
 * constructor to generate collection from a vector of vectors for 1d histograms
 */
PlotCollection::PlotCollection(VariableAxis axis, std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> i_histograms, std::vector<SampleWrapper*> i_samples, RegionCollection* i_regions)
  : samples(i_samples)
{
  regions = i_regions;
  name = axis.variable_name;
  description = axis.variable_description;
  histograms = i_histograms;
  luminosity = 1.0;
  x_log = false;
  y_log = false;
  z_log = false;
  is_efficiency = false;
  is_2d = false;
  save_as_root = false;
  file_extension = "png";
  plot_combine_style = PlotCombineStyle::overlay;
  bottom_style = BottomStyle::none;
}

/**
 * constructor to generate collection from a vector of vectors for 1d efficiencies
 */
PlotCollection::PlotCollection(VariableAxis axis, std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> i_histograms, std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> i_denominator_histograms, std::vector<SampleWrapper*> i_samples, std::string numerator_description, RegionCollection* i_regions)
  : samples(i_samples)
{
  std::string temp = numerator_description; //temp to avoid unused variable
  regions = i_regions;
  name = axis.variable_name;
  description = axis.variable_description;
  histograms = i_histograms;
  denominator_histograms = i_denominator_histograms;
  luminosity = 1.0;
  x_log = false;
  y_log = false;
  z_log = false;
  is_efficiency = true;
  is_2d = false;
  save_as_root = false;
  file_extension = "png";
  plot_combine_style = PlotCombineStyle::overlay;
  bottom_style = BottomStyle::none;
}

/**
 * constructor to generate collection from a vector of vectors for 2d histograms
 */
PlotCollection::PlotCollection(VariableAxis x_axis, VariableAxis y_axis, std::vector<std::vector<ROOT::RDF::RResultPtr<TH2D>>> i_twodim_histograms, std::vector<SampleWrapper*> i_samples, RegionCollection* i_regions)
  : samples(i_samples)
{
  regions = i_regions;
  name = x_axis.variable_name;
  description = x_axis.variable_description;
  yname = y_axis.variable_name;
  ydescription = y_axis.variable_description;
  twodim_histograms = i_twodim_histograms;
  luminosity = 1.0;
  x_log = false;
  y_log = false;
  z_log = false;
  is_efficiency = false;
  is_2d = true;
  save_as_root = false;
  file_extension = "png";
  plot_combine_style = PlotCombineStyle::overlay;
  bottom_style = BottomStyle::none;
}

/**
 * constructor to generate collection from a vector of vectors, for 2d efficiencies
 */
PlotCollection::PlotCollection(VariableAxis x_axis, VariableAxis y_axis, std::vector<std::vector<ROOT::RDF::RResultPtr<TH2D>>> i_twodim_histograms, std::vector<std::vector<ROOT::RDF::RResultPtr<TH2D>>> i_twodim_denominator_histograms, std::vector<SampleWrapper*> i_samples, std::string numerator_description, RegionCollection* i_regions)
  : samples(i_samples)
{
  std::string temp = numerator_description; //temp to avoid unused variable
  regions = i_regions;
  name = x_axis.variable_name;
  description = x_axis.variable_description;
  yname = y_axis.variable_name;
  ydescription = y_axis.variable_description;
  twodim_histograms = i_twodim_histograms;
  twodim_denominator_histograms = i_twodim_denominator_histograms;
  luminosity = 1.0;
  x_log = false;
  y_log = false;
  z_log = false;
  is_efficiency = true;
  is_2d = true;
  save_as_root = false;
  file_extension = "png";
  plot_combine_style = PlotCombineStyle::overlay;
  bottom_style = BottomStyle::none;
}

/**
 * function to set luminosity
 */
void PlotCollection::set_luminosity(float i_luminosity) {
	luminosity = i_luminosity;
	if (histograms.size() < 1) {
		std::cout << "ERROR: draw before histograms are booked" << std::endl;
		return;
	}
	//scaling will cancel off in ratios, so don't scale efficiencies
	if (!is_2d && !is_efficiency) {
		for (unsigned int region_idx = 0; region_idx < regions->size(); region_idx++) {
			for (unsigned int sample_idx = 0; sample_idx < histograms.size(); sample_idx++) {
				if (!samples[sample_idx]->is_data) {
					histograms[sample_idx][region_idx]->Scale(luminosity);
				}
			}
		}
	}
	if (is_2d && !is_efficiency) {
		for (unsigned int region_idx = 0; region_idx < regions->size(); region_idx++) {
			for (unsigned int sample_idx = 0; sample_idx < twodim_histograms.size(); sample_idx++) {
				if (!samples[sample_idx]->is_data) {
					twodim_histograms[sample_idx][region_idx]->Scale(luminosity);
				}
			}
		}
	}
}

/**
 * function to save root file
 */
PlotCollection* PlotCollection::set_save_root_file(bool i_set_save_root_file) {
  save_as_root = i_set_save_root_file;
  return this;
}

/**
 * function to set log options
 */
PlotCollection* PlotCollection::set_log(bool i_x_log, bool i_y_log, bool i_z_log) {
  x_log = i_x_log;
  y_log = i_y_log;
  z_log = i_z_log;
  return this;
}

/**
 * function to set file extension
 */
PlotCollection* PlotCollection::set_file_extension(std::string i_file_extension) {
  file_extension = i_file_extension;
  return this;
}

/**
 * function to set combine style option
 */
PlotCollection* PlotCollection::set_plot_combine_style(PlotCombineStyle i_plot_combine_style) {
  plot_combine_style = i_plot_combine_style;
  return this;
}

/**
 * function to set bottom style option
 */
PlotCollection* PlotCollection::set_bottom_style(BottomStyle i_bottom_style) {
  bottom_style = i_bottom_style;
  return this;
}


///**
// * internal function for plotting several plots together for a single region
// */
//void PlotCollection::draw_together_single_region(std::string canvas_name) {
//  TCanvas* c = new TCanvas((canvas_name+"_canvas").c_str());
//  c->cd();
//  TPad *main_pad, *bottom_pad;
//  if (bottom_style == BottomStyle::none) {
//    main_pad = new TPad("main_pad","",0.0,0.0,1.0,1.0);
//    main_pad->Draw();
//  }
//  else {
//    //TODO: implement other bottom types, currently just ratio
//    main_pad = new TPad("main_pad","",0.0,0.2,1.0,1.0);
//    bottom_pad = new TPad("bottom_pad","",0.0,0.0,1.0,0.2);
//    main_pad->Draw();
//    bottom_pad->Draw();
//  }
//  main_pad->cd();
//  if (x_log) main_pad->SetLogx(true);
//  if (y_log) main_pad->SetLogy(true);
//  if (bottom_style != BottomStyle::none) {
//    gPad->SetBottomMargin(0);
//  }
//  TLegend* legend = new TLegend(0.75,0.75,0.9,0.9);
//  std::vector<HistogramAndStyle> ordered_histograms;
//  for (unsigned int sample_idx = 0; sample_idx < histograms.size(); sample_idx++) {
//    if (plot_combine_style == PlotCombineStyle::stack && samples[sample_idx]->is_data) {
//      //for stacks, treat data separately from MC
//    }
//    else {
//      ordered_histograms.push_back(HistogramAndStyle());
//      ordered_histograms[sample_idx].histogram = histograms[sample_idx][region_idx];
//      ordered_histograms[sample_idx].color = samples[sample_idx]->sample_color;
//      ordered_histograms[sample_idx].description = samples[sample_idx]->sample_description;
//      ordered_histograms[sample_idx].is_data = samples[sample_idx]->is_data;
//    }
//  }
//  if (plot_combine_style == PlotCombineStyle::overlay) {
//    //if overlayed, draw highest maximum histogram first so that canvas is scaled appropriately
//    std::sort(ordered_histograms.begin(),ordered_histograms.end(),sort_by_maximum);
//  }
//  else {
//    if (sort_histograms) {
//      std::sort(ordered_histograms.begin(),ordered_histograms.end(),sort_by_integral);
//    }
//  }
//  if (plot_combine_style == PlotCombineStyle::overlay) {
//    //draw overlayed histograms
//    for (unsigned int sample_idx = 0; sample_idx < histograms.size(); sample_idx++) {
//      ordered_histograms[sample_idx].histogram->SetLineColor(ordered_histograms[sample_idx].color);
//      if (sample_idx == 0) {
//        ordered_histograms[sample_idx].histogram->DrawClone("e0");
//      }
//      else {
//        ordered_histograms[sample_idx].histogram->DrawClone("e0 same");
//      }
//      legend->AddEntry(ordered_histograms[sample_idx].histogram->GetName(),ordered_histograms[sample_idx].description.c_str());
//    }
//    legend->Draw();
//    main_pad->Modified();
//    c->Update();
//    c->SaveAs(("plots/"+name+"_overlay_"+regions->get_name(region_idx)+"."+file_extension).c_str());
//  }
//  else {
//    //draw stacked histograms
//    THStack* hist_stack = new THStack((regions->get_name(region_idx)+"_hist_stack").c_str(),histograms[0][region_idx]->GetTitle());
//    for (unsigned int mc_sample_idx = 0; mc_sample_idx < ordered_histograms.size(); mc_sample_idx++) {
//      ordered_histograms[mc_sample_idx].histogram->SetLineColor(ordered_histograms[mc_sample_idx].color);
//      ordered_histograms[mc_sample_idx].histogram->SetFillColor(ordered_histograms[mc_sample_idx].color);
//      TH1D* draw_mc_hist = static_cast<TH1D*>(ordered_histograms[mc_sample_idx].histogram->Clone()); 
//      hist_stack->Add(draw_mc_hist,"hist");
//      legend->AddEntry(draw_mc_hist,ordered_histograms[mc_sample_idx].description.c_str(),"f");
//    }
//    TH1D* data_hist;
//    bool has_data = false;
//    for (unsigned int sample_idx = 0; sample_idx < histograms.size(); sample_idx++) {
//      if (samples[sample_idx]->is_data) {
//        has_data = true;
//        data_hist = static_cast<TH1D*>(histograms[sample_idx][region_idx]->Clone());
//        data_hist->SetLineColor(samples[sample_idx]->sample_color);
//        legend->AddEntry(histograms[sample_idx][region_idx]->GetName(),(samples[sample_idx]->sample_description).c_str());
//      }
//    }
//    //set Y axis based on tallest bin
//    //make TH1D that is the sum of the MC histograms
//    TH1D* hist_ratio = new TH1D("hist_ratio",(";"+std::string(ordered_histograms[0].histogram->GetXaxis()->GetTitle())+";data/MC").c_str(),data_hist->GetNbinsX(),data_hist->GetXaxis()->GetXmin(),data_hist->GetXaxis()->GetXmax());
//    for (unsigned int mc_sample_idx = 0; mc_sample_idx < ordered_histograms.size(); mc_sample_idx++) {
//      hist_ratio->Add(static_cast<TH1D*>(ordered_histograms[mc_sample_idx].histogram->Clone()));
//    }
//    float mc_max = hist_ratio->GetBinContent(hist_ratio->GetMaximumBin());
//    float y_axis_max = mc_max;
//    if (has_data) {
//      float data_max = data_hist->GetBinContent(data_hist->GetMaximumBin());
//      y_axis_max = mc_max > data_max ? mc_max : data_max;
//    }
//    y_axis_max = y_axis_max * 1.15;
//    //draw any data
//    data_hist->Draw();
//    //is title already okay?
//    data_hist->GetYaxis()->SetRangeUser(0,y_axis_max);
//    data_hist->Draw("e0");
//    hist_stack->Draw("same");
//    data_hist->Draw("e0 same");
//    legend->Draw();
//    main_pad->Modified();
//    if (bottom_style != BottomStyle::none) {
//      //draw ratio plot
//      bottom_pad->cd();
//      gPad->SetTopMargin(0);
//      gPad->SetBottomMargin(0.22);
//      hist_ratio->Divide(data_hist,hist_ratio);
//      hist_ratio->GetYaxis()->SetRangeUser(0.4,1.6);
//      hist_ratio->SetTitleSize(0.1,"X");
//      hist_ratio->SetTitleSize(0.1,"Y");
//      hist_ratio->SetTitleOffset(1.0,"X");
//      hist_ratio->SetTitleOffset(0.4,"Y");
//      hist_ratio->SetLabelSize(0.1,"X");
//      hist_ratio->SetLabelSize(0.1,"Y");
//      hist_ratio->SetLineColor(kBlack);
//      hist_ratio->Draw("e0");
//      bottom_pad->Modified();
//    }
//    c->Update();
//    c->SaveAs(("plots/"+name+"_stack_"+regions->get_name(region_idx)+"."+file_extension).c_str());
//  }
//}


/**
 * function to draw several 1d plots together (stacked/overlayed)
 * for stacks, samples marked as 'data' will be drawn over the stack rather than in it
 * sort_histograms - organize non-data histograms so largest non-data histograms are on top of stack
 */
void PlotCollection::draw_together(bool sort_histograms) {
  //do checks
  if (histograms.size() < 1) {
    std::cout << "ERROR: draw before histograms are booked" << std::endl;
    return;
  }
  if (is_2d) {
    std::cout << "ERROR: drawing 2d histograms together is not implemented yet" << std::endl;
    return;
  }
  if (is_efficiency && plot_combine_style == PlotCombineStyle::stack) {
    std::cout << "ERROR: efficiencies cannot be stacked, automatically switching to overlay" << std::endl;
    plot_combine_style = PlotCombineStyle::overlay;
  }
  gStyle->SetOptStat(0);
  //regions
  if (regions != nullptr) {
    for (unsigned int region_idx = 0; region_idx < regions->size(); region_idx++) {
      TCanvas* c = new TCanvas((regions->get_name(region_idx)+"_canvas").c_str());
      c->cd();
      TPad *main_pad, *bottom_pad;
      if (bottom_style == BottomStyle::none) {
        main_pad = new TPad("main_pad","",0.0,0.0,1.0,1.0);
	main_pad->Draw();
      }
      else {
        //TODO: implement other bottom types, currently just ratio
        main_pad = new TPad("main_pad","",0.0,0.2,1.0,1.0);
        bottom_pad = new TPad("bottom_pad","",0.0,0.0,1.0,0.2);
	main_pad->Draw();
	bottom_pad->Draw();
      }
      main_pad->cd();
      if (x_log) main_pad->SetLogx(true);
      if (y_log) main_pad->SetLogy(true);
      if (bottom_style != BottomStyle::none) {
	gPad->SetBottomMargin(0);
      }
      TLegend* legend = new TLegend(0.75,0.75,0.9,0.9);
      std::vector<HistogramAndStyle> ordered_histograms;
      for (unsigned int sample_idx = 0; sample_idx < histograms.size(); sample_idx++) {
	if (plot_combine_style == PlotCombineStyle::stack && samples[sample_idx]->is_data) {
	  //for stacks, treat data separately from MC
	}
	else {
          ordered_histograms.push_back(HistogramAndStyle());
          ordered_histograms[sample_idx].histogram = histograms[sample_idx][region_idx];
          ordered_histograms[sample_idx].color = samples[sample_idx]->sample_color;
          ordered_histograms[sample_idx].description = samples[sample_idx]->sample_description;
          ordered_histograms[sample_idx].is_data = samples[sample_idx]->is_data;
	}
      }
      if (plot_combine_style == PlotCombineStyle::overlay) {
	//if overlayed, draw highest maximum histogram first so that canvas is scaled appropriately
        std::sort(ordered_histograms.begin(),ordered_histograms.end(),sort_by_maximum);
      }
      else {
        if (sort_histograms) {
          std::sort(ordered_histograms.begin(),ordered_histograms.end(),sort_by_integral);
        }
      }
      if (plot_combine_style == PlotCombineStyle::overlay) {
	//draw overlayed histograms
        for (unsigned int sample_idx = 0; sample_idx < histograms.size(); sample_idx++) {
          ordered_histograms[sample_idx].histogram->SetLineColor(ordered_histograms[sample_idx].color);
          if (sample_idx == 0) {
            ordered_histograms[sample_idx].histogram->DrawClone("e0");
          }
          else {
            ordered_histograms[sample_idx].histogram->DrawClone("e0 same");
          }
          legend->AddEntry(ordered_histograms[sample_idx].histogram->GetName(),ordered_histograms[sample_idx].description.c_str());
        }
        legend->Draw();
	main_pad->Modified();
        c->Update();
        c->SaveAs(("plots/"+name+"_overlay_"+regions->get_name(region_idx)+"."+file_extension).c_str());
      }
      else {
	//draw stacked histograms
        THStack* hist_stack = new THStack((regions->get_name(region_idx)+"_hist_stack").c_str(),histograms[0][region_idx]->GetTitle());
        for (unsigned int mc_sample_idx = 0; mc_sample_idx < ordered_histograms.size(); mc_sample_idx++) {
          ordered_histograms[mc_sample_idx].histogram->SetLineColor(ordered_histograms[mc_sample_idx].color);
          ordered_histograms[mc_sample_idx].histogram->SetFillColor(ordered_histograms[mc_sample_idx].color);
          TH1D* draw_mc_hist = static_cast<TH1D*>(ordered_histograms[mc_sample_idx].histogram->Clone()); 
          hist_stack->Add(draw_mc_hist,"hist");
          legend->AddEntry(draw_mc_hist,ordered_histograms[mc_sample_idx].description.c_str(),"f");
        }
        TH1D* data_hist;
        bool has_data = false;
        for (unsigned int sample_idx = 0; sample_idx < histograms.size(); sample_idx++) {
          if (samples[sample_idx]->is_data) {
            has_data = true;
            data_hist = static_cast<TH1D*>(histograms[sample_idx][region_idx]->Clone());
            data_hist->SetLineColor(samples[sample_idx]->sample_color);
            legend->AddEntry(histograms[sample_idx][region_idx]->GetName(),(samples[sample_idx]->sample_description).c_str());
          }
        }
        //set Y axis based on tallest bin
        //make TH1D that is the sum of the MC histograms
        TH1D* hist_ratio = new TH1D("hist_ratio",(";"+std::string(ordered_histograms[0].histogram->GetXaxis()->GetTitle())+";data/MC").c_str(),data_hist->GetNbinsX(),data_hist->GetXaxis()->GetXmin(),data_hist->GetXaxis()->GetXmax());
        for (unsigned int mc_sample_idx = 0; mc_sample_idx < ordered_histograms.size(); mc_sample_idx++) {
          hist_ratio->Add(static_cast<TH1D*>(ordered_histograms[mc_sample_idx].histogram->Clone()));
        }
        float mc_max = hist_ratio->GetBinContent(hist_ratio->GetMaximumBin());
        float y_axis_max = mc_max;
        if (has_data) {
          float data_max = data_hist->GetBinContent(data_hist->GetMaximumBin());
          y_axis_max = mc_max > data_max ? mc_max : data_max;
        }
        y_axis_max = y_axis_max * 1.15;
        //draw any data
        data_hist->Draw();
        //is title already okay?
        data_hist->GetYaxis()->SetRangeUser(0,y_axis_max);
        data_hist->Draw("e0");
        hist_stack->Draw("same");
        data_hist->Draw("e0 same");
        legend->Draw();
	main_pad->Modified();
	if (bottom_style != BottomStyle::none) {
          //draw ratio plot
          bottom_pad->cd();
          gPad->SetTopMargin(0);
          gPad->SetBottomMargin(0.22);
          hist_ratio->Divide(data_hist,hist_ratio);
          hist_ratio->GetYaxis()->SetRangeUser(0.4,1.6);
          hist_ratio->SetTitleSize(0.1,"X");
          hist_ratio->SetTitleSize(0.1,"Y");
          hist_ratio->SetTitleOffset(1.0,"X");
          hist_ratio->SetTitleOffset(0.4,"Y");
          hist_ratio->SetLabelSize(0.1,"X");
          hist_ratio->SetLabelSize(0.1,"Y");
          hist_ratio->SetLineColor(kBlack);
          hist_ratio->Draw("e0");
          bottom_pad->Modified();
	}
        c->Update();
        c->SaveAs(("plots/"+name+"_stack_"+regions->get_name(region_idx)+"."+file_extension).c_str());
      }
    }
  }
  //no regions
  else {
    //todo: implement this
  }
}

/**
 * function to draw several histograms separately
 */
void PlotCollection::draw_separate() {
	if (!is_2d) {
		draweach_1d_histograms();
	}
	else {
		draweach_2d_histograms();
	}
}

/**
 * function to draw several TH1's separately
 */
void PlotCollection::draweach_1d_histograms() {
	//TODO: allow draweach'ing 1d histograms
	if (histograms.size() < 1) {
		std::cout << "ERROR: draw before histograms are booked" << std::endl;
		return;
	}
	if (is_2d) {
		std::cout << "ERROR: draweach 1d called on 2d histograms" << std::endl;
		return;
	}
	gStyle->SetOptStat(0);
	//loop over regions
	if (!is_efficiency) {
		for (unsigned int region_idx = 0; region_idx < regions->size(); region_idx++) {
			for (unsigned int sample_idx = 0; sample_idx < histograms.size(); sample_idx++) {
				TCanvas* c = new TCanvas((regions->get_name(region_idx)+"_"+samples[sample_idx]->sample_name+"_canvas").c_str());
				if (z_log) {
					c->SetLogz(true);
				}
				c->cd();
				TH1D* cloned_hist = static_cast<TH1D*>(histograms[sample_idx][region_idx]->Clone());
				cloned_hist->SetTitle((samples[sample_idx]->sample_description+" "+cloned_hist->GetTitle()).c_str());
				cloned_hist->Draw("e0");
				c->SaveAs(("plots/"+name+"_"+samples[sample_idx]->sample_name+"_"+regions->get_name(region_idx)+"."+file_extension).c_str());
				if (save_as_root) {
					TFile *out_file = TFile::Open("ntuples/output.root","UPDATE");
					cloned_hist->Write();
					out_file->Close();
				}
			}
		}
	}
	else {
		for (unsigned int region_idx = 0; region_idx < regions->size(); region_idx++) {
			for (unsigned int sample_idx = 0; sample_idx < histograms.size(); sample_idx++) {
				TCanvas* c = new TCanvas((regions->get_name(region_idx)+"_"+samples[sample_idx]->sample_name+"_canvas").c_str());
				if (z_log) {
					c->SetLogz(true);
				}
				c->cd();
				TH1D* cloned_numerator = static_cast<TH1D*>(histograms[sample_idx][region_idx]->Clone());
				TH1D* cloned_denominator = static_cast<TH1D*>(denominator_histograms[sample_idx][region_idx]->Clone());
  				TGraphAsymmErrors* hist_ratio = new TGraphAsymmErrors(cloned_numerator,cloned_denominator,"cp");
				hist_ratio->SetTitle((samples[sample_idx]->sample_description+" "+cloned_numerator->GetTitle()).c_str());
				hist_ratio->GetXaxis()->SetTitle(cloned_numerator->GetXaxis()->GetTitle());
				hist_ratio->GetYaxis()->SetTitle(cloned_numerator->GetYaxis()->GetTitle());
				hist_ratio->Draw("AP");
				c->SaveAs(("plots/eff_"+name+"_"+samples[sample_idx]->sample_name+"_"+regions->get_name(region_idx)+"."+file_extension).c_str());
				if (save_as_root) {
					TFile *out_file = TFile::Open("ntuples/output.root","UPDATE");
					cloned_numerator->Write();
					cloned_denominator->Write();
					hist_ratio->Write();
					out_file->Close();
				}
			}
		}
	}
}

/**
 * function to draw several TH2's separately
 */
void PlotCollection::draweach_2d_histograms() {
	//TODO: allow draweach'ing 1d histograms
	if (twodim_histograms.size() < 1) {
		std::cout << "ERROR: draw before histograms are booked" << std::endl;
		return;
	}
	if (!is_2d) {
		std::cout << "ERROR: draweach 2d called on 1d histograms" << std::endl;
		return;
	}
	gStyle->SetOptStat(0);
	//loop over regions
	if (!is_efficiency) {
		for (unsigned int region_idx = 0; region_idx < regions->size(); region_idx++) {
			for (unsigned int sample_idx = 0; sample_idx < twodim_histograms.size(); sample_idx++) {
				TCanvas* c = new TCanvas((regions->get_name(region_idx)+"_"+samples[sample_idx]->sample_name+"_canvas").c_str());
				if (z_log) {
					c->SetLogz(true);
				}
				c->cd();
				TH2D* cloned_hist = static_cast<TH2D*>(twodim_histograms[sample_idx][region_idx]->Clone());
				cloned_hist->SetTitle((samples[sample_idx]->sample_description+" "+cloned_hist->GetTitle()).c_str());
				cloned_hist->Draw("colz");
				c->SaveAs(("plots/"+name+"_"+yname+"_"+samples[sample_idx]->sample_name+"_"+regions->get_name(region_idx)+"."+file_extension).c_str());
				if (save_as_root) {
					TFile *out_file = TFile::Open("ntuples/output.root","UPDATE");
					cloned_hist->Write();
					out_file->Close();
				}
			}
		}
	}
	else {
		for (unsigned int region_idx = 0; region_idx < regions->size(); region_idx++) {
			for (unsigned int sample_idx = 0; sample_idx < twodim_histograms.size(); sample_idx++) {
				TCanvas* c = new TCanvas((regions->get_name(region_idx)+"_"+samples[sample_idx]->sample_name+"_canvas").c_str());
				if (z_log) {
					c->SetLogz(true);
				}
				c->cd();
				TH2D* cloned_hist = static_cast<TH2D*>(twodim_histograms[sample_idx][region_idx]->Clone());
				TH2D* cloned_denominator = static_cast<TH2D*>(twodim_denominator_histograms[sample_idx][region_idx]->Clone());
				cloned_hist->Divide(cloned_denominator);
				cloned_hist->SetTitle((samples[sample_idx]->sample_description+" "+cloned_hist->GetTitle()).c_str());
				cloned_hist->Draw("colz");
				c->SaveAs(("plots/eff_"+name+"_"+yname+"_"+samples[sample_idx]->sample_name+"_"+regions->get_name(region_idx)+"."+file_extension).c_str());
				if (save_as_root) {
					TFile *out_file = TFile::Open("ntuples/output.root","UPDATE");
					TH2D* cloned_numerator = static_cast<TH2D*>(twodim_histograms[sample_idx][region_idx]->Clone());
					cloned_numerator->Write();
					cloned_denominator->Write();
					cloned_hist->Write();
					out_file->Close();
				}
			}
		}
	}
}
