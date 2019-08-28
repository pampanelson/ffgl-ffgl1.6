//
//  Header.h
//  AddSubtract
//
//  Created by Pampa Nie on 2019/8/28.
//

#ifndef MyUtils_h
#define MyUtils_h

#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <sstream>

// split string combine with "," or other mark seperator as vector 
std::vector<std::string> MySplitS1(std::string input){
    std::vector<std::string> result;
    if(input.size()>0){
        
        std::istringstream iss(input);
        for(std::string s; iss >> s; )
            result.push_back(s);
    }
    
    return result;
}



std::vector<float> MyConvertStingToFloatVector(std::string input){

    std::vector<float> vect;
    
    std::stringstream ss(input);
    
    float i;
    
    while (ss >> i)
    {
        vect.push_back(i);
        
        if (ss.peek() == ',')
            ss.ignore();
    }
    
    
    
    return vect;
}

#endif /* MyUtils_h */
