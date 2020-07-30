#include <vector> 

#include "ROOT/RVec.hxx"
#include "../../inc/ttz/ttz_pico_utils.hxx"

/**
 * z_m - returns mass of z-candidate
 */
float z_m(ROOT::VecOps::RVec<float> const &ll_m, int const &z_idx) {
	if (z_idx == -1) {
		return -999;
	}
	return ll_m[static_cast<unsigned int>(z_idx)];
}
