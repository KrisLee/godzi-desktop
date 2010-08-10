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

#include <osg/Math>
#include <Godzi/KML>
#include <Godzi/Features/Placemark>
#include <iostream>

#include "kml/dom.h"
#include "kml/base/file.h"
#include "kml/engine.h"
using namespace Godzi;
using namespace Godzi::Features;

static void printIndented(std::string item, int depth) {
  while (depth--) {
      std::cout << "  ";
  }
  std::cout << item;
}


static const kmldom::FeaturePtr getRootFeature(const kmldom::ElementPtr& root) 
{
    const kmldom::KmlPtr kml = kmldom::AsKml(root);
    if (kml && kml->has_feature()) {
        return kml->get_feature();
    }
    return kmldom::AsFeature(root);
}

void collectFeature(FeatureList& fl, const kmldom::FeaturePtr& feature, int depth)
{
    switch (feature->Type()) {
    case kmldom::Type_Document:
        printIndented("Document", depth);
        break;
    case kmldom::Type_Folder:
        printIndented("Folder", depth);
        break;
    case kmldom::Type_GroundOverlay:
        printIndented("GroundOverlay", depth);
        break;
    case kmldom::Type_NetworkLink:
        printIndented("NetworkLink", depth);
        break;
    case kmldom::Type_PhotoOverlay:
        printIndented("PhotoOverlay", depth);
        break;
    case kmldom::Type_Placemark:
    {
        const kmldom::PlacemarkPtr placemark = kmldom::AsPlacemark(feature);
        if (placemark) {
            Placemark* p = new Placemark;
            p->setName(placemark->get_name());
            if (placemark->has_geometry()) {
                double lat, lon;
                if (kmlengine::GetPlacemarkLatLon(placemark, &lat, &lon) ) {
                    p->setCoordinates(osg::DegreesToRadians(lat),osg::DegreesToRadians(lon));
                }
            }
            printIndented("Placemark", depth);

            fl.push_back(p);
        }
    }
    break;
    case kmldom::Type_ScreenOverlay:
        printIndented("ScreenOverlay", depth);
        break;
    default:
        printIndented("other", depth);
        break;
    }

    if (feature->has_name()) {
        std::cout << " " << feature->get_name();
    }
    std::cout << std::endl;

    if (const kmldom::ContainerPtr container = kmldom::AsContainer(feature)) {
        for (size_t i = 0; i < container->get_feature_array_size(); ++i) {
            collectFeature(fl, container->get_feature_array_at(i), depth+1);
        }
    }
}


FeatureList Godzi::readFeaturesFromKML(const std::string& file)
{
    FeatureList features;
    // Read it.
    std::string kml;
    if (!kmlbase::File::ReadFileToString(file, &kml)) {
        std::cout << file << " read failed" << std::endl;
        return features;
    }

		std::cerr << file << " read:\n\n" << kml;

    // Parse it.
    std::string errors;
    kmldom::ElementPtr root = kmldom::Parse(kml, &errors);

    if (!root) {
        std::cout << errors << std::endl;
        return features;
    }

    const kmldom::FeaturePtr feature = getRootFeature(root);

    if (feature) {
        collectFeature(features, feature, 0);
    } else {
        std::cout << "No root feature" << std::endl;
    }

    return features;
}
