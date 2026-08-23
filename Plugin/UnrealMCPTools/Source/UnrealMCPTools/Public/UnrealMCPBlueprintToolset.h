#pragma once

#include "CoreMinimal.h"
#include "ToolsetRegistry/ToolsetDefinition.h"
#include "UnrealMCPToolsTypes.h"
#include "UnrealMCPBlueprintToolset.generated.h"

/**
 * Safe, transaction-backed Blueprint inspection and graph editing tools for MCP clients.
 *
 * Write operations only target configured asset roots, modify through Unreal Editor APIs,
 * and optionally compile/rollback after semantic mutations.
 */
UCLASS(BlueprintType, Hidden)
class UNREALMCPTOOLS_API UUnrealMCPBlueprintToolset final : public UToolsetDefinition
{
    GENERATED_BODY()

public:
    /** Read a Blueprint's class, variables, components, and graph inventory without mutating it. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Blueprint", meta=(AICallable))
    static FMCPBlueprintSummary GetBlueprintSummary(const FString& asset_path);

    /** Read nodes and pins from one named Blueprint graph. Use this before node mutations to obtain exact GUIDs and pin names. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Blueprint", meta=(AICallable))
    static FMCPGraphInfo GetBlueprintGraph(const FString& asset_path, const FString& graph_name, int32 max_nodes = 0);

    /** Compile a Blueprint and optionally save it when compilation succeeds. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Blueprint", meta=(AICallable))
    static FMCPCompileResult CompileBlueprint(const FString& asset_path, bool save_on_success = false);

    /** Create a new Blueprint package under an allowed asset root using a loaded parent UClass path. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Blueprint", meta=(AICallable))
    static FMCPToolResult CreateBlueprint(const FString& package_path, const FString& parent_class_path);

    /** Add a user-created function graph. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Blueprint", meta=(AICallable))
    static FMCPToolResult CreateFunctionGraph(const FString& asset_path, const FString& function_name);

    /** Rename a graph while preserving Blueprint references through editor utilities. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Blueprint", meta=(AICallable))
    static FMCPToolResult RenameGraph(const FString& asset_path, const FString& graph_name, const FString& new_name);

    /**
     * Add a member variable.
     * variable_type: bool, byte, int, int64, float, double, string, name, text, object, class, or struct.
     * object_type_path is required for object/class/struct. container_type: scalar, array, or set.
     */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Blueprint", meta=(AICallable))
    static FMCPToolResult AddMemberVariable(
        const FString& asset_path,
        const FString& variable_name,
        const FString& variable_type,
        const FString& object_type_path = TEXT(""),
        const FString& container_type = TEXT("scalar"),
        const FString& default_value = TEXT(""));

    /** Rename a Blueprint member variable and update its references. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Blueprint", meta=(AICallable))
    static FMCPToolResult RenameMemberVariable(const FString& asset_path, const FString& variable_name, const FString& new_name);

    /** Remove a member variable declared by the target Blueprint. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Blueprint", meta=(AICallable))
    static FMCPToolResult RemoveMemberVariable(const FString& asset_path, const FString& variable_name);


    /**
     * Add a component to an Actor-based Blueprint through SimpleConstructionScript.
     * component_class_path must resolve to an UActorComponent subclass. parent_component is an optional SCS variable name.
     */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Blueprint", meta=(AICallable))
    static FMCPToolResult AddComponent(
        const FString& asset_path,
        const FString& component_class_path,
        const FString& component_name,
        const FString& parent_component = TEXT(""));

    /** Remove a component declared in the target Blueprint by SCS variable name. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Blueprint", meta=(AICallable))
    static FMCPToolResult RemoveComponent(const FString& asset_path, const FString& component_name);

    /** Add a call-function node for a Blueprint-callable or Blueprint-pure UFunction. Returns the new node GUID in result_id. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Graph", meta=(AICallable))
    static FMCPToolResult AddCallFunctionNode(
        const FString& asset_path,
        const FString& graph_name,
        const FString& owner_class_path,
        const FString& function_name,
        int32 x,
        int32 y);

    /** Add a Branch node. Returns its GUID in result_id. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Graph", meta=(AICallable))
    static FMCPToolResult AddBranchNode(const FString& asset_path, const FString& graph_name, int32 x, int32 y);

    /** Add a getter for a Blueprint member variable. Returns its GUID in result_id. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Graph", meta=(AICallable))
    static FMCPToolResult AddVariableGetNode(const FString& asset_path, const FString& graph_name, const FString& variable_name, int32 x, int32 y);

    /** Add a setter for a Blueprint member variable. Returns its GUID in result_id. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Graph", meta=(AICallable))
    static FMCPToolResult AddVariableSetNode(const FString& asset_path, const FString& graph_name, const FString& variable_name, int32 x, int32 y);

    /** Add a Custom Event node to an event-capable graph. Returns its GUID in result_id. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Graph", meta=(AICallable))
    static FMCPToolResult AddCustomEventNode(const FString& asset_path, const FString& graph_name, const FString& event_name, int32 x, int32 y);

    /** Connect two exact pins using their node GUIDs and pin names. Direction is inferred and schema compatibility is enforced. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Graph", meta=(AICallable))
    static FMCPToolResult ConnectPins(
        const FString& asset_path,
        const FString& graph_name,
        const FString& from_node_guid,
        const FString& from_pin_name,
        const FString& to_node_guid,
        const FString& to_pin_name);

    /** Break every link on one exact node pin. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Graph", meta=(AICallable))
    static FMCPToolResult BreakPinLinks(
        const FString& asset_path,
        const FString& graph_name,
        const FString& node_guid,
        const FString& pin_name);

    /** Set the serialized default value of an input pin through the graph schema. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Graph", meta=(AICallable))
    static FMCPToolResult SetPinDefaultValue(
        const FString& asset_path,
        const FString& graph_name,
        const FString& node_guid,
        const FString& pin_name,
        const FString& default_value);

    /** Move one graph node. Layout-only mutation: this does not trigger a compile. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Graph", meta=(AICallable))
    static FMCPToolResult MoveNode(
        const FString& asset_path,
        const FString& graph_name,
        const FString& node_guid,
        int32 x,
        int32 y);

    /** Delete a user-deletable node by GUID. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Graph", meta=(AICallable))
    static FMCPToolResult DeleteNode(
        const FString& asset_path,
        const FString& graph_name,
        const FString& node_guid);

    /** Inspect Blueprint complexity and common performance/maintainability smells without mutating the asset. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Analysis", meta=(AICallable))
    static FMCPBlueprintAnalysis AnalyzeBlueprint(const FString& asset_path);
};
