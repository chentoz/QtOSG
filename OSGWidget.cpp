#include "OSGWidget.h"
#include "PickHandler.h"
#include "DragHandler.h"

#include <osg/Camera>
#include <osg/DisplaySettings>
#include <osg/Geode>
#include <osg/Material>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/StateSet>
#include <osgDB/WriteFile>
#include <osgGA/EventQueue>
#include <osgGA/TrackballManipulator>
#include <osgUtil/IntersectionVisitor>
#include <osgUtil/PolytopeIntersector>
#include <osgViewer/View>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/PositionAttitudeTransform>
#include <osgGA/TrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgUtil/Optimizer>
#include <osg/CoordinateSystemNode>
#include <osg/Switch>
#include <osg/Types>
#include <osgText/Text>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>
#include <osgGA/SphericalManipulator>
#include <osgGA/Device>
#include <osg/MatrixTransform>

#include <cassert>
#include <stdexcept>
#include <vector>

#include <QDebug>
#include <QKeyEvent>
#include <QPainter>
#include <QWheelEvent>

namespace
{

#ifdef WITH_SELECTION_PROCESSING
QRect makeRectangle( const QPoint& first, const QPoint& second )
{
  // Relative to the first point, the second point may be in either one of the
  // four quadrants of an Euclidean coordinate system.
  //
  // We enumerate them in counter-clockwise order, starting from the lower-right
  // quadrant that corresponds to the default case:
  //
  //            |
  //       (3)  |  (4)
  //            |
  //     -------|-------
  //            |
  //       (2)  |  (1)
  //            |

  if( second.x() >= first.x() && second.y() >= first.y() )
    return QRect( first, second );
  else if( second.x() < first.x() && second.y() >= first.y() )
    return QRect( QPoint( second.x(), first.y() ), QPoint( first.x(), second.y() ) );
  else if( second.x() < first.x() && second.y() < first.y() )
    return QRect( second, first );
  else if( second.x() >= first.x() && second.y() < first.y() )
    return QRect( QPoint( first.x(), second.y() ), QPoint( second.x(), first.y() ) );

  // Should never reach that point...
  return QRect();
}
#endif

}

namespace osgWidget
{
  void Viewer::setupThreading()
  {
    if( _threadingModel == SingleThreaded )
    {
      if(_threadsRunning)
        stopThreading();
    }
    else
    {
      if(!_threadsRunning)
        startThreading();
    }
  }
}

OSGWidget::OSGWidget( QWidget* parent,
                      Qt::WindowFlags f )
  : QOpenGLWidget( parent,
                   f )
  , graphicsWindow_( new osgViewer::GraphicsWindowEmbedded( this->x(),
                                                            this->y(),
                                                            this->width(),
                                                            this->height() ) )
  , viewer_( new osgWidget::Viewer )
  , selectionActive_( false )
  , selectionFinished_( true )
{
  // terrian data
  osg::ref_ptr<osg::Group> root(new osg::Group);

  osg::Node* terrain = osgDB::readNodeFile("C:\\Users\\Lenovo\\Desktop\\OSG\\data\\OpenSceneGraph-Data\\cessna.osg");
  if (terrain == nullptr) {
      return;
  }

  osg::ref_ptr<osg::PositionAttitudeTransform> terrainT = new osg::PositionAttitudeTransform();
  // first object
  auto translateMT = new osg::MatrixTransform;
  translateMT->setMatrix(osg::Matrix::translate(100, 0, 0));
  translateMT->addChild(terrain);
  // second object
  root->addChild(terrain);
  root->addChild(translateMT);

  // transform
  //float aspectRatio = static_cast<float>( this->width() / 2 ) / static_cast<float>( this->height() );
  float aspectRatio = static_cast<float>( this->width() ) / static_cast<float>( this->height() );
  auto pixelRatio   = this->devicePixelRatio();

  // camera
  osg::Camera* mainCamera = new osg::Camera;
  //mainCamera->setViewport( 0, 0, this->width() / 2 * pixelRatio, this->height() * pixelRatio );
  mainCamera->setViewport( 0, 0, this->width() * pixelRatio, this->height() * pixelRatio );
  mainCamera->setClearColor( osg::Vec4( 0.f, 0.f, 1.f, 1.f ) );
  mainCamera->setProjectionMatrixAsPerspective( 30.f, aspectRatio, 1.f, 1000.f );
  mainCamera->setGraphicsContext( graphicsWindow_ );

  // view
  osgViewer::View* mainView = new osgViewer::View;
  mainView->setCamera( mainCamera );
  mainView->setSceneData(root.get());

  try
  {
      // set up the camera manipulators.
      osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

//      keyswitchManipulator->addMatrixManipulator('1', "Trackball", new osgGA::TrackballManipulator());
//      keyswitchManipulator->addMatrixManipulator('2', "Flight", new osgGA::FlightManipulator());
//      keyswitchManipulator->addMatrixManipulator('3', "Drive", new osgGA::DriveManipulator());
//      keyswitchManipulator->addMatrixManipulator('4', "Terrain", new osgGA::TerrainManipulator());
      keyswitchManipulator->addMatrixManipulator('5', "Orbit", new osgGA::OrbitManipulator());
//      keyswitchManipulator->addMatrixManipulator('6', "FirstPerson", new osgGA::FirstPersonManipulator());
//      keyswitchManipulator->addMatrixManipulator('7', "Spherical", new osgGA::SphericalManipulator());

      std::string pathfile;
      mainView->setCameraManipulator(keyswitchManipulator.get());

      // add the state manipulator
      //mainView->addEventHandler(new osgGA::StateSetManipulator(mainView->getCamera()->getOrCreateStateSet()));

      // add the thread model handler
      mainView->addEventHandler(new osgViewer::ThreadingHandler);

      // add the window size toggle handler
      mainView->addEventHandler(new osgViewer::WindowSizeHandler);

      // add the stats handler
      mainView->addEventHandler(new osgViewer::StatsHandler);

      // add the record camera path handler
      mainView->addEventHandler(new osgViewer::RecordCameraPathHandler);

      // add the LOD Scale handler
      mainView->addEventHandler(new osgViewer::LODScaleHandler);

      // add the screen capture handler
      mainView->addEventHandler(new osgViewer::ScreenCaptureHandler);

//      manipulator->setAllowThrow( false );
  }
  catch (...) {
      std::cout << "Failed to init cameras" << std::endl;
  }

#ifdef WITH_PICK_HANDLER
  // mainView->addEventHandler( new PickHandler() );
#endif

  viewer_->addView( mainView );
  viewer_->setThreadingModel(osgViewer::CompositeViewer::SingleThreaded);
  viewer_->realize();

  // This ensures that the widget will receive keyboard events. This focus
  // policy is not set by default. The default, Qt::NoFocus, will result in
  // keyboard events that are ignored.
  this->setFocusPolicy( Qt::StrongFocus );
  this->setMinimumSize( 100, 100 );

  // Ensures that the widget receives mouse move events even though no
  // mouse button has been pressed. We require this in order to let the
  // graphics window switch viewports properly.
  this->setMouseTracking( true );
}

OSGWidget::~OSGWidget()
{
}

void OSGWidget::paintEvent( QPaintEvent* /* paintEvent */ )
{
  this->makeCurrent();

  QPainter painter( this );
  painter.setRenderHint( QPainter::Antialiasing );

  this->paintGL();

#ifdef WITH_SELECTION_PROCESSING
  if( selectionActive_ && !selectionFinished_ )
  {
    painter.setPen( Qt::black );
    painter.setBrush( Qt::transparent );
    painter.drawRect( makeRectangle( selectionStart_, selectionEnd_ ) );
  }
#endif

  painter.end();

  this->doneCurrent();
}

void OSGWidget::paintGL()
{
  viewer_->frame();
}

void OSGWidget::resizeGL( int width, int height )
{
  this->getEventQueue()->windowResize( this->x(), this->y(), width, height );
  graphicsWindow_->resized( this->x(), this->y(), width, height );

  this->onResize( width, height );
}

void OSGWidget::keyPressEvent( QKeyEvent* event )
{
  QString keyString   = event->text();
  const char* keyData = keyString.toLocal8Bit().data();

  if( event->key() == Qt::Key_S )
  {
#ifdef WITH_SELECTION_PROCESSING
    selectionActive_ = !selectionActive_;
#endif

    // Further processing is required for the statistics handler here, so we do
    // not return right away.
  }
  else if( event->key() == Qt::Key_D )
  {
    osgDB::writeNodeFile( *viewer_->getView(0)->getSceneData(),
                          "/tmp/sceneGraph.osg" );

    return;
  }
  else if( event->key() == Qt::Key_H )
  {
    this->onHome();
    return;
  }

  this->getEventQueue()->keyPress( osgGA::GUIEventAdapter::KeySymbol( *keyData ) );
}

void OSGWidget::keyReleaseEvent( QKeyEvent* event )
{
  QString keyString   = event->text();
  const char* keyData = keyString.toLocal8Bit().data();

  this->getEventQueue()->keyRelease( osgGA::GUIEventAdapter::KeySymbol( *keyData ) );
}

void OSGWidget::mouseMoveEvent( QMouseEvent* event )
{
  // Note that we have to check the buttons mask in order to see whether the
  // left button has been pressed. A call to `button()` will only result in
  // `Qt::NoButton` for mouse move events.
  if( selectionActive_ && event->buttons() & Qt::LeftButton )
  {
    selectionEnd_ = event->pos();

    // Ensures that new paint events are created while the user moves the
    // mouse.
    this->update();
  }
  else
  {
    auto pixelRatio = this->devicePixelRatio();

    this->getEventQueue()->mouseMotion( static_cast<float>( event->x() * pixelRatio ),
                                        static_cast<float>( event->y() * pixelRatio ) );
  }
}

void OSGWidget::mousePressEvent( QMouseEvent* event )
{
  // Selection processing
  if( selectionActive_ && event->button() == Qt::LeftButton )
  {
    selectionStart_    = event->pos();
    selectionEnd_      = selectionStart_; // Deletes the old selection
    selectionFinished_ = false;           // As long as this is set, the rectangle will be drawn
  }

  // Normal processing
  else
  {
    // 1 = left mouse button
    // 2 = middle mouse button
    // 3 = right mouse button

    unsigned int button = 0;

    switch( event->button() )
    {
    case Qt::LeftButton:
      button = 1;
      break;

    case Qt::MiddleButton:
      button = 2;
      break;

    case Qt::RightButton:
      button = 3;
      break;

    default:
      break;
    }

    auto pixelRatio = this->devicePixelRatio();

    this->getEventQueue()->mouseButtonPress( static_cast<float>( event->x() * pixelRatio ),
                                             static_cast<float>( event->y() * pixelRatio ),
                                             button );
    }
}

void OSGWidget::mouseReleaseEvent(QMouseEvent* event)
{
  // Selection processing: Store end position and obtain selected objects
  // through polytope intersection.
  if( selectionActive_ && event->button() == Qt::LeftButton )
  {
    selectionEnd_      = event->pos();
    selectionFinished_ = true; // Will force the painter to stop drawing the
                               // selection rectangle

    this->processSelection();
  }

  // Normal processing
  else
  {
    // 1 = left mouse button
    // 2 = middle mouse button
    // 3 = right mouse button

    unsigned int button = 0;

    switch( event->button() )
    {
    case Qt::LeftButton:
      button = 1;
      break;

    case Qt::MiddleButton:
      button = 2;
      break;

    case Qt::RightButton:
      button = 3;
      break;

    default:
      break;
    }

    auto pixelRatio = this->devicePixelRatio();

    this->getEventQueue()->mouseButtonRelease( static_cast<float>( pixelRatio * event->x() ),
                                               static_cast<float>( pixelRatio * event->y() ),
                                               button );
  }
}

void OSGWidget::wheelEvent( QWheelEvent* event )
{
  // Ignore wheel events as long as the selection is active.
  if( selectionActive_ )
    return;

  event->accept();
  int delta = event->delta();

  osgGA::GUIEventAdapter::ScrollingMotion motion = delta > 0 ?   osgGA::GUIEventAdapter::SCROLL_UP
                                                               : osgGA::GUIEventAdapter::SCROLL_DOWN;

  this->getEventQueue()->mouseScroll( motion );
}

bool OSGWidget::event( QEvent* event )
{
  bool handled = QOpenGLWidget::event( event );

  // This ensures that the OSG widget is always going to be repainted after the
  // user performed some interaction. Doing this in the event handler ensures
  // that we don't forget about some event and prevents duplicate code.
  switch( event->type() )
  {
  case QEvent::KeyPress:
  case QEvent::KeyRelease:
  case QEvent::MouseButtonDblClick:
  case QEvent::MouseButtonPress:
  case QEvent::MouseButtonRelease:
  case QEvent::MouseMove:
  case QEvent::Wheel:
    this->update();
    break;

  default:
    break;
  }

  return handled;
}

void OSGWidget::onHome()
{
  osgViewer::ViewerBase::Views views;
  viewer_->getViews( views );

  for( std::size_t i = 0; i < views.size(); i++ )
  {
    osgViewer::View* view = views.at(i);
    view->home();
  }
}

void OSGWidget::onResize( int width, int height )
{
  std::vector<osg::Camera*> cameras;
  viewer_->getCameras( cameras );

//  assert( cameras.size() == 2 );

  auto pixelRatio = this->devicePixelRatio();

  //cameras[0]->setViewport( 0, 0, width / 2 * pixelRatio, height * pixelRatio );
  cameras[0]->setViewport( 0, 0, width * pixelRatio, height * pixelRatio );
  //cameras[1]->setViewport( width / 2 * pixelRatio, 0, width / 2 * pixelRatio, height * pixelRatio );
}

osgGA::EventQueue* OSGWidget::getEventQueue() const
{
  osgGA::EventQueue* eventQueue = graphicsWindow_->getEventQueue();

  if( eventQueue )
    return eventQueue;
  else
    throw std::runtime_error( "Unable to obtain valid event queue");
}

void OSGWidget::processSelection()
{
#ifdef WITH_SELECTION_PROCESSING
  QRect selectionRectangle = makeRectangle( selectionStart_, selectionEnd_ );
  auto widgetHeight        = this->height();
  auto pixelRatio          = this->devicePixelRatio();

  double xMin = selectionRectangle.left();
  double xMax = selectionRectangle.right();
  double yMin = widgetHeight - selectionRectangle.bottom();
  double yMax = widgetHeight - selectionRectangle.top();

  xMin *= pixelRatio;
  yMin *= pixelRatio;
  xMax *= pixelRatio;
  yMax *= pixelRatio;

  osgUtil::PolytopeIntersector* polytopeIntersector
      = new osgUtil::PolytopeIntersector( osgUtil::PolytopeIntersector::WINDOW,
                                          xMin, yMin,
                                          xMax, yMax );

  // This limits the amount of intersections that are reported by the
  // polytope intersector. Using this setting, a single drawable will
  // appear at most once while calculating intersections. This is the
  // preferred and expected behaviour.
  polytopeIntersector->setIntersectionLimit( osgUtil::Intersector::LIMIT_ONE_PER_DRAWABLE );

  osgUtil::IntersectionVisitor iv( polytopeIntersector );

  for( unsigned int viewIndex = 0; viewIndex < viewer_->getNumViews(); viewIndex++ )
  {
    qDebug() << "View index:" << viewIndex;

    osgViewer::View* view = viewer_->getView( viewIndex );

    if( !view )
      throw std::runtime_error( "Unable to obtain valid view for selection processing" );

    osg::Camera* camera = view->getCamera();

    if( !camera )
      throw std::runtime_error( "Unable to obtain valid camera for selection processing" );

    camera->accept( iv );

    if( !polytopeIntersector->containsIntersections() )
      continue;

    auto intersections = polytopeIntersector->getIntersections();

    for( auto&& intersection : intersections )
      qDebug() << "Selected a drawable:" << QString::fromStdString( intersection.drawable->getName() );
  }
#endif
}
