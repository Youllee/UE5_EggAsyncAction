#include "EggAsyncEasingAction.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Stats/Stats.h"

namespace
{
	float easeBase(float percent, float exponential) { return (exponential != 1.0f && exponential != 0.0f) ? FMath::Pow(percent, exponential) : percent; }
	float easeIn(float percent, float func(float)) { return func(percent); }
	float easeOut(float percent, float func(float)) { return 1.0f - func(1.0f - percent); }
	float easeInOut(float percent, float func(float)) { return (percent < 0.5f) ? func(percent * 2.0f) * 0.5f : 1.0f - (func(2.0f - (2.0f * percent)) * 0.5f); }
	float easeOutIn(float percent, float func(float)) { return (percent < 0.5f) ? (1.0f - func(1.0f - (2.0f * percent))) * 0.5f : (0.5f * func((percent * 2.0f) - 1.0f)) + 0.5f; }

//	float easeLinear(float percent) { return percent; }
	float easeStep(float percent, float StepCount)
	{
		const int32 SafeStepCount = FMath::Max(1, FMath::RoundToInt(StepCount));
		return FMath::FloorToFloat(percent * SafeStepCount) / SafeStepCount;
	}
	float easeSine(float percent) { return 1.0f - FMath::Cos(percent * PI * 0.5f); }
	float easeCircle(float percent) { return 1.0f - FMath::Sqrt(1.f - (percent * percent)); }
	float easeElastic(float percent) { return (percent == 0.0f || percent == 1.0f) ? percent : FMath::Pow(2.0f, (10.f * percent) - 10.f) * FMath::Sin(((percent * 10.f) - 10.75f) * ((2.0f * PI) / 3.0f)) * -1.0f; }
	float easeBack(float percent) { return percent * percent * ((3.0f * percent) - 2.0f); }
	float easeBounce(float percent)
	{
		float bounce = 4.0f;
		float pow2 = 16.0f;

		while (percent < (pow2 - 1.0f) / 11.0f)
		{
			pow2 = FMath::Pow(2.0f, --bounce);
		}

		return (1.0f / FMath::Pow(4.0f, (3.0f - bounce))) - (7.5625f * FMath::Pow(((((pow2 * 3.0f) - 2.0f) / 22.0f)) - percent, 2.0f));
	}

	float EasingFunction(float percent, EEggEasingType easingType, float exponential)
	{
	//	float safePercent = FMath::Clamp(percent, 0.0f, 1.0f);
	//	float safeExponential = FMath::Max(exponential, 0.0f);

		switch (easingType)
		{
		case EEggEasingType::Linear:		return percent;
		case EEggEasingType::Step:			return easeStep(percent, exponential);

		case EEggEasingType::EaseIn:		return easeBase(percent, exponential);
		case EEggEasingType::EaseOut:		return 1.0f - easeBase(1.0f - percent, exponential);
		case EEggEasingType::EaseInOut:		return (percent < 0.5f) ? easeBase(percent * 2.0f, exponential) * 0.5f : 1.0f - (easeBase(2.0f - (2.0f * percent), exponential) * 0.5f);
		case EEggEasingType::EaseOutIn:		return (percent < 0.5f) ? (1.0f - easeBase(1.0f - (2.0f * percent), exponential)) * 0.5f : (easeBase((percent * 2.0f) - 1.0f, exponential) * 0.5f) + 0.5f;

		case EEggEasingType::SineIn:		return easeIn(easeBase(percent, exponential), &easeSine);
		case EEggEasingType::SineOut:		return easeOut(easeBase(percent, exponential), &easeSine);
		case EEggEasingType::SineInOut:		return easeInOut(easeBase(percent, exponential), &easeSine);
		case EEggEasingType::SineOutIn:		return easeOutIn(easeBase(percent, exponential), &easeSine);

		case EEggEasingType::CircleIn:		return easeIn(easeBase(percent, exponential), &easeCircle);
		case EEggEasingType::CircleOut:		return easeOut(easeBase(percent, exponential), &easeCircle);
		case EEggEasingType::CircleInOut:	return easeInOut(easeBase(percent, exponential), &easeCircle);
		case EEggEasingType::CircleOutIn:	return easeOutIn(easeBase(percent, exponential), &easeCircle);

		case EEggEasingType::ElasticIn:		return easeIn(easeBase(percent, exponential), &easeElastic);
		case EEggEasingType::ElasticOut:	return easeOut(easeBase(percent, exponential), &easeElastic);
		case EEggEasingType::ElasticInOut:	return easeInOut(easeBase(percent, exponential), &easeElastic);
		case EEggEasingType::ElasticOutIn:	return easeOutIn(easeBase(percent, exponential), &easeElastic);

		case EEggEasingType::BackIn:		return easeIn(easeBase(percent, exponential), &easeBack);
		case EEggEasingType::BackOut:		return easeOut(easeBase(percent, exponential), &easeBack);
		case EEggEasingType::BackInOut:		return easeInOut(easeBase(percent, exponential), &easeBack);
		case EEggEasingType::BackOutIn:		return easeOutIn(easeBase(percent, exponential), &easeBack);

		case EEggEasingType::BounceIn:		return easeIn(easeBase(percent, exponential), &easeBounce);
		case EEggEasingType::BounceOut:		return easeOut(easeBase(percent, exponential), &easeBounce);
		case EEggEasingType::BounceInOut:	return easeInOut(easeBase(percent, exponential), &easeBounce);
		case EEggEasingType::BounceOutIn:	return easeOutIn(easeBase(percent, exponential), &easeBounce);
		}

		return percent;
	}
}

UEggAsyncEasingAction::UEggAsyncEasingAction()
	: FTickableGameObject(ETickableTickType::Never)
{

}

void UEggAsyncEasingAction::Tick(float DeltaTime)
{
	if (bIsRunning == false)
	{
		return;
	}

	if (WorldContextObject.IsValid() == false || ShouldBroadcastDelegates() == false)
	{
		SetReadyToDestroy();
		return;
	}

	ElapsedTime += DeltaTime;

	float localPercent = GetLocalPercent();
	float percent = static_cast<float>(CurrentLoopCount) + localPercent;

	// Update 이벤트는 Tick마다 호출하지 않고, MaxUpdateRate에 따라 호출 횟수를 제한합니다.
	if (ShouldUpdate(DeltaTime))
	{
		float blendPercent = GetLoopBlendPercent(localPercent);
		Blend(percent, blendPercent);

		if (bIsRunning == false)
		{
			return;
		}
	}

	// LoopDuration이 0이면 루프 완료 이벤트를 즉시 호출합니다.
	if (LoopDuration <= 0.0f)
	{
		++CurrentLoopCount;
		ElapsedTime = 0.0f;

		if (LoopCount >= 0 && CurrentLoopCount >= LoopCount)
		{
			// 액션 종료
			Complete();
		}
		else
		{
			// 루프 종료
			NotifyCompleted(static_cast<float>(CurrentLoopCount));
		}

		return;
	}

	// 루프 완료 이벤트를 호출합니다.
	if (ElapsedTime >= LoopDuration)
	{
		const float RemainingElapsedTime = ElapsedTime - LoopDuration;

		++CurrentLoopCount;

		ElapsedTime = 0.0f;

		if (LoopCount >= 0 && CurrentLoopCount >= LoopCount)
		{
			// 액션 종료
			Complete();
		}
		else
		{
			// 루프 종료
			NotifyCompleted(static_cast<float>(CurrentLoopCount));

			if (bIsRunning)
			{
				ElapsedTime = RemainingElapsedTime;
			}
		}

		return;
	}
}

TStatId UEggAsyncEasingAction::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UEggAsyncEasingAction, STATGROUP_Tickables);
}

void UEggAsyncEasingAction::Activate()
{
	if (bIsRunning)
	{
		return;
	}
	ElapsedTime = 0.0f;
	UpdateElapsedTime = 0.0f;
	CurrentLoopCount = 0;

	Super::Activate();
	if (WorldContextObject.IsValid() == false || ShouldBroadcastDelegates() == false)
	{
		SetReadyToDestroy();
		return;
	}

	if (LoopCount != 0)
	{
		WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddUObject(this, &UEggAsyncEasingAction::HandleWorldCleanup);
		bIsRunning = true;
		bTickable = true;
		SetTickableTickType(ETickableTickType::Conditional);
	}
	else
	{
		SetReadyToDestroy();
	}
}

void UEggAsyncEasingAction::Cancel()
{
	const bool bShouldBroadcastCanceled = bIsRunning && ShouldBroadcastDelegates() && HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed) == false;
	const float percent = bShouldBroadcastCanceled ? GetPercent() : 0.0f;
	const float blendPercent = bShouldBroadcastCanceled ? GetLoopBlendPercent(GetLocalPercent()) : 0.0f;

	Super::Cancel();

	if (bShouldBroadcastCanceled)
	{
		BroadcastCanceled(percent, blendPercent);
	}
}

void UEggAsyncEasingAction::Complete()
{
	if (bIsRunning && ShouldBroadcastDelegates())
	{
		const float percent = GetPercent();

		SetReadyToDestroy();

		NotifyCompleted(percent);
	}
	else
	{
		SetReadyToDestroy();
	}
}

void UEggAsyncEasingAction::SetReadyToDestroy()
{
	if (WorldCleanupHandle.IsValid())
	{
		FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupHandle);
		WorldCleanupHandle.Reset();
	}
	bIsRunning = false;
	bTickable = false;

	SetTickableTickType(ETickableTickType::Never);

	Super::SetReadyToDestroy();
}

void UEggAsyncEasingAction::HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	if (WorldContextObject.Get() == World)
	{
		SetReadyToDestroy();
	}
}

void UEggAsyncEasingAction::InitializeBase(UWorld* InContextWorld, EEggEasingType InEasingType, float InExponential, float InLoopDuration, int32 InLoopCount, int32 InMaxUpdateRate, bool bInRoundTrip, bool bInTickableWhenPaused)
{
	WorldContextObject = InContextWorld;
	EasingType = InEasingType;
	Exponential = FMath::Max(InExponential, 0.0f);
	LoopDuration = FMath::Max(InLoopDuration, 0.0f);

	LoopCount = InLoopCount;
	MaxUpdateRate = InMaxUpdateRate;
	UpdateInterval = (MaxUpdateRate > 0) ? 1.0f / static_cast<float>(MaxUpdateRate) : 0.0f;
	bRoundTrip = bInRoundTrip;
	bTickableWhenPaused = bInTickableWhenPaused;
}

bool UEggAsyncEasingAction::ShouldUpdate(float DeltaTime)
{
	// MaxUpdateRate < 0 : Update 호출 제한 없음
	if (MaxUpdateRate < 0)
	{
		return true;
	}

	// MaxUpdateRate == 0 : Update 호출 불가, (Complete/Cancel만 호출합니다.)
	if (MaxUpdateRate == 0)
	{
		return false;
	}

	UpdateElapsedTime += DeltaTime;
	if (UpdateElapsedTime < UpdateInterval)
	{
		return false;
	}

	// UpdateInterval을 초과했다면, Update 이벤트 호출합니다.
	UpdateElapsedTime = FMath::Fmod(UpdateElapsedTime, UpdateInterval);
	return true;
}

void UEggAsyncEasingAction::NotifyCompleted(float percent)
{
	if (bIsBroadcastingCompleted == true)
	{
		return;
	}
	bIsBroadcastingCompleted = true;

	BroadcastCompleted(percent);

	bIsBroadcastingCompleted = false;
}

float UEggAsyncEasingAction::GetBlendValue(float percent)
{
	return EasingFunction(percent, EasingType, Exponential);
}

float UEggAsyncEasingAction::GetLocalPercent()
{
	// 현재 Loop의 진행률 (0.0 ~ 1.0)
	return (LoopDuration <= 0.0f) ? 1.0f : FMath::Clamp(ElapsedTime / LoopDuration, 0.0f, 1.0f);
}

float UEggAsyncEasingAction::GetLoopBlendPercent(float percent)
{
	// 단방향 액션(Begin -> End) : LocalPercent = BlendPercent
	// 왕복 액션(Begin -> End -> Begin) : 0.0 ~ 0.5 -> 0.0 ~ 1.0, 0.5 ~ 1.0 -> 1.0 ~ 0.0
	return bRoundTrip ? (percent < 0.5f) ? percent * 2.0f : (1.0f - percent) * 2.0f : percent;
}

float UEggAsyncEasingAction::GetPercent()
{
	// 현재 액션의 누적 진행률, N회 루프 시 0.0 ~ N.0 반환
	return static_cast<float>(CurrentLoopCount) + GetLocalPercent();
}

UEggAsyncEasingAction_Float* UEggAsyncEasingAction_Float::AsyncEasingAction_Float(const UObject* WorldContextObject, EEggEasingType InEasingType, float InExponential, float InLoopDuration, float InBegin, float InEnd, int32 InLoopCount, int32 InMaxUpdateRate, bool bInRoundTrip, bool InTickableWhenPaused)
{
	if (IsValid(GEngine) == false)
	{
		return nullptr;
	}

	UWorld* ContextWorld = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (IsValid(ContextWorld) == false)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = ContextWorld->GetGameInstance();
	if (IsValid(GameInstance) == false)
	{
		return nullptr;
	}

	UEggAsyncEasingAction_Float* NewAction = NewObject<UEggAsyncEasingAction_Float>(GameInstance);
	if (IsValid(NewAction) == false)
	{
		return nullptr;
	}

	NewAction->Begin = InBegin;
	NewAction->End = InEnd;
	NewAction->InitializeBase(ContextWorld, InEasingType, InExponential, InLoopDuration, InLoopCount, InMaxUpdateRate, bInRoundTrip, InTickableWhenPaused);
	NewAction->RegisterWithGameInstance(ContextWorld->GetGameInstance());

	return NewAction;
}

void UEggAsyncEasingAction_Float::BroadcastCanceled(float percent, float blendPercent)
{
	if (Canceled.IsBound())
	{
		float result = FMath::Lerp(Begin, End, GetBlendValue(blendPercent));
		Canceled.Broadcast(result, percent);
	}
}

void UEggAsyncEasingAction_Float::Blend(float percent, float blendPercent)
{
	if (Update.IsBound())
	{
		float result = FMath::Lerp(Begin, End, GetBlendValue(blendPercent));
		Update.Broadcast(result, percent);
	}
}

void UEggAsyncEasingAction_Float::BroadcastCompleted(float percent)
{
	if (Completed.IsBound())
	{
		float result = FMath::Lerp(Begin, End, GetBlendValue(GetLoopBlendPercent(1.0f)));
		Completed.Broadcast(result, percent);
	}
}

UEggAsyncEasingAction_Vector2D* UEggAsyncEasingAction_Vector2D::AsyncEasingAction_Vector2D(const UObject* WorldContextObject, EEggEasingType InEasingType, float InExponential, float InLoopDuration, FVector2D InBegin, FVector2D InEnd, int32 InLoopCount, int32 InMaxUpdateRate, bool bInRoundTrip, bool InTickableWhenPaused)
{
	if (IsValid(GEngine) == false)
	{
		return nullptr;
	}

	UWorld* ContextWorld = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (IsValid(ContextWorld) == false)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = ContextWorld->GetGameInstance();
	if (IsValid(GameInstance) == false)
	{
		return nullptr;
	}

	UEggAsyncEasingAction_Vector2D* NewAction = NewObject<UEggAsyncEasingAction_Vector2D>(GameInstance);
	if (IsValid(NewAction) == false)
	{
		return nullptr;
	}

	NewAction->Begin = InBegin;
	NewAction->End = InEnd;
	NewAction->InitializeBase(ContextWorld, InEasingType, InExponential, InLoopDuration, InLoopCount, InMaxUpdateRate, bInRoundTrip, InTickableWhenPaused);
	NewAction->RegisterWithGameInstance(ContextWorld->GetGameInstance());

	return NewAction;
}

void UEggAsyncEasingAction_Vector2D::BroadcastCanceled(float percent, float blendPercent)
{
	if (Canceled.IsBound())
	{
		FVector2D result = FMath::Lerp(Begin, End, GetBlendValue(blendPercent));
		Canceled.Broadcast(result, percent);
	}
}

void UEggAsyncEasingAction_Vector2D::Blend(float percent, float blendPercent)
{
	if (Update.IsBound())
	{
		FVector2D result = FMath::Lerp(Begin, End, GetBlendValue(blendPercent));
		Update.Broadcast(result, percent);
	}
}

void UEggAsyncEasingAction_Vector2D::BroadcastCompleted(float percent)
{
	if (Completed.IsBound())
	{
		FVector2D result = FMath::Lerp(Begin, End, GetBlendValue(GetLoopBlendPercent(1.0f)));
		Completed.Broadcast(result, percent);
	}
}

UEggAsyncEasingAction_Vector* UEggAsyncEasingAction_Vector::AsyncEasingAction_Vector(const UObject* WorldContextObject, EEggEasingType InEasingType, float InExponential, float InLoopDuration, FVector InBegin, FVector InEnd, int32 InLoopCount, int32 InMaxUpdateRate, bool bInRoundTrip, bool InTickableWhenPaused)
{
	if (IsValid(GEngine) == false)
	{
		return nullptr;
	}

	UWorld* ContextWorld = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (IsValid(ContextWorld) == false)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = ContextWorld->GetGameInstance();
	if (IsValid(GameInstance) == false)
	{
		return nullptr;
	}

	UEggAsyncEasingAction_Vector* NewAction = NewObject<UEggAsyncEasingAction_Vector>(GameInstance);
	if (IsValid(NewAction) == false)
	{
		return nullptr;
	}

	NewAction->Begin = InBegin;
	NewAction->End = InEnd;
	NewAction->InitializeBase(ContextWorld, InEasingType, InExponential, InLoopDuration, InLoopCount, InMaxUpdateRate, bInRoundTrip, InTickableWhenPaused);
	NewAction->RegisterWithGameInstance(ContextWorld->GetGameInstance());

	return NewAction;
}

void UEggAsyncEasingAction_Vector::BroadcastCanceled(float percent, float blendPercent)
{
	if (Canceled.IsBound())
	{
		FVector result = FMath::Lerp(Begin, End, GetBlendValue(blendPercent));
		Canceled.Broadcast(result, percent);
	}
}

void UEggAsyncEasingAction_Vector::Blend(float percent, float blendPercent)
{
	if (Update.IsBound())
	{
		FVector result = FMath::Lerp(Begin, End, GetBlendValue(blendPercent));
		Update.Broadcast(result, percent);
	}
}

void UEggAsyncEasingAction_Vector::BroadcastCompleted(float percent)
{
	if (Completed.IsBound())
	{
		FVector result = FMath::Lerp(Begin, End, GetBlendValue(GetLoopBlendPercent(1.0f)));
		Completed.Broadcast(result, percent);
	}
}

UEggAsyncEasingAction_Color* UEggAsyncEasingAction_Color::AsyncEasingAction_Color(const UObject* WorldContextObject, EEggEasingType InEasingType, float InExponential, float InLoopDuration, FLinearColor InBegin, FLinearColor InEnd, int32 InLoopCount, int32 InMaxUpdateRate, bool bInRoundTrip, bool InTickableWhenPaused)
{
	if (IsValid(GEngine) == false)
	{
		return nullptr;
	}

	UWorld* ContextWorld = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (IsValid(ContextWorld) == false)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = ContextWorld->GetGameInstance();
	if (IsValid(GameInstance) == false)
	{
		return nullptr;
	}

	UEggAsyncEasingAction_Color* NewAction = NewObject<UEggAsyncEasingAction_Color>(GameInstance);
	if (IsValid(NewAction) == false)
	{
		return nullptr;
	}

	NewAction->Begin = InBegin;
	NewAction->End = InEnd;
	NewAction->InitializeBase(ContextWorld, InEasingType, InExponential, InLoopDuration, InLoopCount, InMaxUpdateRate, bInRoundTrip, InTickableWhenPaused);
	NewAction->RegisterWithGameInstance(ContextWorld->GetGameInstance());

	return NewAction;
}

void UEggAsyncEasingAction_Color::BroadcastCanceled(float percent, float blendPercent)
{
	if (Canceled.IsBound())
	{
		FLinearColor result = FMath::Lerp(Begin, End, GetBlendValue(blendPercent));
		Canceled.Broadcast(result, percent);
	}
}

void UEggAsyncEasingAction_Color::Blend(float percent, float blendPercent)
{
	if (Update.IsBound())
	{
		FLinearColor result = FMath::Lerp(Begin, End, GetBlendValue(blendPercent));
		Update.Broadcast(result, percent);
	}
}

void UEggAsyncEasingAction_Color::BroadcastCompleted(float percent)
{
	if (Completed.IsBound())
	{
		FLinearColor result = FMath::Lerp(Begin, End, GetBlendValue(GetLoopBlendPercent(1.0f)));
		Completed.Broadcast(result, percent);
	}
}

UEggAsyncEasingAction_Rotator* UEggAsyncEasingAction_Rotator::AsyncEasingAction_Rotator(const UObject* WorldContextObject, EEggEasingType InEasingType, float InExponential, float InLoopDuration, FRotator InBegin, FRotator InEnd, int32 InLoopCount, int32 InMaxUpdateRate, bool bInRoundTrip, bool InTickableWhenPaused)
{
	if (IsValid(GEngine) == false)
	{
		return nullptr;
	}

	UWorld* ContextWorld = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (IsValid(ContextWorld) == false)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = ContextWorld->GetGameInstance();
	if (IsValid(GameInstance) == false)
	{
		return nullptr;
	}

	UEggAsyncEasingAction_Rotator* NewAction = NewObject<UEggAsyncEasingAction_Rotator>(GameInstance);
	if (IsValid(NewAction) == false)
	{
		return nullptr;
	}

	NewAction->Begin = InBegin;
	NewAction->End = InEnd;
	NewAction->InitializeBase(ContextWorld, InEasingType, InExponential, InLoopDuration, InLoopCount, InMaxUpdateRate, bInRoundTrip, InTickableWhenPaused);
	NewAction->RegisterWithGameInstance(ContextWorld->GetGameInstance());

	return NewAction;
}

void UEggAsyncEasingAction_Rotator::BroadcastCanceled(float percent, float blendPercent)
{
	if (Canceled.IsBound())
	{
		FQuat rotation = FQuat::Slerp(FQuat(Begin), FQuat(End), GetBlendValue(blendPercent));
		Canceled.Broadcast(rotation.Rotator(), percent);
	}
}

void UEggAsyncEasingAction_Rotator::Blend(float percent, float blendPercent)
{
	if (Update.IsBound())
	{
		FQuat BeginQuat = FQuat(Begin);
		FQuat EndQuat = FQuat(End);
		FQuat ResultQuat = FQuat::Slerp(BeginQuat, EndQuat, GetBlendValue(blendPercent));
		Update.Broadcast(ResultQuat.Rotator(), percent);
	}
}

void UEggAsyncEasingAction_Rotator::BroadcastCompleted(float percent)
{
	if (Completed.IsBound())
	{
		FQuat rotation = FQuat::Slerp(FQuat(Begin), FQuat(End), GetBlendValue(GetLoopBlendPercent(1.0f)));
		Completed.Broadcast(rotation.Rotator(), percent);
	}
}
