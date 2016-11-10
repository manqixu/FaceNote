#include "ofApp.h"

using namespace ofxCv;

const int N = 256;		//Number of bands in spectrum
float spectrum[ N ];	//Smoothed spectrum values
float Rad = 500;		//Cloud raduis parameter
float Vel = 0.5;		//Cloud points velocity parameter
int bandRad = 2;		//Band index in spectrum, affecting Rad value
int bandVel = 100;		//Band index in spectrum, affecting Vel value

const int n = 50;		//Number of squares

// Offsets for Perlin noise calculation for points
float tx[n], ty[n];
ofPoint p[n];			//Cloud's points positions

float time0 = 0;		//Time value, used for dt computing

void ofApp::setup() {
    ofBackground(0);
    ofSetWindowTitle("FaceNote");

    //---- set up camera ----//
    cam.initGrabber(1280, 720);
    tracker.setup();
    
    //---- load drum set audio ----//
    snare.load("snare-acoustic02.wav");
    snare.setVolume(0.8);
    tom.load("tom-acoustic02.wav");
    tom.setVolume(0.8);
    kick.load("kick-acoustic02.wav");
    kick.setVolume(0.8);
    hihat.load("hihat-acoustic02.wav");
    hihat.setVolume(0.8);
    ride.load("ride-acoustic02.wav");
    ride.setVolume(0.8);
    crash.load("crash-acoustic02.wav");
    crash.setVolume(0.8);
    
    
    //---- ofxGenSound ---//
    
    // 22050 samples per second, 512 samples per buffer, 4 num buffers (latency)
    soundStream.setup(this, 2, 0, 44100, 512, 4);
    audioOutLeft.resize(512);
    audioOutRight.resize(512);
    
    // set up oscillator, initial frequency and envelope parameters
    triWave.setup(soundStream.getSampleRate(), soundStream.getBufferSize());
    triWave.setFrequency(440);
    sineWave.setup(soundStream.getSampleRate(), soundStream.getBufferSize());
    sineWave.setFrequency(220);
    env.set(0.9, 0.07);
    
    
    //---- set up square cloud ----//
    
    // set spectrum values to 0
    for (int i=0; i<N; i++) {
        spectrum[i] = 0.0f;
    }
    
    // initialize points offsets by random numbers
    for ( int j=0; j<n; j++ ) {
        tx[j] = ofRandom( 0, 1000 );
        ty[j] = ofRandom( 0, 1000 );
    }
}

void ofApp::update() {
    //---- update camera and play drum set----//
    cam.update();
    if(cam.isFrameNew()) {
        tracker.update(toCv(cam));
        checkOrientation();
    }
    
    //---- update generative audio output ----//
    
    // update frequency and trigger an envelope
    triWave.setFrequency(ofMap(tracker.getGesture(ofxFaceTracker::MOUTH_HEIGHT), 0, 10, 220, 600, true));
    sineWave.setFrequency(ofMap(tracker.getGesture(ofxFaceTracker::MOUTH_WIDTH), 5, 20, 220, 600, true));
    env.trigger(ofGetKeyPressed());
    
    // control filter and delay
    filter.setCutoff(0.5); // 0-0.5
    delay.setFeedback(0.8); // 0-0.9
    delay.setMix(0.5);
    
    
    //---- update square cloud ----//

    // get current spectrum with N bands
    float *val = ofSoundGetSpectrum( N );
    
    // update our smoothed spectrum
    for ( int i=0; i<N; i++ ) {
        spectrum[i] *= 0.97;	//Slow decreasing
        spectrum[i] = max( spectrum[i], val[i] );
    }
    
    // computing dt as a time between the last and current calling of update()
    float time = ofGetElapsedTimef();
    float dt = time - time0;
    dt = ofClamp( dt, 0.0, 0.1 );
    time0 = time; //Store the current time
    
    // update Rad and Vel from spectrum
    Rad = ofMap( spectrum[ bandRad ], 1, 3, 800, 2400, true );
    Vel = ofMap( spectrum[ bandVel ], 0, 0.1, 0.05, 0.5 );
    
    // update square positions
    for (int j=0; j<n; j++) {
        tx[j] += Vel * dt;	//move offset
        ty[j] += Vel * dt;	//move offset
        p[j].x = ofSignedNoise( tx[j] ) * Rad;
        p[j].y = ofSignedNoise( ty[j] ) * Rad;	
    }
}

void ofApp::draw() {
    //---- draw face ----//
    ofSetColor(255,255,255);
    ofSetLineWidth(1);
    tracker.draw();
    
    //---- draw waves ----//
    ofVec2f vec = ofVec2f(0, ofGetHeight() * .5);
    drawWaveform(vec);
    wave.draw();
    ofDrawBitmapString("Shake Your Head, Open Your Mouth and Press Any Key!", ofGetWidth() * .10, ofGetHeight() * .25);
    
    //---- draw square cloud ----//
    
    // move center of coordinate system to the screen center
    ofPushMatrix();
        ofTranslate( ofGetWidth() / 2, ofGetHeight() / 2 );
    
        // draw squares
        ofSetColor(ofRandom(20,40), ofRandom(80,100), ofRandom(120,140) ,50);
        ofFill();
        for (int i=0; i<n; i++) {
            ofRect(p[i].x, p[i].y,100,100);
        }
    
    // restore coordinate system
    ofPopMatrix();
}

// reset FaceTracker
void ofApp::keyPressed(int key) {
    if(key == 'r') {
        tracker.reset();
    }
}

// play drum set --------------------------------------------------------------
void ofApp::checkOrientation(){
    ofVec3f last_orientation = orientation;
    orientation = tracker.getOrientation();
    
    if (orientation.z > 0.2 && last_orientation.z < 0.2) {
        snare.play();
    }
    
    if (orientation.z < -0.2 && last_orientation.z > -0.2) {
        tom.play();
    }
    
    if (orientation.y > 0.15 && last_orientation.y < 0.15) {
        kick.play();
    }
    
    if (orientation.y < -0.15 && last_orientation.y > -0.15) {
        hihat.play();
    }
    
    if (orientation.x > 0.3 && last_orientation.x < 0.3) {
        ride.play();
    }
    
    if (orientation.x < -0.15 && last_orientation.x > -0.15) {
        crash.play();
    }
}

// play generative sound -----------------------------------------------------
void ofApp::audioOut(float * output, int bufferSize, int nChannels){
    
    // This is your left and right speakers //
    
    float leftScale = -1.0;
    float rightScale = 1.0;
    
    // This runs through the audio buffer at the rate of the audioOut core audio event //
    
    for (int i = 0; i < bufferSize; i++){
        
        // Signal Chain //
        float waveOut = (triWave.setOscillatorType(OF_TRIANGLE_WAVE) + sineWave.setOscillatorType(OF_SINE_WAVE)) * env.addEnvelope();
        
        float filterOut = filter.addFilter(OF_FILTER_LP, waveOut);
        
        float delayOut = delay.addDelay(filterOut);
        
        
        // Output Sound //
        audioOutRight[i] = output[ i * nChannels    ] = delayOut * rightScale;
        audioOutLeft[i] = output[ i * nChannels + 1 ] = delayOut * leftScale;
        
    }
    
}

// play wave --------------------------------------------------------------
void ofApp::drawWaveform(ofVec2f& position) {
    
    ofPushStyle();
    ofPushMatrix();
    ofTranslate(position);
    ofSetColor( ofRandom(20,30), ofRandom(80,100), ofRandom(120,130));
    ofNoFill();
    ofBeginShape();
    
    for (int i = 0; i < audioOutRight.size(); i++) {
        ofVertex(i * 5, audioOutRight[i] * 100 );
    }
    ofEndShape();
    
    ofPopMatrix();
    ofPopStyle();
    
}
