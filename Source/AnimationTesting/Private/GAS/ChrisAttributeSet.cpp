// Christopher Naglik All Rights Reserved


#include "GAS/ChrisAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GAS/ChrisAbilitySystemStatics.h"

void UChrisAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
}

void UChrisAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        float Magnitude = Data.EvaluatedData.Magnitude;

        if (Magnitude < 0.f)
        {
            // Check for burn tag to prevent uneccessary damage buffs on tick burn
            bool bIsDot = false;
            if (Data.EffectSpec.Def)
            {
                FGameplayTag DotTag = FGameplayTag::RequestGameplayTag("damage.type.dot");
                bIsDot = Data.EffectSpec.Def->InheritableGameplayEffectTags.CombinedTags.HasTag(DotTag);
            }

            // ======================================================
            // Get source actor and ASC once — used by multiple checks
            // ======================================================
            AActor* SourceActor = Data.EffectSpec.GetContext().GetEffectCauser();
            UAbilitySystemComponent* SourceASC = nullptr;

            if (SourceActor)
            {
                IAbilitySystemInterface* SourceASI = Cast<IAbilitySystemInterface>(SourceActor);
                if (SourceASI)
                {
                    SourceASC = SourceASI->GetAbilitySystemComponent();
                }
            }

            // ======================================================
            // STEP 1: Loaded Dice — source's 4th hit has 30% chance
            //         to deal 30% more damage
            // ======================================================
            if (SourceASC && !bIsDot)
            {
                bool bFoundActive = false;
                float Active = SourceASC->GetGameplayAttributeValue(UChrisAttributeSet::GetLoadedDiceActiveAttribute(), bFoundActive);

                if (bFoundActive && Active > 0.f)
                {
                    bool bFoundCounter = false;
                    float Counter = SourceASC->GetGameplayAttributeValue(UChrisAttributeSet::GetLoadedDiceHitCounterAttribute(), bFoundCounter);
                    Counter += 1.f;

                    if (Counter >= 4.f)
                    {
                        SourceASC->SetNumericAttributeBase(UChrisAttributeSet::GetLoadedDiceHitCounterAttribute(), 0.f);

                        if (FMath::FRand() < 0.3f)
                        {
                            float ExtraDamage = FMath::Abs(Magnitude) * 0.3f;
                            SetHealth(GetHealth() - ExtraDamage);
                            Magnitude -= ExtraDamage;
                            UE_LOG(LogTemp, Warning, TEXT("[LoadedDice] PROC! 30%% extra damage (%.1f bonus)"), ExtraDamage);
                        }
                    }
                    else
                    {
                        SourceASC->SetNumericAttributeBase(UChrisAttributeSet::GetLoadedDiceHitCounterAttribute(), Counter);
                    }
                }
            }

            // ======================================================
            // STEP 2: Stealth Strike — 40% more damage when hitting
            //         the enemy from behind.
            // ======================================================
            if (SourceActor && SourceASC && !bIsDot)
            {
                bool bFoundStealth = false;
                float StealthActive = SourceASC->GetGameplayAttributeValue(
                    UChrisAttributeSet::GetStealthStrikeActiveAttribute(), bFoundStealth);

                if (bFoundStealth && StealthActive > 0.f)
                {
                    // Target's forward vector (the direction they are facing)
                    FVector TargetForward = GetOwningActor()->GetActorForwardVector();

                    // Direction from target TO the attacker
                    FVector DirToAttacker = (SourceActor->GetActorLocation() - GetOwningActor()->GetActorLocation()).GetSafeNormal();

                    // Dot product:
                    //   +1.0 = attacker is directly IN FRONT of target (facing them)
                    //    0.0 = attacker is to the side (90 degrees)
                    //   -1.0 = attacker is directly BEHIND target
                    //
                    // Dot < 0 means attacker is anywhere in the rear hemisphere
                    float Dot = FVector::DotProduct(TargetForward, DirToAttacker);

                    if (Dot <= -0.6428f)
                    {
                        // Attacker is behind within the 100 degrees angle — deal 40% extra damage
                        float ExtraDamage = FMath::Abs(Magnitude) * 0.4f;
                        SetHealth(GetHealth() - ExtraDamage);
                        Magnitude -= ExtraDamage;
                        UE_LOG(LogTemp, Warning, TEXT("[StealthStrike] Back attack! 40%% extra damage (%.1f bonus, Dot: %.2f)"), ExtraDamage, Dot);
                    }
                }
            }

            // ======================================================
            // STEP 2.5: Consumable Damage Bonus (Blood Serum etc.)
            // ======================================================
            if (SourceASC)
            {
                bool bFoundBonus = false;
                float DamageBonus = SourceASC->GetGameplayAttributeValue(
                    UChrisAttributeSet::GetConsumableDamageBonusAttribute(), bFoundBonus);

                if (bFoundBonus && DamageBonus > 0.f)
                {
                    // DamageBonus is 0.2 for Blood Serum (20% more damage)
                    float ExtraDamage = FMath::Abs(Magnitude) * DamageBonus;
                    SetHealth(GetHealth() - ExtraDamage);
                    Magnitude -= ExtraDamage;
                    UE_LOG(LogTemp, Warning, TEXT("[ConsumableBonus] %.0f%% extra damage (%.1f bonus)"), DamageBonus * 100.f, ExtraDamage);
                }
            }

            // ======================================================
            // STEP 2.6: Smash Bonus Damage (Bonebreaker)
            // ======================================================
            if (SourceASC)
            {
                bool bFoundSmashBonus = false;
                float SmashBonus = SourceASC->GetGameplayAttributeValue(
                    UChrisAttributeSet::GetSmashDamageBonusAttribute(), bFoundSmashBonus);

                if (bFoundSmashBonus && SmashBonus > 0.f && Data.EffectSpec.Def)
                {
                    // Check if this GE is tagged as smash damage
                    FGameplayTag SmashDamageTag = FGameplayTag::RequestGameplayTag("damage.type.smash");
                    if (Data.EffectSpec.Def->InheritableGameplayEffectTags.CombinedTags.HasTag(SmashDamageTag))
                    {
                        // SmashBonus is 0.5 for Bonebreaker → 50% of original magnitude as extra damage
                        float ExtraDamage = FMath::Abs(Magnitude) * SmashBonus;
                        SetHealth(GetHealth() - ExtraDamage);
                        Magnitude -= ExtraDamage;
                        UE_LOG(LogTemp, Warning, TEXT("[Bonebreaker] Smash bonus! %.0f%% extra damage (%.1f bonus)"),
                            SmashBonus * 100.f, ExtraDamage);
                    }
                }
            }

            // ======================================================
            // STEP 2.8: Deadeye — if all 3 projectile shots hit, 
            // the 3rd deals TRIPLE damage.
            // 
            // The counter is ALSO reset between volleys by GA_Shoot in
            // OnShootCooldownFinished, so partial hits from a previous
            // volley don't carry over and falsely trigger the proc.
            // ======================================================
            if (SourceASC && !bIsDot)
            {
                bool bFoundDeadeye = false;
                float DeadeyeFlag = SourceASC->GetGameplayAttributeValue(
                    UChrisAttributeSet::GetDeadeyeActiveAttribute(), bFoundDeadeye);

                if (bFoundDeadeye && DeadeyeFlag > 0.f && Data.EffectSpec.Def)
                {
                    FGameplayTag ProjectileTag = FGameplayTag::RequestGameplayTag("damage.type.projectile");
                    if (Data.EffectSpec.Def->InheritableGameplayEffectTags.CombinedTags.HasTag(ProjectileTag))
                    {
                        // Read the current hit counter from the source (the shooter)
                        bool bFoundCounter = false;
                        float Counter = SourceASC->GetGameplayAttributeValue(
                            UChrisAttributeSet::GetDeadeyeHitCounterAttribute(), bFoundCounter);
                        Counter += 1.f;

                        if (Counter >= 3.f)
                        {
                            // 3rd consecutive hit in this volley — TRIPLE DAMAGE.
                            SourceASC->SetNumericAttributeBase(
                                UChrisAttributeSet::GetDeadeyeHitCounterAttribute(), 0.f);

                            float ExtraDamage = FMath::Abs(Magnitude) * 2.f;
                            SetHealth(GetHealth() - ExtraDamage);
                            Magnitude -= ExtraDamage;
                            UE_LOG(LogTemp, Warning,
                                TEXT("[Deadeye] All 3 shots hit! 3rd shot deals TRIPLE damage (%.1f extra)"),
                                ExtraDamage);
                        }
                        else
                        {
                            // Not at 3 yet — just store the updated counter
                            SourceASC->SetNumericAttributeBase(
                                UChrisAttributeSet::GetDeadeyeHitCounterAttribute(), Counter);
                            UE_LOG(LogTemp, Log, TEXT("[Deadeye] Projectile hit %d/3"), (int32)Counter);
                        }
                    }
                }
            }

            // ======================================================
            // STEP 3: Damage Reductions — multiplicative stacking
            // ======================================================
            float DamageReductionMultiplier = 1.f;

            // --- Arcane Aegis: 30% less from heavy abilities ---
            if (GetHeavyDamageReduction() > 0.f && Data.EffectSpec.Def)
            {
                FGameplayTag HeavyDamageTag = FGameplayTag::RequestGameplayTag("damage.type.heavy");
                if (Data.EffectSpec.Def->InheritableGameplayEffectTags.CombinedTags.HasTag(HeavyDamageTag))
                {
                    DamageReductionMultiplier *= (1.f - GetHeavyDamageReduction());
                    UE_LOG(LogTemp, Log, TEXT("[ArcaneAegis] Heavy ability damage reduced by %.0f%%"), GetHeavyDamageReduction() * 100.f);
                }
            }

            // --- Stasis Ward: 20% less damage when stunned ---
            if (GetStasisWardReduction() > 0.f)
            {
                UAbilitySystemComponent* OwnerASC = GetOwningAbilitySystemComponent();
                if (OwnerASC && OwnerASC->HasMatchingGameplayTag(UChrisAbilitySystemStatics::GetStunStatsTag()))
                {
                    DamageReductionMultiplier *= (1.f - GetStasisWardReduction());
                    UE_LOG(LogTemp, Log, TEXT("[StasisWard] Stunned - damage reduced by %.0f%%"), GetStasisWardReduction() * 100.f);
                }
            }

            // --- Bulwark: flat % reduction on ALL damage ---
            if (GetDamageReduction() > 0.f)
            {
                DamageReductionMultiplier *= (1.f - GetDamageReduction());
            }

            // Warden's Phial activation temporary
            if (GetConsumableDamageReduction() > 0.f)
            {
                DamageReductionMultiplier *= (1.f - GetConsumableDamageReduction());
                UE_LOG(LogTemp, Log, TEXT("[WardensPhial] Consumable damage reduced by %.0f%%"),
                    GetConsumableDamageReduction() * 100.f);
            }

            // Apply total reduction as a single heal-back
            if (DamageReductionMultiplier < 1.f)
            {
                float DamageToHealBack = FMath::Abs(Magnitude) * (1.f - DamageReductionMultiplier);
                SetHealth(GetHealth() + DamageToHealBack);
            }
        }

        SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
        SetCachedHealthPercent(GetHealth() / GetMaxHealth());
    }
    if (Data.EvaluatedData.Attribute == GetManaAttribute())
    {
        SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
        SetCachedManaPercent(GetMana() / GetMaxMana());
    }
}

void UChrisAttributeSet::RescaleHealth()
{
	if (!GetOwningActor()->HasAuthority()) return;

	if(GetCachedHealthPercent() != 0 && GetHealth() != 0)
	{
		SetHealth(GetMaxHealth() * GetCachedHealthPercent());
	}
}

void UChrisAttributeSet::RescaleMana()
{
	if (!GetOwningActor()->HasAuthority()) return;

	if(GetCachedManaPercent() != 0 && GetMana() != 0)
	{
		SetMana(GetMaxMana() * GetCachedManaPercent());
	}
}

void UChrisAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, Health, OldValue);
}

void UChrisAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, MaxHealth, OldValue);
}
void UChrisAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, Mana, OldValue);
}
void UChrisAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, MaxMana, OldValue);
}

void UChrisAttributeSet::OnRep_AttackDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, AttackDamage, OldValue);
}

void UChrisAttributeSet::OnRep_Armour(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, Armour, OldValue);
}

void UChrisAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, MoveSpeed, OldValue);
}

void UChrisAttributeSet::OnRep_MoveAcceleration(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, MoveAcceleration, OldValue);
}

// ********************************* //
//         ABILITY UPGRADES          //
//*********************************  //



void UChrisAttributeSet::OnRep_SmashSweepRadius(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, SmashSweepRadius, OldValue);
}

void UChrisAttributeSet::OnRep_SmashDamageBonus(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, SmashDamageBonus, OldValue);
}

void UChrisAttributeSet::OnRep_StunSweepRadius(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, StunSweepRadius, OldValue);
}

void UChrisAttributeSet::OnRep_ScorchedActive(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, ScorchedActive, OldValue);
}

void UChrisAttributeSet::OnRep_DeadeyeActive(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, DeadeyeActive, OldValue);
}

void UChrisAttributeSet::OnRep_DeadeyeHitCounter(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, DeadeyeHitCounter, OldValue);
}

// ********************************* //
//           SKILL BUFFS             //
//*********************************  //

void UChrisAttributeSet::OnRep_LightAttackBuffDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, LightAttackBuffDamage, OldValue);
}

void UChrisAttributeSet::OnRep_HeavyAttackBuffDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, HeavyAttackBuffDamage, OldValue);
}
void UChrisAttributeSet::OnRep_DamageReduction(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, DamageReduction, OldValue);
}

void UChrisAttributeSet::OnRep_DoubleDownChance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, DoubleDownChance, OldValue);
}

void UChrisAttributeSet::OnRep_StackedOddsChance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, StackedOddsChance, OldValue);
}

void UChrisAttributeSet::OnRep_LoadedDiceActive(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, LoadedDiceActive, OldValue);
}

void UChrisAttributeSet::OnRep_LoadedDiceHitCounter(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, LoadedDiceHitCounter, OldValue);
}

void UChrisAttributeSet::OnRep_StasisWardReduction(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, StasisWardReduction, OldValue);
}

void UChrisAttributeSet::OnRep_HeavyDamageReduction(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, HeavyDamageReduction, OldValue);
}

void UChrisAttributeSet::OnRep_DominionBonus(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, DominionBonus, OldValue);
}

void UChrisAttributeSet::OnRep_StealthStrikeActive(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, StealthStrikeActive, OldValue);
}

// ********************************* //   
//         CONSUMABLE BUFFS          //
//*********************************  //


void UChrisAttributeSet::OnRep_ConsumableDamageBonus(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, ConsumableDamageBonus, OldValue);
}

void UChrisAttributeSet::OnRep_ConsumableDamageReduction(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UChrisAttributeSet, ConsumableDamageReduction, OldValue);
}


void UChrisAttributeSet::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const 
{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);
		DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, Health, COND_None, REPNOTIFY_Always);
		DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
		DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, Mana, COND_None, REPNOTIFY_Always);
		DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
		DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, AttackDamage, COND_None, REPNOTIFY_Always);
		DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, Armour, COND_None, REPNOTIFY_Always);
		DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
		DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, MoveAcceleration, COND_None, REPNOTIFY_Always);

        DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, SmashSweepRadius, COND_None, REPNOTIFY_Always);
        DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, SmashDamageBonus, COND_None, REPNOTIFY_Always);
        DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, StunSweepRadius, COND_None, REPNOTIFY_Always);
        DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, ScorchedActive, COND_None, REPNOTIFY_Always);
        DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, DeadeyeActive, COND_None, REPNOTIFY_Always);
        DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, DeadeyeHitCounter, COND_None, REPNOTIFY_Always);

		DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, LightAttackBuffDamage, COND_None, REPNOTIFY_Always);
		DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, HeavyAttackBuffDamage, COND_None, REPNOTIFY_Always);
		DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, DamageReduction, COND_None, REPNOTIFY_Always);
		DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, DoubleDownChance, COND_None, REPNOTIFY_Always);
		DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, StackedOddsChance, COND_None, REPNOTIFY_Always);
		DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, LoadedDiceActive, COND_None, REPNOTIFY_Always);
		DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, LoadedDiceHitCounter, COND_None, REPNOTIFY_Always);
        DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, StasisWardReduction, COND_None, REPNOTIFY_Always);
		DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, HeavyDamageReduction, COND_None, REPNOTIFY_Always);
        DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, DominionBonus, COND_None, REPNOTIFY_Always);
        DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, StealthStrikeActive, COND_None, REPNOTIFY_Always);

        DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, ConsumableDamageBonus, COND_None, REPNOTIFY_Always);
        DOREPLIFETIME_CONDITION_NOTIFY(UChrisAttributeSet, ConsumableDamageReduction, COND_None, REPNOTIFY_Always);
}