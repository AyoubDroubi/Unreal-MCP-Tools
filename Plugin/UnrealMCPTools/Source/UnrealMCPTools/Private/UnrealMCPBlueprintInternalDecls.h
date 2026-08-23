#pragma once

#include "UnrealMCPBlueprintToolset.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/ActorComponent.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"
#include "Editor.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_DynamicCast.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/CompilerResultsLog.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Logging/TokenizedMessage.h"
#include "Misc/PackageName.h"
#include "ScopedTransaction.h"
#include "Subsystems/EditorAssetSubsystem.h"
#include "UnrealMCPToolsSettings.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"

namespace UnrealMCPBlueprintTools
{
    FString NormalizePackagePath(const FString& Path);
    FString NormalizeObjectPath(const FString& Path);
    bool IsWriteAllowed(const FString& Path, FString& OutError);
    UBlueprint* LoadBlueprint(const FString& AssetPath, FString& OutError, bool bForWrite);
    UEdGraph* FindGraph(UBlueprint* Blueprint, const FString& GraphName);
    FString GetGraphType(UBlueprint* Blueprint, UEdGraph* Graph);
    FString BlueprintStatusToString(EBlueprintStatus Status);
    FString PinTypeToString(const FEdGraphPinType& PinType);
    FMCPPinInfo MakePinInfo(const UEdGraphPin* Pin);
    FMCPNodeInfo MakeNodeInfo(const UEdGraphNode* Node);
    UEdGraphNode* FindNode(UBlueprint* Blueprint, UEdGraph* Graph, const FString& GuidText);
    UEdGraphPin* FindPin(UEdGraphNode* Node, const FString& PinName);
    bool ParseVariableType(
        const FString& TypeName,
        const FString& ObjectTypePath,
        const FString& ContainerType,
        FEdGraphPinType& OutType,
        FString& OutError);
    FMCPCompileResult CompileInternal(UBlueprint* Blueprint, bool bSave);
    FMCPToolResult Success(UBlueprint* Blueprint, const FString& Message, const FString& ResultId = TEXT(""));
    void ApplyCompilePolicy(UBlueprint* Blueprint, FMCPToolResult& MutationResult);

    template <typename TNode>
    TNode* AddSimpleNode(UEdGraph* Graph, const int32 X, const int32 Y)
    {
        TNode* Node = NewObject<TNode>(Graph);
        Graph->AddNode(Node, true, false);
        Node->CreateNewGuid();
        Node->PostPlacedNewNode();
        Node->AllocateDefaultPins();
        Node->NodePosX = X;
        Node->NodePosY = Y;
        return Node;
    }
}

using namespace UnrealMCPBlueprintTools;
