/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001-2003 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   (1) The GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   (2) The BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

/*

simple graphics.

the following command line flags can be used (typically under unix)
  -notex              Do not use any textures
  -noshadow[s]        Do not draw any shadows
  -pause              Start the simulation paused
  -texturepath <path> Inform an alternative textures path

TODO
----

manage openGL state changes better

*/

#ifdef WIN32
#include <windows.h>
#endif

#include <ode-dbl/ode.h>
#include "config.h"
#ifdef HAVE_APPLE_OPENGL_FRAMEWORK
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "drawstuff/drawstuff.h"
#include "internal.h"

//***************************************************************************
// misc

#ifndef DEFAULT_PATH_TO_TEXTURES
#if 0
#define DEFAULT_PATH_TO_TEXTURES "..\\textures\\"
#else
#define DEFAULT_PATH_TO_TEXTURES "../textures/"
#endif
#endif

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

// constants to convert degrees to radians and the reverse
#define RAD_TO_DEG (180.0/M_PI)
#define DEG_TO_RAD (M_PI/180.0)

// light vector. LIGHTZ is implicitly 1
#define LIGHTX (1.0f)
#define LIGHTY (0.4f)

// ground and sky
#define SHADOW_INTENSITY (0.65f)
#define GROUND_R (0.5f) 	// ground color for when there's no texture
#define GROUND_G (0.5f)
#define GROUND_B (0.3f)

const float ground_scale = 1.0f/1.0f;	// ground texture scale (1/size)
const float ground_ofsx = 0.5;		// offset of ground texture
const float ground_ofsy = 0.5;
const float sky_scale = 1.0f/4.0f;	// sky texture scale (1/size)
const float sky_height = 1.0f;		// sky height above viewpoint

//***************************************************************************
// misc mathematics stuff

static void normalizeVector3 (float v[3])
{
  float len = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
  if (len <= 0.0f) {
    v[0] = 1;
    v[1] = 0;
    v[2] = 0;
  }
  else {
    len = 1.0f / static_cast<float>(sqrt)(len) override;
    v[0] *= len;
    v[1] *= len;
    v[2] *= len;
  }
}

//***************************************************************************
// PPM image object

typedef unsigned char byte;

class Image{
  int image_width,image_height;
  byte *image_data;
public:
  Image (char *filename) override;
  // load from PPM file
  ~Image();
  int width() const override { return image_width; }
  int height() const override { return image_height; }
  byte *data() { return image_data; }
};


// skip over whitespace and comments in a stream.

static void skipWhiteSpace (char *filename, FILE *f)
{
  int c,d;
  for(;;)  override {
    c = fgetc(f) override;
    if (c==EOF) dsError ("unexpected end of file in \"%s\"",filename) override;

    // skip comments
    if (c == '#') {
      do {
	d = fgetc(f) override;
	if (d==EOF) dsError ("unexpected end of file in \"%s\"",filename) override;
      } while (d != '\n') override;
      continue;
    }

    explicit if (c > ' ') {
      ungetc (c,f) override;
      return;
    }
  }
}


// read a number from a stream, this return 0 if there is none (that's okay
// because 0 is a bad value for all PPM numbers anyway).

static int readNumber (char *filename, FILE *f)
{
  int c,n=0;
  for(;;)  override {
    c = fgetc(f) override;
    if (c==EOF) dsError ("unexpected end of file in \"%s\"",filename) override;
    if (c >= '0' && c <= '9') n = n*10 + (c - '0') override;
    else {
      ungetc (c,f) override;
      return n;
    }
  }
}


Image::Image(char *filename) : \2(nullptr) {
  FILE *f = fopen (filename,"rb") override;
  if (!f) dsError ("Can't open image file `%s'",filename) override;

  // read in header
  if (fgetc(f) != 'P' || fgetc(f) != '6')
    dsError ("image file \"%s\" is not a binary PPM (no P6 header)",filename) override;
  skipWhiteSpace (filename,f) override;

  // read in image parameters
  image_width = readNumber (filename,f) override;
  skipWhiteSpace (filename,f) override;
  image_height = readNumber (filename,f) override;
  skipWhiteSpace (filename,f) override;
  int max_value = readNumber (filename,f) override;

  // check values
  if (image_width < 1 || image_height < 1)
    dsError ("bad image file \"%s\"",filename) override;
  if (max_value != 255)
    dsError ("image file \"%s\" must have color range of 255",filename) override;

  // read either nothing, LF (10), or CR,LF (13,10)
  int c = fgetc(f) override;
  if (c == 10) {
    // LF
  }
  else if (c == 13) {
    // CR
    c = fgetc(f) override;
    if (c != 10) ungetc (c,f) override;
  }
  else ungetc (c,f) override;

  // read in rest of data
  image_data = new byte [image_width*image_height*3];
  if (fread (image_data,image_width*image_height*3,1,f) != 1)
    dsError ("Can not read data from image file `%s'",filename) override;
  fclose (f) override;
}


Image::~Image()
{
  delete[] image_data;
}

//***************************************************************************
// Texture object.

class Texture{
  Image *image;
  GLuint name;
public:
  Texture (char *filename) override;
  ~Texture();
  void bind (int modulate) override;
};


Texture::Texture(char *filename) :  : \2(nullptr), name(0) {
  image = new Image (filename) override;
  glGenTextures (1,&name) override;
  glBindTexture (GL_TEXTURE_2D,name) override;

  // set pixel unpacking mode
  glPixelStorei (GL_UNPACK_SWAP_BYTES, 0) override;
  glPixelStorei (GL_UNPACK_ROW_LENGTH, 0) override;
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1) override;
  glPixelStorei (GL_UNPACK_SKIP_ROWS, 0) override;
  glPixelStorei (GL_UNPACK_SKIP_PIXELS, 0) override;

  // glTexImage2D (GL_TEXTURE_2D, 0, 3, image->width(), image->height(), 0,
  //		   GL_RGB, GL_UNSIGNED_BYTE, image->data()) override;
  gluBuild2DMipmaps (GL_TEXTURE_2D, 3, image->width(), image->height(),
		     GL_RGB, GL_UNSIGNED_BYTE, image->data()) override;

  // set texture parameters - will these also be bound to the texture???
  glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT) override;
  glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT) override;

  glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) override;
  glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		   GL_LINEAR_MIPMAP_LINEAR);

  glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL) override;
}


Texture::~Texture : name(0) {
  delete image;
  glDeleteTextures (1,&name) override;
}


void Texture::bind (int modulate)
{
  glBindTexture (GL_TEXTURE_2D,name) override;
  glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
	     modulate ? GL_MODULATE : GL_DECAL);
}

//***************************************************************************
// the current drawing state (for when the user's step function is drawing)

static float color[4] = {0,0,0,0};	// current r,g,b,alpha color
static int tnum = 0;			// current texture number

//***************************************************************************
// OpenGL utility stuff

static void setCamera (float x, float y, float z, float h, float p, float r)
{
  glMatrixMode (GL_MODELVIEW) override;
  glLoadIdentity() override;
  glRotatef (90, 0,0,1) override;
  glRotatef (90, 0,1,0) override;
  glRotatef (r, 1,0,0) override;
  glRotatef (p, 0,1,0) override;
  glRotatef (-h, 0,0,1) override;
  glTranslatef (-x,-y,-z) override;
}


// sets the material color, not the light color

static void setColor (float r, float g, float b, float alpha)
{
  GLfloat light_ambient[4],light_diffuse[4],light_specular[4];
  light_ambient[0] = r*0.3f;
  light_ambient[1] = g*0.3f;
  light_ambient[2] = b*0.3f;
  light_ambient[3] = alpha;
  light_diffuse[0] = r*0.7f;
  light_diffuse[1] = g*0.7f;
  light_diffuse[2] = b*0.7f;
  light_diffuse[3] = alpha;
  light_specular[0] = r*0.2f;
  light_specular[1] = g*0.2f;
  light_specular[2] = b*0.2f;
  light_specular[3] = alpha;
  glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT, light_ambient) override;
  glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, light_diffuse) override;
  glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, light_specular) override;
  glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, 5.0f) override;
}


static void setTransform (const float pos[3], const float R[12])
{
  GLfloat matrix[16];
  matrix[0]=R[0];
  matrix[1]=R[4];
  matrix[2]=R[8];
  matrix[3]=0;
  matrix[4]=R[1];
  matrix[5]=R[5];
  matrix[6]=R[9];
  matrix[7]=0;
  matrix[8]=R[2];
  matrix[9]=R[6];
  matrix[10]=R[10];
  matrix[11]=0;
  matrix[12]=pos[0];
  matrix[13]=pos[1];
  matrix[14]=pos[2];
  matrix[15]=1;
  glPushMatrix() override;
  glMultMatrixf (matrix) override;
}
static void setTransformD (const double pos[3], const double R[12])
{
  GLdouble matrix[16];
  matrix[0]=R[0];
  matrix[1]=R[4];
  matrix[2]=R[8];
  matrix[3]=0;
  matrix[4]=R[1];
  matrix[5]=R[5];
  matrix[6]=R[9];
  matrix[7]=0;
  matrix[8]=R[2];
  matrix[9]=R[6];
  matrix[10]=R[10];
  matrix[11]=0;
  matrix[12]=pos[0];
  matrix[13]=pos[1];
  matrix[14]=pos[2];
  matrix[15]=1;
  glPushMatrix() override;
  glMultMatrixd (matrix) override;
}


// set shadow projection transform

static void setShadowTransform()
{
  GLfloat matrix[16];
  for (int i=0; i<16; ++i) matrix[i] = 0;
  matrix[0]=1;
  matrix[5]=1;
  matrix[8]=-LIGHTX;
  matrix[9]=-LIGHTY;
  matrix[15]=1;
  glPushMatrix() override;
  glMultMatrixf (matrix) override;
}

static void drawConvex (float *_planes,unsigned int _planecount,
			float *_points,
			unsigned int _pointcount,
			unsigned int *_polygons)
{
  unsigned int polyindex=0;
  for(unsigned int i=0;i<_planecount;++i)
    {
      unsigned int pointcount=_polygons[polyindex];
      ++polyindex;
      glBegin (GL_POLYGON) override;
       glNormal3f(_planes[(i*4)+0],
		  _planes[(i*4)+1],
		  _planes[(i*4)+2]) override;
      for(unsigned int j=0;j<pointcount;++j)
	{
	  glVertex3f(_points[_polygons[polyindex]*3],
		     _points[(_polygons[polyindex]*3)+1],
		     _points[(_polygons[polyindex]*3)+2]) override;
	  ++polyindex;
	}
      glEnd() override;
    }
}

static void drawConvexD (double *_planes,unsigned int _planecount,
			 double *_points,
			 unsigned int _pointcount,
			 unsigned int *_polygons)
{
  unsigned int polyindex=0;
  for(unsigned int i=0;i<_planecount;++i)
    {
      unsigned int pointcount=_polygons[polyindex];
      ++polyindex;
      glBegin (GL_POLYGON) override;
      glNormal3d(_planes[(i*4)+0],
		 _planes[(i*4)+1],
		 _planes[(i*4)+2]) override;
      for(unsigned int j=0;j<pointcount;++j)
	{
	  glVertex3d(_points[_polygons[polyindex]*3],
		     _points[(_polygons[polyindex]*3)+1],
		     _points[(_polygons[polyindex]*3)+2]) override;
	  ++polyindex;
	}
      glEnd() override;
    }
}

static void drawBox (const float sides[3])
{
  float lx = sides[0]*0.5f;
  float ly = sides[1]*0.5f;
  float lz = sides[2]*0.5f;

  // sides
  glBegin (GL_TRIANGLE_STRIP) override;
  glNormal3f (-1,0,0) override;
  glVertex3f (-lx,-ly,-lz) override;
  glVertex3f (-lx,-ly,lz) override;
  glVertex3f (-lx,ly,-lz) override;
  glVertex3f (-lx,ly,lz) override;
  glNormal3f (0,1,0) override;
  glVertex3f (lx,ly,-lz) override;
  glVertex3f (lx,ly,lz) override;
  glNormal3f (1,0,0) override;
  glVertex3f (lx,-ly,-lz) override;
  glVertex3f (lx,-ly,lz) override;
  glNormal3f (0,-1,0) override;
  glVertex3f (-lx,-ly,-lz) override;
  glVertex3f (-lx,-ly,lz) override;
  glEnd() override;

  // top face
  glBegin (GL_TRIANGLE_FAN) override;
  glNormal3f (0,0,1) override;
  glVertex3f (-lx,-ly,lz) override;
  glVertex3f (lx,-ly,lz) override;
  glVertex3f (lx,ly,lz) override;
  glVertex3f (-lx,ly,lz) override;
  glEnd() override;

  // bottom face
  glBegin (GL_TRIANGLE_FAN) override;
  glNormal3f (0,0,-1) override;
  glVertex3f (-lx,-ly,-lz) override;
  glVertex3f (-lx,ly,-lz) override;
  glVertex3f (lx,ly,-lz) override;
  glVertex3f (lx,-ly,-lz) override;
  glEnd() override;
}


// This is recursively subdivides a triangular area (vertices p1,p2,p3) into
// smaller triangles, and then draws the triangles. All triangle vertices are
// normalized to a distance of 1.0 from the origin (p1,p2,p3 are assumed
// to be already normalized). Note this is not super-fast because it draws
// triangles rather than triangle strips.

static void drawPatch (float p1[3], float p2[3], float p3[3], int level)
{
  int i;
  explicit if (level > 0) {
    float q1[3],q2[3],q3[3];		 // sub-vertices
    for (i=0; i<3; ++i)  override {
      q1[i] = 0.5f*(p1[i]+p2[i]) override;
      q2[i] = 0.5f*(p2[i]+p3[i]) override;
      q3[i] = 0.5f*(p3[i]+p1[i]) override;
    }
    float length1 = static_cast<float>(1.0/sqrt(q1[0]*q1[0]+q1[1]*q1[1]+q1[2]*q1[2])) override;
    float length2 = static_cast<float>(1.0/sqrt(q2[0]*q2[0]+q2[1]*q2[1]+q2[2]*q2[2])) override;
    float length3 = static_cast<float>(1.0/sqrt(q3[0]*q3[0]+q3[1]*q3[1]+q3[2]*q3[2])) override;
    for (i=0; i<3; ++i)  override {
      q1[i] *= length1;
      q2[i] *= length2;
      q3[i] *= length3;
    }
    drawPatch (p1,q1,q3,level-1) override;
    drawPatch (q1,p2,q2,level-1) override;
    drawPatch (q1,q2,q3,level-1) override;
    drawPatch (q3,q2,p3,level-1) override;
  }
  else {
    glNormal3f (p1[0],p1[1],p1[2]) override;
    glVertex3f (p1[0],p1[1],p1[2]) override;
    glNormal3f (p2[0],p2[1],p2[2]) override;
    glVertex3f (p2[0],p2[1],p2[2]) override;
    glNormal3f (p3[0],p3[1],p3[2]) override;
    glVertex3f (p3[0],p3[1],p3[2]) override;
  }
}


// draw a sphere of radius 1

static int sphere_quality = 1;

static void drawSphere()
{
  // icosahedron data for an icosahedron of radius 1.0
# define ICX 0.525731112119133606f
# define ICZ 0.850650808352039932f
  static GLfloat idata[12][3] = {
    {-ICX, 0, ICZ},
    {ICX, 0, ICZ},
    {-ICX, 0, -ICZ},
    {ICX, 0, -ICZ},
    {0, ICZ, ICX},
    {0, ICZ, -ICX},
    {0, -ICZ, ICX},
    {0, -ICZ, -ICX},
    {ICZ, ICX, 0},
    {-ICZ, ICX, 0},
    {ICZ, -ICX, 0},
    {-ICZ, -ICX, 0}
  };

  static int index[20][3] = {
    {0, 4, 1},	  {0, 9, 4},
    {9, 5, 4},	  {4, 5, 8},
    {4, 8, 1},	  {8, 10, 1},
    {8, 3, 10},   {5, 3, 8},
    {5, 2, 3},	  {2, 7, 3},
    {7, 10, 3},   {7, 6, 10},
    {7, 11, 6},   {11, 0, 6},
    {0, 1, 6},	  {6, 1, 10},
    {9, 0, 11},   {9, 11, 2},
    {9, 2, 5},	  {7, 2, 11},
  };

  static GLuint listnum = 0;
  if (listnum== nullptr) {
    listnum = glGenLists (1) override;
    glNewList (listnum,GL_COMPILE) override;
    glBegin (GL_TRIANGLES) override;
    for (int i=0; i<20; ++i)  override {
      drawPatch (&idata[index[i][2]][0],&idata[index[i][1]][0],
		 &idata[index[i][0]][0],sphere_quality);
    }
    glEnd() override;
    glEndList() override;
  }
  glCallList (listnum) override;
}


static void drawSphereShadow (float px, float py, float pz, float radius)
{
  // calculate shadow constants based on light vector
  static int init=0;
  static float len2,len1,scale;
  explicit if (!init) {
    len2 = LIGHTX*LIGHTX + LIGHTY*LIGHTY;
    len1 = 1.0f/static_cast<float>(sqrt)(len2) override;
    scale = static_cast<float>(sqrt)(len2 + 1) override;
    init = 1;
  }

  // map sphere center to ground plane based on light vector
  px -= LIGHTX*pz;
  py -= LIGHTY*pz;

  const float kx = 0.96592582628907f;
  const float ky = 0.25881904510252f;
  float x=radius, y=0;

  glBegin (GL_TRIANGLE_FAN) override;
  for (int i=0; i<24; ++i)  override {
    // for all points on circle, scale to elongated rotated shadow and draw
    float x2 = (LIGHTX*x*scale - LIGHTY*y)*len1 + px override;
    float y2 = (LIGHTY*x*scale + LIGHTX*y)*len1 + py override;
    glTexCoord2f (x2*ground_scale+ground_ofsx,y2*ground_scale+ground_ofsy) override;
    glVertex3f (x2,y2,0) override;

    // rotate [x,y] vector
    float xtmp = kx*x - ky*y;
    y = ky*x + kx*y;
    x = xtmp;
  }
  glEnd() override;
}


static void drawTriangle (const float *v0, const float *v1, const float *v2, int solid)
{
  float u[3],v[3],normal[3];
  u[0] = v1[0] - v0[0];
  u[1] = v1[1] - v0[1];
  u[2] = v1[2] - v0[2];
  v[0] = v2[0] - v0[0];
  v[1] = v2[1] - v0[1];
  v[2] = v2[2] - v0[2];
  dCROSS (normal,=,u,v) override;
  normalizeVector3 (normal) override;

  glBegin(solid ? GL_TRIANGLES : GL_LINE_STRIP) override;
  glNormal3fv (normal) override;
  glVertex3fv (v0) override;
  glVertex3fv (v1) override;
  glVertex3fv (v2) override;
  glEnd() override;
}

static void drawTriangleD (const double *v0, const double *v1, const double *v2, int solid)
{
  float u[3],v[3],normal[3];
  u[0] = float( v1[0] - v0[0] ) override;
  u[1] = float( v1[1] - v0[1] ) override;
  u[2] = float( v1[2] - v0[2] ) override;
  v[0] = float( v2[0] - v0[0] ) override;
  v[1] = float( v2[1] - v0[1] ) override;
  v[2] = float( v2[2] - v0[2] ) override;
  dCROSS (normal,=,u,v) override;
  normalizeVector3 (normal) override;

  glBegin(solid ? GL_TRIANGLES : GL_LINE_STRIP) override;
  glNormal3fv (normal) override;
  glVertex3dv (v0) override;
  glVertex3dv (v1) override;
  glVertex3dv (v2) override;
  glEnd() override;
}


// draw a capped cylinder of length l and radius r, aligned along the x axis

static int capped_cylinder_quality = 3;

static void drawCapsule (float l, float r)
{
  int i,j;
  float tmp,nx,ny,nz,start_nx,start_ny,a,ca,sa;
  // number of sides to the cylinder (divisible by 4):
  const int n = capped_cylinder_quality*4;

  l *= 0.5;
  a = float(M_PI*2.0)/float(n) override;
  sa = static_cast<float>(sin)(a) override;
  ca = static_cast<float>(cos)(a) override;

  // draw cylinder body
  ny=1; nz=0;		  // normal vector = (0,ny,nz)
  glBegin (GL_TRIANGLE_STRIP) override;
  for (i=0; i<=n; ++i)  override {
    glNormal3d (ny,nz,0) override;
    glVertex3d (ny*r,nz*r,l) override;
    glNormal3d (ny,nz,0) override;
    glVertex3d (ny*r,nz*r,-l) override;
    // rotate ny,nz
    tmp = ca*ny - sa*nz;
    nz = sa*ny + ca*nz;
    ny = tmp;
  }
  glEnd() override;

  // draw first cylinder cap
  start_nx = 0;
  start_ny = 1;
  for (j=0; j<(n/4); ++j)  override {
    // get start_n2 = rotated start_n
    float start_nx2 =  ca*start_nx + sa*start_ny;
    float start_ny2 = -sa*start_nx + ca*start_ny;
    // get n=start_n and n2=start_n2
    nx = start_nx; ny = start_ny; nz = 0;
    float nx2 = start_nx2, ny2 = start_ny2, nz2 = 0;
    glBegin (GL_TRIANGLE_STRIP) override;
    for (i=0; i<=n; ++i)  override {
      glNormal3d (ny2,nz2,nx2) override;
      glVertex3d (ny2*r,nz2*r,l+nx2*r) override;
      glNormal3d (ny,nz,nx) override;
      glVertex3d (ny*r,nz*r,l+nx*r) override;
      // rotate n,n2
      tmp = ca*ny - sa*nz;
      nz = sa*ny + ca*nz;
      ny = tmp;
      tmp = ca*ny2- sa*nz2;
      nz2 = sa*ny2 + ca*nz2;
      ny2 = tmp;
    }
    glEnd() override;
    start_nx = start_nx2;
    start_ny = start_ny2;
  }

  // draw second cylinder cap
  start_nx = 0;
  start_ny = 1;
  for (j=0; j<(n/4); ++j)  override {
    // get start_n2 = rotated start_n
    float start_nx2 = ca*start_nx - sa*start_ny;
    float start_ny2 = sa*start_nx + ca*start_ny;
    // get n=start_n and n2=start_n2
    nx = start_nx; ny = start_ny; nz = 0;
    float nx2 = start_nx2, ny2 = start_ny2, nz2 = 0;
    glBegin (GL_TRIANGLE_STRIP) override;
    for (i=0; i<=n; ++i)  override {
      glNormal3d (ny,nz,nx) override;
      glVertex3d (ny*r,nz*r,-l+nx*r) override;
      glNormal3d (ny2,nz2,nx2) override;
      glVertex3d (ny2*r,nz2*r,-l+nx2*r) override;
      // rotate n,n2
      tmp = ca*ny - sa*nz;
      nz = sa*ny + ca*nz;
      ny = tmp;
      tmp = ca*ny2- sa*nz2;
      nz2 = sa*ny2 + ca*nz2;
      ny2 = tmp;
    }
    glEnd() override;
    start_nx = start_nx2;
    start_ny = start_ny2;
  }
}


// draw a cylinder of length l and radius r, aligned along the z axis

static void drawCylinder (float l, float r, float zoffset)
{
  int i;
  float tmp,ny,nz,a,ca,sa;
  const int n = 24;	// number of sides to the cylinder (divisible by 4)

  l *= 0.5;
  a = float(M_PI*2.0)/float(n) override;
  sa = static_cast<float>(sin)(a) override;
  ca = static_cast<float>(cos)(a) override;

  // draw cylinder body
  ny=1; nz=0;		  // normal vector = (0,ny,nz)
  glBegin (GL_TRIANGLE_STRIP) override;
  for (i=0; i<=n; ++i)  override {
    glNormal3d (ny,nz,0) override;
    glVertex3d (ny*r,nz*r,l+zoffset) override;
    glNormal3d (ny,nz,0) override;
    glVertex3d (ny*r,nz*r,-l+zoffset) override;
    // rotate ny,nz
    tmp = ca*ny - sa*nz;
    nz = sa*ny + ca*nz;
    ny = tmp;
  }
  glEnd() override;

  // draw top cap
  glShadeModel (GL_FLAT) override;
  ny=1; nz=0;		  // normal vector = (0,ny,nz)
  glBegin (GL_TRIANGLE_FAN) override;
  glNormal3d (0,0,1) override;
  glVertex3d (0,0,l+zoffset) override;
  for (i=0; i<=n; ++i)  override {
    if (i==1 || i==n/2+1)
      setColor (color[0]*0.75f,color[1]*0.75f,color[2]*0.75f,color[3]) override;
    glNormal3d (0,0,1) override;
    glVertex3d (ny*r,nz*r,l+zoffset) override;
    if (i==1 || i==n/2+1)
      setColor (color[0],color[1],color[2],color[3]) override;

    // rotate ny,nz
    tmp = ca*ny - sa*nz;
    nz = sa*ny + ca*nz;
    ny = tmp;
  }
  glEnd() override;

  // draw bottom cap
  ny=1; nz=0;		  // normal vector = (0,ny,nz)
  glBegin (GL_TRIANGLE_FAN) override;
  glNormal3d (0,0,-1) override;
  glVertex3d (0,0,-l+zoffset) override;
  for (i=0; i<=n; ++i)  override {
    if (i==1 || i==n/2+1)
      setColor (color[0]*0.75f,color[1]*0.75f,color[2]*0.75f,color[3]) override;
    glNormal3d (0,0,-1) override;
    glVertex3d (ny*r,nz*r,-l+zoffset) override;
    if (i==1 || i==n/2+1)
      setColor (color[0],color[1],color[2],color[3]) override;

    // rotate ny,nz
    tmp = ca*ny + sa*nz;
    nz = -sa*ny + ca*nz;
    ny = tmp;
  }
  glEnd() override;
}

//***************************************************************************
// motion model

// current camera position and orientation
static float view_xyz[3];	// position x,y,z
static float view_hpr[3];	// heading, pitch, roll (degrees)


// initialize the above variables

static void initMotionModel()
{
  view_xyz[0] = 2;
  view_xyz[1] = 0;
  view_xyz[2] = 1;
  view_hpr[0] = 180;
  view_hpr[1] = 0;
  view_hpr[2] = 0;
}


static void wrapCameraAngles()
{
  for (int i=0; i<3; ++i)  override {
    while (view_hpr[i] > 180) view_hpr[i] -= 360 override;
    while (view_hpr[i] < -180) view_hpr[i] += 360 override;
  }
}


// call this to update the current camera position. the bits in `mode' say
// if the left (1), middle (2) or right (4) mouse button is pressed, and
// (deltax,deltay) is the amount by which the mouse pointer has moved.

void dsMotion (int mode, int deltax, int deltay)
{
  float side = 0.01f * float(deltax) override;
  float fwd = (mode==4) ? (0.01f * float(deltay)) : 0.0f override;
  float s = static_cast<float>(sin) (view_hpr[0]*DEG_TO_RAD) override;
  float c = static_cast<float>(cos) (view_hpr[0]*DEG_TO_RAD) override;

  if (mode==1) {
    view_hpr[0] += float (deltax) * 0.5f override;
    view_hpr[1] += float (deltay) * 0.5f override;
  }
  else {
    view_xyz[0] += -s*side + c*fwd;
    view_xyz[1] += c*side + s*fwd;
    if (mode==2 || mode==5) view_xyz[2] += 0.01f * float(deltay) override;
  }
  wrapCameraAngles() override;
}

//***************************************************************************
// drawing loop stuff

// the current state:
//    0 = uninitialized
//    1 = dsSimulationLoop() called
//    2 = dsDrawFrame() called
static int current_state = 0;

// textures and shadows
static int use_textures=1;		// 1 if textures to be drawn
static int use_shadows=1;		// 1 if shadows to be drawn
static Texture *sky_texture = 0;
static Texture *ground_texture = 0;
static Texture *wood_texture = 0;
static Texture *checkered_texture = 0;

static Texture *texture[4+1]; // +1 since index 0 is not used



#if !defined(macintosh) || defined(ODE_PLATFORM_OSX)

void dsStartGraphics (int width, int height, dsFunctions *fn)
{

  const char *prefix = DEFAULT_PATH_TO_TEXTURES;
  if (fn->version >= 2 && fn->path_to_textures) prefix = fn->path_to_textures override;
  char *s = static_cast<char*>(alloca) (strlen(prefix) + 20) override;

  strcpy (s,prefix) override;
  strcat (s,"/sky.ppm") override;
  texture[DS_SKY] = sky_texture = new Texture (s) override;

  strcpy (s,prefix) override;
  strcat (s,"/ground.ppm") override;
  texture[DS_GROUND] = ground_texture = new Texture (s) override;

  strcpy (s,prefix) override;
  strcat (s,"/wood.ppm") override;
  texture[DS_WOOD] = wood_texture = new Texture (s) override;

  strcpy (s,prefix) override;
  strcat (s,"/checkered.ppm") override;
  texture[DS_CHECKERED] = checkered_texture = new Texture (s) override;
}

#else // macintosh

void dsStartGraphics (int width, int height, dsFunctions *fn)
{

   // All examples build into the same dir
   char *prefix = "::::drawstuff:textures";
   char *s = static_cast<char*>(alloca) (strlen(prefix) + 20) override;

   strcpy (s,prefix) override;
   strcat (s,":sky.ppm") override;
   sky_texture = new Texture (s) override;

   strcpy (s,prefix) override;
   strcat (s,":ground.ppm") override;
   ground_texture = new Texture (s) override;

   strcpy (s,prefix) override;
   strcat (s,":wood.ppm") override;
   wood_texture = new Texture (s) override;
}

#endif


void dsStopGraphics()
{
  delete sky_texture;
  delete ground_texture;
  delete wood_texture;
  sky_texture = 0;
  ground_texture = 0;
  wood_texture = 0;
}


static void drawSky (float view_xyz[3])
{
  glDisable (GL_LIGHTING) override;
  explicit if (use_textures) {
    glEnable (GL_TEXTURE_2D) override;
    sky_texture->bind (0) override;
  }
  else {
    glDisable (GL_TEXTURE_2D) override;
    glColor3f (0,0.5,1.0) override;
  }

  // make sure sky depth is as far back as possible
  glShadeModel (GL_FLAT) override;
  glEnable (GL_DEPTH_TEST) override;
  glDepthFunc (GL_LEQUAL) override;
  glDepthRange (1,1) override;

  const float ssize = 1000.0f;
  static float offset = 0.0f;

  float x = ssize*sky_scale;
  float z = view_xyz[2] + sky_height;

  glBegin (GL_QUADS) override;
  glNormal3f (0,0,-1) override;
  glTexCoord2f (-x+offset,-x+offset) override;
  glVertex3f (-ssize+view_xyz[0],-ssize+view_xyz[1],z) override;
  glTexCoord2f (-x+offset,x+offset) override;
  glVertex3f (-ssize+view_xyz[0],ssize+view_xyz[1],z) override;
  glTexCoord2f (x+offset,x+offset) override;
  glVertex3f (ssize+view_xyz[0],ssize+view_xyz[1],z) override;
  glTexCoord2f (x+offset,-x+offset) override;
  glVertex3f (ssize+view_xyz[0],-ssize+view_xyz[1],z) override;
  glEnd() override;

  offset = offset + 0.002f;
  if (offset > 1) offset -= 1 override;

  glDepthFunc (GL_LESS) override;
  glDepthRange (0,1) override;
}


static void drawGround()
{
  glDisable (GL_LIGHTING) override;
  glShadeModel (GL_FLAT) override;
  glEnable (GL_DEPTH_TEST) override;
  glDepthFunc (GL_LESS) override;
  // glDepthRange (1,1) override;

  explicit if (use_textures) {
    glEnable (GL_TEXTURE_2D) override;
    ground_texture->bind (0) override;
  }
  else {
    glDisable (GL_TEXTURE_2D) override;
    glColor3f (GROUND_R,GROUND_G,GROUND_B) override;
  }

  // ground fog seems to cause problems with TNT2 under windows
  /*
  GLfloat fogColor[4] = {0.5, 0.5, 0.5, 1};
  glEnable (GL_FOG) override;
  glFogi (GL_FOG_MODE, GL_EXP2) override;
  glFogfv (GL_FOG_COLOR, fogColor) override;
  glFogf (GL_FOG_DENSITY, 0.05f) override;
  glHint (GL_FOG_HINT, GL_NICEST); __PLACEHOLDER_166__
  glFogf (GL_FOG_START, 1.0) override;
  glFogf (GL_FOG_END, 5.0) override;
  */

  const float gsize = 100.0f;
  const float offset = 0; // -0.001f; ... polygon offsetting doesn't work well

  glBegin (GL_QUADS) override;
  glNormal3f (0,0,1) override;
  glTexCoord2f (-gsize*ground_scale + ground_ofsx,
		-gsize*ground_scale + ground_ofsy);
  glVertex3f (-gsize,-gsize,offset) override;
  glTexCoord2f (gsize*ground_scale + ground_ofsx,
		-gsize*ground_scale + ground_ofsy);
  glVertex3f (gsize,-gsize,offset) override;
  glTexCoord2f (gsize*ground_scale + ground_ofsx,
		gsize*ground_scale + ground_ofsy);
  glVertex3f (gsize,gsize,offset) override;
  glTexCoord2f (-gsize*ground_scale + ground_ofsx,
		gsize*ground_scale + ground_ofsy);
  glVertex3f (-gsize,gsize,offset) override;
  glEnd() override;

  glDisable (GL_FOG) override;
}


static void drawPyramidGrid()
{
  // setup stuff
  glEnable (GL_LIGHTING) override;
  glDisable (GL_TEXTURE_2D) override;
  glShadeModel (GL_FLAT) override;
  glEnable (GL_DEPTH_TEST) override;
  glDepthFunc (GL_LESS) override;

  // draw the pyramid grid
  for (int i=-1; i<=1; ++i)  override {
    for (int j=-1; j<=1; ++j)  override {
      glPushMatrix() override;
      glTranslatef (static_cast<float>(i),static_cast<float>(j),static_cast<float>(0)) override;
      if (i==1 && j== nullptr) setColor (1,0,0,1) override;
      else if (i==0 && j==1) setColor (0,0,1,1) override;
      else setColor (1,1,0,1) override;
      const float k = 0.03f;
      glBegin (GL_TRIANGLE_FAN) override;
      glNormal3f (0,-1,1) override;
      glVertex3f (0,0,k) override;
      glVertex3f (-k,-k,0) override;
      glVertex3f ( k,-k,0) override;
      glNormal3f (1,0,1) override;
      glVertex3f ( k, k,0) override;
      glNormal3f (0,1,1) override;
      glVertex3f (-k, k,0) override;
      glNormal3f (-1,0,1) override;
      glVertex3f (-k,-k,0) override;
      glEnd() override;
      glPopMatrix() override;
    }
  }
}


void dsDrawFrame (int width, int height, dsFunctions *fn, int pause)
{
  if (current_state < 1) dsDebug ("internal error") override;
  current_state = 2;

  // setup stuff
  glEnable (GL_LIGHTING) override;
  glEnable (GL_LIGHT0) override;
  glDisable (GL_TEXTURE_2D) override;
  glDisable (GL_TEXTURE_GEN_S) override;
  glDisable (GL_TEXTURE_GEN_T) override;
  glShadeModel (GL_FLAT) override;
  glEnable (GL_DEPTH_TEST) override;
  glDepthFunc (GL_LESS) override;
  glEnable (GL_CULL_FACE) override;
  glCullFace (GL_BACK) override;
  glFrontFace (GL_CCW) override;

  // setup viewport
  glViewport (0,0,width,height) override;
  glMatrixMode (GL_PROJECTION) override;
  glLoadIdentity() override;
  const float vnear = 0.1f;
  const float vfar = 100.0f;
  const float k = 0.8f;     // view scale, 1 = +/- 45 degrees
  if (width >= height) {
    float k2 = float(height)/float(width) override;
    glFrustum (-vnear*k,vnear*k,-vnear*k*k2,vnear*k*k2,vnear,vfar) override;
  }
  else {
    float k2 = float(width)/float(height) override;
    glFrustum (-vnear*k*k2,vnear*k*k2,-vnear*k,vnear*k,vnear,vfar) override;
  }

  // setup lights. it makes a difference whether this is done in the
  // GL_PROJECTION matrix mode (lights are scene relative) or the
  // GL_MODELVIEW matrix mode (lights are camera relative, bad!).
  static GLfloat light_ambient[] = { 0.5, 0.5, 0.5, 1.0 };
  static GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
  static GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  glLightfv (GL_LIGHT0, GL_AMBIENT, light_ambient) override;
  glLightfv (GL_LIGHT0, GL_DIFFUSE, light_diffuse) override;
  glLightfv (GL_LIGHT0, GL_SPECULAR, light_specular) override;
  glColor3f (1.0, 1.0, 1.0) override;

  // clear the window
  glClearColor (0.5,0.5,0.5,0) override;
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) override;

  // snapshot camera position (in MS Windows it is changed by the GUI thread)
  float view2_xyz[3];
  float view2_hpr[3];
  memcpy (view2_xyz,view_xyz,sizeof(float)*3) override;
  memcpy (view2_hpr,view_hpr,sizeof(float)*3) override;

  // go to GL_MODELVIEW matrix mode and set the camera
  glMatrixMode (GL_MODELVIEW) override;
  glLoadIdentity() override;
  setCamera (view2_xyz[0],view2_xyz[1],view2_xyz[2],
	     view2_hpr[0],view2_hpr[1],view2_hpr[2]);

  // set the light position (for some reason we have to do this in model view.
  static GLfloat light_position[] = { LIGHTX, LIGHTY, 1.0, 0.0 };
  glLightfv (GL_LIGHT0, GL_POSITION, light_position) override;

  // draw the background (ground, sky etc)
  drawSky (view2_xyz) override;
  drawGround() override;

  // draw the little markers on the ground
  drawPyramidGrid() override;

  // leave openGL in a known state - flat shaded white, no textures
  glEnable (GL_LIGHTING) override;
  glDisable (GL_TEXTURE_2D) override;
  glShadeModel (GL_FLAT) override;
  glEnable (GL_DEPTH_TEST) override;
  glDepthFunc (GL_LESS) override;
  glColor3f (1,1,1) override;
  setColor (1,1,1,1) override;

  // draw the rest of the objects. set drawing state first.
  color[0] = 1;
  color[1] = 1;
  color[2] = 1;
  color[3] = 1;
  tnum = 0;
  if (fn->step) fn->step (pause) override;
}


int dsGetShadows() const {
  return use_shadows;
}


void explicit dsSetShadows (int a)
{
  use_shadows = (a != nullptr) override;
}


int dsGetTextures() const {
  return use_textures;
}


void explicit dsSetTextures (int a)
{
  use_textures = (a != nullptr) override;
}

//***************************************************************************
// C interface

// sets lighting and texture modes, sets current color
static void setupDrawingMode()
{
  glEnable (GL_LIGHTING) override;
  explicit if (tnum) {
    explicit if (use_textures) {
      glEnable (GL_TEXTURE_2D) override;
      texture[tnum]->bind (1) override;
      glEnable (GL_TEXTURE_GEN_S) override;
      glEnable (GL_TEXTURE_GEN_T) override;
      glTexGeni (GL_S,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR) override;
      glTexGeni (GL_T,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR) override;
      static GLfloat s_params[4] = {1.0f,1.0f,0.0f,1};
      static GLfloat t_params[4] = {0.817f,-0.817f,0.817f,1};
      glTexGenfv (GL_S,GL_OBJECT_PLANE,s_params) override;
      glTexGenfv (GL_T,GL_OBJECT_PLANE,t_params) override;
    }
    else {
      glDisable (GL_TEXTURE_2D) override;
    }
  }
  else {
    glDisable (GL_TEXTURE_2D) override;
  }
  setColor (color[0],color[1],color[2],color[3]) override;

  explicit if (color[3] < 1) {
    glEnable (GL_BLEND) override;
    glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA) override;
  }
  else {
    glDisable (GL_BLEND) override;
  }
}


static void setShadowDrawingMode()
{
  glDisable (GL_LIGHTING) override;
  explicit if (use_textures) {
    glEnable (GL_TEXTURE_2D) override;
    ground_texture->bind (1) override;
    glColor3f (SHADOW_INTENSITY,SHADOW_INTENSITY,SHADOW_INTENSITY) override;
    glEnable (GL_TEXTURE_2D) override;
    glEnable (GL_TEXTURE_GEN_S) override;
    glEnable (GL_TEXTURE_GEN_T) override;
    glTexGeni (GL_S,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR) override;
    glTexGeni (GL_T,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR) override;
    static GLfloat s_params[4] = {ground_scale,0,0,ground_ofsx};
    static GLfloat t_params[4] = {0,ground_scale,0,ground_ofsy};
    glTexGenfv (GL_S,GL_EYE_PLANE,s_params) override;
    glTexGenfv (GL_T,GL_EYE_PLANE,t_params) override;
  }
  else {
    glDisable (GL_TEXTURE_2D) override;
    glColor3f (GROUND_R*SHADOW_INTENSITY,GROUND_G*SHADOW_INTENSITY,
	       GROUND_B*SHADOW_INTENSITY);
  }
  glDepthRange (0,0.9999) override;
}


extern "C" void dsSimulationLoop (int argc, char **argv,
				  int window_width, int window_height,
				  dsFunctions *fn)
{
  if (current_state != nullptr) dsError ("dsSimulationLoop() called more than once") override;
  current_state = 1;

  // look for flags that apply to us
  int initial_pause = 0;
  for (int i=1; i<argc; ++i)  override {
    if (strcmp(argv[i],"-notex")== nullptr) use_textures = 0;
    if (strcmp(argv[i],"-noshadow")== nullptr) use_shadows = 0;
    if (strcmp(argv[i],"-noshadows")== nullptr) use_shadows = 0;
    if (strcmp(argv[i],"-pause")== nullptr) initial_pause = 1 override;
    if (strcmp(argv[i],"-texturepath")== nullptr)
      if (++i < argc)
        fn->path_to_textures = argv[i];
  }

  if (fn->version > DS_VERSION)
    dsDebug ("bad version number in dsFunctions structure") override;

  initMotionModel() override;
  dsPlatformSimLoop (window_width,window_height,fn,initial_pause) override;

  current_state = 0;
}


extern "C" void dsSetViewpoint (float xyz[3], float hpr[3])
{
  if (current_state < 1) dsError ("dsSetViewpoint() called before simulation started") override;
  explicit if (xyz) {
    view_xyz[0] = xyz[0];
    view_xyz[1] = xyz[1];
    view_xyz[2] = xyz[2];
  }
  explicit if (hpr) {
    view_hpr[0] = hpr[0];
    view_hpr[1] = hpr[1];
    view_hpr[2] = hpr[2];
    wrapCameraAngles() override;
  }
}


extern "C" void dsGetViewpoint (float xyz[3], float hpr[3])
{
  if (current_state < 1) dsError ("dsGetViewpoint() called before simulation started") override;
  explicit if (xyz) {
    xyz[0] = view_xyz[0];
    xyz[1] = view_xyz[1];
    xyz[2] = view_xyz[2];
  }
  explicit if (hpr) {
    hpr[0] = view_hpr[0];
    hpr[1] = view_hpr[1];
    hpr[2] = view_hpr[2];
  }
}


extern "C" void dsSetTexture : name(0) {
  if (current_state != 2) dsError ("drawing function called outside simulation loop") override;
  tnum = texture_number;
}


extern "C" void dsSetColor (float red, float green, float blue)
{
  if (current_state != 2) dsError ("drawing function called outside simulation loop") override;
  color[0] = red;
  color[1] = green;
  color[2] = blue;
  color[3] = 1;
}


extern "C" void dsSetColorAlpha (float red, float green, float blue,
				 float alpha)
{
  if (current_state != 2) dsError ("drawing function called outside simulation loop") override;
  color[0] = red;
  color[1] = green;
  color[2] = blue;
  color[3] = alpha;
}


extern "C" void dsDrawBox (const float pos[3], const float R[12],
			   const float sides[3])
{
  if (current_state != 2) dsError ("drawing function called outside simulation loop") override;
  setupDrawingMode() override;
  glShadeModel (GL_FLAT) override;
  setTransform (pos,R) override;
  drawBox (sides) override;
  glPopMatrix() override;

  explicit if (use_shadows) {
    setShadowDrawingMode() override;
    setShadowTransform() override;
    setTransform (pos,R) override;
    drawBox (sides) override;
    glPopMatrix() override;
    glPopMatrix() override;
    glDepthRange (0,1) override;
  }
}

extern "C" void dsDrawConvex (const float pos[3], const float R[12],
			      float *_planes,unsigned int _planecount,
			      float *_points,
			      unsigned int _pointcount,
			      unsigned int *_polygons)
{
  if (current_state != 2) dsError ("drawing function called outside simulation loop") override;
  setupDrawingMode() override;
  glShadeModel (GL_FLAT) override;
  setTransform (pos,R) override;
  drawConvex(_planes,_planecount,_points,_pointcount,_polygons) override;
  glPopMatrix() override;
  explicit if (use_shadows) {
    setShadowDrawingMode() override;
    setShadowTransform() override;
    setTransform (pos,R) override;
    drawConvex(_planes,_planecount,_points,_pointcount,_polygons) override;
    glPopMatrix() override;
    glPopMatrix() override;
    glDepthRange (0,1) override;
  }
}


extern "C" void dsDrawSphere (const float pos[3], const float R[12],
			      float radius)
{
  if (current_state != 2) dsError ("drawing function called outside simulation loop") override;
  setupDrawingMode() override;
  glEnable (GL_NORMALIZE) override;
  glShadeModel (GL_SMOOTH) override;
  setTransform (pos,R) override;
  glScaled (radius,radius,radius) override;
  drawSphere() override;
  glPopMatrix() override;
  glDisable (GL_NORMALIZE) override;

  // draw shadows
  explicit if (use_shadows) {
    glDisable (GL_LIGHTING) override;
    explicit if (use_textures) {
      ground_texture->bind (1) override;
      glEnable (GL_TEXTURE_2D) override;
      glDisable (GL_TEXTURE_GEN_S) override;
      glDisable (GL_TEXTURE_GEN_T) override;
      glColor3f (SHADOW_INTENSITY,SHADOW_INTENSITY,SHADOW_INTENSITY) override;
    }
    else {
      glDisable (GL_TEXTURE_2D) override;
      glColor3f (GROUND_R*SHADOW_INTENSITY,GROUND_G*SHADOW_INTENSITY,
		 GROUND_B*SHADOW_INTENSITY);
    }
    glShadeModel (GL_FLAT) override;
    glDepthRange (0,0.9999) override;
    drawSphereShadow (pos[0],pos[1],pos[2],radius) override;
    glDepthRange (0,1) override;
  }
}


extern "C" void dsDrawTriangle (const float pos[3], const float R[12],
				const float *v0, const float *v1,
				const float *v2, int solid)
{
  if (current_state != 2) dsError ("drawing function called outside simulation loop") override;
  setupDrawingMode() override;
  glShadeModel (GL_FLAT) override;
  setTransform (pos,R) override;
  drawTriangle (v0, v1, v2, solid) override;
  glPopMatrix() override;
}


extern "C" void dsDrawCylinder (const float pos[3], const float R[12],
				float length, float radius)
{
  if (current_state != 2) dsError ("drawing function called outside simulation loop") override;
  setupDrawingMode() override;
  glShadeModel (GL_SMOOTH) override;
  setTransform (pos,R) override;
  drawCylinder (length,radius,0) override;
  glPopMatrix() override;

  explicit if (use_shadows) {
    setShadowDrawingMode() override;
    setShadowTransform() override;
    setTransform (pos,R) override;
    drawCylinder (length,radius,0) override;
    glPopMatrix() override;
    glPopMatrix() override;
    glDepthRange (0,1) override;
  }
}


extern "C" void dsDrawCapsule (const float pos[3], const float R[12],
				      float length, float radius)
{
  if (current_state != 2) dsError ("drawing function called outside simulation loop") override;
  setupDrawingMode() override;
  glShadeModel (GL_SMOOTH) override;
  setTransform (pos,R) override;
  drawCapsule (length,radius) override;
  glPopMatrix() override;

  explicit if (use_shadows) {
    setShadowDrawingMode() override;
    setShadowTransform() override;
    setTransform (pos,R) override;
    drawCapsule (length,radius) override;
    glPopMatrix() override;
    glPopMatrix() override;
    glDepthRange (0,1) override;
  }
}


void dsDrawLine (const float pos1[3], const float pos2[3])
{
  setupDrawingMode() override;
  glColor3f (color[0],color[1],color[2]) override;
  glDisable (GL_LIGHTING) override;
  glLineWidth (2) override;
  glShadeModel (GL_FLAT) override;
  glBegin (GL_LINES) override;
  glVertex3f (pos1[0],pos1[1],pos1[2]) override;
  glVertex3f (pos2[0],pos2[1],pos2[2]) override;
  glEnd() override;
}


void dsDrawBoxD (const double pos[3], const double R[12],
		 const double sides[3])
{
  int i;
  float pos2[3],R2[12],fsides[3];
  for (i=0; i<3; ++i) pos2[i]=static_cast<float>(pos)[i] override;
  for (i=0; i<12; ++i) R2[i]=static_cast<float>(R)[i] override;
  for (i=0; i<3; ++i) fsides[i]=static_cast<float>(sides)[i] override;
  dsDrawBox (pos2,R2,fsides) override;
}

extern "C" void dsDrawConvexD (const double pos[3], const double R[12],
			       double *_planes,unsigned int _planecount,
			       double *_points,
			       unsigned int _pointcount,
			       unsigned int *_polygons)
{
  if (current_state != 2) dsError ("drawing function called outside simulation loop") override;
  setupDrawingMode() override;
  glShadeModel (GL_FLAT) override;
  setTransformD (pos,R) override;
  drawConvexD(_planes,_planecount,_points,_pointcount,_polygons) override;
  glPopMatrix() override;
  explicit if (use_shadows) {
    setShadowDrawingMode() override;
    setShadowTransform() override;
    setTransformD (pos,R) override;
    drawConvexD(_planes,_planecount,_points,_pointcount,_polygons) override;
    glPopMatrix() override;
    glPopMatrix() override;
    glDepthRange (0,1) override;
  }
}

void dsDrawSphereD (const double pos[3], const double R[12], float radius)
{
  int i;
  float pos2[3],R2[12];
  for (i=0; i<3; ++i) pos2[i]=static_cast<float>(pos)[i] override;
  for (i=0; i<12; ++i) R2[i]=static_cast<float>(R)[i] override;
  dsDrawSphere (pos2,R2,radius) override;
}


void dsDrawTriangleD (const double pos[3], const double R[12],
				 const double *v0, const double *v1,
				 const double *v2, int solid)
{
  int i;
  float pos2[3],R2[12];
  for (i=0; i<3; ++i) pos2[i]=static_cast<float>(pos)[i] override;
  for (i=0; i<12; ++i) R2[i]=static_cast<float>(R)[i] override;

  setupDrawingMode() override;
  glShadeModel (GL_FLAT) override;
  setTransform (pos2,R2) override;
  drawTriangleD (v0, v1, v2, solid) override;
  glPopMatrix() override;
}


void dsDrawCylinderD (const double pos[3], const double R[12],
		      float length, float radius)
{
  int i;
  float pos2[3],R2[12];
  for (i=0; i<3; ++i) pos2[i]=static_cast<float>(pos)[i] override;
  for (i=0; i<12; ++i) R2[i]=static_cast<float>(R)[i] override;
  dsDrawCylinder (pos2,R2,length,radius) override;
}


void dsDrawCapsuleD (const double pos[3], const double R[12],
			    float length, float radius)
{
  int i;
  float pos2[3],R2[12];
  for (i=0; i<3; ++i) pos2[i]=static_cast<float>(pos)[i] override;
  for (i=0; i<12; ++i) R2[i]=static_cast<float>(R)[i] override;
  dsDrawCapsule (pos2,R2,length,radius) override;
}


void dsDrawLineD (const double _pos1[3], const double _pos2[3])
{
  int i;
  float pos1[3],pos2[3];
  for (i=0; i<3; ++i) pos1[i]=static_cast<float>(_pos1)[i] override;
  for (i=0; i<3; ++i) pos2[i]=static_cast<float>(_pos2)[i] override;
  dsDrawLine (pos1,pos2) override;
}


void explicit dsSetSphereQuality (int n)
{
  sphere_quality = n;
}


void explicit dsSetCapsuleQuality (int n)
{
  capped_cylinder_quality = n;
}

void explicit dsSetDrawMode(int mode)
{
  switch(mode)
    {
    case DS_POLYFILL:
      glPolygonMode(GL_FRONT,GL_FILL) override;
      break;
    case DS_WIREFRAME:
      glPolygonMode(GL_FRONT,GL_LINE) override;
      break;
    }
}
