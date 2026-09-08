#include "BlasterAnimationEditorLibrary.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimBlueprintGeneratedClass.h"
#include "AnimGraphNode_StateMachine.h"
#include "AnimGraphNode_SaveCachedPose.h"
#include "AnimGraphNode_UseCachedPose.h"
#include "AnimGraphNode_SequenceEvaluator.h"
#include "AnimGraphNode_BlendListByBool.h"
#include "AnimGraphNode_RotateRootBone.h"
#include "AnimGraphNode_LayeredBoneBlend.h"
#include "AnimGraphNode_Slot.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphSchema.h"
#include "K2Node_VariableGet.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "KismetCompiler.h"
#include "ScopedTransaction.h"
#include "UObject/UnrealType.h"

namespace
{
template<typename T, typename ConfigureType>
T* AddTurningNode(UEdGraph* Graph, TArray<UEdGraphNode*>& Added, ConfigureType Configure)
{
	T* Node = NewObject<T>(Graph, NAME_None, RF_Transactional);
	Node->CreateNewGuid();
	Graph->AddNode(Node, false, false);
	Node->PostPlacedNewNode();
	Configure(Node);
	Node->AllocateDefaultPins();
	Node->NodeComment = TEXT("B01 Turning");
	Added.Add(Node);
	return Node;
}
}

FString UBlasterAnimationEditorLibrary::InstallFireSlot()
{
	UAnimBlueprint* Blueprint = LoadObject<UAnimBlueprint>(nullptr,
		TEXT("/Game/Blueprints/Character/Animation/ABP_Blaster.ABP_Blaster"));
	if (!Blueprint) return TEXT("ERROR: Missing ABP.");
	TArray<UEdGraph*> Graphs;
	Blueprint->GetAllGraphs(Graphs);
	UEdGraph* Graph = nullptr;
	UEdGraphPin* Input = nullptr;
	int32 Matches = 0;
	for (UEdGraph* Candidate : Graphs)
	{
		if (Candidate->GetFName() != TEXT("AnimGraph")) continue;
		Graph = Candidate;
		for (UEdGraphNode* Node : Graph->Nodes)
		{
			if (Node->NodeComment == TEXT("B02 Fire")) return TEXT("ERROR: Fire slot already installed.");
			if (Node->IsA<UAnimGraphNode_LayeredBoneBlend>() && Node->NodeComment == TEXT("B01 Turning"))
			{
				Input = Node->FindPin(TEXT("BlendPoses_0"));
				++Matches;
			}
		}
	}
	if (Matches != 1 || !Input || Input->LinkedTo.Num() != 1) return TEXT("ERROR: Upper body contract changed.");
	const FScopedTransaction Transaction(FText::FromString(TEXT("Blaster Fire Slot")));
	Blueprint->Modify();
	Graph->Modify();
	UEdGraphPin* Output = Input->LinkedTo[0];
	Output->GetOwningNode()->Modify();
	Input->GetOwningNode()->Modify();
	TArray<UEdGraphNode*> Added;
	auto* Slot = AddTurningNode<UAnimGraphNode_Slot>(Graph, Added, [](auto* Node)
	{
		Node->Node.SlotName = TEXT("DefaultSlot");
	});
	Slot->NodeComment = TEXT("B02 Fire");
	Slot->NodePosX = Input->GetOwningNode()->NodePosX - 300;
	Slot->NodePosY = Input->GetOwningNode()->NodePosY + 300;
	Input->BreakLinkTo(Output);
	bool bOK = Graph->GetSchema()->TryCreateConnection(Output, Slot->FindPin(TEXT("Source")))
		&& Graph->GetSchema()->TryCreateConnection(Slot->FindPin(TEXT("Pose")), Input);
	FCompilerResultsLog Results;
	if (bOK)
	{
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
		FKismetEditorUtilities::CompileBlueprint(Blueprint, EBlueprintCompileOptions::None, &Results);
		bOK = Results.NumErrors == 0;
	}
	if (!bOK)
	{
		Slot->DestroyNode();
		Graph->GetSchema()->TryCreateConnection(Output, Input);
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
		FKismetEditorUtilities::CompileBlueprint(Blueprint);
		return TEXT("ERROR: Fire slot rolled back.");
	}
	Blueprint->MarkPackageDirty();
	return TEXT("OK: Upper body fire slot compiled; not saved.");
}

FString UBlasterAnimationEditorLibrary::InstallTurningGraph()
{
	UAnimBlueprint* Blueprint = LoadObject<UAnimBlueprint>(nullptr,
		TEXT("/Game/Blueprints/Character/Animation/ABP_Blaster.ABP_Blaster"));
	UAnimSequence* Left = LoadObject<UAnimSequence>(nullptr,
		TEXT("/Game/Blueprints/Character/Animation/B01Turn/Turn_Left_B01.Turn_Left_B01"));
	UAnimSequence* Right = LoadObject<UAnimSequence>(nullptr,
		TEXT("/Game/Blueprints/Character/Animation/B01Turn/Turn_Right_B01.Turn_Right_B01"));
	if (!Blueprint || !Left || !Right || Left->GetSkeleton() != Blueprint->TargetSkeleton
		|| Right->GetSkeleton() != Blueprint->TargetSkeleton)
	{
		return TEXT("ERROR: Missing assets or incompatible skeletons; no changes.");
	}
	for (const FName Property : {FName(TEXT("TurnRootYaw")), FName(TEXT("TurnAnimationTime")),
		FName(TEXT("bTurningLeft")), FName(TEXT("bTurningRight"))})
	{
		if (!FindFProperty<FProperty>(Blueprint->ParentClass, Property))
		{
			return TEXT("ERROR: Build and restart to load the turning properties first.");
		}
	}
	TArray<UEdGraph*> Graphs;
	Blueprint->GetAllGraphs(Graphs);
	UEdGraph* Graph = nullptr;
	for (UEdGraph* Candidate : Graphs)
	{
		if (Candidate->GetFName() == TEXT("AnimGraph")) Graph = Candidate;
	}
	if (!Graph) return TEXT("ERROR: AnimGraph not found.");
	UAnimGraphNode_StateMachine* Equipped = nullptr;
	int32 Matches = 0;
	for (UEdGraphNode* Node : Graph->Nodes)
	{
		if (Node->NodeComment == TEXT("B01 Turning"))
			return TEXT("ERROR: Turning nodes already exist; inspect instead of installing twice.");
		if (UAnimGraphNode_StateMachine* Machine = Cast<UAnimGraphNode_StateMachine>(Node))
		{
			if (Machine->GetStateMachineName() == TEXT("Equipped"))
			{
				Equipped = Machine;
				++Matches;
			}
		}
	}
	UEdGraphPin* OriginalOutput = Equipped ? Equipped->FindPin(TEXT("Pose")) : nullptr;
	if (Matches != 1 || !OriginalOutput || OriginalOutput->LinkedTo.Num() != 1)
		return TEXT("ERROR: Equipped must have one outgoing pose link.");
	UEdGraphPin* OriginalInput = OriginalOutput->LinkedTo[0];
	if (OriginalInput->PinName != TEXT("LocalPose")
		|| OriginalInput->GetOwningNode()->GetClass()->GetFName() != TEXT("AnimGraphNode_LocalToComponentSpace"))
		return TEXT("ERROR: Expected existing FABRIK local-to-component boundary.");

	const FScopedTransaction Transaction(NSLOCTEXT("Blaster", "InstallTurning", "Install B01 turning branch"));
	Blueprint->Modify();
	Graph->Modify();
	OriginalOutput->GetOwningNode()->Modify();
	OriginalInput->GetOwningNode()->Modify();
	TArray<UEdGraphNode*> Added;
	auto* Cache = AddTurningNode<UAnimGraphNode_SaveCachedPose>(Graph, Added, [](auto* Node)
	{
		Node->CacheName = TEXT("B01 Equipped Base");
	});
	auto UseCache = [&]()
	{
		return AddTurningNode<UAnimGraphNode_UseCachedPose>(Graph, Added, [Cache](auto* Node)
		{
			Node->SaveCachedPoseNode = Cache;
		});
	};
	auto* LowerBase = UseCache();
	auto* UpperBase = UseCache();
	auto Evaluator = [&](UAnimSequence* Animation)
	{
		return AddTurningNode<UAnimGraphNode_SequenceEvaluator>(Graph, Added, [Animation](auto* Node)
		{
			Node->Node.SetSequence(Animation);
			Node->Node.SetShouldLoop(false);
		});
	};
	auto* LeftPose = Evaluator(Left);
	auto* RightPose = Evaluator(Right);
	auto Selector = [&]()
	{
		auto* Node = AddTurningNode<UAnimGraphNode_BlendListByBool>(Graph, Added, [](auto*) {});
		for (const FName PinName : {FName(TEXT("BlendTime_0")), FName(TEXT("BlendTime_1"))})
		{
			if (UEdGraphPin* Pin = Node->FindPin(PinName)) Pin->DefaultValue = TEXT("0.0");
		}
		return Node;
	};
	auto* LeftSelect = Selector();
	auto* RightSelect = Selector();
	auto* Rotate = AddTurningNode<UAnimGraphNode_RotateRootBone>(Graph, Added, [](auto* Node)
	{
		Node->Node.MeshToComponent = FRotator(0.f, -90.f, 0.f);
	});
	auto* Layer = AddTurningNode<UAnimGraphNode_LayeredBoneBlend>(Graph, Added, [](auto* Node)
	{
		Node->Node.BlendPoses.SetNum(1);
		Node->Node.BlendWeights.Init(1.f, 1);
		Node->Node.LayerSetup.SetNum(1);
		FBranchFilter Filter;
		Filter.BoneName = TEXT("spine_01");
		Filter.BlendDepth = 1;
		Node->Node.LayerSetup[0].BranchFilters.Add(Filter);
		Node->Node.bMeshSpaceRotationBlend = true;
	});
	auto Variable = [&](FName Name)
	{
		return AddTurningNode<UK2Node_VariableGet>(Graph, Added, [Name](auto* Node)
		{
			Node->VariableReference.SetSelfMember(Name);
		});
	};
	auto* LeftFlag = Variable(TEXT("bTurningLeft"));
	auto* RightFlag = Variable(TEXT("bTurningRight"));
	auto* Time = Variable(TEXT("TurnAnimationTime"));
	auto* Yaw = Variable(TEXT("TurnRootYaw"));
	bool bOK = true;
	FString FailedLink;
	auto Wire = [&](UEdGraphNode* From, const TCHAR* Output, UEdGraphNode* To, const TCHAR* Input)
	{
		UEdGraphPin* A = From->FindPin(Output);
		UEdGraphPin* B = To->FindPin(Input);
		if (!A || !B || !Graph->GetSchema()->TryCreateConnection(A, B))
		{
			bOK = false;
			FailedLink += FString::Printf(TEXT(" %s -> %s;"), Output, Input);
		}
	};
	Wire(Equipped, TEXT("Pose"), Cache, TEXT("Pose"));
	Wire(LeftPose, TEXT("Pose"), LeftSelect, TEXT("BlendPose_0"));
	Wire(LowerBase, TEXT("Pose"), LeftSelect, TEXT("BlendPose_1"));
	Wire(LeftFlag, TEXT("bTurningLeft"), LeftSelect, TEXT("bActiveValue"));
	Wire(RightPose, TEXT("Pose"), RightSelect, TEXT("BlendPose_0"));
	Wire(LeftSelect, TEXT("Pose"), RightSelect, TEXT("BlendPose_1"));
	Wire(RightFlag, TEXT("bTurningRight"), RightSelect, TEXT("bActiveValue"));
	Wire(Time, TEXT("TurnAnimationTime"), LeftPose, TEXT("ExplicitTime"));
	Wire(Time, TEXT("TurnAnimationTime"), RightPose, TEXT("ExplicitTime"));
	Wire(RightSelect, TEXT("Pose"), Rotate, TEXT("BasePose"));
	Wire(Yaw, TEXT("TurnRootYaw"), Rotate, TEXT("Yaw"));
	Wire(Rotate, TEXT("Pose"), Layer, TEXT("BasePose"));
	Wire(UpperBase, TEXT("Pose"), Layer, TEXT("BlendPoses_0"));
	if (bOK)
	{
		bOK = Graph->GetSchema()->TryCreateConnection(Layer->FindPin(TEXT("Pose")), OriginalInput);
	}
	for (int32 Index = 0; Index < Added.Num(); ++Index)
	{
		Added[Index]->NodePosX = -2400 + (Index % 4) * 360;
		Added[Index]->NodePosY = -1600 + (Index / 4) * 240;
	}
	FCompilerResultsLog Results;
	if (bOK)
	{
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
		FKismetEditorUtilities::CompileBlueprint(Blueprint, EBlueprintCompileOptions::None, &Results);
		bOK = Results.NumErrors == 0;
	}
	if (!bOK)
	{
		for (UEdGraphNode* Node : Added) Node->DestroyNode();
		Graph->GetSchema()->TryCreateConnection(OriginalOutput, OriginalInput);
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
		FKismetEditorUtilities::CompileBlueprint(Blueprint);
		return FString::Printf(TEXT("ERROR: New nodes rolled back. Links:%s Compile errors:%d"), *FailedLink, Results.NumErrors);
	}
	Blueprint->MarkPackageDirty();
	return TEXT("OK: Turning graph installed and compiled. Inspect and save the asset.");
}
