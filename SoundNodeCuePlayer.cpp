
#include "SoundNodeCuePlayer.h"
#include "ActiveSound.h"
#include "Sound/SoundCue.h"
#include "FrameworkObjectVersion.h"

#define LOCTEXT_NAMESPACE "SoundNodeCuePlayer"

void USoundNodeCuePlayer::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar.UsingCustomVersion(FFrameworkObjectVersion::GUID);

	if (Ar.CustomVer(FFrameworkObjectVersion::GUID) >= FFrameworkObjectVersion::HardSoundReferences)
	{
		if (Ar.IsLoading())
		{
			Ar << SoundCue;
		}
		else if (Ar.IsSaving())
		{
			USoundCue* HardReference = (ShouldHardReferenceAsset() ? SoundCue : nullptr);
			Ar << HardReference;
		}
	}
}

void USoundNodeCuePlayer::LoadAsset(bool bAddToRoot)
{
	if (IsAsyncLoading())
	{
		SoundCue = SoundCueAssetPtr.Get();
		if (!SoundCue)
		{
			const FString LongPackageName = SoundCueAssetPtr.GetLongPackageName();
			if (!LongPackageName.IsEmpty())
			{
				bAsyncLoading = true;
				LoadPackageAsync(LongPackageName, FLoadPackageAsyncDelegate::CreateUObject(this, &USoundNodeCuePlayer::OnSoundCueLoaded, bAddToRoot));
			}
		}
		else if (bAddToRoot)
		{
			SoundCue->AddToRoot();
		}
		if (SoundCue)
		{
			SoundCue->AddToCluster(this);
		}
	}
	else
	{
		SoundCue = SoundCueAssetPtr.LoadSynchronous();
		if (SoundCue)
		{
			if (bAddToRoot)
			{
				SoundCue->AddToRoot();
			}
			SoundCue->AddToCluster(this);
		}
	}
}

void USoundNodeCuePlayer::ClearAssetReferences()
{
	SoundCue = nullptr;
}

void USoundNodeCuePlayer::OnSoundCueLoaded(const FName& PackageName, UPackage* Package, EAsyncLoadingResult::Type Result, bool bAddToRoot)
{
	if (Result == EAsyncLoadingResult::Succeeded)
	{
		SoundCue = SoundCueAssetPtr.Get();
		if (SoundCue)
		{
			if (bAddToRoot)
			{
				SoundCue->AddToRoot();
			}
			SoundCue->AddToCluster(this);
		}
	}
	bAsyncLoading = false;
}

#if WITH_EDITOR
void USoundNodeCuePlayer::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(USoundNodeCuePlayer, SoundCueAssetPtr))
	{
		LoadAsset();
	}
}
#endif

void USoundNodeCuePlayer::ParseNodes(FAudioDevice* AudioDevice, const UPTRINT NodeWaveInstanceHash, FActiveSound& ActiveSound, const FSoundParseParameters& ParseParams, TArray<FWaveInstance*>& WaveInstances)
{
	if (bAsyncLoading)
	{
		UE_LOG(LogAudio, Verbose, TEXT("Asynchronous load of %s not complete in USoundNodeCuePlayer::ParseNodes, will attempt to play later."), *GetFullNameSafe(this));
		// We're still loading so don't stop this active sound yet
		ActiveSound.bFinished = false;
		return;
	}

	if (SoundCue && !IsTheSameSoundCue())
	{
		SoundCue->Parse(AudioDevice, NodeWaveInstanceHash, ActiveSound, ParseParams, WaveInstances);
	}
}

float USoundNodeCuePlayer::GetDuration()
{
	return SoundCue->Duration;
}

#if WITH_EDITOR
FText USoundNodeCuePlayer::GetTitle() const
{
	FText SoundCueName;
	if (SoundCue)
	{
		SoundCueName = FText::FromString(SoundCue->GetFName().ToString());
	}
	else
	{
		SoundCueName = LOCTEXT("NoSoundCue", "NONE");
	}

	FText Title;

	FFormatNamedArguments Arguments;
	Arguments.Add(TEXT("Description"), Super::GetTitle());
	Arguments.Add(TEXT("SoundCueName"), SoundCueName);
	Title = FText::Format(LOCTEXT("SoundCueDescription", "{Description} : {SoundCueName}"), Arguments);

	return Title;
}
#endif

bool USoundNodeCuePlayer::IsTheSameSoundCue() const
{
	if (SoundCueAssetPtr)
	{
		return SoundCueAssetPtr.GetAssetName() == CuePlayerAssetPtr.GetAssetName();
	}
	return false;
}

#undef LOCTEXT_NAMESPACE
