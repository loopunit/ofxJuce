#ifndef OFXJUCEOPENGLCANVAS_H
#define OFXJUCEOPENGLCANVAS_H

#include "JuceHeader.h"
#include "ofxJuce.h"

namespace juce
{
	class ofxJuceOpenGLCanvas
	:	public Component,
	public OpenGLRenderer,
	public Timer,
	public JuceToOFBridge
	{
		OFContainerSharedPtr mOFContainer;
		Array< std::pair<int, int> > mKeysDown;
	public:
		ofxJuceOpenGLCanvas()
		{
			openGLContext.setRenderer(this);
			openGLContext.setComponentPaintingEnabled(true);
			openGLContext.attachTo(*this);
			
			setWantsKeyboardFocus(true);
			
			startTimer(1000 / 60);
		}
		
		virtual ~ofxJuceOpenGLCanvas()
		{
			openGLContext.detach();
		}
		
		virtual void newOpenGLContextCreated()
		{
			mOFContainer = InitOFApp(this, getWidth(), getHeight());
			notifySetup();
		}
		
		virtual void openGLContextClosing()
		{
		}
		
		virtual void mouseDown(const MouseEvent &e)
		{
			notifyMousePressed(e.x, e.y, e.mods.isLeftButtonDown() ? 0 : 1);
		}
		
		virtual void mouseUp(const MouseEvent &e)
		{
			notifyMouseReleased(e.x, e.y, e.mods.isLeftButtonDown() ? 0 : 1);
		}
		
		virtual void mouseDrag(const MouseEvent &e)
		{
			notifyMouseDragged(e.x, e.y, e.mods.isLeftButtonDown() ? 0 : 1);
		}
		
		void mouseMove(const MouseEvent &e)
		{
			notifyMouseMoved(e.x, e.y);
		}
		
		virtual bool keyPressed(const KeyPress &kp)
		{
			notifyKeyPressed(kp.getTextCharacter());
			mKeysDown.add(std::pair<int, int>(kp.getKeyCode(), kp.getTextCharacter()));
			return true;
		}
		
		virtual bool keyStateChanged(const bool isKeyDown)
		{
			for (int i = 0; i < mKeysDown.size(); ++i)
			{
				if (!KeyPress::isKeyCurrentlyDown(mKeysDown[i].first))
				{
					notifyKeyReleased(mKeysDown[i].second);
					mKeysDown.set(i, std::pair<int, int>(-1, -1));
				}
			}
			
			mKeysDown.removeAllInstancesOf(std::pair<int, int>(-1, -1));
			return true;
		}
		
		virtual void resized()
		{
			notifyWindowResized(getWidth(), getHeight());
		}
		
		virtual void paint(Graphics&)
		{
		}
		
		virtual void renderOpenGL()
		{
			notifyUpdate();
			notifyDraw();
		}
		
		virtual double getScale() const
		{
			return Desktop::getInstance().getDisplays().getDisplayContaining(getScreenBounds().getCentre()).scale;
		}
		
		virtual int getContextWidth() const
		{
			return roundToInt(getScale() * getWidth());
		}
		
		virtual int getContextHeight() const
		{
			return roundToInt(getScale() * getHeight());
		}
		
		virtual void timerCallback()
		{
			openGLContext.triggerRepaint();
		}
		
		virtual bool confirmAndQuit()
		{
			stopTimer();
			ShutdownOFApp(mOFContainer);
			return true;
		}
		
	public:
		// OF bridge functionality - most of these do nothing by default
		virtual void setupOpenGL(int w, int h, int screenMode) { }
		
		virtual void initializeWindow() { }
		
		virtual void runAppViaInfiniteLoop() { }
		
		virtual void hideCursor() { }
		virtual void showCursor() { }
		
		virtual void setWindowPosition(int x, int y) { }
		virtual void setWindowShape(int w, int h) { }
		
		virtual int getWindowPositionX() { return 0; }
		virtual int getWindowPositionY() { return 0; }
		
		virtual int getWindowSizeX() { return getWidth(); }
		virtual int getWindowSizeY() { return getHeight(); }
		
		virtual int getScreenSizeX() { return getWidth(); }
		virtual int getScreenSizeY() { return getHeight(); }
		
		virtual void setWindowTitle(const char *title) { }
		
		virtual int getWindowMode() { return 0; }
		
		virtual void setFullscreen(bool fullscreen) { }
		virtual void toggleFullscreen() { }
		
		virtual void enableSetupScreen() { }
		virtual void disableSetupScreen() { }
		
	private:
		OpenGLContext openGLContext;
	};

} // namespace juce

#endif // OFXJUCEOPENGLCANVAS_H