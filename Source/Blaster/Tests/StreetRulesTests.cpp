#include "Misc/AutomationTest.h"
#include "Blaster/BlasterComponent/BlasterHitRules.h"
#include "Blaster/BlasterComponent/InventoryComponent.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStreetCombatRules, "Blaster.Street.CombatAndInventoryBoundaries",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FStreetCombatRules::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Head bone comparison survives imported capitalization"), BlasterHit::IsHead(TEXT("Head")));
	TestFalse(TEXT("Neck is not head"), BlasterHit::IsHead(TEXT("neck_01")));
	TestFalse(TEXT("Missing bone is not head"), BlasterHit::IsHead(NAME_None));
	TestEqual(TEXT("Mixed shotgun: two head + six body pellets = 40 damage"),
		2*BlasterHit::Damage(4,2,true)+6*BlasterHit::Damage(4,2,false),40.f);
	TestEqual(TEXT("Negative damage cannot heal an enemy"),BlasterHit::Damage(-10,2,true),0.f);
	TestEqual(TEXT("Invalid multiplier cannot reduce head damage"),BlasterHit::Damage(20,-2,true),20.f);
	TestTrue(TEXT("Last inventory slot accepts a pickup"),UInventoryComponent::CanAdd(2));
	TestFalse(TEXT("Full inventory leaves world pickup intact"),UInventoryComponent::CanAdd(3));
	TestFalse(TEXT("Corrupt negative stock rejected"),UInventoryComponent::CanAdd(-1));
	return true;
}
#endif
