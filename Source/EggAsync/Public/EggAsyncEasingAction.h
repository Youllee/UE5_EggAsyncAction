#pragma once

#include "CoreMinimal.h"
#include "Engine/CancellableAsyncAction.h"
#include "Tickable.h"
#include "EggAsyncTypes.h"
#include "EggAsyncEasingAction.generated.h"

UCLASS(Abstract)
class EGGASYNC_API UEggAsyncEasingAction : public UCancellableAsyncAction, public FTickableGameObject
{
	GENERATED_BODY()

public:
	UEggAsyncEasingAction();

public:
	virtual void Tick(float DeltaTime) override;
	bool IsTickable() const override { return bTickable; }
	bool IsTickableInEditor() const override { return false; }
	bool IsTickableWhenPaused() const override { return bTickableWhenPaused; }
	UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }
	TStatId GetStatId() const override;

public:
	virtual void Activate() override;
	// 실행 중인 액션을 취소하고 Canceled 이벤트를 한 번 전달합니다.
	virtual void Cancel() override;
	// 실행 중인 액션을 완료하고 Completed 이벤트를 한 번 전달합니다.
	UFUNCTION(BlueprintCallable, Category = "Async Action")
	virtual void Complete();
	virtual void SetReadyToDestroy() override;

private:
	void HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);
	FDelegateHandle WorldCleanupHandle;

protected:
	// 각 타입별 AsyncEasingAction_* 함수에서 공통 설정을 초기화하기 위한 함수입니다.
	void InitializeBase(UWorld* InContextWorld, EEggEasingType InEasingType, float InExponential, float InLoopDuration, int32 InLoopCount, int32 InMaxUpdateRate, bool bInRoundTrip, bool bInTickableWhenPaused);
	// 이번 Tick에서 Update 델리게이트를 호출할지 판단합니다.
	bool ShouldUpdate(float DeltaTime);
	// 비동기 액션이 완료되었음을 알리는 함수입니다. Complete()에서 호출됩니다.
	void NotifyCompleted(float percent);
	// EasingType과 Exponential을 적용해 보간용 값(0.0 ~ 1.0)을 반환합니다.
	float GetBlendValue(float percent);
	// 현재 Loop의 순수 진행률입니다. (0.0 ~ 1.0)
	float GetLocalPercent();
	// 실제 Begin/End 보간에 사용할 percent를 계산합니다.
	float GetLoopBlendPercent(float percent);

	virtual void Blend(float percent, float blendPercent) {};
	virtual void BroadcastCanceled(float percent, float blendPercent) {};
	virtual void BroadcastCompleted(float percent) {};

public:
	// 현재 Action의 누적 진행률입니다. N회 루프 시 0.0 ~ N.0 사이의 값을 반환합니다.
	UFUNCTION(BlueprintPure, Category = "EggAsync")
	float GetPercent();
	// 현재 Action이 진행 중인지 여부를 반환합니다.
	UFUNCTION(BlueprintPure, Category = "EggAsync")
	bool IsRunning() { return bIsRunning; };

	virtual UWorld* GetWorld() const override { return WorldContextObject.IsValid() ? WorldContextObject.Get() : nullptr; }

	// Input Datas
protected:
	EEggEasingType EasingType = EEggEasingType::Linear;
	// Exponential Easing에서 사용할 지수 값입니다. 0보다 작으면 0으로 처리합니다.
	float Exponential = 2.0f;
	// 루프 1회를 수행하는 데 걸리는 시간입니다. 0이면 Tick마다 루프를 완료합니다.
	float LoopDuration = 1.0f;
	// 진행할 루프 횟수입니다. 0이면 실행하지 않고, 음수이면 무한히 반복합니다.
	int32 LoopCount = 1;
	// true이면 Begin -> End -> Begin을 하나의 루프로 취급합니다.
	bool bRoundTrip = false;
	// 초당 최대 Update 호출 횟수입니다. 0이면 Update를 호출하지 않습니다. 0보다 작으면 제한없이 Update를 호출합니다.
	int32 MaxUpdateRate = 60;
	// true이면 게임이 Pause 상태일 때도 Tick을 수행합니다.
	bool bTickableWhenPaused = false;

protected:
	TWeakObjectPtr<UWorld> WorldContextObject = nullptr;
	// 현재 액션이 실행 중인지 여부입니다.
	bool bIsRunning = false;
	// Tickable 여부입니다.
	bool bTickable = false;
	// Completed 재진입 호출을 방지하기 위한 플래그입니다.
	bool bIsBroadcastingCompleted = false;
	// 현재 Loop 안에서 흐른 시간입니다. Loop가 시작될 때마다 0으로 초기화됩니다.
	float ElapsedTime = 0.0f;
	// MaxUpdateRate 제한을 위한 별도 누적 시간입니다.
	float UpdateElapsedTime = 0.0f;
	// MaxUpdateRate 제한에 따른, Update 호출 주기입니다.
	float UpdateInterval = 0.0f;
	// 현재까지 Loop가 완료된 횟수입니다.
	int32 CurrentLoopCount = 0;

};

UCLASS()
class EGGASYNC_API UEggAsyncEasingAction_Float : public UEggAsyncEasingAction
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintAssignable)
	FEggAsyncDelegate_Float Update;
	UPROPERTY(BlueprintAssignable)
	FEggAsyncDelegate_Float Completed;
	UPROPERTY(BlueprintAssignable)
	FEggAsyncDelegate_Float Canceled;
	UPROPERTY(BlueprintReadOnly, Category = "EggAsync")
	float Begin;
	UPROPERTY(BlueprintReadOnly, Category = "EggAsync")
	float End;

public:
	/**
	 * float 값의 비동기 Easing 액션을 시작합니다.
	 * @param InEasingType 사용할 Easing 곡선의 종류입니다.
	 * @param InExponential 지수 기반 Easing의 지수입니다. Step 곡선에서는 가장 가까운 정수로 반올림한 단계 수(최소 1)로 사용합니다.
	 * @param InLoopDuration Easing 루프 1회의 실행 시간(초)입니다.
	 * @param InBegin 시작 값입니다.
	 * @param InEnd 종료 값입니다.
	 * @param InLoopCount Easing 루프를 반복할 횟수입니다.
	 * @param InMaxUpdateRate 초당 최대 Update 이벤트 호출 횟수입니다. 0이면 Update 이벤트를 호출하지 않고, 음수이면 매 Tick마다 호출합니다.
	 * @param bInRoundTrip true이면 Begin에서 End까지 진행한 뒤 다시 Begin으로 돌아옵니다.
	 * @param InTickableWhenPaused true이면 게임이 일시 정지된 동안에도 액션의 Tick을 실행합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "EggAsync", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Async Easing Action (Float)"
		, HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay = "InLoopCount, InMaxUpdateRate, bInRoundTrip, InTickableWhenPaused"))
	static UEggAsyncEasingAction_Float* AsyncEasingAction_Float(const UObject* WorldContextObject
		, EEggEasingType InEasingType, float InExponential = 2.0f, float InLoopDuration = 1.0f, float InBegin = 0.0f, float InEnd = 1.0f
		, int32 InLoopCount = 1, int32 InMaxUpdateRate = 60, bool bInRoundTrip = false, bool InTickableWhenPaused = false);

protected:
	virtual void Blend(float percent, float blendPercent) override;
	virtual void BroadcastCanceled(float percent, float blendPercent) override;
	virtual void BroadcastCompleted(float percent) override;

};

UCLASS()
class EGGASYNC_API UEggAsyncEasingAction_Vector2D : public UEggAsyncEasingAction
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintAssignable)
	FEggAsyncDelegate_Vector2D Update;
	UPROPERTY(BlueprintAssignable)
	FEggAsyncDelegate_Vector2D Completed;
	UPROPERTY(BlueprintAssignable)
	FEggAsyncDelegate_Vector2D Canceled;
	UPROPERTY(BlueprintReadOnly, Category = "EggAsync")
	FVector2D Begin;
	UPROPERTY(BlueprintReadOnly, Category = "EggAsync")
	FVector2D End;

public:
	/**
	 * FVector2D 값의 비동기 Easing 액션을 시작합니다.
	 * @param InEasingType 사용할 Easing 곡선의 종류입니다.
	 * @param InExponential 지수 기반 Easing의 지수입니다. Step 곡선에서는 가장 가까운 정수로 반올림한 단계 수(최소 1)로 사용합니다.
	 * @param InLoopDuration Easing 루프 1회의 실행 시간(초)입니다.
	 * @param InBegin 시작 값입니다.
	 * @param InEnd 종료 값입니다.
	 * @param InLoopCount Easing 루프를 반복할 횟수입니다.
	 * @param InMaxUpdateRate 초당 최대 Update 이벤트 호출 횟수입니다. 0이면 Update 이벤트를 호출하지 않고, 음수이면 매 Tick마다 호출합니다.
	 * @param bInRoundTrip true이면 Begin에서 End까지 진행한 뒤 다시 Begin으로 돌아옵니다.
	 * @param InTickableWhenPaused true이면 게임이 일시 정지된 동안에도 액션의 Tick을 실행합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "EggAsync", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Async Easing Action (Vector2D)"
		, HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay = "InLoopCount, InMaxUpdateRate, bInRoundTrip, InTickableWhenPaused"))
	static UEggAsyncEasingAction_Vector2D* AsyncEasingAction_Vector2D(const UObject* WorldContextObject
		, EEggEasingType InEasingType, float InExponential = 2.0f, float InLoopDuration = 1.0f, FVector2D InBegin = FVector2D::ZeroVector, FVector2D InEnd = FVector2D::ZeroVector
		, int32 InLoopCount = 1, int32 InMaxUpdateRate = 60, bool bInRoundTrip = false, bool InTickableWhenPaused = false);

protected:
	virtual void Blend(float percent, float blendPercent) override;
	virtual void BroadcastCanceled(float percent, float blendPercent) override;
	virtual void BroadcastCompleted(float percent) override;

};

UCLASS()
class EGGASYNC_API UEggAsyncEasingAction_Vector : public UEggAsyncEasingAction
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintAssignable)
	FEggAsyncDelegate_Vector Update;
	UPROPERTY(BlueprintAssignable)
	FEggAsyncDelegate_Vector Completed;
	UPROPERTY(BlueprintAssignable)
	FEggAsyncDelegate_Vector Canceled;
	UPROPERTY(BlueprintReadOnly, Category = "EggAsync")
	FVector Begin;
	UPROPERTY(BlueprintReadOnly, Category = "EggAsync")
	FVector End;

public:
	/**
	 * FVector 값의 비동기 Easing 액션을 시작합니다.
	 * @param InEasingType 사용할 Easing 곡선의 종류입니다.
	 * @param InExponential 지수 기반 Easing의 지수입니다. Step 곡선에서는 가장 가까운 정수로 반올림한 단계 수(최소 1)로 사용합니다.
	 * @param InLoopDuration Easing 루프 1회의 실행 시간(초)입니다.
	 * @param InBegin 시작 값입니다.
	 * @param InEnd 종료 값입니다.
	 * @param InLoopCount Easing 루프를 반복할 횟수입니다.
	 * @param InMaxUpdateRate 초당 최대 Update 이벤트 호출 횟수입니다. 0이면 Update 이벤트를 호출하지 않고, 음수이면 매 Tick마다 호출합니다.
	 * @param bInRoundTrip true이면 Begin에서 End까지 진행한 뒤 다시 Begin으로 돌아옵니다.
	 * @param InTickableWhenPaused true이면 게임이 일시 정지된 동안에도 액션의 Tick을 실행합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "EggAsync", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Async Easing Action (Vector)"
		, HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay = "InLoopCount, InMaxUpdateRate, bInRoundTrip, InTickableWhenPaused"))
	static UEggAsyncEasingAction_Vector* AsyncEasingAction_Vector(const UObject* WorldContextObject
		, EEggEasingType InEasingType, float InExponential = 2.0f, float InLoopDuration = 1.0f, FVector InBegin = FVector::ZeroVector, FVector InEnd = FVector::ZeroVector
		, int32 InLoopCount = 1, int32 InMaxUpdateRate = 60, bool bInRoundTrip = false, bool InTickableWhenPaused = false);

protected:
	virtual void Blend(float percent, float blendPercent) override;
	virtual void BroadcastCanceled(float percent, float blendPercent) override;
	virtual void BroadcastCompleted(float percent) override;

};

UCLASS()
class EGGASYNC_API UEggAsyncEasingAction_Color : public UEggAsyncEasingAction
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintAssignable)
	FEggAsyncDelegate_Color Update;
	UPROPERTY(BlueprintAssignable)
	FEggAsyncDelegate_Color Completed;
	UPROPERTY(BlueprintAssignable)
	FEggAsyncDelegate_Color Canceled;
	UPROPERTY(BlueprintReadOnly, Category = "EggAsync")
	FLinearColor Begin;
	UPROPERTY(BlueprintReadOnly, Category = "EggAsync")
	FLinearColor End;

public:
	/**
	 * FLinearColor 값의 비동기 Easing 액션을 시작합니다.
	 * @param InEasingType 사용할 Easing 곡선의 종류입니다.
	 * @param InExponential 지수 기반 Easing의 지수입니다. Step 곡선에서는 가장 가까운 정수로 반올림한 단계 수(최소 1)로 사용합니다.
	 * @param InLoopDuration Easing 루프 1회의 실행 시간(초)입니다.
	 * @param InBegin 시작 값입니다.
	 * @param InEnd 종료 값입니다.
	 * @param InLoopCount Easing 루프를 반복할 횟수입니다.
	 * @param InMaxUpdateRate 초당 최대 Update 이벤트 호출 횟수입니다. 0이면 Update 이벤트를 호출하지 않고, 음수이면 매 Tick마다 호출합니다.
	 * @param bInRoundTrip true이면 Begin에서 End까지 진행한 뒤 다시 Begin으로 돌아옵니다.
	 * @param InTickableWhenPaused true이면 게임이 일시 정지된 동안에도 액션의 Tick을 실행합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "EggAsync", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Async Easing Action (Color)"
		, HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay = "InLoopCount, InMaxUpdateRate, bInRoundTrip, InTickableWhenPaused"))
	static UEggAsyncEasingAction_Color* AsyncEasingAction_Color(const UObject* WorldContextObject
		, EEggEasingType InEasingType, float InExponential = 2.0f, float InLoopDuration = 1.0f, FLinearColor InBegin = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f), FLinearColor InEnd = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f)
		, int32 InLoopCount = 1, int32 InMaxUpdateRate = 60, bool bInRoundTrip = false, bool InTickableWhenPaused = false);

protected:
	virtual void Blend(float percent, float blendPercent) override;
	virtual void BroadcastCanceled(float percent, float blendPercent) override;
	virtual void BroadcastCompleted(float percent) override;

};

UCLASS()
class EGGASYNC_API UEggAsyncEasingAction_Rotator : public UEggAsyncEasingAction
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintAssignable)
	FEggAsyncDelegate_Rotator Update;
	UPROPERTY(BlueprintAssignable)
	FEggAsyncDelegate_Rotator Completed;
	UPROPERTY(BlueprintAssignable)
	FEggAsyncDelegate_Rotator Canceled;
	UPROPERTY(BlueprintReadOnly, Category = "EggAsync")
	FRotator Begin;
	UPROPERTY(BlueprintReadOnly, Category = "EggAsync")
	FRotator End;

public:
	/**
	 * FRotator 값의 비동기 Easing 액션을 시작합니다.
	 * @param InEasingType 사용할 Easing 곡선의 종류입니다.
	 * @param InExponential 지수 기반 Easing의 지수입니다. Step 곡선에서는 가장 가까운 정수로 반올림한 단계 수(최소 1)로 사용합니다.
	 * @param InLoopDuration Easing 루프 1회의 실행 시간(초)입니다.
	 * @param InBegin 시작 값입니다.
	 * @param InEnd 종료 값입니다.
	 * @param InLoopCount Easing 루프를 반복할 횟수입니다.
	 * @param InMaxUpdateRate 초당 최대 Update 이벤트 호출 횟수입니다. 0이면 Update 이벤트를 호출하지 않고, 음수이면 매 Tick마다 호출합니다.
	 * @param bInRoundTrip true이면 Begin에서 End까지 진행한 뒤 다시 Begin으로 돌아옵니다.
	 * @param InTickableWhenPaused true이면 게임이 일시 정지된 동안에도 액션의 Tick을 실행합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "EggAsync", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Async Easing Action (Rotator)"
		, HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", AdvancedDisplay = "InLoopCount, InMaxUpdateRate, bInRoundTrip, InTickableWhenPaused"))
	static UEggAsyncEasingAction_Rotator* AsyncEasingAction_Rotator(const UObject* WorldContextObject
		, EEggEasingType InEasingType, float InExponential = 2.0f, float InLoopDuration = 1.0f, FRotator InBegin = FRotator::ZeroRotator, FRotator InEnd = FRotator::ZeroRotator
		, int32 InLoopCount = 1, int32 InMaxUpdateRate = 60, bool bInRoundTrip = false, bool InTickableWhenPaused = false);

protected:
	virtual void Blend(float percent, float blendPercent) override;
	virtual void BroadcastCanceled(float percent, float blendPercent) override;
	virtual void BroadcastCompleted(float percent) override;

};
