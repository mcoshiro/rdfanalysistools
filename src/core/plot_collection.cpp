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

#include "../../inc/core/sample_wrapper.hxx"
#include "../../inc/core/region_collection.hxx"
#include "../../inc/core/plot_collection.hxx"

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
	draw_log = false;
	is_efficiency = false;
	is_2d = false;
	save_as_root = false;
	file_extension = "png";
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
	draw_log = false;
	is_efficiency = true;
	is_2d = false;
	save_as_root = false;
	file_extension = "png";
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
	draw_log = false;
	is_efficiency = false;
	is_2d = true;
	save_as_root = false;
	file_extension = "png";
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
	draw_log = false;
	is_efficiency = true;
	is_2d = true;
	save_as_root = false;
	file_extension = "png";
}

/**
 * function to save root file
 */
void PlotCollection::set_save_root_file(bool i_set_save_root_file) {
	save_as_root = i_set_save_root_file;
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
 * function to set log y
 */
void PlotCollection::set_draw_log(bool i_draw_log) {
	draw_log = i_draw_log;
}

/**
 * function to set file extension
 */
void PlotCollection::set_file_extension(std::string i_file_extension) {
	file_extension = i_file_extension;
}


/**
 * function to draw several TH1's overlayed on each other
 */
void PlotCollection::overlay_1d_histograms() {
	//TODO: allow overlaying 1d efficiencies
	if (histograms.size() < 1) {
		std::cout << "ERROR: draw before histograms are booked" << std::endl;
		return;
	}
	if (is_2d) {
		std::cout << "ERROR: cannot overlay 2d histograms" << std::endl;
		return;
	}
	gStyle->SetOptStat(0);
	//loop over regions
	for (unsigned int region_idx = 0; region_idx < regions->size(); region_idx++) {
		TCanvas* c = new TCanvas((regions->get_name(region_idx)+"_canvas").c_str());
		if (draw_log) {
			c->SetLogy(true);
		}
		c->cd();
		TLegend* legend = new TLegend(0.75,0.75,0.9,0.9);
		//draw highest maximum histogram first so that canvas is scaled appropriately
		std::vector<HistogramAndStyle> ordered_histograms;
		for (unsigned int sample_idx = 0; sample_idx < histograms.size(); sample_idx++) {
			ordered_histograms.push_back(HistogramAndStyle());
			ordered_histograms[sample_idx].histogram = histograms[sample_idx][region_idx];
			ordered_histograms[sample_idx].color = samples[sample_idx]->sample_color;
			ordered_histograms[sample_idx].description = samples[sample_idx]->sample_description;
			ordered_histograms[sample_idx].is_data = samples[sample_idx]->is_data;
		}
		std::sort(ordered_histograms.begin(),ordered_histograms.end(),sort_by_maximum);
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
		c->SaveAs(("plots/"+name+"_overlay_"+regions->get_name(region_idx)+"."+file_extension).c_str());
	}
}


/**
 * function to draw several TH1's stacked on each other; samples marked as 'data' will be drawn over the stack rather than in it
 */
void PlotCollection::stack_1d_histograms(bool sort_histograms) {
	if (histograms.size() < 1) {
		std::cout << "ERROR: draw before histograms are booked" << std::endl;
		return;
	}
	if (is_2d) {
		std::cout << "ERROR: cannot stack 2d histograms" << std::endl;
		return;
	}
	if (is_efficiency) {
		std::cout << "ERROR: cannot stack ratios" << std::endl;
		return;
	}
	gStyle->SetOptStat(0);
	//loop over regions
	for (unsigned int region_idx = 0; region_idx < regions->size(); region_idx++) {
		TCanvas* c = new TCanvas((regions->get_name(region_idx)+"_canvas").c_str());
		if (draw_log) {
			c->SetLogy(true);
		}
		c->cd();
		THStack* hist_stack = new THStack((regions->get_name(region_idx)+"_hist_stack").c_str(),histograms[0][region_idx]->GetTitle());
		TLegend* legend = new TLegend(0.75,0.75,0.9,0.9);
		std::vector<HistogramAndStyle> ordered_histograms;
		for (unsigned int sample_idx = 0; sample_idx < histograms.size(); sample_idx++) {
			if (!samples[sample_idx]->is_data) {
				ordered_histograms.push_back(HistogramAndStyle());
				ordered_histograms[sample_idx].histogram = histograms[sample_idx][region_idx];
				ordered_histograms[sample_idx].color = samples[sample_idx]->sample_color;
				ordered_histograms[sample_idx].description = samples[sample_idx]->sample_description;
			}
		}
		if (sort_histograms) {
			std::sort(ordered_histograms.begin(),ordered_histograms.end(),sort_by_integral);
		}
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
		data_hist->Draw();
		//is title already okay?
		data_hist->GetYaxis()->SetRangeUser(0,y_axis_max);
		data_hist->Draw("e0");
		hist_stack->Draw("same");
		data_hist->Draw("e0 same");
		legend->Draw();
		//draw any data
		c->SaveAs(("plots/"+name+"_stack_"+regions->get_name(region_idx)+"."+file_extension).c_str());
	}
}

/**
 * function to draw several TH1's stacked on each other; samples marked as 'data' will be drawn over the stack rather than in it
 * additionally, draw a data/MC ratio plot
 */
void PlotCollection::stack_ratio_1d_histograms(bool sort_histograms) {
	if (histograms.size() < 1) {
		std::cout << "ERROR: draw before histograms are booked" << std::endl;
		return;
	}
	if (is_2d) {
		std::cout << "ERROR: cannot stack 2d histograms" << std::endl;
		return;
	}
	if (is_efficiency) {
		std::cout << "ERROR: cannot stack ratios" << std::endl;
		return;
	}
	gStyle->SetOptStat(0);
	//loop over regions
	for (unsigned int region_idx = 0; region_idx < regions->size(); region_idx++) {
		//split canvas into two pads; upper pad draws histograms lower pad draws ratio
		TCanvas* c = new TCanvas((regions->get_name(region_idx)+"_canvas").c_str());
		TPad* stack_pad = new TPad("stack_pad","",0.0,0.2,1.0,1.0);
		TPad* ratio_pad = new TPad("ratio_pad","",0.0,0.0,1.0,0.2);
		stack_pad->Draw();
		ratio_pad->Draw();
		stack_pad->cd();
		gPad->SetBottomMargin(0);
		//draw THStack
		THStack* hist_stack = new THStack((regions->get_name(region_idx)+"_hist_stack").c_str(),histograms[0][region_idx]->GetTitle());
		TLegend* legend = new TLegend(0.75,0.75,0.9,0.9);
		std::vector<HistogramAndStyle> ordered_histograms;
		for (unsigned int sample_idx = 0; sample_idx < histograms.size(); sample_idx++) {
			if (!samples[sample_idx]->is_data) {
				ordered_histograms.push_back(HistogramAndStyle());
				ordered_histograms[sample_idx].histogram = histograms[sample_idx][region_idx];
				ordered_histograms[sample_idx].color = samples[sample_idx]->sample_color;
				ordered_histograms[sample_idx].description = samples[sample_idx]->sample_description;
			}
		}
		if (sort_histograms) {
			std::sort(ordered_histograms.begin(),ordered_histograms.end(),sort_by_integral);
		}
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
		data_hist->Draw();
		data_hist->GetYaxis()->SetRangeUser(0,y_axis_max);
		data_hist->Draw("e0");
		hist_stack->Draw("same");
		hist_stack->GetHistogram()->GetXaxis()->SetTickLength(0);
		hist_stack->GetHistogram()->GetXaxis()->SetLabelOffset(999);
		data_hist->Draw("e0 same");
		data_hist->GetXaxis()->SetTickLength(0);
		data_hist->GetXaxis()->SetLabelOffset(999);
		legend->Draw("same");
		if (draw_log) {
			stack_pad->SetLogy(true);
		}
		stack_pad->Modified();
		//draw ratio plot
		ratio_pad->cd();
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
		ratio_pad->Modified();
		c->Update();
		c->SaveAs(("plots/"+name+"_stack_ratio_"+regions->get_name(region_idx)+"."+file_extension).c_str());
	}
}

/**
 * function to draw several histograms separately
 */
void PlotCollection::draweach_histograms() {
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
				if (draw_log) {
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
				if (draw_log) {
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
				if (draw_log) {
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
				if (draw_log) {
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
