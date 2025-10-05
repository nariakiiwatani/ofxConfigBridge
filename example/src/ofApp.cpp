#include "ofApp.h"
#include "ofxConfigBridgeAPI.hpp"

//--------------------------------------------------------------
void ofApp::setup(){
	{
		// implicit type indferring
		YAML::Node y;
		ofx::configbridge::loadFile("config.yaml", y);
		nlohmann::ordered_json j;
		ofx::configbridge::convert(y, j);
		ofx::configbridge::saveFile("out.json", j);
	}
	{
		// explicit specification
		auto j = ofx::configbridge::loadFile<nlohmann::ordered_json>("config.json");
		auto y = ofx::configbridge::convert<YAML::Node>(j);
		ofx::configbridge::saveFile("out.yaml", y);
	}
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::exit(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

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
