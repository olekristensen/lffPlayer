//
//  VideoReference.cpp
//  videoPlayerExample
//
//  Created by Ole Kristensen on 23/02/15.
//
//

#include "ofMain.h"
#include "VideoReference.h"

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
    date.tm_year = ofToInt(file.getFileName().substr(0,4))- 1900;
    date.tm_mon = ofToInt(file.getFileName().substr(5,2))-1;
    date.tm_mday = ofToInt(file.getFileName().substr(8,2));
    
    mktime ( &date );
    
    char buffer [255];
    
    strftime(buffer, 80, "%A %F",&date);
    
    printf ("Made a reference to video file from %s\n", buffer);
    
}
