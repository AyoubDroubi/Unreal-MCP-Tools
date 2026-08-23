#pragma once

#include "UnrealMCPBlueprintToolset.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"
#include "Editor.h"
#include "Subsystems/EditorAssetSubsystem.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Components/ActorComponent.h"
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
#include "UnrealMCPToolsSettings.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"

namespace UnrealMCPBlueprintTools
{
    FString NormalizePackagePath(const FString& Path)
    {
        FString Result = Path;
        Result.TrimStartAndEndInline();

        const int32 DotIndex = Result.Find(TEXT("."), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
        if (DotIndex != INDEX_NONE)
        {
            Result = Result.Left(DotIndex);
        }
        return Result;
    }

    FString NormalizeObjectPath(const FString& Path)
    {
        const FString PackagePath = NormalizePackagePath(Path);
        FString AssetName;
        if (!PackagePath.Split(TEXT("/"), nullptr, &AssetName, ESearchCase::CaseSensitive, ESearchDir::FromEnd))
        {
            return Path;
        }
        return FString::Printf(TEXT("%s.%s"), *PackagePath, *AssetName);
    }

    bool IsWriteAllowed(const FString& Path, FString& OutError)
    {
        const FString PackagePath = NormalizePackagePath(Path);
        const UUnrealMCPToolsSettings* Settings = GetDefault<UUnrealMCPToolsSettings>();

        if (PackagePath.StartsWith(TEXT("/Engine")) && !Settings->bAllowEngineContent)
        {
            OutError = TEXT("Engine content is read-only. Enable bAllowEngineContent explicitly if this is intentional.");
            return false;
        }

        for (const FString& Root : Settings->AllowedAssetRoots)
        {
            if (!Root.IsEmpty() && PackagePath.StartsWith(Root))
            {
                return true;
            }
        }

        OutError = FString::Printf(
            TEXT("Write rejected: '%s' is outside AllowedAssetRoots."),
            *PackagePath);
        return false;
    }

    UBlueprint* LoadBlueprint(const FString& AssetPath, FString& OutError, const bool bForWrite)
    {
        if (bForWrite && !IsWriteAllowed(AssetPath, OutError))
        {
            return nullptr;
        }

        const FString ObjectPath = NormalizeObjectPath(AssetPath);
        UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *ObjectPath);
        if (!Blueprint)
        {
            OutError = FString::Printf(TEXT("Blueprint not found: %s"), *ObjectPath);
        }
        return Blueprint;
    }

    UEdGraph* FindGraph(UBlueprint* Blueprint, const FString& GraphName)
    {
        if (!Blueprint)
        {
            return nullptr;
        }

        auto Match = [&GraphName](const TArray<TObjectPtr<UEdGraph>>& Graphs) -> UEdGraph*
        {
            for (UEdGraph* Graph : Graphs)
            {
                if (Graph && Graph->GetName().Equals(GraphName, ESearchCase::CaseSensitive))
                {
                    return Graph;
                }
            }
            return nullptr;
        };

        if (UEdGraph* Graph = Match(Blueprint->UbergraphPages)) return Graph;
        if (UEdGraph* Graph = Match(Blueprint->FunctionGraphs)) return Graph;
        if (UEdGraph* Graph = Match(Blueprint->MacroGraphs)) return Graph;
        if (UEdGraph* Graph = Match(Blueprint->DelegateSignatureGraphs)) return Graph;

        return nullptr;
    }

    FString GetGraphType(UBlueprint* Blueprint, UEdGraph* Graph)
    {
        if (Blueprint->UbergraphPages.Contains(Graph)) return TEXT("event");
        if (Blueprint->FunctionGraphs.Contains(Graph)) return TEXT("function");
        if (Blueprint->MacroGraphs.Contains(Graph)) return TEXT("macro");
        if (Blueprint->DelegateSignatureGraphs.Contains(Graph)) return TEXT("delegate");
        return TEXT("other");
    }

    FString BlueprintStatusToString(const EBlueprintStatus Status)
    {
        switch (Status)
        {
        case BS_Unknown: return TEXT("Unknown");
        case BS_Dirty: return TEXT("Dirty");
        case BS_Error: return TEXT("Error");
        case BS_UpToDate: return TEXT("UpToDate");
        case BS_BeingCreated: return TEXT("BeingCreated");
        case BS_UpToDateWithWarnings: return TEXT("UpToDateWithWarnings");
        default: return TEXT("Other");
        }
    }

    FString PinTypeToString(const FEdGraphPinType& PinType)
    {
        FString Result = PinType.PinCategory.ToString();

        if (PinType.PinSubCategoryObject != nullptr)
        {
            Result += FString::Printf(TEXT("<%s>"), *PinType.PinSubCategoryObject->GetPathName());
        }

        if (PinType.IsArray())
        {
            Result = FString::Printf(TEXT("array<%s>"), *Result);
        }
        else if (PinType.IsSet())
        {
            Result = FString::Printf(TEXT("set<%s>"), *Result);
        }
        else if (PinType.IsMap())
        {
            Result = FString::Printf(TEXT("map<%s>"), *Result);
        }

        return Result;
    }

    FMCPPinInfo MakePinInfo(const UEdGraphPin* Pin)
    {
        FMCPPinInfo Info;
        if (!Pin)
        {
            return Info;
        }

        Info.name = Pin->PinName.ToString();
        Info.direction = Pin->Direction == EGPD_Input ? TEXT("input") : TEXT("output");
        Info.type = PinTypeToString(Pin->PinType);
        Info.default_value = Pin->DefaultValue;

        for (const UEdGraphPin* Linked : Pin->LinkedTo)
        {
            if (Linked && Linked->GetOwningNode())
            {
                Info.links.Add(FString::Printf(
                    TEXT("%s:%s"),
                    *Linked->GetOwningNode()->NodeGuid.ToString(EGuidFormats::DigitsWithHyphens),
                    *Linked->PinName.ToString()));
            }
        }
        return Info;
    }

    FMCPNodeInfo MakeNodeInfo(const UEdGraphNode* Node)
    {
        FMCPNodeInfo Info;
        if (!Node)
        {
            return Info;
        }

        Info.guid = Node->NodeGuid.ToString(EGuidFormats::DigitsWithHyphens);
        Info.class_path = Node->GetClass()->GetPathName();
        Info.title = Node->GetNodeTitle(ENodeTitleType::ListView).ToString();
        Info.x = Node->NodePosX;
        Info.y = Node->NodePosY;

        for (const UEdGraphPin* Pin : Node->Pins)
        {
            Info.pins.Add(MakePinInfo(Pin));
        }
        return Info;
    }

    UEdGraphNode* FindNode(UBlueprint* Blueprint, UEdGraph* Graph, const FString& GuidText)
    {
        FGuid Guid;
        if (!FGuid::Parse(GuidText, Guid))
        {
            return nullptr;
        }

        UEdGraphNode* Node = FBlueprintEditorUtils::GetNodeByGUID(Blueprint, Guid);
        return Node && Node->GetGraph() == Graph ? Node : nullptr;
    }

    UEdGraphPin* FindPin(UEdGraphNode* Node, const FString& PinName)
    {
        if (!Node)
        {
            return nullptr;
        }

        for (UEdGraphPin* Pin : Node->Pins)
        {
            if (Pin && Pin->PinName.ToString().Equals(PinName, ESearchCase::CaseSensitive))
            {
                return Pin;
            }
        }
        return nullptr;
    }

    bool ParseVariableType(
        const FString& TypeName,
        const FString& ObjectTypePath,
        const FString& ContainerType,
        FEdGraphPinType& OutType,
        FString& OutError)
    {
        const FString Lower = TypeName.ToLower();
        OutType = FEdGraphPinType();

        if (Lower == TEXT("bool")) OutType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
        else if (Lower == TEXT("byte")) OutType.PinCategory = UEdGraphSchema_K2::PC_Byte;
        else if (Lower == TEXT("int")) OutType.PinCategory = UEdGraphSchema_K2::PC_Int;
        else if (Lower == TEXT("int64")) OutType.PinCategory = UEdGraphSchema_K2::PC_Int64;
        else if (Lower == TEXT("float"))
        {
            OutType.PinCategory = UEdGraphSchema_K2::PC_Real;
            OutType.PinSubCategory = UEdGraphSchema_K2::PC_Float;
        }
        else if (Lower == TEXT("double"))
        {
            OutType.PinCategory = UEdGraphSchema_K2::PC_Real;
            OutType.PinSubCategory = UEdGraphSchema_K2::PC_Double;
        }
        else if (Lower == TEXT("string")) OutType.PinCategory = UEdGraphSchema_K2::PC_String;
        else if (Lower == TEXT("name")) OutType.PinCategory = UEdGraphSchema_K2::PC_Name;
        else if (Lower == TEXT("text")) OutType.PinCategory = UEdGraphSchema_K2::PC_Text;
        else if (Lower == TEXT("object") || Lower == TEXT("class") || Lower == TEXT("struct"))
        {
            if (ObjectTypePath.IsEmpty())
            {
                OutError = TEXT("object_type_path is required for object, class, and struct variables.");
                return false;
            }

            UObject* SubType = StaticLoadObject(UObject::StaticClass(), nullptr, *ObjectTypePath);
            if (!SubType)
            {
                OutError = FString::Printf(TEXT("Unable to load variable subtype: %s"), *ObjectTypePath);
                return false;
            }

            if (Lower == TEXT("object"))
            {
                UClass* Class = Cast<UClass>(SubType);
                if (!Class)
                {
                    OutError = TEXT("object_type_path must resolve to a UClass for object variables.");
                    return false;
                }
                OutType.PinCategory = UEdGraphSchema_K2::PC_Object;
                OutType.PinSubCategoryObject = Class;
            }
            else if (Lower == TEXT("class"))
            {
                UClass* Class = Cast<UClass>(SubType);
                if (!Class)
                {
                    OutError = TEXT("object_type_path must resolve to a UClass for class variables.");
                    return false;
                }
                OutType.PinCategory = UEdGraphSchema_K2::PC_Class;
                OutType.PinSubCategoryObject = Class;
            }
            else
            {
                UScriptStruct* Struct = Cast<UScriptStruct>(SubType);
                if (!Struct)
                {
                    OutError = TEXT("object_type_path must resolve to a UScriptStruct for struct variables.");
                    return false;
                }
                OutType.PinCategory = UEdGraphSchema_K2::PC_Struct;
                OutType.PinSubCategoryObject = Struct;
            }
        }
        else
        {
            OutError = FString::Printf(TEXT("Unsupported variable_type: %s"), *TypeName);
            return false;
        }

        const FString Container = ContainerType.ToLower();
        if (Container.IsEmpty() || Container == TEXT("scalar"))
        {
            OutType.ContainerType = EPinContainerType::None;
        }
        else if (Container == TEXT("array"))
        {
            OutType.ContainerType = EPinContainerType::Array;
        }
        else if (Container == TEXT("set"))
        {
            OutType.ContainerType = EPinContainerType::Set;
        }
        else
        {
            OutError = TEXT("container_type must be scalar, array, or set. Map variables are intentionally not exposed yet.");
            return false;
        }

        if (!FBlueprintEditorUtils::IsPinTypeValid(OutType))
        {
            OutError = TEXT("The requested variable pin type is not valid for Blueprint.");
            return false;
        }

        return true;
    }

    FMCPCompileResult CompileInternal(UBlueprint* Blueprint, const bool bSave)
    {
        FMCPCompileResult Result;
        if (!Blueprint)
        {
            Result.message = TEXT("Blueprint is null.");
            return Result;
        }

        Result.asset_path = Blueprint->GetPathName();

        FCompilerResultsLog CompilerLog(false);
        CompilerLog.bAnnotateMentionedNodes = false;
        FKismetEditorUtilities::CompileBlueprint(Blueprint, EBlueprintCompileOptions::None, &CompilerLog);

        Result.error_count = CompilerLog.NumErrors;
        Result.warning_count = CompilerLog.NumWarnings;
        Result.success = CompilerLog.NumErrors == 0 && Blueprint->Status != BS_Error;

        for (const TSharedRef<FTokenizedMessage>& Message : CompilerLog.Messages)
        {
            Result.compiler_messages.Add(Message->ToText().ToString());
        }

        if (Result.success && bSave)
        {
            if (UEditorAssetSubsystem* AssetSubsystem = GEditor ? GEditor->GetEditorSubsystem<UEditorAssetSubsystem>() : nullptr)
            {
                Result.saved = AssetSubsystem->SaveLoadedAsset(Blueprint, false);
            }
        }

        Result.message = Result.success
            ? FString::Printf(TEXT("Compile succeeded with %d warning(s)."), Result.warning_count)
            : FString::Printf(TEXT("Compile failed with %d error(s) and %d warning(s)."), Result.error_count, Result.warning_count);

        return Result;
    }

    FMCPToolResult Success(UBlueprint* Blueprint, const FString& Message, const FString& ResultId = TEXT(""))
    {
        FMCPToolResult Result;
        Result.success = true;
        Result.message = Message;
        Result.asset_path = Blueprint ? Blueprint->GetPathName() : TEXT("");
        Result.result_id = ResultId;
        return Result;
    }

    void ApplyCompilePolicy(UBlueprint* Blueprint, FMCPToolResult& MutationResult)
    {
        if (!MutationResult.success || !Blueprint)
        {
            return;
        }

        const UUnrealMCPToolsSettings* Settings = GetDefault<UUnrealMCPToolsSettings>();
        if (!Settings->bAutoCompileAfterMutation)
        {
            MutationResult.warnings.Add(TEXT("Automatic Blueprint compile is disabled in Unreal MCP Tools settings."));
            return;
        }

        const FMCPCompileResult CompileResult = CompileInternal(Blueprint, Settings->bAutoSaveAfterSuccessfulCompile);
        if (CompileResult.warning_count > 0)
        {
            MutationResult.warnings.Add(FString::Printf(TEXT("Compile completed with %d warning(s)."), CompileResult.warning_count));
        }

        if (!CompileResult.success)
        {
            MutationResult.success = false;
            MutationResult.message += TEXT(" Automatic compile failed.");

            for (const FString& CompilerMessage : CompileResult.compiler_messages)
            {
                MutationResult.warnings.Add(CompilerMessage);
            }

            if (Settings->bRollbackOnCompileFailure && GEditor)
            {
                const bool bUndone = GEditor->UndoTransaction(true);
                MutationResult.message += bUndone
                    ? TEXT(" The latest MCP transaction was rolled back.")
                    : TEXT(" Automatic rollback was requested but Unreal could not undo the transaction.");
            }
        }
    }

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
