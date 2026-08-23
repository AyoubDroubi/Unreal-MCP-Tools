#include "UnrealMCPBlueprintInternalDecls.h"

FMCPToolResult UUnrealMCPBlueprintToolset::AddCallFunctionNode(
    const FString& asset_path,
    const FString& graph_name,
    const FString& owner_class_path,
    const FString& function_name,
    const int32 x,
    const int32 y)
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
    UClass* OwnerClass = LoadObject<UClass>(nullptr, *owner_class_path);
    UFunction* Function = OwnerClass ? OwnerClass->FindFunctionByName(FName(*function_name)) : nullptr;

    if (!Graph)
    {
        Result.message = TEXT("Graph not found.");
        return Result;
    }
    if (!Function)
    {
        Result.message = TEXT("Function not found on owner_class_path.");
        return Result;
    }
    if (!Function->HasAnyFunctionFlags(FUNC_BlueprintCallable | FUNC_BlueprintPure))
    {
        Result.message = TEXT("Function is not BlueprintCallable or BlueprintPure.");
        return Result;
    }

    {
        const FScopedTransaction Transaction(NSLOCTEXT("UnrealMCPTools", "AddCallFunctionNode", "MCP: Add Call Function Node"));
        Blueprint->Modify();
        Graph->Modify();

        UK2Node_CallFunction* Node = NewObject<UK2Node_CallFunction>(Graph);
        Node->SetFromFunction(Function);
        Graph->AddNode(Node, true, false);
        Node->CreateNewGuid();
        Node->PostPlacedNewNode();
        Node->AllocateDefaultPins();
        Node->NodePosX = x;
        Node->NodePosY = y;

        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
        Result = Success(Blueprint, TEXT("Call-function node added."), Node->NodeGuid.ToString(EGuidFormats::DigitsWithHyphens));
    }

    ApplyCompilePolicy(Blueprint, Result);
    return Result;
}

FMCPToolResult UUnrealMCPBlueprintToolset::AddBranchNode(
    const FString& asset_path,
    const FString& graph_name,
    const int32 x,
    const int32 y)
{
    FMCPToolResult Result;
    FString Error;
    UBlueprint* Blueprint = LoadBlueprint(asset_path, Error, true);
    UEdGraph* Graph = Blueprint ? FindGraph(Blueprint, graph_name) : nullptr;

    if (!Blueprint || !Graph)
    {
        Result.message = Blueprint ? TEXT("Graph not found.") : Error;
        return Result;
    }

    {
        const FScopedTransaction Transaction(NSLOCTEXT("UnrealMCPTools", "AddBranchNode", "MCP: Add Branch Node"));
        Blueprint->Modify();
        Graph->Modify();
        UK2Node_IfThenElse* Node = AddSimpleNode<UK2Node_IfThenElse>(Graph, x, y);
        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
        Result = Success(Blueprint, TEXT("Branch node added."), Node->NodeGuid.ToString(EGuidFormats::DigitsWithHyphens));
    }

    ApplyCompilePolicy(Blueprint, Result);
    return Result;
}

FMCPToolResult UUnrealMCPBlueprintToolset::AddVariableGetNode(
    const FString& asset_path,
    const FString& graph_name,
    const FString& variable_name,
    const int32 x,
    const int32 y)
{
    FMCPToolResult Result;
    FString Error;
    UBlueprint* Blueprint = LoadBlueprint(asset_path, Error, true);
    UEdGraph* Graph = Blueprint ? FindGraph(Blueprint, graph_name) : nullptr;

    if (!Blueprint || !Graph)
    {
        Result.message = Blueprint ? TEXT("Graph not found.") : Error;
        return Result;
    }

    FProperty* Property = Blueprint->SkeletonGeneratedClass
        ? FindFProperty<FProperty>(Blueprint->SkeletonGeneratedClass, FName(*variable_name))
        : nullptr;
    if (!Property)
    {
        Result.message = TEXT("Variable was not found on the Blueprint skeleton class.");
        return Result;
    }

    {
        const FScopedTransaction Transaction(NSLOCTEXT("UnrealMCPTools", "AddVariableGetNode", "MCP: Add Variable Get Node"));
        Blueprint->Modify();
        Graph->Modify();

        const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();
        UK2Node_VariableGet* Node = Schema->SpawnVariableGetNode(
            FVector2D(static_cast<double>(x), static_cast<double>(y)),
            Graph,
            FName(*variable_name),
            Blueprint->SkeletonGeneratedClass);

        if (!Node)
        {
            Result.message = TEXT("Failed to spawn variable getter.");
            return Result;
        }

        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
        Result = Success(Blueprint, TEXT("Variable getter added."), Node->NodeGuid.ToString(EGuidFormats::DigitsWithHyphens));
    }

    ApplyCompilePolicy(Blueprint, Result);
    return Result;
}

FMCPToolResult UUnrealMCPBlueprintToolset::AddVariableSetNode(
    const FString& asset_path,
    const FString& graph_name,
    const FString& variable_name,
    const int32 x,
    const int32 y)
{
    FMCPToolResult Result;
    FString Error;
    UBlueprint* Blueprint = LoadBlueprint(asset_path, Error, true);
    UEdGraph* Graph = Blueprint ? FindGraph(Blueprint, graph_name) : nullptr;

    if (!Blueprint || !Graph)
    {
        Result.message = Blueprint ? TEXT("Graph not found.") : Error;
        return Result;
    }

    FProperty* Property = Blueprint->SkeletonGeneratedClass
        ? FindFProperty<FProperty>(Blueprint->SkeletonGeneratedClass, FName(*variable_name))
        : nullptr;
    if (!Property)
    {
        Result.message = TEXT("Variable was not found on the Blueprint skeleton class.");
        return Result;
    }

    {
        const FScopedTransaction Transaction(NSLOCTEXT("UnrealMCPTools", "AddVariableSetNode", "MCP: Add Variable Set Node"));
        Blueprint->Modify();
        Graph->Modify();

        const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();
        UK2Node_VariableSet* Node = Schema->SpawnVariableSetNode(
            FVector2D(static_cast<double>(x), static_cast<double>(y)),
            Graph,
            FName(*variable_name),
            Blueprint->SkeletonGeneratedClass);

        if (!Node)
        {
            Result.message = TEXT("Failed to spawn variable setter.");
            return Result;
        }

        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
        Result = Success(Blueprint, TEXT("Variable setter added."), Node->NodeGuid.ToString(EGuidFormats::DigitsWithHyphens));
    }

    ApplyCompilePolicy(Blueprint, Result);
    return Result;
}

FMCPToolResult UUnrealMCPBlueprintToolset::AddCustomEventNode(
    const FString& asset_path,
    const FString& graph_name,
    const FString& event_name,
    const int32 x,
    const int32 y)
{
    FMCPToolResult Result;
    FString Error;
    UBlueprint* Blueprint = LoadBlueprint(asset_path, Error, true);
    UEdGraph* Graph = Blueprint ? FindGraph(Blueprint, graph_name) : nullptr;

    if (!Blueprint || !Graph)
    {
        Result.message = Blueprint ? TEXT("Graph not found.") : Error;
        return Result;
    }

    if (event_name.IsEmpty())
    {
        Result.message = TEXT("event_name cannot be empty.");
        return Result;
    }

    {
        const FScopedTransaction Transaction(NSLOCTEXT("UnrealMCPTools", "AddCustomEventNode", "MCP: Add Custom Event Node"));
        Blueprint->Modify();
        Graph->Modify();

        UK2Node_CustomEvent* Node = NewObject<UK2Node_CustomEvent>(Graph);
        Node->CustomFunctionName = FName(*event_name);
        Graph->AddNode(Node, true, false);
        Node->CreateNewGuid();
        Node->PostPlacedNewNode();
        Node->AllocateDefaultPins();
        Node->NodePosX = x;
        Node->NodePosY = y;

        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
        Result = Success(Blueprint, TEXT("Custom Event node added."), Node->NodeGuid.ToString(EGuidFormats::DigitsWithHyphens));
    }

    ApplyCompilePolicy(Blueprint, Result);
    return Result;
}

FMCPToolResult UUnrealMCPBlueprintToolset::ConnectPins(
    const FString& asset_path,
    const FString& graph_name,
    const FString& from_node_guid,
    const FString& from_pin_name,
    const FString& to_node_guid,
    const FString& to_pin_name)
{
    FMCPToolResult Result;
    FString Error;
    UBlueprint* Blueprint = LoadBlueprint(asset_path, Error, true);
    UEdGraph* Graph = Blueprint ? FindGraph(Blueprint, graph_name) : nullptr;

    if (!Blueprint || !Graph)
    {
        Result.message = Blueprint ? TEXT("Graph not found.") : Error;
        return Result;
    }

    UEdGraphNode* FromNode = FindNode(Blueprint, Graph, from_node_guid);
    UEdGraphNode* ToNode = FindNode(Blueprint, Graph, to_node_guid);
    UEdGraphPin* FromPin = FindPin(FromNode, from_pin_name);
    UEdGraphPin* ToPin = FindPin(ToNode, to_pin_name);

    if (!FromPin || !ToPin)
    {
        Result.message = TEXT("One or both nodes/pins were not found. Read the graph first and use exact GUIDs and pin names.");
        return Result;
    }

    if (FromPin->Direction == ToPin->Direction)
    {
        Result.message = TEXT("Cannot connect two pins with the same direction.");
        return Result;
    }

    UEdGraphPin* OutputPin = FromPin->Direction == EGPD_Output ? FromPin : ToPin;
    UEdGraphPin* InputPin = FromPin->Direction == EGPD_Input ? FromPin : ToPin;
    const UEdGraphSchema* Schema = Graph->GetSchema();

    if (!Schema)
    {
        Result.message = TEXT("Graph schema is unavailable.");
        return Result;
    }

    {
        const FScopedTransaction Transaction(NSLOCTEXT("UnrealMCPTools", "ConnectPins", "MCP: Connect Pins"));
        Blueprint->Modify();
        Graph->Modify();
        OutputPin->Modify();
        InputPin->Modify();

        if (!Schema->TryCreateConnection(OutputPin, InputPin))
        {
            Result.message = TEXT("Graph schema rejected the pin connection.");
            return Result;
        }

        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
        Result = Success(Blueprint, TEXT("Pins connected."));
    }

    ApplyCompilePolicy(Blueprint, Result);
    return Result;
}

FMCPToolResult UUnrealMCPBlueprintToolset::BreakPinLinks(
    const FString& asset_path,
    const FString& graph_name,
    const FString& node_guid,
    const FString& pin_name)
{
    FMCPToolResult Result;
    FString Error;
    UBlueprint* Blueprint = LoadBlueprint(asset_path, Error, true);
    UEdGraph* Graph = Blueprint ? FindGraph(Blueprint, graph_name) : nullptr;

    if (!Blueprint || !Graph)
    {
        Result.message = Blueprint ? TEXT("Graph not found.") : Error;
        return Result;
    }

    UEdGraphNode* Node = FindNode(Blueprint, Graph, node_guid);
    UEdGraphPin* Pin = FindPin(Node, pin_name);
    if (!Pin)
    {
        Result.message = TEXT("Node or pin not found.");
        return Result;
    }

    {
        const FScopedTransaction Transaction(NSLOCTEXT("UnrealMCPTools", "BreakPinLinks", "MCP: Break Pin Links"));
        Blueprint->Modify();
        Graph->Modify();
        Pin->Modify();

        const UEdGraphSchema* Schema = Graph->GetSchema();
        if (!Schema)
        {
            Result.message = TEXT("Graph schema is unavailable.");
            return Result;
        }

        Schema->BreakPinLinks(*Pin, true);
        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
        Result = Success(Blueprint, TEXT("Pin links broken."));
    }

    ApplyCompilePolicy(Blueprint, Result);
    return Result;
}

FMCPToolResult UUnrealMCPBlueprintToolset::SetPinDefaultValue(
    const FString& asset_path,
    const FString& graph_name,
    const FString& node_guid,
    const FString& pin_name,
    const FString& default_value)
{
    FMCPToolResult Result;
    FString Error;
    UBlueprint* Blueprint = LoadBlueprint(asset_path, Error, true);
    UEdGraph* Graph = Blueprint ? FindGraph(Blueprint, graph_name) : nullptr;

    if (!Blueprint || !Graph)
    {
        Result.message = Blueprint ? TEXT("Graph not found.") : Error;
        return Result;
    }

    UEdGraphNode* Node = FindNode(Blueprint, Graph, node_guid);
    UEdGraphPin* Pin = FindPin(Node, pin_name);
    if (!Pin)
    {
        Result.message = TEXT("Node or pin not found.");
        return Result;
    }
    if (Pin->Direction != EGPD_Input)
    {
        Result.message = TEXT("Default values can only be set on input pins.");
        return Result;
    }

    const UEdGraphSchema* Schema = Graph->GetSchema();
    if (!Schema)
    {
        Result.message = TEXT("Graph schema is unavailable.");
        return Result;
    }

    {
        const FScopedTransaction Transaction(NSLOCTEXT("UnrealMCPTools", "SetPinDefaultValue", "MCP: Set Pin Default Value"));
        Blueprint->Modify();
        Graph->Modify();
        Pin->Modify();

        const FString PreviousValue = Pin->DefaultValue;
        Schema->TrySetDefaultValue(*Pin, default_value, true);
        const FString ValidationError = Schema->IsCurrentPinDefaultValid(Pin);
        if (!ValidationError.IsEmpty())
        {
            Schema->TrySetDefaultValue(*Pin, PreviousValue, true);
            Result.message = FString::Printf(TEXT("Default value is invalid: %s"), *ValidationError);
            return Result;
        }

        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
        Result = Success(Blueprint, TEXT("Pin default value set."));
    }

    ApplyCompilePolicy(Blueprint, Result);
    return Result;
}

FMCPToolResult UUnrealMCPBlueprintToolset::MoveNode(
    const FString& asset_path,
    const FString& graph_name,
    const FString& node_guid,
    const int32 x,
    const int32 y)
{
    FMCPToolResult Result;
    FString Error;
    UBlueprint* Blueprint = LoadBlueprint(asset_path, Error, true);
    UEdGraph* Graph = Blueprint ? FindGraph(Blueprint, graph_name) : nullptr;

    if (!Blueprint || !Graph)
    {
        Result.message = Blueprint ? TEXT("Graph not found.") : Error;
        return Result;
    }

    UEdGraphNode* Node = FindNode(Blueprint, Graph, node_guid);
    if (!Node)
    {
        Result.message = TEXT("Node not found in the requested graph.");
        return Result;
    }

    const FScopedTransaction Transaction(NSLOCTEXT("UnrealMCPTools", "MoveNode", "MCP: Move Node"));
    Blueprint->Modify();
    Graph->Modify();
    Node->Modify();
    Node->NodePosX = x;
    Node->NodePosY = y;
    Graph->NotifyGraphChanged();
    Blueprint->MarkPackageDirty();
    return Success(Blueprint, TEXT("Node moved."));
}

FMCPToolResult UUnrealMCPBlueprintToolset::DeleteNode(
    const FString& asset_path,
    const FString& graph_name,
    const FString& node_guid)
{
    FMCPToolResult Result;
    FString Error;
    UBlueprint* Blueprint = LoadBlueprint(asset_path, Error, true);
    UEdGraph* Graph = Blueprint ? FindGraph(Blueprint, graph_name) : nullptr;

    if (!Blueprint || !Graph)
    {
        Result.message = Blueprint ? TEXT("Graph not found.") : Error;
        return Result;
    }

    UEdGraphNode* Node = FindNode(Blueprint, Graph, node_guid);
    if (!Node)
    {
        Result.message = TEXT("Node not found in the requested graph.");
        return Result;
    }
    if (!Node->CanUserDeleteNode())
    {
        Result.message = TEXT("Unreal reports that this node cannot be deleted by the user.");
        return Result;
    }

    {
        const FScopedTransaction Transaction(NSLOCTEXT("UnrealMCPTools", "DeleteNode", "MCP: Delete Node"));
        Blueprint->Modify();
        Graph->Modify();
        Node->Modify();
        FBlueprintEditorUtils::RemoveNode(Blueprint, Node, true);
        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
        Result = Success(Blueprint, TEXT("Node deleted."));
    }

    ApplyCompilePolicy(Blueprint, Result);
    return Result;
}
