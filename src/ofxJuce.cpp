#include "OFXJuce.h"

#include "ofMain.h"
#include "ofAppBaseWindow.h"
#include "ofEvents.h"

#ifdef TARGET_WIN32
#define GLUT_BUILDING_LIB
#include "glut.h"
#endif
#ifdef TARGET_OSX
#include <Carbon/Carbon.h>
#endif
#ifdef TARGET_LINUX
#include <GL/glut.h>
#endif

float JuceToOFBridge::getFrameRate()
{
	return frameRate;
}

double JuceToOFBridge::getLastFrameTime()
{
	return lastFrameTime;
}

int JuceToOFBridge::getFrameNum(){
	return nFrameCount;
}

void JuceToOFBridge::setFrameRate(float targetRate)
{
	// given this FPS, what is the amount of millis per frame
	// that should elapse?
	
	// --- > f / s
	
	if (targetRate == 0){
		bFrameRateSet = false;
		return;
	}
	
	bFrameRateSet 			= true;
	float durationOfFrame 	= 1.0f / (float)targetRate;
	millisForFrame 			= (int)(1000.0f * durationOfFrame);
	
	frameRate				= targetRate;
}

void JuceToOFBridge::notifyDraw()
{
	int windowX = getWindowSizeX();
	int windowY = getWindowSizeY();
	
	ofSetupScreenOrtho(windowX, windowY, OF_ORIENTATION_DEFAULT, true, 0, -1.0f);
	
	if (getWindowMode() != OF_GAME_MODE){
		if ( bNewScreenMode ){
			if( getWindowMode() == OF_FULLSCREEN){
				
				//----------------------------------------------------
				// before we go fullscreen, take a snapshot of where we are:
				nonFullScreenX = windowX;
				nonFullScreenY = windowY;
				//----------------------------------------------------
				
				//glutFullScreen();
				
#ifdef TARGET_OSX
				SetSystemUIMode(kUIModeAllHidden,NULL);
#ifdef MAC_OS_X_VERSION_10_7 //needed for Lion as when the machine reboots the app is not at front level
				if( nFrameCount <= 10 ){  //is this long enough? too long?
					ProcessSerialNumber psn;
					OSErr err = GetCurrentProcess( &psn );
					if ( err == noErr ){
						SetFrontProcess( &psn );
					}
				}
#endif
				
			}else if( windowMode == OF_WINDOW ){
				
				//glutReshapeWindow(requestedWidth, requestedHeight);
				
				//----------------------------------------------------
				// if we have recorded the screen posion, put it there
				// if not, better to let the system do it (and put it where it wants)
				if (nFrameCount > 0){
					//glutPositionWindow(nonFullScreenX,nonFullScreenY);
				}
				//----------------------------------------------------
				
#ifdef TARGET_OSX
				SetSystemUIMode(kUIModeNormal,NULL);
#endif
			}
			bNewScreenMode = false;
		}
	}
#endif
	
	// set viewport, clear the screen
	ofViewport(0, 0, windowX, windowY);		// used to be glViewport( 0, 0, width, height );

	float * bgPtr = ofBgColorPtr();
	bool bClearAuto = ofbClearBg();
	
    // to do non auto clear on PC for now - we do something like "single" buffering --
    // it's not that pretty but it work for the most part
	
#ifdef TARGET_WIN32
    if (bClearAuto == false){
        glDrawBuffer (GL_FRONT);
    }
#endif
	
	if ( bClearAuto == true || nFrameCount < 3)
	{
		ofClear(bgPtr[0]*255,bgPtr[1]*255,bgPtr[2]*255, bgPtr[3]*255);
	}
	
	if (bEnableSetupScreen)
	{
		ofSetupScreen();
	}
	
	ofNotifyDraw();
	
#ifdef TARGET_WIN32
    if (bClearAuto == false){
        // on a PC resizing a window with this method of accumulation (essentially single buffering)
        // is BAD, so we clear on resize events.
        if (nFramesSinceWindowResized < 3){
        	ofClear(bgPtr[0]*255,bgPtr[1]*255,bgPtr[2]*255, bgPtr[3]*255);
        }
//		else
//		{
//            if ( (nFrameCount < 3 || nFramesSinceWindowResized < 3) && bDoubleBuffered)
//				glutSwapBuffers();
//            else
//				glFlush();
//        }
    }
//	else
//	{
        // afaik - juce handles this
		//if(bDoubleBuffered){
		//	glutSwapBuffers();
		//} else{
		//	glFlush();
		//}
//  }
#else
	if (bClearAuto == false){
		// in accum mode resizing a window is BAD, so we clear on resize events.
		if (nFramesSinceWindowResized < 3){
			ofClear(bgPtr[0]*255,bgPtr[1]*255,bgPtr[2]*255, bgPtr[3]*255);
		}
	}

	// afaik - juce handles this
	//if(bDoubleBuffered){
	//	glutSwapBuffers();
	//} else{
	//	glFlush();
	//}
#endif
	
    nFramesSinceWindowResized++;
	
	//fps calculation moved to idle_cb as we were having fps speedups when heavy drawing was occuring
	//wasn't reflecting on the actual app fps which was in reality slower.
	//could be caused by some sort of deferred drawing?
	
	nFrameCount++;		// increase the overall frame count
	
	//setFrameNum(nFrameCount); // get this info to ofUtils for people to access
	

}

void JuceToOFBridge::notifySetup()
{
	timeNow				= 0;
	timeThen			= 0;
	fps					= 60.0; //give a realistic starting value - win32 issues
	frameRate			= 60.0;
	windowMode			= OF_WINDOW;
	bNewScreenMode		= true;
	//nFramesForFPS		= 0;
	nFramesSinceWindowResized = 0;
	nFrameCount			= 0;
	//buttonInUse			= 0;
	bEnableSetupScreen	= true;
	bFrameRateSet		= false;
	millisForFrame		= 0;
	prevMillis			= 0;
	diffMillis			= 0;
	//requestedWidth		= 0;
	//requestedHeight		= 0;
	nonFullScreenX		= -1;
	nonFullScreenY		= -1;
	lastFrameTime		= 0.0;
	//displayString		= "";
	//orientation			= OF_ORIENTATION_DEFAULT;
	//bDoubleBuffered = true; // LIA

	ofNotifySetup();
}

void JuceToOFBridge::notifyUpdate()
{
	if (nFrameCount != 0 && bFrameRateSet == true){
		diffMillis = ofGetElapsedTimeMillis() - prevMillis;
		if (diffMillis > millisForFrame){
			; // we do nothing, we are already slower than target frame
		} else {
			int waitMillis = millisForFrame - diffMillis;
#ifdef TARGET_WIN32
			Sleep(waitMillis);         //windows sleep in milliseconds
#else
			usleep(waitMillis * 1000);   //mac sleep in microseconds - cooler :)
#endif
		}
	}
	prevMillis = ofGetElapsedTimeMillis(); // you have to measure here
	
    // -------------- fps calculation:
	// theo - now moved from display to idle_cb
	// discuss here: http://github.com/openframeworks/openFrameworks/issues/labels/0062#issue/187
	//
	//
	// theo - please don't mess with this without letting me know.
	// there was some very strange issues with doing ( timeNow-timeThen ) producing different values to: double diff = timeNow-timeThen;
	// http://www.openframeworks.cc/forum/viewtopic.php?f=7&t=1892&p=11166#p11166
	
	timeNow = ofGetElapsedTimef();
	double diff = timeNow-timeThen;
	if( diff  > 0.00001 ){
		fps			= 1.0 / diff;
		frameRate	*= 0.9f;
		frameRate	+= 0.1f*fps;
	}
	lastFrameTime	= diff;
	timeThen		= timeNow;
  	// --------------
	
	ofNotifyUpdate();
}

void JuceToOFBridge::notifyKeyPressed(int key)
{
	ofNotifyKeyPressed(key);
}

void JuceToOFBridge::notifyKeyReleased(int key)
{
	ofNotifyKeyReleased(key);
}

void JuceToOFBridge::notifyMousePressed(int x, int y, int button)
{
	ofNotifyMousePressed(x, y, button);
}

void JuceToOFBridge::notifyMouseReleased(int x, int y, int button)
{
	ofNotifyMouseReleased(x, y, button);
}

void JuceToOFBridge::notifyMouseDragged(int x, int y, int button)
{
	ofNotifyMouseDragged(x, y, button);
}

void JuceToOFBridge::notifyMouseMoved(int x, int y)
{
	ofNotifyMouseMoved(x, y);
}

void JuceToOFBridge::notifyWindowResized(int width, int height)
{
	ofNotifyWindowResized(width, height);
}

////

extern ofCoreEvents &ofEvents();
extern ofEventArgs voidEventArgs;

class ofxAppJuceWindow : public ofAppBaseWindow {
	JuceToOFBridge *mBridge;
public:
	ofxAppJuceWindow(JuceToOFBridge *Bridge) : mBridge(Bridge) { }
	virtual ~ofxAppJuceWindow() { }

	virtual void setupOpenGL(int w, int h, int screenMode) { mBridge->setupOpenGL(w, h, screenMode); }

	virtual void initializeWindow() { mBridge->initializeWindow(); }

	virtual void runAppViaInfiniteLoop(ofBaseApp * appPtr) { mBridge->runAppViaInfiniteLoop(); }

	virtual void hideCursor() { mBridge->hideCursor(); }
	virtual void showCursor() { mBridge->showCursor(); }

	virtual void setWindowPosition(int x, int y) { mBridge->setWindowPosition(x, y); }
	virtual void setWindowShape(int w, int h) { mBridge->setWindowShape(w, h); }

	virtual int getFrameNum() { return mBridge->getFrameNum(); }
	virtual	float getFrameRate() { return mBridge->getFrameRate(); }
	virtual double getLastFrameTime() { return mBridge->getLastFrameTime(); }

	virtual ofPoint getWindowPosition()
	{
		return ofPoint(float(mBridge->getWindowPositionX()), float(mBridge->getWindowPositionY()));
	}

	virtual ofPoint getWindowSize()
	{
		return ofPoint(float(mBridge->getWindowSizeX()), float(mBridge->getWindowSizeY()));
	}
	
	virtual ofPoint getScreenSize()
	{
		return ofPoint(float(mBridge->getScreenSizeX()), float(mBridge->getScreenSizeY()));
	}

	virtual int	getWidth()
	{
		return mBridge->getWindowSizeX();
	}
	
	virtual int getHeight()
	{
		return mBridge->getWindowSizeX();
	}

	virtual void setFrameRate(float targetRate) { mBridge->setFrameRate(targetRate); }
	
	virtual void setWindowTitle(string title) { mBridge->setWindowTitle(title.c_str()); }

	virtual int getWindowMode() { return mBridge->getWindowMode(); }

	virtual void setFullscreen(bool fullscreen) { mBridge->setFullscreen(fullscreen); }
	virtual void toggleFullscreen() { mBridge->toggleFullscreen(); }

	virtual void enableSetupScreen() { mBridge->enableSetupScreen(); }
	virtual void disableSetupScreen() { mBridge->disableSetupScreen(); }
};

typedef std::shared_ptr<ofxAppJuceWindow> ofxAppJuceWindowSharedPtr;

////

extern ofBaseApp *CreateApp();
typedef std::shared_ptr<ofBaseApp> ofBaseAppSharedPtr;

////

class OFContainer
{
public:
	OFContainer() { }
	~OFContainer() { }
	
	ofxAppJuceWindowSharedPtr mJuceWindow;
	ofBaseApp* mApp;
};

////

extern void ShutdownOFApp(OFContainerSharedPtr &container)
{
	// ??
	exitApp();
}

OFContainerSharedPtr InitOFApp(JuceToOFBridge *ofBridge, int w, int h)
{
	OFContainer *newOFContainer = new OFContainer();
	newOFContainer->mJuceWindow.reset(new ofxAppJuceWindow(ofBridge));
	newOFContainer->mApp = CreateApp();
	ofSetupOpenGL(newOFContainer->mJuceWindow.get(), w, h, OF_WINDOW);
	ofRunApp(newOFContainer->mApp);
	return OFContainerSharedPtr(newOFContainer);
}

