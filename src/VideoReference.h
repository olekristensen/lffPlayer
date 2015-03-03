//
//  VideoReference.h
//  videoPlayerExample
//
//  Created by Ole Kristensen on 23/02/15.
//
//

#ifndef __videoPlayerExample__VideoReference__
#define __videoPlayerExample__VideoReference__

#include <stdio.h>
#include <string>
#include <time.h>

/*
 2012-08-07_23-56-53-0992_n29603_e489_d0.00356242_average.bayer16.gz
 */

class VideoReference {
    
public:
    
    struct frameData {
        tm time;
        long exposure; // in microseconds
        float difference; // normalised
    };
    
    std::string path;
    tm date;
    ofFile file;
    vector <frameData> data;
    
    VideoReference( std::string p );
    VideoReference();
    
    static frameData frameDataFromString(string s);
    
    string dateStringForFrameDataIndex(int index);
    string exposureStringForFrameDataIndex(int index);
    
};


#endif /* defined(__videoPlayerExample__VideoReference__) */
