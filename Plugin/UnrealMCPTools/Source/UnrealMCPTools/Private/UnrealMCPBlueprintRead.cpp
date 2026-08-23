#include "UnrealMCPBlueprintInternal.h"

FMCPBlueprintSummary UUnrealMCPBlueprintToolset::GetBlueprintSummary(const FString& asset_path)
{
    FMCPBlueprintSummary Result;
    FString Error;
    UBlueprint* Blueprint = LoadBlueprint(asset_path, Error, false);
    if (!Blueprint)
    {
        Result.message = Error;
        return Result;
    }

    Result.success = true;
    Result.message = TEXT("Blueprint summary read successfully.");
    Result.asset_path = Blueprint->GetPathName();
    Result.parent_class = Blueprint->ParentClass ? Blueprint->ParentClass->GetPathName() : TEXT("");
    Result.blueprint_status = BlueprintStatusToString(Blueprint->Status);

    for (const FBPVariableDescription& Variable : Blueprint->NewVariables)
    {
        FMCPVariableInfo Info;
        Info.name = Variable.VarName.ToString();
        Info.type = PinTypeToString(Variable.VarType);
        Info.category = Variable.Category.ToString();
        Info.default_value = Variable.DefaultValue;
        Info.instance_editable =
            (Variable.PropertyFlags & CPF_Edit) != 0 &&
            (Variable.PropertyFlags & CPF_DisableEditOnInstance) == 0;
        Info.blueprint_read_only = (Variable.PropertyFlags & CPF_BlueprintReadOnly) != 0;
        Info.replicated = (Variable.PropertyFlags & CPF_Net) != 0;
        Info.rep_notify_function = Variable.RepNotifyFunc.ToString();
        Result.variables.Add(MoveTemp(Info));
    }

    if (Blueprint->SimpleConstructionScript)
    {
        for (USCS_Node* Node : Blueprint->SimpleConstructionScript->GetAllNodes())
        {
            if (!Node)
            {
                continue;
            }

            FMCPComponentInfo Info;
            Info.variable_name = Node->GetVariableName().ToString();
            Info.component_class = Node->ComponentClass ? Node->ComponentClass->GetPathName() : TEXT("");
            Info.parent_component = Node->ParentComponentOrVariableName.ToString();
            Info.is_root = Node->IsRootNode();
            Result.components.Add(MoveTemp(Info));
        }
    }

    const UUnrealMCPToolsSettings* Settings = GetDefault<UUnrealMCPToolsSettings>();
    auto AddGraphs = [&](const TArray<TObjectPtr<UEdGraph>>& Graphs)
    {
        for (UEdGraph* Graph : Graphs)
        {
            if (!Graph)
            {
                continue;
            }

            FMCPGraphInfo Info;
            Info.success = true;
            Info.message = TEXT("Graph read successfully.");
            Info.name = Graph->GetName();
            Info.graph_type = GetGraphType(Blueprint, Graph);
            Info.node_count = Graph->Nodes.Num();
            const int32 Limit = FMath::Max(1, Settings->MaxNodesPerGraphResponse);
            Info.truncated = Graph->Nodes.Num() > Limit;

            for (int32 Index = 0; Index < Graph->Nodes.Num() && Index < Limit; ++Index)
            {
                Info.nodes.Add(MakeNodeInfo(Graph->Nodes[Index]));
            }

            Result.node_count += Info.node_count;
            Result.graphs.Add(MoveTemp(Info));
        }
    };

    AddGraphs(Blueprint->UbergraphPages);
    AddGraphs(Blueprint->FunctionGraphs);
    AddGraphs(Blueprint->MacroGraphs);
    AddGraphs(Blueprint->DelegateSignatureGraphs);
    Result.graph_count = Result.graphs.Num();

    return Result;
}

FMCPGraphInfo UUnrealMCPBlueprintToolset::GetBlueprintGraph(
    const FString& asset_path,
    const FString& graph_name,
    int32 max_nodes)
{
    FMCPGraphInfo Result;
    FString Error;
    UBlueprint* Blueprint = LoadBlueprint(asset_path, Error, false);
    if (!Blueprint)
    {
        Result.name = graph_name;
        Result.message = Error;
        return Result;
    }

    UEdGraph* Graph = FindGraph(Blueprint, graph_name);
    if (!Graph)
    {
        Result.name = graph_name;
        Result.message = TEXT("Graph not found.");
        return Result;
    }

    const UUnrealMCPToolsSettings* Settings = GetDefault<UUnrealMCPToolsSettings>();
    const int32 Limit = max_nodes > 0 ? max_nodes : Settings->MaxNodesPerGraphResponse;

    Result.success = true;
    Result.message = TEXT("Graph read successfully.");
    Result.name = Graph->GetName();
    Result.graph_type = GetGraphType(Blueprint, Graph);
    Result.node_count = Graph->Nodes.Num();
    Result.truncated = Graph->Nodes.Num() > Limit;

    for (int32 Index = 0; Index < Graph->Nodes.Num() && Index < Limit; ++Index)
    {
        Result.nodes.Add(MakeNodeInfo(Graph->Nodes[Index]));
    }

    return Result;
}

FMCPCompileResult UUnrealMCPBlueprintToolset::CompileBlueprint(const FString& asset_path, const bool save_on_success)
{
    FString Error;
    UBlueprint* Blueprint = LoadBlueprint(asset_path, Error, true);
    if (!Blueprint)
    {
        FMCPCompileResult Result;
        Result.message = Error;
        Result.asset_path = asset_path;
        return Result;
    }

    return CompileInternal(Blueprint, save_on_success);
}


FMCPBlueprintAnalysis UUnrealMCPBlueprintToolset::AnalyzeBlueprint(const FString& asset_path)
{
    FMCPBlueprintAnalysis Result;
    FString Error;
    UBlueprint* Blueprint = LoadBlueprint(asset_path, Error, false);
    if (!Blueprint)
    {
        Result.findings.Add(Error);
        return Result;
    }

    Result.success = true;
    Result.asset_path = Blueprint->GetPathName();

    const UUnrealMCPToolsSettings* Settings = GetDefault<UUnrealMCPToolsSettings>();
    TArray<UEdGraph*> AllGraphs;
    for (UEdGraph* Graph : Blueprint->UbergraphPages) { if (Graph) AllGraphs.Add(Graph); }
    for (UEdGraph* Graph : Blueprint->FunctionGraphs) { if (Graph) AllGraphs.Add(Graph); }
    for (UEdGraph* Graph : Blueprint->MacroGraphs) { if (Graph) AllGraphs.Add(Graph); }

    for (UEdGraph* Graph : AllGraphs)
    {
        if (!Graph)
        {
            continue;
        }

        Result.total_nodes += Graph->Nodes.Num();

        if (Graph->Nodes.Num() >= Settings->LargeGraphNodeThreshold)
        {
            Result.findings.Add(FString::Printf(
                TEXT("Large graph '%s': %d nodes."),
                *Graph->GetName(),
                Graph->Nodes.Num()));
        }

        if (FBlueprintEditorUtils::CheckIfGraphHasLatentFunctions(Graph))
        {
            ++Result.latent_graphs;
        }

        for (UEdGraphNode* Node : Graph->Nodes)
        {
            if (Cast<UK2Node_DynamicCast>(Node))
            {
                ++Result.dynamic_cast_nodes;
            }

            if (UK2Node_CallFunction* Call = Cast<UK2Node_CallFunction>(Node))
            {
                if (const UFunction* Function = Call->GetTargetFunction())
                {
                    const FName Name = Function->GetFName();
                    if (Name == TEXT("GetAllActorsOfClass") ||
                        Name == TEXT("GetAllActorsWithInterface") ||
                        Name == TEXT("GetAllActorsWithTag"))
                    {
                        ++Result.get_all_actors_calls;
                        Result.findings.Add(FString::Printf(
                            TEXT("Global actor lookup in graph '%s': %s."),
                            *Graph->GetName(),
                            *Name.ToString()));
                    }
                }
            }
        }
    }

    if (Result.dynamic_cast_nodes >= Settings->ExcessiveDynamicCastThreshold)
    {
        Result.findings.Add(FString::Printf(
            TEXT("High dynamic-cast count: %d. Review whether interfaces/components/event dispatchers would reduce coupling."),
            Result.dynamic_cast_nodes));
    }

    if (Result.findings.IsEmpty())
    {
        Result.findings.Add(TEXT("No configured Blueprint smells were detected."));
    }

    return Result;
}
