#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>

enum class TextureID {
    Grass,
    Road1100 = 12, // 1100: 1 - дорога направо, 1 - дорога наверх, 0 - нет дорого налево, 0 - нет дороги наверх
    Road0110 = 6,
    Road0011 = 3,
    Road1001 = 9,
    Road1010 = 10,
    Road0101 = 5,
    Road0111 = 7,
    Road1011 = 11,
    Road1101 = 13,
    Road1110 = 14,
    Road1111 = 15,
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
    MinigunCoolingUpgradeI,
    MinigunLubricantUpgradeI,
    Arrow,
    Path,
    NextWaveIcon,
    Question
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
