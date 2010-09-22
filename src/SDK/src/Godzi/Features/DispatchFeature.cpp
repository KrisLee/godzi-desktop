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

#include <Godzi/Features/DispatchFeature>
#include <Godzi/Features/Symbol>
#include <Godzi/Features/Feature>
#include <osgEarthDrivers/agglite/AGGLiteOptions>
#include <osgEarthDrivers/model_feature_geom/FeatureGeomModelOptions>

using namespace Godzi::Features;

void Godzi::Features::applyFeatureToMap(osgEarth::Map* map, const Godzi::Features::KMLFeatureSource* fs)
{
    // do the model with geom driver
    osgEarth::Features::FeatureList model;
    osgEarth::Features::FeatureList agglite;

    for (osgEarth::Features::FeatureList::const_iterator it = fs->getFeaturesList().begin(); it != fs->getFeaturesList().end(); ++it) {
        Godzi::Features::Feature* f = dynamic_cast<Godzi::Features::Feature*>((*it).get());
        if (f->style().isSet()) {
            {
                const KMLLineSymbol* s = dynamic_cast<const KMLLineSymbol*>(f->style()->get());
                if (s) {
                    if ( s->extrude()->getExtrude() == true && s->altitude()->getAltitudeMode() == KMLAltitude::ClampToGround)
                        agglite.push_back(f);
                    else
                        model.push_back(f);
                    continue;
                }
            }
            {
                const KMLPolygonSymbol* s = dynamic_cast<const KMLPolygonSymbol*>(f->style()->get());
                if (s) {
                    if ( s->extrude()->getExtrude() == true && s->altitude()->getAltitudeMode() == KMLAltitude::ClampToGround)
                        agglite.push_back(f);
                    else
                        model.push_back(f);
                    continue;
                }
            }

            model.push_back(f);

        } else {
            // no style put it in model
            model.push_back(f);
        }
    }
    
    if (!model.empty() ) {
        Godzi::Features::KMLFeatureSource* fsm = new Godzi::Features::KMLFeatureSource(0);
        fsm->setFeaturesList(model);

        osgEarth::Drivers::FeatureGeomModelOptions* worldOpt = new osgEarth::Drivers::FeatureGeomModelOptions();
        worldOpt->featureSource() = fsm;
        ModelLayer* iml = new ModelLayer("world", worldOpt);
        map->addModelLayer( iml );

    }
    if (!agglite.empty() ) {
        Godzi::Features::KMLFeatureSource* fsm = new Godzi::Features::KMLFeatureSource(0);
        fsm->setFeaturesList(agglite);

        osgEarth::Drivers::AGGLiteOptions* worldOpt = new osgEarth::Drivers::AGGLiteOptions();
        worldOpt->featureSource() = fsm;
        ImageMapLayer* iml = new ImageMapLayer("world", worldOpt);
        map->addMapLayer( iml );
    }
}
