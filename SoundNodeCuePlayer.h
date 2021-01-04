
#pragma once

#include "Sound/SoundNodeAssetReferencer.h"
#include "SoundNodeCuePlayer.generated.h"

class USoundCue;

/**
* Sound node that contains a reference to the Sound Cue file to be played
*/
UCLASS(hidecategories = Object, editinlinenew, MinimalAPI, meta = (DisplayName = "Cue Player"))
class USoundNodeCuePlayer : public USoundNodeAssetReferencer
{
	// IMPORTANT: Please remember to update the '*_API' identifier above to match your own project
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Category = CuePlayer, meta = (DisplayName = "Sound Cue"))
	TSoftObjectPtr<USoundCue> SoundCueAssetPtr;

	UPROPERTY(transient)
	USoundCue* SoundCue;

	// Make sure Cue Player doesn't play the same Cue we created
	TSoftObjectPtr<USoundNodeCuePlayer> CuePlayerAssetPtr = this;
	bool IsTheSameSoundCue() const;

	void OnSoundCueLoaded(const FName& PackageName, UPackage* Package, EAsyncLoadingResult::Type Result, bool bAddToRoot);

	uint32 bAsyncLoading : 1;

public:
	//~ Begin UObject Interface
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UObject Interface

	//~ Begin USoundNode Interface
	virtual int32 GetMaxChildNodes() const { return 0; } // A Cue Player is the end of the chain, so it has no children
	virtual float GetDuration() override;
	virtual void ParseNodes(FAudioDevice* AudioDevice, const UPTRINT NodeWaveInstanceHash, FActiveSound& ActiveSound, const FSoundParseParameters& ParseParams, TArray<FWaveInstance*>& WaveInstances) override;
#if WITH_EDITOR
	virtual FText GetTitle() const override;
#endif
	//~ End USoundNode Interface

	//~ Begin USoundNodeAssetReferencer Interface
	virtual void LoadAsset(bool bAddToRoot = false) override;
	virtual void ClearAssetReferences() override;
	//~ End USoundNode Interface

};
