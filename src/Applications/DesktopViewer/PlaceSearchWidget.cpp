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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include <QtGui>
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtXml>
#include <osgEarth/HTTPClient>
#include <osgEarthUtil/EarthManipulator>
#include <Godzi/Actions>
#include <Godzi/Application>
#include "PlaceSearchWidget"

#define LC "[Godzi.PlaceSearchWidget] "

bool ZoomToPlaceAction::doAction(void* sender, Godzi::Application* app)
{
  Godzi::UI::IViewController* view = app->getView();
  osgEarth::Util::EarthManipulator* manip =view->getManipulator();
  if (manip)
  {
	  osgEarth::Util::Viewpoint viewpoint = manip->getViewpoint();

    viewpoint.setFocalPoint(osg::Vec3d((_maxLon + _minLon) / 2.0,
                                       (_maxLat + _minLat) / 2.0,
                                       0L));

		double rangeFactor = _maxLat != _minLat ? _maxLat - _minLat : _maxLon - _minLon;
		viewpoint.setRange(((0.5 * rangeFactor) / 0.267949849) * 111000.0);
		if (viewpoint.getRange() == 0.0)
			viewpoint.setRange(20000000.0);

		manip->setViewpoint(viewpoint, 3.0);
  }

  return true;
}

//------------------------------------------------------------------------

PlaceSearchWidget::PlaceSearchWidget(Godzi::Application* app)
{
  _app = app;

  _networkManager = new QNetworkAccessManager(this);
  QObject::connect(_networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

  initUi();
}

void PlaceSearchWidget::initUi()
{
  //QLabel* searchLabel = new QLabel();

  _searchLine = new QLineEdit();
  _searchLine->setMaximumWidth(200);
  _searchLine->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  QObject::connect(_searchLine, SIGNAL(returnPressed()), this, SLOT(doSearch()));
  QObject::connect(_searchLine, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged()));

  _searchButton = new QPushButton();
  _searchButton->setText(tr("Go"));
  _searchButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  QObject::connect(_searchButton, SIGNAL(clicked()), this, SLOT(doSearch()));

  QHBoxLayout* layout = new QHBoxLayout;
  layout->setSpacing(2);
  layout->addWidget(_searchLine);
  layout->addWidget(_searchButton);
  setLayout(layout);
}

void PlaceSearchWidget::onTextChanged()
{
  _searchLine->setStyleSheet("");
}

void PlaceSearchWidget::doSearch()
{
  if (_searchLine->text().isEmpty())
    return;

  _searchLine->setEnabled(false);
  _searchButton->setEnabled(false);

  QString data = "documentContent=";
  data.append(_searchLine->text());
  data.append("&documentType=text/plain&appid=gDsuaMrV34EDUJMKtcjZFcPViQ3TFa4fFYY1XoxtF8QYJdM7OMmfanB6DRKQMsAEO6zgeQ--");

  _networkManager->post(QNetworkRequest(QUrl("http://wherein.yahooapis.com/v1/document")), data.toUtf8());
}

void PlaceSearchWidget::replyFinished(QNetworkReply* reply)
{
  if (reply->error() != QNetworkReply::NoError)
  {
    _searchLine->setStyleSheet("color : red");
    OE_WARN << LC << reply->errorString().toUtf8().data() << std::endl;
  }
  else
  {
    QString s = reply->readAll();
    std::string replyString = s.toUtf8().data();

    QDomDocument doc;
    QString error;
    int line, col;
    if (!doc.setContent(s, false, &error, &line, &col))
    {
      OE_WARN << LC << "Error parsing Placemaker response:  " << error.toUtf8().data() << std::endl;
    }
    else
    {
      QDomElement docElem = doc.documentElement();
      QDomNodeList extents = docElem.elementsByTagName("extents");
      if (extents.length() > 0)
      {
        QDomElement extent = extents.at(0).toElement();
        if (!extent.isNull())
        {
          double minLat, minLon, maxLat, maxLon;
          if (parseExtents(extent, minLat, minLon, maxLat, maxLon))
            _app->actionManager()->doAction(this, new ZoomToPlaceAction(minLat, minLon, maxLat, maxLon), false);
        }
      }
    }
  }

  _searchLine->setEnabled(true);
  _searchButton->setEnabled(true);
}

bool PlaceSearchWidget::parseExtents(QDomElement extents, double &out_minLat, double &out_minLon, double &out_maxLat, double &out_maxLon)
{
  out_minLat = out_minLon = out_maxLat = out_maxLon = 0.0;

  QDomNodeList sw = extents.elementsByTagName("southWest");
  QDomNodeList ne = extents.elementsByTagName("northEast");
  if (sw.length() > 0 && ne.length() > 0)
  {
    QDomElement swElem = sw.at(0).toElement();
    QDomElement neElem = ne.at(0).toElement();
    if (!swElem.isNull() && !neElem.isNull())
    {
      bool isOk = parseLatLon(swElem, out_minLat, out_minLon);
      isOk = isOk && parseLatLon(neElem, out_maxLat, out_maxLon);

      return isOk;
    }
  }

  return false;
}

bool PlaceSearchWidget::parseLatLon(QDomElement parent, double &out_lat, double &out_lon)
{
  QDomNodeList lats =  parent.elementsByTagName("latitude");
  QDomNodeList lons =  parent.elementsByTagName("longitude");

  if (lats.length() > 0 && lons.length() > 0)
  {
    QDomElement latElem = lats.at(0).toElement();
    QDomElement lonElem = lons.at(0).toElement();
    if (!latElem.isNull() && !lonElem.isNull())
    {
      bool isOk;
      out_lat = latElem.text().toDouble(&isOk);
      out_lon = isOk ? lonElem.text().toDouble(&isOk) : 0.0;

      return isOk;
    }
  }

  return false;
}