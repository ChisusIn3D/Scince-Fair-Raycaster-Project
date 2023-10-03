//WADS para mover al jugador. E para interactuar con puertas y botones

#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>

#include "Texturas/All_T.ppm"
#include "Texturas/Sky.ppm"
#include "Texturas/title.ppm"
#include "Texturas/won.ppm"
#include "Texturas/lost.ppm"
#include "Texturas/Sprites.ppm"

float degToRad(float a) { return a*M_PI/180.0;}												    //operacion para convertir grados en radianes		
float FixAng(float a){ if(a>359){ a-=360;} if(a<0){ a+=360;} return a;}						    //resetear el angulo si supera los 0 o los 360 grados
float distance(ax,ay,bx,by,ang){ return cos(degToRad(ang))*(bx-ax)-sin(degToRad(ang))*(by-ay);} //distacia (usando pitagoras)
float px,py,pdx,pdy,pa; 																	    //posicion del jugador en x y, sus delta y el angulo del jugador
float frame1,frame2, fps; 																	    //variables de frames para usarlas despues
int gameState=0, timer=0, dpr, der;																//estado del juego y contador
int playerH=150, playerD=50, enemyH=100, enemyD=50; 											//salud del jugador y el daño que hace el enemigo
float fade=0;

typedef struct
{
 int w,a,d,s;                     
} ButtonKeys; ButtonKeys Keys;

//-----------------------------MAPA---------------------------------------------
#define mapX  8      //ancho del mapa
#define mapY  8      //largo del mapa
#define mapS 64      //tamaño cuandrantes del mapa
int mapW[]=           //Peredes del mapa. Editar para cambiar el nivel, ¡matener paredes externas!
{
 5,5,5,5,1,1,1,1,
 6,0,0,5,0,0,0,2,
 5,0,0,4,0,2,0,2,
 5,5,7,5,0,0,0,2,
 1,0,0,0,0,0,0,2,
 1,0,0,0,0,1,0,2,
 1,0,0,0,0,0,0,2,
 2,2,2,2,2,2,2,2,	
};

int mapF[]=           //Piso del mapa. Editar para cambiar el diseño
{
 2,2,2,2,2,2,2,2,
 2,2,2,2,2,2,2,2,
 2,2,2,2,2,2,2,2,
 2,2,2,2,2,2,2,2,
 2,2,2,2,2,2,2,2,
 2,2,2,2,2,2,2,2,
 2,2,2,2,2,2,2,2,
 2,2,2,2,2,2,2,2,	
};

int mapC[]=           //Techo del mapa. Editar para cambiar el diseño
{
 7,7,7,7,7,0,0,0,
 7,7,7,7,7,0,0,0,
 7,7,7,7,7,0,0,0,
 7,7,7,7,7,0,0,0,
 7,7,7,7,7,0,0,0,
 0,0,7,0,0,0,0,0,
 0,0,7,0,0,0,0,0,
 0,0,0,0,0,0,0,0,	
};

typedef struct
{
  int type;  //estatica, llave, enemigo
  int state; //on off
  int map;	 //textura a mostrar
  float x,y,z; //posicion
}sprite; sprite sp[4];
int depth[120];

void drawSprite()
{
	int x,y,s;
	//llave
	if(px<sp[0].x+30 && px>sp[0].x-30 && py<sp[0].y+30 && py>sp[0].y-30){ sp[0].state=0;} 	  				    //recojer llave
	//enemigo
	if(px<sp[3].x+250 && px>sp[3].x-250 && py<sp[3].y+250 && py>sp[3].y-250){der+=1*fps; if(der>3000){ der=0;}} //esperar antes de atacar
	if(px<sp[3].x+30 && px>sp[3].x-30 && py<sp[3].y+30 && py>sp[3].y-30 && der>=1500){ playerH-=enemyD; der=0;} //dañar jugador
	if(playerH<=0){ playerH=0; gameState=4;}													  				//matar jugador
	if(enemyH<=0)																								//enemigo muerto
	{
		enemyH=0; sp[3].state=0; enemyD=0; //desaparecer y dejar de hacer daño
		sp[3].x-=0*fps; sp[3].x+=0*fps; sp[3].y-=0*fps; sp[3].y+=0*fps; //dejar de moverse
	}
	
	//movimiento enemigo
	if(px<sp[3].x+25 && px>sp[3].x-25 && py<sp[3].y+25 && py>sp[3].y-25)
	{
		sp[3].x-=0*fps; sp[3].x+=0*fps; sp[3].y-=0*fps; sp[3].y+=0*fps;	
	}
	else
	{
		int spx=(int)sp[3].x>>6         , spy=(int)(int)sp[3].y>>6;		//posicion cuadros normal
		int spx_add=((int)sp[3].x+15)>>6, spy_add=((int)sp[3].y+15)>>6; //posicion cuadros mas offset
		int spx_sub=((int)sp[3].x-15)>>6, spy_sub=((int)sp[3].y-15)>>6; //posicion cuadros menos offeset
		if(sp[3].x>px && mapW[spy*8+spx_sub]==0){ sp[3].x-=0.02*fps;}
		if(sp[3].x<px && mapW[spy*8+spx_add]==0){ sp[3].x+=0.02*fps;} 
		if(sp[3].y>py && mapW[spy_sub*8+spx]==0){ sp[3].y-=0.02*fps;} 
		if(sp[3].y<py && mapW[spy_add*8+spx]==0){ sp[3].y+=0.02*fps;} 
	}

	
	for(s=0;s<4;s++)
	{
		float sx=sp[s].x-px; //variables float temporales
		float sy=sp[s].y-py;
		float sz=sp[s].z;
	
		float CS=cos(degToRad(pa)), SN=sin(degToRad(pa)); //rotar alrededor de origen
		float a=sy*CS+sx*SN;
		float b=sx*CS-sy*SN;
		sx=a; sy=b;
	
		sx=(sx*108.0/sy)+(120/2); //convetir posicion x,y a la pantalla
		sy=(sz*108.0/sy)+( 80/2);
	
		int scale=32*80/b;
		if (scale<0){ scale=0;} if (scale>120){ scale=120;}
		
		//texturas
		float t_x=0, t_y=31, t_x_step=31.5/(float)scale, t_y_step=32.0/(float)scale;
		
		for(x=sx-scale/2; x<sx+scale/2; x++)
		{
			t_y=31;
			for(y=0; y<scale; y++)
			{
				if (sp[s].state==1 && x>0 && x<120 && b<depth[x])
				{
					int pixel = ((int)t_y*32+(int)t_x)*3+(sp[s].map*32*32*3);
			  		int red   = Sprites[pixel+0];
			  		int green = Sprites[pixel+1];
			  		int blue  = Sprites[pixel+2];
			  		if(red!=255, green!=0, blue!=255) //no dibujar el magenta
					{
					 glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(x*8,sy*8-y*8); glEnd();
			  		}
					t_y-=t_y_step; if(t_y<0){t_y=0;}
				}
			}
			t_x+=t_x_step;
		}
	}
}

//---------------------------Draw Rayos y Paredes-------------------------------

void drawRays2D()
{
 int r,mx,my,mp,dof,side; float vx,vy,rx,ry,ra,xo,yo,disV,disH; 

 ra=FixAng(pa+30);                                                              //rayo en 30 grados

 for(r=0;r<120;r++)
 {
 	int vmt=0,hmt=0; //numero de textura del mapa, vertical y horizontal
  //---Vertical--- 
  dof=0; side=0; disV=100000;
  float Tan=tan(degToRad(ra));
       if(cos(degToRad(ra))> 0.001){ rx=(((int)px>>6)<<6)+64;      ry=(px-rx)*Tan+py; xo= 64; yo=-xo*Tan;}//mirando izquierda
  else if(cos(degToRad(ra))<-0.001){ rx=(((int)px>>6)<<6) -0.0001; ry=(px-rx)*Tan+py; xo=-64; yo=-xo*Tan;}//mirando derecha
  else { rx=px; ry=py; dof=8;}                                                  //mirando arrba o abajo, no hay pared  

  while(dof<8) 
  { 
   mx=(int)(rx)>>6; my=(int)(ry)>>6; mp=my*mapX+mx;                     
   if(mp>0 && mp<mapX*mapY && mapW[mp]>0){ vmt=mapW[mp]-1; dof=8; disV=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}//hay pared         
   else{ rx+=xo; ry+=yo; dof+=1;}                                               //checkear siguiente vertical
  } 
  vx=rx; vy=ry;

  //---Horizontal---
  dof=0; disH=100000;
  Tan=1.0/Tan; 
       if(sin(degToRad(ra))> 0.001){ ry=(((int)py>>6)<<6) -0.0001; rx=(py-ry)*Tan+px; yo=-64; xo=-yo*Tan;}//mirando arriba
  else if(sin(degToRad(ra))<-0.001){ ry=(((int)py>>6)<<6)+64;      rx=(py-ry)*Tan+px; yo= 64; xo=-yo*Tan;}//mirando abajo
  else{ rx=px; ry=py; dof=8;}                                                   //mirando izquierda o derecha

  while(dof<8) 
  { 
   mx=(int)(rx)>>6; my=(int)(ry)>>6; mp=my*mapX+mx;                          
   if(mp>0 && mp<mapX*mapY && mapW[mp]>0){ hmt=mapW[mp]-1; dof=8; disH=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}//encuentra pared         
   else{ rx+=xo; ry+=yo; dof+=1;}                                               //checkear siguiente horizontal
  } 

  float shade=1;
  glColor3f(0,0,0.5);
  if(disV<disH){ hmt=vmt; shade=0.5; rx=vx; ry=vy; disH=disV; glColor3f(0,0,0.8);}                  //horizontal encuentra pared primero

  int ca=FixAng(pa-ra); disH=disH*cos(degToRad(ca));                            //arreglar fisheye 
  int lineH = (mapS*640)/(disH);                                                //altura de linea y su limite
  float ty_step=32.0/(float)lineH;
  float ty_off=0;
  if(lineH>640){ ty_off=(lineH-640)/2.0; lineH=640;}
  int lineOff = 320 - (lineH>>1);                                               //offset linea
  
  depth[r]=disH; //guardar profundidad de linea
  //dibujado de paredes
  int y;
  float ty=ty_off*ty_step;
  float tx;
  if (shade==1){ tx=(int)(rx/2.0)%32; if (ra>180){ tx=31-tx;}}
  else         {tx=(int)(ry/2.0)%32; if (ra>90 && ra<270){ tx=31-tx;}}
  
  for(y=0;y<lineH;y++)
  {
  	  int pixel = ((int)ty*32+(int)tx)*3+(hmt*32*32*3);
 	  int red   = All_T[pixel+0]*shade;
 	  int green = All_T[pixel+1]*shade;
 	  int blue  = All_T[pixel+2]*shade;
 	  glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(r*8,y+lineOff); glEnd();
  	ty+=ty_step;
  }
  
    //Dibujado de piso.
 for(y=lineOff+lineH;y<640;y++)
 {
  float dy=y-(640/2.0), deg=degToRad(ra), raFix=cos(degToRad(FixAng(pa-ra)));
  tx=px/2 + cos(deg)*158*2*32/dy/raFix;
  ty=py/2 - sin(deg)*158*2*32/dy/raFix;
  int mp=mapF[(int)(ty/32.0)*mapX+(int)(tx/32.0)]*32*32;
  int pixel = (((int)(ty)&31)*32 + ((int)(tx)&31))*3+mp*3;
  int red   = All_T[pixel+0]*0.7;
  int green = All_T[pixel+1]*0.7;
  int blue  = All_T[pixel+2]*0.7;
  glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(r*8,y); glEnd();
 	  
   //Dibujado de techo
  mp=mapC[(int)(ty/32.0)*mapX+(int)(tx/32.0)]*32*32;
  pixel = (((int)(ty)&31)*32 + ((int)(tx)&31))*3+mp*3;
  red   = All_T[pixel+0]*0.8;
  green = All_T[pixel+1]*0.8;
  blue  = All_T[pixel+2]*0.8;
  if (mp>0){ glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(r*8,640-y); glEnd();}
 }

 ra=FixAng(ra-0.5);
 }
}//-----------------------------------------------------------------------------

void drawSky()
{int x,y;
  for (y=0;y<40;y++)
  { 
   for (x=0;x<120;x++)
   {  	  
    int xo=(int)pa*2-x; if (xo<0){ xo+=120;} xo=xo % 120;
    int pixel = (y*120+xo)*3;
 	int red   = Sky[pixel+0];
 	int green = Sky[pixel+1];
 	int blue  = Sky[pixel+2];
 	glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(x*8,y*8); glEnd();
   }
  }
}

void screen(int v)
{
  int x,y;
  int *T;
  if (v==1){T=title;}
  if (v==2){T=won;}
  if (v==3){T=lost;}
  for (y=0;y<80;y++)
  { 
   for (x=0;x<120;x++)
   {  	  
    int pixel = (y*120+x)*3;
 	int red   = T[pixel+0]*fade;
 	int green = T[pixel+1]*fade;
 	int blue  = T[pixel+2]*fade;
 	glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(x*8,y*8); glEnd();
   }
  }
  if (fade<1){ fade+=0.001*fps;}
  if (fade>1){ fade=1;}
}

void init()
{
 glClearColor(0.3,0.3,0.3,0);
 px=160; py=435; pa=90; playerH=150;
 pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa)); 
 mapW[19]=4; mapW[26]=7; mapW[8]=6;//cerrar puertas y reabilitar boton

 sp[0].type=1; sp[0].state=1; sp[0].map=0; sp[0].x=1.5*64; sp[0].y=6*64; sp[0].z=20;//llave
 sp[1].type=2; sp[1].state=1; sp[1].map=1; sp[1].x=1.5*64; sp[1].y=5*64; sp[1].z= 0;//lampara 1
 sp[2].type=2; sp[2].state=1; sp[2].map=1; sp[2].x=3.5*64; sp[2].y=5*64; sp[2].z= 0;//lampara 2
 sp[3].type=3; sp[3].state=1; sp[3].map=2; sp[3].x=4.5*64; sp[3].y=6*64; sp[3].z=15;//enemigo(SLIME)
}

void display()
{	//frames por segundo

frame2=glutGet(GLUT_ELAPSED_TIME); fps=(frame2-frame1); frame1=glutGet(GLUT_ELAPSED_TIME);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

if (gameState==0){ init(); fade=0; timer=0; dpr=0; gameState=1;}//init juego
if (gameState==1){ screen(1); timer+=1*fps; if(timer>3000){ fade=1; timer=0; gameState=2;}}//pantalla de inicio
if (gameState==2)
{
	//butones
if(Keys.a==1){ pa+=0.225*fps; pa=FixAng(pa); pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));} 	
if(Keys.d==1){ pa-=0.225*fps; pa=FixAng(pa); pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));} 

int xo=0; if(pdx<0){ xo=-15;} else{ xo=15;}                                     //colisiones
int yo=0; if(pdy<0){ yo=-15;} else{ yo=15;}
int ipx=px/64.0, ipx_add_xo=(px+xo)/64.0, ipx_sub_xo=(px-xo)/64.0;
int ipy=py/64.0, ipy_add_yo=(py+yo)/64.0, ipy_sub_yo=(py-yo)/64.0;

if(Keys.w==1)
{ 
 if(mapW[ipy*mapX        + ipx_add_xo]==0){ px+=pdx*0.15*fps;}
 if(mapW[ipy_add_yo*mapX + ipx       ]==0){ py+=pdy*0.15*fps;}
}

if(Keys.s==1)
{
  if(mapW[ipy*mapX        + ipx_sub_xo]==0){ px-=pdx*0.15*fps;}
  if(mapW[ipy_sub_yo*mapX + ipx       ]==0){ py-=pdy*0.15*fps;}
}

 drawSky();
 drawRays2D();
 drawSprite();
}
if (gameState==3){ screen(2); timer+=1*fps; if(timer>3000){ fade=1; timer=0; gameState=0;}}//victoria
if (gameState==4){ screen(3); timer+=1*fps; if(timer>3000){ fade=1; timer=0; gameState=0;}}//derrota

 glutPostRedisplay();
 glutSwapBuffers(); 
}

void ButtonDown(unsigned char key,int x,int y)                                  
{
 if(key=='a' || key=='A'){ Keys.a=1;} 	
 if(key=='d' || key=='D'){ Keys.d=1;} 
 if(key=='w' || key=='W'){ Keys.w=1;}
 if(key=='s' || key=='S'){ Keys.s=1;}
 if(key=='e' || key=='E')
 {
  int xo=0; if(pdx<0){xo=-25;} else {xo=25;}
  int yo=0; if(pdy<0){yo=-25;} else {yo=25;}
  int ipx=px/64.0, ipx_add_xo=(px+xo)/64.0;
  int ipy=py/64.0, ipy_add_yo=(py+yo)/64.0;
  if(mapW[ipy_add_yo*mapX+ipx_add_xo]==4){ mapW[ipy_add_yo*mapX+ipx_add_xo]=0;}
  if(mapW[ipy_add_yo*mapX+ipx_add_xo]==7 && sp[0].state==0){ mapW[ipy_add_yo*mapX+ipx_add_xo]=0;}
  if(mapW[ipy_add_yo*mapX+ipx_add_xo]==6){ mapW[ipy_add_yo*mapX+ipx_add_xo]=1; fade=0; timer=0; gameState=3;}
 }
 if(key=='q' || key=='Q')
 {
 	if(px<sp[3].x+100 && px>sp[3].x-100 && py<sp[3].y+100 && py>sp[3].y-100){ enemyH-=playerD;}
 }
 glutPostRedisplay();
}

void ButtonUp(unsigned char key,int x,int y)                                    
{
 if(key=='a' || key=='A'){ Keys.a=0;} 	
 if(key=='d' || key=='S'){ Keys.d=0;} 
 if(key=='w' || key=='W'){ Keys.w=0;}
 if(key=='s' || key=='S'){ Keys.s=0;}
 glutPostRedisplay();
}

void resize(int w, int h)
{
	glutReshapeWindow(960,640);
}


int main(int argc, char* argv[])
{ 
 glutInit(&argc, argv);
 glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
 glutInitWindowSize(960,640);
 glutInitWindowPosition( glutGet(GLUT_SCREEN_WIDTH)/2-960/2 , glutGet(GLUT_SCREEN_HEIGHT)/2-640/2 );
 glutCreateWindow("Raycaster-Feria2023");
 gluOrtho2D(0,960,640,0);
 init();
 glutDisplayFunc(display);
 glutReshapeFunc(resize);
 glutKeyboardFunc(ButtonDown);
 glutKeyboardUpFunc(ButtonUp);
 glutMainLoop();
}
