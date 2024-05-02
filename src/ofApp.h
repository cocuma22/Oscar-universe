#pragma once

#include "ofMain.h"
#include "FilmBox.h"
#include "ofxAssimpModelLoader.h"   //addon to load 3D model
#include "ofxJSON.h"                //addon to read JSON file
#include "ofxGui.h"                 //addon to show GUI
#include "ofxRaycaster.h"           //addon to do raycasting

class ofApp : public ofBaseApp{
    private:
        ofTrueTypeFont font;   //font to use in texts
        string helpText;       //text to show which keyboard commands are currently available
    
        //GUI
        ofxPanel boxPanel;                    //ofxPanel instance for movie box parameters
        ofxPanel universePanel;               //ofxPanel instance for universe parameters
        ofParameter<bool> help;               //flag to show or hide help text
        ofParameterGroup boxGroup;            //instance of ofParameterGroup for the ofApp class from FilmBox class
        ofParameterGroup universeGroup;       //instance of ofParameterGroup to group universe parameters
        ofParameterGroup wrapperGroupUni;     //instance of ofParameterGroup to which to add group 'universeGroup'
    
        //camera
        ofCamera camera;
        ofPoint cameraOrigin;        //default camera position
        ofPoint targetBoxPosition;   //position of the selected movie box
        bool moveCameraEnabled;      //this flag is true if the camera is moving
        bool isZoomingInsideBox;     //this flag is true if the camera is zooming inside a movie box
    
        //movie box rotation
        bool rotateBoxRight;         //this flag is true if the right arrow is pressed
        bool rotateBoxLeft;          //this flag is true if the left arrow is pressed
    
        //3D model
        ofxAssimpModelLoader model;   //3D model of the Oscar statuette
        bool isModelRotated;          //this flag is true if the model is rotated by 180° on the y axis
    
        //file JSON
        ofxJSONElement data;   //JSON file containing all movies information
    
        //movies
        FilmBox * movieSelected;   //pointer to the movie box currently selected
        vector<FilmBox> movies;    //vector of movies
        float angle;               //angle where to place the movie box
        float angleOffset;         //offset angle between movies
        int distance;              //distance of the movies from the Oscar statuette
    
        //scroll movies
        float stepScrollBox;   //step box rotation around Oscar statuette 
        bool scrollBoxEnable;  //this flag is true if movie boxes are sliding around the Oscar statuette
    
        //textures
        ofTexture backgroundImage;   //texture to use as background
        ofTexture playIcon;          //play icon to show on video trailer when it is paused
    
        //raycasting
        ofxraycaster::Mousepicker mousepicker;             //ray from camera position to mouse position
        glm::vec2 baricentricCoordinates;
        glm::vec3 surfaceNormal;
        float dist;
        bool intersects;                                   //true if the ray intersects a movie box
        int indexIntersectedPrimitive;                     //index of movie box intersected by the ray
        bool foundIntersection;                            //true if an intersection is found
    
        //lights
        ofLight lightBox;                //light inside selected movie box
        ofLight lHeadRight, lHeadLeft;   //lights to illuminate the sides of the model head
        ofLight lBodyRight, lBodyLeft;   //lights to illuminate the sides of the model
        ofLight lFace;                   //front light to illuminate the face of the model
        ofLight lBody;                   //front light to illuminate the body of the model
        ofLight lBase;                   //light to illuminate the base of the model
    
	public:
		void setup();
		void update();
		void draw();
        void exit();
        void getData(string file);           //retrieves data stored in JSON file and saves them in vector 'movies'
        void moveCamera(ofPoint target);     //moves the camera inside and outside the movie box
        void rotateMovieBox();               //rotates the selected movie box when right or left arrow key is released
        void drawBoxesAndSelection();        //draws boxes around the Oscar and highlights the selected one
        void setupLights();                  //setup the lights of the Oscar statuette
        void updatePositionLights();         //update positions lights of the Oscar statuette
        void showHelp();                     //show possible keyboard commands
        void modelRotation();                //rotates the model by 180° on the y axis
        void setupGUIs();                    //setups GUIs
		void keyReleased(int key);
		void mouseMoved(int x, int y );
        void mouseReleased(int x, int y, int button);
};
