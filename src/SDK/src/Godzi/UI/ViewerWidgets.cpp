/* --*-c++-*-- */
/**
 * Godzi
 * Copyright 2010 Pelican Mapping
 * http://pelicanmapping.com
 * http://github.com/gwaldron/godzi
 *
 * Godzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 */
#include <Godzi/UI/ViewerWidgets>
#include <QtGui/QDesktopServices>
#include <osgEarthUtil/AutoClipPlaneHandler>

#define USE_QT4

using namespace Godzi;
using namespace Godzi::UI;
using namespace osgEarth::Util;


ViewerWidget::ViewerWidget( QWidget* parent, const char* name, WindowFlags f, bool overrideTraits) :
QWidget(parent, f),
_overrideTraits (overrideTraits)
{
    // set up the OpenGL context for an OSG window
    createContext();

    // configure Qt to render the OSG window
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setFocusPolicy(Qt::ClickFocus);
}

EarthManipulator*
ViewerWidget::getManipulator() const
{
    return static_cast<EarthManipulator*>(_viewer->getCameraManipulator());
}

void
ViewerWidget::createContext()
{
    osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();

    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;

    traits->readDISPLAY();
    if (traits->displayNum<0) traits->displayNum = 0;

    traits->windowName = "osgViewerQt";
    traits->screenNum = 0;
    traits->x = x();
    traits->y = y();
    traits->width = width();
    traits->height = height();
    traits->alpha = ds->getMinimumNumAlphaBits();
    traits->stencil = ds->getMinimumNumStencilBits();
    traits->windowDecoration = false;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;
    traits->sampleBuffers = ds->getMultiSamples();
    traits->samples = ds->getNumMultiSamples();

#if defined(__APPLE__) 
    // Extract a WindowPtr from the HIViewRef that QWidget::winId() returns.
    // Without this change, the peer tries to call GetWindowPort on the HIViewRef
    // which returns 0 and we only render white.
    traits->inheritedWindowData = new WindowData(HIViewGetWindow((HIViewRef)winId()));

#else // all others
    traits->inheritedWindowData = new WindowData(winId());
#endif


    if (ds->getStereo())
    {
        switch(ds->getStereoMode())
        {
        case(osg::DisplaySettings::QUAD_BUFFER): traits->quadBufferStereo = true; break;
        case(osg::DisplaySettings::VERTICAL_INTERLACE):
        case(osg::DisplaySettings::CHECKERBOARD):
        case(osg::DisplaySettings::HORIZONTAL_INTERLACE): traits->stencil = 8; break;
        default: break;
        }
    }

    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    _gw = dynamic_cast<osgViewer::GraphicsWindow*>(gc.get());

    // get around dearanged traits on X11 (MTCompositeViewer only)
    if (_overrideTraits)
    {
        traits->x = x();
        traits->y = y();
        traits->width = width();
        traits->height = height();
    }

    _viewer = new osgViewer::Viewer();
    _viewer->addEventHandler( new osgEarth::Util::AutoClipPlaneHandler() );
    _viewer->setCameraManipulator( new osgEarth::Util::EarthManipulator() );
    _viewer->setThreadingModel(osgViewer::Viewer::DrawThreadPerContext);
    _viewer->getCamera()->setGraphicsContext( _gw );
    _viewer->getCamera()->setProjectionMatrixAsPerspective( 30.0f, (double)traits->width/(double)traits->height, 1.0, 1000.0 );
    _viewer->getCamera()->setViewport( new osg::Viewport(0,0,traits->width,traits->height) );
    _viewer->addEventHandler( new osgViewer::StatsHandler() );
    _viewer->addEventHandler( new osgGA::StateSetManipulator() );
    _viewer->addEventHandler( new osgViewer::ThreadingHandler() );

    _viewer->setKeyEventSetsDone( 0 );
    _viewer->setQuitEventSetsDone( false );
    
     connect( &_timer, SIGNAL(timeout()), this, SLOT(repaint()) );
     _timer.start( 15 );
}

#ifndef WIN32 

void
ViewerWidget::destroyEvent(bool destroyWindow, bool destroySubWindows)
{   
    _gw->getEventQueue()->closeWindow();
}


void 
ViewerWidget::closeEvent( QCloseEvent * event )
{
#ifndef USE_QT4
    event->accept();
#endif

    _gw->getEventQueue()->closeWindow();
}


void 
ViewerWidget::resizeEvent( QResizeEvent * event )
{
    const QSize & size = event->size();
    _gw->getEventQueue()->windowResize(0, 0, size.width(), size.height() );
    _gw->resized(0, 0, size.width(), size.height());
}

void
ViewerWidget::keyPressEvent( QKeyEvent* event )
{
#ifdef USE_QT4
    _gw->getEventQueue()->keyPress( (osgGA::GUIEventAdapter::KeySymbol) *(event->text().toAscii().data() ) );
#else
    _gw->getEventQueue()->keyPress( (osgGA::GUIEventAdapter::KeySymbol) event->ascii() );
#endif
}

void 
ViewerWidget::keyReleaseEvent( QKeyEvent* event )
{
#ifdef USE_QT4
    int c = *event->text().toAscii().data();
#else
    int c = event->ascii();
#endif

    _gw->getEventQueue()->keyRelease( (osgGA::GUIEventAdapter::KeySymbol) (c) );
}

void 
ViewerWidget::mousePressEvent( QMouseEvent* event )
{
    int button = 0;
    switch(event->button())
    {
        case(Qt::LeftButton): button = 1; break;
        case(Qt::MidButton): button = 2; break;
        case(Qt::RightButton): button = 3; break;
        case(Qt::NoButton): button = 0; break;
        default: button = 0; break;
    }
    _gw->getEventQueue()->mouseButtonPress(event->x(), event->y(), button);
}
void 
ViewerWidget::mouseDoubleClickEvent ( QMouseEvent * event )
{
    int button = 0;
    switch(event->button())
    {
        case(Qt::LeftButton): button = 1; break;
        case(Qt::MidButton): button = 2; break;
        case(Qt::RightButton): button = 3; break;
        case(Qt::NoButton): button = 0; break;
        default: button = 0; break;
    }
    _gw->getEventQueue()->mouseDoubleButtonPress(event->x(), event->y(), button);
}
void 
ViewerWidget::mouseReleaseEvent( QMouseEvent* event )
{
    int button = 0;
    switch(event->button())
    {
        case(Qt::LeftButton): button = 1; break;
        case(Qt::MidButton): button = 2; break;
        case(Qt::RightButton): button = 3; break;
        case(Qt::NoButton): button = 0; break;
        default: button = 0; break;
    }
    _gw->getEventQueue()->mouseButtonRelease(event->x(), event->y(), button);
}

void 
ViewerWidget::mouseMoveEvent( QMouseEvent* event )
{
    _gw->getEventQueue()->mouseMotion(event->x(), event->y());
}

#endif // WIN32

