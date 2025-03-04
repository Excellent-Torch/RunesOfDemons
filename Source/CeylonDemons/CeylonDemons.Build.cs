// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class CeylonDemons : ModuleRules
{
	public CeylonDemons(ReadOnlyTargetRules Target) : base(Target)
	{
        // Include path for rendering dependencies
		// PrivateIncludePaths.AddRange(new string[] { Path.Combine(EngineDirectory, "Source/Runtime/Renderer/Private") });

        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", 
															"CoreUObject", 
															"Engine", 
															"InputCore",
															"EnhancedInput",
															// Rendering Dependencies
															"Renderer",
															"RenderCore",												
															// Effects
                                                            "Niagara",
															// Multiplayer Dependencies
															"OnlineSubsystem", 
															"OnlineSubsystemSteam",
                                                            "AdvancedSessions",
                                                            "AdvancedSteamSessions",
                                                            "MediaAssets"



                                                                                });


        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
    }
}
