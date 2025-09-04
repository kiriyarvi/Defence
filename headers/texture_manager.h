#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>

enum class TextureID {
    Grass,
    RoadTileset,
    AlphaltRoadTileset,
    GunBase,
    TwinGunTurret,
    TwinGunUpperBarrel,
    Shot,
    AntitankGunTurret,
    AntitankGunBarrel,
    AntitankGunTurretSubstrate,
    MiniGun,
    MiniGunEquipment,
    Mine,
    MineBlast,
    ButtonBackground,
    ButtonClickedBackground,
    MinigunIcon,
    TwingunIcon,
    TwingunConstructed,
    AntitankGunIcon,
    AntitankGunConstructed,
    SpikesRight,
    SpikesT,
    SpikesCross,
    SpikesD,
    SpikesIcon,
    Hedgehog,
    Locked,
    Tank,
    TankDestroyed,
    Truck,
    TruckDestroyed,
    MedBlustOfDestruction1,
    MedBlustOfDestruction2,
    Bike,
    SolderWalkAnimation,
    SolderAmmunition,
    DeadSolder,
    DestroyedBike,
    DoubleBlust,
    Blusts16x16,
    RepairWrench,
    Pickup,
    PickupDestroyed,
    BTR,
    BTRDestroyed,
    Trucks,
    CruiserIBase,
    CruiserIEquipment,
    UpgradeButtonBackground,
    UpgradeButtonBackgroundCompleted,
    MinigunShellsUpgradeI,
    MinigunShellsUpgradeII,
    MinigunShellsUpgradeIII,
    MinigunCoolingUpgradeI,
    MinigunLubricantUpgradeI,
    MinigunShells,
    Arrow,
    Path,
    NextWaveIcon,
    Question,
    CruiserIDemagedBase,
    CruiserIDemagedEquipment,
    CruiserISupplementary,
    CruiserIDestroyed,
    CruiserIBlust
};

class TextureManager {
public:
    static TextureManager& Instance() {
        static TextureManager instance; // Создаётся при первом вызове, потокобезопасно в C++11+
        return instance;
    }

    // Удаляем копирование и перемещение
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
    TextureManager(TextureManager&&) = delete;
    TextureManager& operator=(TextureManager&&) = delete;

    std::unordered_map<TextureID, sf::Texture> textures;
private:
    TextureManager();
};
