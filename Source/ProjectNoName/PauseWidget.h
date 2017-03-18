

#pragma once
#include "Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/Components/ContentWidget.h"
#include "Runtime/UMG/Public/Components/Button.h"
#include "PauseWidget.generated.h"
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTNONAME_API UPauseWidget: public UUserWidget
{
    GENERATED_BODY()
    
private:
    UButton* _playButton;
    UButton* _returnMainMenuButton;
    
public:
    void Init();
    
    UFUNCTION()
    void PlayButtonClicked();
    
    UFUNCTION()
    void ReturnMainMenuButtonCliked();
};
