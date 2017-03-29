/*
    Created by Michael John Wright, 201176962
    2014

    Different attempts at creating mandlebrot implementations with parallelism in mind.
*/
#include <iostream>
#include <cmath>
#include <GL/gl.h>
#include <GL/glu.h>
#include <gl/glut.h>
#include <stdio.h>
#include <windows.h>
#include "Mandlebrot.h"
#include "glext.h"
#include <time.h>
# include <omp.h>


//SSE STUFF
#ifdef __SSE__
#include <xmmintrin.h>
#endif



using namespace std;

//Shader
void GenerateMandle(int xres, int yres);
GLuint IterateMandelbrot(float a, float b );

//SSE BASIC
void GenMandleBrotSSE(int xres, int yres );
__m128i IterateMandelbrotSSE( __m128 a, __m128 b );

//BASIC OMP
void GenerateMandleOMP(int xres, int yres);
GLuint IterateMandelbrotOMP(float a, float b );

//OMP OPT
void GenMandleBrotOMPOPT(int xres, int yres);
GLuint IterateMandelbrotOPTOMP(float a, float b );
UCHAR*  MandleSingleBlock(int from, int to, int xres, int yres, const float ixres, const float iyres);

//glwindow globals
int font=(int)GLUT_BITMAP_8_BY_13;
int frame2,time2,timebase2=0;
char strz[30];

//NONSHADER GLOBALS
//sequential
static float sIters = 512;
static float xRESO = DEFAULTWIDTH, yRESO = DEFAULTHEIGHT;
//timers
clock_t init, final;
//OMP
static int threads = 4;
//OMPOPT
static int sliceSize = 3;
//sse
static unsigned int buffer[DEFAULTHEIGHT*DEFAULTWIDTH];

GLubyte picture[DEFAULTHEIGHT*DEFAULTWIDTH];


///OMP OPTIMIZED START------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///Code is optimized to try work with OPENMP
///and other optimizations to take into account cache, cpu architectures and feature
///OMP OPTIMIZED START------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void ompOPINIT(){

    GenMandleBrotOMPOPT(DEFAULTWIDTH,DEFAULTHEIGHT);

    glClearColor (1.0, 1.0, 1.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);


	 GLfloat redmap[256],
        greenmap[256],
		bluemap[256];
	for ( int i = 0; i < 256; i++ ){
         redmap[i]=(i)/255.0;
         greenmap[i]=(i)/255.0;
         bluemap[i]=(i)/255.0;
    }

    glPixelMapfv(GL_PIXEL_MAP_I_TO_R, 256, redmap);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_G, 256, greenmap);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_B, 256, bluemap);
    threads = omp_get_max_threads();


}
void ompOPKEYS(bool PresedKeys[256]){
        if (PresedKeys['Q']==true){
            PresedKeys['Q']= false;
            exit(0);//quit
    }
    if (PresedKeys['I']==true){
         PresedKeys['I']= false;
         sIters+=100;
         cerr<<"iterations:"<<sIters<<endl;
    }
    if (PresedKeys['D']==true){
        PresedKeys['D']= false;
         sIters-=100;
         cerr<<"iterations:"<<sIters<<endl;
    }
    if (PresedKeys['L']==true){
        PresedKeys['L']= false;
        if(sliceSize>128)
            sliceSize-=128;
         cerr<<"sliceSize:"<<sliceSize<<endl;
    }

     if (PresedKeys['O']==true){
        PresedKeys['O']= false;
        sliceSize+=128;
        cerr<<"sliceSize:"<<sliceSize<<endl;
    }
    if (PresedKeys['T']==true){
        PresedKeys['T']= false;
        if(threads!=1)
            threads-=1;
         cerr<<"threads:"<<threads<<endl;
    }
    if (PresedKeys['G']==true){
        PresedKeys['G']= false;
         threads+=1;
         cerr<<"threads:"<<threads<<endl;
    }

}
void ompOPLOOP(){
    GenMandleBrotOMPOPT(DEFAULTHEIGHT,DEFAULTWIDTH);
  	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
  	gluOrtho2D(-1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
	glClear( GL_COLOR_BUFFER_BIT );
    glDrawPixels( DEFAULTHEIGHT, DEFAULTWIDTH, GL_COLOR_INDEX, GL_UNSIGNED_INT, buffer );
    displayFPS(false);

}

void GenMandleBrotOMPOPT(int xres, int yres ){
     omp_set_num_threads(threads);
    __m128i  *buffer4 = (__m128i *)buffer;
    const __m128 ixres = _mm_set1_ps( 1.0f/(float)xres );
    const __m128 iyres = _mm_set1_ps( 1.0f/(float)yres );

    int j=0,i=0;
    int iCnt = 0;
    #pragma omp parallel for schedule(dynamic) private(iCnt ,i, j)
    for( j=0; j < yres; j++ )
    for( i=0; i < xres; i+=4 )
    {
         __m128  a, b;

        a = _mm_set_ps( i+3, i+2, i+1, i+0 );
        a = _mm_mul_ps( a, ixres );
        a = _mm_mul_ps( a, _mm_set1_ps( 3.00f) );
        a = _mm_add_ps( a, _mm_set1_ps(-2.25f) );

        b = _mm_set1_ps( (float)j );
        b = _mm_mul_ps( b, iyres );
        b = _mm_mul_ps( b, _mm_set1_ps(-2.24f) );
        b = _mm_add_ps( b, _mm_set1_ps( 1.12f) );


        iCnt = ((j * (DEFAULTHEIGHT)) + i)/4;

        _mm_store_si128( buffer4+iCnt, IterateMandelbrotSSE( a, b ) );


    }
}

inline __m128i IterateMandelbrotOMPOPT( __m128 a, __m128 b ){
    __m128  x, y, x2, y2, m2;
    __m128  co, ite;

    unsigned int i;

    const __m128 one = _mm_set1_ps(1.0f);
    const __m128 th  = _mm_set1_ps(4.0f);


        x   = _mm_setzero_ps();
        y   = _mm_setzero_ps();
        x2  = _mm_setzero_ps();
        y2  = _mm_setzero_ps();
        co  = _mm_setzero_ps();
        ite = _mm_setzero_ps();

    volatile bool flag=false;

    for( i=0; i < 512; i++ ){

        if(flag)continue;
        y  = _mm_mul_ps( x, y );
        y  = _mm_add_ps( _mm_add_ps(y,y),   b );
        x  = _mm_add_ps( _mm_sub_ps(x2,y2), a );

        x2 = _mm_mul_ps( x, x );
        y2 = _mm_mul_ps( y, y );

        m2 = _mm_add_ps(x2,y2);
        co = _mm_or_ps( co, _mm_cmpgt_ps( m2, th ) );


        ite = _mm_add_ps( ite, _mm_andnot_ps( co, one ) );
        if( _mm_movemask_ps( co )==0x0f )
           flag = true;
        }

    // create color
    const __m128i bb = _mm_cvtps_epi32( ite );
    const __m128i gg = _mm_slli_si128( bb, 1 );
    const __m128i rr = _mm_slli_si128( bb, 2 );
    const __m128i color = _mm_or_si128( _mm_or_si128(rr,gg),bb );

    return( color );
}

///OMP OPTIMIZED END------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///
///OMP OPTIMIZED END------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------



///OMP START------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///basic use of OPENMP to try increase performace
///OMP START------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void ompINIT(){
    GenerateMandleOMP(DEFAULTWIDTH,DEFAULTHEIGHT);

    glClearColor (1.0, 1.0, 1.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);


	 GLfloat redmap[256],
        greenmap[256],
		bluemap[256];
	for ( int i = 0; i < 256; i++ ){
         redmap[i]=(i)/255.0;
         greenmap[i]=(i)/255.0;
         bluemap[i]=(i)/255.0;
    }

    glPixelMapfv(GL_PIXEL_MAP_I_TO_R, 256, redmap);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_G, 256, greenmap);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_B, 256, bluemap);
    threads = omp_get_max_threads();

}

void ompKEYS(bool PresedKeys[256]){
        if (PresedKeys['Q']==true){
        PresedKeys['Q']= false;
         exit(0);//quit
    }
    if (PresedKeys['I']==true){
         PresedKeys['I']= false;
         sIters+=100;
         cerr<<"iterations:"<<sIters<<endl;
    }
    if (PresedKeys['D']==true){
        PresedKeys['D']= false;
         sIters-=100;
         cerr<<"iterations:"<<sIters<<endl;
    }
    if (PresedKeys['T']==true){
        PresedKeys['T']= false;
        if(threads!=1)
            threads-=1;
         cerr<<"threads:"<<threads<<endl;
    }
    if (PresedKeys['G']==true){
        PresedKeys['G']= false;
         threads+=1;
         cerr<<"threads:"<<threads<<endl;
    }


}


void ompLOOP(){
    GenerateMandleOMP(DEFAULTHEIGHT,DEFAULTWIDTH);
  	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
  	gluOrtho2D(-1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
	glClear( GL_COLOR_BUFFER_BIT );
    glDrawPixels( DEFAULTHEIGHT, DEFAULTWIDTH, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, picture );
    displayFPS(false);

}



inline void GenerateMandleOMP(int xres, int yres){
    omp_set_num_threads(threads);
    const float ixres = 1.0f/(float)xres;
    const float iyres = 1.0f/(float)yres;
    int oneDindex;
    float a,b;
    int j = 0, i = 0;
    #pragma omp parallel for private(j,i, oneDindex) schedule(dynamic)
     for(j=0; j < yres; j++ )
        for(i=0; i < xres; i++ ){
            a = -2.05f + 3.00f*(float)i*ixres;
            b =  1.10f - 2.24f*(float)j*iyres;

            oneDindex = (j * DEFAULTHEIGHT) + i;
            picture[oneDindex] = IterateMandelbrot(a,b);
        }
}


inline GLuint IterateMandelbrotOMP(float a, float b ){
    float x, y, x2, y2;

    x = x2 = 0.0f;
    y = y2 = 0.0f;
    float m2;
    // escape time algorithm
    int i;
    float h = 0.0;
    volatile bool flag=false;

    for(i=0; i< 512; i++ )
    {
        if(flag)continue;
        y = 2.0f*x*y+b;
        x = x2-y2+a;

        x2 = x*x;
        y2 = y*y;

        m2 = x2+y2;
        if( m2>4.0f )
            flag=true;
        h+=1.0;
    }
    h=h+1.0-log2(.5*log(m2));
    // create color
    return (h/100.0)*256;
}



///OMP END------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///
///OMP END------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


///SSE START------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///SSE optimizations make use of the SSE simd chip on the cpu
///as we are working with floating point operations
///SSE will enhance performance
///SSE is s simd format, so in essence similar to the shader structure of the GPU
///SSE code courtesy of http://iquilezles.org/www/articles/sse/sse.htm
///it has been adapted to work for this framework
///SSE START------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void sseINIT(){
    GenMandleBrotSSE(DEFAULTWIDTH,DEFAULTHEIGHT);//generate first image

    glClearColor (1.0, 1.0, 1.0, 1.0);//clear screen to black
    glMatrixMode(GL_PROJECTION);//CAMERA calculations as lense
    glLoadIdentity();//load identity matrix to mem
    gluOrtho2D(-1, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);//change back to default


	 GLfloat redmap[256],//define colour conversions
        greenmap[256],
		bluemap[256];
	for ( int i = 0; i < 256; i++ ){//1-1
         redmap[i]=(i)/255.0;
         greenmap[i]=(i)/255.0;
         bluemap[i]=(i)/255.0;
    }

    glPixelMapfv(GL_PIXEL_MAP_I_TO_R, 256, redmap);//load conversions to opengl
    glPixelMapfv(GL_PIXEL_MAP_I_TO_G, 256, greenmap);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_B, 256, bluemap);


}

void sseKEYS(bool PresedKeys[256]){
        if (PresedKeys['Q']==true){
        PresedKeys['Q']= false;
         exit(0);//quit
    }
    if (PresedKeys['I']==true){
         PresedKeys['I']= false;
         sIters+=100;
         cerr<<"iterations:"<<sIters<<endl;
    }
    if (PresedKeys['D']==true){
        PresedKeys['D']= false;
         sIters-=100;
         cerr<<"iterations:"<<sIters<<endl;
    }

}

void sseLOOP(){
    GenMandleBrotSSE(DEFAULTHEIGHT,DEFAULTWIDTH);
  	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
  	gluOrtho2D(-1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
	glClear( GL_COLOR_BUFFER_BIT );
    glDrawPixels( DEFAULTHEIGHT, DEFAULTWIDTH, GL_COLOR_INDEX, GL_UNSIGNED_INT, buffer );
    displayFPS(false);

}

inline void GenMandleBrotSSE(int xres, int yres ){
    __m128i  *buffer4 = (__m128i *)buffer;
    const __m128 ixres = _mm_set1_ps( 1.0f/(float)xres );
    const __m128 iyres = _mm_set1_ps( 1.0f/(float)yres );

    for( int j=0; j < yres; j++ )
        for( int i=0; i < xres; i+=4 ){
        __m128  a, b;
        a = _mm_set_ps( i+3, i+2, i+1, i+0 );
        a = _mm_mul_ps( a, ixres );
        a = _mm_mul_ps( a, _mm_set1_ps( 3.00f) );
        a = _mm_add_ps( a, _mm_set1_ps(-2.25f) );

        b = _mm_set1_ps( (float)j );
        b = _mm_mul_ps( b, iyres );
        b = _mm_mul_ps( b, _mm_set1_ps(-2.24f) );
        b = _mm_add_ps( b, _mm_set1_ps( 1.12f) );
        _mm_store_si128( buffer4++, IterateMandelbrotSSE( a, b ) );
    }
}

inline __m128i IterateMandelbrotSSE( __m128 a, __m128 b ){
    __m128  x, y, x2, y2, m2;
    __m128  co, ite;

    unsigned int i;

    const __m128 one = _mm_set1_ps(1.0f);
    const __m128 th  = _mm_set1_ps(4.0f);

    x   = _mm_setzero_ps();
    y   = _mm_setzero_ps();
    x2  = _mm_setzero_ps();
    y2  = _mm_setzero_ps();
    co  = _mm_setzero_ps();
    ite = _mm_setzero_ps();

    //escape time algo
    for( i=0; i < 512; i++ )
        {
        y  = _mm_mul_ps( x, y );
        y  = _mm_add_ps( _mm_add_ps(y,y),   b );
        x  = _mm_add_ps( _mm_sub_ps(x2,y2), a );

        x2 = _mm_mul_ps( x, x );
        y2 = _mm_mul_ps( y, y );

        m2 = _mm_add_ps(x2,y2);
        co = _mm_or_ps( co, _mm_cmpgt_ps( m2, th ) );


        ite = _mm_add_ps( ite, _mm_andnot_ps( co, one ) );
        if( _mm_movemask_ps( co )==0x0f )
           break;
        }

    // create color
    const __m128i bb = _mm_cvtps_epi32( ite );
    const __m128i gg = _mm_slli_si128( bb, 1 );
    const __m128i rr = _mm_slli_si128( bb, 2 );
    const __m128i color = _mm_or_si128( _mm_or_si128(rr,gg),bb );

    return( color );
}

///SSE END------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///
///SSE END------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------








///SEQUENTIAL BASE START-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///The sequential portion has been made with minimal optimizations in mind
///to provide a baseline to work with
///SEQUENTIAL BASE START-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//sequential no PARALLEL shit
void sequentialINIT(){
    GenerateMandle(DEFAULTWIDTH,DEFAULTHEIGHT);

    glClearColor (1.0, 1.0, 1.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);


	 GLfloat redmap[256],
        greenmap[256],
		bluemap[256];
	for ( int i = 0; i < 256; i++ ){
         redmap[i]=(i)/255.0;
         greenmap[i]=(i)/255.0;
         bluemap[i]=(i)/255.0;
    }

    glPixelMapfv(GL_PIXEL_MAP_I_TO_R, 256, redmap);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_G, 256, greenmap);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_B, 256, bluemap);

}

void sequentialKEYS(bool PresedKeys[256]){
    if (PresedKeys['Q']==true){
        PresedKeys['Q']= false;
         exit(0);//quit
    }
    if (PresedKeys['I']==true){
         PresedKeys['I']= false;
         sIters+=100;
         cerr<<"iterations:"<<sIters<<endl;
    }
    if (PresedKeys['D']==true){
        PresedKeys['D']= false;
         sIters-=100;
         cerr<<"iterations:"<<sIters<<endl;
    }
}

void sequentialResizeer(int xRezer, int yRezer){

    //xRESO = xRezer;
    //yRESO = yRezer;

}

void sequentialLOOP(){
    GenerateMandle(DEFAULTHEIGHT,DEFAULTWIDTH);
  	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
  	gluOrtho2D(-1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
	glClear( GL_COLOR_BUFFER_BIT );
    glDrawPixels( DEFAULTHEIGHT, DEFAULTWIDTH, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, picture );
    displayFPS(false);

}

void GenerateMandle(int xres, int yres){
    const float ixres = 1.0f/(float)xres;
    const float iyres = 1.0f/(float)yres;

     for(int j=0; j < yres; j++ )
        for( int i=0; i < xres; i++ ){
            const float a = -2.05f + 3.00f*(float)i*ixres;
            const float b =  1.10f - 2.24f*(float)j*iyres;

            int oneDindex = (j * DEFAULTHEIGHT) + i;
            picture[oneDindex] = IterateMandelbrot(a,b);
        }
}


GLuint IterateMandelbrot(float a, float b ){
    float x, y, x2, y2;
   //simple local cache opt
    x = x2 = 0.0f;
    y = y2 = 0.0f;
    float m2;
    // escape time algorithm
    int i;
    float h = 0.0;
    for(i=0; i< 512; i++ )
    {
        y = 2.0f*x*y+b;
        x = x2-y2+a;

        x2 = x*x;
        y2 = y*y;

        m2 = x2+y2;
        if( m2>4.0f )
            break;
        h+=1.0;
    }
    h=h+1.0-log2(.5*log(m2));
    // create color


    //simple local cache opt


    return (h/100.0)*256;
}
///SEQUENTIAL BASE END -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///
///SEQUENTIAL BASE END -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------



///SHADER CODE START---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
///Here lies the shader portion of the code.
///attempts to keep it as clode to sequential portion have been made
///SHADER CODE START---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//shader code
static const char *vertexShader = \
"void main(void)"
"{"
    "gl_Position=gl_Vertex;"
    "gl_TexCoord[0]=gl_Vertex;"
"}";


static const char *fragmentShader = \

"uniform vec4 fpar[4];"
"uniform int iter;"

"void main(void)"
"{"
	"vec2 c=vec2(-0.75,0)+gl_TexCoord[0].xy*vec2(1.333,1.00);"
	"vec2 z=vec2(0.0);"
	"float h=0.0;"
	"float m;"
	"for(int i=0;i<512;i++)"
	"{"
		"z=vec2(z.x*z.x-z.y*z.y,2.0*z.x*z.y)+c;"
		"m=dot(z,z);"
		"if(m>4.0) break;"
		"h+=1.0;"
	"}"
	"h=h+1.0-log2(.5*log(m));"
	"gl_FragColor=vec4(h/100.0);"
"}";

//shader global vars
static float x;
static float y;
static float fparams[4*4];
static int p,s,v;
//shader initialization
void shaderINIT(){//assume window has been created.

    // create shader
    p = ((PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram"))();
    s = ((PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader"))(GL_FRAGMENT_SHADER);
    v = ((PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader"))(GL_VERTEX_SHADER);



    ((PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource"))(s, 1, &fragmentShader, 0);
    ((PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource"))(v , 1, &vertexShader, 0);


    ((PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader"))(s);
    ((PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader"))(v);

    //GLint length, result;

    //GLchar log[256];
    //(PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog"))(s, length, &result, log);

    /* print an error message and the info log */
    //fprintf(stderr, "shaderCompileFromFile(): Unable to compile: %s\n", log);

    ((PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader"))(p,s);
    ((PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader"))(p,v);
    ((PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram"))(p);
    ((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(p);

}

void shadermkeys(bool PresedKeys[256]){
    if (PresedKeys['Q']==true){
        PresedKeys['Q']= false;
         exit(0);//quit
    }

    if (PresedKeys['I']==true){
        PresedKeys['I']= false;
         sIters+=1000;//quit
         cerr<<"iterations:"<<sIters<<endl;
    }
    if (PresedKeys['D']==true){
        PresedKeys['D']= false;
         sIters-=1000;//quit
         cerr<<"iterations:"<<sIters<<endl;
    }


}

void shaderLOOPmain(){
   // glClearColor(0.0, 0.0, 0.0, 0.0);
   // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glRects( -1, -1, 1, 1 );


    const float t  = 0.001f;

    displayFPS(false);
    //--- render -----------------------------------------
    ((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(p);
    ((PFNGLUNIFORM4FVPROC)wglGetProcAddress("glUniform4fv"))(((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(p,"fpar"),  4, fparams );
    ((PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i"))(((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(p,"iter"), sIters);
}

///SHADER CODE END--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------



//helper functions
void renderBitmapString(float x, float y, void *font,char *string){
  char *c;

  glRasterPos2f(x, y);

  for (c=string; *c != '\0'; c++) {
    glutBitmapCharacter(font, *c);
  }
}
void setOrthographicProjection() {

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
       gluOrtho2D(0, DEFAULTHEIGHT, 0, DEFAULTWIDTH);
    glScalef(1, -1, 1);
    glTranslatef(0, -DEFAULTHEIGHT, 0);
    glMatrixMode(GL_MODELVIEW);
}

void resetPerspectiveProjection() {

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}


void displayFPS(bool drawem){

    time2=glutGet(GLUT_ELAPSED_TIME);
    frame2++;
    if (time2 - timebase2 > 1000) {
        sprintf(strz,"fps:%4.2f",frame2*1000.0/(time2-timebase2));
        timebase2 = time2;
        frame2 = 0;
        cout<<strz<<endl;
    }
    //else
     //
    if(drawem){
        glColor3f(1.0f,1.0f,1.0f);
            setOrthographicProjection();
            glPushMatrix();
                glLoadIdentity();
                renderBitmapString(0,10,(void *)font,strz);
            glPopMatrix();
           resetPerspectiveProjection();
    }
}


