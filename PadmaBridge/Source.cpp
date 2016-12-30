#define _CRT_SECURE_NO_DEPRECATE
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<windows.h>
#include<GL/glut.h>


double cameraAngle;
double move_X, move_Y, move_Z;
int canDrawGrid, canDrawAxis;
double speed = 5, ang_speed = .1;
double cameraRadius, cameraHeight, cameraAngleDelta;
int num_texture = -1;
GLuint waterImage, stoneImage, stone2Image;

/***************************** VECTOR structure **********************************/

struct V;

V operator+(V a, V b);
V operator*(V a, V b);
V operator*(V b, double a);
V operator*(double a, V b);

struct V {
	double x, y, z;

	V() {}
	V(double _x, double _y) { x = _x; y = _y; z = 0; }
	V(double _x, double _y, double _z) { x = _x; y = _y; z = _z; }


	double	mag() { return sqrt(x*x + y*y + z*z); }

	void 	norm() { double d = mag(); x /= d; y /= d;	z /= d; }
	V 		unit() { V ret = *this;	ret.norm(); return ret; }

	double	dot(V b) { return x*b.x + y*b.y + z*b.z; }
	V		cross(V b) { return V(y*b.z - z*b.y, z*b.x - x*b.z, x*b.y - y*b.x); }


	double	projL(V on) { on.norm();	return this->dot(on); }
	V		projV(V on) { on.norm();	return on * projL(on); }

	V rot(V axis, double angle) {
		return this->rot(axis, cos(angle), sin(angle));
	}

	V rot(V axis, double ca, double sa) {
		V rotatee = *this;
		axis.norm();
		V normal = (axis * rotatee).unit();
		V mid = (normal * axis).unit();
		double r = rotatee.projL(mid);
		V ret = r*mid*ca + r*normal*sa + rotatee.projV(axis);
		return ret.unit();
	}
};

V operator+(V a, V b) { return V(a.x + b.x, a.y + b.y, a.z + b.z); }
V operator-(V a) { return V(-a.x, -a.y, -a.z); }
V operator-(V a, V b) { return V(a.x - b.x, a.y - b.y, a.z - b.z); }
V operator*(V a, V b) { return a.cross(b); }
V operator*(double a, V b) { return V(a*b.x, a*b.y, a*b.z); }
V operator*(V b, double a) { return V(a*b.x, a*b.y, a*b.z); }
V operator/(V b, double a) { return V(b.x / a, b.y / a, b.z / a); }
V loc, dir, perp;
V  _L(0, -150, 20);
V  _D(0, 1, 0);
V  _P(0, 0, 1);

/***************************** Texture Functions *******************************/

int LoadBitmapImage(char *filename)
{
	int i, j = 0;
	FILE *l_file;
	unsigned char *l_texture;

	BITMAPFILEHEADER fileheader;
	BITMAPINFOHEADER infoheader;
	RGBTRIPLE rgb;

	num_texture++;

	if ((l_file = fopen(filename, "rb")) == NULL) return (-1);

	fread(&fileheader, sizeof(fileheader), 1, l_file);

	fseek(l_file, sizeof(fileheader), SEEK_SET);
	fread(&infoheader, sizeof(infoheader), 1, l_file);

	l_texture = (byte *)malloc(infoheader.biWidth * infoheader.biHeight * 4);
	memset(l_texture, 0, infoheader.biWidth * infoheader.biHeight * 4);
	for (i = 0; i < infoheader.biWidth*infoheader.biHeight; i++)
	{
		fread(&rgb, sizeof(rgb), 1, l_file);

		l_texture[j + 0] = rgb.rgbtRed;
		l_texture[j + 1] = rgb.rgbtGreen;
		l_texture[j + 2] = rgb.rgbtBlue;
		l_texture[j + 3] = 255;
		j += 4;
	}
	fclose(l_file);

	glBindTexture(GL_TEXTURE_2D, num_texture);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

	// glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, infoheader.biWidth, infoheader.biHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, l_texture);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, infoheader.biWidth, infoheader.biHeight, GL_RGBA, GL_UNSIGNED_BYTE, l_texture);

	free(l_texture);

	return (num_texture);

}

void loadImage()
{
	waterImage = LoadBitmapImage("images/water.bmp");
	stoneImage = LoadBitmapImage("images/stone.bmp");
	stone2Image = LoadBitmapImage("images/stone2.bmp");
	printf("Load successful");
}


void cube(float x = 1, float y = 1, float z = 1, GLuint texture = 0) {

	// FrontSide
	glEnable(GL_TEXTURE_2D); {
		glBindTexture(GL_TEXTURE_2D, texture);
		glColor3f(1, 1, 1);
		glBegin(GL_QUADS); {
			glTexCoord2f(0, 1);
			glVertex3f(-x, y, z);
			glTexCoord2f(0, 0);
			glVertex3f(-x, y, -z);
			glTexCoord2f(1, 0);
			glVertex3f(x, y, -z);
			glTexCoord2f(1, 1);
			glVertex3f(x, y, z);
		}glEnd();
	}glDisable(GL_TEXTURE_2D);

	// BackSide
	glEnable(GL_TEXTURE_2D); {
		glBindTexture(GL_TEXTURE_2D, texture);
		glColor3f(1, 1, 1);
		glBegin(GL_QUADS); {
			glTexCoord2f(0, 1);
			glVertex3f(-x, -y, z);
			glTexCoord2f(0, 0);
			glVertex3f(-x, -y, -z);
			glTexCoord2f(1, 0);
			glVertex3f(x, -y, -z);
			glTexCoord2f(1, 1);
			glVertex3f(x, -y, z);
		}glEnd();
	}glDisable(GL_TEXTURE_2D);

	// RightSide
	glEnable(GL_TEXTURE_2D); {
		glBindTexture(GL_TEXTURE_2D, texture);
		glColor3f(1, 1, 1);
		glBegin(GL_QUADS); {
			glTexCoord2f(0, 1);
			glVertex3f(x, y, z);
			glTexCoord2f(0, 0);
			glVertex3f(x, y, -z);
			glTexCoord2f(1, 0);
			glVertex3f(x, -y, -z);
			glTexCoord2f(1, 1);
			glVertex3f(x, -y, z);
		}glEnd();
	}glDisable(GL_TEXTURE_2D);

	// LeftSide
	glEnable(GL_TEXTURE_2D); {
		glBindTexture(GL_TEXTURE_2D, texture);
		glColor3f(1, 1, 1);
		glBegin(GL_QUADS); {
			glTexCoord2f(0, 1);
			glVertex3f(-x, y, z);
			glTexCoord2f(0, 0);
			glVertex3f(-x, y, -z);
			glTexCoord2f(1, 0);
			glVertex3f(-x, -y, -z);
			glTexCoord2f(1, 1);
			glVertex3f(-x, -y, z);
		}glEnd();
	}glDisable(GL_TEXTURE_2D);

	// Top Side
	glEnable(GL_TEXTURE_2D); {
		glBindTexture(GL_TEXTURE_2D, texture);
		glColor3f(1, 1, 1);
		glBegin(GL_QUADS); {
			glTexCoord2f(0, 1);
			glVertex3f(-x, y, z);
			glTexCoord2f(0, 0);
			glVertex3f(x, y, z);
			glTexCoord2f(1, 0);
			glVertex3f(x, -y, z);
			glTexCoord2f(1, 1);
			glVertex3f(-x, -y, z);
		}glEnd();
	}glDisable(GL_TEXTURE_2D);

	// Bottom Side
	glEnable(GL_TEXTURE_2D); {
		glBindTexture(GL_TEXTURE_2D, texture);
		glColor3f(1, 1, 1);
		glBegin(GL_QUADS); {
			glTexCoord2f(0, 1);
			glVertex3f(-x, y, -z);
			glTexCoord2f(0, 0);
			glVertex3f(x, y, -z);
			glTexCoord2f(1, 0);
			glVertex3f(x, -y, -z);
			glTexCoord2f(1, 1);
			glVertex3f(-x, -y, -z);
		}glEnd();
	}glDisable(GL_TEXTURE_2D);
}


void singlePiller(float x = 0, float y = 0, float z = 0, float angle = 0, float rx = 0, float ry = 0, float rz = 0) {
	glPushMatrix(); {
		glRotatef(angle, rx, ry, rz);
		glTranslatef(x, y, 10+z);

		// Left Part
		glPushMatrix(); {
			glTranslatef(-106, 0, 20);
			double equ1[4];
			equ1[0] = 1;
			equ1[1] = 0;
			equ1[2] = 0;
			equ1[3] = -60;

			double equ2[4];
			equ2[0] = 0;
			equ2[1] = 0;
			equ2[2] = 1;
			equ2[3] = -30;

			glClipPlane(GL_CLIP_PLANE0, equ1);
			glClipPlane(GL_CLIP_PLANE1, equ2);

			glPushMatrix(); {
				glPushMatrix(); {
					glEnable(GL_CLIP_PLANE1); {
						glEnable(GL_CLIP_PLANE0); {
							glColor3f(0, 0.3, 0.8);
							glPushMatrix(); {
								glRotatef(90, 1, 0, 0);
								glutSolidTorus(10, 100, 80, 150);
							}glPopMatrix();
						}glDisable(GL_CLIP_PLANE0);
					}glDisable(GL_CLIP_PLANE1);
				}glPopMatrix();
			}glPopMatrix();
		}glPopMatrix();

		// Right Part
		glPushMatrix(); {
			glRotatef(180, 0, 0, 1);
			glTranslatef(-106, 0, 20);
			double equ1[4];
			equ1[0] = 1;
			equ1[1] = 0;
			equ1[2] = 0;
			equ1[3] = -60;

			double equ2[4];
			equ2[0] = 0;
			equ2[1] = 0;
			equ2[2] = 1;
			equ2[3] = -30;

			glClipPlane(GL_CLIP_PLANE0, equ1);
			glClipPlane(GL_CLIP_PLANE1, equ2);

			glPushMatrix(); {
				glPushMatrix(); {
					glEnable(GL_CLIP_PLANE1); {
						glEnable(GL_CLIP_PLANE0); {
							glColor3f(0, 0.3, 0.8);
							glPushMatrix(); {
								glRotatef(90, 1, 0, 0);
								glutSolidTorus(10, 100, 80, 150);
							}glPopMatrix();
						}glDisable(GL_CLIP_PLANE0);
					}glDisable(GL_CLIP_PLANE1);
				}glPopMatrix();
			}glPopMatrix();
		}glPopMatrix();

		// Bottom Cube
		glColor3f(0, 0.3, 0.8);
		glPushMatrix(); {
			glTranslatef(0, 0, 40);
			glScalef(60, 20, 80);
			glutSolidCube(1);
		}glPopMatrix();


		// Upper Cube
		glColor3f(0, 0.3, 0.8);
		glPushMatrix(); {
			glTranslatef(0, 0, 101);
			glScalef(90, 15, 15);
			glutSolidCube(1);
		}glPopMatrix();

		// Upper Cube
		glColor3f(0, 0.3, 0.8);
		glPushMatrix(); {
			glTranslatef(0, 0, 85);
			glScalef(60, 15, 20);
			glutSolidCube(1);
		}glPopMatrix();

		// Base
		/*glColor3f(0.39, 0.517, 0.56);
		glPushMatrix(); {
			glTranslatef(0, 0, -5);
			glScalef(100, 40, 10);
			glutSolidCube(1);
		}glPopMatrix();*/

		cube(50, 20, 20, stoneImage);

	}glPopMatrix();
}

void singleBlock(float x = 0, float y = 0, float z = 0, float angle = 0, float rx = 0, float ry = 0, float rz = 0) {
	// Right
	glPushMatrix(); {
		glTranslatef(42, 0, 125);
		cube(3, 150, 15, stone2Image);
	}glPopMatrix();

	//Left
	glPushMatrix(); {
		glColor3f(0.4, 0, 0.2);
		glTranslatef(-42, 0, 125);
		cube(3, 150, 15, stone2Image);
	}glPopMatrix();
}

void piller() {
	for (int i = 0; i <= 8; i++) {
		singlePiller(0, i*300);
		singlePiller(0, i*-300);
	}
	
}

void waterBody() {
	glEnable(GL_TEXTURE_2D); {
		glBindTexture(GL_TEXTURE_2D, waterImage);
		glColor3f(1, 1, 1);
		glBegin(GL_QUADS); {
			glTexCoord2f(0, 1);
			glVertex3f(-2000, 2000 , 0);
			glTexCoord2f(0, 0);
			glVertex3f(-2000, -2000, 0);
			glTexCoord2f(1, 0);
			glVertex3f(2000, -2000, 0);
			glTexCoord2f(1, 1);
			glVertex3f(2000, 2000, 0);
		}glEnd();
	}glDisable(GL_TEXTURE_2D);
}

void block() {
	singleBlock();
}

void myObjects() {
	waterBody();
	piller();
	block();
	
}

void gridAndAxis() {
	// draw the three major AXES
	if (canDrawAxis == 1) {
		glBegin(GL_LINES);
		//X axis
		glColor3f(0, 1, 0);	//100% Green
		glVertex3f(-150, 0, 0);
		glVertex3f(150, 0, 0);

		//Y axis
		glColor3f(0, 0, 1);	//100% Blue
		glVertex3f(0, -150, 0);	// intentionally extended to -150 to 150, no big deal
		glVertex3f(0, 150, 0);

		//Z axis
		glColor3f(1, 1, 1);	//100% White
		glVertex3f(0, 0, -150);
		glVertex3f(0, 0, 150);
		glEnd();
	}

	if (canDrawGrid == 1) {
		//some gridlines along the field
		int i;

		glColor3f(0.5, 0.5, 0.5);	//grey
		glBegin(GL_LINES);
		for (i = -10; i <= 10; i++) {

			if (i == 0)
				continue;	//SKIP the MAIN axes

							//lines parallel to Y-axis
			glVertex3f(i * 10, -100, 0);
			glVertex3f(i * 10, 100, 0);

			//lines parallel to X-axis
			glVertex3f(-100, i * 10, 0);
			glVertex3f(100, i * 10, 0);
		}
		glEnd();

	}
}

void display() {
	//codes for Models, Camera

	//clear the display
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(1, 1, 1, 0);	//color black
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		//clear buffers to preset values

															/***************************
															/ set-up camera (view) here
															****************************/
															//load the correct matrix -- MODEL-VIEW matrix
	glMatrixMode(GL_MODELVIEW);		//specify which matrix is the current matrix

									//initialize the matrix
	glLoadIdentity();				//replace the current matrix with the identity matrix [Diagonals have 1, others have 0]

									//now give three info
									//1. where is the camera (viewer)?
									//2. where is the camera looking?
									//3. Which direction is the camera's UP direction?

									//gluLookAt(0,-150,20,	0,0,0,	0,0,1);
									//gluLookAt(cameraRadius*sin(cameraAngle), -cameraRadius*cos(cameraAngle), cameraHeight,		0,0,0,		0,0,1);

	V to = loc + dir;
	gluLookAt(loc.x, loc.y, loc.z, to.x, to.y, to.z, perp.x, perp.y, perp.z);

	//again select MODEL-VIEW
	glMatrixMode(GL_MODELVIEW);


	/**************************************************
	/ Grid and axes Lines(You can remove them if u want)
	***************************************************/
	gridAndAxis();


	/****************************
	/ Add your objects from here
	****************************/
	myObjects();

	//ADD this line in the end --- if you use double buffer (i.e. GL_DOUBLE)
	glFlush();
	glutSwapBuffers();
}

void animate() {
	//codes for any changes in Models, Camera

	//cameraAngle += cameraAngleDelta;	// camera will rotate at 0.002 radians per frame.

	//codes for any changes in Models

	//MISSING SOMETHING? -- YES: add the following
	glutPostRedisplay();	//this will call the display AGAIN

}

void init() {
	loadImage();
	//codes for initialization

	cameraAngle = 0;	//angle in radian
	move_X = 0;
	move_Y = 0;
	move_Z = 0;
	canDrawGrid = 1;
	canDrawAxis = 1;

	cameraAngleDelta = .001;

	cameraRadius = 150;
	cameraHeight = 50;


	loc = _L, dir = _D, perp = _P;
	//clear the screen
	glClearColor(0, 0, 0, 0);

	/************************
	/ set-up projection here
	************************/
	//load the PROJECTION matrix
	glMatrixMode(GL_PROJECTION);

	//initialize the matrix
	glLoadIdentity();

	/*
	gluPerspective() — set up a perspective projection matrix

	fovy -         Specifies the field of view angle, in degrees, in the y direction.
	aspect ratio - Specifies the aspect ratio that determines the field of view in the x direction. The aspect ratio is the ratio of x (width) to y (height).
	zNear -        Specifies the distance from the viewer to the near clipping plane (always positive).
	zFar  -        Specifies the distance from the viewer to the far clipping plane (always positive).
	*/

	gluPerspective(70, 1, 0.1, 10000.0);

}

void keyboardListener(unsigned char key, int x, int y) {
	switch (key) {

	case 'w':case 'W':
		loc = loc + speed*dir;
		break;

	case 'a':case 'A':
		loc = loc + speed*((perp*dir).unit());
		break;

	case 's':case 'S':
		loc = loc - speed*dir;
		break;

	case 'd':case 'D':
		loc = loc + speed*((dir*perp).unit());
		break;

	case 'q':case 'Q':
		perp = perp.rot(dir, ang_speed);
		break;

	case 'e':case 'E':
		perp = perp.rot(-dir, ang_speed);
		break;


	default:
		break;
	}
}

void specialKeyListener(int key, int x, int y) {
	V p;

	switch (key) {
	case GLUT_KEY_DOWN:
		p = (dir*perp).unit();
		dir = dir.rot(p, ang_speed);
		perp = perp.rot(p, ang_speed);
		break;
	case GLUT_KEY_UP:
		p = (perp*dir).unit();
		dir = dir.rot(p, ang_speed);
		perp = perp.rot(p, ang_speed);
		break;

	case GLUT_KEY_LEFT:
		dir = dir.rot(-perp, ang_speed);
		break;
	case GLUT_KEY_RIGHT:
		dir = dir.rot(perp, ang_speed);
		break;

	case GLUT_KEY_PAGE_UP:
		loc = loc + speed*perp;
		break;
	case GLUT_KEY_PAGE_DOWN:
		loc = loc - speed*perp;
		break;

	case GLUT_KEY_INSERT:
		break;

	case GLUT_KEY_HOME:
		loc = _L, dir = _D, perp = _P;
		break;
	case GLUT_KEY_END:
		break;

	default:
		break;
	}
}

void mouseListener(int button, int state, int x, int y) {	//x, y is the x-y of the screen (2D)
	switch (button) {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN) {		// 2 times?? in ONE click? -- solution is checking DOWN or UP

		}
		break;

	case GLUT_RIGHT_BUTTON:
		//........
		break;

	case GLUT_MIDDLE_BUTTON:
		//........
		break;

	default:
		break;
	}
}

int main(int argc, char **argv) {

	glutInit(&argc, argv);							//initialize the GLUT library

	glutInitWindowSize(750, 750);
	glutInitWindowPosition(0, 0);

	/*
	glutInitDisplayMode - inits display mode
	GLUT_DOUBLE - allows for display on the double buffer window
	GLUT_RGBA - shows color (Red, green, blue) and an alpha
	GLUT_DEPTH - allows for depth buffer
	*/
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);

	glutCreateWindow("The Padma Bridge in Bangladesh");

	printf("Camera Control\n");
	printf("_____________\n");
	printf("Roll: UP and DOWN arrow\n");
	printf("Pitch: Q and E\n");
	printf("Yaw: LEFT and RIGHT arrow\n");
	printf("Up-Down: PAGEUP and PAGEDOWN\n");
	printf("Left-Right: A and D\n");
	printf("Zoom in-out: W and S\n");
	printf("Reset Camera: HOME\n");

	init();						//codes for initialization

	glEnable(GL_DEPTH_TEST);	//enable Depth Testing

	glutDisplayFunc(display);	//display callback function
	glutIdleFunc(animate);		//what you want to do in the idle time (when no drawing is occuring)

	glutKeyboardFunc(keyboardListener);
	glutSpecialFunc(specialKeyListener);

	glutMouseFunc(mouseListener);

	glutMainLoop();		//The main loop of OpenGL

	return 0;
}
