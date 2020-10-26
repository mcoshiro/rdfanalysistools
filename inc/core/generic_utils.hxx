#ifndef H_GENERIC_UTILS
#define H_GENERIC_UTILS

//generic functions that may be useful

/**
 * function that converts a float into a std::string with a fixed number of digits
 */
std::string float_to_string(float value, int precision);

/**
 * function returning delta phi between two particles
 */
float delta_phi(float phi1, float phi2);

/**
 * function returning delta R between two particles
 */
float delta_r(float eta1, float phi1, float eta2, float phi2);

/**
 * function returning transverse mass of two physics objets
 */
float mt(float pt1, float phi1, float pt2, float phi2);

#endif
