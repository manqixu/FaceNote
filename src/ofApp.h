#pragma once

#include "ofMain.h"

#include "ofxCv.h"
#include "ofxFaceTracker.h"
#include "ofxGenSound.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();
    void keyPressed(int key);
    
    void checkOrientation();
    
    void audioOut(float * input, int bufferSize, int nChannels);
    
    ofSoundStream soundStream;
    vector <float> audioOutLeft;
    vector <float> audioOutRight;
    void drawWaveform(ofVec2f& position);

private:
    // Head orientation tracker1
    ofVideoGrabber cam;
    ofxFaceTracker tracker;
    ofVec3f orientation;
    
    // Drum set
    ofSoundPlayer snare;
    ofSoundPlayer tom;
    ofSoundPlayer kick;
    ofSoundPlayer hihat;
    ofSoundPlayer ride;
    ofSoundPlayer crash;
    
    //ofxGenSound
    ofxGenSoundOsc triWave, sineWave;
    ofxGenSoundEnvelope env;
    ofxGenSoundDelay delay;
    ofxGenSoundFilter filter;
    
    // Wave GUI
    ofxPanel wave;
};
