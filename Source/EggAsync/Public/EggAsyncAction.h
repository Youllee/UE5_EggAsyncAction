#pragma once

#include "CoreMinimal.h"
#include "Engine/CancellableAsyncAction.h"
#include "InputActionValue.h"
#include "Tickable.h"
#include "EggAsyncTypes.h"
#include "EggAsyncAction.generated.h"

class UEnhancedInputComponent;
class UInputAction;

UCLASS()
class EGGASYNC_API UEggAsyncCancelableDelay : public UCancellableAsyncAction, public FTickableGameObject
{
	GENERATED_BODY()

	UEggAsyncCancelableDelay();

public:
	virtual void Tick(float DeltaTime) override;
	bool IsTickable() const override { return bTickable; }
	bool IsTickableInEditor() const override { return false; }
	bool IsTickableWhenPaused() const override { return bTickableWhenPaused; }
	UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }
	TStatId GetStatId() const override;
	virtual UWorld* GetWorld() const override { return WorldContextObject.Get(); }

public:
	UPROPERTY(BlueprintAssignable, Category = "EggAsync")
	FEggAsyncDelegate Completed;

	UPROPERTY(BlueprintAssignable, Category = "EggAsync")
	FEggAsyncDelegate Canceled;

public:
	/**
	 * 지정한 시간이 지나면 Completed를 전달합니다. 실행 중 Cancel을 호출하면 Canceled를 전달합니다.
	 * 시간이 0 이하이면 다음 Tick에서 완료합니다.
	 * @param WorldContextObject 실행할 월드를 찾는 컨텍스트입니다.
	 * @param InDuration 대기할 시간(초)입니다.
	 * @param InTickableWhenPaused true이면 게임 일시 정지 중에도 시간을 진행합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "EggAsync"
		, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", DisplayName = "Cancelable Delay (Async)", AdvancedDisplay = "InTickableWhenPaused"))
	static UEggAsyncCancelableDelay* CancelableDelay(UObject* WorldContextObject, float InDuration, bool InTickableWhenPaused = false);

public:
	virtual void Activate() override;
	virtual void Cancel() override;
	virtual void SetReadyToDestroy() override;

private:
	void HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

	UPROPERTY(Transient)
	TWeakObjectPtr<UWorld> WorldContextObject;

	FDelegateHandle WorldCleanupHandle;
	bool bTickable = false;
	bool bTickableWhenPaused = false;
	float Duration = 0.0f;
	float ElapsedTime = 0.0f;

};

UCLASS()
class EGGASYNC_API UEggAsyncInputAction : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "EggAsync")
	FEggAsyncDelegate_InputActionValue Triggered;
	UPROPERTY(BlueprintAssignable, Category = "EggAsync")
	FEggAsyncDelegate_InputActionValue Started;
	UPROPERTY(BlueprintAssignable, Category = "EggAsync")
	FEggAsyncDelegate_InputActionValue Ongoing;
	UPROPERTY(BlueprintAssignable, Category = "EggAsync")
	FEggAsyncDelegate_InputActionValue Canceled;
	UPROPERTY(BlueprintAssignable, Category = "EggAsync")
	FEggAsyncDelegate_InputActionValue Completed;

public:
	/**
	 * 현재 플레이어 컨트롤러 또는 폰의 Enhanced Input 컴포넌트에 입력 액션을 바인딩합니다.
	 * 컴포넌트가 교체되면 새 컴포넌트에 자동으로 다시 바인딩되지 않습니다.
	 * @param WorldContextObject 플레이어와 월드를 찾는 컨텍스트입니다.
	 * @param InInputAction 관찰할 입력 액션입니다.
	 * @param InPlayerIndex 로컬 플레이어 인덱스입니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "EggAsync"
		, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", DisplayName = "Bind Input Action (Async)"))
	static UEggAsyncInputAction* BindInputAction(UObject* WorldContextObject, UInputAction* InInputAction, int32 InPlayerIndex = 0);

public:
	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;

protected:
	bool BindInputAction();
	void UnbindInputAction();

private:
	void HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);
	void HandleTriggered(const FInputActionValue& InputActionValue);
	void HandleStarted(const FInputActionValue& InputActionValue);
	void HandleOngoing(const FInputActionValue& InputActionValue);
	void HandleCanceled(const FInputActionValue& InputActionValue);
	void HandleCompleted(const FInputActionValue& InputActionValue);

public:
	// 이 액션의 이벤트 전달만 일시 중지합니다. 입력 바인딩은 유지됩니다.
	UFUNCTION(BlueprintCallable, Category = "EggAsync")
	void SetPause(bool bInPaused) { bPaused = bInPaused; }
	// 이 액션의 이벤트 전달이 일시 중지되었는지 반환합니다.
	UFUNCTION(BlueprintPure, Category = "EggAsync")
	bool IsPaused() const { return bPaused; }

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UObject> WorldContextObject;
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> InputAction;
	UPROPERTY(Transient)
	int32 PlayerIndex = INDEX_NONE;
	UPROPERTY(Transient)
	TWeakObjectPtr<UEnhancedInputComponent> CachedInputComponent;

	TArray<uint32> BindHandles;
	FDelegateHandle WorldCleanupHandle;

	bool bPaused = false;

};
