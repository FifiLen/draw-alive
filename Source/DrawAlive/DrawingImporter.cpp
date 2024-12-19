#include "DrawingImporter.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "Misc/FileHelper.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "Modules/ModuleManager.h"
#include "Engine/Texture2D.h"
#include "RenderUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/StaticMeshActor.h" // Import aktora StaticMesh
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/StaticMeshComponent.h" // Import komponentu StaticMesh
#include "Engine/World.h"
#include "EngineUtils.h"

// Konstruktor
ADrawingImporter::ADrawingImporter()
{
    PrimaryActorTick.bCanEverTick = true; // Ustawienie, aby aktor mógł wykonywać Tick
    ImportedTexture = nullptr;
}

// Funkcja wywoływana po rozpoczęciu gry
void ADrawingImporter::BeginPlay()
{
    Super::BeginPlay();

    // Automatyczne wywołanie okna dialogowego po rozpoczęciu gry
    OpenFileDialogAndImport();

    // Znalezienie obiektu w scenie, do którego przypiszemy teksturę (np. StaticMesh)
    if (ImportedTexture)
    {
        for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
        {
            AStaticMeshActor* StaticMeshActor = *It;
            if (StaticMeshActor && StaticMeshActor->GetName().Contains("Cube"))
            {
                // Przypisz teksturę do znalezionego aktora (zakładamy, że tekstura jest już utworzona)
                ApplyTextureToMesh(StaticMeshActor->GetStaticMeshComponent(), ImportedTexture);
            }
        }
    }
}

// Funkcja wykonywana na każdej klatce gry
void ADrawingImporter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// Funkcja do importowania rysunku
void ADrawingImporter::ImportDrawing(const FString& FilePath)
{
    TArray<uint8> FileData;

    // Sprawdzenie, czy plik istnieje
    if (!FPaths::FileExists(FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Nie można znaleźć pliku: %s"), *FilePath);
        return;
    }

    // Wczytanie pliku do tablicy bajtów
    if (FFileHelper::LoadFileToArray(FileData, *FilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("Pomyślnie załadowano plik: %s"), *FilePath);

        // Ładowanie modułu ImageWrapper
        IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
        TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

        // Ustawienie danych obrazu
        if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num()))
        {
            TArray<uint8> UncompressedBGRA;
            if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
            {
                // Utworzenie tekstury z danych
                UTexture2D* Texture = UTexture2D::CreateTransient(
                    ImageWrapper->GetWidth(),
                    ImageWrapper->GetHeight(),
                    PF_B8G8R8A8);

                if (Texture)
                {
                    // Ustawienia tekstury - pozwala na modyfikację
                    Texture->MipGenSettings = TMGS_NoMipmaps;
                    Texture->CompressionSettings = TC_VectorDisplacementmap;
                    Texture->SRGB = true;

                    // Przekopiowanie danych do tekstury
                    void* TextureData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
                    FMemory::Memcpy(TextureData, UncompressedBGRA.GetData(), UncompressedBGRA.Num());
                    Texture->GetPlatformData()->Mips[0].BulkData.Unlock();

                    // Aktualizacja zasobu tekstury
                    Texture->UpdateResource();
                    UE_LOG(LogTemp, Warning, TEXT("Tekstura została utworzona!"));

                    ImportedTexture = Texture;  // Przechowaj zaimportowaną teksturę
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Nie udało się utworzyć tekstury!"));
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Nie udało się rozpakować danych obrazu!"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Nie udało się odczytać obrazu za pomocą ImageWrapper!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Nie udało się załadować pliku: %s"), *FilePath);
    }
}

// Funkcja otwierająca okno dialogowe do wyboru pliku
void ADrawingImporter::OpenFileDialogAndImport()
{
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (DesktopPlatform)
    {
        TArray<FString> OutFileNames;
        const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
        int32 OutFilterIndex;
        bool bOpened = DesktopPlatform->OpenFileDialog(
            ParentWindowHandle,
            TEXT("Wybierz rysunek do zaimportowania"),
            TEXT(""),
            TEXT(""),
            TEXT("Pliki graficzne (*.png; *.jpg; *.jpeg)|*.png;*.jpg;*.jpeg"),
            EFileDialogFlags::None,
            OutFileNames,
            OutFilterIndex
        );

        if (bOpened && OutFileNames.Num() > 0)
        {
            ImportDrawing(OutFileNames[0]);
        }
    }
}

// Funkcja przypisująca teksturę do StaticMeshComponent
void ADrawingImporter::ApplyTextureToMesh(UStaticMeshComponent* MeshComponent, UTexture2D* Texture)
{
    if (!MeshComponent || !Texture)
    {
        UE_LOG(LogTemp, Error, TEXT("Nie można przypisać tekstury. Brak obiektu StaticMeshComponent lub tekstury."));
        return;
    }

    UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
    if (DynamicMaterial)
    {
        DynamicMaterial->SetTextureParameterValue(FName("BaseTexture"), Texture);
        UE_LOG(LogTemp, Warning, TEXT("Tekstura została przypisana do obiektu."));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Nie udało się utworzyć dynamicznego materiału."));
    }
}
