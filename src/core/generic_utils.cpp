#include "../../inc/core/generic_utils.hxx"
#include "TMath.h"

/**
 * function returning delta phi between two particles
 */
float delta_phi(float phi1, float phi2) {
	//function to get Delta Phi between two particles
	return TMath::Min(TMath::Min(static_cast<Float_t>(TMath::Abs(phi2-phi1)),static_cast<Float_t>(TMath::Abs(phi2+2*3.1415-phi1))),static_cast<Float_t>(TMath::Abs(phi2-2*3.1415-phi1)));
}

/**
 * function returning delta R between two particles
 */
float delta_r(float eta1, float phi1, float eta2, float phi2) {
	//function that returns Delta R given eta and phi of particles 1 and 2
	return TMath::Sqrt(TMath::Power(eta2-eta1,2)+TMath::Power(delta_phi(phi1,phi2),2));
}
