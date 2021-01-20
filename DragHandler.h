#ifndef MOUSEDRAGHANDLER_H
#define MOUSEDRAGHANDLER_H

#include <osgGA/GUIEventHandler>
#include <osgViewer/View>

class MouseDragHandler : public osgGA::GUIEventHandler
{
public:
    MouseDragHandler( double devicePixelRatio = 1.0 );
    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
    void pick(float x, float y);
    osg::Vec3 screen2World(float x, float y);
    bool PickObject;
    osg::ref_ptr<osg::MatrixTransform> picked;
    osgViewer::View* view;
protected:
    virtual ~MouseDragHandler();
private:
    double devicePixelRatio;
};

#endif // MOUSEDRAGHANDLER_H
