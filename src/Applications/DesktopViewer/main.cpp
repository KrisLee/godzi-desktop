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
#include <QtGui/QApplication>
#include <osgEarth/XmlUtils>
#include <Godzi/Earth>
#include <Godzi/Application>
#include <Godzi/Project>
#include "GodziQtApplication"
#include "DesktopMainWindow"

#define EARTH_FILE "http://demo.pelicanmapping.com/rmweb/maps/godzi.earth"
#define LOCAL_EARTH_FILE "./data/default.earth"
#define GODZI_CONFIG_FILE "godzi.config"
#define GODZI_CACHE_FILE "godzi.cache"

int
main( int argc, char** argv )
{
    //QApplication qtApp( argc, argv );
		GodziQtApplication qtApp(argc, argv);

		// Check for user home directory
		QDir homedir(QDir::homePath() + QDir::separator() + "Godzi");
		if (!homedir.exists())
			QDir::home().mkdir("Godzi");

		std::string homepath = "";
		if (homedir.exists())
			homepath = homedir.absolutePath().append(QDir::separator()).toStdString();

		// Attempt to read the app settings file
		std::string configPath = homepath + GODZI_CONFIG_FILE;
		Godzi::Config conf;
		std::ifstream input( configPath.c_str() );
    osg::ref_ptr<osgEarth::XmlDocument> doc = osgEarth::XmlDocument::load( input );
    if ( doc.valid() )
				conf = doc->getConfig().child( "godzi_desktop" );

		// Initialize cache
		Godzi::Config appConf = conf.child("godzi_app");
		Godzi::Config cacheOptConf = appConf.child("cache_config").child("cache_opt");
		osgEarth::TMSCacheOptions cacheOpt;
		osgEarth::optional<std::string> cachePath;
		cacheOpt.setPath(cacheOptConf.getIfSet("path", cachePath) && !cachePath.get().empty() ? cachePath.get() : homepath + GODZI_CACHE_FILE);


		osg::ref_ptr<Godzi::Application> app = new Godzi::Application(cacheOpt, appConf);


		// Attempt to initialize remote map file and use local if unsuccesful
		std::string defaultMap = "";
		osgEarth::MapNode* node = Godzi::readEarthFile(EARTH_FILE);
		if (node)
		{
			defaultMap = EARTH_FILE;
		}
		else
		{
			node = Godzi::readEarthFile(LOCAL_EARTH_FILE);
			if (node)
				defaultMap = LOCAL_EARTH_FILE;
		}


		DesktopMainWindow top(app, configPath,  defaultMap);
    top.resize( 800, 600 );
    top.show();

		app->actionManager()->doAction(NULL, new Godzi::NewProjectAction(node ? node->getMap() : 0L));

    qtApp.connect( &qtApp, SIGNAL(lastWindowClosed()), &qtApp, SLOT(quit()) );
    return qtApp.exec();
}
