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


class VideoReference {
    
public:
    
    std::string path;
    tm date;
    ofFile file;
    
    
    
    VideoReference( std::string p );
    VideoReference();
    
};


#endif /* defined(__videoPlayerExample__VideoReference__) */
