/*
 FilmBox.cpp
 OscarUniverse
 
 Created by Marta Serafini on 16/09/2019
 
 FilmBox class: each FilmBox object is a film represented as a box which contains information (name, director, genres, plot, Oscars won and Oscar nominations) and multimedia files (poster, trailer and soundtrack) about that film
 */

#include "FilmBox.h"

//typedef
typedef ofPoint dimensions;   //this typedef is used to save width and height values in a single variable

//static variables inside a class should be initialized explicitly outside the class
ofParameter<float> FilmBox::volumeSoundtrack;
ofParameter<float> FilmBox::volumeTrailer;
ofParameter<int> FilmBox::rotationSpeed;
ofParameterGroup FilmBox::filmBoxGroup;


//--------------------------------------------------------------
FilmBox::FilmBox() {
    dimensionBox = ofPoint(100.f, 100.f * 1.5, 100.f);   //the height of the movie box is proportional to its width
    dimensionInnerBox = ofPoint(dimensionBox.x - 0.2,
                                dimensionBox.y - 0.2,
                                dimensionBox.z - 0.2); //inner movie box is slightly smaller than outer movie box
    texturePosition = ofPoint(- dimensionInnerBox.x/2 * 0.8,
                              - dimensionInnerBox.y/2 * 0.8,
                              0.3 - dimensionInnerBox.z/2);   //textures are slightly in front of inner box surface
    textureDimension = ofPoint(dimensionInnerBox.x * 0.8, dimensionInnerBox.y * 0.8);   //80% inner box face dimension
    dimensionTrailer = ofPoint(dimensionInnerBox.x, dimensionInnerBox.x * 0.56);   //height is proportional to width
    
    trailerCoords.resize(2);

    playIconTexture = NULL;
    
    rotationBox = 0;
    rotationStep = 0;
    
    isBoxHorizontal = false;
}


//--------------------------------------------------------------
FilmBox::~FilmBox(){
    playIconTexture = NULL;
    delete playIconTexture;
}


//METHODS
//--------------------------------------------------------------
void FilmBox::display() {
    innerBox.setParent(outerBox);   //external box is parent of the inner box
    
    //EXTERNAL BOX
    outerBox.set(dimensionBox.x, dimensionBox.y, dimensionBox.z);
    
    ofPushMatrix();
    ofTranslate(worldPosBox);
    outerBox.setPosition(0,0,0);
    ofRotateYDeg(rotationBox);
    
    //the poster covers the surface of the external box
    poster.bind();
    outerBox.draw();
    poster.unbind();
    
    
    //INNER BOX
    innerBox.set(dimensionInnerBox.x, dimensionInnerBox.y, dimensionInnerBox.z);
    innerBox.setPosition(0, 0, 0);
    
    //'movieBackground' image covers the surface of the inner box
    movieBackground.bind();
    innerBox.draw();
    movieBackground.unbind();
    
    
    //HORIZONTAL INTERNAL BOX ROTATION TEXTURES IN OSCAR VIEW
   
    
    //FACE BOX WITH POSTER (frontal face)
    poster.draw(texturePosition, textureDimension.x, textureDimension.y);
    
    
    //FACE BOX WITH INFORMATION ABOUT MOVIE (right face)
    ofPushMatrix();
    ofRotateYDeg(-90);
    movieInfoTexture.draw(texturePosition, textureDimension.x, textureDimension.y);
    ofPopMatrix();
    
    
    //FACE BOX WITH INFORMATION ABOUT AWARDS
    ofPushMatrix();
    ofRotateYDeg(90);
    movieAwardsTexture.draw(texturePosition, textureDimension.x, textureDimension.y);
    ofPopMatrix();
    
    
    //FACE BOX WITH TRAILER
    glPushMatrix();
    glRotatef(180, 0, 1, 0);                 //needed because the video is flipped
    glTranslatef(0, 0, texturePosition.z);   //translation on the z axis
    
    trailer.setAnchorPoint(dimensionTrailer.x/2, dimensionTrailer.y/2);
    trailer.draw(0, 0, dimensionTrailer.x, dimensionTrailer.y);
    
    //if the video is paused, the play icon is shown
    if(trailer.isPaused()) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);   //in this mode, when the camera moves inside the box, the play icon is not seen
        playIconTexture -> draw(ofPoint(0, 0, 0.2),
                                dimensionTrailer.y/2,
                                dimensionTrailer.y/2);   //play icon sizes the half of trailer height
        glDisable(GL_CULL_FACE);
    }
    glPopMatrix();
    
    //stop video trailer
    trailer.setLoopState(OF_LOOP_NONE);   //the video stops when it ends
    
    ofPopMatrix();
    outerBox.setPosition(worldPosBox);   //resets correct external box position
    
    trailerCoords[0] = ofPoint(worldPosBox.x - dimensionTrailer.x/2,
                               worldPosBox.y + dimensionTrailer.y/2,
                               worldPosBox.z + 0.3 - dimensionInnerBox.z/2);   //top left corner
    trailerCoords[1] = ofPoint(worldPosBox.x + dimensionTrailer.x/2,
                               worldPosBox.y - dimensionTrailer.y/2,
                               worldPosBox.z + 0.3 - dimensionInnerBox.z/2);   //bottom right corner
}


//--------------------------------------------------------------
void FilmBox::rotateBox(char orientation) {
    if(rotationStep < 90) {   //check if the 90° rotation is over
        
        if(rotationStep + rotationSpeed <= 90) {
            
            if(orientation == 'r') {         //gradual rotation to the right
                rotationBox += rotationSpeed % 360;
            } else if(orientation == 'l'){   //gradual rotation to the left
                rotationBox -= rotationSpeed % 360;
            }
            
            rotationStep += rotationSpeed;
        } else {   //if the sum is greater than 90°, the missing value is added to reach 90°
            
            if(orientation == 'r') {
                rotationBox += (rotationStep + rotationSpeed - 90) % 360 ;
            } else if(orientation == 'l'){
                rotationBox -= (rotationStep + rotationSpeed - 90) % 360;
            }
            
            rotationStep += rotationStep + rotationSpeed - 90;
        }
    }
}


//--------------------------------------------------------------
void FilmBox::update() {
    //trailer
    trailer.update();                   //updates the movie trailer internal state to continue playback
    trailer.setVolume(volumeTrailer);   //updates trailer volume
    
    //soundtrack
    ofSoundUpdate();                          //updates sound engine
    soundtrack.setVolume(volumeSoundtrack);   //updates soundtrack volume
    
}


//--------------------------------------------------------------
void FilmBox::settingVideoControls() {
    if(!trailer.isPlaying()) {
        trailer.play();
        soundtrack.setPaused(true);   //while the trailer is playing, the soundtrack is paused
    } else {
        trailer.setPaused(true);      //while the trailer is paused, the soundtrack is playing
        soundtrack.setPaused(false);  //starts from the last pause
    }
}


//--------------------------------------------------------------
void FilmBox::settingAudioControls(bool b) {
    //handles audio playback
    if(b) {
        if(!soundtrack.isPlaying()) {
            soundtrack.play();
        }
    } else {
        soundtrack.stop();
    }
}


//--------------------------------------------------------------
void FilmBox::setupParametersGroup() {
    //setup ofParameter attributes
    volumeSoundtrack.set("Soundtrack volume", 0.5, 0.f, 1.f);
    volumeTrailer.set("Trailer volume", 0.5, 0.f, 1.f);
    rotationSpeed.set("Rotation speed", 3, 1, 5);
    
    //setup ofParameterGroup
    filmBoxGroup.setName("Movie box parameters");
    
    //add parameters to the group
    filmBoxGroup.add(volumeSoundtrack);
    filmBoxGroup.add(volumeTrailer);
    filmBoxGroup.add(rotationSpeed);
}


//SETTER
//--------------------------------------------------------------
void FilmBox::setId(string idMovie) {
    
    //poster
    ofLoadImage(poster, idMovie + "/" + idMovie + ".jpg");
    poster.generateMipmap();
    poster.setTextureMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_NEAREST);   //antialiasing effect in textures
    
    //information about movie name, director, genres and plot
    ofLoadImage(movieInfoTexture, idMovie + "/" + idMovie + "1.png");
    movieInfoTexture.generateMipmap();
    movieInfoTexture.setTextureMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_NEAREST);   //antialiasing effect in textures
    
    //information about movie awards and nominations
    ofLoadImage(movieAwardsTexture, idMovie + "/" + idMovie + "2.png");
    movieAwardsTexture.generateMipmap();
    movieInfoTexture.setTextureMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_NEAREST);   //antialiasing effect in textures
    
    //background of the inner box
    ofLoadImage(movieBackground, idMovie + "/" + idMovie + "b.jpg");
    movieBackground.mirror(false, true);   //the image is flipped horizontally because inside the box we see its back face
    
    //movie trailer
    trailer.load(idMovie + "/" + idMovie + ".mp4");
    
    //movie soundtrack
    soundtrack.setLoop(true);   //loops the sound
    soundtrack.load(idMovie + "/" + idMovie + ".mp3");
}


//--------------------------------------------------------------
void FilmBox::setWorldPosBox(int x, int y, int z) {
    worldPosBox = ofPoint(x, y, z);
}


//--------------------------------------------------------------
void FilmBox::setScreenPosBox(int x, int y) {
    screenPosBox = ofPoint(x, y);
}


//--------------------------------------------------------------
void FilmBox::setRotationStep(int n) {
    rotationStep = n;
}


//--------------------------------------------------------------
void FilmBox::setRotationBox(int n) {
    rotationBox = n;
}


//--------------------------------------------------------------
void FilmBox::setPlayIconTexture(ofTexture *texture) {
    playIconTexture = texture;
}


//GETTER
//--------------------------------------------------------------
ofPoint FilmBox::getWorldPosBox() {
    return worldPosBox;
}


//--------------------------------------------------------------
ofBoxPrimitive FilmBox::getBox() {
    return outerBox;
}


//--------------------------------------------------------------
int FilmBox::getRotationStep() {
    return rotationStep;
}


//--------------------------------------------------------------
int FilmBox::getRotationBox() {
    return rotationBox;
}


//--------------------------------------------------------------
vector<ofPoint> FilmBox::getTrailerCoords() {
    return trailerCoords;
}


//--------------------------------------------------------------
ofVideoPlayer FilmBox::getTrailer() {
    return trailer;
}
