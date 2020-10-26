#include <string>
#include <sstream>
#include <iomanip>

#include "core/generic_utils.hxx"
#include "TMath.h"

/**
 * function that converts a float into a std::string with a fixed number of digits
 */
std::string float_to_string(float value, int precision) {
  std::ostringstream str_stream;
  str_stream << std::fixed;
  str_stream << std::setprecision(precision);
  str_stream << value;
  return str_stream.str();
}

/**
 * function returning delta phi between two particles
 */
float delta_phi(float phi1, float phi2) {
  return TMath::Min(TMath::Min(static_cast<Float_t>(TMath::Abs(phi2-phi1)),static_cast<Float_t>(TMath::Abs(phi2+2*TMath::Pi()-phi1))),static_cast<Float_t>(TMath::Abs(phi2-2*TMath::Pi()-phi1)));
}

/**
 * function returning delta R between two particles
 */
float delta_r(float eta1, float phi1, float eta2, float phi2) {
  return TMath::Sqrt(TMath::Power(eta2-eta1,2)+TMath::Power(delta_phi(phi1,phi2),2));
}

/**
 * function returning transverse mass of two physics objets
 */
float mt(float pt1, float phi1, float pt2, float phi2) {
  return TMath::Sqrt(2.0*pt1*pt2*(1.0-TMath::Cos(phi1-phi2)));
}
