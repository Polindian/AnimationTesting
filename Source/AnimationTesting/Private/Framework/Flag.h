// Christopher Naglik All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraActor.h"
#include "Flag.generated.h"

// Which team currently owns capture progress on this flag
UENUM(BlueprintType)
enum class EFlagOwnership : uint8
{
    Neutral,    // 0% for both teams
    TeamOne,    // Red team has capture progress
    TeamTwo     // Blue team has capture progress
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnTeamInfluenceUpdated, float /*TeamOneRate*/, float /*TeamTwoRate*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnFlagCaptured, EFlagOwnership /*WinningTeam*/);

UCLASS()
class AFlag : public AActor
{
    GENERATED_BODY()

public:
    AFlag();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Read the current state from UI or game mode
    FORCEINLINE float GetCapturePercent() const { return CapturePercent; }
    FORCEINLINE EFlagOwnership GetOwnership() const { return Ownership; }
    FORCEINLINE bool IsCaptured() const { return bIsCaptured; }

    FORCEINLINE int32 GetZoneID() const { return ZoneID; }
    FORCEINLINE float GetTeamOneCaptureRate() const { return TeamOneCaptureRate; }
    FORCEINLINE float GetTeamTwoCaptureRate() const { return TeamTwoCaptureRate; }
    float GetInfluenceRadius() const;

    FOnTeamInfluenceUpdated OnTeamInfluenceUpdated;
    FOnFlagCaptured OnFlagCaptured;

    void SetCaptureEnabled(bool bEnabled);
    void ResetCapture();

    void PlayBannerSlam(FLinearColor Color);
    void DismissBanner();
    void PlayTeamCaptureBanner(EFlagOwnership Team);

private:
    UPROPERTY(EditInstanceOnly, Category = "Banner")
    ANiagaraActor* BannerActor;

    FTimerHandle BannerPauseTimerHandle;

    // 0=off, 1=neutral/white, 2=TeamOne color, 3=TeamTwo color
    UPROPERTY(ReplicatedUsing = OnRep_BannerState)
    uint8 BannerState = 0;

    UFUNCTION()
    void OnRep_BannerState();

    void ApplyBannerState();

    // Timer for delayed team banner slam after dismiss
    FTimerHandle BannerTransitionTimerHandle;

    // Team colors (set in Blueprint defaults)
    UPROPERTY(EditDefaultsOnly, Category = "Banner")
    FLinearColor NeutralBannerColor = FLinearColor::White;

    UPROPERTY(EditDefaultsOnly, Category = "Banner")
    FLinearColor TeamOneBannerColor = FLinearColor::Red;

    UPROPERTY(EditDefaultsOnly, Category = "Banner")
    FLinearColor TeamTwoBannerColor = FLinearColor::Blue;

    // Zone detection
    UPROPERTY(EditDefaultsOnly, Category = "Detection")
    class USphereComponent* InfluenceRange;

    UPROPERTY(EditInstanceOnly, Replicated, Category = "Capture")
    int32 ZoneID = 0;

    UFUNCTION()
    void OnInfluencerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInfluencerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // Hero = 3, AI Minion = 1, anything else = 0
    float GetActorCaptureWeight(AActor* Actor) const;

    UPROPERTY(Replicated)
    bool bCaptureEnabled = false;

    // How many percentage points per second each team is generating
    UPROPERTY(Replicated)
    float TeamOneCaptureRate = 0.f;

    UPROPERTY(Replicated)
    float TeamTwoCaptureRate = 0.f;

    // Current capture state
    // CapturePercent ranges from 0 to 100
    UPROPERTY(Replicated, VisibleAnywhere, Category = "Capture")
    float CapturePercent = 0.f;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Capture")
    EFlagOwnership Ownership = EFlagOwnership::Neutral;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Capture")
    bool bIsCaptured = false;

    // Configurable weights
    UPROPERTY(EditDefaultsOnly, Category = "Capture")
    float HeroCaptureWeight = 3.f;

    UPROPERTY(EditDefaultsOnly, Category = "Capture")
    float MinionCaptureWeight = 1.f;

    // Heroes currently overlapping this flag zone
    UPROPERTY()
    TSet<class AChrisPlayerCharacter*> OverlappingHeroes;

    // How much XP to grant per threshold
    UPROPERTY(EditDefaultsOnly, Category = "Capture|Reward")
    float ZoneXPReward = 50.f;

    // Seconds needed on zone(s) before reward
    UPROPERTY(EditDefaultsOnly, Category = "Capture|Reward")
    float ZoneTimeThreshold = 8.f;
};