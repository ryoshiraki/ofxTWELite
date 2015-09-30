#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(0);
    
    if (!twelite.connect("/dev/tty.usbserial-AHXH5L20", 115200)) {
        cerr << "cannot connect" << endl;
        exit();
    }
    
    ofAddListener(twelite.stateReceived, this, &ofApp::receiveState);
}

void ofApp::receiveState(ofxTWELiteState &e) {
    stringstream ss;
    ss << e << endl;
    shown = ss.str();
}

//--------------------------------------------------------------
void ofApp::update(){
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofDrawBitmapString(shown, 20, 20);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == '1') {
        twelite.analogWrite(0, ofRandom(TWE_ANALOG_WRITE_MAX));
    } else if (key == '2') {
        twelite.digitalWrite(0, HIGH);
    }
    else if (key == '3') {
        twelite.digitalWrite(0, LOW);
    }
}