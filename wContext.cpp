//Written and made by Michael Wright
#include <windows.h>
#include <iostream>
#include "wContext.h"

using namespace std;


OpenGlRenderingContext::OpenGlRenderingContext(){glEnabled = false;}//default constructor
OpenGlRenderingContext::~OpenGlRenderingContext(){
    if (glEnabled)
        gltest.purge();


}

void OpenGlRenderingContext::init(int x, int y, int width, int height){
 WNDCLASSEX wndclass = { sizeof(WNDCLASSEX), CS_DBLCLKS, WindowProcedure,
                            0, 0, GetModuleHandle(0), LoadIcon(0,IDI_APPLICATION),
                            LoadCursor(0,IDC_ARROW), HBRUSH(COLOR_WINDOW+1),
                            0, myclass, LoadIcon(0,IDI_APPLICATION) } ;
    if( RegisterClassEx(&wndclass) )
    {
        window = CreateWindowEx( 0, myclass, "Mandelbrot 201176962",
                   WS_OVERLAPPEDWINDOW, x, y,
                   width, height, 0, 0, GetModuleHandle(0), 0 ) ;

    }

}

void OpenGlRenderingContext::mainloop(void (*routingtorun)(void)){
if(window)
        {
            ShowWindow( window, SW_SHOWDEFAULT ) ;
            MSG msg ;

            while (true){
                if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE )){
                    if ( msg.message == WM_QUIT ){
                        break;
                    }else{
                        TranslateMessage( &msg );
                        DispatchMessage( &msg );
                        }
                }else{//opengl code goes here

                    (*routingtorun)();//call the drawing routine continously
                    if ((keyHandling)!=NULL)
                        (keyHandling)(KeyboardPress);

                    if ((resizer)!=NULL)
                        if(windowchange==true){
                            windowchange=false;
                            resizer(xRez, yRez);
                        }

                     wglSwapLayerBuffers( wHdc, WGL_SWAP_MAIN_PLANE );//SwapBuffers( GetDC(window) );//display to screen
                }
            }
        }
        else{
            cerr<<"No window Created to show!";
        }

            //while( GetMessage( &msg, 0, 0, 0 ) ){//simple code to do same as above
            //    DispatchMessage(&msg) ;
           //     (*f)();
           // }


}

void OpenGlRenderingContext::glinit(){
    glEnabled = true;//for destructor
    (keyHandling) = NULL;
    (resizer) = NULL;
    wHdc = gltest.init(window);

}

void OpenGlRenderingContext::glinit(void (*glsetup)()){
    glinit();
    (*glsetup)();//call the drawing routine continously
}

void OpenGlRenderingContext::setkeys(void (*kh)(bool key[256])){
    (keyHandling) =(kh);
}


void OpenGlRenderingContext::resizehandler(void (*rz)(int xRez, int yRez)){
    (resizer) =(rz);
}

//handling the window and everything to do with it
bool KeyboardPress[256];
long __stdcall WindowProcedure( HWND window, unsigned int msg, WPARAM wp, LPARAM lp )
{
    switch(msg)
    {
        case WM_DESTROY:
            cout << "\nDestroying window\n" ;
            PostQuitMessage(0) ;
            return 0 ;
        case WM_LBUTTONDOWN:
            std::cout << "\nmouse left button down at (" << LOWORD(lp) << ',' << HIWORD(lp) << ")\n" ;
            return 0 ;
        case WM_KEYDOWN:
            KeyboardPress[wp]=true;
           return 0 ;
		case WM_KEYUP:
            KeyboardPress[wp]=false;
            return 0 ;
        case WM_SIZE:
                xRez = LOWORD(lp);
                yRez = HIWORD(lp);
                windowchange=true;
                glViewport(0,0,LOWORD(lp),HIWORD(lp));//resizes window!
           return 0 ;

        default:
            return DefWindowProc( window, msg, wp, lp ) ;
    }
}
