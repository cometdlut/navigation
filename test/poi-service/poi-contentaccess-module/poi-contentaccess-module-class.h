/**
* @licence app begin@
* SPDX-License-Identifier: MPL-2.0
*
* \copyright Copyright (C) 2013-2014, PCA Peugeot Citroen
*
* \file poi-contentaccess-module-class.h
*
* \brief This file is part of the poi proof of concept.
*
* \author Philippe Colliot <philippe.colliot@mpsa.com>
*
* \version 1.1
*
* This Source Code Form is subject to the terms of the
* Mozilla Public License (MPL), v. 2.0.
* If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/.
*
* For further information see http://www.genivi.org/.
*
* List of changes:
* 10-02-2014, Philippe Colliot, refinement and migration to the new repository
* <date>, <name>, <description of change>
*
* @licence end@
*/
#ifndef POICONTENTACCESSMODULECLASS_H
#define POICONTENTACCESSMODULECLASS_H


#include "../poi-common/genivi-poiservice-constants.h"
#include "../poi-common/genivi-poiservice-contentaccessmodule_adaptor.h"
#include "../poi-common/genivi-poiservice-poicontentaccess_proxy.h"
#include "../poi-common/poi-common-data-model.h"
#include <dbus-c++/glib-integration.h>

#include "poi-datamodel.h"
#include "database.h"

#include "mainwindow.h"


// string conversion to numeric: the third parameter of fromString() should be one of std::hex, std::dec or std::oct
template <class T>
bool fromString(T& t, const std::string& s, std::ios_base& (*f)(std::ios_base&))
{
  std::istringstream iss(s);
  return !(iss >> f >> t).fail();
}

using namespace DBus;
using namespace Glib;

class contentAccessModuleServer;
class poiContentAccess;


class  poiContentAccess
: public org::genivi::poiservice::POIContentAccess_proxy,
  public DBus::IntrospectableProxy,
  public DBus::ObjectProxy
{

public:

    poiContentAccess(DBus::Connection &connection);

    ~poiContentAccess();

private:

};


class  contentAccessModuleServer
: public org::genivi::poiservice::POIContentAccessModule_adaptor,
  public DBus::IntrospectableAdaptor,
  public DBus::ObjectAdaptor
{
public:
    enum {
        INVALID_HANDLE = 0xFF,
        VALID_HANDLE = 1, //the POC manages only one handle !
        BICYCLE_CATEGORY_ID = 0xFE //the POC manages only one category !
    } CONSTANTS;

    contentAccessModuleServer(DBus::Connection &connection, const char* poiDatabaseFileName);

    ~contentAccessModuleServer();

    DBus_version::DBus_version_t GetVersion();

    void SetLanguage(const std::string& languageCode, const std::string& countryCode);

    void PoiSearchStarted(const uint8_t& poiSearchHandle, const uint16_t& maxSize, const DBus_geoCoordinate3D::DBus_geoCoordinate3D_t& location, const std::vector< DBus_categoryRadius::DBus_categoryRadius_t >& poiCategories, const std::vector< DBus_attributeDetails::DBus_attributeDetails_t >& poiAttributes, const std::string& inputString, const uint16_t& sortOption);

    void PoiSearchCanceled(const uint8_t& poiSearchHandle);

    void ResultListRequested(const uint8_t& camId, const uint8_t& poiSearchHandle, const std::vector< std::string >& attributes, uint16_t& statusValue, uint16_t& resultListSize, std::vector< DBus_poiCAMDetails::DBus_poiCAMDetails_t >& resultList);

    std::vector< DBus_searchResultDetails::DBus_searchResultDetails_t > PoiDetailsRequested(const std::vector< uint32_t >& source_id);

    void connectToHMI(MainWindow *w);

    void connectCAM(uint8_t camId);

    void disconnectCAM();

    std::vector<DBus_CAMcategory::DBus_CAMcategory_t> getCategories();

    void setCategoriesID(std::vector<uint16_t> categoryIDList);

    std::string getCAMName();

    uint8_t getCAMId();


private:

// data conversion routines

    DBus::Variant createVariantString(std::string str);

    DBus::Variant createVariantUint16(uint16_t value);

// search routines

    uint16_t searchAroundALocation(DBus_geoCoordinate3D::geoCoordinate3D_t location,const std::string* inputString);

    uint16_t searchPOIRequest(uint16_t categoryIndex, std::string search_string, DBus_geoCoordinate3D::geoCoordinate3D_t left_bottom_location,DBus_geoCoordinate3D::geoCoordinate3D_t right_top_location);

// error management

    void sendDBusError(const char* message);

    void onError();

//
    bool isCategoryAvailable(uint16_t id, uint16_t *category_id);

// geometrical routines


    uint32_t calculateDistance(const DBus_geoCoordinate3D::geoCoordinate3D_t origin, const DBus_geoCoordinate3D::geoCoordinate3D_t target);


    double calculateAngle(const uint32_t radius);

// data

    DBus_version m_version;

    std::string m_languageCode, m_countryCode;

    uint8_t m_poiSearchHandle; // the POC is limited to the management of one handle !

    Database *mp_database; // database access
    DBus_geoCoordinate3D::geoCoordinate3D_t m_leftBottomLocation;
    DBus_geoCoordinate3D::geoCoordinate3D_t m_rightTopLocation;

    MainWindow *mp_HMI;

    ushort m_searchStatus;

    DBus_geoCoordinate3D::geoCoordinate3D_t m_centerLocation;

    uint16_t m_totalSize;

    uint8_t m_camId;


// buffers
    std::vector< DBus_poiCAMDetails::DBus_poiCAMDetails_t >  m_poiTable; // source_id, name, category, location, distance, attributes
    std::vector< DBus_searchResultDetails::DBus_searchResultDetails_t > m_poiDetailsTable; //details(unique id, name, lat, lon, alt), categories, attributes(name, type, value)


// category and attribute management
    uint16_t m_availableCategories;
    poi_category_t m_availableCategoryTable[MAX_CATEGORIES];
    uint16_t m_rootCategory;
};



#endif // POIPOCCONTENTACCESSMODULECLASS_H
