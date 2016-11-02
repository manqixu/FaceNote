#pragma once

#include "ofMain.h"

#include "ofxCv.h"
#include "ofxFaceTracker.h"

class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();
    void keyPressed(int key);
    
    void updateWaveform(int waveformResolution);
    void audioOut(float * output, int bufferSize, int nChannels);
    
    
    ofVideoGrabber cam;
    ofxFaceTracker tracker;
    
    std::vector<float> waveform; // this is the lookup table
    double phase;
    float frequency;
    
    ofMutex waveformMutex;
    ofPolyline waveLine;
    ofPolyline outLine;
};
