//------------------------YouTube-3DSage----------------------------------------
//Full video: https://youtu.be/8j0gakEHJuI
//WADS to move player, E open door after picking up the key
//q=clear current texture to zero
//spacebar=return to 2D edit view
//load in the test level to try it

#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>

#include "Textures/All_Textures.ppm"
#include "Textures/sky.ppm"
#include "Textures/title.ppm"
#include "Textures/won.ppm"
#include "Textures/lost.ppm"
#include "Textures/sprites.ppm"
#include "Textures/All_Buttons.ppm"

float degToRad(float a) { return a*M_PI/180.0;}
float FixAng(float a){ if(a>359){ a-=360;} if(a<0){ a+=360;} return a;}
float distance(ax,ay,bx,by,ang){ return cos(degToRad(ang))*(bx-ax)-sin(degToRad(ang))*(by-ay);}
float px,py,pdx,pdy,pa;
float frame1,frame2,fps;
int gameState=-1, timer=0; //game state. init, start screen, game loop, win/lose
float fade=0;              //the 3 screens can fade up from black

int currentMap=0;      //0=walls, 1=floor, 2=ceiling
int currentTexture=0;  //texture id number, 0=empty    
int numSprite=2;       //total number of sprites
int buttonState=0;     //which UI button is pressed  
int dragItem=0;        //state if we are dragging an item
int currrentLevel=1;   //level that we can save in a file

typedef struct
{
 int w,a,d,s;                     //button state on off
}ButtonKeys; ButtonKeys Keys;

//-----------------------------MAP----------------------------------------------
#include<stdio.h>    //for printf
int mapX=16;         //map width
int mapY=10;         //map height
#define mapS 64      //map cube size

                     //Edit these 3 arrays with values 0-4 to create your own level! 
int mapW[17*13];     //walls
int mapF[17*13];     //floors
int mapC[17*13];     //ceiling


typedef struct       //All veriables per sprite
{
 int type;           //static, key, enemy
 int state;          //on off
 int map;            //texture to show
 float x,y,z;        //position
 int r,g,b;          //color
}sprite; sprite sp[32];
int depth[120];      //hold wall line depth to compare for sprite depth


void addTextures(int x,int y)
{
 //convert mouse to larger pixel screen
 x=x/48;
 y=y/48;
 //can't add outside of map area
 if(x>=mapX || y>=mapY){ return;}
 //add on click or dragging  
 int arrayPosition=x+y*mapX;
 if(currentMap==0){ mapW[arrayPosition]=currentTexture;}
 if(currentMap==1){ mapF[arrayPosition]=currentTexture;}
 if(currentMap==2){ mapC[arrayPosition]=currentTexture;}	
}


void save() 
{int x,y;
 char fileName[16]; 
 snprintf(fileName,sizeof(fileName),"level_%d.h",currrentLevel); 

 FILE *fp = fopen(fileName,"w");
 if(fp == NULL){ return;}

 //map width and height
 fprintf(fp,"%i,%i,",mapX,mapY);

 //player info
 fprintf(fp,"%f,%f,%f,", px, py, pa);

 //number of sprites
 fprintf(fp,"%i, ", numSprite);

 //sprites
 for(x=0;x<numSprite;x++)
 { 
  fprintf(fp,"%i,%i,%i, %f,%f,%f, %i,%i,%i, ",
  sp[x].type,sp[x].state,sp[x].map, 
  sp[x].x,sp[x].y,sp[x].z, 
  sp[x].r,sp[x].g,sp[x].b);
 }

 //walls, floor, ceiling
 for(x=0;x<mapX*mapY;x++)
 { 
  fprintf(fp,"%i,%i,%i,", mapW[x],mapF[x],mapC[x]);
 }

 //close
 fclose(fp);
 //print if saved
 printf("saved level: %i\n",currrentLevel);
}


void load() 
{int x,y;
 char fileName[16];
 snprintf(fileName,sizeof(fileName),"level_%i.h",currrentLevel);
 FILE *fp = fopen(fileName,"r");
 //file doesn't exist
 if(fp == NULL){ printf("no file to load\n"); return;}

 char c;
 //map width and height
 fscanf(fp,"%i",&mapX); c=getc(fp); 
 fscanf(fp,"%i",&mapY); c=getc(fp); 

 //player variables 
 fscanf(fp,"%f",&px); c=getc(fp);   
 fscanf(fp,"%f",&py); c=getc(fp);     
 fscanf(fp,"%f",&pa); c=getc(fp); 
 pdx= cos(degToRad(pa)); 
 pdy=-sin(degToRad(pa));

 //number of sprites
 fscanf(fp,"%i",&numSprite); c=getc(fp); 

 //sprites
 for(x=0;x<numSprite;x++)
 { 
  fscanf(fp,"%i",&sp[x].type);  c=getc(fp);
  fscanf(fp,"%i",&sp[x].state); c=getc(fp);
  fscanf(fp,"%i",&sp[x].map);   c=getc(fp);
  fscanf(fp,"%f",&sp[x].x);     c=getc(fp);
  fscanf(fp,"%f",&sp[x].y);     c=getc(fp);
  fscanf(fp,"%f",&sp[x].z);     c=getc(fp);
  fscanf(fp,"%i",&sp[x].r);     c=getc(fp);
  fscanf(fp,"%i",&sp[x].g);     c=getc(fp);
  fscanf(fp,"%i",&sp[x].b);     c=getc(fp);
 }

 //walls, floor, ceiling
 for(x=0;x<mapX*mapY;x++)
 { 
  fscanf(fp,"%i",&mapW[x]); c=getc(fp);
  fscanf(fp,"%i",&mapF[x]); c=getc(fp);
  fscanf(fp,"%i",&mapC[x]); c=getc(fp);
 }

 //close
 fclose(fp); 
 //print if loaded
 printf("loaded level: %i\n",currrentLevel);
} 

void mouse(int button, int state, int x, int y) 
{ 
 //Add to array
 if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) 
 {
  //mouse click add, texture
  addTextures(x,y);

  //click over buttons
  if(x/8>=104)
  {
   if(y/8<16+8*0){ currentTexture+=1; if(currentTexture>20){ currentTexture=0;}} //next texture
   if(y/8>=16+8*0 && y/8<24+8*0){ if(buttonState>0){ buttonState=0;} else{ buttonState=1;} printf("save\n");}
   if(y/8>=16+8*1 && y/8<24+8*1){ if(buttonState>0){ buttonState=0;} else{ buttonState=2;} printf("Load\n");}
   if(y/8>=16+8*2 && y/8<24+8*2){ if(buttonState>0){ buttonState=0;} else{ buttonState=3;} printf("player rotate\n");}
   if(y/8>=16+8*3 && y/8<24+8*3){ if(buttonState>0){ buttonState=0;} else{ buttonState=4;} printf("add enemy\n");}
   if(y/8>=16+8*4 && y/8<24+8*4){ if(buttonState>0){ buttonState=0;} else{ buttonState=5;} printf("view walls\n");}
   if(y/8>=16+8*5 && y/8<24+8*5){ if(buttonState>0){ buttonState=0;} else{ buttonState=6;} printf("view floors\n");}
   if(y/8>=16+8*6 && y/8<24+8*6){ if(buttonState>0){ buttonState=0;} else{ buttonState=7;} printf("view ceiling\n");}
   if(y/8>=16+8*7 && y/8<24+8*7){ if(buttonState>0){ buttonState=0;} else{ buttonState=8;} printf("play game\n");}	

   //button functions
   if(buttonState==1){ save();}
   if(buttonState==2){ load();}
   if(buttonState==3){ pa+=45; if(pa>359){ pa-=360;} pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));} //rotate player
   if(buttonState==4) //add enemy
   { 
    int s=numSprite;
    sp[s].type=3; sp[s].state=1; sp[s].map=2; sp[s].x=numSprite*32; sp[s].y=1*32; sp[s].z=20; //enemy
    sp[s].r=255; sp[s].g=255; sp[s].b=0;  
    numSprite+=1;
   }  
   if(buttonState==5){ currentMap=0;} //view walls
   if(buttonState==6){ currentMap=1;} //view floors
   if(buttonState==7){ currentMap=2;} //view ceiling
   if(buttonState==8){ gameState=0;} //start the game

  }
 }
 if(button == GLUT_LEFT_BUTTON && state == GLUT_UP)
 { 
  //toggle buttons
  if(x/8>=104){ buttonState=0;}
 }

 //right button down
 if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) 
 {
  //previous texture
  if(x/8>=104 && y/8<16+8*0){ currentTexture-=1; if(currentTexture<0){ currentTexture=20;}} 
 
  //convert mouse to grid
  int mx=x*64/6/8;
  int my=y*64/6/8;

  //select player
  if(mx+10>px && mx-10<px && my+10>py && my-10<py){ dragItem=1;}

  //check all sprites if selected
  for(x=0;x<numSprite;x++)
  {
   if(mx+10>sp[x].x && mx-10<sp[x].x && my+10>sp[x].y && my-10<sp[x].y){ dragItem=x+10; break;}
  }
 }

 //right button up
 if(button == GLUT_RIGHT_BUTTON && state == GLUT_UP){ dragItem=0;}

}


void MouseMove(int x,int y)
{
 //player 
 if(dragItem==1)
 {
  px=x*64/6/8; 
  py=y*64/6/8;
 }
 //move sprites
 if(dragItem>1)
 {
  sp[dragItem-10].x=x*64/6/8; 
  sp[dragItem-10].y=y*64/6/8;
 }

 //drag to draw walls
 if(dragItem==0)
 {
  addTextures(x,y);
 }

 glutPostRedisplay();
}


void drawCurrentTexture(int v,int posX, int posY)
{int x,y;
 for(y=0;y<16;y++)
 {
  for(x=0;x<16;x++)
  {
   int pixel=(x*2)+(y*2)*32+v*32*32; 
   int red   =All_Textures[pixel*3+0]; 
   int green =All_Textures[pixel*3+1];
   int blue  =All_Textures[pixel*3+2];
   glColor3ub(red,green,blue); 
   glBegin(GL_POINTS); 
   glVertex2i(x*8+posX+4,y*8+posY+4); 
   glEnd();
  }	
 }	
}

void drawSquareFromArray(int v,int posX, int posY, int *array,int black)
{int x,y;
 //skip if not a wall 
 if(array[v]==0){ return;}

 for(y=0;y<6;y++)
 {
  for(x=0;x<6;x++)
  {
   int pixel=((x*2)*32+(y*2))+array[v]*32*32; 
   int red   =All_Textures[pixel*3+0]; 
   int green =All_Textures[pixel*3+1];
   int blue  =All_Textures[pixel*3+2];
   if(black==1 && array[v]*32*32>0){ red=0; green=0; blue=0;}
   glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(x*8+(8*6*posX)+4,y*8+(8*6*posY)+4); glEnd();
  }	
 }	
}

void mapEditor()
{int x,y,i,j;
 //clear background 
 for(y=0;y<80;y++)
 {
  for(x=0;x<120;x++)
  {
   glColor3ub(200,220,240);
   glBegin(GL_POINTS); 
   glVertex2i(x*8+4,y*8+4); 
   glEnd();
  }	
 }

 //grid
 for(y=0;y<mapY;y++)
 {
  for(x=0;x<mapX;x++)
  {
   if(currentMap==0){ drawSquareFromArray(x+y*mapX,x,y,mapW,0);}
   if(currentMap==1){ drawSquareFromArray(x+y*mapX,x,y,mapF,0);}
   if(currentMap==2){ drawSquareFromArray(x+y*mapX,x,y,mapC,0);}
   if(currentMap> 0){ drawSquareFromArray(x+y*mapX,x,y,mapW,1);} //draw black walls
  }
 }

 //the current texture
 drawCurrentTexture(currentTexture,832,0);                     

 //buttons
 for(y=0;y<64;y++)
 {
  for(x=0;x<16;x++)
  {
   int pixel=x+y*16; 
   int red   =All_Buttons[pixel*3+0]; 
   int green =All_Buttons[pixel*3+1];
   int blue  =All_Buttons[pixel*3+2];
   //highlight press button down
        if(buttonState==1 && y>= 0 && y< 8){ red=0;}
   else if(buttonState==2 && y>= 8 && y<16){ red=0;}
   else if(buttonState==3 && y>=16 && y<24){ red=0;}
   else if(buttonState==4 && y>=24 && y<32){ red=0;}
   else if(buttonState==5 && y>=32 && y<40){ red=0;}
   else if(buttonState==6 && y>=40 && y<48){ red=0;}
   else if(buttonState==7 && y>=48 && y<56){ red=0;}
   else if(buttonState==8 && y>=56 && y<64){ red=0;}

   glColor3ub(red,green,blue); 
   glBegin(GL_POINTS); glVertex2i(x*8+(8*104)+4,y*8+(8*16)+4); glEnd();
  }	
 }

 //player
 glColor3ub(0,255,0); glBegin(GL_POINTS); glVertex2i(px/64*6*8,py/64*6*8); glEnd();
 glColor3ub(0,155,0); glBegin(GL_POINTS); glVertex2i((px+pdx*16)/64*6*8,(py+pdy*16)/64*6*8); glEnd();
 //draw all sprites, key,lights,enemy
 for(x=0;x<numSprite;x++)
 {
  glColor3ub(sp[x].r,sp[x].g,sp[x].b); 
  glBegin(GL_POINTS); 
  glVertex2i(sp[x].x/64*6*8,sp[x].y/64*6*8); 
  glEnd();
 }
}

void drawSprite()
{
 int x,y,s;
 if(px<sp[0].x+30 && px>sp[0].x-30 && py<sp[0].y+30 && py>sp[0].y-30){ sp[0].state=0;} //pick up key 	

 //enemy attack
 for(x=0;x<numSprite;x++)
 {
  //skip if not an enemy 
  if(sp[x].type!=3){ continue;}
  if(px<sp[x].x+30 && px>sp[x].x-30 && py<sp[x].y+30 && py>sp[x].y-30){ gameState=4; return;} //enemy kills

  //add variation to player position and speed
  float ran = (float)rand()/(float)(RAND_MAX/0.1); 

  int spx=(int)sp[x].x>>6,          spy=(int)sp[x].y>>6;          //normal grid position
  int spx_add=((int)sp[x].x+15)>>6, spy_add=((int)sp[x].y+15)>>6; //normal grid position plus     offset
  int spx_sub=((int)sp[x].x-15)>>6, spy_sub=((int)sp[x].y-15)>>6; //normal grid position subtract offset
  if(sp[x].x>px && mapW[spy*mapX+spx_sub]==0){ sp[x].x-=ran*fps;}
  if(sp[x].x<px && mapW[spy*mapX+spx_add]==0){ sp[x].x+=ran*fps;}
  if(sp[x].y>py && mapW[spy_sub*mapX+spx]==0){ sp[x].y-=ran*fps;}
  if(sp[x].y<py && mapW[spy_add*mapX+spx]==0){ sp[x].y+=ran*fps;}
 }

 for(s=0;s<numSprite;s++)
 {
  float sx=sp[s].x-px; //temp float variables
  float sy=sp[s].y-py;
  float sz=sp[s].z;

  float CS=cos(degToRad(pa)), SN=sin(degToRad(pa)); //rotate around origin
  float a=sy*CS+sx*SN; 
  float b=sx*CS-sy*SN; 
  sx=a; sy=b;

  sx=(sx*108.0/sy)+(120/2); //convert to screen x,y
  sy=(sz*108.0/sy)+( 80/2);

  int scale=32*80/b;   //scale sprite based on distance
  if(scale<0){ scale=0;} if(scale>120){ scale=120;}  

  //texture
  float t_x=0, t_y=31, t_x_step=31.5/(float)scale, t_y_step=32.0/(float)scale;

  for(x=sx-scale/2;x<sx+scale/2;x++)
  {
   t_y=31;
   for(y=0;y<scale;y++)
   {
    if(sp[s].state==1 && x>0 && x<120 && b<depth[x])
    {
     int pixel=((int)t_y*32+(int)t_x)*3+(sp[s].map*32*32*3);
     int red   =sprites[pixel+0];
     int green =sprites[pixel+1];
     int blue  =sprites[pixel+2];
     if(red!=255, green!=0, blue!=255) //dont draw if purple
     {
      glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(x*8,sy*8-y*8); glEnd(); //draw point 
     }
     t_y-=t_y_step; if(t_y<0){ t_y=0;}
    }
   }
   t_x+=t_x_step;
  }
 }
}

//---------------------------Draw Rays and Walls--------------------------------
void drawRays2D()
{	
 int r,mx,my,mp,dof,side; float vx,vy,rx,ry,ra,xo,yo,disV,disH; 
 
 ra=FixAng(pa+30);                                                              //ray set back 30 degrees
 
 for(r=0;r<120;r++)
 {
  int vmt=0,hmt=0;                                                              //vertical and horizontal map texture number 
  //---Vertical--- 
  dof=0; side=0; disV=100000;
  float Tan=tan(degToRad(ra));
       if(cos(degToRad(ra))> 0.001){ rx=(((int)px>>6)<<6)+64;      ry=(px-rx)*Tan+py; xo= 64; yo=-xo*Tan;}//looking left
  else if(cos(degToRad(ra))<-0.001){ rx=(((int)px>>6)<<6) -0.0001; ry=(px-rx)*Tan+py; xo=-64; yo=-xo*Tan;}//looking right
  else { rx=px; ry=py; dof=20;}                                                  //looking up or down. no hit  

  while(dof<20) 
  { 
   mx=(int)(rx)>>6; my=(int)(ry)>>6; mp=my*mapX+mx;                     
   if(mp>0 && mp<mapX*mapY && mapW[mp]>0){ vmt=mapW[mp]; dof=20; disV=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}//hit         
   else{ rx+=xo; ry+=yo; dof+=1;}                                               //check next horizontal
  } 
  vx=rx; vy=ry;

  //---Horizontal---
  dof=0; disH=100000;
  Tan=1.0/Tan; 
       if(sin(degToRad(ra))> 0.001){ ry=(((int)py>>6)<<6) -0.0001; rx=(py-ry)*Tan+px; yo=-64; xo=-yo*Tan;}//looking up 
  else if(sin(degToRad(ra))<-0.001){ ry=(((int)py>>6)<<6)+64;      rx=(py-ry)*Tan+px; yo= 64; xo=-yo*Tan;}//looking down
  else{ rx=px; ry=py; dof=20;}                                                   //looking straight left or right
 
  while(dof<20) 
  { 
   mx=(int)(rx)>>6; my=(int)(ry)>>6; mp=my*mapX+mx;                          
   if(mp>0 && mp<mapX*mapY && mapW[mp]>0){ hmt=mapW[mp]; dof=20; disH=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}//hit         
   else{ rx+=xo; ry+=yo; dof+=1;}                                               //check next horizontal
  } 
  
  float shade=1;
  glColor3f(0,0.8,0);
  if(disV<disH){ hmt=vmt; shade=0.5; rx=vx; ry=vy; disH=disV; glColor3f(0,0.6,0);}//horizontal hit first
    
  int ca=FixAng(pa-ra); disH=disH*cos(degToRad(ca));                            //fix fisheye 
  int lineH = (mapS*640)/(disH); 
  float ty_step=32.0/(float)lineH; 
  float ty_off=0; 
  if(lineH>640){ ty_off=(lineH-640)/2.0; lineH=640;}                            //line height and limit
  int lineOff = 320 - (lineH>>1);                                               //line offset

  depth[r]=disH; //save this line's depth
  //---draw walls---
  int y;
  float ty=ty_off*ty_step;//+hmt*32;
  float tx;
  if(shade==1){ tx=(int)(rx/2.0)%32; if(ra>180){ tx=31-tx;}}  
  else        { tx=(int)(ry/2.0)%32; if(ra>90 && ra<270){ tx=31-tx;}}
  for(y=0;y<lineH;y++)
  {
   int pixel=((int)ty*32+(int)tx)*3+(hmt*32*32*3);
   int red   =All_Textures[pixel+0]*shade;
   int green =All_Textures[pixel+1]*shade;
   int blue  =All_Textures[pixel+2]*shade;
   glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(r*8+4,y+lineOff); glEnd();
   ty+=ty_step;
  }
 
  //---draw floors---
 for(y=lineOff+lineH;y<640;y++)
 {
  float dy=y-(640/2.0), deg=degToRad(ra), raFix=cos(degToRad(FixAng(pa-ra)));
  tx=px/2 + cos(deg)*158*2*32/dy/raFix;
  ty=py/2 - sin(deg)*158*2*32/dy/raFix;
  int mp=mapF[(int)(ty/32.0)*mapX+(int)(tx/32.0)]*32*32;
  int pixel=(((int)(ty)&31)*32 + ((int)(tx)&31))*3+mp*3;
  int red   =All_Textures[pixel+0]*0.7;
  int green =All_Textures[pixel+1]*0.7;
  int blue  =All_Textures[pixel+2]*0.7;
  glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(r*8+4,y); glEnd();

 //---draw ceiling---
  mp=mapC[(int)(ty/32.0)*mapX+(int)(tx/32.0)]*32*32;
  pixel=(((int)(ty)&31)*32 + ((int)(tx)&31))*3+mp*3;
  red   =All_Textures[pixel+0];
  green =All_Textures[pixel+1];
  blue  =All_Textures[pixel+2];
  if(mp>0){ glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(r*8+4,640-y); glEnd();}
 }
 
 ra=FixAng(ra-0.5);                                                               //go to next ray, 60 total
 }
}//-----------------------------------------------------------------------------


void drawSky()     //draw sky and rotate based on player rotation
{int x,y;
 for(y=0;y<40;y++)
 {
  for(x=0;x<120;x++)
  {
   int xo=(int)pa*2-x; if(xo<0){ xo+=120;} xo=xo % 120; //return 0-120 based on player angle
   int pixel=(y*120+xo)*3;
   int red   =sky[pixel+0];
   int green =sky[pixel+1];
   int blue  =sky[pixel+2];
   glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(x*8+4,y*8+4); glEnd();
  }	
 }
}

void screen(int v) //draw any full screen image. 120x80 pixels
{
 int x,y;
 int *T;
 if(v==1){ T=title;}
 if(v==2){ T=won;}
 if(v==3){ T=lost;}
 for(y=0;y<80;y++)
 {
  for(x=0;x<120;x++)
  {
   int pixel=(y*120+x)*3;
   int red   =T[pixel+0]*fade;
   int green =T[pixel+1]*fade;
   int blue  =T[pixel+2]*fade;
   glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(x*8+4,y*8+4); glEnd();
  }	
 }	
 if(fade<1){ fade+=0.001*fps;} 
 if(fade>1){ fade=1;}
}

void init()//init all variables when game starts
{
 glClearColor(0.3,0.3,0.3,0);
 glPointSize(8);

 px=150; py=400; pa=90;
 pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));                                 //init player

 sp[0].type=1; sp[0].state=1; sp[0].map=0; sp[0].x=1.5*64; sp[0].y=5*64; sp[0].z=20; //key
 sp[1].type=3; sp[1].state=1; sp[1].map=2; sp[1].x=2.5*64; sp[1].y=2*64; sp[1].z=20; //enemy
 //add color just for 2D edit view
 sp[0].r=255; sp[0].g=  0; sp[0].b=0; //key
 sp[1].r=255; sp[1].g=255; sp[1].b=0; //enemy

 //create wall perimeter
 int x,y;
 for(y=0;y<mapY;y++)
 {
  for(x=0;x<mapX;x++)
  { 
   if(x==0 || x==mapX-1 || y==0 || y==mapY-1){ mapW[y*mapX+x]=2;} 
   mapF[y*mapX+x]=1;
   mapC[y*mapX+x]=0;
  }
 }
}

void display()
{  
 //frames per second
 frame2=glutGet(GLUT_ELAPSED_TIME); fps=(frame2-frame1); frame1=glutGet(GLUT_ELAPSED_TIME); 
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

 //2D edit 
 if(gameState<0){ mapEditor();}

 //3D game
 else
 {
  if(gameState==0){ fade=0; timer=0; gameState=1;} //init game
  if(gameState==1){ screen(1); timer+=1*fps; if(timer>2000){ fade=0; timer=0; gameState=2;}} //start screen
  if(gameState==2) //The main game loop
  {
   //buttons
   if(Keys.a==1){ pa+=0.2*fps; pa=FixAng(pa); pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));} 	
   if(Keys.d==1){ pa-=0.2*fps; pa=FixAng(pa); pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));} 

   int xo=0; if(pdx<0){ xo=-20;} else{ xo=20;}                                    //x offset to check map
   int yo=0; if(pdy<0){ yo=-20;} else{ yo=20;}                                    //y offset to check map
   int ipx=px/64.0, ipx_add_xo=(px+xo)/64.0, ipx_sub_xo=(px-xo)/64.0;             //x position and offset
   int ipy=py/64.0, ipy_add_yo=(py+yo)/64.0, ipy_sub_yo=(py-yo)/64.0;             //y position and offset
   if(Keys.w==1)                                                                  //move forward
   {  
    if(mapW[ipy*mapX        + ipx_add_xo]==0){ px+=pdx*0.2*fps;}
    if(mapW[ipy_add_yo*mapX + ipx       ]==0){ py+=pdy*0.2*fps;}
   }
   if(Keys.s==1)                                                                  //move backward
   { 
    if(mapW[ipy*mapX        + ipx_sub_xo]==0){ px-=pdx*0.2*fps;}
    if(mapW[ipy_sub_yo*mapX + ipx       ]==0){ py-=pdy*0.2*fps;}
   } 
   drawSky();
   drawRays2D();
   drawSprite();
   if(mapF[((int)px>>6)+((int)py>>6)*mapX]==20){ fade=0; timer=0; gameState=3;} //Entered block 1, Win game!!
  }
  //won screen
  if(gameState==3)
  { 
   screen(2); timer+=1*fps; 
   if(timer>2000)
   { 
    fade=0; timer=0; gameState=0;
    currrentLevel+=1; if(currrentLevel>2){ currrentLevel=0;}
    load();
   }
  } 
  if(gameState==4){ screen(3); timer+=1*fps; if(timer>2000){ fade=0; timer=0; gameState=0;}} //lost screen
 }

 glutPostRedisplay();
 glutSwapBuffers();  
}

void ButtonDown(unsigned char key,int x,int y)                                  //keyboard button pressed down
{
 if(key=='a'){ Keys.a=1;} 	
 if(key=='d'){ Keys.d=1;} 
 if(key=='w'){ Keys.w=1;}
 if(key=='s'){ Keys.s=1;}
 if(key=='e' && sp[0].state==0)             //open doors
 { 
  int xo=0; if(pdx<0){ xo=-25;} else{ xo=25;}
  int yo=0; if(pdy<0){ yo=-25;} else{ yo=25;} 
  int ipx=px/64.0, ipx_add_xo=(px+xo)/64.0;
  int ipy=py/64.0, ipy_add_yo=(py+yo)/64.0;
  if(mapW[ipy_add_yo*mapX+ipx_add_xo]==4){ mapW[ipy_add_yo*mapX+ipx_add_xo]=0;}
 }
 //quickly set current texture to empty
 if(key=='q'){ currentTexture=0;} 
 //spacebar return to 2D edit view
 if(key==32){ gameState=-1;} 

 glutPostRedisplay();
}

void ButtonUp(unsigned char key,int x,int y)                                    //keyboard button pressed up
{
 if(key=='a'){ Keys.a=0;} 	
 if(key=='d'){ Keys.d=0;} 
 if(key=='w'){ Keys.w=0;}
 if(key=='s'){ Keys.s=0;}
 glutPostRedisplay();
}

void resize(int w,int h)                                                        //screen window rescaled, snap back
{
 glutReshapeWindow(960,640);
}

int main(int argc, char* argv[])
{ 
 glutInit(&argc, argv);
 glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
 glutInitWindowSize(960,640);
 glutInitWindowPosition( glutGet(GLUT_SCREEN_WIDTH)/2-960/2 ,glutGet(GLUT_SCREEN_HEIGHT)/2-640/2 );
 glutCreateWindow("YouTube-3DSage");
 gluOrtho2D(0,960,640,0);
 init();
 glutMouseFunc(mouse);
 glutMotionFunc(MouseMove);
 glutDisplayFunc(display);
 glutReshapeFunc(resize);
 glutKeyboardFunc(ButtonDown);
 glutKeyboardUpFunc(ButtonUp);
 glutMainLoop();
}

