// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "ProjectNoName.h"
#include "ProjectNoNameCharacter.h"
#include "ProjectNoNameProjectile.h"
#include "ProjectNoNameGameMode.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "MotionControllerComponent.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AProjectNoNameCharacter

AProjectNoNameCharacter::AProjectNoNameCharacter()
{
    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
    
    // set our turn rates for input
    BaseTurnRate = 45.f;
    BaseLookUpRate = 45.f;
    
    // Create a CameraComponent
    FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
    FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
    FirstPersonCameraComponent->bUsePawnControlRotation = true;
    
    // Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
    Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
    Mesh1P->SetOnlyOwnerSee(true);
    Mesh1P->SetupAttachment(FirstPersonCameraComponent);
    Mesh1P->bCastDynamicShadow = false;
    Mesh1P->CastShadow = false;
    Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
    Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);
    
    // Create a gun mesh component
    FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
    FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
    FP_Gun->bCastDynamicShadow = false;
    FP_Gun->CastShadow = false;
    // FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
    FP_Gun->SetupAttachment(RootComponent);
    
    FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
    FP_MuzzleLocation->SetupAttachment(FP_Gun);
    FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));
    
    // Default offset from the character location for projectiles to spawn
    GunOffset = FVector(100.0f, 0.0f, 10.0f);
    
    // Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun
    // are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.
    
    // Create VR Controllers.
    R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
    R_MotionController->Hand = EControllerHand::Right;
    R_MotionController->SetupAttachment(RootComponent);
    L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
    L_MotionController->SetupAttachment(RootComponent);
    
    // Create a gun and attach it to the right-hand VR controller.
    // Create a gun mesh component
    VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
    VR_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
    VR_Gun->bCastDynamicShadow = false;
    VR_Gun->CastShadow = false;
    VR_Gun->SetupAttachment(R_MotionController);
    VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    
    VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
    VR_MuzzleLocation->SetupAttachment(VR_Gun);
    VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
    VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.
    
    // Uncomment the following line to turn motion controllers on by default:
    //bUsingMotionControllers = true;
    
    UClass *myClass = this->GetClass();
    if (myClass) {
        for(UProperty* p = myClass->PropertyLink; p; p = p->PropertyLinkNext ) {
            if (p->GetFName() == TEXT("HealthPoints")) {
                this->_health = Cast<UNumericProperty>(p);
                if (!this->_health) {
                    UE_LOG(LogTemp, Warning, TEXT("Warning: p is not an UNumericProperty!!!!!!"));
                } else {
                    UE_LOG(LogTemp, Warning, TEXT("this->_health -> ok"));
                }
                this->_initialHealth = -1.f;//this->GetCurrentHealthOfPlayer();
                UE_LOG(LogTemp, Warning, TEXT("initHealth: %f"), this->_initialHealth);
                break;
            }
        }
    }
}

void AProjectNoNameCharacter::BeginPlay()
{
    // Call the base class
    Super::BeginPlay();
    
    //Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
    FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
    
    // Show or hide the two versions of the gun based on whether or not we're using motion controllers.
    if (bUsingMotionControllers)
    {
        VR_Gun->SetHiddenInGame(false, true);
        Mesh1P->SetHiddenInGame(true, true);
    }
    else
    {
        VR_Gun->SetHiddenInGame(true, true);
        Mesh1P->SetHiddenInGame(false, true);
    }
}

//////////////////////////////////////////////////////////////////////////
// Input

void AProjectNoNameCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
    // set up gameplay key bindings
    check(PlayerInputComponent);
    
    PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &AProjectNoNameCharacter::ShowPause);
    
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
    
    //InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AProjectNoNameCharacter::TouchStarted);
    if (EnableTouchscreenMovement(PlayerInputComponent) == false)
    {
        PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AProjectNoNameCharacter::OnFire);
    }
    
    PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AProjectNoNameCharacter::OnResetVR);
    
    PlayerInputComponent->BindAxis("MoveForward", this, &AProjectNoNameCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AProjectNoNameCharacter::MoveRight);
    
    // We have 2 versions of the rotation bindings to handle different kinds of devices differently
    // "turn" handles devices that provide an absolute delta, such as a mouse.
    // "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("TurnRate", this, &AProjectNoNameCharacter::TurnAtRate);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
    PlayerInputComponent->BindAxis("LookUpRate", this, &AProjectNoNameCharacter::LookUpAtRate);
}

void AProjectNoNameCharacter::ShowPause() {
    AProjectNoNameGameMode* gameMode = (AProjectNoNameGameMode*) GetWorld()->GetAuthGameMode();
    if (gameMode != nullptr) {
        gameMode->ShowPause();
    } else {
        UE_LOG(LogTemp, Warning, TEXT("AProjectNoNameCharacter::ShowPause -> FAIL CAST"));
    }
}

void AProjectNoNameCharacter::OnFire()
{}

void AProjectNoNameCharacter::OnResetVR()
{
    UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AProjectNoNameCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
    if (TouchItem.bIsPressed == true)
    {
        return;
    }
    TouchItem.bIsPressed = true;
    TouchItem.FingerIndex = FingerIndex;
    TouchItem.Location = Location;
    TouchItem.bMoved = false;
}

void AProjectNoNameCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
    if (TouchItem.bIsPressed == false)
    {
        return;
    }
    if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
    {
        OnFire();
    }
    TouchItem.bIsPressed = false;
}

void AProjectNoNameCharacter::MoveForward(float Value)
{
    if (Value != 0.0f)
    {
        // add movement in that direction
        AddMovementInput(GetActorForwardVector(), Value);
    }
}

void AProjectNoNameCharacter::MoveRight(float Value)
{
    if (Value != 0.0f)
    {
        // add movement in that direction
        AddMovementInput(GetActorRightVector(), Value);
    }
}

void AProjectNoNameCharacter::TurnAtRate(float Rate)
{
    // calculate delta for this frame from the rate information
    AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AProjectNoNameCharacter::LookUpAtRate(float Rate)
{
    // calculate delta for this frame from the rate information
    AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AProjectNoNameCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
    bool bResult = false;
    if (FPlatformMisc::GetUseVirtualJoysticks() || GetDefault<UInputSettings>()->bUseMouseForTouch)
    {
        bResult = true;
        PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AProjectNoNameCharacter::BeginTouch);
        PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AProjectNoNameCharacter::EndTouch);
        
        //Commenting this out to be more consistent with FPS BP template.
        //PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AProjectNoNameCharacter::TouchUpdate);
    }
    return bResult;
}

double AProjectNoNameCharacter::GetInitialHealth() {
    if (this->_initialHealth < 0.f) {
        this->_initialHealth = this->GetCurrentHealthOfPlayer();
    }
    return this->_initialHealth;
}

double AProjectNoNameCharacter::GetCurrentHealthOfPlayer() {
    if (this->_health) {
        if (this->_health->IsFloatingPoint()) {
            auto valuePtr = this->_health->ContainerPtrToValuePtr<void>(this);
            auto r = this->_health->GetFloatingPointPropertyValue(valuePtr);
            return r;
        } else {
            return 0.f;
        }
    } else {
        return 0.f;
    }
}








