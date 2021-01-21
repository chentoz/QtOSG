#ifndef PickHandler_h__
#define PickHandler_h__

#include <osgGA/GUIEventHandler>
#include <osgViewer/Viewer>

class PickHandler : public osgGA::GUIEventHandler
{
public:
  PickHandler();
  virtual ~PickHandler();

  virtual bool handle( const osgGA::GUIEventAdapter&  ea,
                             osgGA::GUIActionAdapter& aa );

protected:
    // Store mouse xy location for button press & move events.
    float _mX, _mY;

private:
  bool pick( const double x, const double y,
            osgViewer::Viewer *viewer );

  double devicePixelRatio_;
};

#endif
