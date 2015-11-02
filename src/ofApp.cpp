#include "ofApp.h"
#include "dispatch/dispatch.h"
#include "ofxAudioUnitHardwareUtils.h"
#import <mach/mach_time.h>

int numLights = 9;

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetDataPathRoot("/Volumes/Qajaq/Player/data/");
    
    ofBackground(255,255,255);
    ofSetWindowPosition(2000, 0);
    ofSetFullscreen(true);
    ofSetBackgroundAuto(false);
    ofEnableSmoothing();
    
    videoPlayerNext = new ofVideoPlayer();
    videoPlayer = new ofVideoPlayer();
    
    videoPlayerNext->setPixelFormat(OF_PIXELS_RGB);
    videoPlayer->setPixelFormat(OF_PIXELS_RGB);
    
    videoPlayerNext->setUseTexture(false);
    videoPlayer->setUseTexture(false);
    
    parameters.setName("settings");
    vSync.addListener(this,&ofApp::vSyncChanged);
    parameters.add(vSync.set("vSync",true));
    
    ofDisableArbTex();
    
    gui.setup(parameters);
    gui.setPosition((ofGetWidth()-gui.getWidth())-20, gui.getPosition().y);
    gui.minimizeAll();
    
    gui.loadFromFile("settings.xml");
    
    showOSD = false;
    message = "";
    
    vector<AudioDeviceID> deviceList = AudioOutputDeviceList();
    output.listOutputDevices();
    
    if (deviceList.size() > 0) {
        output.setDevice(deviceList[deviceList.size()-1]);
    }
    
    mixer.setInputBusCount(2);
    filePlayerLeft.connectTo(mixer, 0);
    filePlayerRight.connectTo(mixer, 1);
    mixer.setPan(0,0);
    mixer.setPan(1,1);
    
    int outputChannels = output.getNumOutputChannels();
    cout << "There are " << outputChannels << " channels" << endl;
    for (int i = 0; i < outputChannels; i++) {
        cout << "connecting " << i%2 << " to "  << i << endl;
        mixer.connectTo(output,i, i%2);
    }
    output.start();
    
    //	As of right now, the output will pull audio down the chain,
    //	but the filePlayer won't produce anything yet since we
    //	haven't given it a file to play.
    
    filePlayerLeft.setFile("/Volumes/Qajaq/Music/LFF_REC_1#10.caf");
    filePlayerRight.setFile("/Volumes/Qajaq/Music/LFF_REC_2#10.caf");
    
    filePlayerLeft.prime();
    filePlayerRight.prime();
    
    shader.load("shaders/shader");
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,0), ^{
        // background operation
        setupVideoReferences("/Volumes/Qajaq/Videos/");
        
        cout << "VIDEO REFERENCES SETUP" << endl;
        
        currentVideoIndex = 0;
        currentVideoReference = &VideoReferences[currentVideoIndex];
        
        setupDone = true;
        
    });
    
    if( finalCutXML.load("/Volumes/Qajaq/Kullorsuaq Timelapse.fcpxml") ){
        message = "XML loaded";
    }
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    if(setupDone){
        if(!firstVideoLoaded) {
            
            videoPlayer->load(currentVideoReference->path);
            videoPlayer->setLoopState(OF_LOOP_NONE);
            videoPlayer->play();
            
            cout << "FIRST VIDEO LOADED "  << currentVideoReference->path << endl;
            
            uint64_t startTime = mach_absolute_time() + (1000000000/10); // 1/10 of a second
            
            filePlayerLeft.play(startTime);
            filePlayerRight.play(startTime);
            
            videoPlayerNextReadyForPreloading = true;
            
            firstVideoLoaded = true;
            
        }
        if(videoPlayer->getIsMovieDone()){
            swapToNextVideo();
        }
        if (videoPlayer->getPosition() > 0.01 && videoPlayerNextReadyForPreloading) {
            uint64_t millisBeforeCallToPreload = ofGetElapsedTimeMillis();
            preloadNextVideo();
            cout << "UPDATE CALLED PRELOAD AND IT TOOK " << ofGetElapsedTimeMillis() - millisBeforeCallToPreload << " MS" << endl;
        }
        
        videoPlayer->update();
        
        message = currentTagsForVideoReference(currentVideoReference, videoPlayer);
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    
    if(setupDone){
        
        float videoHeight = ofGetWidth()*(videoPlayer->getHeight()*1.0/videoPlayer->getWidth());
        
        ofSetColor(255,255,255,255);
        
        if(videoPlayer->isFrameNew()){
            ofBackground(ofColor::black);
            ofPixels & pixels = videoPlayer->getPixels();
            texture.loadData(pixels);
            plane.set(ofGetWidth(), videoHeight, 2, 2);
            plane.setPosition(ofGetWidth()/2, videoHeight/2, 0);
            shader.begin();
            shader.setUniformTexture("tex", texture, 0);
            plane.draw();
            shader.end();
        }
        
        if(showOSD){
            
            ofFill();
            
            ofSetHexColor(0x000000);
            ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()), 10,20);
            ofDrawBitmapStringHighlight(currentVideoReference->path, 10,40);
            if(videoPlayer->isLoaded()){
                int frameDataIndex = max(min(videoPlayer->getCurrentFrame()+1, videoPlayer->getTotalNumFrames()), 0);
                
                ofDrawBitmapStringHighlight(currentVideoReference->dateStringForFrameDataIndex(frameDataIndex), 10,60);
                ofDrawBitmapStringHighlight(ofToString(currentVideoReference->exposureStringForFrameDataIndex(frameDataIndex)), 10,80);
                ofDrawBitmapStringHighlight(ofToString(currentVideoReference->data[frameDataIndex].difference), 10,100);
                
                ofSetColor(255);
                ofDrawRectangle((ofGetWidth()-2)*(videoPlayer->getCurrentFrame()*1.0/videoPlayer->getTotalNumFrames()), ofGetHeight()-20, 2, 20);
                ofSetHexColor(0x000000);

            }
            ofDrawBitmapStringHighlight(message, 10,120);
            
            ofSetColor(255);
            gui.draw();
            
            
        }
        
    } else {
        ofBackground(ofColor::black);
        ofSetColor(255,255,255,255);
        ofDrawBitmapStringHighlight(message, 10,10);
    }
    
}

void ofApp::vSyncChanged(bool & vSync){
    ofSetVerticalSync(vSync);
}

void ofApp::swapToNextVideo() {
    
    // point current to the next
    currentVideoIndex = (currentVideoIndex+1) % VideoReferences.size();
    currentVideoReference = &VideoReferences[currentVideoIndex];
    
    videoPlayerNext->play();
    std::swap(videoPlayerNext, videoPlayer);
    
    cout << "SWAPPED FROM " << videoPlayerNext->getMoviePath() << " TO " << videoPlayer->getMoviePath() << endl;
    
    // message = loadTagsForVideoReference(currentVideoReference);
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,0), ^{

        videoPlayerNext->stop();
        videoPlayerNextReadyForPreloading = true;
        
    });

    
}

void ofApp::preloadNextVideo() {
    
    if(setupDone){
        if(videoPlayerNextReadyForPreloading){
            videoPlayerNextReadyForPreloading = false;
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,0), ^{
                // background operation
                
                // find the next video to load
                int nextVideoIndex = (currentVideoIndex+1) % VideoReferences.size();
                VideoReference * nextVideoReference = &VideoReferences[nextVideoIndex];
                videoPlayerNext->load(nextVideoReference->path);
                uint64_t loadingTimeout = ofGetElapsedTimeMillis();
                while(ofGetElapsedTimeMillis() < loadingTimeout + 1000) {
                    if(videoPlayerNext->isLoaded()){
                        cout << "LOADING " << nextVideoReference->path << " TOOK " << ofGetElapsedTimeMillis() - loadingTimeout << " MS" << endl;
                        break;
                    }
                }
                videoPlayerNext->setLoopState(OF_LOOP_NONE);
                videoPlayerNext->play();
                videoPlayerNext->setPaused(true);
                videoPlayerNext->firstFrame();
                videoPlayerNext->update();
                cout << "PRELOADED " << nextVideoReference->path << endl;
                
            });
        }
    }
}

void ofApp::setupVideoReferences(string videoFolder){
    
    ofBuffer rawFrameListBuffer = ofBufferFromFile(videoFolder + "../filelist.txt");
    
    struct linesForFile {
        string fileBaseName;
        vector<VideoReference::frameData> data;
    };
    
    linesForFile f;
    vector<linesForFile> vf;
    string formerDate = "";
    
    
    if(rawFrameListBuffer.size()) {
        for (ofBuffer::Line it = rawFrameListBuffer.getLines().begin(), end = rawFrameListBuffer.getLines().end(); it != end; ++it) {
            
            string line = *it;
            
            // copy the line to draw later
            // make sure its not a empty line
            if(line.empty() == false) {
                if(ofStringTimesInString(line, "average") > 0){
                    string date = line.substr(2,10);
                    if(ofStringTimesInString(formerDate, date) < 1){
                        message = "Loading metadata for " + date;
                        linesForFile nf;
                        nf.fileBaseName = date;
                        vf.push_back(nf);
                    }
                    vf.back().data.push_back(VideoReference::frameDataFromString(line));
                    formerDate = date;
                }
            }
        }
    }
    videoDir.listDir(videoFolder);
    
    videoDir.sort(); // in linux the file system doesn't return file lists ordered in alphabetical order
    
    if( videoDir.size() ){
        VideoReferences.assign(videoDir.size(), VideoReference());
    }
    
    for(int i = 0; i < (int)videoDir.size(); i++){
        string currentFilePath = videoDir.getPath(i);
        VideoReferences[i] = VideoReference(currentFilePath);
        string fileBaseName = VideoReferences[i].file.getBaseName();
        message = "Creating reference for " + fileBaseName;
        for(int j = 0; j < vf.size(); j++){
            if(ofStringTimesInString(vf[j].fileBaseName, fileBaseName) > 0){
                VideoReferences[i].data = vf[j].data;
                break;
            }
        }
    }
    
}

string ofApp::loadTagsForVideoReference(VideoReference * vRef){
    string returnString = "";
    string XMLpath = "//fcpxml/library/event/clip[@name=" + ofFilePath::getBaseName(vRef->path) + "]";
    message = XMLpath;
    finalCutXML.setTo("//fcpxml/library/event/clip[@name=" + ofFilePath::getBaseName(vRef->path) + "]");
    for (int i=0; i < finalCutXML.getNumChildren(); i++) {
        if (finalCutXML.exists("keyword[" + ofToString(i) + "]")) {
            returnString += finalCutXML.getValue("keyword[" + ofToString(i) + "][@value]") + "\n";
        }
    }
    if(returnString.compare("") == 0){
        returnString = "";
    } else {
        returnString = returnString.substr(0,returnString.size()-1); // get rid of the last linebreak
    }
    return returnString;
}

float ofApp::finalCutTimeToFloat(string t){
    
    if(t.find('/') == string::npos) {
        return ofToFloat(t.substr(0,t.size()-1));
    } else {
        return ofToFloat(t.substr(0,t.find('/'))) / ofToFloat(t.substr(t.find('/')+1, t.size()-(1+t.find('/'))));
        
    }
}

string ofApp::currentTagsForVideoReference(VideoReference * vRef, ofVideoPlayer * p){
    string returnString = "";
    string XMLpath = "//fcpxml/library/event/clip[@name=" + ofFilePath::getBaseName(vRef->path) + "]";
    message = XMLpath;
    finalCutXML.setTo("//fcpxml/library/event/clip[@name=" + ofFilePath::getBaseName(vRef->path) + "]");
    for (int i=0; i < finalCutXML.getNumChildren(); i++) {
        if (finalCutXML.exists("keyword[" + ofToString(i) + "]")) {
            
            float startTimeSecs = finalCutTimeToFloat(finalCutXML.getValue("keyword[" + ofToString(i) + "][@start]"));
            
            float durationSecs = finalCutTimeToFloat(finalCutXML.getValue("keyword[" + ofToString(i) + "][@duration]"));
            
            float endTimeSecs = startTimeSecs + durationSecs;
            
            float currentVideoPositionSecs = p->getPosition() * p->getDuration();
            
            if(currentVideoPositionSecs > startTimeSecs &&  currentVideoPositionSecs < endTimeSecs )
                returnString += finalCutXML.getValue("keyword[" + ofToString(i) + "][@value]") + "\n";
            
        }
    }
    if(returnString.compare("") == 0){
        returnString = "";
    } else {
        returnString = returnString.substr(0,returnString.size()-1); // get rid of the last linebreak
    }
    return returnString;
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
            if(videoPlayerNext->isLoaded())
                swapToNextVideo();
            break;
        case '0':
            videoPlayer->firstFrame();
            break;
        case 'f':
            ofToggleFullscreen();
            break;
        case 's':
            settings.serialize(parameters);
            settings.save("settings.xml");
            break;
        case 'g':
            output.showUI();
            mixer.showUI();
            break;
        case 'l':
            settings.load("settings.xml");
            settings.deserialize(parameters);
            break;
        case OF_KEY_TAB:
            showOSD = !showOSD;
            if (showOSD) {
                ofShowCursor();
            } else {
                ofHideCursor();
            }
            break;
        case ' ':
            videoPlayer->setPaused(!videoPlayer->isPaused());
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
    if(button == OF_MOUSE_BUTTON_1){
        int width = ofGetWidth();
        float pct = (float)x / (float)width;
        videoPlayer->setPosition(pct);
        videoPlayer->update();
    }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    if(button == OF_MOUSE_BUTTON_1){
        videoPlayer->setPaused(true);
    }
}


//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    if(button == OF_MOUSE_BUTTON_1){
        videoPlayer->setPaused(false);
    }
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
