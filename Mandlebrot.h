#ifndef MANDLEBROT_H_INCLUDED
#define MANDLEBROT_H_INCLUDED


//default window size
const int DEFAULTHEIGHT = 500;
const int DEFAULTWIDTH = 500;



//shader code
void shaderINIT();
void shadermkeys(bool PresedKeys[256]);
void shaderLOOPmain();

//sequential NO OMP
void sequentialINIT();
void sequentialKEYS(bool PresedKeys[256]);
void sequentialLOOP();
void sequentialResizeer(int xRezer, int yRezer);

//SSE ONLY
void sseINIT();
void sseKEYS(bool PresedKeys[256]);
void sseLOOP();



//OMP Sequential
void ompINIT();
void ompKEYS(bool PresedKeys[256]);
void ompLOOP();


//OMP optimized (blockwise, cachelines)
void ompOPINIT();
void ompOPKEYS(bool PresedKeys[256]);
void ompOPLOOP();




//helpers
void displayFPS(bool drawem);




#endif // MANDLEBROT_H_INCLUDED
