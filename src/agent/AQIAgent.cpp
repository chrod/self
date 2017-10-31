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


#include "AQIAgent.h"
#include "blackboard/Goal.h"
#include "blackboard/Environment.h"
#include "blackboard/AQIIntent.h"
#include "blackboard/Say.h"
#include "blackboard/Person.h"

REG_SERIALIZABLE(AQIAgent);
RTTI_IMPL(AQIAgent, IAgent);

void AQIAgent::Serialize(Json::Value & json)
{
	IAgent::Serialize(json);

	// These correspond to config data in the AQIAgent definition in bootstrap.json file
	json["m_bFahrenheit"] = m_bFahrenheit;
	json["m_DefaultCity"] = m_DefaultCity;
	json["m_AQIOutOfRangeMessage"] = m_AQIOutOfRangeMessage;
}

void AQIAgent::Deserialize(const Json::Value & json)
{
	IAgent::Deserialize(json);
	if (json.isMember("m_bFahrenheit"))
		m_bFahrenheit = json["m_bFahrenheit"].asBool();
	if (json.isMember("m_AQIOutOfRangeMessage"))
		m_AQIOutOfRangeMessage = json["m_AQIOutOfRangeMessage"].asString();
	if (json.isMember("m_DefaultCity"))
		m_DefaultCity = json["m_DefaultCity"].asString();
}

bool AQIAgent::OnStart()
{
	SelfInstance * pInstance = SelfInstance::GetInstance();
	if (pInstance == NULL)
		return false;

	IAQI * pAQI = pInstance->FindService<IAQI>();
	if (pAQI == NULL)
	{
		Log::Error("AQIAgent", "Failed to find IAQI service.");
		return false;
	}
	ILocation * pLocation = pInstance->FindService<ILocation>();
	if (pLocation == NULL)
	{
		Log::Error("AQIAgent", "Failed to find ILocation service.");
		return false;
	}

	// Two registered function callbacks, subscribed to blackboard topics
	pInstance->GetBlackBoard()->SubscribeToType("Environment",
		DELEGATE(AQIAgent, OnAnomaly, const ThingEvent &, this), TE_STATE);
	pInstance->GetBlackBoard()->SubscribeToType("AQIIntent",
		DELEGATE(AQIAgent, OnAQIRequest, const ThingEvent &, this), TE_ADDED);
	
	return true;
}

bool AQIAgent::OnStop()
{
	SelfInstance * pInstance = SelfInstance::GetInstance();
	if (pInstance == NULL)
		return false;

	pInstance->GetBlackBoard()->UnsubscribeFromType("Environment", this);
	pInstance->GetBlackBoard()->UnsubscribeFromType("AQIIntent", this);
	return true;
}

void AQIAgent::OnAnomaly(const ThingEvent &a_ThingEvent)
{
	Environment::SP spEnvironment = DynamicCast<Environment>(a_ThingEvent.GetIThing());

	if (StringUtil::StartsWith(spEnvironment->GetState(), "Anomaly"))
	{
		Goal::SP spGoal(new Goal(spEnvironment->GetState(), spEnvironment->ToJson()));
		Log::Debug("AQIAgent", "Adding Anomaly goal: %s", spGoal->ToJson().toStyledString().c_str());
		spEnvironment->AddChild(spGoal);
	}
}

void AQIAgent::OnAQIRequest(const ThingEvent & a_ThingEvent)
{
	// When AQI report is requested, create an AQIRequest object
	AQIIntent::SP spAQI = DynamicCast<AQIIntent>(a_ThingEvent.GetIThing());
	if (spAQI)
		new AQIRequest(this, spAQI);
}

AQIAgent::AQIRequest::AQIRequest(AQIAgent * a_pAgent, const AQIIntent::SP & a_spIntent) : m_pAgent(a_pAgent), m_spIntent(a_spIntent)
{
	m_DateSpecified = m_spIntent->GetDate();
	if (m_spIntent->GetLocation().empty() && !m_pAgent->m_DefaultCity.empty())
		m_spIntent->SetLocation(m_pAgent->m_DefaultCity);
	
	ILocation * pLocation = Config::Instance()->FindService<ILocation>();
	if (pLocation != NULL)
	{
		m_City = m_spIntent->GetLocation();
		pLocation->GetLocation(m_City, DELEGATE(AQIRequest, OnLocation, const Json::Value &, this));
	}
	else
		delete this;
}


void AQIAgent::AQIRequest::OnLocation(const Json::Value & json)
{
	IAQI * pAQI = Config::Instance()->FindService<IAQI>();
	if (pAQI != NULL)
	{
		bool bLocationFound = false;
		if (json.isMember("location"))
		{
			Json::Value location = json["location"];
			//Log::Status("AQIAgent", "Location: %s", location.toStyledString().c_str());
			if (location.isMember("latitude"))
			{
				float fLat = 0.0f;
				float fLong = 0.0f;

				if (location["latitude"].isArray())
				{
					fLat = location["latitude"][0].asFloat();
					fLong = location["longitude"][0].asFloat();
				}
				else
				{
					fLat = location["latitude"].asFloat();
					fLong = location["longitude"].asFloat();
				}

				Location a_Location(m_City, fLat, fLong);

				pAQI->GetCurrentConditions(&a_Location,
					DELEGATE(AQIRequest,
						OnCurrentConditionsReceived, const Json::Value &, this));
				
				bLocationFound = true;
			}
		}

		if (!bLocationFound)
		{
			pAQI->GetCurrentConditions(NULL,
				DELEGATE(AQIRequest,
					OnCurrentConditionsReceived,
					const Json::Value &, this));
		}
	}
	else
		delete this;
}

void AQIAgent::AQIRequest::OnCurrentConditionsReceived(const Json::Value & json)
{
	// Report AQI sensor readings to the user (verbal, same as weather)
	// Uses AQI Goal, and "Say" feature
	{
		Json::Value aqi_data = json;
		float temperature = aqi_data["current_temp_f"].asFloat();
		float concentration_pm25 = aqi_data["pm2_5_atm"].asFloat();
		std::string categoryPM25;

		int roundTemp = (int)floor(temperature);
		int roundPM25 = (int)floor(concentration_pm25);
		IAQI::ConcentrationToCategory(roundPM25, categoryPM25);

		Goal::SP spGoal(new Goal("AQI"));
		spGoal->GetParams()["temperature"] = roundTemp;
		//spGoal->GetParams()["location"] = m_City;
		spGoal->GetParams()["date"] = "currently";
		spGoal->GetParams()["concentration"] = roundPM25;
		spGoal->GetParams()["category"] = categoryPM25;

		m_spIntent->AddChild(spGoal);
	}

	delete this;
}