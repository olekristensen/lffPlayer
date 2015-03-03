//
//  hueLight.cpp
//  videoPlayerExample
//
//  Created by Ole Kristensen on 25/02/15.
//
//

#include "hueLight.h"

hueLight::hueLight(int adr, ofVec2f pos){
    parameters.setName("Light " + ofToString(adr));
    // parameters.add(size.set("size",30,0,100));
    parameters.add(address.set("number",adr,1,20));
    parameters.add(position.set("position",pos,ofVec2f(0,0),ofVec2f(1920,1100)));
    
    color.set("color",ofFloatColor(0),ofFloatColor(0,0),ofFloatColor(1.0));
    
    parameters.add(color);
}