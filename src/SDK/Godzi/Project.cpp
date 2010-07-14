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
#include <Godzi/Project>
#include <Godzi/Application>
#include <osgEarth/XmlUtils>
#include <fstream>

using namespace Godzi;

//---------------------------------------------------------------------------

ProjectProperties::ProjectProperties() :
_name( "Untitled" )
{
    //nop
}

ProjectProperties::ProjectProperties( const Config& conf )
{
    conf.getIfSet( "name", _name );
}

Config
ProjectProperties::toConfig() const
{
    Config conf;
    conf.addIfSet( "name", _name );
    return conf;
}

//---------------------------------------------------------------------------

Project::Project()
{
    _map = new osgEarth::Map();
}

Project::Project( const osgEarth::Config& conf )
{
    _props = ProjectProperties( conf.child( "properties" ) );
}

Config
Project::toConfig() const
{
    Config conf( "godzi_project" );

    conf.add( "properties", _props.toConfig() );

    //if ( _map.valid() )
    //    conf.add( "map", _map->toConfig() );

    return conf;
}

//---------------------------------------------------------------------------

bool
NewProjectAction::doAction( const ActionContext& ac, Application* app )
{
    app->setProject( new Project() );
    return true;
}

//---------------------------------------------------------------------------

bool
OpenProjectAction::doAction( const ActionContext& ac, Application* app )
{    
    std::ifstream input( _location.c_str() );
    osg::ref_ptr<osgEarth::XmlDocument> doc = osgEarth::XmlDocument::load( input );
    if ( doc.valid() )
    {
        osgEarth::Config conf = doc->toConfig().child( "godzi_project" );
        Project* project = new Project( conf );
        app->setProject( project, _location );
        return true;
    }
    else
    {
        return setError( "Failed to read project XML document" );
    }
}

//---------------------------------------------------------------------------

bool
SaveProjectAction::doAction( const ActionContext& ac, Application* app )
{
    std::string location = !_location.empty() ? _location : app->getProjectLocation();

    if ( location.empty() || app->getProject() == 0L )
        return setError( "Illegal: empty project location" );

    osg::ref_ptr<osgEarth::XmlDocument> doc = new osgEarth::XmlDocument( app->getProject()->toConfig() );
    if ( !doc.valid() )
        return setError( "Failed to serialize project to XML" );

    std::ofstream output( location.c_str() );
    if ( !output.is_open() )
        return setError( "Failed to open output file for writing" );

    doc->store( output );

    app->setProjectClean();
    return true;
}

//---------------------------------------------------------------------------

bool
UpdateProjectPropertiesAction::doAction( const ActionContext& ac, Application* app )
{
    Project* project = app->getProject();
    if ( !project )
        return setError( "Illegal: no active project" );

    // save the old one
    _oldProps = project->getProperties();

    // ..and apply the new one.
    project->setProperties( _newProps );
    project->dirty();

    return true;
}

bool
UpdateProjectPropertiesAction::undoAction( const ActionContext& ac, Application* app )
{
    Project* project = app->getProject();
    if ( !project )
        return setError( "Illegal: no active project" );

    project->setProperties( _oldProps );
    project->dirty();

    return true;
}

//---------------------------------------------------------------------------