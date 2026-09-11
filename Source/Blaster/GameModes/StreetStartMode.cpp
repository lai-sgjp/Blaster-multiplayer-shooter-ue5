#include "StreetStartMode.h"
#include "Blaster/Character/BlasterPlayerController.h"
AStreetStartMode::AStreetStartMode()
{
	PlayerControllerClass = ABlasterPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
}
