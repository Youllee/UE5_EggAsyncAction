#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "EggAsyncTypes.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEggAsyncDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEggAsyncDelegate_InputActionValue, const FInputActionValue&, InputActionValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEggAsyncDelegate_Float, float, value, float, percent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEggAsyncDelegate_Vector2D, FVector2D, value, float, percent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEggAsyncDelegate_Vector, FVector, value, float, percent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEggAsyncDelegate_Color, FLinearColor, value, float, percent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEggAsyncDelegate_Rotator, FRotator, value, float, percent);

UENUM(BlueprintType)
enum class EEggEasingType : uint8
{
//	None,
	Linear,
	Step,
	EaseIn,
	EaseOut,
	EaseInOut,
	EaseOutIn,
	SineIn,
	SineOut,
	SineInOut,
	SineOutIn,
	CircleIn,
	CircleOut,
	CircleInOut,
	CircleOutIn,
	ElasticIn,
	ElasticOut,
	ElasticInOut,
	ElasticOutIn,
	BackIn,
	BackOut,
	BackInOut,
	BackOutIn,
	BounceIn,
	BounceOut,
	BounceInOut,
	BounceOutIn,
};
