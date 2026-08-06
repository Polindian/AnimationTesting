// Christopher Naglik All Rights Reserved


#include "ChrisAudioSubsystem.h"
#include "ChrisSoundLibrary.h"
#include "ChrisAudioSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Components/AudioComponent.h"

void UChrisAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (const UChrisAudioSettings* Settings = GetDefault<UChrisAudioSettings>())
	{
		// Synchronous load is acceptable here: runs once at startup, before any level loads.
		Library = Settings->SoundLibrary.LoadSynchronous();
	}

	if (!Library)
	{
		UE_LOG(LogTemp, Warning, TEXT("ChrisAudio: no sound library set in Project Settings > Chris Audio"));
	}
}

void UChrisAudioSubsystem::Play2D(FGameplayTag Tag)
{
	const FChrisSoundDef* Def = Library ? Library->FindSound(Tag) : nullptr;
	if (!Def)
	{
		UE_LOG(LogTemp, Warning, TEXT("ChrisAudio: no sound for tag %s"), *Tag.ToString());
		return;
	}

	UGameplayStatics::PlaySound2D(this, Def->Sound, Def->VolumeMultiplier,
		Def->PitchMultiplier, 0.f, Def->Concurrency);
}

void UChrisAudioSubsystem::PlayAtLocation(FGameplayTag Tag, FVector Location)
{
	const FChrisSoundDef* Def = Library ? Library->FindSound(Tag) : nullptr;
	if (!Def) { return; }

	UGameplayStatics::PlaySoundAtLocation(this, Def->Sound, Location, FRotator::ZeroRotator,
		Def->VolumeMultiplier, Def->PitchMultiplier, 0.f, Def->Attenuation, Def->Concurrency);
}

UAudioComponent* UChrisAudioSubsystem::PlayAttached(FGameplayTag Tag, USceneComponent* AttachTo, FName SocketName)
{
	const FChrisSoundDef* Def = Library ? Library->FindSound(Tag) : nullptr;
	if (!Def || !AttachTo) { return nullptr; }

	return UGameplayStatics::SpawnSoundAttached(Def->Sound, AttachTo, SocketName,
		FVector::ZeroVector, EAttachLocation::SnapToTarget,
		/*bStopWhenAttachedToDestroyed*/ true,
		Def->VolumeMultiplier, Def->PitchMultiplier, 0.f,
		Def->Attenuation, Def->Concurrency, /*bAutoDestroy*/ true);
}

UChrisAudioSubsystem* UChrisAudioSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject) { return nullptr; }

	// ReturnNull rather than asserting: widgets can be initialised during level
	// transitions when there's briefly no valid world
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull))
	{
		if (const UGameInstance* GI = World->GetGameInstance())
		{
			return GI->GetSubsystem<UChrisAudioSubsystem>();
		}
	}
	return nullptr;
}