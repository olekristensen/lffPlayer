#pragma once

#include "ofMain.h"
#include "VideoReference.h"
//#include "ofxPhilipsHue.h"
//#include "hueLight.h"
#include "ofxAudioUnit.h"

#define HOST "localhost"
#define PORT 12345

#include "ofxGui.h"

class ofApp : public ofBaseApp{

	public:
    
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);		

        void setupVideoReferences(string videoFolder);
    
    void vSyncChanged(bool & vSync);
    void useShaderChanged(bool & useShader);
//        void hueOffsetChanged(float & hueOffset);
//        void hueSaturationChanged(float & hueSaturation);

        void preloadNextVideo();
    
        void swapToNextVideo();
    
        ofDirectory videoDir;
        vector <VideoReference> VideoReferences;
    
        int currentVideoIndex;
        VideoReference * currentVideoReference;
    
        ofDirectory stillDir;
    
        ofVideoPlayer *     videoPlayerNext;
        ofVideoPlayer *     videoPlayer;
    
        ofXml finalCutXML;
        string loadTagsForVideoReference(VideoReference * vRef);
        string currentTagsForVideoReference(VideoReference * vRef, ofVideoPlayer * p);

        ofBuffer rawFrameListBuffer;

        string message;
    
        bool videoPlayerNextReady;
        Poco::Timestamp lastFragTimestamp, lastVertTimestamp;
    
        float finalCutTimeToFloat(string t);
        
        ofTexture texture;
        ofShader shader;
        ofPlanePrimitive plane;
    
//        ofxPhilipsHue hue;
//        double lastHueUpdateSeconds;
//        vector<hueLight> hueLights;
        bool setupDone = false;
//        int draggingHueLight;
    
//        ofxOscParameterSync sync;
    
    ofParameterGroup parameters;
    ofParameter<int> currentYear;
    ofParameter<int> currentMonth;
    ofParameter<int> currentDay;
    ofParameter<int> currentHour;
    ofParameter<int> currentMinute;
    ofParameter<int> currentSecond;
//    ofParameter<float> hueOffset;
//    ofParameter<float> hueSaturation;
    ofParameter<string> videoFolder;
    ofParameter<bool> vSync;
    ofParameter<bool> useShader;
    ofParameter<bool> showOSD;
    
    ofXml settings;

        ofxPanel gui;

    ofxAudioUnitFilePlayer filePlayerLeft;
    ofxAudioUnitFilePlayer filePlayerRight;
    ofxAudioUnitOutput output;
    ofxAudioUnitMixer mixer;

    
};
