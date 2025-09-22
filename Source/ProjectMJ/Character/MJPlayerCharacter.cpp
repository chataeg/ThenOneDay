// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MJPlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystem/MJAbilitySystemComponent.h"
#include "DataAsset/DataAsset_StartDataBase.h"
#include "Component/MJPlayerCombatComponent.h"
#include "Component/MJPlayerSkillComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "UI/Inventory/MJInventoryComponent.h"
#include "Component/MJFadeObjectComponent.h"
#include "Component/MJPlayerEffectComponent.h"
#include "Component/MJPlayerStatComponent.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Hearing.h"


class UMJSaveGameSubsystem;

AMJPlayerCharacter::AMJPlayerCharacter()
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 800.0f;
	//CameraBoom->SocketOffset = FVector(0.0f, 55.0f, 65.0);
	CameraBoom->SetRelativeRotation(FRotator(-45.0f, 0.0f, 0.0f));
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;
	CameraBoom->bDoCollisionTest = false;

	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom,USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	// GetCharacterMovement()->BrakingDecelerationWalking = 2000.0;
	GetCharacterMovement()->MaxAcceleration = 99999.f;
	GetCharacterMovement()->BrakingFrictionFactor = 999.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 99999.f;

	PlayerCombatComponent = CreateDefaultSubobject<UMJPlayerCombatComponent>(TEXT("PlayerCombatComponent"));

	//Add FadeComponent
	FadeComponent = CreateDefaultSubobject<UMJFadeObjectComponent>(TEXT("FadeComponent"));

	// Minjin: AI Perception-캐릭터를 StimuliSource로 등록(AI가 감지)
	PerceptionStimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("AIPerceptionStimuliSourceComponent"));
	if (nullptr!= PerceptionStimuliSourceComponent)
	{
		// Sight, Damage, Hearing source 등록(명시적으로 나타내기 위함)
		PerceptionStimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());
		PerceptionStimuliSourceComponent->RegisterForSense(UAISense_Damage::StaticClass());
		PerceptionStimuliSourceComponent->RegisterForSense(UAISense_Hearing::StaticClass());
		
		// RegisterWithPerceptionSystem(): bAutoRegisterAsSource == true 해줌
		PerceptionStimuliSourceComponent->RegisterWithPerceptionSystem();
	}

	UITarget = nullptr;

	UITrigger = CreateDefaultSubobject<USphereComponent>(TEXT("UITrigger"));
	UITrigger->SetupAttachment(RootComponent);
	UITrigger->InitSphereRadius(180.f);
	UITrigger->SetCollisionProfileName(TEXT("Trigger"));
	UITrigger->SetGenerateOverlapEvents(true);
	UITrigger->SetHiddenInGame(true);

	InventoryComponent = CreateDefaultSubobject<UMJInventoryComponent>(TEXT("InventoryComponent"));
	// Skill Component
	SkillComponent = CreateDefaultSubobject<UMJPlayerSkillComponent>(TEXT("SkillComponent"));
	// Stat Component
	StatComponent = CreateDefaultSubobject<UMJPlayerStatComponent>(TEXT("StatComponent"));
	// Effect Component
	EffectComponent = CreateDefaultSubobject<UMJPlayerEffectComponent>(TEXT("EffectComponent"));

	// Minjin: ID 설정
	ID = ETeam_ID::PLAYER;

	static ConstructorHelpers::FClassFinder<AActor>WEAPONCLASS(TEXT("/Game/Characters/Item/BP_Sword.BP_Sword_C"));
	if (WEAPONCLASS.Succeeded())
	{
		WeaponClass = WEAPONCLASS.Class;
	}
}

void AMJPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	APawn* WeaponInstigator = Cast<APawn>(GetMesh()->GetOwner());
	FTransform Trans;
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetMesh()->GetOwner();
	SpawnParams.Instigator = WeaponInstigator;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	AActor* Weapon = GetWorld()->SpawnActor<AActor>(WeaponClass, Trans, SpawnParams);

	if (!Weapon)
	{
		return;
	}
	Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("Weapon"));
	
}

void AMJPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (!CharacterStartData.IsNull())
	{
		if (UDataAsset_StartDataBase* LoadData = CharacterStartData.LoadSynchronous())
		{
			LoadData->GiveToAbilitySystemComponent(Cast<UMJAbilitySystemComponent>(GetAbilitySystemComponent()));
		}
	}
	// 로딩 데이터 있을 시 받아와서 AttributeSet에 적용
	// 없을 시엔 무시하고 기본 AttributeSet 으로 진행됩니다.

	// 태관 : MJPlayerState 에서 LoadPlayersessionData 함수가 InitializeStat 을 호출합니다.
	// if (StatComponent)
	// {
	// 	StatComponent->InitializeStat();
	// }
	StatComponent->OnDamage.AddDynamic(this,&ThisClass::OnDamage);
}

void AMJPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

}

void AMJPlayerCharacter::OnDamage(float Magnitude, bool bIsCritical)
{
	FloatDamage(Magnitude, bIsCritical,EOwnerType::Player);
}
