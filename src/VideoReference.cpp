//
//  VideoReference.cpp
//  videoPlayerExample
//
//  Created by Ole Kristensen on 23/02/15.
//
//

#include "ofMain.h"
#include "VideoReference.h"
#include <time.h>

VideoReference::VideoReference()
{
    path = "";
}

VideoReference::VideoReference( std::string p )
{
    path = p;
    file = ofFile(path);
    
    date.tm_hour = 0;
    date.tm_min = 0;
    date.tm_sec = 0;
    date.tm_year = ofToInt(ofFilePath::getFileName(p).substr(0,4))- 1900;
    date.tm_mon = ofToInt(ofFilePath::getFileName(p).substr(5,2))-1;
    date.tm_mday = ofToInt(ofFilePath::getFileName(p).substr(8,2));
    
    mktime ( &date );
}


 VideoReference::frameData VideoReference::frameDataFromString(string s){
     
     if(s.find('/') != string::npos){
         int startPos = s.rfind('/')+1;
         int len = s.size()-startPos;
         s = s.substr(startPos,len);
     }
     
     frameData d;
    
    vector <string> sParts = ofSplitString(s, "_");
    
    for (int i = 0; i < sParts.size(); i++) {
        string sPart = sParts[i];
        if(sPart.substr(0,1).find('e') != string::npos){
            // exposure
            d.exposure = ofToInt(sPart.substr(1,sPart.size()-1));
        }
        if(sPart.substr(0,1).find('d') != string::npos){
            // difference
            d.difference = ofToFloat(sPart.substr(1,sPart.size()-1));
        }
    }
    
    d.time.tm_hour = ofToInt(s.substr(11,2));
    d.time.tm_min = ofToInt(s.substr(14,2));
    d.time.tm_sec = ofToInt(s.substr(17,2));
    d.time.tm_year = ofToInt(s.substr(0,4))- 1900;
    d.time.tm_mon = ofToInt(s.substr(5,2))-1;
    d.time.tm_mday = ofToInt(s.substr(8,2));
    
    mktime ( &d.time );
    
    return d;
};

string VideoReference::dateStringForFrameDataIndex(int index){
    
    char buffer [255];
    
    strftime(buffer, 80, "%A %F %T", &data[index].time);
    
    return string(buffer);
    
}

string VideoReference::exposureStringForFrameDataIndex(int index){
    
    string s = "";
    
    double exposureSeconds = long(data[index].exposure)/(1000.0*1000.0);
    
    if(exposureSeconds < 1.0 ){
        s = "1/" + ofToString(1.0/exposureSeconds);
    } else {
        s = ofToString(exposureSeconds);
    }
        
    return s;
    
}