#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>
#define PI 3.1415926535
#define P2 PI/2
#define P3 3*PI/2
#define GR 0.0174533 //un grado en radianes

float px,py,pdx,pdy,pa; //posicion del jugador

void drawPlayer()//jugador 2D representado en color rojo
{
	glColor3f(1,0,0);
	glPointSize(8);
	glBegin(GL_POINTS);
	glVertex2i(px,py);
	glEnd();
	
	glLineWidth(3); //draw del jugador
	glBegin(GL_LINES);
	glVertex2i(px,py);
	glVertex2i(px+pdx*5,py+pdy*5);
	glEnd();
}

int mapX=8,mapY=8,mapS=64; //ancho, alto y tamaños de los cuadrantes del mapa
int map[]=
{
	1,1,1,1,1,1,1,1,
    1,0,1,0,0,0,0,1,
    1,0,1,0,0,0,0,1,
    1,0,1,0,0,0,0,1,
    1,0,0,0,0,0,0,1,
    1,0,0,0,0,1,0,1,
    1,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,


};

void drawMap2D()//draw del mapa
{
	int x,y,xo,yo;
	for (y=0;y<mapY;y++)
	{
		for (x=0;x<mapX;x++)
		{
			if (map[x*mapY+y]==1){ glColor3f(1,1,1);} else{ glColor3f(0,0,0);}
			xo=x*mapS;yo=y*mapS;
			glBegin(GL_QUADS);
			glVertex2i(yo     +1,xo     +1);
			glVertex2i(yo     +1,xo+mapS-1);
			glVertex2i(yo+mapS-1,xo+mapS-1);
			glVertex2i(yo+mapS-1,xo     +1);
			glEnd();
		}
	}
}


float dist(float ax,float ay, float bx,float by, float ang)//teorema de pitagoras
{
	return ( sqrt( (bx-ax)*(bx-ax) + (by-ay)*(by-ay) ));
}
 
void drawRays3D()
{
	 glColor3f(0.6,0.3,1); glBegin(GL_QUADS); glVertex2i(526,  0); glVertex2i(1006,  0); glVertex2i(1006,160); glVertex2i(526,160); glEnd();	
 	 glColor3f(0.3,1,0); glBegin(GL_QUADS); glVertex2i(526,160); glVertex2i(1006,160); glVertex2i(1006,320); glVertex2i(526,320); glEnd();
	
	int r,mx,my,mp,dof; float rx,ry,ra,xo,yo,disT; 
	ra=pa-GR*30; if (ra<0){ ra+=2*PI;} if (ra>2*PI){ ra-=2*PI;}
	for (r=0;r<60;r++)
	{   //---check lineas horizontales
		dof=0;
		float aTan=-1/tan(ra);
		float disH=1000000, hx=px, hy=py;
		if (ra>PI){ ry=(((int)py>>6)<<6)-0.0001; rx=(py-ry)*aTan+px; yo=-64; xo=-yo*aTan;}//mirando hacia abajo
		if (ra<PI){ ry=(((int)py>>6)<<6)+64;     rx=(py-ry)*aTan+px; yo= 64; xo=-yo*aTan;}//mirando hacia arriba
		if (ra==0 || ra==PI){ rx=px; ry=py; dof=8;}//mirando a la izquierda o a la derecha
		while (dof<8)
		{
			mx=((int)rx)>>6; my=((int)ry)>>6; mp=my*mapX+mx;
			if (mp>0 && mp<mapX*mapY && map[mp]==1){ hx=rx; hy=ry; disH=dist(px,py,hx,hy,ra); dof=8;}//encuentra pared
			else { rx+=xo; ry+=yo; dof+=1;}//siguiente linea
		}
		
		//---check lineas verticales
		dof=0;
		float nTan=-tan(ra);
		float disV=1000000, vx=px, vy=py;
		if (ra>P2 && ra<P3){ rx=(((int)px>>6)<<6)-0.0001; ry=(px-rx)*nTan+py; xo=-64; yo=-xo*nTan;}//mirando hacia la izquierda
		if (ra<P2 || ra>P3){ rx=(((int)px>>6)<<6)+64;     ry=(px-rx)*nTan+py; xo= 64; yo=-xo*nTan;}//mirando hacia derecha
		if (ra==0 || ra==PI){ rx=px; ry=py; dof=8;}//mirando a la arriba o a la abajo
		while (dof<8)
		{
			mx=(int)(rx)>>6; my=(int)(ry)>>6; mp=my*mapX+mx;
			if (mp>0 && mp<mapX*mapY && map[mp]==1){ vx=rx; vy=ry; disV=dist(px,py,vx,vy,ra); dof=8;}//encuentra pared
			else { rx+=xo; ry+=yo; dof+=1;}//siguiente linea
		}
		if(disV<disH){ rx=vx; ry=vy; disT=disV; glColor3f(0,0,0.9);}            //encuentra linea vertical
		if(disH<disV){ rx=hx; ry=hy; disT=disH; glColor3f(0,0,0.5);}            //encuentra linea horizontal
		glLineWidth(3); glBegin(GL_LINES); glVertex2i(px,py); glVertex2i(rx,ry); glEnd();
		
		//---draw paredes 3D
		float ca=pa-ra; if (ca<0){ ca+=2*PI;} if (ca>2*PI){ ca-=2*PI;} disT=disT*cos(ca); //arreglar fisheye
		float lineH=(mapS*320)/disT; if (lineH>320){lineH=320;}                           //altura de las lineas
		float lineO=160-lineH/2;                                                           //offset de las lineas
		glLineWidth(8);glBegin(GL_LINES);glVertex2i(r*8+530,lineO);glVertex2i(r*8+530,lineH+lineO);glEnd();
		ra+=GR; if (ra<0){ ra+=2*PI;} if (ra>2*PI){ ra-=2*PI;}
	}
}

void display()//funciones del juego
{
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 drawMap2D();
 drawRays3D();
 drawPlayer();
 glutSwapBuffers();
}

void resize(int w, int h)
{
	glutReshapeWindow(1024,512);
}

void buttons(unsigned char key,int x,int y)//movimiento
{
	if (key=='a'){ pa-=0.1; if (pa<   0){ pa+=2*PI;} pdx=cos(pa)*5; pdy=sin(pa)*5;}
	if (key=='d'){ pa+=0.1; if (pa>2*PI){ pa-=2*PI;} pdx=cos(pa)*5; pdy=sin(pa)*5;}
	if (key=='s'){ px-=pdx; py-=pdy;}
	if (key=='w'){ px+=pdx; py+=pdy;}
	glutPostRedisplay();
}

void init()
{
	glClearColor(0.3,0.3,0.3,0);
	gluOrtho2D(0,1024,512,0);
	px=300; py=300; pdx=cos(pa)*5; pdy=sin(pa)*5;
}

int main(int argc, char** argv)
{ 
 glutInit(&argc, argv);
 glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
 glutInitWindowSize(1024, 512);
 glutInitWindowPosition(125,125);
 glutCreateWindow("Raycasters - ScienceFair2023");
 init();
 glutDisplayFunc(display);
 glutReshapeFunc(resize);
 glutKeyboardFunc(buttons);
 glutMainLoop();
 return 0;
}
