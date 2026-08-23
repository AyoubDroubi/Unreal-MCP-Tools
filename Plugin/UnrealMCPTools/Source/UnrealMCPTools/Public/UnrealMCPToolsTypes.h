#pragma once

#include "CoreMinimal.h"
#include "UnrealMCPToolsTypes.generated.h"

USTRUCT(BlueprintType)
struct FMCPToolResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    bool success = false;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString message;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString asset_path;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString result_id;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    TArray<FString> warnings;
};

USTRUCT(BlueprintType)
struct FMCPPinInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString name;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString direction;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString type;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString default_value;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    TArray<FString> links;
};

USTRUCT(BlueprintType)
struct FMCPNodeInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString guid;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString class_path;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString title;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    int32 x = 0;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    int32 y = 0;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    TArray<FMCPPinInfo> pins;
};

USTRUCT(BlueprintType)
struct FMCPGraphInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    bool success = false;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString message;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString name;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString graph_type;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    int32 node_count = 0;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    bool truncated = false;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    TArray<FMCPNodeInfo> nodes;
};

USTRUCT(BlueprintType)
struct FMCPVariableInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString name;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString type;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString category;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString default_value;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    bool instance_editable = false;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    bool blueprint_read_only = false;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    bool replicated = false;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString rep_notify_function;
};

USTRUCT(BlueprintType)
struct FMCPComponentInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString variable_name;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString component_class;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString parent_component;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    bool is_root = false;
};

USTRUCT(BlueprintType)
struct FMCPBlueprintSummary
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    bool success = false;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString message;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString asset_path;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString parent_class;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString blueprint_status;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    int32 graph_count = 0;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    int32 node_count = 0;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    TArray<FMCPVariableInfo> variables;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    TArray<FMCPComponentInfo> components;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    TArray<FMCPGraphInfo> graphs;
};

USTRUCT(BlueprintType)
struct FMCPCompileResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    bool success = false;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString message;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString asset_path;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    int32 error_count = 0;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    int32 warning_count = 0;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    bool saved = false;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    TArray<FString> compiler_messages;
};

USTRUCT(BlueprintType)
struct FMCPAssetInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString asset_name;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString object_path;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString package_name;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString class_name;
};

USTRUCT(BlueprintType)
struct FMCPAssetSearchResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    bool success = false;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString message;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    bool truncated = false;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    TArray<FMCPAssetInfo> assets;
};

USTRUCT(BlueprintType)
struct FMCPReferenceResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    bool success = false;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString message;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString package_name;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    TArray<FString> packages;
};

USTRUCT(BlueprintType)
struct FMCPProjectOverview
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    bool success = false;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString project_name;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString engine_version;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString project_dir;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString content_dir;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString project_file;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    int32 asset_count = 0;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    int32 blueprint_count = 0;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    bool has_source = false;
};

USTRUCT(BlueprintType)
struct FMCPBlueprintAnalysis
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    bool success = false;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    FString asset_path;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    int32 total_nodes = 0;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    int32 dynamic_cast_nodes = 0;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    int32 get_all_actors_calls = 0;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    int32 latent_graphs = 0;

    UPROPERTY(BlueprintReadOnly, Category="MCP")
    TArray<FString> findings;
};
