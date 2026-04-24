// Christopher Naglik All Rights Reserved


#include "GAS/PA_GenericAbilitySystem.h"

const FRealCurve* UPA_GenericAbilitySystem::GetExperienceCurve() const
{
	return ExperienceCurveTable->FindCurve(ExperienceRowName, "");
}
