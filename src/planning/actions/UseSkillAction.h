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


#ifndef SELF_USE_SKILL_ACTION_H
#define SELF_USE_SKILL_ACTION_H

#include "planning/IAction.h"

//! This action uses a skill
class SELF_API UseSkillAction : public IAction
{
public:
	RTTI_DECL();

	//! Construction
	UseSkillAction() : m_ReplaceParams(true)
	{}
	UseSkillAction(const std::string & a_Skill) : m_Skill(a_Skill), m_ReplaceParams(true)
	{}

	//! ISerializable interface
	virtual void Serialize(Json::Value & json);
	virtual void Deserialize(const Json::Value & json);

	//! IAction interface
	virtual void Execute(const Goal::SP & a_spGoal, Delegate<const State &> a_Callback);
	virtual IAction * Clone();

	//! Accessors
	const std::string & GetSkill() const;
	bool GetReplaceParams() const;
	const ParamsMap & GetSkillParams() const;

	//! Mutators
	void SetSkill(const std::string & a_Skill);
	void SetReplaceParams(bool a_bReplace);
	ParamsMap & GetSkillParams();

private:
	//! Data
	std::string		m_Skill;			// name of the goal to create
	bool			m_ReplaceParams;	// if true, our params will replace the goals params. 
	ParamsMap		m_SkillParams;		// goal parameters

	class UseSkill
	{
	public:
		UseSkill(UseSkillAction * a_pAction, const Goal::SP & a_spGoal,
			Delegate<const IAction::State &> a_Callback);

	private:
		void OnSkillState(SkillInstance * a_pInstance);

		//! Data
		UseSkillAction *		m_pAction;
		Goal::SP				m_spGoal;
		Delegate<const IAction::State &>
								m_Callback;
	};
};

//-------------------------------------------------

inline const std::string & UseSkillAction::GetSkill() const
{
	return m_Skill;
}

inline const ParamsMap & UseSkillAction::GetSkillParams() const
{
	return m_SkillParams;
}

inline bool UseSkillAction::GetReplaceParams() const
{
	return m_ReplaceParams;
}

inline void UseSkillAction::SetSkill(const std::string & a_SkillName)
{
	m_Skill = a_SkillName;
}

inline ParamsMap & UseSkillAction::GetSkillParams()
{
	return m_SkillParams;
}

inline void UseSkillAction::SetReplaceParams(bool a_bReplace)
{
	m_ReplaceParams = a_bReplace;
}

#endif
