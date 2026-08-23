#include "UnrealMCPBlueprintInternal.h"

FMCPToolResult UUnrealMCPBlueprintToolset::CreateBlueprint(
    const FString& package_path,
    const FString& parent_class_path)
{
    FMCPToolResult Result;
    FString Error;

    if (!IsWriteAllowed(package_path, Error))
    {
        Result.message = Error;
        return Result;
    }

    const FString PackagePath = NormalizePackagePath(package_path);
    if (!FPackageName::IsValidLongPackageName(PackagePath))
    {
        Result.message = TEXT("package_path must be a valid long package name such as /Game/Blueprints/BP_Player.");
        return Result;
    }

    if (LoadObject<UObject>(nullptr, *NormalizeObjectPath(PackagePath)))
    {
        Result.message = TEXT("An asset already exists at that package path.");
        return Result;
    }

    UClass* ParentClass = LoadObject<UClass>(nullptr, *parent_class_path);
    if (!ParentClass)
    {
        Result.message = FString::Printf(TEXT("Parent class not found: %s"), *parent_class_path);
        return Result;
    }

    if (!FKismetEditorUtilities::CanCreateBlueprintOfClass(ParentClass))
    {
        Result.message = TEXT("Unreal reports that the requested parent class cannot be used to create a Blueprint.");
        return Result;
    }

    FString AssetName;
    PackagePath.Split(TEXT("/"), nullptr, &AssetName, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
    UPackage* Package = CreatePackage(*PackagePath);
    if (!Package)
    {
        Result.message = TEXT("Failed to create package.");
        return Result;
    }

    const FScopedTransaction Transaction(NSLOCTEXT("UnrealMCPTools", "CreateBlueprint", "MCP: Create Blueprint"));
    Package->Modify();

    UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
        ParentClass,
        Package,
        FName(*AssetName),
        BPTYPE_Normal,
        FName(TEXT("UnrealMCPTools")));

    if (!Blueprint)
    {
        Result.message = TEXT("FKismetEditorUtilities::CreateBlueprint failed.");
        return Result;
    }

    FAssetRegistryModule::AssetCreated(Blueprint);
    Package->MarkPackageDirty();

    const FMCPCompileResult Compile = CompileInternal(Blueprint, false);
    Result = Success(Blueprint, Compile.success ? TEXT("Blueprint created and compiled.") : TEXT("Blueprint created but compile failed."));
    Result.success = Compile.success;
    Result.warnings = Compile.compiler_messages;
    return Result;
}

FMCPToolResult UUnrealMCPBlueprintToolset::CreateFunctionGraph(
    const FString& asset_path,
    const FString& function_name)
{
    FMCPToolResult Result;
    FString Error;
    UBlueprint* Blueprint = LoadBlueprint(asset_path, Error, true);
    if (!Blueprint)
    {
        Result.message = Error;
        return Result;
    }

    if (function_name.IsEmpty() || FindGraph(Blueprint, function_name))
    {
        Result.message = TEXT("function_name is empty or a graph with that exact name already exists.");
        return Result;
    }

    {
        const FScopedTransaction Transaction(NSLOCTEXT("UnrealMCPTools", "CreateFunctionGraph", "MCP: Create Function Graph"));
        Blueprint->Modify();

        UEdGraph* Graph = FBlueprintEditorUtils::CreateNewGraph(
            Blueprint,
            FName(*function_name),
            UEdGraph::StaticClass(),
            UEdGraphSchema_K2::StaticClass());

        if (!Graph)
        {
            Result.message = TEXT("Unable to allocate function graph.");
            return Result;
        }

        FBlueprintEditorUtils::AddFunctionGraph<UFunction>(Blueprint, Graph, true, nullptr);
        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
        Result = Success(Blueprint, TEXT("Function graph created."), Graph->GetName());
    }

    ApplyCompilePolicy(Blueprint, Result);
    return Result;
}

FMCPToolResult UUnrealMCPBlueprintToolset::RenameGraph(
    const FString& asset_path,
    const FString& graph_name,
    const FString& new_name)
{
    FMCPToolResult Result;
    FString Error;
    UBlueprint* Blueprint = LoadBlueprint(asset_path, Error, true);
    if (!Blueprint)
    {
        Result.message = Error;
        return Result;
    }

    UEdGraph* Graph = FindGraph(Blueprint, graph_name);
    if (!Graph || new_name.IsEmpty())
    {
        Result.message = TEXT("Graph not found or new_name is empty.");
        return Result;
    }

    if (FindGraph(Blueprint, new_name))
    {
        Result.message = TEXT("A graph with new_name already exists.");
        return Result;
    }

    {
        const FScopedTransaction Transaction(NSLOCTEXT("UnrealMCPTools", "RenameGraph", "MCP: Rename Graph"));
        Blueprint->Modify();
        Graph->Modify();
        FBlueprintEditorUtils::RenameGraph(Graph, new_name);
        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
        Result = Success(Blueprint, TEXT("Graph renamed."), Graph->GetName());
    }

    ApplyCompilePolicy(Blueprint, Result);
    return Result;
}

FMCPToolResult UUnrealMCPBlueprintToolset::AddMemberVariable(
    const FString& asset_path,
    const FString& variable_name,
    const FString& variable_type,
    const FString& object_type_path,
    const FString& container_type,
    const FString& default_value)
{
    FMCPToolResult Result;
    FString Error;
    UBlueprint* Blueprint = LoadBlueprint(asset_path, Error, true);
    if (!Blueprint)
    {
        Result.message = Error;
        return Result;
    }

    if (variable_name.IsEmpty())
    {
        Result.message = TEXT("variable_name cannot be empty.");
        return Result;
    }

    FEdGraphPinType PinType;
    if (!ParseVariableType(variable_type, object_type_path, container_type, PinType, Error))
    {
        Result.message = Error;
        return Result;
    }

    {
        const FScopedTransaction Transaction(NSLOCTEXT("UnrealMCPTools", "AddMemberVariable", "MCP: Add Member Variable"));
        Blueprint->Modify();

        if (!FBlueprintEditorUtils::AddMemberVariable(Blueprint, FName(*variable_name), PinType, default_value))
        {
            Result.message = TEXT("Unreal rejected the new member variable. Check for name conflicts or an incompatible type.");
            return Result;
        }

        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
        Result = Success(Blueprint, TEXT("Member variable added."), variable_name);
    }

    ApplyCompilePolicy(Blueprint, Result);
    return Result;
}

FMCPToolResult UUnrealMCPBlueprintToolset::RenameMemberVariable(
    const FString& asset_path,
    const FString& variable_name,
    const FString& new_name)
{
    FMCPToolResult Result;
    FString Error;
    UBlueprint* Blueprint = LoadBlueprint(asset_path, Error, true);
    if (!Blueprint)
    {
        Result.message = Error;
        return Result;
    }

    if (FBlueprintEditorUtils::FindNewVariableIndex(Blueprint, FName(*variable_name)) == INDEX_NONE)
    {
        Result.message = TEXT("The member variable is not declared by this Blueprint.");
        return Result;
    }

    if (new_name.IsEmpty() || FBlueprintEditorUtils::FindNewVariableIndex(Blueprint, FName(*new_name)) != INDEX_NONE)
    {
        Result.message = TEXT("new_name is empty or already exists.");
        return Result;
    }

    {
        const FScopedTransaction Transaction(NSLOCTEXT("UnrealMCPTools", "RenameMemberVariable", "MCP: Rename Member Variable"));
        Blueprint->Modify();
        FBlueprintEditorUtils::RenameMemberVariable(Blueprint, FName(*variable_name), FName(*new_name));
        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
        Result = Success(Blueprint, TEXT("Member variable renamed."), new_name);
    }

    ApplyCompilePolicy(Blueprint, Result);
    return Result;
}

FMCPToolResult UUnrealMCPBlueprintToolset::RemoveMemberVariable(
    const FString& asset_path,
    const FString& variable_name)
{
    FMCPToolResult Result;
    FString Error;
    UBlueprint* Blueprint = LoadBlueprint(asset_path, Error, true);
    if (!Blueprint)
    {
        Result.message = Error;
        return Result;
    }

    if (FBlueprintEditorUtils::FindNewVariableIndex(Blueprint, FName(*variable_name)) == INDEX_NONE)
    {
        Result.message = TEXT("The member variable is not declared by this Blueprint.");
        return Result;
    }

    {
        const FScopedTransaction Transaction(NSLOCTEXT("UnrealMCPTools", "RemoveMemberVariable", "MCP: Remove Member Variable"));
        Blueprint->Modify();
        FBlueprintEditorUtils::RemoveMemberVariable(Blueprint, FName(*variable_name));
        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
        Result = Success(Blueprint, TEXT("Member variable removed."));
    }

    ApplyCompilePolicy(Blueprint, Result);
    return Result;
}

FMCPToolResult UUnrealMCPBlueprintToolset::AddComponent(
    const FString& asset_path,
    const FString& component_class_path,
    const FString& component_name,
    const FString& parent_component)
{
    FMCPToolResult Result;
    FString Error;
    UBlueprint* Blueprint = LoadBlueprint(asset_path, Error, true);
    if (!Blueprint)
    {
        Result.message = Error;
        return Result;
    }

    if (!Blueprint->SimpleConstructionScript || !FBlueprintEditorUtils::DoesSupportComponents(Blueprint))
    {
        Result.message = TEXT("This Blueprint does not support SimpleConstructionScript components.");
        return Result;
    }

    UClass* ComponentClass = LoadObject<UClass>(nullptr, *component_class_path);
    if (!ComponentClass || !ComponentClass->IsChildOf(UActorComponent::StaticClass()))
    {
        Result.message = TEXT("component_class_path must resolve to an UActorComponent subclass.");
        return Result;
    }

    if (component_name.IsEmpty())
    {
        Result.message = TEXT("component_name cannot be empty.");
        return Result;
    }

    USimpleConstructionScript* SCS = Blueprint->SimpleConstructionScript;
    if (SCS->FindSCSNode(FName(*component_name)))
    {
        Result.message = TEXT("A component with that SCS variable name already exists.");
        return Result;
    }

    USCS_Node* ParentNode = nullptr;
    if (!parent_component.IsEmpty())
    {
        ParentNode = SCS->FindSCSNode(FName(*parent_component));
        if (!ParentNode)
        {
            Result.message = TEXT("parent_component was supplied but no matching SCS node exists.");
            return Result;
        }
    }

    {
        const FScopedTransaction Transaction(NSLOCTEXT("UnrealMCPTools", "AddComponent", "MCP: Add Blueprint Component"));
        Blueprint->Modify();
        SCS->Modify();

        const FName SafeName = SCS->GenerateNewComponentName(ComponentClass, FName(*component_name));
        USCS_Node* NewNode = SCS->CreateNode(ComponentClass, SafeName);
        if (!NewNode)
        {
            Result.message = TEXT("SimpleConstructionScript failed to create the component node.");
            return Result;
        }

        NewNode->Modify();
        if (ParentNode)
        {
            ParentNode->Modify();
            ParentNode->AddChildNode(NewNode, true);
        }
        else
        {
            SCS->AddNode(NewNode);
        }

        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
        Result = Success(Blueprint, TEXT("Blueprint component added."), NewNode->GetVariableName().ToString());
        if (!NewNode->GetVariableName().ToString().Equals(component_name, ESearchCase::CaseSensitive))
        {
            Result.warnings.Add(FString::Printf(
                TEXT("Unreal adjusted the component name to '%s' to keep it valid/unique."),
                *NewNode->GetVariableName().ToString()));
        }
    }

    ApplyCompilePolicy(Blueprint, Result);
    return Result;
}

FMCPToolResult UUnrealMCPBlueprintToolset::RemoveComponent(
    const FString& asset_path,
    const FString& component_name)
{
    FMCPToolResult Result;
    FString Error;
    UBlueprint* Blueprint = LoadBlueprint(asset_path, Error, true);
    if (!Blueprint)
    {
        Result.message = Error;
        return Result;
    }

    USimpleConstructionScript* SCS = Blueprint->SimpleConstructionScript;
    USCS_Node* Node = SCS ? SCS->FindSCSNode(FName(*component_name)) : nullptr;
    if (!SCS || !Node)
    {
        Result.message = TEXT("Component was not found in this Blueprint's SimpleConstructionScript.");
        return Result;
    }

    {
        const FScopedTransaction Transaction(NSLOCTEXT("UnrealMCPTools", "RemoveComponent", "MCP: Remove Blueprint Component"));
        Blueprint->Modify();
        SCS->Modify();
        Node->Modify();

        SCS->RemoveNodeAndPromoteChildren(Node);
        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
        Result = Success(Blueprint, TEXT("Blueprint component removed."));
    }

    ApplyCompilePolicy(Blueprint, Result);
    return Result;
}
