#include <iostream>
#include <string>
#include <vector>
#include <utility>

#include "core/sample_wrapper.hxx"
#include "core/region_collection.hxx"

/**
 * default constructor
 */
RegionCollection::RegionCollection() {
  //do nothing
}

/**
 * method to add a region
 */
void RegionCollection::add(std::string name, std::string cuts, std::string description) {
  //first check another region with the same name doesn't already exist
  for (unsigned int region_idx = 0; region_idx < region_names.size(); region_idx++) {
    if (region_names[region_idx] == name) {
      std::cout << "ERROR: attempt to redefine region " << name << std::endl;
      return;
    }
  }
  region_names.push_back(name);
  if (description != "") {
    region_descriptions.push_back(description);
  }
  else {
    region_descriptions.push_back(cuts);
  }
  region_cuts_default.push_back(cuts);
  region_cuts_by_flag.push_back({});
}

/**
 * change the cuts in a particular region for samples with a particular flag
 */
void RegionCollection::set_flag_cuts(std::string name, std::string flag, std::string cuts) {
  //find region
  bool found_region = false;
  for (unsigned int region_idx = 0; region_idx < region_names.size(); region_idx++) {
    if (region_names[region_idx] == name) {
      found_region = true;
      //check if flag is already in set
      bool flag_in_set = false;
      for (unsigned int flag_idx = 0; flag_idx < region_cuts_by_flag[region_idx].size(); flag_idx++) {
        if (region_cuts_by_flag[region_idx][flag_idx].first == flag) {
          flag_in_set = true;
          //overwrite cuts for this region
          region_cuts_by_flag[region_idx][flag_idx].second = cuts;
        }
      }
      if (!flag_in_set) {
        //add flag to set
        region_cuts_by_flag[region_idx].push_back(std::make_pair(flag,cuts));
      }
    }
  }
  if (!found_region) {
    std::cout << "ERROR: unable to find region " << name << std::endl;
  }
}

/**
 * get the name for a particular region
 */
std::string RegionCollection::get_name (unsigned int region_idx) {
  return region_names[region_idx];
}

/**
 * get the description for a particular region
 */
std::string RegionCollection::get_description (unsigned int region_idx) {
  return region_descriptions[region_idx];
}

/**
 * returns number of regions in collection
 */
unsigned int RegionCollection::size() {
  return region_names.size();
}

/**
 * get the cuts for an appropriate flag type in an appropriate region
 */
std::string RegionCollection::get_cuts(unsigned int region_idx, SampleWrapper* sample) {
  //loop through flags to see if there are special cuts for this flag
  for (unsigned int flag_idx = 0; flag_idx < region_cuts_by_flag[region_idx].size(); flag_idx++) {
    //return cut for first flag with specially assigned cuts
    if (sample->check_flag(region_cuts_by_flag[region_idx][flag_idx].first)) {
      return region_cuts_by_flag[region_idx][flag_idx].second;
    }
  }
  return region_cuts_default[region_idx];
}
