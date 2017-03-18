// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectNoName.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "PauseWidget.h"

void UPauseWidget::Init() {
    UE_LOG(LogTemp, Warning, TEXT("UPauseWidget::Init"));
    
    this->_playButton = Cast<UButton>(this->GetWidgetFromName(TEXT("Button_42")));
    if (this->_playButton == nullptr) {
        UE_LOG(LogTemp, Warning, TEXT("WARNING: fail init ref on UPauseWidget (this->_playButton)"));
    } else {
        this->_playButton->OnClicked.AddDynamic(this, &UPauseWidget::PlayButtonClicked);
        UE_LOG(LogTemp, Warning, TEXT("add UPauseWidget::PlayButtonClicked"));
    }
    
    this->_returnMainMenuButton = Cast<UButton>(this->GetWidgetFromName(TEXT("Button_210")));
    if (this->_returnMainMenuButton == nullptr) {
        UE_LOG(LogTemp, Warning, TEXT("WARNING: fail init ref on UPauseWidget (_returnMainMenuButton)"));
    } else {
        this->_returnMainMenuButton->OnClicked.AddDynamic(this, &UPauseWidget::ReturnMainMenuButtonCliked);
        UE_LOG(LogTemp, Warning, TEXT("add UPauseWidget::ReturnMainMenuButtonCliked"));
    }
}

void UPauseWidget::PlayButtonClicked() {
    UE_LOG(LogTemp, Warning, TEXT("void UPauseWidget::PlayButtonClicked"));
    UGameplayStatics::SetGamePaused(GetWorld(), false);
    
    APlayerController* MyController = GetWorld()->GetFirstPlayerController();
    MyController->bShowMouseCursor = false;
    MyController->bEnableClickEvents = false;
    MyController->bEnableMouseOverEvents = false;
    
    this->RemoveFromViewport();
}

void UPauseWidget::ReturnMainMenuButtonCliked() {
    UE_LOG(LogTemp, Warning, TEXT("void UPauseWidget::ReturnMainMenuButtonCliked"));
    
    UGameplayStatics::OpenLevel(GetWorld(), "Menu_HUD");
}
