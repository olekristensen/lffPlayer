#include "ofApp.h"
#include "dispatch/dispatch.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofBackground(255,255,255);
	ofSetVerticalSync(false);
    ofSetWindowPosition(0, 0);
    ofSetFullscreen(true);
	showOSD = true;
    message = "";
    
    setupVideoReferences("/Volumes/Qajaq/Videos/");
    
    if( finalCutXML.load("/Volumes/Qajaq/Kullorsuaq Timelapse.fcpxml") ){
        message = "XML loaded";
    }

    currentVideoIndex = 30;
    currentVideoReference = &VideoReferences[currentVideoIndex];

    // Uncomment this to show movies with alpha channels
	// videoPlayer.setPixelFormat(OF_PIXELS_RGBA);
    
    videoPlayerNext = new ofVideoPlayer();
    videoPlayer = new ofVideoPlayer();

    videoPlayer->load(currentVideoReference->path);
    videoPlayer->setLoopState(OF_LOOP_NONE);
    videoPlayer->play();
    
    videoPlayerNextReady = true;

}

void ofApp::swapToNextVideo() {
    
    // point current to the next
    currentVideoIndex = (currentVideoIndex+1) % VideoReferences.size();
    currentVideoReference = &VideoReferences[currentVideoIndex];

    videoPlayerNext->play();
    
    std::swap(videoPlayerNext, videoPlayer);

    message = loadTagsForVideoReference(currentVideoReference);

    videoPlayerNext->stop();
    videoPlayerNextReady = true;
    
}

void ofApp::preloadNextVideo() {

    if(videoPlayerNextReady){
        videoPlayerNextReady = false;
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,0), ^{
            // background operation
        
            // find the next video to load
            int nextVideoIndex = (currentVideoIndex+1) % VideoReferences.size();
            VideoReference * nextVideoReference = &VideoReferences[nextVideoIndex];
            videoPlayerNext->load(nextVideoReference->path);
            videoPlayerNext->setLoopState(OF_LOOP_NONE);
            
        });
    }
}

void ofApp::setupVideoReferences(string videoFolder){
    videoDir.listDir(videoFolder);
    videoDir.sort(); // in linux the file system doesn't return file lists ordered in alphabetical order
    
    //allocate the vector to have as many ofImages as files
    if( videoDir.size() ){
        VideoReferences.assign(videoDir.size(), VideoReference());
    }
    
    // you can now iterate through the files and load them into the ofImage vector
    for(int i = 0; i < (int)videoDir.size(); i++){
        VideoReferences[i] = VideoReference(videoDir.getPath(i));
    }
    
}

string ofApp::loadTagsForVideoReference(VideoReference * vRef){
    string returnString = "";
    string XMLpath = "//fcpxml/library/event/clip[@name=" + vRef->file.getBaseName() + "]";
    message = XMLpath;
    finalCutXML.setTo("//fcpxml/library/event/clip[@name=" + vRef->file.getBaseName() + "]");
    for (int i=0; i < finalCutXML.getNumChildren(); i++) {
        if (finalCutXML.exists("keyword[" + ofToString(i) + "]")) {
            returnString += finalCutXML.getValue("keyword[" + ofToString(i) + "][@value]") + "\n";
        }
    }
    if(returnString.compare("") == 0){
        returnString = "no tags";
    }
    return returnString;
}

//--------------------------------------------------------------
void ofApp::update(){
    if(videoPlayer->getIsMovieDone()){
        swapToNextVideo();
    }
    if (videoPlayer->getPosition() > 0.0 && videoPlayerNextReady) {
        cout << "preloading" << endl;
        preloadNextVideo();
        videoPlayerNext->play();
        videoPlayerNext->setPaused(true);
        videoPlayerNext->firstFrame();
    }
    
    videoPlayer->update();

}

//--------------------------------------------------------------
void ofApp::draw(){

	ofSetHexColor(0xFFFFFF);

    float videoHeight = ofGetWidth()*(videoPlayer->getHeight()*1.0/videoPlayer->getWidth());
    
    videoPlayer->draw(0,0, ofGetWidth(), videoHeight);
    
    if(showOSD){
    ofSetHexColor(0x000000);
        ofDrawBitmapStringHighlight(currentVideoReference->path, 10,20);
        ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()), 10,40);
        ofDrawBitmapStringHighlight(message, 10,60);
    }
    
    ofPixels & pixels = videoPlayer->getPixels();

    int vidWidth = pixels.getWidth();
    int vidHeight = pixels.getHeight();
    int nChannels = pixels.getNumChannels();
    
    /*
    // let's move through the "RGB(A)" char array
    // using the red pixel to control the size of a circle.
    for (int i = 4; i < vidWidth; i+=8){
        for (int j = 4; j < vidHeight; j+=8){
            unsigned char r = pixels[(j * 320 + i)*nChannels];
            float val = 1 - ((float)r / 255.0f);
			ofDrawCircle(400 + i,20+j,10*val);
        }
    }

     */
    
    
    
}

//--------------------------------------------------------------
void ofApp::keyPressed  (int key){
    switch(key){
        case OF_KEY_LEFT:
            videoPlayer->previousFrame();
        break;
        case OF_KEY_RIGHT:
            videoPlayer->nextFrame();
            break;
        case OF_KEY_RETURN:
            swapToNextVideo();
            break;
        case '0':
            videoPlayer->firstFrame();
            break;
        case ' ':
            showOSD = !showOSD;
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
        int width = ofGetWidth();
        float pct = (float)x / (float)width;
        videoPlayer->setPosition(pct);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
        videoPlayer->setPaused(true);
}


//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
        videoPlayer->setPaused(false);
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
