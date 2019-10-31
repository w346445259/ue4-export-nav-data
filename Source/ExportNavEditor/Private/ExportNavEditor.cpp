// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ExportNavEditor.h"
#include "ExportNavStyle.h"
#include "ExportNavCommands.h"

#include "FlibExportNavData.h"

#include "Misc/MessageDialog.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "DesktopPlatformModule.h"
#include "LevelEditor.h"
#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"

static const FName ExportNavTabName("ExportNav");

#define LOCTEXT_NAMESPACE "FExportNavEditorModule"

void FExportNavEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FExportNavStyle::Initialize();
	FExportNavStyle::ReloadTextures();

	FExportNavCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FExportNavCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FExportNavEditorModule::PluginButtonClicked),
		FCanExecuteAction());
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FExportNavEditorModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FExportNavEditorModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}

void FExportNavEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FExportNavStyle::Shutdown();

	FExportNavCommands::Unregister();
}

void FExportNavEditorModule::PluginButtonClicked()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	FString MapName = World->GetMapName();

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	FString PluginPath = FPaths::ConvertRelativePathToFull(IPluginManager::Get().FindPlugin(TEXT("ExportNav"))->GetBaseDir());

	FString OutPath;
	if (DesktopPlatform)
	{
		const bool bOpened = DesktopPlatform->OpenDirectoryDialog(
			nullptr,
			LOCTEXT("SaveNav", "Save Recast Navigation NavMesh & NavData").ToString(),
			PluginPath,
			OutPath
		);
		if (!OutPath.IsEmpty() && FPaths::DirectoryExists(OutPath))
		{
			FString CurrentTime = FDateTime::Now().ToString();
			FString NavMeshFile = FPaths::Combine(OutPath, MapName + TEXT("-NavMesh-") + CurrentTime+TEXT(".obj"));
			DoExportNavMesh(NavMeshFile);
			FString NavDataFile = FPaths::Combine(OutPath, MapName + TEXT("-NavData-") + CurrentTime+TEXT(".bin"));
			DoExportNavData(NavDataFile);

#if PLATFORM_WINDOWS
			FString FinalCommdParas = TEXT("/e,/root,");
			FString OpenPath = UFlibExportNavData::ConvPath_Slash2BackSlash(OutPath);
			FinalCommdParas.Append(OpenPath);
			FPlatformProcess::CreateProc(TEXT("explorer "), *FinalCommdParas, true, false, false, NULL, NULL, NULL, NULL, NULL);
#endif
		}
	}
	
}

void FExportNavEditorModule::DoExportNavMesh(const FString& SaveToFile)
{
	UFlibExportNavData::ExportRecastNavMesh(SaveToFile);
}

void FExportNavEditorModule::DoExportNavData(const FString& SaveToFile)
{
	UFlibExportNavData::ExportRecastNavData(SaveToFile);

}

void FExportNavEditorModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FExportNavCommands::Get().PluginAction);
}



void FExportNavEditorModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FExportNavCommands::Get().PluginAction);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FExportNavEditorModule, ExportNavEditor)