/*
 ofApp.cpp
 OscarUniverse
 
 Created by Marta Serafini on 16/09/2019
 
 This application represents a Universe where each planet is a movie that won at least an Oscar in 2017.
 It is possible to learn more information about each film by clicking on it with the mouse.
 */

#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
    
    //background
    ofLoadImage(backgroundImage, "sky.jpg");   //upload image to use as background
    
    //font
    //old OF default is 96 but this results in fonts looking larger than in other programs
    ofTrueTypeFont::setGlobalDpi(72);
    font.load("Ubuntu-Regular.ttf", 14);
    
    ofEnableDepthTest();   //enable z buffer
    ofDisableArbTex();     //it uses GL_TEXTURE_2D textures and it supports mipmaps (a core OpenGL feature)
    
    //camera
    cameraOrigin = ofPoint(0, 0, 1000);
    camera.setPosition(cameraOrigin);
    targetBoxPosition = cameraOrigin;
    moveCameraEnabled = false;
    isZoomingInsideBox = false;
    
    //model
    model.loadModel("model/oscar.obj");   //uploads 3D model of the Oscar statuette
    model.setRotation(0, 180, 90, 0, 1);   //rotation on z and y axes
    model.setScale(0.7, 0.7, 0.7);        //shrinks 3D model
    isModelRotated = false;
    
    movieSelected = NULL;
    
    //JSON data
    getData("movies.json");   //retrieves data stored in the JSON file and saves them in the vector 'movies'
    
    //default data to draw movies boxes around Oscar statuette
    angleOffset = 360 / movies.size();   //the movies are uniformly distributed around an immaginary circle
    angle = 0.0;                         //angle where to place the movie box
    distance = 450;                      //distance of the movies from the Oscar statuette
    stepScrollBox = 0.f;                 //this value increments each loop by 'speedScrollBox'
    
    //parameters for raycasting
    intersects = false;
    indexIntersectedPrimitive = -1;
    foundIntersection = false;
    
    //play icon
    ofLoadImage(playIcon, "play-button.png");
    playIcon.setAnchorPercent(0.5, 0.5);
    for(int i = 0; i < movies.size(); i++) {
        movies[i].setPlayIconTexture(&playIcon);   //set texture to use as play icon for each FilmBox
    }
    
    //establish communication pipeline between FilmBox instances and the GUI
    setupGUIs();
    
    //lights
    setupLights();   //setup lights of the Oscar statuette
}


//--------------------------------------------------------------
void ofApp::update(){
    
    //lights positions
    updatePositionLights();
    
    //model rotation
    modelRotation();   //180° model rotation on the y axis when Oscar view is activated
    
    //camera movement
    if(moveCameraEnabled) {   //if true, the camera moves to 'targetBoxPosition' position
        moveCamera(targetBoxPosition);
    }
    
    //box rotation
    if(rotateBoxRight || rotateBoxLeft) {
        rotateMovieBox();
    }
    
    //trailer and soundtrack update of the currently selected movie box
    if(movieSelected != NULL) {
        movieSelected -> update();
    }
    
    //help
    showHelp();
    
    //stop scrolling
    if(scrollBoxEnable) {
        scrollBoxEnable = false;
    }
}


//--------------------------------------------------------------
void ofApp::draw(){

    //background
    ofDisableDepthTest();   //disables depth test to have the image behind all other objects
    backgroundImage.draw(ofPoint(0, 0), ofGetWidth(), ofGetHeight());   //background
    ofEnableDepthTest();
    
    camera.begin();
    
    //Oscar model and movies boxes
    model.drawFaces();         //draws 3D model
    drawBoxesAndSelection();   //draws movies boxes around the Oscar statuette and manages raycasting
    
    camera.end();
    
    //GUI and FPS
    ofDisableLighting();    //if lights are enabled, the GUI is illegible
    ofDisableDepthTest();   //turning it off is useful for combining 3D scene with 2D overlays such as a control panel
    
    //the GUI to draw depends from the current view
    if(!isZoomingInsideBox) {
        universePanel.draw();
    } else {
        boxPanel.draw();
    }
    
    font.drawString("FPS: " + ofToString((int)ofGetFrameRate()), 10, 20);   //write FPS
    font.drawString(helpText, 10, ofGetHeight() - 40);                      //draw help
    
    ofEnableDepthTest();
    ofEnableLighting();
    
}


//--------------------------------------------------------------
void ofApp::exit() {
    movieSelected = NULL;
    delete movieSelected;
}


//--------------------------------------------------------------
void ofApp::getData(string file) {
    data.open(file);
    
    //fill the movies vector with information retrieved from JSON file
    movies.resize(data["movies"].size());
    for(int i = 0; i < movies.size(); i++) {
        movies[i].setId(data["movies"][i]["ID"].asString());   //uploads poster, trailer, and soundtrack for each movie
    }
}


//--------------------------------------------------------------
void ofApp::moveCamera(ofPoint target) {
    float tweenValue = 0.3;
    ofPoint startPos = camera.getPosition();   //initial camera position
    ofPoint targetPos;                         //final camera position
    int zPos;                                  //z axis value, it depends if the camera is zooming in or zooming out
    int yPos = target.y;                       //y axis value, it depends if the Oscar View is activated or deactivated
    ofVec3f lerpPos;                           //holds the tweened position
    
    if(isZoomingInsideBox) {
        zPos = 56;
    } else {
        zPos = cameraOrigin.z;
    }
    
    //update final camera position
    targetPos = ofVec3f(target.x, yPos, zPos);
    
    //calculate the interpolated values
    lerpPos.x = ofLerp(startPos.x, targetPos.x, tweenValue);
    lerpPos.y = ofLerp(startPos.y, targetPos.y, tweenValue);
    lerpPos.z = ofLerp(startPos.z, targetPos.z, tweenValue);
    
    camera.setPosition(lerpPos);   //updates camera position
    
    //when camera reaches the final position, it stops
    if(abs(camera.getPosition().x - targetPos.x) < 0.01 &&
       abs(camera.getPosition().y - targetPos.y) < 0.01 &&
       abs(camera.getPosition().z - targetPos.z) < 0.01) {
        
        moveCameraEnabled = false;
    }
}


//--------------------------------------------------------------
void ofApp::rotateMovieBox() {
    //gradual rotation of the selected movie box
    if(rotateBoxRight) {   //if true, the movie box rotates 90° to show the right side of the box
        movieSelected -> rotateBox('r');
        
        if(movieSelected -> getRotationStep() == 90) {   //when the movie box is rotated 90°, the rotation stops
            movieSelected -> setRotationStep(0);
            rotateBoxRight = false;
        }
    } else if(rotateBoxLeft) {   //if true, the selected movie box rotates 90° to show the left side of the box
        movieSelected -> rotateBox('l');
        
        if(movieSelected -> getRotationStep() == 90) {   //when the movie box is rotated 90°, the rotation stops
            movieSelected -> setRotationStep(0);
            rotateBoxLeft = false;
        }
    }
}


//--------------------------------------------------------------
void ofApp::drawBoxesAndSelection() {
    foundIntersection = false;
    
    
    for(int i = 0; i < movies.size(); i++) {
        movies[i].setWorldPosBox(cos(ofDegToRad(angle)) * distance,
                                 sin(ofDegToRad(angle)) * distance, 0);                  //box position in World Space
        movies[i].setScreenPosBox(camera.worldToScreen(movies[i].getWorldPosBox()).x,
                                  camera.worldToScreen(movies[i].getWorldPosBox()).y);   //box position in Screen Space

        movies[i].display();   //draw movies boxes
        
        angle += angleOffset;
        
        //raycasting: check if the i-th box is now pointed by the mouse
        intersects = mousepicker.getRay().intersectsPrimitive(movies[i].getBox(), baricentricCoordinates, dist, surfaceNormal);
        if(intersects) { //if it's true, the ray hit a box
            foundIntersection = true;   //it was found an intersection
            indexIntersectedPrimitive = i;   //saves the index of the selected box
        }
    }
    
    //bool intersectsPrimitive(const of3dPrimitive& primitive,  glm::vec2 & baricentricCoords, float & distance, glm::vec3 & intNormal)
    
    angle = 0.f + stepScrollBox;
    
    //if the mouse is pointing a movie box, it is highlighted
    if(foundIntersection && movieSelected == NULL) {   //selection is visible only when no movie box has been selected
        ofPushStyle();
        glPointSize(5);   //magnifies vertex size
        ofSetColor(ofColor::gold);
        movies[indexIntersectedPrimitive].getBox().drawVertices();   //draws vertices of selected movie box
        ofPopStyle();
    }
}
//--------------------------------------------------------------

//--------------------------------------------------------------
void ofApp::setupLights() {
    
    //Front face light
    lFace.setAmbientColor(ofColor::white);
    lFace.setDiffuseColor(ofColor::white);
    lFace.setSpecularColor(ofColor::white);
    lFace.setPointLight();
    lFace.enable();
    
    //Front body light
    lBody.setAmbientColor(ofColor::white);
    lBody.setDiffuseColor(ofColor::white);
    lBody.setSpecularColor(ofColor::white);
    lBody.setPointLight();
    lBody.enable();
    
    //Right face light
    lHeadRight.setAmbientColor(ofColor::white);
    lHeadRight.setDiffuseColor(ofColor::white);
    lHeadRight.setSpecularColor(ofColor::white);
    lHeadRight.setPointLight();
    lHeadRight.enable();
    
    //Left face light
    lHeadLeft.setAmbientColor(ofColor::white);
    lHeadLeft.setDiffuseColor(ofColor::white);
    lHeadLeft.setSpecularColor(ofColor::white);
    lHeadLeft.setPointLight();
    lHeadLeft.enable();
    
    //Right body light
    lBodyRight.setAmbientColor(ofColor::white);
    lBodyRight.setDiffuseColor(ofColor::white);
    lBodyRight.setSpecularColor(ofColor::white);
    lBodyRight.setPointLight();
    lBodyRight.enable();
    
    //Left body light
    lBodyLeft.setAmbientColor(ofColor::white);
    lBodyLeft.setDiffuseColor(ofColor::white);
    lBodyLeft.setSpecularColor(ofColor::white);
    lBodyLeft.setPointLight();
    lBodyLeft.enable();
    
    //Right base light
    lBase.setAmbientColor(ofColor::white);
    lBase.setDiffuseColor(ofColor::white);
    lBase.setSpecularColor(ofColor::white);
    lBase.setPointLight();
    lBase.enable();
    
    //Internal box light
    lightBox.setAmbientColor(ofColor::white);
    lightBox.setDiffuseColor(ofColor::white);
    lightBox.setSpecularColor(ofColor::white);
    lightBox.setSpotlight();
    lightBox.setSpotlightCutOff(60.f);
    lightBox.disable();
}


//--------------------------------------------------------------
void ofApp::updatePositionLights() {
    lFace.setPosition(0, 160, 300);
    lBody.setPosition(0, 0, 300);
    lHeadRight.setPosition(100, 250, 100);
    lHeadLeft.setPosition(-100, 250, 100);
    lBodyRight.setPosition(150, 30, 50);
    lBodyLeft.setPosition(-150, 30, 50);
    lBase.setPosition(200, -200, 100);
}


//--------------------------------------------------------------
void ofApp::showHelp(){
    //help text shows only commands currently available based on the current position of the camera
    if(help) {
        if(isZoomingInsideBox) {
            helpText = "Press RIGHT ARROW or LEFT ARROW to rotate the box \nPress 'Q' to exit from the box";
        } else {
            helpText = "";
        }
    } else {
        helpText = "";
    }
}


//--------------------------------------------------------------
void ofApp::modelRotation() {
    if(isModelRotated) {
        model.setRotation(model.getNumRotations()+1, 180, 0, 1, 0);
        isModelRotated = false;
    }
}
//--------------------------------------------------------------


//--------------------------------------------------------------
void ofApp::setupGUIs(){
    
    help.set("Help", false);
    
    //PARAMETER GROUPS
    //boxGroup
    boxGroup.add(FilmBox::filmBoxGroup);   //add the filmBox parameter group to the ofApp parameter group
    FilmBox::setupParametersGroup();
    boxGroup.add(help);
    
    //PANELS
    //boxPanel
    boxPanel.loadFont("Ubuntu-Regular.ttf", 14);
    boxPanel.setup(boxGroup);
    boxPanel.setPosition(ofPoint(boxPanel.getPosition().x, boxPanel.getPosition().y + 20, boxPanel.getPosition().z));
    
    //universePanel
    universePanel.loadFont("Ubuntu-Regular.ttf", 14);
    universePanel.setup(wrapperGroupUni);
    universePanel.setPosition(boxPanel.getPosition());
    
    //SET NAMES
    universeGroup.setName("Universe parameters");
    
    boxPanel.setName("GUI");
    universePanel.setName("GUI");
}


//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    //reset camera position
    if(key == 'q' && movieSelected != NULL &&
        movieSelected -> getRotationStep() == 0) {   //during the rotation of the selected movie box,
                                                     //the camera position is not reset
        targetBoxPosition = cameraOrigin;
        
        isZoomingInsideBox = false;   //camera is not zooming in
        moveCameraEnabled = true;     //camera is moving (it is zooming out)
        
        //enable Oscar lights
        lFace.enable();
        lBody.enable();
        lHeadRight.enable();
        lHeadLeft.enable();
        lBodyRight.enable();
        lBodyLeft.enable();
        lBase.enable();
        
        //disable light of the selected movie box
        lightBox.disable();
        
        movieSelected -> setRotationBox(0);               //resets rotation movie box
        if(movieSelected -> getTrailer().isPlaying()) {   //pause the video when it isn't paused before exit the box
            movieSelected -> settingVideoControls();      //needed to solve a bug
        }
        movieSelected -> settingAudioControls(false);     //stops soundtrack of the selected movie box
        movieSelected -> getTrailer().stop();             //stops trailer of the selected movie box
        
        
        movieSelected = NULL;   //when the camera is outside movies boxes, the pointer points to NULL
    }
    
    //rotation of the selected movie box
    if(movieSelected != NULL && movieSelected -> getRotationStep() == 0) {   //the box rotates only if the previous
                                                                             //rotation is over
        if(key == OF_KEY_RIGHT) {   //rotation of the movie box to show right side of the box
            rotateBoxRight = true;
        }
        
        if(key == OF_KEY_LEFT) {   //rotation of the movie box to show left side of the box
            rotateBoxLeft = true;
        }
    }
}


//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    //the ray starts at camera position and it ends at mouse position
    mousepicker.setFromCamera(glm::vec2(x, y), camera);
}


//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

    //check if the mouse is hover a movie box
    if(foundIntersection && !scrollBoxEnable && !isZoomingInsideBox){
        movieSelected = &movies[indexIntersectedPrimitive];   //points to the movie box currently selected
        targetBoxPosition = movies[indexIntersectedPrimitive].getWorldPosBox();   //update 'targetPoxPosition' to the
                                                                                      //position of the currently selected
                                                                                      //movie box
        moveCameraEnabled = true;    //camera is moving
        isZoomingInsideBox = true;   //camera is zooming inside the selected movie box
            
        //disable Oscar lights
        lFace.disable();
        lBody.disable();
        lHeadRight.disable();
        lHeadLeft.disable();
        lBodyRight.disable();
        lBodyLeft.disable();
        lBase.disable();
            
        //enable light inside selected movie box
        lightBox.setPosition(movies[indexIntersectedPrimitive].getWorldPosBox());
        lightBox.lookAt(ofPoint(movies[indexIntersectedPrimitive].getWorldPosBox().x,
                                movies[indexIntersectedPrimitive].getWorldPosBox().y,
                                movies[indexIntersectedPrimitive].getWorldPosBox().z - 1)); //illuminates frontal face
        lightBox.enable();
        
        //enable audio
        movies[indexIntersectedPrimitive].settingAudioControls(true);   //play soundtrack of the selected movie box
    }
    
    //check if the camera is currently showing the cube face with the trailer
    if(movieSelected != NULL) {
        
        //screen coordinates of top-left corner and bottom-right corner of the trailer
        ofPoint topLeftScreenVideo = camera.worldToScreen(movieSelected -> getTrailerCoords()[0]);
        ofPoint bottomRightScreenVideo = camera.worldToScreen(movieSelected -> getTrailerCoords()[1]);
        
        //check if the camera is showing the correct cube face
        if(movieSelected -> getRotationBox() % 180 == 0 && movieSelected -> getRotationBox() != 0) {
            //check if the mouse has clicked on the video
            if(mouseX >= topLeftScreenVideo.x && mouseX <= bottomRightScreenVideo.x &&
               mouseY >= topLeftScreenVideo.y && mouseY <= bottomRightScreenVideo.y) {

                movieSelected -> settingVideoControls();   //play or pause the video trailer
            }
        }
    }
}
