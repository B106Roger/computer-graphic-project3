#ifndef TRAINVIEW_H  
#define TRAINVIEW_H  
#include <QGLWidget> 
#include <QtGui>  
#include <QtOpenGL>  
#include <GL/GLU.h>
#include <QtMultimedia/QMediaPlayer>
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib") 
#include "Utilities/ArcBallCam.H"
#include "Utilities/3DUtils.H"
#include "Track.H"
#include "Triangle.h"
#include "Square.h"
#include "Model.h"
#include "Water.h"
#include "Skybox.h"
#include "Mountain.h"



class AppMain;
class CTrack;
typedef struct tag_PARTICLE
{
	GLfloat xpos;//(xpos,ypos,zpos)為particle的position
	GLfloat ypos;
	GLfloat zpos;
	GLfloat xspeed;//(xspeed,yspeed,zspeed)為particle的speed
	GLfloat yspeed;
	GLfloat zspeed;
	GLfloat r;//(r,g,b)為particle的color
	GLfloat g;
	GLfloat b;
	GLfloat life;// particle的壽命 
	GLfloat fade;// particle的衰減速度
	GLfloat size;// particle的大小  
	GLbyte bFire;
	GLbyte nExpl;//哪種particle效果  
	GLbyte bAddParts;// particle是否含有尾巴
	GLfloat AddSpeed;//尾巴粒子的加速度  
	GLfloat AddCount;//尾巴粒子的增加量  
	tag_PARTICLE* pNext;//下一particle 
	tag_PARTICLE* pPrev;//上一particle   
} Particle, *pParticle;

//#######################################################################
// TODO
// You might change the TrainView in order to add different objects to
// be drawn, or adjust the lighting, or ...
//#######################################################################

struct TrainItem
{
	Model * train;
	int curveIndex;
	int pointIndex;
	bool isHead;
	TrainItem(int curveInt, int pointInt, bool head, Model *tr = NULL)
	{
		curveIndex = curveInt;
		pointIndex = pointInt;
		head = isHead;
		train = tr;
	}
};

class TrainView : public QGLWidget, protected QOpenGLFunctions_4_3_Core
{  
	Q_OBJECT  
public:  
	explicit TrainView(QWidget *parent = 0);  
	~TrainView();  

public:
	// overrides of important window things
	//virtual int handle(int);
	virtual void paintGL();

	// all of the actual drawing happens in this routine
	// it has to be encapsulated, since we draw differently if
	// we're drawing shadows (no colors, for example)
	void drawStuff(bool doingShadows=false);
	void drawTrack(bool doingShadows = false);
	void DrawParticles();
	// setup the projection - assuming that the projection stack has been
	// cleared for you
	void setProjection();

	// Reset the Arc ball control
	void resetArcball();

	// pick a point (for when the mouse goes down)
	void doPick(int mx, int my);

	void initializeGL();
	void initializeTexture();
	void initializeMedia();
	void printFPS();

public:
	ArcBallCam		arcball;			// keep an ArcBall for the UI
	int				selectedCube;  // simple - just remember which cube is selected

	CTrack*			m_pTrack;		// The track of the entire scene
	QMediaPlayer* DJ;
	int camera;
	int curve;
	int track;
	float speed;
	bool isrun;
	bool isTire;
	vector<vector<vector<float>>> M_curve;

	Triangle* triangle;
	Water *water;
	Square* square;
	GLfloat ProjectionMatrex[16];
	GLfloat ModelViewMatrex[16];
	QVector<QOpenGLTexture*> Textures;

	float t_time;
	unsigned int DIVIDE_LINE;
	const float RAIL_WIDTH = 2.f;
	float TRAIN_SPEED;
	void drawTrain(float);
	vector<TrainItem> trainList;
	
	// **********************************
	// ********** 其他小物件   **********
	// **********************************
	Model *spaceShip;
	Model *spaceShipReflection;
	Model *spaceShipRefraction;
	Skybox *sky;
	Mountain * mountain;

	Model *planet;
	Model **spaceTest;


	// **********************************
	// ****** Particle Parameter   ******
	// **********************************
	pParticle Particles;
	UINT nOfFires;
	UINT Tick1, Tick2;  //(GetTickCount())
	float DTick;  //(float(Tick2 - Tick1))
	GLfloat grav; //(0.00003f)
	GLuint textureID;
	void DeleteParticle(pParticle* p);
	void DeleteAll(pParticle* Part);
	void AddParticle(Particle ex);
	void ProcessParticles();
	void InitParticle(Particle& ep);
	void Explosion1(Particle* par);
	void Explosion2(Particle* par);
	void Explosion3(Particle* par);
#define MAX_PARTICLES 1000  
#define MAX_FIRES 5  
};  



void DimensionTransformation(GLfloat source[], GLfloat target[][4]);
#endif // TRAINVIEW_H  