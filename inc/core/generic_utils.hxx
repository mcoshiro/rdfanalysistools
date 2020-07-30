#ifndef H_GENERIC_UTILS
#define H_GENERIC_UTILS

//generic functions that may be useful

/**
 * function returning delta phi between two particles
 */
float delta_phi(float phi1, float phi2);

/**
 * function returning delta R between two particles
 */
float delta_r(float eta1, float phi1, float eta2, float phi2);

#endif
