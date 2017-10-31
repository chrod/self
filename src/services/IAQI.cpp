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


#include "IAQI.h"

RTTI_IMPL( IAQI, IService );

void IAQI::CelsiusToFahrenheit( const float & a_Celsius, float & a_Fahrenheit)
{
	a_Fahrenheit = (a_Celsius * 1.8f) + 32.0f;
}

void IAQI::ConcentrationToCategory( const float & a_Concentration, std::string & a_Category)
{	
	// Convert concentration value to category
	// For PM2.5 measurements, split into EPA defined air quality categories
	// Ref: https://www.epa.gov/sites/production/files/2014-05/documents/zell-aqi.pdf
	float ub_good = 51;
	float ub_moderate = 101;
	float ub_usg = 151;
	float ub_unhealthy = 201;
	float ub_veryunhealthy = 301;
	float ub_hazardous = 500;

	if (a_Concentration >= 0 && a_Concentration < ub_good) {
		a_Category = "good";
	} else if (a_Concentration >= ub_good && a_Concentration < ub_moderate) {
		a_Category = "moderate";	
	} else if (a_Concentration >= ub_moderate && a_Concentration < ub_usg) {
		a_Category = "unhealthy for sensitive groups";
	} else if (a_Concentration >= ub_usg && a_Concentration < ub_unhealthy) {
		a_Category = "unhealthy";
	} else if (a_Concentration >= ub_unhealthy && a_Concentration < ub_veryunhealthy) {
		a_Category = "very unhealthy";
	} else if (a_Concentration >= ub_veryunhealthy && a_Concentration < ub_hazardous) {
		a_Category = "hazardous";
	} else {
		a_Category = "an incorrect sensor reading";
	}
}