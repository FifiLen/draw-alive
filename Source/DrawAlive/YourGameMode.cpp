#include "YourGameMode.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "ShapePickerWidget.h"
#include "Components/StaticMeshComponent.h"
#include "Components/LightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Camera/CameraActor.h"
#include "Components/SkyLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ShapeType.h"
#include "TimerManager.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

// Konstruktor
AYourGameMode::AYourGameMode()
{
    // Wczytanie klasy ShapePickerWidget
    static ConstructorHelpers::FClassFinder<UUserWidget> ShapePickerWidgetClassFinder(TEXT("/Game/BP_ShapePickerWidget"));
    if (ShapePickerWidgetClassFinder.Succeeded())
    {
        ShapePickerWidgetClass = ShapePickerWidgetClassFinder.Class;
    }

    // Wczytanie meshy dla kształtów
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        CubeStaticMesh = CubeMesh.Object;
    }

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded())
    {
        SphereStaticMesh = SphereMesh.Object;
    }

    static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
    if (ConeMesh.Succeeded())
    {
        ConeStaticMesh = ConeMesh.Object;
    }

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (CylinderMesh.Succeeded())
    {
        CylinderStaticMesh = CylinderMesh.Object;
    }

    // Wczytanie mesha płaszczyzny
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
    if (PlaneMesh.Succeeded())
    {
        WaterPlaneMesh = PlaneMesh.Object;
    }

    // Wczytanie domyślnego materiału
    static ConstructorHelpers::FObjectFinder<UMaterial> ShapeMaterial(TEXT("/Game/Materials/M_ShapeMaterial"));
    if (ShapeMaterial.Succeeded())
    {
        DefaultMaterial = ShapeMaterial.Object;
    }

    // Wczytanie materiału wody
    static ConstructorHelpers::FObjectFinder<UMaterial> WaterMaterialFinder(TEXT("/Game/Materials/M_WaterSurface"));
    if (WaterMaterialFinder.Succeeded())
    {
        WaterMaterial = WaterMaterialFinder.Object;
    }

    // Wczytanie systemu cząstek
    static ConstructorHelpers::FObjectFinder<UParticleSystem> BubblesParticleSystem(TEXT("/Game/Particles/P_Bubbles"));
    if (BubblesParticleSystem.Succeeded())
    {
        BubblesFX = BubblesParticleSystem.Object;
    }

    // Definicja granic obszaru ruchu
    AreaMin = FVector(-400.0f, -400.0f, 50.0f);
    AreaMax = FVector(400.0f, 400.0f, 200.0f);
}

// Funkcja wykonywana przy starcie gry
void AYourGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Utworzenie widżetu ShapePickerWidget i dodanie go do widoku
    if (ShapePickerWidgetClass)
    {
        ShapePickerWidgetInstance = CreateWidget<UShapePickerWidget>(GetWorld(), ShapePickerWidgetClass);
        if (ShapePickerWidgetInstance)
        {
            ShapePickerWidgetInstance->AddToViewport();
            ShapePickerWidgetInstance->SetVisibility(ESlateVisibility::Visible);
            ShapePickerWidgetInstance->OnShapeSelected.AddDynamic(this, &AYourGameMode::SpawnShape);
        }
    }

    // Utworzenie podłogi
    CreateWallOrFloor(FVector(0.0f, 0.0f, 0.0f), FVector(10.0f, 10.0f, 1.0f)); // Podłoga

    // Dodanie płaszczyzny wody
    if (WaterPlaneMesh)
    {
        // Ustawienie pozycji i rotacji płaszczyzny wody
        FVector WaterLocation = FVector(0.0f, 0.0f, 200.0f); // Poziom wody nad podłogą
        FRotator WaterRotation = FRotator(0.0f, 0.0f, 0.0f);

        // Utworzenie aktora StaticMeshActor dla wody
        AStaticMeshActor* WaterActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), WaterLocation, WaterRotation);
        if (WaterActor)
        {
            UStaticMeshComponent* MeshComponent = WaterActor->GetStaticMeshComponent();
            if (MeshComponent)
            {
                MeshComponent->SetStaticMesh(WaterPlaneMesh);
                MeshComponent->SetMobility(EComponentMobility::Static);
                MeshComponent->SetWorldScale3D(FVector(10.0f, 10.0f, 1.0f)); // Skalowanie płaszczyzny

                // Przypisanie materiału wody
                if (WaterMaterial)
                {
                    MeshComponent->SetMaterial(0, WaterMaterial);
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("WaterMaterial jest null."));
                }
                MeshComponent->SetCastShadow(false);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Nie udało się wczytać mesha płaszczyzny."));
    }

    // Dodanie Directional Light do sceny
    FVector LightLocation = FVector(0.0f, 0.0f, 300.0f);
    FRotator LightRotation = FRotator(-15.0f, 0.0f, 0.0f); // Mniejszy kąt nachylenia
    ADirectionalLight* DirectionalLight = GetWorld()->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), LightLocation, LightRotation);
    if (DirectionalLight)
    {
        DirectionalLight->SetMobility(EComponentMobility::Movable);
        DirectionalLight->GetLightComponent()->SetIntensity(3.0f); // Zmniejszona intensywność
        DirectionalLight->GetLightComponent()->SetLightColor(FLinearColor(0.9f, 0.95f, 1.0f)); // Chłodniejszy kolor światła
    }

    // Dodanie Sky Light do sceny
    ASkyLight* SkyLight = GetWorld()->SpawnActor<ASkyLight>(ASkyLight::StaticClass());
    if (SkyLight)
    {
        SkyLight->GetLightComponent()->SetIntensity(1.0f);
        SkyLight->GetLightComponent()->SetMobility(EComponentMobility::Movable);
        SkyLight->GetLightComponent()->SetLightColor(FLinearColor(0.8f, 0.9f, 1.0f)); // Chłodniejszy kolor
    }

    // Dodanie Post Process Volume
    APostProcessVolume* PostProcessVolume = GetWorld()->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
    if (PostProcessVolume)
    {
        PostProcessVolume->bUnbound = true; // Efekty globalne

        // Korekcja kolorów
        PostProcessVolume->Settings.bOverride_ColorSaturation = true;
        PostProcessVolume->Settings.ColorSaturation = FVector4(0.8f, 0.9f, 1.2f, 1.0f); // Lekkie zaniebieszczenie

        // Dostosowanie Bloom
        PostProcessVolume->Settings.bOverride_BloomIntensity = true;
        PostProcessVolume->Settings.BloomIntensity = 0.3f;

        // Dostosowanie Winiety
        PostProcessVolume->Settings.bOverride_VignetteIntensity = true;
        PostProcessVolume->Settings.VignetteIntensity = 0.2f;
    }

    // Dodanie Exponential Height Fog
    AExponentialHeightFog* ExponentialHeightFog = GetWorld()->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
    if (ExponentialHeightFog)
    {
        UExponentialHeightFogComponent* FogComponent = ExponentialHeightFog->GetComponent();
        if (FogComponent)
        {
            FogComponent->SetFogDensity(0.02f);
            FogComponent->SetFogInscatteringColor(FLinearColor(0.0f, 0.2f, 0.5f)); // Użycie metody settera
        }
    }

    // Dodanie systemu cząstek bąbelków
    if (BubblesFX)
    {
        FVector BubblesLocation = FVector(0.0f, 0.0f, 100.0f);
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BubblesFX, BubblesLocation, FRotator::ZeroRotator, true);
    }

    // Ustawienie kamery z perspektywy pierwszej osoby
    FVector CameraLocation = FVector(0.0f, -500.0f, 150.0f); // Pozycja kamery
    FVector TargetLocation = FVector(0.0f, 0.0f, 150.0f);    // Punkt, na który kamera patrzy
    FRotator CameraRotation = UKismetMathLibrary::FindLookAtRotation(CameraLocation, TargetLocation);

    ACameraActor* CameraActor = GetWorld()->SpawnActor<ACameraActor>(CameraLocation, CameraRotation);
    if (CameraActor)
    {
        APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
        if (PlayerController)
        {
            PlayerController->SetViewTarget(CameraActor);
        }
    }
}

// Definicja funkcji CreateWallOrFloor
void AYourGameMode::CreateWallOrFloor(FVector Location, FVector Scale)
{
    AStaticMeshActor* WallOrFloor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator);
    if (WallOrFloor)
    {
        UStaticMeshComponent* MeshComponent = WallOrFloor->GetStaticMeshComponent();
        if (MeshComponent)
        {
            MeshComponent->SetStaticMesh(CubeStaticMesh);
            MeshComponent->SetMobility(EComponentMobility::Static);
            MeshComponent->SetWorldScale3D(Scale);
        }
    }
}

FString AYourGameMode::OpenFileDialog()
{
    FString FilePath;
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (DesktopPlatform)
    {
        TArray<FString> OutFiles;
        FString DefaultPath = FPaths::ProjectDir(); // Lub dowolna ścieżka domyślna
        DesktopPlatform->OpenFileDialog(
            nullptr,
            TEXT("Wybierz obraz"),
            DefaultPath,
            TEXT(""),
            TEXT("Pliki graficzne|*.png;*.jpg;*.jpeg;*.bmp"),
            EFileDialogFlags::None,
            OutFiles
        );

        if (OutFiles.Num() > 0)
        {
            FilePath = OutFiles[0];
        }
    }
    return FilePath;
}

UTexture2D* AYourGameMode::LoadTextureFromFile(const FString& FilePath)
{
    if (FilePath.IsEmpty()) return nullptr;

    TArray<uint8> RawFileData;
    if (!FFileHelper::LoadFileToArray(RawFileData, *FilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("Nie udało się wczytać danych z pliku %s"), *FilePath);
        return nullptr;
    }

    EImageFormat ImageFormat = EImageFormat::PNG; // Domyślnie PNG

    if (FilePath.EndsWith(".jpg") || FilePath.EndsWith(".jpeg"))
    {
        ImageFormat = EImageFormat::JPEG;
    }
    else if (FilePath.EndsWith(".bmp"))
    {
        ImageFormat = EImageFormat::BMP;
    }

    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
    TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);

    if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
    {
        TArray<uint8> RawData;
        if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData))
        {
            UTexture2D* LoadedT2D = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);
            if (!LoadedT2D)
            {
                return nullptr;
            }

            void* TextureData = LoadedT2D->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
            FMemory::Memcpy(TextureData, RawData.GetData(), RawData.Num());
            LoadedT2D->GetPlatformData()->Mips[0].BulkData.Unlock();

            LoadedT2D->UpdateResource();

            return LoadedT2D;
        }
    }
    return nullptr;
}

// Funkcja tworząca wybrany kształt z teksturą użytkownika
void AYourGameMode::SpawnShape(EShapeType ShapeType, FString ImageFilePath)
{
    // Wczytanie tekstury z pliku
    UTexture2D* UserTexture = LoadTextureFromFile(ImageFilePath);

    if (!UserTexture)
    {
        UE_LOG(LogTemp, Warning, TEXT("Nie udało się wczytać tekstury z pliku."));
        return;
    }

    // Logowanie szczegółów tekstury
    UE_LOG(LogTemp, Log, TEXT("Wczytano teksturę: %s, Rozmiar: %dx%d"), *UserTexture->GetName(), UserTexture->GetSizeX(), UserTexture->GetSizeY());

    // Losowe położenie spawnu w obszarze ruchu
    FVector SpawnLocation = FVector(
        FMath::RandRange(AreaMin.X, AreaMax.X),
        FMath::RandRange(AreaMin.Y, AreaMax.Y),
        FMath::RandRange(AreaMin.Z, AreaMax.Z)
    );
    FRotator SpawnRotation = FRotator::ZeroRotator;

    UStaticMesh* ShapeMesh = nullptr;

    // Wybór mesha na podstawie typu kształtu
    switch (ShapeType)
    {
    case EShapeType::Cube:
        ShapeMesh = CubeStaticMesh;
        break;
    case EShapeType::Sphere:
        ShapeMesh = SphereStaticMesh;
        break;
    case EShapeType::Cone:
        ShapeMesh = ConeStaticMesh;
        break;
    case EShapeType::Cylinder:
        ShapeMesh = CylinderStaticMesh;
        break;
    }

    if (ShapeMesh && DefaultMaterial)
    {
        AStaticMeshActor* ShapeActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnLocation, SpawnRotation);
        if (ShapeActor)
        {
            UStaticMeshComponent* MeshComponent = ShapeActor->GetStaticMeshComponent();
            if (MeshComponent)
            {
                MeshComponent->SetMobility(EComponentMobility::Movable);
                MeshComponent->SetStaticMesh(ShapeMesh);
                MeshComponent->SetSimulatePhysics(true);

                UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(DefaultMaterial, this);
                if (!DynamicMaterial)
                {
                    UE_LOG(LogTemp, Error, TEXT("Nie udało się utworzyć dynamicznej instancji materiału."));
                    return;
                }

                // Ustawienie parametru tekstury
                DynamicMaterial->SetTextureParameterValue("BaseTexture", UserTexture);

                // Logowanie potwierdzenia
                UE_LOG(LogTemp, Log, TEXT("Zastosowano teksturę do dynamicznego materiału."));

                MeshComponent->SetMaterial(0, DynamicMaterial);

                EnableFloatingMovement(ShapeActor);
                LastSpawnedShape = ShapeActor;
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("MeshComponent jest null."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Nie udało się zespawnować ShapeActor."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ShapeMesh lub DefaultMaterial jest null."));
    }
}

// Funkcja umożliwiająca ruch kształtów z wykrywaniem kolizji
void AYourGameMode::EnableFloatingMovement(AStaticMeshActor* ShapeActor)
{
    if (!ShapeActor) return;

    float RandomSpeed = FMath::RandRange(0.5f, 1.5f);
    bool bIsFleeing = false;
    FVector CurrentDirection = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0).GetSafeNormal();

    FTimerHandle MovementTimerHandle;
    GetWorldTimerManager().SetTimer(MovementTimerHandle, [this, ShapeActor, RandomSpeed, CurrentDirection, bIsFleeing]() mutable
    {
        if (!ShapeActor) return;

        float Speed = bIsFleeing ? 3.0f : RandomSpeed;
        FVector NewLocation = ShapeActor->GetActorLocation();
        NewLocation += CurrentDirection * Speed * 10.0f * GetWorld()->DeltaTimeSeconds;

        // Ograniczenie nowej pozycji do obszaru ruchu
        NewLocation.X = FMath::Clamp(NewLocation.X, AreaMin.X, AreaMax.X);
        NewLocation.Y = FMath::Clamp(NewLocation.Y, AreaMin.Y, AreaMax.Y);
        NewLocation.Z = FMath::Clamp(NewLocation.Z, AreaMin.Z, AreaMax.Z);

        ShapeActor->SetActorLocation(NewLocation);

        // Wykrywanie pobliskich obiektów
        TArray<FHitResult> HitResults;
        FVector DetectionOrigin = ShapeActor->GetActorLocation();
        float DetectionRadius = 200.0f;
        FCollisionShape DetectionSphere = FCollisionShape::MakeSphere(DetectionRadius);

        if (GetWorld()->SweepMultiByChannel(HitResults, DetectionOrigin, DetectionOrigin, FQuat::Identity, ECC_WorldDynamic, DetectionSphere))
        {
            for (auto& Hit : HitResults)
            {
                if (Hit.GetActor() && Hit.GetActor() != ShapeActor)
                {
                    // Ucieczka, jeśli wykryto inny aktor
                    bIsFleeing = true;
                    CurrentDirection = (ShapeActor->GetActorLocation() - Hit.ImpactPoint).GetSafeNormal();
                    return;
                }
            }
        }
        else
        {
            bIsFleeing = false;
            // Okazjonalna zmiana kierunku
            if (FMath::FRand() < 0.05f)
            {
                CurrentDirection = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0).GetSafeNormal();
            }
        }

    }, 0.05f, true);
}
