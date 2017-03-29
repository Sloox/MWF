#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#include <iostream>
#include "wContext.h"
#include <cmath>
#include <gl/glut.h>
#include "Mandlebrot.h"
#include <cstdlib>


using namespace std;

//TODO : WINDOW CLEANUP!!!
void startmandle(int choice);

void Menu(){
    int i = 0;
    system("cls");
    cout<<"----------------Parallel Mandlebrot------------"<<endl<<endl;
    cout<<"Written for IT00197 by Wright, MJ"<<endl;
    cout<<"Select an option to begin"<<endl;
    cout<<"1)Sequential Mandlebrot(Baseline)"<<endl;
    cout<<"2)SSE optimised Mandlebrot"<<endl;
    cout<<"3)OMP optimised Mandlebrot"<<endl;
    cout<<"4)SSE+OMP optimised Mandlebrot"<<endl;
    cout<<"5)Opengl GLSL Mandlebrot"<<endl;
    cout<<"Choice:";

    cin>>i;
    if(cin.fail()){
        cin.clear();
        Menu();
    }
    else{
        startmandle(i);
    }

}

void startmandle(int choice){
    OpenGlRenderingContext Mandlebrot;//creates a window
    switch(choice){
        case 1:
                Mandlebrot.init(50,50,DEFAULTHEIGHT,DEFAULTWIDTH);//posX,posY,widht,height of window
                Mandlebrot.glinit(sequentialINIT);//init code here!
                Mandlebrot.setkeys(sequentialKEYS);
                Mandlebrot.resizehandler(sequentialResizeer);
                Mandlebrot.mainloop(sequentialLOOP);//rending code here!
            break;
        case 2:
                Mandlebrot.init(50,50,DEFAULTHEIGHT,DEFAULTWIDTH);//posX,posY,widht,height of window
                Mandlebrot.glinit(sseINIT);//init code here!
                Mandlebrot.setkeys(sseKEYS);
                Mandlebrot.mainloop(sseLOOP);//rending code here!
            break;
        case 3:
                Mandlebrot.init(50,50,DEFAULTHEIGHT,DEFAULTWIDTH);//posX,posY,widht,height of window
                Mandlebrot.glinit(ompINIT);//init code here!
                Mandlebrot.setkeys(ompKEYS);
                Mandlebrot.mainloop(ompLOOP);//rending code here!
            break;
        case 4:
                Mandlebrot.init(50,50,DEFAULTHEIGHT,DEFAULTWIDTH);//posX,posY,widht,height of window
                Mandlebrot.glinit(ompOPINIT);//init code here!
                Mandlebrot.setkeys(ompOPKEYS);
                Mandlebrot.mainloop(ompOPLOOP);//rending code here!
            break;
        case 5:
                Mandlebrot.init(50,50,DEFAULTHEIGHT,DEFAULTWIDTH);//posX,posY,widht,height of window
                Mandlebrot.glinit(shaderINIT);//init code here!
                Mandlebrot.setkeys(shadermkeys);
                Mandlebrot.mainloop(shaderLOOPmain);//rending code here!
            break;
        default:
        Menu();
    }
    return;
}
int main()
{
    Menu();

    return 0;
}
