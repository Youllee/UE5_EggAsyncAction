#include "EggAsyncAction.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "Kismet/GameplayStatics.h"
#include "Stats/Stats.h"

namespace
{
	UEnhancedInputComponent* GetEnhancedInputComponent(const UObject* WorldContextObject, int32 PlayerIndex/* = 0*/)
	{
		if (WorldContextObject == nullptr || PlayerIndex < 0)
		{
			return nullptr;
		}

		const UWorld* World = (GEngine != nullptr) ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
		if (IsValid(World) == false)
		{
			return nullptr;
		}

		// PlayerIndex에 해당하는 PlayerController를 가져옵니다.
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, PlayerIndex);
		if (IsValid(PlayerController) == false)
		{
			return nullptr;
		}

		// PlayerController의 InputComponent가 UEnhancedInputComponent인지 확인합니다.
		UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
		if (IsValid(EnhancedInputComponent) == false)
		{
			// PlayerController의 InputComponent가 UEnhancedInputComponent가 아닌 경우, Pawn의 InputComponent를 확인합니다.
			const APawn* Pawn = PlayerController->GetPawn();
			EnhancedInputComponent = IsValid(Pawn) ? Cast<UEnhancedInputComponent>(Pawn->InputComponent) : nullptr;
		}

		return EnhancedInputComponent;
	}
}

UEggAsyncCancelableDelay::UEggAsyncCancelableDelay()
	: FTickableGameObject(ETickableTickType::Never)
{

}

void UEggAsyncCancelableDelay::Tick(float DeltaTime)
{
	if (bTickable == false)
	{
		return;
	}

	if (WorldContextObject.IsValid() == false)
	{
		SetReadyToDestroy();
		return;
	}

	ElapsedTime += DeltaTime;
	if (ElapsedTime >= Duration)
	{
		const bool bShouldBroadcast = ShouldBroadcastDelegates();
		SetReadyToDestroy();
		if (bShouldBroadcast)
		{
			Completed.Broadcast();
		}
	}
}

TStatId UEggAsyncCancelableDelay::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UEggAsyncCancelableDelay, STATGROUP_Tickables);
}

UEggAsyncCancelableDelay* UEggAsyncCancelableDelay::CancelableDelay(UObject* WorldContextObject, float InDuration, bool InTickableWhenPaused/*= false*/)
{
	UWorld* ContextWorld = (GEngine != nullptr) ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (IsValid(ContextWorld) == false)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = ContextWorld->GetGameInstance();
	if (IsValid(GameInstance) == false)
	{
		return nullptr;
	}

	UEggAsyncCancelableDelay* CancelableDelay = NewObject<UEggAsyncCancelableDelay>();
	if (IsValid(CancelableDelay) == false)
	{
		return nullptr;
	}

	CancelableDelay->WorldContextObject = ContextWorld;
	CancelableDelay->Duration = InDuration;
	CancelableDelay->bTickableWhenPaused = InTickableWhenPaused;
	CancelableDelay->RegisterWithGameInstance(GameInstance);

	return CancelableDelay;
}

void UEggAsyncCancelableDelay::Activate()
{
	if (bTickable)
	{
		return;
	}

	Super::Activate();

	if (WorldContextObject.IsValid() == false || ShouldBroadcastDelegates() == false)
	{
		SetReadyToDestroy();
		return;
	}

	ElapsedTime = 0.0f;
	WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddUObject(this, &UEggAsyncCancelableDelay::HandleWorldCleanup);
	bTickable = true;
	SetTickableTickType(ETickableTickType::Conditional);
}

void UEggAsyncCancelableDelay::Cancel()
{
	const bool bShouldBroadcast = bTickable && ShouldBroadcastDelegates() && HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed) == false;

	Super::Cancel();

	if (bShouldBroadcast)
	{
		Canceled.Broadcast();
	}
}

void UEggAsyncCancelableDelay::SetReadyToDestroy()
{
	if (WorldCleanupHandle.IsValid())
	{
		FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupHandle);
		WorldCleanupHandle.Reset();
	}

	bTickable = false;
	SetTickableTickType(ETickableTickType::Never);
	WorldContextObject.Reset();

	Super::SetReadyToDestroy();
}

void UEggAsyncCancelableDelay::HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	if (WorldContextObject.Get() == World)
	{
		SetReadyToDestroy();
	}
}

UEggAsyncInputAction* UEggAsyncInputAction::BindInputAction(UObject* WorldContextObject, UInputAction* InInputAction, int32 InPlayerIndex)
{
	UWorld* ContextWorld = (GEngine != nullptr) ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (IsValid(ContextWorld) == false)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = ContextWorld->GetGameInstance();
	if (IsValid(GameInstance) == false)
	{
		return nullptr;
	}

	if (IsValid(InInputAction) == false || InPlayerIndex < 0)
	{
		return nullptr;
	}

	UEggAsyncInputAction* AsyncInputAction = NewObject<UEggAsyncInputAction>();
	if (IsValid(AsyncInputAction) == false)
	{
		return nullptr;
	}

	AsyncInputAction->WorldContextObject = ContextWorld;
	AsyncInputAction->InputAction = InInputAction;
	AsyncInputAction->PlayerIndex = InPlayerIndex;
	AsyncInputAction->RegisterWithGameInstance(GameInstance);

	return AsyncInputAction;
}

void UEggAsyncInputAction::Activate()
{
	if (WorldCleanupHandle.IsValid())
	{
		return;
	}
	Super::Activate();

	if (ShouldBroadcastDelegates() == false || IsValid(InputAction) == false || BindInputAction() == false)
	{
		SetReadyToDestroy();
		return;
	}
	WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddUObject(this, &UEggAsyncInputAction::HandleWorldCleanup);
}

void UEggAsyncInputAction::SetReadyToDestroy()
{
	if (WorldCleanupHandle.IsValid())
	{
		FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupHandle);
		WorldCleanupHandle.Reset();
	}
	UnbindInputAction();

	Triggered.Clear();
	Started.Clear();
	Ongoing.Clear();
	Canceled.Clear();
	Completed.Clear();

	WorldContextObject = nullptr;
	InputAction = nullptr;
	PlayerIndex = INDEX_NONE;

	Super::SetReadyToDestroy();
}

void UEggAsyncInputAction::HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	if (WorldContextObject.Get() == World)
	{
		SetReadyToDestroy();
	}
}

bool UEggAsyncInputAction::BindInputAction()
{
	UnbindInputAction();

	if (WorldContextObject == nullptr || IsValid(InputAction) == false || PlayerIndex < 0)
	{
		return false;
	}

	UEnhancedInputComponent* EnhancedInputComponent = GetEnhancedInputComponent(WorldContextObject.Get(), PlayerIndex);
	if (IsValid(EnhancedInputComponent) == false)
	{
		return false;
	}

	BindHandles.Add(EnhancedInputComponent->BindAction(InputAction.Get(), ETriggerEvent::Triggered, this, &UEggAsyncInputAction::HandleTriggered).GetHandle());
	BindHandles.Add(EnhancedInputComponent->BindAction(InputAction.Get(), ETriggerEvent::Started, this, &UEggAsyncInputAction::HandleStarted).GetHandle());
	BindHandles.Add(EnhancedInputComponent->BindAction(InputAction.Get(), ETriggerEvent::Ongoing, this, &UEggAsyncInputAction::HandleOngoing).GetHandle());
	BindHandles.Add(EnhancedInputComponent->BindAction(InputAction.Get(), ETriggerEvent::Canceled, this, &UEggAsyncInputAction::HandleCanceled).GetHandle());
	BindHandles.Add(EnhancedInputComponent->BindAction(InputAction.Get(), ETriggerEvent::Completed, this, &UEggAsyncInputAction::HandleCompleted).GetHandle());

	CachedInputComponent = EnhancedInputComponent;

	return true;
}

void UEggAsyncInputAction::UnbindInputAction()
{
	UEnhancedInputComponent* EnhancedInputComponent = CachedInputComponent.Get();
	if (IsValid(EnhancedInputComponent))
	{
		for (const uint32 BindHandle : BindHandles)
		{
			EnhancedInputComponent->RemoveBindingByHandle(BindHandle);
		}
	}
	BindHandles.Reset();

	CachedInputComponent = nullptr;
}

void UEggAsyncInputAction::HandleTriggered(const FInputActionValue& InputActionValue)
{
	if (bPaused == false && ShouldBroadcastDelegates())
	{
		Triggered.Broadcast(InputActionValue);
	}
}

void UEggAsyncInputAction::HandleStarted(const FInputActionValue& InputActionValue)
{
	if (bPaused == false && ShouldBroadcastDelegates())
	{
		Started.Broadcast(InputActionValue);
	}
}

void UEggAsyncInputAction::HandleOngoing(const FInputActionValue& InputActionValue)
{
	if (bPaused == false && ShouldBroadcastDelegates())
	{
		Ongoing.Broadcast(InputActionValue);
	}
}

void UEggAsyncInputAction::HandleCanceled(const FInputActionValue& InputActionValue)
{
	if (bPaused == false && ShouldBroadcastDelegates())
	{
		Canceled.Broadcast(InputActionValue);
	}
}

void UEggAsyncInputAction::HandleCompleted(const FInputActionValue& InputActionValue)
{
	if (bPaused == false && ShouldBroadcastDelegates())
	{
		Completed.Broadcast(InputActionValue);
	}
}
