// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "ProjectNoName.h"
#include "ProjectNoNameGameMode.h"
#include "ProjectNoNameHUD.h"
#include "Blueprint/UserWidget.h"

AProjectNoNameGameMode::AProjectNoNameGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AProjectNoNameHUD::StaticClass();
    
    PrimaryActorTick.bCanEverTick = true;
    
    
}

void AProjectNoNameGameMode::BeginPlay() {
    Super::BeginPlay();
    
    this->CurrentCharacter = Cast<AProjectNoNameCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    
    if (this->PlayerHUDClass != nullptr) {
        this->CurrentWidget = CreateWidget<UMainUserWidget>(GetWorld(), this->PlayerHUDClass);
        
        if(this->CurrentWidget != nullptr) {
            this->CurrentWidget->Init();
            UE_LOG(LogTemp, Warning, TEXT("this->CurrentWidget->Init();"));
            this->CurrentWidget->AddToViewport();
        } else {
            UE_LOG(LogTemp, Warning, TEXT("this->CurrentWidget->Init(); -----> FAIL"));
        }
    }
}

void AProjectNoNameGameMode::ShowPause() {
    if (this->PauseHUDClass != nullptr) {
        UGameplayStatics::SetGamePaused(GetWorld(), true);
        auto pauseMenu = CreateWidget<UPauseWidget>(GetWorld(), this->PauseHUDClass);
        if (pauseMenu != nullptr) {
            pauseMenu->Init();
            pauseMenu->AddToViewport();
            
            APlayerController* MyController = GetWorld()->GetFirstPlayerController();
            MyController->bShowMouseCursor = true;
            
            UE_LOG(LogTemp, Warning, TEXT("draw pause HUD"));
        } else {
            UE_LOG(LogTemp, Warning, TEXT("WARNING: can't draw pause HUD"));
        }
    } else {
        UE_LOG(LogTemp, Warning, TEXT("draw pause HUD: this->PauseHUDClass == nullptr"));
    }
}

void AProjectNoNameGameMode::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);
    
    if (this->CurrentCharacter != nullptr && this->CurrentWidget != nullptr) {
        this->CurrentWidget->SetGoldText(TEXT("0G"));
        this->CurrentWidget->SetLevelProgress(2203, 10000);
        this->CurrentWidget->SetLevelText(TEXT("1"));
        this->CurrentWidget->SetLifeDisplay((this->CurrentCharacter->GetCurrentHealthOfPlayer() / this->CurrentCharacter->GetInitialHealth()) * 100.f);
        this->CurrentWidget->SetScoreText(TEXT("0"));
        this->CurrentWidget->SetNbWaveText(TEXT("1"));
        this->CurrentWidget->SetEnemiesText(TEXT("0"), TEXT("0"));
    }
}
