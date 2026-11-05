#pragma once
#include <Windows.h> 
#include <imgui.h>
#include <string>

namespace Cheat {
	class Options {
	public:
		struct LegitBot {
			struct AimBot
			{	        // Core Settings
				bool Enabled = false;
				bool ExtremeEnabled;
				bool VisibleEnabled;
				bool MouseEnabled;
				bool SniperScopeEnabled = false;



				int Mode = 0; // 0: Extreme, 1: Visible, 2: Mouse, 3: Silent

				// Activation Method
				bool WhenShooting = false;
				bool LMouse = true;
				int AimKey = 0;
				int SniperKey = 0;

				int RaiseEnemykey = 0;
				int MagnetEnemykey = 0;
				int TeleBaseEnemyKey = 0;
				int Telekey = 0;
				int speedkey = 0;
				int UnderGroundKillkey = 0;
				int FakeLagkey = 0;
				int FakeLagFreezekey = 0;


				int wallhackkey = 0;

				int cameraupkey = 0;
				int cameraleftkey = 0;

				// Target Logic
				bool ClosestEnemy = false;
				bool IgnoreDowned = false;
				bool IgnoreTrainingBots = false;
				bool IgnoreTeam = false;

				// Targeting Settings
				int Hitbox = 0;         // 0: Head, 1: Neck, 
				int SniperHitbox = 0;
				bool MultiComboTargetPriority[3] = { false, false, false }; // 0: Closest to Crosshair, 1: Lowest HP, 2: Nearest Distance
				// FOV Settings
				bool UseFOV = true;
				int FOVRange = 50;
				bool DrawFOV = false;



				// Distances (per mode)
				int DistanceExtreme = 100;
				int DistanceVisible = 300;
				int DistanceMouse = 100;
				int DistanceSilent = 200;

				// Delays (per mode)
				int DelayVisible = 10;
				//int DelayMouse = 20;

				//// Mouse-only
				//float SmoothMouse = 10.0f;

				int DelayMouse = 5;

				// Mouse-only
				float SmoothMouse = 5.0f;

				bool OnlyVisible = false; // Only for mouse mode

				// Silent-only options (optional)
				bool SilentEnabled = false;
				int SilentKey = 0;              // 0 = always active, otherwise hold to activate
				bool SilentLiteEnabled = false;
				bool ExtremeModeSilent = false;
				int SilentHitbox = 0;           // 0 = Head, 1 = Spine

				bool Aimkillenable = false;
				bool enablespeed = false;
			} AimBot;
		} LegitBot;
		struct HookSettings
		{
			int VariantIndex = 0;
			bool IsInjected = false;
			bool GameArch64 = false;

			std::string EmulatorName = "Unknown";
			std::string EmulatorVersion = "Unknown";
			bool EmulatorArch64 = false;

			std::string GameVersion = "Unknown";
		} Hook;
		struct Visuals
		{
			struct ESP
			{
				struct Players
				{
					// === General Settings ===
					bool Enabled = true;
					bool ShowLocalPlayer = false;
					bool radar = false;
					bool indicator = false;
					bool ShowNPCs = false;
					bool VisibleOnly = false;
					bool RefreshESP = false;
					int KeyBind = 0;
					int RenderDistance = 60;

					// === Filtering Options ===
					bool RainbowESP = false;
					bool IgnoreTrainingBots = false;
					bool IgnoreGameBots = false;
					bool IgnoreKnockedEnemies = false;
					ImColor KnockedEnemiesColor = ImColor(192, 0, 0, 255);


					// === Snaplines ===
					bool Lines = false;
					bool SnapLines = false;

					int TopPosStyles = 0;
					bool TintedCircle;
					int ShowLogo = 1;
					int EspLines = 0; // 0 = bottom, 1 = center, 2 = top, etc.
					ImColor SnapLinesColor = ImColor(255, 255, 255, 255);
					ImColor OriginLineColor = ImColor(255, 255, 255, 255);
					float LineThickness = 1.0f;
					bool GlowLines = false;
					float GlowRadius = 10.0f;
					float GlowFeather = 2.0f;

					// === Bounding Box ===
					bool Box = false;
					int players_box = 0; // 0 = 2D, 1 = Cornered, etc.
					int boxRounding = 0;
					ImColor BoxColor = ImColor(255, 255, 255, 255);
					bool fillBox = false;
					ImColor fillBoxColor = ImColor(0, 0, 0, 0); // semi-transparent
					bool FilledBox = false;
					float FillColor[4] = {0.0f, 0.0f, 0.0f, 0.5f};
					bool FillColorBox = false;
					float Filledboxcolor[4] = {0.0f, 0.0f, 0.0f, 0.5f};

					// === Name Tag ===
					bool Name = false;
					bool Uid  = false;
					bool ShowRank = false;
					int players_nametag = 0;
					ImColor NameColor = ImColor(255, 255, 255, 255);

					// === Distance Display ===
					bool Distance = false;
					ImColor DistColor = ImColor(255, 255, 255, 255);

					// === Health Display ===
					bool HealthBar = false;
					bool HealthPercentage = false;
					int players_healthbar = 1;
					int HealthBarPosition = 4; // 1=Right, 2=Left, 3=Top, 4=Bottom

					// === Armor Bar ===
					bool ArmorBar = false;
					int players_armorbar = 0;

					// === Skeleton ===
					bool Skeleton = false;
					ImColor SkeletonColor = ImColor(255, 255, 255, 255);
					float Thickness = 1.0f;
					bool Debug = false;

					// === Weapon Information ===
					bool WeaponName = false;
					bool WeaponIcon = false;
					ImColor WeaponColor = ImColor(255, 255, 255, 255);
					bool ESPWeapon = false;
					bool ESPWeaponIcon = false;

					// === Floating Information Tags ===
					bool EnemyInfo = false;
					bool WeaponInfo = false;

					// === Style / Effects ===
					bool RainbowMode = false;
					float RainbowSpeed = 1.0f;
					bool ShinyEffects = false;
					float ShinyThickness = 1.0f;
					bool Glow = true;
					bool RGB = false;
					float TextSize = 15.0f;
					bool Watermark = false;
					float WatermarkColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
					bool Level = false;
					bool ID = false;
					bool moco = false;
					bool HeadDot = false;
					float HeadDotColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
					float HeadDotSize = 3.0f;
					bool Enemy = false;
					float EnemyColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
					bool Wukong = false;
					int DistanceEsp = 300;
					float HipWidthScale = 0.8f;
					float HipWidthOffset = 0.5f;
					float LeftHipHeightOffset = 0.3f;
					float RightHipHeightOffset = 0.3f;

					// === Editor / Preview ===
					bool ShowESPEditor = false;
					bool CustomName = true;
					bool ShowTeamMates = false;

					// === Internal Use Only ===
					bool isVisible = false;
					bool ShowActiveSkills = false;
					ImColor ActiveSkillsColor = ImColor(103, 22, 138);

				} Players;

			} ESP;

			struct Chams
			{
				// Main toggles
				bool Chams3D = false;
				bool WhiteBody = false;
				bool BlackBody = false;
				bool ColorTexture = false;
				bool ColorGlow = false;

				// Colors
				float TextureColor[4] = { 0.53f, 0.81f, 0.92f, 1.0f };
				float TextureWireFrame[4] = { 0.53f, 0.81f, 0.92f, 1.0f };
				ImColor TextureOutline = ImColor(0.53f, 0.81f, 0.92f, 1.0f);
				float GlowColor[4] = { 249 / 255.f, 6 / 255.f, 6 / 255.f, 255 / 255.f };

				// Outline / wireframe
				float ThicknessOutline = 1.0f;
				bool Wireframe = false;
				bool Outline = false;

				// Texture maps
				int  ComboMap = 0;
				bool MultiComboTextures[3] = { false, false, false };
				bool TextureHDRMap = false;
				bool TextureShadowMap = false;

				// Special skins / textures
				bool TextureWukong = false;
				bool TextureOrion = false;
				bool TextureSkins = false;

			} Chams;


			bool HideFromCapture = false;

		} Visuals;
		struct Misc
		{
			struct Screen
			{
				bool ShowAimbotFov;
				float AimbotFovColor[4] = { 1.f, 1.f, 1.f, 1.f };
			} Screen;

			struct Exploits
			{
				struct LocalPlayer
				{
					// Combat Features
					bool RaiseEnemy = false;
					bool telekill = false;
					bool TeleBaseEnemy = false;
					bool UndergroundKillEnemy = false;

					// Aim Features
					bool aimlock = false;
					int  aimlockStrength = 100;
					bool MagnetEnemy = false;
					int MagnetEnemyKey = 0;
					// Magnet behavior speed multiplier (used by Magnet Enemy / 360 Pull feature)
					float MagnetSpeed = 1.0f;
					bool ForceAim = false;
					int ForceAimKey = 0;
					int ForceAimMode = 0;
					float ForceAimMaxPull = 8.0f;
					float ForceAimMaxPullVertical = 8.0f;
					bool PullEnemy360Enabled = false;
					int PullEnemy360Key = 0;
					int PullEnemy360TickMs = 6;
					float PullEnemy360MaxDistance = 250.0f;

					// AimLock
					bool AimLockEnabled = false;
					int AimLockKey = 0;

					// SmoothAimbot
					bool SmoothAimbotEnabled = false;
					int SmoothAimHoldKey = 0;
					float SmoothAimSmoothness = 5.0f;

					// SpoofName
					bool SpoofNameEnabled = false;
					float SpoofNameColor[4] = { 0.85f, 0.85f, 0.85f, 1.0f }; // Dark white color

					// Weapon Control
					bool norecoil = false;
					int NorecoilKey = 0;
					int  norecoilPercentage = 50;
					bool Noreload = false;
					int NoreloadKey = 0;

					// Speed & Movement
					bool speed = false;
					float SpeedMultiplier = 1.7f;
					int SpeedHackKey = 0;
					bool FastReload = false;
					bool FastShot = false;
					int FastShotKey = 0;
					bool FastSwitch = false;
					bool FastSwitchActive = false;
					float FastSwitchDelay = 1.0f;
					bool BackJump = false;
					int BackJumpKey = 0;

					// Ammo & Rapid Fire
					bool LastRapidFireState = false;
					bool InfiniteAmmo = false;
					bool InfiniteAmmoActive = false;
					int InfiniteAmmoKey = 0;
					// Health & Medkit
					int  health_ammount = 0;
					bool Start_Health = false;
					bool FastMedkit = false;
					bool FastMedkitActive = false;
					bool revive = false;

					// Hit Detection
					bool NoHitDelay = false;
					bool NoHitDelayActive = false;
					float NoHitDelayValue = 0.0f;

					// Visual Features
					bool onlyredxd = false;
					bool glitchfire = false;
					bool blacksky = false;
					bool wall = false;
					bool doublegun = false;
					bool cameraup = false;
					bool cameraleft = false;
					bool Ghost = false;
					bool HudVisible = false;
					bool crosshair = false;
					int VisionHackKey = 0;

					// Rapid Spin / Player Rotate
					bool playerRotate = false;
					int PlayerRotateKey = 0;
					float PlayerRotateSpeed = 45.0f;

					// Switching Features
					bool switchxd = false;
					bool ultraswitch = false;

					// Utility
					bool delay = false;
					bool reset = false;

					// Fly Hack
					bool FlyHackInternalEnabled = false;
					bool FlyHackExternalEnabled = false;
					int FlyHackExternalKey = 0;
					int FlyHackExternalOffKey = 0;
					bool NoGravityFlyEnabled = false;
					int NoGravityFlyKey = 0;

					// Climb Up
					bool ClimbUpEnabled = false;
					int ClimbUpKey = 0;

					// Shake Kill
					bool ShakeKillEnabled = false;
					int ShakeKillKey = 0;

					// Teleport Base
					bool TeleportBaseEnabled = false;
					int TeleportBaseKey = 0;
					float TeleportBaseDistance = 50.0f;

					// Map Teleport
					bool MapTeleportEnabled = false;
					int MapTeleportKey = 0;

					// Underground movement
					bool UndergroundEnabled = false;

					// Down player
					bool DownPlayerEnabled = false;
					float DownValue = -0.4f;
				}LocalPlayer;

				struct FakeLagX
				{
					bool EnableFreezeLag = false;
					bool EnableTeleportLag = false;
					bool EnableGhostLag = false;
					bool EnableClipLag = false;
					bool EnableTeleKillLag = false;

					bool FreezeLagAlways = false;
					bool TeleportLagAlways = false;
					bool GhostLagAlways = false;
					bool ClipLagAlways = false;
					bool TeleKillLagAlways = false;

					float FreezeLagDuration = 1.0f;
					float TeleportLagDuration = 1.0f;
					float GhostLagDuration = 1.0f;
					float ClipLagDuration = 1.0f;
					float TeleKillLagDuration = 1.0f;

					int FreezeLagKey = 0;
					int TeleportLagKey = 0;
					int GhostLagKey = 0;
					int GhostLagKeyxdxd = 0;
					int ClipLagKey = 0;
					int TeleKillLagKey = 0;
				} FakeLag;

			} Exploits;
		} Misc;
		struct General
		{
			char Username[25] = "";
			char PassWord[20] = "";
			bool ShutDown = false;
			int MenuKey = VK_INSERT;
			int SHUTDOWNKEY = VK_END;
			bool CaptureBypass = false;
			bool KeyBindMenu = false;
			int ThreadDelay = 0;
		} General;
	};
}
inline Cheat::Options g_Options;
