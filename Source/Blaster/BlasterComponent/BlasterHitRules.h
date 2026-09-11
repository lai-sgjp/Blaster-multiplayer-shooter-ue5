#pragma once

#include "CoreMinimal.h"

// Dedicated trace channel. World geometry blocks by default; pawn capsules ignore it.
namespace BlasterHit
{
constexpr ECollisionChannel Channel = ECC_GameTraceChannel1;
inline bool IsHead(FName Bone) { return Bone == FName(TEXT("head")); }
inline float Damage(float Body, float Multiplier, bool bHead)
{
	return FMath::Max(0.f, Body) * (bHead ? FMath::Max(1.f, Multiplier) : 1.f);
}
}
