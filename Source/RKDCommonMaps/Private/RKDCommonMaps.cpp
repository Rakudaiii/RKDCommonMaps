// Copyright (c) 2026 Rakudai. All Rights Reserved.

#include "RKDCommonMaps.h"
#include "AssetSelection.h"
#include "FileHelpers.h"
#include "RKDCommonMapsDeveloperSettings.h"

#define LOCTEXT_NAMESPACE "FRKDCommonMapsModule"

namespace RKDMapUtils
{
    static bool IsPlayWorldActive()
    {
        return GEditor->PlayWorld != nullptr;
    }

    static bool IsPlayWorldInactive()
    {
        return !IsPlayWorldActive();
    }

    static void OnMapSelected(const FString MapPath)
    {
        constexpr bool bPromptUserToSave = true;
        constexpr bool bSaveMapPackages = true;
        constexpr bool bSaveContentPackages = true;
        constexpr bool bFastSave = false;
        constexpr bool bNotifyNoPackagesSaved = false;
        constexpr bool bCanBeDeclined = true;
        if (FEditorFileUtils::SaveDirtyPackages(bPromptUserToSave, bSaveMapPackages, bSaveContentPackages, bFastSave,
                                                bNotifyNoPackagesSaved, bCanBeDeclined))
        {
            if (ensure(MapPath.Len()))
            {
                GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(MapPath);
            }
        }
    }

    static TSharedRef<SWidget> BuildMapDropdown()
    {
        FMenuBuilder MenuBuilder(true, nullptr);

        for (const auto& [CategoryName, Maps] : GetDefault<URKDCommonMapsDeveloperSettings>()->Maps)
        {
            TAttribute<FText> SectionText;
            SectionText.Set(FText::FromName(CategoryName));
            MenuBuilder.BeginSection(NAME_None, SectionText);
            for (const auto& Map : Maps.MapURL)
            {
                if (!Map.IsValid()) continue;

                const FText DisplayName = FText::FromString(Map.GetAssetName());
                MenuBuilder.AddMenuEntry(
                    DisplayName,
                    FText::Format(LOCTEXT("MapPathDescription", "{0}"), FText::FromString(Map.ToString())),
                    FSlateIcon(),
                    FUIAction(
                        FExecuteAction::CreateStatic(&RKDMapUtils::OnMapSelected, Map.ToString()),
                        FCanExecuteAction::CreateStatic(&RKDMapUtils::IsPlayWorldInactive),
                        FIsActionChecked(),
                        FIsActionButtonVisible::CreateStatic(&RKDMapUtils::IsPlayWorldInactive)
                    )
                );
            }
            MenuBuilder.EndSection();
        }

        return MenuBuilder.MakeWidget();
    }

    static bool ShouldShowMapList()
    {
        return IsPlayWorldInactive() && !GetDefault<URKDCommonMapsDeveloperSettings>()->Maps.IsEmpty();
    }

    static void SetupToolbarButton()
    {
        UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
        FToolMenuSection& Section = Menu->AddSection("RKDMapExtensions", TAttribute<FText>(),
                                                     FToolMenuInsert("Play", EToolMenuInsertType::Default));

        FToolMenuEntry MapListEntry = FToolMenuEntry::InitComboButton(
            "RKDMapList",
            FUIAction(
                FExecuteAction(),
                FCanExecuteAction::CreateStatic(&RKDMapUtils::IsPlayWorldInactive),
                FIsActionChecked(),
                FIsActionButtonVisible::CreateStatic(&RKDMapUtils::ShouldShowMapList)),
            FOnGetContent::CreateStatic(&RKDMapUtils::BuildMapDropdown),
            LOCTEXT("MapList_Label", "Common Maps"),
            LOCTEXT("MapList_ToolTip", "Quick access to commonly used maps"),
            FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Level")
        );
        MapListEntry.StyleNameOverride = "CalloutToolbar";
        Section.AddEntry(MapListEntry);
    }
}

void FRKDCommonMapsModule::StartupModule()
{
    RegisterContextMenu();

    if (!IsRunningGame())
    {
        if (FSlateApplication::IsInitialized())
        {
            ToolMenusHandle = UToolMenus::RegisterStartupCallback(
                FSimpleMulticastDelegate::FDelegate::CreateStatic(&RKDMapUtils::SetupToolbarButton));
        }
    }
}

void FRKDCommonMapsModule::RegisterContextMenu()
{
    UToolMenu* WorldAssetMenu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AssetContextMenu.World");
    if (!WorldAssetMenu) return;

    FToolMenuSection& Section = WorldAssetMenu->AddSection("RKDMapContextSection",
                                                           LOCTEXT("MapContextMenuHeading", "Common Maps"));

    Section.AddSubMenu(FName("RKDAddToMapList"),
        LOCTEXT("AddToMapList", "Add to Common Maps"),
        LOCTEXT("AddToMapListTooltip", "Add this map into Common Maps list."),
        FNewMenuDelegate::CreateRaw(this, &FRKDCommonMapsModule::BuildAddCategorySubmenu),
        FToolUIActionChoice(),
        EUserInterfaceActionType::None,
        false,
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Star"));

    Section.AddSubMenu(FName("RKDRemoveFromMapList"),
        LOCTEXT("RemoveFromMapList", "Remove from Common Maps"),
        LOCTEXT("RemoveFromMapListTooltip", "Remove this map from Common Maps list."),
        FNewMenuDelegate::CreateRaw(this, &FRKDCommonMapsModule::BuildRemoveCategorySubmenu),
        FToolUIActionChoice(),
        EUserInterfaceActionType::None,
        false,
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Delete"));
}

void FRKDCommonMapsModule::AddMapToCategory(FName CategoryName)
{
    TArray<FAssetData> SelectedAssets;
    AssetSelectionUtils::GetSelectedAssets(SelectedAssets);

    if (auto* Settings = GetMutableDefault<URKDCommonMapsDeveloperSettings>())
    {
        for (const auto& AssetData : SelectedAssets)
        {
            if (FCommonMapContainer* Category = Settings->Maps.Find(CategoryName))
                Category->MapURL.Add(AssetData.GetSoftObjectPath());
            else
                Settings->Maps.Add(CategoryName, FCommonMapContainer({AssetData.GetSoftObjectPath()}));
        }

        Settings->SaveConfig(CPF_Config);
    }
}

void FRKDCommonMapsModule::RemoveMapFromCategory(FName CategoryName)
{
    TArray<FAssetData> SelectedAssets;
    AssetSelectionUtils::GetSelectedAssets(SelectedAssets);

    if (auto* Settings = GetMutableDefault<URKDCommonMapsDeveloperSettings>())
    {
        for (const auto& AssetData : SelectedAssets)
        {
            if (FCommonMapContainer* Category = Settings->Maps.Find(CategoryName))
            {
                Category->MapURL.Remove(AssetData.GetSoftObjectPath());

                if (Category->MapURL.IsEmpty())
                    Settings->Maps.Remove(CategoryName);
            }
        }

        Settings->SaveConfig(CPF_Config);
    }
}

void FRKDCommonMapsModule::BuildAddCategorySubmenu(FMenuBuilder& MenuBuilder)
{
    const URKDCommonMapsDeveloperSettings* Settings = GetDefault<URKDCommonMapsDeveloperSettings>();
    if (!Settings) return;

    TArray<FName> Categories;
    Settings->Maps.GetKeys(Categories);

    for (const FName& CategoryName : Categories)
    {
        MenuBuilder.AddMenuEntry(
            FText::Format(LOCTEXT("AddCategoryLabel", "{0}"), FText::FromName(CategoryName)),
            FText::Format(LOCTEXT("AddCategoryTooltip", "Add this map to \"{0}\" category."), FText::FromName(CategoryName)),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateRaw(this, &FRKDCommonMapsModule::AddMapToCategory, CategoryName)));
    }
}

void FRKDCommonMapsModule::BuildRemoveCategorySubmenu(FMenuBuilder& MenuBuilder)
{
    TArray<FAssetData> SelectedAssets;
    AssetSelectionUtils::GetSelectedAssets(SelectedAssets);

    const URKDCommonMapsDeveloperSettings* Settings = GetDefault<URKDCommonMapsDeveloperSettings>();
    if (!Settings) return;

    bool bAnyEntryAdded = false;

    TArray<FName> Categories;
    Settings->Maps.GetKeys(Categories);

    for (const FName& CategoryName : Categories)
    {
        const FCommonMapContainer* Category = Settings->Maps.Find(CategoryName);
        if (!Category) continue;

        bool bFoundInCategory = false;
        for (const auto& AssetData : SelectedAssets)
        {
            if (Category->MapURL.Contains(AssetData.GetSoftObjectPath()))
            {
                bFoundInCategory = true;
                break;
            }
        }

        if (!bFoundInCategory) continue;

        MenuBuilder.AddMenuEntry(
            FText::Format(LOCTEXT("RemoveCategoryLabel", "{0}"), FText::FromName(CategoryName)),
            FText::Format(LOCTEXT("RemoveCategoryTooltip", "Remove this map from \"{0}\" category."), FText::FromName(CategoryName)),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateRaw(this, &FRKDCommonMapsModule::RemoveMapFromCategory, CategoryName)));

        bAnyEntryAdded = true;
    }

    if (!bAnyEntryAdded)
    {
        MenuBuilder.AddMenuEntry(
            LOCTEXT("MapNotListed", "Not in any Common Maps"),
            FText::GetEmpty(),
            FSlateIcon(),
            FUIAction(FExecuteAction(), FCanExecuteAction::CreateLambda([] { return false; }))
        );
    }
}

void FRKDCommonMapsModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRKDCommonMapsModule, RKDCommonMaps)