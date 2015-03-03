//
//  hueLight.h
//  videoPlayerExample
//
//  Created by Ole Kristensen on 25/02/15.
//
//

#ifndef __videoPlayerExample__hueLight__
#define __videoPlayerExample__hueLight__

#include <stdio.h>
#include "ofParameterGroup.h"
//#include "ofxOscParameterSync.h"
#include "ofParameter.h"
#include "ofMain.h"

class hueLight {
public:
    
    hueLight(int adr = 0, ofVec2f pos = ofVec2f(10,10));
 
    ofParameterGroup parameters;
    // ofParameter<float> size;
    ofParameter<float> updateSpeed;
    ofParameter<int> address;
    ofParameter<ofVec2f> position;
    ofParameter<ofFloatColor> color;
    
};

#endif /* defined(__videoPlayerExample__hueLight__) */
