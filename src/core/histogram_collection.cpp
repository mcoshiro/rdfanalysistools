#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "TStyle.h"
#include "TLegend.h"
#include "TCanvas.h"
#include "TH1.h"
#include "ROOT/RResultPtr.hxx"
#include "ROOT/RDF/InterfaceUtils.hxx"

#include "../../inc/core/sample_wrapper.hxx"
#include "../../inc/core/region_collection.hxx"
#include "../../inc/core/histogram_collection.hxx"

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
 * constructor to generate collection from a vector of vectors
 */
HistogramCollection::HistogramCollection(std::string i_name, std::string i_description, std::vector<std::vector<ROOT::RDF::RResultPtr<TH1D>>> i_histograms, std::vector<SampleWrapper*> i_samples, RegionCollection i_regions)
	: samples(i_samples)
	, regions(i_regions)
{
	name = i_name;
	description = i_description;
	histograms = i_histograms;
	luminosity = 1.0;
}

/**
 * function to set luminosity
 */
void HistogramCollection::set_luminosity(float i_luminosity) {
	luminosity = i_luminosity;
}

/**
 * function to draw several TH1's on top of each other
 */
void HistogramCollection::overlay_1d_histograms() {
	if (histograms.size() < 1) {
		std::cout << "ERROR: draw before histograms are booked" << std::endl;
		return;
	}
	gStyle->SetOptStat(0);
	//loop over regions
	for (unsigned int region_idx = 0; region_idx < regions.size(); region_idx++) {
		TCanvas* c = new TCanvas((regions.get_name(region_idx)+"_canvas").c_str());
		c->cd();
		TLegend* legend = new TLegend(0.7,0.7,0.9,0.9);
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
			if (!ordered_histograms[sample_idx].is_data) {
				ordered_histograms[sample_idx].histogram->Scale(luminosity);
			}
			ordered_histograms[sample_idx].histogram->SetLineColor(ordered_histograms[sample_idx].color);
			if (sample_idx == 0) {
				ordered_histograms[sample_idx].histogram->DrawClone();
			}
			else {
				ordered_histograms[sample_idx].histogram->DrawClone("same");
			}
			legend->AddEntry(ordered_histograms[sample_idx].histogram->GetName(),ordered_histograms[sample_idx].description.c_str());
		}
		legend->Draw();
		c->SaveAs(("plots/"+name+"_overlay_"+regions.get_name(region_idx)+".png").c_str());
	}

}
