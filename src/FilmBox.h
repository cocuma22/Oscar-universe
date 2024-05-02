#pragma once

#include "ofMain.h"   //includes the Header file of OpenFrameworks

class FilmBox {
    
    //typedef
    typedef ofPoint dimensions;   //this typedef is used to save width, height and depth values in a single variable
    
    private:
        //ATTRIBUTES
        ofTexture poster;               //movie poster
        ofTexture movieInfoTexture;     //texture with information about the movie (name, director, genres, plot)
        ofTexture movieAwardsTexture;   //texture with information about nominations  and awards won by the movie
        ofTexture * playIconTexture;    //pointer to play icon texture to show on the video trailer when it is paused
    
        ofImage movieBackground;    //image to use as background of the inner movie box
        ofVideoPlayer trailer;      //movie trailer
        ofSoundPlayer soundtrack;   //movie soundtrack
    
        ofBoxPrimitive outerBox;   //external movie box (it is covered with the movie poster)
        ofBoxPrimitive innerBox;   //inner movie box (it shows the information about the movie)
    
        dimensions dimensionBox;        //dimensions of the external movie box
        dimensions dimensionInnerBox;   //dimensions of the inner movie box
        dimensions textureDimension;    //dimension of all inner box textures
        dimensions dimensionTrailer;    //dimension of movie trailer
    
        vector<ofPoint> trailerCoords;   //vector of trailer coordinates [Top-Left, Bottom-Right]
        ofPoint texturePosition;         //position of all inner box textures
        ofPoint worldPosBox;             //movie box position in world coordinates
        ofPoint screenPosBox;            //movie box position in screen coordinates
    
        int rotationBox;    //angle rotation of the movie box
        int rotationStep;   //current movie box rotation performed (from 0° to 90°)
    
        bool isBoxHorizontal;   //if true, the movie box is drawn horizontally
    
        //GUI attributes
        static ofParameter<float> volumeSoundtrack;   //soundtrack volume in range [0.f - 1.f]
        static ofParameter<float> volumeTrailer;      //trailer volume in range [0.f - 1.f]
        static ofParameter<int> rotationSpeed;        //film box rotation speed in range [1 - 5]
    
    public:
        static ofParameterGroup filmBoxGroup;   //ofParameter container for GUI communication
        //INTERFACE
        FilmBox();    //FilmBox class constructor
        ~FilmBox();   //FilmBox class decontructor. It is invoked automatically when it is called a delete on an object
    
        //SETTER
        void setId(string idMovie);
        void setWorldPosBox(int x, int y, int z);
        void setScreenPosBox(int x, int y);
        void setRotationStep(int n);
        void setRotationBox(int n);
        void setPlayIconTexture(ofTexture * texture);
    
        //GETTER
        ofPoint getWorldPosBox();
        ofBoxPrimitive getBox();
        int getRotationStep();
        int getRotationBox();
        vector<ofPoint> getTrailerCoords();
        ofVideoPlayer getTrailer();
    
        //METHODS
        void display();            //draw the FilmBox object
        void rotateBox(char orientation);                        //rotate movie box, orientation can only be 'r' or 'l'
        void update();                                           //update trailer frame and soundtrack of the movie
        void settingVideoControls();                             //set video trailer to play or pause
        void settingAudioControls(bool b);                       //set soundtrack to play or pause
        static void setupParametersGroup();                      //add parameters to ParameterGroup
};
