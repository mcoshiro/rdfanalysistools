//collection of functions used for ttz analysis
#ifndef H_TTZ_PICO_UTILS
#define H_TTZ_PICO_UTILS

#include <vector> 

#include "ROOT/RVec.hxx"
#include "../../inc/ttz/ttz_pico_utils.hxx"

/**
 * z_m - returns mass of z-candidate
 */
float z_m(ROOT::VecOps::RVec<float> const &ll_m, int const &z_idx);

#endif
