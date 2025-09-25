// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/MJFadeObjectComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine.h"

#include "Kismet/GameplayStatics.h"
void FFadeSystemStuc::NewElement(TObjectPtr<UPrimitiveComponent> _Primitive, const TArray<UMaterialInterface*>& _MaterialInt, const TArray<UMaterialInstanceDynamic*>& _MID, float _FadeCount, bool _bToHide)
{
	PrimitiveComp = _Primitive;
	BaseMatInterface = _MaterialInt;
	FadeMID = _MID;
	FadeCount = _FadeCount;
	bToHide = _bToHide;
}
void FFadeSystemStuc::SetToHide(bool _ToHide)
{
	bToHide = _ToHide;
}
void FFadeSystemStuc::SetHideAndFade(bool _ToHide, float _FadeCount)
{
	bToHide = _ToHide;
	FadeCount = _FadeCount;

}
void FFadeSystemStuc::Destroy()
{
	PrimitiveComp = nullptr;
}
// Sets default values for this component's properties
UMJFadeObjectComponent::UMJFadeObjectComponent()
{
	
	PrimaryComponentTick.bCanEverTick = false;

	CurrentFade = 0.0;
	FadeNowID = 0;
	bIsEnabled = true;
	bIsActivate = true;
	bIsTraceComplex = false;

	AddObjectInterval = 0.1f;
	CalcFadeInterval = 0.05f;
	
	WorkDistance = 5000.0f;
	FadeRate = 10.0f;

	CapsuleRadius = 34.0f;
	
	NearObjectFade = 0.3f;
	FarObjectFade = 0.1f;

	ImmediatelyFade = 0.5;

	ObjectTypes.Add(ECC_WorldStatic);
}


// Called when the game starts
void UMJFadeObjectComponent::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (Controller == nullptr)
	{
		return;
	}
	if (Controller->IsLocalController())
	{
		GetOwner()->GetWorld()->GetTimerManager().SetTimer(TimerHandle_AddObjectsTimer, this, &ThisClass::AddObjectHideTimer, AddObjectInterval,true);
		GetOwner()->GetWorld()->GetTimerManager().SetTimer(TimerHandle_ObjectComputeTimer, this, &ThisClass::FadeWorkTimer, CalcFadeInterval, true);
		
		GetOwner()->GetWorld()->GetTimerManager().PauseTimer(TimerHandle_ObjectComputeTimer);
		GetOwner()->GetWorld()->GetTimerManager().PauseTimer(TimerHandle_AddObjectsTimer);

		SetActivate(bIsActivate);
	}
		
	UGameplayStatics::GetAllActorsOfClass(this, PlayerClass, CharacterArray);

	CachedTraceObjectTypes.Empty();
	CachedTraceObjectTypes.Reserve(ObjectTypes.Num());

	for (const auto& OT : ObjectTypes)
	{
		CachedTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(OT.GetValue()));
	}

	CachedTraceParams = FCollisionQueryParams(TEXT("FadeObjectsTrace"), bIsTraceComplex, GetOwner());
	CachedTraceParams.bReturnPhysicalMaterial = false;
	CachedTraceParams.bTraceComplex = bIsTraceComplex;
	CachedTraceParams.AddIgnoredActors(ActorIgnore);
}

void UMJFadeObjectComponent::AddObjectHideTimer()
{
	const FVector TraceStart = GEngine->GetFirstLocalPlayerController(GetOwner()->GetWorld())->PlayerCameraManager->GetCameraLocation();

	TArray<FHitResult>HitArray;
	HitArray.Reserve(64);

	for (AActor* CurrentActor : CharacterArray)
	{

		if (!IsValid(CurrentActor))
		{
			return;
		}
		const FVector TargetLocation = CurrentActor->GetActorLocation();
		const float DistSq = FVector::DistSquared(TraceStart, TargetLocation);
		if (DistSq > FMath::Square(WorkDistance))
		{
			continue;
		}
		HitArray.Reset();

		GetOwner()->GetWorld()->SweepMultiByObjectType(HitArray, TraceStart, TargetLocation, FQuat::Identity, CachedTraceObjectTypes,
			FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight), CachedTraceParams);


		for (const FHitResult& HR : HitArray)
		{
			UPrimitiveComponent* HitComp = HR.GetComponent();
			if (!HR.bBlockingHit || !IsValid(HitComp))
			{
				continue;
			}

			if (!FadeObjectsHit.Contains(HitComp))
			{
				FadeObjectsHit.Add(HitComp);
			}

		}
		for (int32 i = FadeObjectTemp.Num() - 1; i >= 0; --i)
		{
			UPrimitiveComponent* Comp = FadeObjectTemp[i];
			if (!IsValid(Comp))
			{
				FadeObjectTemp.RemoveAt(i);
				continue;
			}

			if (!FadeObjectsHit.Contains(Comp))
			{
				int32 FoundIndex = INDEX_NONE;
				for (int32 j = 0; j < FadeObject.Num(); ++j)
				{
					if (FadeObject[j].PrimitiveComp == Comp)
					{
						FoundIndex = j;
						break;
					}
				}

				if (FoundIndex != INDEX_NONE)
				{
					FFadeSystemStuc& Item = FadeObject[FoundIndex];

				
					for (int32 m = 0; m < Item.BaseMatInterface.Num(); ++m)
					{
						Comp->SetMaterial(m, Item.BaseMatInterface[m]);
					}
					Item.SetToHide(false);

				}

			}
		}

	}

	for (UPrimitiveComponent* PrimComp : FadeObjectsHit)
	{
		if (!IsValid(PrimComp))
		{
			continue;
		}

		if (ComponentMIDMap.Contains(PrimComp))
		{
			if (!FadeObjectTemp.Contains(PrimComp))
			{
				FadeObjectTemp.Add(PrimComp);
			}
			continue;
		}

		TArray<UMaterialInterface*> IBaseMaterials;
		TArray<UMaterialInstanceDynamic*> IMID;
		const int32 NumMats = PrimComp->GetNumMaterials();
		IBaseMaterials.Reserve(NumMats);
		IMID.Reserve(NumMats);

		for (int32 idx = 0; idx < NumMats; ++idx)
		{
			UMaterialInterface* CurMat = PrimComp->GetMaterial(idx);
			IBaseMaterials.Add(CurMat);

			UMaterialInstanceDynamic* NewMID = UMaterialInstanceDynamic::Create(FadeMaterial, PrimComp);
			IMID.Add(NewMID);

			PrimComp->SetMaterial(idx, NewMID);
		}

		FFadeSystemStuc NewObject;
		NewObject.NewElement(PrimComp, IBaseMaterials, IMID, ImmediatelyFade, true);
		NewObject.CameraCollsion = PrimComp->GetCollisionResponseToChannel(ECC_Camera);

		FadeObject.Add(NewObject);


		TArray<TWeakObjectPtr<UMaterialInstanceDynamic>> WeakMIDs;
		for (UMaterialInstanceDynamic* m : IMID) WeakMIDs.Add(m);
		ComponentMIDMap.Add(PrimComp, WeakMIDs);


		PrimComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	}

	FadeObjectsHit.Empty();

}

void UMJFadeObjectComponent::FadeWorkTimer()
{
	if (FadeObject.Num() == 0) return;

	UWorld* World = GetWorld();
	if (!World) return;

	for (int32 i = FadeObject.Num() - 1; i >= 0; --i)
	{
		FFadeSystemStuc& Item = FadeObject[i];

		const bool bToHide = Item.bToHide;
		const float TargetF = bToHide ? FarObjectFade : 1.0f;
		const float CurrentF = Item.FadeCount;

		const float NewFade = FMath::FInterpConstantTo(CurrentF, TargetF, World->GetDeltaSeconds(), FadeRate);


		for (UMaterialInstanceDynamic* MID : Item.FadeMID)
		{
			if (IsValid(MID))
			{
				MID->SetScalarParameterValue(TEXT("Fade"), NewFade);
			}
		}


		Item.SetHideAndFade(bToHide, NewFade);
		Item.FadeCount = NewFade;


		if (FMath::IsNearlyEqual(NewFade, 1.0f, 1e-3f))
		{
			ComponentMIDMap.Remove(Item.PrimitiveComp);
			FadeObject.RemoveAt(i, 1, false);
			FadeObjectTemp.Remove(Item.PrimitiveComp);
		}
	}


}

void UMJFadeObjectComponent::SetEnable(bool _bIsEnable)
{
	bIsEnabled = _bIsEnable;
}

void UMJFadeObjectComponent::SetActivate(bool _bIsActive)
{
	bIsActivate = _bIsActive;

	if (!bIsActivate)
	{
		GetOwner()->GetWorld()->GetTimerManager().PauseTimer(TimerHandle_ObjectComputeTimer);
		GetOwner()->GetWorld()->GetTimerManager().PauseTimer(TimerHandle_AddObjectsTimer);
	}
	else
	{
		GetOwner()->GetWorld()->GetTimerManager().UnPauseTimer(TimerHandle_ObjectComputeTimer);
		GetOwner()->GetWorld()->GetTimerManager().UnPauseTimer(TimerHandle_AddObjectsTimer);
	}
}


// Called every frame
void UMJFadeObjectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

