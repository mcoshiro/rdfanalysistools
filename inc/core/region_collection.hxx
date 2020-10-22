#ifndef H_REGION_COLLECTION
#define H_REGION_COLLECTION

#include <string>
#include <vector>
#include <utility>

//class to hold a collection of regions (i.e. cuts), used to make tables plots in many different regions
//for example, one could have regions be njets=0, 1, 2 or nmu=0, 1, 2

class RegionCollection {
  private:
    std::vector<std::string> region_names;
    std::vector<std::string> region_descriptions;
    std::vector<std::vector<std::pair<std::string, std::string>>> region_cuts_by_flag;
    std::vector<std::string> region_cuts_default;
  
  public:
    /**
     * default constructor
     */
    RegionCollection();
    
    /**
     * method to add a region
     */
    void add(std::string name, std::string cuts, std::string description="");
    
    /**
     * change the cuts in a particular region for samples with a particular flag
     */
    void set_flag_cuts(std::string name, std::string flag, std::string cuts);
    
    /**
     * get the name for a particular region
     */
    std::string get_name (unsigned int region_idx);
    
    /**
     * get the description for a particular region
     */
    std::string get_description (unsigned int region_idx);
    
    /**
     * returns number of regions in collection
     */
    unsigned int size();
    
    /**
     * get the cuts for an appropriate flag type in an appropriate region
     */
    std::string get_cuts(unsigned int region_idx, SampleWrapper* sample);
};

#endif
