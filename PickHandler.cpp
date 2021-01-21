#include "PickHandler.h"

#include <osg/io_utils>
#include <osgUtil/IntersectionVisitor>
#include <osgUtil/LineSegmentIntersector>
#include <osgViewer/Viewer>
#include <iostream>

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgUtil/PolytopeIntersector>
#include <osg/Camera>
#include <osg/NodeCallback>
#include <osg/Group>
#include <osg/MatrixTransform>
#include <iostream>
#include <osg/Notify>

osg::ref_ptr<osg::Node> _selectedNode;

// Derive a class from NodeCallback to manipulate a
//   MatrixTransform object's matrix.
class RotateCB : public osg::NodeCallback
{
public:
    RotateCB() : _angle( 0. ) {}

    virtual void operator()( osg::Node* node,
            osg::NodeVisitor* nv )
    {
        // Normally, check to make sure we have an update
        //   visitor, not necessary in this simple example.
        osg::MatrixTransform* mt =
                dynamic_cast<osg::MatrixTransform*>( node );
        osg::Matrix m;
        m.makeRotate( _angle, osg::Vec3( 0., 0., 1. ) );
        mt->setMatrix( m );

        // Increment the angle for the next from.
        _angle += 0.01;

        // Continue traversing so that OSG can process
        //   any other nodes with callbacks.
        traverse( node, nv );
    }

protected:
    double _angle;
};

PickHandler::PickHandler() : _mX( 0. ),_mY( 0. ) {}

PickHandler::~PickHandler()
{
}

bool PickHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
  if( ea.getEventType() != osgGA::GUIEventAdapter::RELEASE &&
      ea.getButton()    != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON )
  {
    return false;
  }

  osgViewer::Viewer* viewer =
          dynamic_cast<osgViewer::Viewer*>( &aa );
  if (!viewer)
      return( false );

  switch( ea.getEventType() )
  {
      case osgGA::GUIEventAdapter::PUSH:
      case osgGA::GUIEventAdapter::MOVE:
      {
          // Record mouse location for the button press
          //   and move events.
          _mX = ea.getX();
          _mY = ea.getY();
          return( false );
      }
      case osgGA::GUIEventAdapter::RELEASE:
      {
          // If the mouse hasn't moved since the last
          //   button press or move event, perform a
          //   pick. (Otherwise, the trackball
          //   manipulator will handle it.)
          if (_mX == ea.getX() && _mY == ea.getY())
          {
              if (pick( ea.getXnormalized(),
                          ea.getYnormalized(), viewer ))
                  return( true );
          }
          return( false );
      }

      default:
          return( false );
  }

  return true;
}

// Perform a pick operation.
bool PickHandler::pick( const double x, const double y, osgViewer::Viewer *viewer )
{
    if (!viewer->getSceneData())
        // Nothing to pick.
        return( false );

    double w( .05 ), h( .05 );
    osgUtil::PolytopeIntersector* picker =
            new osgUtil::PolytopeIntersector(
                osgUtil::Intersector::PROJECTION,
                    x-w, y-h, x+w, y+h );

    osgUtil::IntersectionVisitor iv( picker );
    viewer->getCamera()->accept( iv );

    if (picker->containsIntersections())
    {
        const osg::NodePath& nodePath =
                picker->getFirstIntersection().nodePath;
        unsigned int idx = nodePath.size();
        while (idx--)
        {
            // Find the LAST MatrixTransform in the node
            //   path; this will be the MatrixTransform
            //   to attach our callback to.
            osg::MatrixTransform* mt =
                    dynamic_cast<osg::MatrixTransform*>(
                        nodePath[ idx ] );
            if (mt == NULL)
                continue;

            // If we get here, we just found a
            //   MatrixTransform in the nodePath.

            if (_selectedNode.valid())
                // Clear the previous selected node's
                //   callback to make it stop spinning.
                _selectedNode->setUpdateCallback( NULL );

            _selectedNode = mt;
            _selectedNode->setUpdateCallback( new RotateCB );
            break;
        }
        if (!_selectedNode.valid())
            osg::notify() << "Pick failed." << std::endl;
    }
    else if (_selectedNode.valid())
    {
        _selectedNode->setUpdateCallback( NULL );
        _selectedNode = NULL;
    }
    return( _selectedNode.valid() );
}
