#include "texture_manager.h"

TextureManager::TextureManager() {
    textures[TextureID::Grass].loadFromFile("tiles/grass.png");
    textures[TextureID::Road1100].loadFromFile("tiles/road1100.png");
    textures[TextureID::Road0110].loadFromFile("tiles/road0110.png");
    textures[TextureID::Road0011].loadFromFile("tiles/road0011.png");
    textures[TextureID::Road1001].loadFromFile("tiles/road1001.png");
    textures[TextureID::Road1010].loadFromFile("tiles/road1010.png");
    textures[TextureID::Road0101].loadFromFile("tiles/road0101.png");
    textures[TextureID::Road0111].loadFromFile("tiles/road0111.png");
    textures[TextureID::Road1011].loadFromFile("tiles/road1011.png");
    textures[TextureID::Road1101].loadFromFile("tiles/road1101.png");
    textures[TextureID::Road1110].loadFromFile("tiles/road1110.png");
    textures[TextureID::Road1111].loadFromFile("tiles/road1111.png");

    textures[TextureID::GunBase].loadFromFile("sprites/gun_base.png");
    textures[TextureID::TwinGunTurret].loadFromFile("sprites/twin_gun_turret.png");
    textures[TextureID::TwinGunUpperBarrel].loadFromFile("sprites/twin_gun_upper_barrel.png");
    textures[TextureID::Shot].loadFromFile("sprites/shot.png");
             
    textures[TextureID::AntitankGunTurret].loadFromFile("sprites/antitank_gun_turret.png");
    textures[TextureID::AntitankGunBarrel].loadFromFile("sprites/antitank_gun_barrel.png");
    textures[TextureID::AntitankGunTurretSubstrate].loadFromFile("sprites/antitank_gun_turret_substrate.png");
             
    textures[TextureID::MiniGun].loadFromFile("sprites/minigun.png");
    textures[TextureID::MiniGunEquipment].loadFromFile("sprites/minigun_equipment.png");
    textures[TextureID::MineBlast].loadFromFile("sprites/mine_blast.png");
    textures[TextureID::Mine].loadFromFile("sprites/mine.png");
             
    textures[TextureID::ButtonBackground].loadFromFile("sprites/button_background.png");
    textures[TextureID::ButtonClickedBackground].loadFromFile("sprites/button_clicked_background.png");
    textures[TextureID::MinigunIcon].loadFromFile("sprites/minigun_icon.png");
    textures[TextureID::TwingunIcon].loadFromFile("sprites/twingun_icon.png");
    textures[TextureID::TwingunConstructed].loadFromFile("sprites/twingun_constructed.png");
    textures[TextureID::AntitankGunIcon].loadFromFile("sprites/antitank_gun_icon.png");
    textures[TextureID::AntitankGunConstructed].loadFromFile("sprites/antitank_gun_constructed.png");
             
    textures[TextureID::SpikesT].loadFromFile("sprites/spikes_T.png");
    textures[TextureID::SpikesRight].loadFromFile("sprites/spikes_right.png");
    textures[TextureID::SpikesCross].loadFromFile("sprites/spikes_cross.png");
    textures[TextureID::SpikesD].loadFromFile("sprites/spikes_d.png");
    textures[TextureID::SpikesIcon].loadFromFile("sprites/spikes_icon.png");
             
    textures[TextureID::Hedgehog].loadFromFile("sprites/hedgehog.png");
             
    textures[TextureID::Locked].loadFromFile("sprites/lock.png");


    textures[TextureID::Tank].loadFromFile("sprites/tank.png");
    textures[TextureID::TankDestroyed].loadFromFile("sprites/tank_destroyed.png");
    textures[TextureID::Truck].loadFromFile("sprites/truck.png");
    textures[TextureID::TruckDestroyed].loadFromFile("sprites/truck_destroyed.png");
    textures[TextureID::MedBlustOfDestruction1].loadFromFile("sprites/med_blust_of_destruction1.png");
    textures[TextureID::MedBlustOfDestruction2].loadFromFile("sprites/med_blust_of_destruction2.png");
    textures[TextureID::Bike].loadFromFile("sprites/bike.png");
    textures[TextureID::SolderWalkAnimation].loadFromFile("sprites/solder_walk_animation.png");
    textures[TextureID::SolderAmmunition].loadFromFile("sprites/solder_ammunition.png");
    textures[TextureID::DeadSolder].loadFromFile("sprites/dead_solder.png");
    textures[TextureID::DestroyedBike].loadFromFile("sprites/destroyed_bike.png");
    textures[TextureID::DoubleBlust].loadFromFile("sprites/double_blust.png");
    textures[TextureID::Blusts16x16].loadFromFile("sprites/16x16_blusts.png");
    textures[TextureID::RepairWrench].loadFromFile("sprites/repair_wrench.png");
    textures[TextureID::Pickup].loadFromFile("sprites/pickup.png");
    textures[TextureID::PickupDestroyed].loadFromFile("sprites/pickup_destroyed.png");
    textures[TextureID::BTR].loadFromFile("sprites/BTR.png");
    textures[TextureID::Trucks].loadFromFile("sprites/Trucks.png");
    textures[TextureID::Trucks].setRepeated(true);
    textures[TextureID::BTRDestroyed].loadFromFile("sprites/BTR_destroyed.png");
    textures[TextureID::CruiserIBase].loadFromFile("sprites/cruiser_I_base.png");
    textures[TextureID::CruiserIEquipment].loadFromFile("sprites/cruiser_I_equipment.png");

    textures[TextureID::UpgradeButtonBackground].loadFromFile("sprites/upgrade_button.png");
    textures[TextureID::UpgradeButtonBackgroundCompleted].loadFromFile("sprites/upgrade_button_completed.png");
    textures[TextureID::MinigunShellsUpgradeI].loadFromFile("sprites/minigun_shells_upgrade_I.png");
    textures[TextureID::MinigunCoolingUpgradeI].loadFromFile("sprites/minigun_cooling_upgrade_I.png");
    textures[TextureID::MinigunLubricantUpgradeI].loadFromFile("sprites/minigun_lubricant_upgrade_I.png");

    textures[TextureID::Arrow].loadFromFile("sprites/arrow.png");
    textures[TextureID::Path].loadFromFile("sprites/path.png");
    textures[TextureID::Path].setRepeated(true);

    textures[TextureID::NextWaveIcon].loadFromFile("sprites/next_wave_button.png");
    textures[TextureID::Question].loadFromFile("sprites/question.png");
}
