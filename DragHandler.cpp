#include "DragHandler.h"
#include <osg/io_utils>

#include <osgUtil/IntersectionVisitor>
#include <osgUtil/LineSegmentIntersector>

#include <osgViewer/Viewer>
#include <osg/MatrixTransform>
#include <osg/Point>
#include <iostream>

MouseDragHandler::MouseDragHandler(double devicePixelRatio)
    :devicePixelRatio(devicePixelRatio)
{
}

void MouseDragHandler::pick(float x, float y)
{
    osgUtil::LineSegmentIntersector::Intersections intersections;
    if (view->computeIntersections(x, y, intersections))
    {
        osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();
        osg::NodePath getNodePath = hitr->nodePath;
        for (int i = getNodePath.size() - 1; i >= 0; --i)
        {
            osg::MatrixTransform* mt = dynamic_cast<osg::MatrixTransform*>(getNodePath[i]);
            if (mt == NULL)
            {
                continue;
            }
            else
            {
                PickObject = true;
                picked = mt;
            }

        }
    }
    else
    {
        PickObject = false;
    }
}

osg::Vec3 MouseDragHandler::screen2World(float x, float y)
{
    osg::Vec3 vec3;
    osg::ref_ptr<osg::Camera> camera = view->getCamera();
    osg::Vec3 vScreen(x, y, 0);
    osg::Matrix mVPW = camera->getViewMatrix() * camera->getProjectionMatrix() * camera->getViewport()->computeWindowMatrix();
    osg::Matrix invertVPW;
    invertVPW.invert(mVPW);
    vec3 = vScreen * invertVPW;
    return vec3;
}

MouseDragHandler::~MouseDragHandler()
{

}

bool MouseDragHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    view = dynamic_cast<osgViewer::View*>(&aa);
    if (!view) return false;

    bool lbuttonDown = false;
    osg::Vec2 last_point, first_point;
    osg::Matrix originPos;

    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::PUSH:
    {
        if (view)
        {
            int button = ea.getButton();
            if (button == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
            {
                lbuttonDown = true;
                pick(ea.getX(), ea.getY());
                if (PickObject)
                {
                    osg::Vec3 vec1 = screen2World(ea.getX(), ea.getY());
                    first_point = { vec1.x(), vec1.z() };
                    originPos = picked->getMatrix();
                }
            }
            else
            {
                lbuttonDown = false;
            }

        }
        return false;
    }

    case  osgGA::GUIEventAdapter::DRAG:
    {
        if (PickObject&&lbuttonDown)
        {
            osg::Vec3 vec2 = screen2World(ea.getX(), ea.getY());
            last_point = { vec2.x(), vec2.z() };
            float dx = last_point.x() - first_point.y();
            float dy = last_point.x() - first_point.y();
            std::cout<< dx << "  " << dy << std::endl;
            if (fabs(dx) + fabs(dy) < 0.06)
                return false;
            picked->setMatrix(originPos*osg::Matrix::translate(dx, 0, dy));
        }
        return false;
    }
    case osgGA::GUIEventAdapter::RELEASE:
    {
        PickObject = false;
        return false;
    }
    default:
        return false;
    }
}
