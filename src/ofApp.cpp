#include "ofApp.h"

using namespace ofxCv;

void ofApp::setup() {
    cam.initGrabber(1280, 720);
    tracker.setup();
    
    frequency = 60;
    updateWaveform(32);
    ofSoundStreamSetup(1, 0); // mono output
}

void ofApp::update() {
    cam.update();
    if(cam.isFrameNew()) {
        tracker.update(toCv(cam));
        ofScopedLock waveformLock(waveformMutex);
        //    updateWaveform(ofMap(ofGetMouseX(), 0, ofGetWidth(), 3, 64, true));
        frequency = ofMap(tracker.getGesture(ofxFaceTracker::MOUTH_HEIGHT), 0, 10, 60, 700, true);
    }
}

void ofApp::draw() {
    ofSetColor(255);
//    cam.draw(0, 0);
    ofSetLineWidth(2);
    tracker.draw();
    ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, 20);
}

void ofApp::keyPressed(int key) {
    if(key == 'r') {
        tracker.reset();
    }
}

void ofApp::updateWaveform(int waveformResolution) {
    waveform.resize(waveformResolution);
    waveLine.clear();
    
    // "waveformStep" maps a full oscillation of sin() to the size
    // of the waveform lookup table
    float waveformStep = (M_PI * 2.) / (float) waveform.size();
    
    for(int i = 0; i < waveform.size(); i++) {
        waveform[i] = sin(i * waveformStep);
        
        waveLine.addVertex(ofMap(i, 0, waveform.size() - 1, 0, ofGetWidth()),
                           ofMap(waveform[i], -1, 1, 0, ofGetHeight()));
    }
}

void ofApp::audioOut(float * output, int bufferSize, int nChannels) {
    ofScopedLock waveformLock(waveformMutex);
    
    float sampleRate = 44100;
    float phaseStep = frequency / sampleRate;
    
    outLine.clear();
    
    for(int i = 0; i < bufferSize * nChannels; i += nChannels) {
        phase += phaseStep;
        int waveformIndex = (int)(phase * waveform.size()) % waveform.size();
        output[i] = waveform[waveformIndex];
        
        outLine.addVertex(ofMap(i, 0, bufferSize - 1, 0, ofGetWidth()),
                          ofMap(output[i], -1, 1, 0, ofGetHeight()));
    }
}
