//Written and made by Michael Wright

#ifndef WCONTEXT_H_INCLUDED
#define WCONTEXT_H_INCLUDED

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>

using namespace std;


static bool windowchange=false;
static int xRez, yRez;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class openGLContext//moved here as c++ irritating me :P
{
public:

    openGLContext()
    {
        reset();
    }

    ~openGLContext()
    {
        purge();
    }

    HDC init(HWND hWnd)//inline to speed up the context
    {
        // remember the window handle (HWND)
        mhWnd = hWnd;

        // get the device context (DC)
        mhDC = GetDC( mhWnd );

        // set the pixel format for the DC
        PIXELFORMATDESCRIPTOR pfd;
        ZeroMemory( &pfd, sizeof( pfd ) );
        pfd.nSize = sizeof( pfd );
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
                      PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 24;
        pfd.cDepthBits = 16;
        pfd.iLayerType = PFD_MAIN_PLANE;
        int format = ChoosePixelFormat( mhDC, &pfd );
        SetPixelFormat( mhDC, format, &pfd );

        // create the render context (RC)
        mhRC = wglCreateContext( mhDC );

        // make it the current render context
        wglMakeCurrent( mhDC, mhRC );
        return mhDC;
    }

    void purge()
    {
        if ( mhRC )
        {
            wglMakeCurrent( NULL, NULL );
            wglDeleteContext( mhRC );
        }
        if ( mhWnd && mhDC )
        {
            ReleaseDC( mhWnd, mhDC );
        }
        reset();
    }

private:

    void reset()
    {
        mhWnd = NULL;
        mhDC = NULL;
        mhRC = NULL;
     }

    HWND mhWnd;
    HDC mhDC;
    HGLRC mhRC;

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* const myclass = "windowClass" ;//class of the window
long __stdcall WindowProcedure( HWND window, unsigned int msg, WPARAM wp, LPARAM lp );//handles window stuff
extern bool KeyboardPress[256];
class OpenGlRenderingContext{//creates a window32 window then creates a rendering context for opengl for that window

    public:
        OpenGlRenderingContext();//default overloaded constructor
        ~OpenGlRenderingContext();//default destructor
        void init(int x, int y, int width, int height);//sets up basic parameters for window creation;
        void mainloop(void (*routingtorun)());//what to do while window is idle
        void setkeys(void (*kh)(bool key[256]));
        void resizehandler(void (*rz)(int xRes, int yRez));

        void glinit(void (*glsetup)());
        void glinit();


    private:
        openGLContext gltest;
        bool glEnabled;
        HWND window;

        HDC wHdc;
        void (*resizer)(int xRes, int yRez);
        void (*keyHandling)(bool key[256]);



};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // WCONTEXT_H_INCLUDED
