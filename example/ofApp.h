#pragma once

#include "ofMain.h"
#include "ofxTWELite.h"

class ofApp : public ofBaseApp{
public:
    void setup();
    void update();
    void draw();
    void keyPressed(int key);
    void receiveState(ofxTWELiteState & e);
    
    string shown;
    ofxTWELite twelite;
};
