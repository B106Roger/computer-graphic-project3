#include "AppMain.h"

#include "Utilities/3DUtils.H"
#include "Track.H"
#include <math.h>
#include <time.h>

AppMain* AppMain::Instance = NULL;
AppMain::AppMain(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	trainview = new TrainView();  
	trainview->m_pTrack =  &m_Track;
	setGeometry(200,200,1000,768);   // 1000, 768
	ui.mainLayout->layout()->addWidget(trainview);
	trainview->installEventFilter(this);
	this->canpan = false;
	this->isHover = false;
	this->trainview->camera = 0;
	this->trainview->track = 0;
	this->trainview->curve = 0;
	this->trainview->isrun = false;

	setWindowTitle( "Roller Coaster" );

	connect( ui.aLoadPath	,SIGNAL(triggered()),this,SLOT(LoadTrackPath())	);
	connect( ui.aSavePath	,SIGNAL(triggered()),this,SLOT(SaveTrackPath())	);
	connect( ui.aExit		,SIGNAL(triggered()),this,SLOT(ExitApp())		);

	connect( ui.comboCamera	,SIGNAL(currentIndexChanged(QString)),this,SLOT(ChangeCameraType(QString)));
	connect( ui.aWorld		,SIGNAL(triggered()),this,SLOT(ChangeCamToWorld())	);
	connect( ui.aTop		,SIGNAL(triggered()),this,SLOT(ChangeCamToTop())	);
	connect( ui.aTrain		,SIGNAL(triggered()),this,SLOT(ChangeCamToTrain())	);

	connect( ui.comboCurve	,SIGNAL(currentIndexChanged(QString)),this,SLOT(ChangeCurveType(QString)));
	connect( ui.aLinear		,SIGNAL(triggered()),this,SLOT(ChangeCurveToLinear())	);
	connect( ui.aCardinal	,SIGNAL(triggered()),this,SLOT(ChangeCurveToCardinal())	);
	connect( ui.aCubic		,SIGNAL(triggered()),this,SLOT(ChangeCurveToCubic())	);

	connect( ui.comboTrack	,SIGNAL(currentIndexChanged(QString)),this,SLOT(ChangeTrackType(QString)));
	connect( ui.aLine		,SIGNAL(triggered()),this,SLOT(ChangeTrackToLine())		);
	connect( ui.aTrack		,SIGNAL(triggered()),this,SLOT(ChangeTrackToTrack())	);
	connect( ui.aRoad		,SIGNAL(triggered()),this,SLOT(ChangeTrackToRoad())		);

	connect( ui.bPlay		,SIGNAL(clicked()),this,SLOT(SwitchPlayAndPause())				);
	connect( ui.sSpeed		,SIGNAL(valueChanged(int)),this,SLOT(ChangeSpeedOfTrain(int))	);
	connect( ui.bAdd		,SIGNAL(clicked()),this,SLOT(AddControlPoint())					);
	connect( ui.bDelete		,SIGNAL(clicked()),this,SLOT(DeleteControlPoint())				);

	connect( ui.rcpxadd		,SIGNAL(clicked()),this,SLOT(RotateControlPointAddX())					);
	connect( ui.rcpxsub		,SIGNAL(clicked()),this,SLOT(RotateControlPointSubX())				);
	connect( ui.rcpzadd		,SIGNAL(clicked()),this,SLOT(RotateControlPointAddZ())					);
	connect( ui.rcpzsub		,SIGNAL(clicked()),this,SLOT(RotateControlPointSubZ())				);

	connect(ui.train_add, SIGNAL(clicked()), this, SLOT(AddTrain()));
	connect(ui.train_sub, SIGNAL(clicked()), this, SLOT(SubTrain()));
	connect(ui.tire, SIGNAL(clicked()), this, SLOT(ToggleTire()));
	connect(ui.tension, SIGNAL(valueChanged(int)), this, SLOT(ChangeTensionOfCurve(int)));
}

AppMain::~AppMain()
{

}

bool AppMain::eventFilter(QObject *watched, QEvent *e) {
	if (e->type() == QEvent::MouseButtonPress) {
		QMouseEvent *event = static_cast<QMouseEvent*> (e);
		// Get the mouse position
		float x, y;
		trainview->arcball.getMouseNDC((float)event->localPos().x(), (float)event->localPos().y(), x,y);

		// Compute the mouse position
		trainview->arcball.down(x, y);
		if(event->button()==Qt::LeftButton){
			trainview->doPick(event->localPos().x(), event->localPos().y());
			this->isHover = true;
			if(this->canpan)
				trainview->arcball.mode = trainview->arcball.Pan;
		}
		if(event->button()==Qt::RightButton){
			trainview->arcball.mode = trainview->arcball.Rotate;
		}
	}

	if (e->type() == QEvent::MouseButtonRelease) {
		this->canpan = false;
		this->isHover = false;
		trainview->arcball.mode = trainview->arcball.None;
	}

	if (e->type() == QEvent::Wheel) {
		QWheelEvent *event = static_cast<QWheelEvent*> (e);
		float zamt = (event->delta() < 0) ? 1.1f : 1/1.1f;
		trainview->arcball.eyeZ *= zamt;
	}

	if (e->type() == QEvent::MouseMove) {
		QMouseEvent *event = static_cast<QMouseEvent*> (e);
		if(isHover && trainview->selectedCube >= 0){
			ControlPoint* cp = &trainview->m_pTrack->points[trainview->selectedCube];

			double r1x, r1y, r1z, r2x, r2y, r2z;
			int x = event->localPos().x();
			int iy = event->localPos().y();
			double mat1[16],mat2[16];		// we have to deal with the projection matrices
			int viewport[4];

			glGetIntegerv(GL_VIEWPORT, viewport);
			glGetDoublev(GL_MODELVIEW_MATRIX,mat1);
			glGetDoublev(GL_PROJECTION_MATRIX,mat2);

			int y = viewport[3] - iy; // originally had an extra -1?

			int i1 = gluUnProject((double) x, (double) y, .25, mat1, mat2, viewport, &r1x, &r1y, &r1z);
			int i2 = gluUnProject((double) x, (double) y, .75, mat1, mat2, viewport, &r2x, &r2y, &r2z);

			double rx, ry, rz;
			mousePoleGo(r1x, r1y, r1z, r2x, r2y, r2z, 
				static_cast<double>(cp->pos.x), 
				static_cast<double>(cp->pos.y),
				static_cast<double>(cp->pos.z),
				rx, ry, rz,
				false);

			cp->pos.x = (float) rx;
			cp->pos.y = (float) ry;
			cp->pos.z = (float) rz;
			setTrackDirty("eventFilter MoveControlPoint");
		}
		if(trainview->arcball.mode != trainview->arcball.None) { // we're taking the drags
			float x,y;
			trainview->arcball.getMouseNDC((float)event->localPos().x(), (float)event->localPos().y(),x,y);
			trainview->arcball.computeNow(x,y);
		};
	}

	if(e->type() == QEvent::KeyPress){
		 QKeyEvent *event = static_cast< QKeyEvent*> (e);
		// Set up the mode
		if (event->key() == Qt::Key_Alt) 
			this->canpan = true;
	}

	return QWidget::eventFilter(watched, e);
}

void AppMain::ExitApp()
{
	QApplication::quit();
}

AppMain * AppMain::getInstance()
{
	if( !Instance )
	{
		Instance = new AppMain();
		return Instance;
	}
	else 
		return Instance;
}

void AppMain::AddTrain()
{
	TrainItem tmp = this->trainview->trainList.back();
	float accuDist = 0.f;
	Pnt3f previous = this->trainview->m_pTrack->samplePoints[tmp.curveIndex][tmp.pointIndex];

	int newPointIndex = tmp.pointIndex - 1;
	int newCurveIndex = tmp.curveIndex;
	while (accuDist < 15.f)
	{
		
		if (newPointIndex < 0)
		{
			newPointIndex = CTrack::DIVIDE_LINE - 1;
			newCurveIndex -= 1;
			if (newCurveIndex < 0)
			{
				newCurveIndex = (int)this->trainview->m_pTrack->samplePoints.size() - 1;
			}
		}
		Pnt3f current = this->trainview->m_pTrack->samplePoints[newCurveIndex][newPointIndex];
		float dx2 = (current.x - previous.x) * (current.x - previous.x);
		float dy2 = (current.y - previous.y) * (current.y - previous.y);
		float dz2 = (current.z - previous.z) * (current.z - previous.z);

		accuDist += sqrt(dx2 + dy2 + dz2);

		previous = current;
		newPointIndex--;
	}
	newPointIndex++;
	TrainItem newItem(newCurveIndex, newPointIndex, false);
	newItem.train = new Model(this->trainview->trainList.front().train);
	this->trainview->trainList.push_back(newItem);
}

void AppMain::SubTrain()
{
	if (this->trainview->trainList.size() == 1u) return;
	else {
		delete this->trainview->trainList.back().train;
		this->trainview->trainList.pop_back();
	}

}

void AppMain::ToggleTire()
{
	this->trainview->isTire = !this->trainview->isTire;
}

void AppMain::ChangeTensionOfCurve(int val)
{
	this->trainview->m_pTrack->dirty = true;
	this->trainview->m_pTrack->tensionScale = 1 + (val - 100.f) / 100.f;
}

void AppMain::ToggleMenuBar()
{
	ui.menuBar->setHidden( !ui.menuBar->isHidden() );
}

void AppMain::ToggleToolBar()
{
	ui.mainToolBar->setHidden( !ui.mainToolBar->isHidden() );
}

void AppMain::ToggleStatusBar()
{
	ui.statusBar->setHidden( !ui.statusBar->isHidden() );
}

void AppMain::LoadTrackPath()
{
	QString fileName = QFileDialog::getOpenFileName( 
		this,
		"OpenImage",
		"./",
		tr("Txt (*.txt)" )
		);
	QByteArray byteArray = fileName.toLocal8Bit();
	const char* fname = byteArray.data();
	if ( !fileName.isEmpty() )
	{
		this->m_Track.readPoints(fname);
	}
	this->trainview->m_pTrack->dirty = true;
}

void AppMain::SaveTrackPath()
{
	QString fileName = QFileDialog::getSaveFileName( 
		this,
		"OpenImage",
		"./",
		tr("Txt (*.txt)" )
		);

	QByteArray byteArray = fileName.toLocal8Bit();
	const char* fname = byteArray.data();
	if ( !fileName.isEmpty() )
	{
		this->m_Track.writePoints(fname);
	}
}

void AppMain::TogglePanel()
{
	if( !ui.groupCamera->isHidden() )
	{
		ui.groupCamera->hide();
		ui.groupCurve->hide();
		ui.groupTrack->hide();
		ui.groupPlay->hide();
		ui.groupCP->hide();
	}
	else
	{
		ui.groupCamera->show();
		ui.groupCurve->show();
		ui.groupTrack->show();
		ui.groupPlay->show();
		ui.groupCP->show();
	}
}

void AppMain::ChangeCameraType( QString type )
{
	if( type == "World" )
	{
		this->trainview->camera = 0;
		update();
	}
	else if( type == "Top" )
	{
		this->trainview->camera = 1;
		update();
	}
	else if( type == "Train" )
	{
		this->trainview->camera = 2;
		update();
	}
}

void AppMain::ChangeCurveType( QString type )
{
	if( type == "Linear" )
	{
		setTrackDirty("ChangeCurveType Linear");
		this->trainview->m_pTrack->splineType = LINE;
	}
	else if( type == "Cardinal" )
	{
		setTrackDirty("ChangeCurveType Cardinal");
		this->trainview->m_pTrack->splineType = CARDINAL;
	}
	else if( type == "Cubic" )
	{
		setTrackDirty("ChangeCurveType Cubic");
		this->trainview->m_pTrack->splineType = B_SPLINE;
	}


}

void AppMain::ChangeTrackType( QString type )
{
	if( type == "Line" )
	{
		this->trainview->track = 0;
	}
	else if( type == "Track" )
	{
		this->trainview->track = 1;
	}
	else if( type == "Road" )
	{
		this->trainview->track = 2;
	}
}

static unsigned long lastRedraw = 0;
void AppMain::SwitchPlayAndPause()
{
	if( !this->trainview->isrun )
	{
		ui.bPlay->setIcon(QIcon(":/AppMain/Resources/Icons/play.ico"));
		this->trainview->isrun = !this->trainview->isrun;
	}
	else
	{
		ui.bPlay->setIcon(QIcon(":/AppMain/Resources/Icons/pause.ico"));
		this->trainview->isrun = !this->trainview->isrun;
	}
	if(this->trainview->isrun){
		if (clock() - lastRedraw > CLOCKS_PER_SEC/30) {
			lastRedraw = clock();
			this->advanceTrain();
			this->damageMe();
		}
	}
}

void AppMain::ChangeSpeedOfTrain( int val )
{
	this->trainview->speed = 20.f + val * 0.8f;
}

void AppMain::AddControlPoint()
{
	setTrackDirty("AddControlPoint");
	// get the number of points
	size_t npts = this->m_Track.points.size();
	// the number for the new point
	size_t newidx = (this->trainview->selectedCube>=0) ? this->trainview->selectedCube : 0;

	// pick a reasonable location
	size_t previdx = (newidx + npts -1) % npts;
	Pnt3f npos = (this->m_Track.points[previdx].pos + this->m_Track.points[newidx].pos) * .5f;

	this->m_Track.points.insert(this->m_Track.points.begin() + newidx,npos);

	// make it so that the train doesn't move - unless its affected by this control point
	// it should stay between the same points
	if (ceil(this->m_Track.trainU) > ((float)newidx)) {
		this->m_Track.trainU += 1;
		if (this->m_Track.trainU >= npts) this->m_Track.trainU -= npts;
	}
	this->damageMe();
}

void AppMain::DeleteControlPoint()
{
	setTrackDirty("DeleteControlPoint");
	if (this->m_Track.points.size() > 4) {
		if (this->trainview->selectedCube >= 0) {
			this->m_Track.points.erase(this->m_Track.points.begin() + this->trainview->selectedCube);
		} else
			this->m_Track.points.pop_back();
	}
	this->damageMe();
}


//***************************************************************************
//
// * Rotate the selected control point about x axis
//===========================================================================
void AppMain::rollx(float dir)
{
	int s = this->trainview->selectedCube;
	if (s >= 0) {
		Pnt3f old = this->m_Track.points[s].orient;
		float si = sin(((float)M_PI_4) * dir);
		float co = cos(((float)M_PI_4) * dir);
		this->m_Track.points[s].orient.y = co * old.y - si * old.z;
		this->m_Track.points[s].orient.z = si * old.y + co * old.z;
	}
	this->damageMe();
} 

void AppMain::RotateControlPointAddX()
{
	setTrackDirty("RotateControlPointAddX");
	rollx(1);
}

void AppMain::RotateControlPointSubX()
{
	setTrackDirty("RotateControlPointSubX");
	rollx(-1);
}

void AppMain::rollz(float dir)
{
	int s = this->trainview->selectedCube;
	if (s >= 0) {

		Pnt3f old = this->m_Track.points[s].orient;

		float si = sin(((float)M_PI_4) * dir);
		float co = cos(((float)M_PI_4) * dir);

		this->m_Track.points[s].orient.y = co * old.y - si * old.x;
		this->m_Track.points[s].orient.x = si * old.y + co * old.x;
	}
	this->damageMe();
} 

void AppMain::RotateControlPointAddZ()
{
	setTrackDirty("RotateControlPointAddZ");
	rollz(1);
}

void AppMain::RotateControlPointSubZ()
{
	setTrackDirty("RotateControlPointSubZ");
	rollz(-1);
}

void AppMain::ChangeCamToWorld()
{
	this->trainview->camera = 0;
}

void AppMain::ChangeCamToTop()
{
	this->trainview->camera = 1;
}

void AppMain::ChangeCamToTrain()
{
	this->trainview->camera = 2;
}

void AppMain::ChangeCurveToLinear()
{
	setTrackDirty("ChangeCurveToLinear");
	this->trainview->curve = 0;
}

void AppMain::ChangeCurveToCardinal()
{
	setTrackDirty("ChangeCurveToCardinal");
	this->trainview->curve = 1;
}

void AppMain::ChangeCurveToCubic()
{
	setTrackDirty("ChangeCurveToCubic");
	this->trainview->curve = 2;
}

void AppMain::ChangeTrackToLine()
{
	this->trainview->track = 0;
}

void AppMain::ChangeTrackToTrack()
{
	this->trainview->track = 1;
}

void AppMain::ChangeTrackToRoad()
{
	this->trainview->track = 2;
}

void AppMain::UpdateCameraState( int index )
{
	ui.aWorld->setChecked( (index==0)?true:false );
	ui.aTop	 ->setChecked( (index==1)?true:false );
	ui.aTrain->setChecked( (index==2)?true:false );
}

void AppMain::UpdateCurveState( int index )
{
	ui.aLinear	->setChecked( (index==0)?true:false );
	ui.aCardinal->setChecked( (index==1)?true:false );
	ui.aCubic	->setChecked( (index==2)?true:false );
}

void AppMain::UpdateTrackState( int index )
{
	ui.aLine ->setChecked( (index==0)?true:false );
	ui.aTrack->setChecked( (index==1)?true:false );
	ui.aRoad ->setChecked( (index==2)?true:false );
}

//************************************************************************
//
// *
//========================================================================
void AppMain::
damageMe()
//========================================================================
{
	if (trainview->selectedCube >= ((int)m_Track.points.size()))
		trainview->selectedCube = 0;
	//trainview->damage(1);
}

//************************************************************************
//
// * This will get called (approximately) 30 times per second
//   if the run button is pressed
//========================================================================
void AppMain::
advanceTrain(float dir)
//========================================================================
{
	//#####################################################################
	// TODO: make this work for your train
	//#####################################################################
	if (this->trainview->isrun) {
		

		if (this->trainview->isrun) {
			if (clock() - lastRedraw > CLOCKS_PER_SEC / 30) {
				lastRedraw = clock();

				this->damageMe();
				float targetDistance = this->trainview->speed / 30.f;
				
				for (TrainItem &item : this->trainview->trainList)
				{
					float accuDist = 0.f;
					int newPointIndex = item.pointIndex, newCurveIndex = item.curveIndex;
					Pnt3f previous = this->trainview->m_pTrack->samplePoints[newCurveIndex][newPointIndex];
					while (accuDist < targetDistance)
					{

						if (newPointIndex > CTrack::DIVIDE_LINE - 1)
						{
							newPointIndex = 0;
							newCurveIndex += 1;
							if (newCurveIndex > (int)this->trainview->m_pTrack->samplePoints.size() - 1)
							{
								newCurveIndex = 0;
							}
						}
						Pnt3f current = this->trainview->m_pTrack->samplePoints[newCurveIndex][newPointIndex];
						float dx2 = (current.x - previous.x) * (current.x - previous.x);
						float dy2 = (current.y - previous.y) * (current.y - previous.y);
						float dz2 = (current.z - previous.z) * (current.z - previous.z);

						accuDist += sqrt(dx2 + dy2 + dz2);

						previous = current;
						newPointIndex++;
					}
					newPointIndex--;
					item.curveIndex = newCurveIndex;
					item.pointIndex = newPointIndex;
				}

				float deltaAngle = 360.f * this->trainview->speed / 30.f / (6.28318*this->trainview->tire->radius);
				this->trainview->tire->angle -= deltaAngle;
				if (this->trainview->tire->angle < 0.f)
					this->trainview->tire->angle += 360.f;
			}
		}
	}
	
	/*glMatrixMode(GL_PROJECTION);
	gluPerspective(120, 1, 1, 200); glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	matrix4_look.normalize();
	matrix2_look.normalize();
	gluLookAt(matrix1_look.x, matrix1_look.y + 5,
		matrix1_look.z, matrix1_look.x + matrix4_look.x,
		matrix1_look.y + matrix4_look.y + 5,
		matrix1_look.z + matrix4_look.z, matrix2_look.x, matrix2_look.y + 5, matrix2_look.z);*/


}

void AppMain::
setTrackDirty(const char * msg)
{
	//system("CLS");
	//cout << msg << endl;
	this->trainview->m_pTrack->dirty = true;
	
}