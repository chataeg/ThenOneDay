// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MJFadeObjectComponent.generated.h"
/*
* Class Description : Fade Object System
* Author : Lee JuHyeon
* Created Date : 2025_07_02
* Last Modified By :
* Last Modified Date : 
*/

class UMaterialInterface;

USTRUCT(BlueprintType)
struct FFadeSystemStuc
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TObjectPtr<UPrimitiveComponent> PrimitiveComp;

	UPROPERTY()
	TArray<UMaterialInterface*> BaseMatInterface;

	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> FadeMID;

	UPROPERTY()
	float FadeCount;

	UPROPERTY()
	bool bToHide;

	UPROPERTY()
	TEnumAsByte<ECollisionResponse> CameraCollsion;

	void NewElement(TObjectPtr<UPrimitiveComponent> _Primitive, const TArray<UMaterialInterface*>& _MaterialInt, const TArray<UMaterialInstanceDynamic*>& _MID, float _FadeCount, bool _bToHide);

	void SetToHide(bool _ToHide);

	void SetHideAndFade(bool _ToHide, float _FadeCount);
	
	void Destroy();
	FFadeSystemStuc() :PrimitiveComp(nullptr), FadeCount(0.0f), bToHide(true) {};

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTMJ_API UMJFadeObjectComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMJFadeObjectComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	void AddObjectHideTimer();

	void FadeWorkTimer();

	UFUNCTION(BlueprintCallable, Category = "Fade Object")
	void SetEnable(bool _bIsEnable);

	UFUNCTION(BlueprintCallable, Category = "Fade Object")
	void SetActivate(bool _bIsActive);

	TEnumAsByte<ETraceTypeQuery> MJTraceType = TraceTypeQuery1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fade Object")
	TArray<AActor*> ActorIgnore;

private:

	TArray<FFadeSystemStuc>FadeObject;

	FTimerHandle TimerHandle_ObjectComputeTimer;
	FTimerHandle TimerHandle_AddObjectsTimer;
	FTimerHandle TimerHandle_AddPlayersTimer;

	float CurrentFade;

	int32 FadeNowID;

	UPROPERTY()
	TArray<UPrimitiveComponent*> FadeObjectTemp;

	UPROPERTY()
	TArray<UPrimitiveComponent*> FadeObjectsHit;

	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	UMaterialInstance* FadeMaterial;

	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	bool bIsEnabled;

	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	bool bIsActivate;

	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	bool bIsTraceComplex;

	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	float AddObjectInterval;
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	float CalcFadeInterval;

	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	float WorkDistance;

	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	TSubclassOf<AActor> PlayerClass;

	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	TArray<TEnumAsByte<ECollisionChannel>> ObjectTypes;

	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	float FadeRate;

	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	float CapsuleHalfHeight;

	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	float CapsuleRadius;

	UPROPERTY()
	TArray<AActor*> CharacterArray;

	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	float NearObjectFade ;

	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	float FarObjectFade ;

	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	float ImmediatelyFade;

	TMap<TWeakObjectPtr<UPrimitiveComponent>, TArray<TWeakObjectPtr<UMaterialInstanceDynamic>>> ComponentMIDMap;

	TArray<TEnumAsByte<EObjectTypeQuery>> CachedTraceObjectTypes;

	FCollisionQueryParams CachedTraceParams;
};

