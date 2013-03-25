#ifndef OFXJUCE_H
#define OFXJUCE_H

#include <memory>

class JuceToOFBridge
{
	int windowMode;
	bool bNewScreenMode;
	int nFrameCount;
	int nFramesSinceWindowResized;
	bool bFrameRateSet;
	int millisForFrame;
	int prevMillis;
	int diffMillis;
	float timeNow;
	float timeThen;
	float fps;
	float frameRate;
	double lastFrameTime;
	bool bEnableSetupScreen;
	int nonFullScreenX;
	int nonFullScreenY;
public:
	virtual void setupOpenGL(int w, int h, int screenMode) = 0;
	
	virtual void initializeWindow() = 0;
	
	virtual void runAppViaInfiniteLoop() = 0;
	
	virtual void hideCursor() = 0;
	virtual void showCursor() = 0;
	
	virtual void setWindowPosition(int x, int y) = 0;
	virtual void setWindowShape(int w, int h) = 0;
	
	virtual int getFrameNum();
	virtual	float getFrameRate();
	virtual double getLastFrameTime();
	
	virtual int getWindowPositionX() = 0;
	virtual int getWindowPositionY() = 0;
	virtual int getWindowSizeX() = 0;
	virtual int getWindowSizeY() = 0;
	virtual int getScreenSizeX() = 0;
	virtual int getScreenSizeY() = 0;
	
	virtual void setFrameRate(float targetRate);
	virtual void setWindowTitle(const char *title) = 0;
	
	virtual int getWindowMode() = 0;
	
	virtual void setFullscreen(bool fullscreen) = 0;
	virtual void toggleFullscreen() = 0;
	
	virtual void enableSetupScreen() = 0;
	virtual void disableSetupScreen() = 0;
	
	void notifyDraw();
	void notifySetup();
	void notifyUpdate();
	void notifyKeyPressed(int key);
	void notifyKeyReleased(int key);
	void notifyMousePressed(int x, int y, int button);
	void notifyMouseReleased(int x, int y, int button);
	void notifyMouseDragged(int x, int y, int button);
	void notifyMouseMoved(int x, int y);
	void notifyWindowResized(int width, int height);
};

class OFContainer;
typedef std::shared_ptr<OFContainer> OFContainerSharedPtr;

OFContainerSharedPtr InitOFApp(JuceToOFBridge *ofBridge, int w, int h);
extern void ShutdownOFApp(OFContainerSharedPtr &container);

#endif // OFXJUCE_H
