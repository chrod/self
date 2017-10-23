/**
* Copyright 2017 IBM Corp. All Rights Reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/


#ifndef SELF_AQIAGENT_H
#define SELF_AQIAGENT_H

#include "IAgent.h"
#include "blackboard/AQIIntent.h"
#include "services/IAQI.h"
#include "services/ILocation.h"
#include "SelfLib.h"

class SELF_API AQIAgent : public IAgent
{
public:
    RTTI_DECL();

	AQIAgent() : 
		m_bFahrenheit (true),
        m_DefaultCity( "this city" ),
		m_AQIOutOfRangeMessage( "I'm afraid I do not have a AQI report for the day you mentioned. I can only check AQI for the next 10 days" )
    {}

    //! ISerializable interface
    void Serialize( Json::Value & json );
    void Deserialize( const Json::Value & json );

    //! IAgent interface
	virtual bool OnStart();
	virtual bool OnStop();

private:
	//! Data
	bool					m_bFahrenheit;
    std::string             m_AQIOutOfRangeMessage;
    std::string             m_DefaultCity;

	struct AQIRequest
	{
		AQIRequest(AQIAgent * a_pAgent, const AQIIntent::SP & a_spIntent );

		AQIAgent *		m_pAgent;
		AQIIntent::SP	m_spIntent;
		std::string			m_City;
		std::string			m_DateSpecified;
		float				m_Lat;
		float				m_Long;

		void				OnLocation(const Json::Value & json);
		void		        OnCurrentConditionsReceived( const Json::Value & json );
		//void		        OnTenDayForecastReceived( const Json::Value & json );
	};

    //! Event Handlers
    void                    OnAnomaly(const ThingEvent & a_ThingEvent);
    void                    OnAQIRequest(const ThingEvent & a_ThingEvent);
};

#endif //SELF_AQIAGENT_H
