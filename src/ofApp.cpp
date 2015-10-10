#include "ofApp.h"
#include "dispatch/dispatch.h"
#include "ofxAudioUnitHardwareUtils.h"
#import <mach/mach_time.h>

int numLights = 9;

//--------------------------------------------------------------
void ofApp::setup(){
    
    //ofSetDataPathRoot("/Volumes/Qajaq/Player/data/");
    
    ofBackground(255,255,255);
    ofSetWindowPosition(2000, 0);
    ofSetFullscreen(true);
    ofEnableSmoothing();
//    draggingHueLight = -1;
    
    videoPlayerNext = new ofVideoPlayer();
    videoPlayer = new ofVideoPlayer();
    
    parameters.setName("settings");
    vSync.addListener(this,&ofApp::vSyncChanged);
    parameters.add(vSync.set("vSync",true));
    useShader.addListener(this,&ofApp::useShaderChanged);
    parameters.add(useShader.set("Shader",true));
//    hueOffset.addListener(this, &ofApp::hueOffsetChanged);
//    parameters.add(hueOffset.set("Hue Offset",0,-1,1));
//    hueSaturation.addListener(this, &ofApp::hueSaturationChanged);
//    parameters.add(hueSaturation.set("Hue Saturation",1,0,1));
    
    ofDisableArbTex();
    
    videoPlayerNext->setUseTexture(false);
    videoPlayer->setUseTexture(false);

    /*
    for(int i = 0; i < numLights; i++){
        
        hueLight hl(i, ofVec2f(
                               ((i*1.0/numLights)*1920.0)+(1920.0/(numLights*2)),
                               0.25*1100.0) );
        hl.updateSpeed = 1.0;
        parameters.add(hl.parameters);
        hueLights.push_back(hl);
    }
    */
    gui.setup(parameters);
    gui.setPosition((ofGetWidth()-gui.getWidth())-20, gui.getPosition().y);
    gui.minimizeAll();
    
    gui.loadFromFile("settings.xml");
    
//    hue.setup("192.168.1.2", "newdeveloper");

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

    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,0), ^{
        // background operation
        setupVideoReferences("/Volumes/Qajaq/Videos/");
        
        currentVideoIndex = 0;
        currentVideoReference = &VideoReferences[currentVideoIndex];
        
        // Uncomment this to show movies with alpha channels
        // videoPlayer.setPixelFormat(OF_PIXELS_RGBA);
        
        videoPlayer->load(currentVideoReference->path);
        videoPlayer->setLoopState(OF_LOOP_NONE);
        videoPlayer->play();
        
        uint64_t startTime = mach_absolute_time() + (1000000000/10); // 1/10 of a second
        
        filePlayerLeft.play(startTime);
        filePlayerRight.play(startTime);

        videoPlayerNextReady = true;

        setupDone = true;
        
    });
    
    if( finalCutXML.load("/Volumes/Qajaq/Kullorsuaq Timelapse.fcpxml") ){
        message = "XML loaded";
    }

    shader.load("shaders/shader");
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    if(setupDone){
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

    message = currentTagsForVideoReference(currentVideoReference, videoPlayer);
    }
/*    if(ofGetElapsedTimef() - lastHueUpdateSeconds > 0.5){
        for(int i = 0; i < hueLights.size(); i++){
            hueLight * hl = & hueLights[i];
            ofColor c(hl->color.get());
            int address(hl->address);
            hue.setLightState(hl->address, true, ofColor(hl->color.get()), 500);
        }
        lastHueUpdateSeconds = ofGetElapsedTimef();
    }
*/
}

//--------------------------------------------------------------
void ofApp::draw(){

    ofBackground(ofColor::black);
	ofSetHexColor(0xFFFFFF);
    
    if(setupDone){
    
    float videoHeight = ofGetWidth()*(videoPlayer->getHeight()*1.0/videoPlayer->getWidth());
        
/*
    ofFile fragFile("shaders/shader.frag"), vertFile("shaders/shader.vert");
    Poco::Timestamp fragTimestamp = fragFile.getPocoFile().getLastModified();
    Poco::Timestamp vertTimestamp = vertFile.getPocoFile().getLastModified();
    if(fragTimestamp != lastFragTimestamp || vertTimestamp != lastVertTimestamp) {
        cout << "reloading shader" << endl;
        bool validShader = shader.load("shaders/shader");
    }
    lastFragTimestamp = fragTimestamp;
    lastVertTimestamp = vertTimestamp;
*/
    ofSetColor(255,255,255,255);

        if (useShader){
            ofPixels & pixels = videoPlayer->getPixels();
            texture.loadData(pixels);
            plane.set(ofGetWidth(), videoHeight, 2, 2);
            plane.setPosition(ofGetWidth()/2, videoHeight/2, 0);
            shader.begin();
            shader.setUniformTexture("tex", texture, 0);
            plane.draw();
            shader.end();
        } else {
            videoPlayer->draw(0, 0, ofGetWidth(), videoHeight);
        }
/*
    for(int i = 0; i < hueLights.size(); i++){
        hueLight * hl = &hueLights[i];
        int posX = hl->position.get().x;
        int posY = hl->position.get().y;
        int spacing = 3;
        ofFloatColor cCenter = pixels.getColor(posX, posY);
        ofFloatColor cTop = pixels.getColor(posX, posY-spacing);
        ofFloatColor cLeft = pixels.getColor(posX-spacing, posY);
        ofFloatColor cBottom = pixels.getColor(posX, posY+spacing);
        ofFloatColor cRight = pixels.getColor(posX+spacing, posY);
        
        float r = (cCenter.r + cTop.r + cLeft.r + cBottom.r + cRight.r) / 5.0;
        float g = (cCenter.g + cTop.g + cLeft.g + cBottom.g + cRight.g) / 5.0;
        float b = (cCenter.b + cTop.b + cLeft.b + cBottom.b + cRight.b) / 5.0;
        
        hl->color = (hl->color.get() * (1.0-hl->updateSpeed.get())) +(ofFloatColor(r,g,b,1.0) * hl->updateSpeed.get());
    }
  */
    if(showOSD){

/*        ofNoFill();
        for(int i = 0; i < hueLights.size(); i++){
            hueLight * hl = &hueLights[i];
            
            ofSetLineWidth(5);
            ofSetColor(127,127,127,127);
            ofEllipse((hl->position.get().x/1920)*ofGetWidth(), (hl->position.get().y/1100)*videoHeight, 30,30);

            ofSetLineWidth(2);
            ofSetColor(hl->color.get());
            ofEllipse((hl->position.get().x/1920)*ofGetWidth(), (hl->position.get().y/1100)*videoHeight, 30,30);
        }
*/
        ofFill();

        ofSetHexColor(0x000000);
        ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()), 10,20);
        ofDrawBitmapStringHighlight(currentVideoReference->path, 10,40);
        
        int frameDataIndex = min(videoPlayer->getCurrentFrame()+1, videoPlayer->getTotalNumFrames());
        
        ofDrawBitmapStringHighlight(currentVideoReference->dateStringForFrameDataIndex(frameDataIndex), 10,60);
        ofDrawBitmapStringHighlight(ofToString(currentVideoReference->exposureStringForFrameDataIndex(frameDataIndex)), 10,80);
        ofDrawBitmapStringHighlight(ofToString(currentVideoReference->data[frameDataIndex].difference), 10,100);
        ofDrawBitmapStringHighlight(message, 10,120);

        ofSetColor(255);
        gui.draw();
        
    }
        
    } else {
        ofDrawBitmapStringHighlight(message, 10,10);
    }
    
}

void ofApp::vSyncChanged(bool & vSync){
    ofSetVerticalSync(vSync);
}

void ofApp::useShaderChanged(bool & useShader){
    videoPlayer->setUseTexture(useShader);
    videoPlayerNext->setUseTexture(useShader);
}

/*
 void ofApp::hueOffsetChanged(float & hueOffset){
 cout << "hueoffset event: " << hueOffset << endl;
 hue.hueOffset = hueOffset;
 }
 
 void ofApp::hueSaturationChanged(float & hueSaturation){
 cout << "hueSaturation event: " << hueSaturation << endl;
 hue.saturationFactor = hueSaturation;
 }
 */

void ofApp::swapToNextVideo() {
    
    // point current to the next
    currentVideoIndex = (currentVideoIndex+1) % VideoReferences.size();
    currentVideoReference = &VideoReferences[currentVideoIndex];
    
    videoPlayerNext->play();
    
    std::swap(videoPlayerNext, videoPlayer);
    
    // message = loadTagsForVideoReference(currentVideoReference);
    
    videoPlayerNext->stop();
    videoPlayerNextReady = true;
    
}

void ofApp::preloadNextVideo() {
    
    if(setupDone){
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
    }
    if(button == OF_MOUSE_BUTTON_3){
/*        if (draggingHueLight >= 0) {
            float videoHeight = ofGetWidth()*(videoPlayer->getHeight()*1.0/videoPlayer->getWidth());

            hueLight * hl = &hueLights[draggingHueLight];
            hl->position.set(ofVec3f(x*1920.0/ofGetWidth(),y*1100.0/videoHeight));
        }
*/
    }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    if(button == OF_MOUSE_BUTTON_1){
        videoPlayer->setPaused(true);
    }
    if(button == OF_MOUSE_BUTTON_3){
/*        for(int i = 0; i < hueLights.size(); i++){
            hueLight * hl = &hueLights[i];
            
            float videoHeight = ofGetWidth()*(videoPlayer->getHeight()*1.0/videoPlayer->getWidth());
            
            ofPoint p((hl->position.get().x/1920)*ofGetWidth(), (hl->position.get().y/1100)*videoHeight);
            
            if(p.distance(ofPoint(x,y)) < 30){
                draggingHueLight = i;
                break;
            }
        }
*/
    }
}


//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    if(button == OF_MOUSE_BUTTON_1){
        videoPlayer->setPaused(false);
    }
    if(button == OF_MOUSE_BUTTON_3){
//        draggingHueLight = -1;
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
