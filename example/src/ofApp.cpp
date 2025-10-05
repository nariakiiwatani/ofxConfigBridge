#include "ofApp.h"
#include "ofxConfigBridgeAPI.hpp"

//--------------------------------------------------------------
void ofApp::setup(){
	{
		auto y = ofx::configbridge::loadFile<YAML::Node>("config.yaml");
		auto j = ofx::configbridge::convert<nlohmann::ordered_json>(y);
		ofx::configbridge::saveFile("yaml_to_json.json", j);
		auto t = ofx::configbridge::convert<toml::ordered_value>(y);
		ofx::configbridge::saveFile("yaml_to_toml.toml", t);
	}
	{
		auto j = ofx::configbridge::loadFile<nlohmann::ordered_json>("config.json");
		auto y = ofx::configbridge::convert<YAML::Node>(j);
		ofx::configbridge::saveFile("json_to_yaml.yaml", y);
		auto t = ofx::configbridge::convert<toml::ordered_value>(j);
		ofx::configbridge::saveFile("json_to_toml.toml", t);
	}
	{
		auto t = ofx::configbridge::loadFile<toml::ordered_value>("config.toml");
		auto j = ofx::configbridge::convert<nlohmann::ordered_json>(t);
		ofx::configbridge::saveFile("toml_to_json.json", j);
		auto y = ofx::configbridge::convert<YAML::Node>(t);
		ofx::configbridge::saveFile("toml_to_yaml.yaml", y);
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
