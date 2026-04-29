// Christopher Naglik All Rights Reserved


#include "GAS/ChrisGameplayAbilityTypes.h"

FGenericDamageEffectDefinition::FGenericDamageEffectDefinition():DamageEffect{ nullptr }, PushVelocity{0.f}
{

}

FHeroBaseStats::FHeroBaseStats() :Class{ nullptr }, BaseAttackDamage{ 0.f }, BaseArmor{ 0.f }, 
									Intelligence{ 0.f }, IntelligenceGrowthRate{ 0.f }, 
									Strength{ 0.f }, StrengthGrowthRate{ 0.f }, 
									BaseMoveSpeed{ 0.f }, BaseMaxHealth{ 0.f }, BaseMaxMana{ 0.f }
{
}
