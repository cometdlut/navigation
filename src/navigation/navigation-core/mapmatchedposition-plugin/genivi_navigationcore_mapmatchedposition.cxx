/**
* @licence app begin@
* SPDX-License-Identifier: MPL-2.0
*
* \copyright Copyright (C) 2013-2014, PCA Peugeot Citroen
*
* \file genivi_navigationcore_mapmatchedposition.cxx
*
* \brief This file is part of the Navit POC.
*
* \author Martin Schaller <martin.schaller@it-schaller.de>
*
* \version 1.0
*
* This Source Code Form is subject to the terms of the
* Mozilla Public License (MPL), v. 2.0.
* If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/.
*
* For further information see http://www.genivi.org/.
*
* List of changes:
* 
* <date>, <name>, <description of change>
*
* @licence end@
*/
#include <dbus-c++/glib-integration.h>
#include "constants/genivi-navigationcore-constants.h"
#include "genivi_navigationcore_mapmatchedposition_adaptor.h"
#include "navit/config.h"
#define USE_PLUGINS 1
#include "navit/debug.h"
#include "navit/plugin.h"
#include "navit/item.h"
#include "navit/config_.h"
#include "navit/navit.h"
#include "navit/callback.h"
#include "navit/navigation.h"
#include "navit/map.h"
#include "navit/transform.h"
#include "navit/track.h"
#include "navit/vehicle.h"
#include "navit/route.h"
#include "navit/config_.h"

static DBus::Glib::BusDispatcher dispatcher;
static DBus::Connection *conn;
static class MapMatchedPosition *server;
static struct attr config_cb,navit_cb,tracking_cb;
static struct tracking *tracking;
static struct item last_item;
static char *street_name;
static struct attr vehicle_speed={attr_speed,(char *)40};
static uint16_t simulationMode=GENIVI_NAVIGATIONCORE_SIMULATION_STATUS_NO_SIMULATION;

static struct navit *
get_navit(void)
{
	struct attr navit;
	if (!config_get_attr(config, attr_navit, &navit, NULL))
		return NULL;
	return navit.u.navit;
}

static struct vehicle *
get_vehicle(const char *source_prefix)
{
	struct navit *navit=get_navit();
	struct attr vehicle;
	struct vehicle *ret=NULL;
	if (!source_prefix) {
		if (navit_get_attr(navit, attr_vehicle, &vehicle, NULL))
			ret=vehicle.u.vehicle;
	} else {
		struct attr_iter *iter=navit_attr_iter_new();
		while (navit_get_attr(navit, attr_vehicle, &vehicle, iter)) {
			struct attr source;
			if (vehicle.u.vehicle && vehicle_get_attr(vehicle.u.vehicle, attr_source, &source, NULL) && 
				!strncmp(source.u.str, source_prefix, strlen(source_prefix))) { 
				ret=vehicle.u.vehicle;
				break;
			}
		}
		navit_attr_iter_destroy(iter);
	}	
	return ret;
}

void
demo_update(bool start)
{
	struct attr vehicle_null_speed={attr_speed,(char *)0};
	struct vehicle *vehicle=get_vehicle("demo:");
	vehicle_set_attr(vehicle, start?&vehicle_speed:&vehicle_null_speed);
}

static DBus::Variant
variant_double(double d)
{
	DBus::Variant variant;
	DBus::MessageIter iter=variant.writer();
	iter << d;
	return variant;
}

static DBus::Variant
variant_string(std::string s)
{
	DBus::Variant variant;
	DBus::MessageIter iter=variant.writer();
	iter << s;
	return variant;
}


class  MapMatchedPosition
: public org::genivi::navigationcore::MapMatchedPosition_adaptor,
  public DBus::IntrospectableAdaptor,
  public DBus::ObjectAdaptor
{
	public:
	MapMatchedPosition(DBus::Connection &connection)
	: DBus::ObjectAdaptor(connection, "/org/genivi/navigationcore")
	{
	}

	::DBus::Struct< uint16_t, uint16_t, uint16_t, std::string >
	GetVersion()
	{
		::DBus::Struct< uint16_t, uint16_t, uint16_t, std::string > Version;
		Version._1=3;
		Version._2=0;
		Version._3=0;
		Version._4=std::string("23-10-2013");
		return Version;
	}

#if 0
	void
	GetSupportedPositionInfo(std::vector< int32_t >& PositionInfo, uint32_t& Result)
	{
		PositionInfo.push_back(GENIVI_NAVIGATIONCORE_LATITUDE);
		PositionInfo.push_back(GENIVI_NAVIGATIONCORE_LONGITUDE);
		PositionInfo.push_back(GENIVI_NAVIGATIONCORE_SPEED);
		PositionInfo.push_back(GENIVI_NAVIGATIONCORE_HEADING);
	}	
#endif

	std::map< uint16_t, ::DBus::Variant >
	GetPosition(const std::vector< uint16_t >& valuesToReturn)
	{
		dbg(1,"enter\n");
		std::map< uint16_t, ::DBus::Variant >map;
		struct attr attr;
		for (int i = 0 ; i < valuesToReturn.size() ; i++) {
			switch (valuesToReturn[i]) {
			case GENIVI_NAVIGATIONCORE_LATITUDE:
				if (tracking_get_attr(tracking, attr_position_coord_geo, &attr, NULL)) 
					map[GENIVI_NAVIGATIONCORE_LATITUDE]=variant_double(attr.u.coord_geo->lat);
				break;
			case GENIVI_NAVIGATIONCORE_LONGITUDE:
				if (tracking_get_attr(tracking, attr_position_coord_geo, &attr, NULL)) 
					map[GENIVI_NAVIGATIONCORE_LONGITUDE]=variant_double(attr.u.coord_geo->lng);
				break;
			case GENIVI_NAVIGATIONCORE_SPEED:
				if (tracking_get_attr(tracking, attr_position_speed, &attr, NULL))
					map[GENIVI_NAVIGATIONCORE_SPEED]=variant_double(*attr.u.numd);
				break;
			case GENIVI_NAVIGATIONCORE_HEADING:
				if (tracking_get_attr(tracking, attr_position_direction, &attr, NULL))
					map[GENIVI_NAVIGATIONCORE_HEADING]=variant_double(*attr.u.numd);
				break;
			}
		}
		return map;
	}

	std::map< uint16_t, ::DBus::Variant >
	GetAddress(const std::vector< uint16_t >& valuesToReturn)
	{
		std::map< uint16_t, ::DBus::Variant >ret;
		std::vector< uint16_t >::const_iterator it;
		for (it = valuesToReturn.begin(); it < valuesToReturn.end(); it++) {
			if (*it == GENIVI_NAVIGATIONCORE_STREET && street_name) {
				ret[*it]=variant_string(street_name);
			}
		}
		return ret;
	}

	std::map< uint16_t, ::DBus::Variant >
	GetPositionOnSegment(const std::vector< uint16_t >& valuesToReturn)
	{
		throw DBus::ErrorNotSupported("Not yet supported");
	}

	std::map< uint16_t, ::DBus::Variant >
	GetStatus(const std::vector< uint16_t >& valuesToReturn)
	{
		throw DBus::ErrorNotSupported("Not yet supported");
	}

	uint16_t
	GetSimulationStatus()
	{
		return simulationMode;
	}

	void
	RemoveSimulationSpeedListener()
	{
		throw DBus::ErrorNotSupported("Not yet supported");
	}

	void
	AddSimulationStatusListener()
	{
		throw DBus::ErrorNotSupported("Not yet supported");
	}

	void
	AddSimulationSpeedListener()
	{
		throw DBus::ErrorNotSupported("Not yet supported");
	}

	void
	SetPosition(const uint32_t& sessionHandle, const std::map< uint16_t, ::DBus::Variant >& position)
	{
		throw DBus::ErrorNotSupported("Not yet supported");
	}

	void
	StartSimulation(const uint32_t& sessionHandle)
	{
		if (simulationMode == GENIVI_NAVIGATIONCORE_SIMULATION_STATUS_PAUSED || simulationMode == GENIVI_NAVIGATIONCORE_SIMULATION_STATUS_FIXED_POSITION ) {
			simulationMode=GENIVI_NAVIGATIONCORE_SIMULATION_STATUS_RUNNING;
			SimulationStatusChanged(simulationMode);
			demo_update(true); 
		}
	}

	void
	PauseSimulation(const uint32_t& sessionHandle)
	{
		if (simulationMode == GENIVI_NAVIGATIONCORE_SIMULATION_STATUS_RUNNING) {
			simulationMode=GENIVI_NAVIGATIONCORE_SIMULATION_STATUS_PAUSED;
			SimulationStatusChanged(simulationMode);
			demo_update(false);
		}
	}


	void
    SetSimulationMode(const uint32_t& SessionHandle, const bool& Activate)
	{
		dbg(0,"enter Activate=%d\n",Activate);
		uint16_t newSimulationMode;
		struct attr vehicle;
		vehicle.type=attr_vehicle;
		vehicle.u.vehicle=get_vehicle(Activate?"demo:":"enhancedposition:");
		if (vehicle.u.vehicle) {
			newSimulationMode = Activate ? GENIVI_NAVIGATIONCORE_SIMULATION_STATUS_FIXED_POSITION : GENIVI_NAVIGATIONCORE_SIMULATION_STATUS_NO_SIMULATION;
			if (newSimulationMode != simulationMode) {
				simulationMode=newSimulationMode;
				SimulationStatusChanged(simulationMode);
			}
			demo_update(false);
			struct navit *navit=get_navit();
			navit_set_attr(navit, &vehicle);
		} else {
			dbg(0,"Failed to get vehicle\n");
        	}
	}

	void
    SetSimulationSpeed(const uint32_t& sessionHandle, const uint8_t& speedFactor)
	{
		int speed=speedFactor*40/4;
		if (vehicle_speed.u.num != speed) {
			vehicle_speed.u.num=speed;
			SimulationSpeedChanged(speedFactor);
			if (simulationMode == GENIVI_NAVIGATIONCORE_SIMULATION_STATUS_RUNNING) {
				demo_update(true);
			}
		}
	}

	uint8_t
	GetSimulationSpeed()
	{
		return vehicle_speed.u.num*4/40;
	}

	void
	RemoveSimulationStatusListener()
	{
		throw DBus::ErrorNotSupported("Not yet supported");
	}
};

static void
tracking_attr_position_coord_geo(void)
{
	struct attr position_coord_geo, current_item;
	dbg(1,"enter\n");
	if (tracking_get_attr(tracking, attr_position_coord_geo, &position_coord_geo, NULL)) {
		std::vector< uint16_t >changes;
		changes.push_back(GENIVI_NAVIGATIONCORE_LATITUDE);
		changes.push_back(GENIVI_NAVIGATIONCORE_LONGITUDE);
		changes.push_back(GENIVI_NAVIGATIONCORE_SPEED);
		changes.push_back(GENIVI_NAVIGATIONCORE_HEADING);
		server->PositionUpdate(changes);
	}
	if (tracking_get_attr(tracking, attr_current_item, &current_item, NULL) && current_item.u.item) {
		if (!item_is_equal(last_item, *current_item.u.item)) {
			last_item=*current_item.u.item;
			char *new_street_name=NULL;
			struct map_rect *mr=map_rect_new(last_item.map, NULL);
			struct item *item;
			if (mr && (item = map_rect_get_item_byid(mr, last_item.id_hi, last_item.id_lo))) {
				struct attr label;
				if (item_attr_get(item, attr_label, &label)) 
					new_street_name=label.u.str;
			}
			if (g_strcmp0(new_street_name, street_name)) {
				g_free(street_name);
				street_name=g_strdup(new_street_name);
				std::vector< uint16_t >changes;
				changes.push_back(GENIVI_NAVIGATIONCORE_STREET);
				server->AddressUpdate(changes);
			}
			if (mr)
				map_rect_destroy(mr);
		}
	}
}

static void
navit_attr_navit(struct navit *navit)
{
	struct attr tracking_attr;
	if (navit_get_attr(navit, attr_trackingo, &tracking_attr, NULL)) {
		tracking=tracking_attr.u.tracking;
		tracking_cb.type=attr_callback;
		tracking_cb.u.callback=callback_new_attr_0(callback_cast(tracking_attr_position_coord_geo), attr_position_coord_geo);
		tracking_add_attr(tracking, &tracking_cb);
	}
}

static void
config_attr_navit(struct navit *navit, int add)
{
	if (!tracking && !navit_cb.u.callback && add) {
		navit_cb.type=attr_callback;
		navit_cb.u.callback=callback_new_attr_0(callback_cast(navit_attr_navit), attr_navit);
		navit_add_attr(navit, &navit_cb);
	}
}


void
plugin_init(void)
{
	dispatcher.attach(NULL);
	DBus::default_dispatcher = &dispatcher;
	conn = new DBus::Connection(DBus::Connection::SessionBus());
	conn->setup(&dispatcher);
	conn->request_name("org.genivi.navigationcore.MapMatchedPosition");
	server=new MapMatchedPosition(*conn);
	config_cb.type=attr_callback;
	config_cb.u.callback=callback_new_attr_0(callback_cast(config_attr_navit), attr_navit);
	config_add_attr(config, &config_cb);
}
