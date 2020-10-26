//Check if your run is inJSON by calling bool inJSON(VRunLumi,run,lumiblock) in the event loop..
//ported from babymaker

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "higgsino/in_json.hxx"

using namespace std;

std::vector< std::vector<int> > MakeVRunLumi(std::string input){
  std::ifstream orgJSON;
  std::string fullpath = "";
  if(input == "golden2016"){
    fullpath = "data/json/golden_Cert_271036-284044_13TeV_23Sep2016ReReco_Collisions16.json";
  } else if(input == "golden2017"){
    fullpath = "data/json/golden_Cert_294927-306462_13TeV_EOY2017ReReco_Collisions17.json";
  } else if(input == "golden2018"){
    fullpath = "data/json/golden_Cert_314472-325175_13TeV_PromptReco_Collisions18.json";
  } else{
    fullpath = input;
  }
  orgJSON.open(fullpath.c_str());
  std::vector<int> VRunLumi;
  if(orgJSON.is_open()){
    char inChar;
    int inInt;
    std::string str;
    while(!orgJSON.eof()){
      char next = orgJSON.peek();
      if( next == '1' || next == '2' || next == '3' ||
          next == '4' || next == '5' || next == '6' ||
          next == '7' || next == '8' || next == '9' || 
          next == '0'){     
        orgJSON >>inInt;
        VRunLumi.push_back(inInt);        
      }
      else if(next == ' '){
        getline(orgJSON,str,' ');
      }
      else{
        orgJSON>>inChar;
      }
    }
  }//check if the file opened.
  else{
    std::cout<<"Invalid JSON File:"<<fullpath<<"!\n";
  }
  orgJSON.close();
  if(VRunLumi.size() == 0){
    std::cout<<"No Lumiblock found in JSON file\n";
  }
  std::vector< std::vector<int> > VVRunLumi;
  for(unsigned int i = 0; i+2 < VRunLumi.size();){
    if(VRunLumi[i] > 130000){
      std::vector<int> RunLumi;
      RunLumi.push_back(VRunLumi[i]);
      while(VRunLumi[i+1] < 130000 && i+1 < VRunLumi.size()){
        RunLumi.push_back(VRunLumi[i+1]);
        ++i;
      }
      VVRunLumi.push_back(RunLumi);
      ++i;
    }
  }
  return VVRunLumi;
}

bool inJSON(std::vector< std::vector<int> > VVRunLumi, int Run, int LS){
  bool answer = false;
  if(Run < 120000){
    answer = true;
  }
  else{
    for(unsigned int i = 0; i < VVRunLumi.size();++i){
      if(Run == VVRunLumi[i][0]){
        for(unsigned int j = 1; j+1 < VVRunLumi[i].size();j=j+2){
          if(LS >= VVRunLumi[i][j] && LS <= VVRunLumi[i][j+1]){
            answer = true;
          }
        }
      }
    }
  }
  return answer;
}
