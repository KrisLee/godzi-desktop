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

#include <Godzi/Features/Geometry>

using namespace Godzi::Features;

Point::Point() { _geom = new osgEarth::Symbology::PointSet; }
Point::Point(const Point& pm, const osg::CopyOp& cp) : Geometry(), _geom(pm._geom) {}
Point::Point(const osg::Vec3dArray* toCopy) : Geometry(), _geom(new osgEarth::Symbology::PointSet(toCopy)) {}

osg::Vec3Array* Point::toVec3Array() const { return _geom->toVec3Array(); }
osg::Vec3dArray* Point::getCoordinates() { return dynamic_cast<osg::Vec3dArray*>(_geom.get()); }
const osg::Vec3dArray* Point::getCoordinates() const { return dynamic_cast<const osg::Vec3dArray*>(_geom.get()); }





LineString::LineString() { _geom = new osgEarth::Symbology::LineString; }
LineString::LineString(const LineString& pm, const osg::CopyOp& cp) : Geometry(), _geom(pm._geom) {}
LineString::LineString(const osg::Vec3dArray* toCopy) : Geometry(), _geom(new osgEarth::Symbology::LineString(toCopy)) {}

osg::Vec3Array* LineString::toVec3Array() const { return _geom->toVec3Array(); }
osg::Vec3dArray* LineString::getCoordinates() { return dynamic_cast<osg::Vec3dArray*>(_geom.get()); }
const osg::Vec3dArray* LineString::getCoordinates() const { return dynamic_cast<const osg::Vec3dArray*>(_geom.get()); }
