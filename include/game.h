#include <StarCitizen.h>

#define sincos(radian, s, c) s = sin(radian); c = cos(radian)
#define EXTRACT_LOWER_BYTES(ptr) ((ptr) & 0xFFFFFFFFFFFF)

/* gamepad keys */
#define BUTTON_DPAD_UP          XINPUT_GAMEPAD_DPAD_UP   
#define BUTTON_DPAD_DOWN        XINPUT_GAMEPAD_DPAD_DOWN 
#define BUTTON_DPAD_LEFT        XINPUT_GAMEPAD_DPAD_LEFT 
#define BUTTON_DPAD_RIGHT       XINPUT_GAMEPAD_DPAD_RIGHT
#define BUTTON_START            XINPUT_GAMEPAD_START
#define BUTTON_BACK             XINPUT_GAMEPAD_BACK 
#define BUTTON_L_THUMB			XINPUT_GAMEPAD_LEFT_THUMB	
#define BUTTON_R_THUMB			XINPUT_GAMEPAD_RIGHT_THUMB	
#define BUTTON_L_SHOULDER		XINPUT_GAMEPAD_LEFT_SHOULDER
#define BUTTON_R_SHOULDER		XINPUT_GAMEPAD_RIGHT_SHOULDER
#define BUTTON_A                XINPUT_GAMEPAD_A
#define BUTTON_B                XINPUT_GAMEPAD_B
#define BUTTON_X                XINPUT_GAMEPAD_X
#define BUTTON_Y                XINPUT_GAMEPAD_Y
#define BUTTON_L_TRIGGER        0x0150
#define BUTTON_R_TRIGGER        0x0250


namespace StarCitizen
{
	typedef std::vector<std::string> vecString;
	typedef std::pair<std::string, Classes::CEntityClass*> pairEntityClass;
	typedef std::vector<pairEntityClass> vecEntityClassPair;

	namespace Enums
	{
		enum EDefaultLoadout : unsigned char
		{
			EDEFAULTLOADOUT_PU_DEFAULT = 0,
			EDEFAULTLOADOUT_PU_HEALING_TOOL,
			EDEFAULTLOADOUT_PU_MINING_TOOL,
			EDEFAULTLOADOUT_ZEUS,
			EDEFAULTLOADOUT_WHALEMAN,
			EDEFAULTLOADOUT_THE_DIRECTOR,
			EDEFAULTLOADOUT_PU_TRACTOR_BEAM,
			EDEFAULTLOADOUT_MFT_LOADOUT_CQC,	
			EDEFAULTLOADOUT_MFT_LOADOUT_SALVAGE,
			EDEFAULTLOADOUT_DEFAULT_EVERYTHING_NEW_MOBI,
			EDEFAULTLOADOUT_DEV_INVIS_PLAYER_01,
			EDEFAULTLOADOUT_MEDICAL_BODY,
			EDEFAULTLOADOUT_MEDICAL_SKELETON,
			EDEFAULTLOADOUT_CHARACTER_RESET = EDEFAULTLOADOUT_DEFAULT_EVERYTHING_NEW_MOBI
		};

		enum ELoadout : uint16_t 
		{
			ELOADOUT_AC_AC_PREVIEW_CONTROL = 0,
			ELOADOUT_AC_AC_PREVIEW_ELIMINATION = 1,
			ELOADOUT_AC_AC_PREVIEW_FPSGUNGAME_AZURE = 2,
			ELOADOUT_AC_AC_PREVIEW_FPSGUNGAME_CERULEAN = 3,
			ELOADOUT_AC_AC_PREVIEW_FPSGUNGAME_CHOCOLATE = 4,
			ELOADOUT_AC_AC_PREVIEW_FPSGUNGAME_EMERALD = 5,
			ELOADOUT_AC_AC_PREVIEW_FPSGUNGAME_FOREST = 6,
			ELOADOUT_AC_AC_PREVIEW_FPSGUNGAME_GREEN = 7,
			ELOADOUT_AC_AC_PREVIEW_FPSGUNGAME_INDIGO = 8,
			ELOADOUT_AC_AC_PREVIEW_FPSGUNGAME_LEMON = 9,
			ELOADOUT_AC_AC_PREVIEW_FPSGUNGAME_MAROON = 10,
			ELOADOUT_AC_AC_PREVIEW_FPSGUNGAME_NAVY = 11,
			ELOADOUT_AC_AC_PREVIEW_FPSGUNGAME_ORANGE = 12,
			ELOADOUT_AC_AC_PREVIEW_FPSGUNGAME_PINK = 13,
			ELOADOUT_AC_AC_PREVIEW_FPSGUNGAME_RED = 14,
			ELOADOUT_AC_AC_PREVIEW_FPSGUNGAME_SAND = 15,
			ELOADOUT_AC_AC_PREVIEW_FPSGUNGAME_VIOLET = 16,
			ELOADOUT_AC_AC_PREVIEW_FPSGUNGAME_WINE = 17,
			ELOADOUT_AC_AC_PREVIEW_FPSGUNGAME_YELLOW = 18,
			ELOADOUT_AC_AC_PREVIEW_FPSKILLCONFIRMED = 19,
			ELOADOUT_AC_AC_PREVIEW_SWELIMINATION = 20,
			ELOADOUT_AC_AC_PREVIEW_TEAMELIMINATION = 21,
			ELOADOUT_DEFAULTLOADOUTS_ARENACOMMANDER_DEFAULT = 22,
			ELOADOUT_DEFAULTLOADOUTS_BRIGHTSKY = 23,
			ELOADOUT_DEFAULTLOADOUTS_CAPTAIN_LOADOUT = 24,
			ELOADOUT_DEFAULTLOADOUTS_CARGOPERSON01 = 25,
			ELOADOUT_DEFAULTLOADOUTS_CARGOPERSON02 = 26,
			ELOADOUT_DEFAULTLOADOUTS_CARGOPERSON03 = 27,
			ELOADOUT_DEFAULTLOADOUTS_CUSTOMIZER_MILKMAN = 28,
			ELOADOUT_DEFAULTLOADOUTS_CUSTOMIZER_REVIEW_01 = 29,
			ELOADOUT_DEFAULTLOADOUTS_CUSTOMIZER_REVIEW_02 = 30,
			ELOADOUT_DEFAULTLOADOUTS_CUSTOMIZER_REVIEW_03 = 31,
			ELOADOUT_DEFAULTLOADOUTS_HEAD_DEFAULT = 32,
			ELOADOUT_DEFAULTLOADOUTS_HELMET_DEFAULT = 33,
			ELOADOUT_DEFAULTLOADOUTS_HOSPITAL_DEFAULT = 34,
			ELOADOUT_DEFAULTLOADOUTS_HOSPITAL_INVENTORY_DEFAULT = 35,
			ELOADOUT_DEFAULTLOADOUTS_MANNEQUIN_DEFAULT = 36,
			ELOADOUT_DEFAULTLOADOUTS_MANNEQUIN_HEAD_DEFAULT = 37,
			ELOADOUT_DEFAULTLOADOUTS_MANNEQUIN_MOBIGLAS = 38,
			ELOADOUT_DEFAULTLOADOUTS_MANNEQUIN_UNDERSUIT = 39,
			ELOADOUT_DEFAULTLOADOUTS_MANNEQUIN_UNDERSUIT_CORE = 40,
			ELOADOUT_DEFAULTLOADOUTS_MANNEQUIN_UNDERSUIT_HELMET = 41,
			ELOADOUT_DEFAULTLOADOUTS_MEDICAL_SHIP_DEFAULT = 42,
			ELOADOUT_DEFAULTLOADOUTS_MFTLOADOUT = 43,
			ELOADOUT_DEFAULTLOADOUTS_MFTLOADOUT_CQC = 44,
			ELOADOUT_DEFAULTLOADOUTS_MFTLOADOUT_LMG = 45,
			ELOADOUT_DEFAULTLOADOUTS_MFTLOADOUT_SALVAGE = 46,
			ELOADOUT_DEFAULTLOADOUTS_MFTLOADOUT_SNIPER = 47,
			ELOADOUT_DEFAULTLOADOUTS_NAKED_DEFAULT = 48,
			ELOADOUT_DEFAULTLOADOUTS_NAKED_PERSON_FEMALE_01 = 49,
			ELOADOUT_DEFAULTLOADOUTS_NAKED_PERSON_MALE_01 = 50,
			ELOADOUT_DEFAULTLOADOUTS_PRISON_DEFAULT = 51,
			ELOADOUT_DEFAULTLOADOUTS_PU_COMBATREADY = 52,
			ELOADOUT_DEFAULTLOADOUTS_PU_DEFAULT = 53,
			ELOADOUT_DEFAULTLOADOUTS_PU_HEAD_DEFAULT = 54,
			ELOADOUT_DEFAULTLOADOUTS_PU_HEALINGTOOL = 55,
			ELOADOUT_DEFAULTLOADOUTS_PU_MININGTOOL = 56,
			ELOADOUT_DEFAULTLOADOUTS_PU_NPE = 57,
			ELOADOUT_DEFAULTLOADOUTS_PU_TRACTORBEAM = 58,
			ELOADOUT_DEFAULTLOADOUTS_PVE_HEAVY_BEHR_LMG_BEHR_SHOTGUN = 59,
			ELOADOUT_DEFAULTLOADOUTS_PVE_LIGHT_BEHR_SMG = 60,
			ELOADOUT_DEFAULTLOADOUTS_PVE_MEDIUM_BEHR_RIFLE_GMNI_RIFLE = 61,
			ELOADOUT_DEFAULTLOADOUTS_PVE_MEDIUM_BEHR_SNIPER__GMNI_SMG = 62,
			ELOADOUT_DEFAULTLOADOUTS_STARMARINE_ELIMINATION = 63,
			ELOADOUT_DEFAULTLOADOUTS_STARMARINE_MARINE = 64,
			ELOADOUT_DEFAULTLOADOUTS_STARMARINE_SLAVER = 65,
			ELOADOUT_DEFAULTLOADOUTS_THEDIRECTOR = 66,
			ELOADOUT_DEFAULTLOADOUTS_VLK_STORM_LOADOUT = 67,
			ELOADOUT_DEFAULTLOADOUTS_WHALEMAN = 68,
			ELOADOUT_DEFAULTLOADOUTS_ZEUS = 69,
			ELOADOUT_DEFAULTLOADOUTS_QASPECIALISTLOADOUT_HEALINGSPECIALIST = 70,
			ELOADOUT_DEMO_CITIZENCON2023_CITCON23_PLAYER_01 = 71,
			ELOADOUT_DEMO_CITIZENCON2023_CITCON23_PLAYER_OPTION1 = 72,
			ELOADOUT_DEMO_CITIZENCON2023_CITCON23_PLAYER_OPTION2 = 73,
			ELOADOUT_DEMO_CITIZENCON2023_CITCON23_PLAYER_OPTION2A = 74,
			ELOADOUT_DEMO_CITIZENCON2023_CITCON23_PLAYER_OPTION2B = 75,
			ELOADOUT_DEMO_CITIZENCON2023_CITCON23_PLAYER_OPTION3 = 76,
			ELOADOUT_DEMO_CITIZENCON2023_CITCON23_PLAYER_OPTION4 = 77,
			ELOADOUT_DEMO_CITIZENCON2023_CITCON23_PLAYER_OPTION5 = 78,
			ELOADOUT_DEMO_CITIZENCON2023_FPS_COMBAT_LOADOUT = 79,
			ELOADOUT_DEV_INVIS_PLAYER_01 = 80,
			ELOADOUT_EA_RACING_EA_RACING_DEFAULT = 81,
			ELOADOUT_EA_SINGLEWEAPONELIM_SINGLEWEAPONELIM_A03 = 82,
			ELOADOUT_EA_SINGLEWEAPONELIM_SINGLEWEAPONELIM_ARCLIGHT = 83,
			ELOADOUT_EA_SINGLEWEAPONELIM_SINGLEWEAPONELIM_CODA = 84,
			ELOADOUT_EA_SINGLEWEAPONELIM_SINGLEWEAPONELIM_GRENADELAUNCHER = 85,
			ELOADOUT_EA_SINGLEWEAPONELIM_SINGLEWEAPONELIM_RAILGUN = 86,
			ELOADOUT_EA_TOW_DEFAULTS_OUTLAWS_LOADOUT01_M_FOOTSOLDIER = 87,
			ELOADOUT_EA_TOW_DEFAULTS_OUTLAWS_LOADOUT02_M_ASSAULT = 88,
			ELOADOUT_EA_TOW_DEFAULTS_OUTLAWS_LOADOUT03_M_SNIPER = 89,
			ELOADOUT_EA_TOW_DEFAULTS_OUTLAWS_LOADOUT04_M_ANTIVEHICLE = 90,
			ELOADOUT_EA_TOW_DEFAULTS_UEE_LOADOUT01_M_FOOTSOLDIER = 91,
			ELOADOUT_EA_TOW_DEFAULTS_UEE_LOADOUT02_M_ASSAULT = 92,
			ELOADOUT_EA_TOW_DEFAULTS_UEE_LOADOUT03_M_SNIPER = 93,
			ELOADOUT_EA_TOW_DEFAULTS_UEE_LOADOUT04_M_ANTIVEHICLE = 94,
			ELOADOUT_EQUIPMENT_SETS_MAINTAINANCE_SET_1 = 95,
			ELOADOUT_EQUIPMENT_SETS_SAFETY_OFFICER_1 = 96,
			ELOADOUT_STARMARINE_STARMARINE_MARINE_HEAVY = 97,
			ELOADOUT_STARMARINE_STARMARINE_MARINE_LIGHT = 98,
			ELOADOUT_STARMARINE_STARMARINE_MARINE_MEDIUM = 99,
			ELOADOUT_STARMARINE_STARMARINE_MARINE_UNDERSUIT = 100,
			ELOADOUT_STARMARINE_STARMARINE_SLAVER_HEAVY = 101,
			ELOADOUT_STARMARINE_STARMARINE_SLAVER_LIGHT = 102,
			ELOADOUT_STARMARINE_STARMARINE_SLAVER_MEDIUM = 103,
			ELOADOUT_STARMARINE_STARMARINE_SLAVER_UNDERSUIT = 104,
			ELOADOUT_STARMARINE_WEAPONSYSTEM = 105,
			ELOADOUT_UI_MEDICALBODY = 106,
			ELOADOUT_UI_MEDICALSKELETON = 107,
			ELOADOUT_UI_M_MED_BODY_ARML = 108,
			ELOADOUT_UI_M_MED_BODY_ARMR = 109,
			ELOADOUT_UI_M_MED_BODY_HEAD = 110,
			ELOADOUT_UI_M_MED_BODY_LEGL = 111,
			ELOADOUT_UI_M_MED_BODY_LEGR = 112,
			ELOADOUT_UI_M_MED_BODY_TORSO = 113,
			ELOADOUT_UI_M_MED_SKELETON_ARML = 114,
			ELOADOUT_UI_M_MED_SKELETON_ARMR = 115,
			ELOADOUT_UI_M_MED_SKELETON_HEAD = 116,
			ELOADOUT_UI_M_MED_SKELETON_LEGL = 117,
			ELOADOUT_UI_M_MED_SKELETON_LEGR = 118,
			ELOADOUT_UI_M_MED_SKELETON_TORSO = 119,
			ELOADOUT_CDS_MEDIUM_ARMOR = 120,
			ELOADOUT_CDS_UNDERSUIT_ARMOR = 121,
			ELOADOUT_CIVILLIAN_CLOTHING_STOCKED_WEAPON = 122,
			ELOADOUT_DEFAULT_EVERYTHING_LOADOUT = 123,
			ELOADOUT_DEFAULT_EVERYTHING_LOADOUT_NEWMOBI = 124,
			ELOADOUT_M_RSI_DECKCREW_EVA = 125,
			ELOADOUT_M_RSI_EXPLORER = 126,
			ELOADOUT_COUNT
		};

		enum EEntityType : unsigned int
		{
			ET_PLAYER = 0,
			ET_NPC,
			ET_ENEMY_NPC,
			ET_ANIMAL,
			ET_SHIP,
			ET_CONTAINER,
			ET_ROCK,
			ET_ORBIT,
			ET_MAX
		};

		enum EWeaponType : unsigned int
		{
			EW_NONE = 0,
			EW_GRENADE,
			EW_PISTOL,
			EW_RIFLE,
			EW_SHOTGUN,
			EW_SNIPER,
			EW_LMG,
			EW_SMG,
			EW_MAX
		};
	}

	namespace Structs
	{
		struct SDataCache
		{
			Classes::CEntityClass* pPlayer;
			Classes::CEntityClass* pOrbitingContainer;
			Classes::CEntityClass* pGoToPoint;
			Classes::CEntityClass* pMissionMarker;
			vecEntityClassPair vNPC;			
			vecEntityClassPair vEnemyNPC;			
			vecEntityClassPair vAnimals;			
			vecEntityClassPair vShips;			
			vecEntityClassPair vContainers;		
			vecEntityClassPair vRocks;		
			vecEntityClassPair vKiosks;
		};

		struct SHitResult
		{
			__int64 pEntity;
			__int64 pEntity_HitPart;
			__int64 pShooter;
			__int64 pWeapon;
			std::string mShooterEntityName;
			std::string mHitEntityName;
			std::string mHitPartName;
			std::string mWeaponName;
		};

		struct SWaypoint
		{
		public:
			std::string						mName{ 0 };						//	Waypoint Name
			Structs::DVector				mLocalPos{ 0, 0, 0 };			//	Waypoint player local Position
			Structs::DVector				mWorldPos{ 0, 0, 0 };			//	Waypoint player world Position
			Structs::DQuat					mWorldRot{ 0, 0, 0, 0 };		//	Waypoint Player Rotation
			Structs::DVector				mZoneLocalPos{ 0, 0, 0 };		//	Waypoint zone local Position which should be used for returning to waypoint
			class Classes::CZone*			pZone{ nullptr };				//	Waypoint Zone
			class Classes::CEntity*			pZoneEntity{ nullptr };			//	Zone Entity
			class Classes::CEntity*			pEntity{ nullptr };				//	Player Entity
			Structs::FQuat					mColor{ 1.f, 1.f, 1.f, 1.f };	//	Waypoint Name Color
			bool							bValid{ false };				//	is waypoint valid ?	
			bool							bRender{ true };				//	should render the waypoint ?
			bool							bTeleport{ false };				//	bit for teleporting
			bool							bLockToWaypoint{ false };		//	bit to hold pEntity to the waypoint
			bool							bRemove{ false };				//	bit to remove the waypoint
		public:
			Structs::DVector				dbg_origin;						//	origin point ( this world position does not get updated but should be used for debug path rendering )
		public:
			void update();
			void clear();

		public:
			SWaypoint() = default;
			SWaypoint(const char* name);
		};

		struct STransforms
		{
		public:
			class Classes::CEntity*			pEntity{nullptr};				//
			bool							bIsActor{ false };				//	
			float							health{ 0.0f };					//
			Structs::DVector				origin{ 0, 0, 0 };				//	
			Structs::DQuat					rotation{ 0, 0, 0, 0 };			//	
			Structs::FVector				angles{ 0, 0, 0 };				//	
			Structs::FVector				direction{ 0, 0, 0 };			//	
			Structs::DBox					box{ DVector(), DVector() };	//		
			std::vector<Structs::DVector>	bones;							//	
			bool							bValidBones{ false };			// is bones array valid

		public:
			void update();	//	updates or resets transform
			void clear();

		public:
			STransforms() = default;
			STransforms(class Classes::CEntity* entity);
		};

		struct SDrawEntity
		{
		public:
			class Classes::CEntity*			pEntity{nullptr};			//
			bool							bValid{ false };			//
			bool							bIsActor{ false };			//	
			bool							bOrigin{ false };			//
			FVector2D 						origin{ 0, 0 };				//
			bool							bOriginCenter{ false };		//
			FVector2D 						originCenter{ 0, 0 };		//
			bool							bOriginTop{ false };		//
			FVector2D 						originTop{ 0, 0 };			//
			bool							bOriginFWD{ false };		//
			FVector2D						originFWD{ 0, 0 };			//
			bool							bBoxVerts[8]{ false };		//
			FVector2D						boxVerts[8];				//
			bool							bBoneHead{ false };			//
			FVector2D						boneHead{ 0, 0 };			//
			float							boneHeadRadius{ 0.f };		//
			std::vector<FQuat>				bones;						//
		};

		struct STargetEntity
		{
		public:
			class Classes::CEntity*			pEntity{ nullptr };				//	Target Entity
			std::string						mName;							//	Target Entity Name	
			STransforms						TM;								//	Target Entity Transforms
			SDrawEntity						screenTM;						//	Target Entity Screen Transforms
			bool							bValid{ false };				//	is entity valid ?
			bool							bRender{ false };				//	should render the player name tag ?
			bool							bSticky{ false };				//	used to teleport & stick to the target entity 
			bool							bTeleport{ false };				//	teleport to the target entity
			bool							bKill{ false };					//	kill the target entity
			bool							bOnScreen{ false };				//	is the target entity on screen
			bool							bForge{ false };				//	is the target entity in forge mode
		
		public:
			void update();	//	updates or clears the target entity
			void clear();

		public:
			STargetEntity() = default;
			STargetEntity(class Classes::CEntity* entity);
		};

		struct SThreadEntity
		{
			Classes::CEntity* pEntity{ nullptr };				//	Thread Entity
			Enums::EEntityType entityType{ EEntityType::ET_MAX };	//	Thread Entity Type
			STransforms TM;										//	Thread Entity Transforms
			SDrawEntity screenTM;								//	Thread Entity Screen Transforms
		};

		struct SLocalPlayer
		{
		public:
			class Classes::CEntity*			pEntity{ nullptr };				//	Local Player Entity
			class Classes::CEntity*			pShipEntity{nullptr};			//	local player ship entity
			Structs::STransforms			TM;								//	Local Player Transforms
			float health{ 0.0f };											//	Local Player Health
			bool							bValid{ false };				//	is local player valid ?
		};

		struct SForgeControls
		{
			float distance{ 5.f };			//	distance to the target entity
			float height{ 0.f };			//	height to the target entity
			float deltaX{ 0.0f };			//	
			float deltaY{ 0.0f };			//	
			float deltaZ{ 0.0f };			//	
		};
	}

	namespace Offsets
	{
		/*
			useful strings
			CTextMessages::PushEntry_Text - useful in finding project to screen
		
		
		*/

		/* patches */
		inline auto oDisableCrashDumps = 0x78B669E;						//	mov     edx, 0A000h     ; dwFlags	|	BA ?? ?? ?? ?? 48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 85 C0

		/* static pointers */
		inline constexpr auto gEnv = 0x9F99E40;										//	
		inline constexpr auto gGoToPointMan = 0xA27CC28;							//	E8 ? ? ? ? 48 8B F8 48 8B 0E 4C 8B 41 ? 41 8D 55 or "Found goto point named %s"
		
		/* EAC */
		inline auto oEAC_HandleDiscipline = 0x6807CD0;					//	"HandleEACResult GlobalGameUI"	| E8 ? ? ? ? C6 87 ? ? ? ? ? 48 8B CF E8 ? ? ? ? 8B D8
		inline auto oBypassPUCheckpoint = 0x24EB5D3;						//	.text:00000001424EB9D3                 jnz     short loc_1424EBA2E	|	"Change Server Start" | "IsShardPersisted[$$] IsServer[$$] IsMultiplayer[$$]"


		/* CXConsole */
		inline auto oCXConsole_RegisterCvar_Int = 0x7918B90;				//	"[CVARS]: [DUPLICATE] CXConsole::Register(int): variable [%s] is already registered"								
		inline auto oCXConsole_RegisterCvar_Float = 0x7918CD0;			//	"[CVARS]: [DUPLICATE] CXConsole::Register(float): variable [%s] is already registered"						
		inline auto oCXConsole_RegisterCvar_String = 0x7918E20;			//	"[CVARS]: [DUPLICATE] CXConsole::Register(const char*): variable [%s] is already registered"								
		inline auto oCXConsole_RegisterCvar_Int64 = 0x7918F70;			//	"[CVARS]: [DUPLICATE] CXConsole::Register(int64): variable [%s] is already registered"								
		inline auto oCXConsole_AddCommand = 0x78ACE90;					//	"[CVARS]: [DUPLICATE] CXConsole::AddCommand(): console command [%s] is already registered"						
		inline auto oCXConsole_GetCommand = 0x78E5960;					//	"GetConsoleCommand(\"%s\") called"	
		inline auto oCXConsole_GetCVar = 0x78E55D0;						//	"GetCVar(\"%s\") called"
		
		/* CXCommands */
		inline constexpr auto oCXCommand_MegaMap = 0x26BB3D0;	//	"Load a map, same usage as 'megamap' cvar."
		inline constexpr auto oCXCommand_LoadMegaMap = 0x26BAE40;	// called via MegaMap ; x__LoadMegaMap(qword_14A21FBB0, v3, (const char *)&szString); ; "Requesting game mode %s/%s"

		/* CDataCore */
		inline auto oCDataCore_RegisterStruct = 0x77FD880;				//	"[DataCore] RegisterStruct: Attempt to register '%s' multiple times"												
		inline auto oCDataCore_GetStructDataFields = 0x77DFFA0;			//	"CDataCore::GetStructDataFields - [%s] has no DCStructDesc"															

		/* CEntityClassRegistry */
		inline auto oCEntityClassRegistry_RegisterClass = 0x6EA2BA0;		//	"CEntityClassRegistry::RegisterClass"	|	@NOTE: Must be called from game thread								
		inline auto oCEntityClassRegistry_FindClass = 0x6E7F350;			//	"CEntityClassRegistry::FindClass"	2nd xref	|	@NOTE: Must be called from game thread		

		/* CGoToPointManager */
		inline constexpr auto oCGoToPointManager_GetGoToPointByName = 0x3F677E0;	//	"CGoToPointsManager::GetPointByName"

		/* CSystem */
		inline auto oCSystem_Update = 0x79295B0;							//	"CSystem::Update"	: "ICharacterManager::Update()"
		inline auto oCSystem_Init = 0x78EF8D0;							//	"CSystem::Init"	

		/* C3DEngine */
		inline constexpr auto oC3DEngine_RenderWorld = 0x727AFC0;					//	"C3DEngine::RenderWorld" : "e_DebugDraw = %d"

		/* CRenderer */
		inline constexpr auto oCRenderer_MTUpdate = 0x0977190;						//	"CRenderer::MT_Update" VFIndex = 7
		inline constexpr auto oCRenderer_ProjectToScreen = 0x097A990;				//	VFIndex = 66
		inline constexpr auto oCRenderer_DrawText = 0x0961300;						//	VFIndex = 169
		inline constexpr auto oCRenderer_DrawText2 = 0x0961540;						//	VFIndex = 170
		inline constexpr auto oCRenderer_DrawTextArgs = 0x09613E0;					//	VFIndex = 171
		inline constexpr auto oCRenderer_DrawTextArgs2 = 0x09615E0;					//	VFIndex = 172
		inline constexpr auto oCRenderer_FlushTextMessages = 0x097E650;				//	VFIndex = 303	|	"CRenderer::RT_FlushTextMessages"

		/* CCamera */
		inline constexpr auto oCCamerViewManager_Update = 0x3B27170;				//	"CCameraViewManager::Update"	

		/* CSCLocalPlayerMovement */
		inline constexpr auto oCSCLocalPlayerMovement_Update = 0x4A1C3E0;			//  E8 ? ? ? ? C5 FA 5F FE ; grab the additive as well	; [4.0.1d]

		/* CEntity */
		inline constexpr auto oCEntity_Init = 0x6E8EAA0;							//	"Entity %llu %s is being initialized from an unexpected state (%u)"
		inline constexpr auto oCEntity_Shutdown = 0x6EAF760;						//	"CEntity::ShutDown" or "[Entity] CEntitySystem::DeleteEntity %s %s %llu" - function call after isValid check

		/* entity helpers */
		inline constexpr auto oIsValidEntity = 0x0317A90;							//	^ found when looking for CEntity::ShutDown	:	C5 F2 5E F0 E8 ?? ?? ?? ?? 84 C0 75 05 ; is_valid_handle_typeB
		inline constexpr auto oGetRenderProxy = 0x03439B0;							//	E8 ?? ?? ?? ?? 48 8B 0F 48 89 4B 10 : "IEntityRenderProxy"
		inline constexpr auto oIsValidRenderProxy = 0x038B430;						//	xref GetRenderProxy , is generally the following call ; AssetMeta::HasActorSubresource

		/* CEntitySystem */
		inline constexpr auto oCEntitySystem_Update = 0x6EB3980;					//	"CEntitySystem::Update"
		inline constexpr auto oCEntitySystem_SpawnEntity = 0x6EB0030;				//	"CEntitySystem::SpawnEntityImpl"
		inline constexpr auto oCEntitySystem_DeleteEntity = 0x6E768C0;				//	"CEntitySystem::DeleteEntity"
		inline constexpr auto oCEntitySystem_GetEntityZoneByID = 0x6E84B10;			//  "CEntitySystem::GetEntityFromIDInternal" xref is method +11 before method+49

		/* CRenderProxy */
		inline constexpr auto oCRenderProxy_GetLocalBounds = 0x6DF2940;				//	"CRenderProxy::GetLocalBounds" 2nd xref
		inline constexpr auto oCActor_GetBoneTransform = 0x68E6B60;					//	"CSCActorResultAdditiveStateDematerialize::SpawnEffectAtBone" around Ln. 30 ; [4.0.1d]							

		/* */
		inline constexpr auto oCRigidEntity_VerifyExistingContacts = 0x68066C0;		//	"CRigidEntity::VerifyExistingContacts"

		/* [DEV] Fly Mode Command */
		inline constexpr auto oGetIActor = 0x690C070;								//		"Not enabling no clip fly mode after goto as the dev tools DLL is missing."
		inline constexpr auto oCCharacterStateHiearchy_SetState = 0x5DE8660;		//		"Not enabling no clip fly mode after goto as the dev tools DLL is missing."
		inline constexpr auto oCCharacterStateHiearchy_VerifyState = 0x510F160;		//		"FlyMode/NoClip ON"

		/* Weapon and Equipment params */
		inline constexpr auto oCSCAmmoContainerComponent_GetAmmoCount = 0x59290C0;	//		"Weapon::CAmmoContainerComponent::GetAmmoCount" - "Weapon::Action::SWeaponActionFireSingleState" analyze vfTable between __StarEngineModule__ calls
		inline constexpr auto oCWeaponActionFireSingle_Update = 0x5C391F0;			//		"Weapon::Action::CWeaponActionFireSingle::Update"
		inline constexpr auto oCWeaponActionFireBurst_Update = 0x5B62E50;			//		"Weapon::Action::CWeaponActionFireBurst::Update"
		inline constexpr auto oCWeaponActionFireRapid_Update = 0x5B63130;			//		"Weapon::Action::CWeaponActionFireRapid::Update"
		inline constexpr auto oCWeaponActionFireHealingBeam_GetRayCastRequest = 0x5D435F0;		//		"CWeaponActionFireHealingBeam::GetRayCastRequest"	
		inline constexpr auto oCWeaponActionFireTractorBeam_GetRayCastRequest = 0x5D43B50;		//		"CWeaponActionFireTractorBeam::GetRayCastRequest"	
		inline constexpr auto oCWeaponActionFireSalvageRepair_GetRayCastRequest = 0x5D438A0;	//		"CWeaponActionFireSalvageRepair::GetRayCastRequest"	//	2nd xref
		inline constexpr auto oCWeaponActionFireBeam_GetRayCastRequest = 0x5D428B0;				//		"CWeaponActionFireBeam::GetRayCastRequest"

		/* CInventoryComponent */
		inline constexpr auto oCInventoryComponent_AddItem = 0x039BD10;				//	called via "CSCLocalPlayerPersonalThoughtComponent::OnInventoryMoveItemOntoUnoccupiedPosition"
		inline constexpr auto oCSCLocalPlayerPersonalThoughtComponent_OnInventoryMoveItemOntoUnoccupiedPosition = 0x62547F0;	//	"CSCLocalPlayerPersonalThoughtComponent::OnInventoryMoveItemOntoUnoccupiedPosition"

		/* CSCItemQuantumDrive */
		//	inline constexpr auto oCSCItemQuantumDriveOnStateChanged = 0x55E1800;		// inside "CSCItemQuantumDrive::TogglePower" at the very bottom C5 FA 11 40 08 * * * * * * * * 24 24 // Insta Spool
		inline constexpr auto oCSCActorGForce_Update = 0x3F2FD70;					// "CSCActorGForce::PostUpdate"	
		inline constexpr auto oCSCItemQuantumDrive_Update = 0x52C57E0;				// "CSCItemQuantumDrive::Update"

		/* Ship Components */
		//	inline constexpr auto oCSCItemShieldEmitter_Update = 0x3919BE0;				// "CSCItemShieldEmitter::Update"
		//	inline constexpr auto oCEntityComponentUIMap_Update = 0x41FEC10;			// "CEntityComponentUIMap::Update" // Infinite Quantum Fuel // C5 FA 11 80 8C 1A 00 00 and 60 C5 FB 10 48 10 * * * * 24 D0
		inline constexpr auto oCSCItemSalvageController_OnRaycastSubmit_stub = 0x6072350;	// "Salvage::CSCItemSalvageController::OnRaycastSubmit"
		inline constexpr auto oCSCItemMiningController_Update = 0x60A6970;					// "CSCItemMiningController::Update"
		inline constexpr auto oCSCItemMiningController_UpdateLaserThrottle = 0x33EB2B0;		//	E8 ? ? ? ? 48 81 C4 ? ? ? ? C3 CC CC CC CC CC CC CC CC CC CC CC CC CC CC 48 89 54 24 ? 48 89 4C 24 ? 48 81 EC ? ? ? ? 48 8B 84 24 ? ? ? ? 0F B6 80 [2nd xRef]
		inline constexpr auto oCEntityComponentMinable_OnHitByMiningLaser = 0x3C4AF60;		// "CEntityComponentMineable::OnHitByMiningLaser" : E8 ? ? ? ? 0F B6 C0 85 C0 75 ? 48 8B 8C 24 ? ? ? ? E8 ? ? ? ? 48 83 C0 [this]
		inline constexpr auto oFractureMineable = 0x3C8D6E0;								//	44 89 44 24 ? 48 89 54 24 ? 48 89 4C 24 ? 48 81 EC ? ? ? ? 41 B0 [this]

		/* damage events */
		inline constexpr auto oCGameRulesSCDamageHandling_OnHit = 0x4FD4BA0; //	"CGameRulesSCDamageHandling::OnHit"
		inline constexpr auto oAddToHitbox = 0x37778E0;				// "Adding hit to outbox hit map -> HitId: %hu, Source: %s"

		/**/
		inline constexpr auto oGetSystem = 0x02C90E0;//	"Camera.PlayerInventoryCamera" -> else { v15 = x__GetSystem(); ... }
		inline constexpr auto oGetStructInstance = 0x0426C40;	// "SGlobalSalvageRepairBeamParams.SGlobalSalvageRepairBeamParams" -> return *(_QWORD *)(GetStructInstance(v2, v3, "SGlobalSalvageRepairBeamParams.SGlobalSalvageRepairBeamParams") + 16);
	
		/**/
		inline constexpr auto oApplyHealthChange = 0x61CD1C0;				//	"CSCBodyHealthComponent::ApplyHealthChange"
		inline constexpr auto oApplyDamageHealing = 0x61CC520;				//	"CSCBodyHealthComponent::ApplyHealthChange"
		inline constexpr auto oAuthorityRequestHit = 0x62665A0;				//	"CSCBodyHealthComponent::m_rmAuthorityRequestHite"

		inline constexpr auto oActorKill = 0x65454B0;				//	"CActor::Kill"

		inline constexpr auto oShipBoostMP = 0x3FF72F0;				//	E8 ? ? ? ? C5 FA 10 4C 24 ? C5 F8 2F C1 73 [call]
	}

	namespace Functions
	{
		inline auto EAC_HandleDiscipline_stub = reinterpret_cast<void(*)(void*)>(GetAddr(Offsets::oEAC_HandleDiscipline));

		inline auto CSystem_Update_stub = reinterpret_cast<__int64(*)(void*, void*, void*)>(GetAddr(Offsets::oCSystem_Update));

		inline auto CXConsole_GetCVar_stub = reinterpret_cast<__int64(*)(__int64, const char*)>(GetAddr(Offsets::oCXConsole_GetCVar));
		inline auto CXConsole_RegisterCvar_Int_stub = reinterpret_cast<__int64* (*)(__int64, const char*, unsigned long*, int, unsigned int, const char*, __int64)>(GetAddr(Offsets::oCXConsole_RegisterCvar_Int));
		inline auto CXConsole_RegisterCvar_Float_stub = reinterpret_cast<__int64* (*)(__int64, const char*, unsigned long*, float, unsigned int, const char*, __int64)>(GetAddr(Offsets::oCXConsole_RegisterCvar_Float));
		inline auto CXConsole_RegisterCvar_Int64_stub = reinterpret_cast<__int64* (*)(__int64, const char*, unsigned long*, __int64, unsigned int, const char*, __int64)>(GetAddr(Offsets::oCXConsole_RegisterCvar_Int64));
		inline auto CXConsole_RegisterCvar_String_stub = reinterpret_cast<__int64* (*)(__int64, const char*, unsigned long*, void*, unsigned int, const char*, __int64)>(GetAddr(Offsets::oCXConsole_RegisterCvar_String));
		inline auto CXConsole_AddCommand_stub = reinterpret_cast<__int64(*)(__int64, const char*, __int64, unsigned int, const char*, char)>(GetAddr(Offsets::oCXConsole_AddCommand));

		inline auto CDataCore_RegisterStruct_stub = reinterpret_cast<__int64(*)(void*, const char*, void*, void*, void*, void*, char, void*)>(GetAddr(Offsets::oCDataCore_RegisterStruct));
		
		inline auto CRenderer_MTUpdate_stub = reinterpret_cast<__int64(*)(__int64, __int64)>(GetAddr(Offsets::oCRenderer_MTUpdate));
		inline auto C3DEngine_RenderWorld_stub = reinterpret_cast<__int64(*)(__int64, unsigned __int8*, __int64)>(GetAddr(Offsets::oC3DEngine_RenderWorld));
		inline auto CRenderer_ProjectToScreen_stub = reinterpret_cast<__int64(*)(void*, double, double, double, float*, float*, float*, int, __int64)>(GetAddr(Offsets::oCRenderer_ProjectToScreen));
		inline auto CRenderer_DrawText_stub = reinterpret_cast<__int64(*)(void*, void*, void*, __int64, const char*)>(GetAddr(Offsets::oCRenderer_DrawText));
		inline auto CRenderer_DrawTextArgs_stub = reinterpret_cast<__int64(*)(void*, void*, void*, __int64, const char*, va_list)>(GetAddr(Offsets::oCRenderer_DrawTextArgs));
		inline auto CRenderer_DrawTextArgs2_stub = reinterpret_cast<__int64(*)(void*, void*, void*, const char*, va_list)>(GetAddr(Offsets::oCRenderer_DrawTextArgs2));
		inline auto CRenderer_FlushTextMessages_stub = reinterpret_cast<__int64(*)(void*)>(GetAddr(Offsets::oCRenderer_FlushTextMessages));
		
		inline auto CCamerViewManager_Update_stub = reinterpret_cast<__int64(*)(__int64, __int64, double)>(GetAddr(Offsets::oCCamerViewManager_Update));

		inline auto CRenderProxy_GetLocalBounds_stub = reinterpret_cast<__int64(*)(void*, void*)>(GetAddr(Offsets::oCRenderProxy_GetLocalBounds));
		inline auto CActor_GetBoneTransform_stub = reinterpret_cast<__int64(*)(__int64, void*, unsigned int)>(GetAddr(Offsets::oCActor_GetBoneTransform));
		inline auto CSCLocalPlayerMovementSpeed_stub = reinterpret_cast<float(*)(__int64)>(GetAddr(Offsets::oCSCLocalPlayerMovement_Update));

		inline auto CEntity_Init_stub = reinterpret_cast<__int64(*)(__int64, __int64)>(GetAddr(Offsets::oCEntity_Init));
		inline auto CEntity_Shutdown_stub = reinterpret_cast<__int64(*)(__int64)>(GetAddr(Offsets::oCEntity_Shutdown));

		inline auto CEntitySystem_Update_stub = reinterpret_cast<__int64(*)(__int64)>(GetAddr(Offsets::oCEntitySystem_Update));
		inline auto CEntitySystem_SpawnEntity_stub = reinterpret_cast<__int64* (*)(__int64, __int64*, __int64*, __int64)>(GetAddr(Offsets::oCEntitySystem_SpawnEntity));
		inline auto CEntitySystem_DeleteEntity_stub = reinterpret_cast<__int64(*)(__int64, __int64*)>(GetAddr(Offsets::oCEntitySystem_DeleteEntity));

		inline auto CEntityClassRegistry_RegisterClass_stub = reinterpret_cast<bool(*)(__int64, __int64*)>(GetAddr(Offsets::oCEntityClassRegistry_RegisterClass));
		inline auto CEntityClassRegistry_FindClass_stub = reinterpret_cast<__int64(*)(__int64, const char*)>(GetAddr(Offsets::oCEntityClassRegistry_FindClass));

		inline auto CRigidEntity_VerifyExistingContacts_stub = reinterpret_cast<__int64(*)(__int64)>(GetAddr(Offsets::oCRigidEntity_VerifyExistingContacts));

		inline auto GetIActor_stub = reinterpret_cast<__int64(*)(__int64*)>(GetAddr(Offsets::oGetIActor));
		inline auto CCharacterStateHiearchy_SetState_stub = reinterpret_cast<__int64(*)(__int64*, unsigned int)>(GetAddr(Offsets::oCCharacterStateHiearchy_SetState));
		inline auto CCharacterStateHiearchy_VerifyState_stub = reinterpret_cast<bool(*)(__int64*, __int64*, __int64, double, char, char)>(GetAddr(Offsets::oCCharacterStateHiearchy_VerifyState));

		inline auto CSCAmmoContainerComponent_GetAmmoCount_stub = reinterpret_cast<__int64(*)(__int64)>(GetAddr(Offsets::oCSCAmmoContainerComponent_GetAmmoCount));
		inline auto CWeaponActionFireSingle_Update_stub = reinterpret_cast<char(*)(__int64, double, __int64)>(GetAddr(Offsets::oCWeaponActionFireSingle_Update));
		inline auto CWeaponActionFireBurst_Update_stub = reinterpret_cast<char(*)(__int64, __int64, __int64, __int64)>(GetAddr(Offsets::oCWeaponActionFireBurst_Update));
		inline auto CWeaponActionFireRapid_Update_stub = reinterpret_cast<char(*)(__int64, double, __int64, __int64)>(GetAddr(Offsets::oCWeaponActionFireRapid_Update));
		inline auto CWeaponActionFireTractorBeam_GetRayCastRequest_stub = reinterpret_cast<char(*)(__int64, __int64)>(GetAddr(Offsets::oCWeaponActionFireTractorBeam_GetRayCastRequest));
		inline auto CWeaponActionFireHealingBeam_GetRayCastRequest_stub = reinterpret_cast<char(*)(__int64, __int64, double)>(GetAddr(Offsets::oCWeaponActionFireHealingBeam_GetRayCastRequest));
		inline auto CWeaponActionFireSalvageRepair_GetRayCastRequest_stub = reinterpret_cast<char(*)(__int64, __int64, double)>(GetAddr(Offsets::oCWeaponActionFireSalvageRepair_GetRayCastRequest));

		/* salvage controller */
		inline auto CSCItemSalvageController_OnRaycastSubmit_stub = reinterpret_cast<bool(*)(__int64, __int64, __int64)>(GetAddr(Offsets::oCSCItemSalvageController_OnRaycastSubmit_stub));

		inline auto CInventoryComponent_AddItem_stub = reinterpret_cast<bool(*)(__int64, __int64, int, int, int, int)>(GetAddr(Offsets::oCInventoryComponent_AddItem));
		inline auto CSCLocalPlayerThoughComponent_OnInventoryMoveItemOntoUnoccupiedPosition_stub = reinterpret_cast<__int64(*)(__int64, const void*, const void*, unsigned int, unsigned int, int)>(GetAddr(Offsets::oCSCLocalPlayerPersonalThoughtComponent_OnInventoryMoveItemOntoUnoccupiedPosition));

		/* mining controller */
		inline auto CSCItemMiningController_UpdateLaserThrottle_stub = reinterpret_cast<__int64(*)(__int64, double, char, unsigned __int8, char)>(GetAddr(Offsets::oCSCItemMiningController_UpdateLaserThrottle));
		inline auto CEntityComponentMineable_OnHitByMiningLaser_stub = reinterpret_cast<__int64(*)(__int64, __int64, double, double, __int64, __int64)>(GetAddr(Offsets::oCEntityComponentMinable_OnHitByMiningLaser));
		inline auto FractureMineable_stub = reinterpret_cast<__int64(*)(__int64, __int64, __int64)>(GetAddr(Offsets::oFractureMineable));

		//	
		inline auto CSCActorGForce_Update_stub = reinterpret_cast<__int64(*)(__int64, __int64)>(GetAddr(Offsets::oCSCActorGForce_Update));
		inline auto CSCItemQuantumDrive_Update_stub = reinterpret_cast<__int64(*)(__int64, __int64)>(GetAddr(Offsets::oCSCItemQuantumDrive_Update));

		inline auto CGoToPointManager_GetPointByName_stub = reinterpret_cast<__int64(*)(__int64, const char*)>(GetAddr(Offsets::oCGoToPointManager_GetGoToPointByName));
		inline auto CEntitySystem_GetEntityZoneByID_stub = reinterpret_cast<__int64(*)(__int64, __int64, __int64)>(GetAddr(Offsets::oCEntitySystem_GetEntityZoneByID));
	
		
		inline auto CXCommand_MegaMap_stub = reinterpret_cast<void(*)(__int64)>(GetAddr(Offsets::oCXCommand_MegaMap));
		inline auto CXCommand_LoadMegaMap_stub = reinterpret_cast<void(*)(__int64, const char*, const char*)>(GetAddr(Offsets::oCXCommand_LoadMegaMap));
		inline auto LoadMap_stub = reinterpret_cast<__int64(*)(double, __int64)>(GetAddr(0x24EB790)); //	"LoadMap" is not exported, so we have to use the address directly. It is called in a sneaky manner
		inline auto AddToHitBox_stub = reinterpret_cast<__int64(*)(void*, __int64, void*, double, __int8, __int8, void*, __int16)>(GetAddr(Offsets::oAddToHitbox));

		inline auto CGameRulesSCDamageHandling_OnHit_stub = reinterpret_cast<__int64(*)(__int64, __int64*)>(GetAddr(Offsets::oCGameRulesSCDamageHandling_OnHit));

		inline auto GetRenderProxy_stub = reinterpret_cast<unsigned __int64* (*)(unsigned __int64*, unsigned __int64*)>(GetAddr(Offsets::oGetRenderProxy));
		inline auto IsValidEntity_stub = reinterpret_cast<bool(*)(unsigned __int64*)>(GetAddr(Offsets::oIsValidEntity));
		inline auto IsValidRenderProxy_stub = reinterpret_cast<bool(*)(unsigned __int64*)>(GetAddr(Offsets::oIsValidRenderProxy));

		/**/
		inline auto GetSystem_stub = reinterpret_cast<__int64(*)()>(GetAddr(Offsets::oGetSystem));	
		inline auto GetStructInstance_stub = reinterpret_cast<__int64(*)(__int64, __int64, __int64)>(GetAddr(Offsets::oGetStructInstance));

		/**/
		inline auto CSCBodyHealthComponent_AuthorityRequestHit = reinterpret_cast<__int64(*)(__int64, __int64, float)>(GetAddr(Offsets::oAuthorityRequestHit));
		inline auto CSCBodyHealthComponent_ApplyHealthChange = reinterpret_cast<__int64(*)(unsigned __int64, float, unsigned __int64, unsigned __int16)>(GetAddr(Offsets::oApplyHealthChange));
		inline auto CSCBodyHealthComponent_ApplyDamageHealing = reinterpret_cast<__int64(*)(unsigned __int64, float, unsigned __int64, unsigned __int16)>(GetAddr(Offsets::oApplyDamageHealing));


		inline auto CActor_Kill_stub = reinterpret_cast<__int64(*)(__int64*, __int64, __int64)>(GetAddr(Offsets::oActorKill));

		inline auto ShipBoostMP_stub = reinterpret_cast<float(*)(__int64, unsigned __int8)>(GetAddr(Offsets::oShipBoostMP));
	}

	namespace Hooks
	{
		namespace vars
		{
			/* core */
			inline bool bFreeModule = false;									//	free module handle
			static FILE* console_output_stream = nullptr;
			static HANDLE console_handle = nullptr;
			static HWND console_wndw = nullptr;
			inline bool console_bShow = false;									//	
			inline WNDPROC origWndProc = nullptr;								//	original window procedure
			inline HWND pGameWndw = nullptr;									//	game window handle	
			inline bool bWndwFocus = true;										//	window focus flag ; true = game window is focused
			inline bool bInitGui = false;										//	overlay window initialization flag
			inline bool bGuiActive = false;										//	overlay window active flag
			inline Structs::FVector2D szCanvas{ 0.0f, 0.0f };					//	canvas size
			inline float gui_FOV = 0.5f;										//	gui field of view

			/* entity array caches */
			inline std::mutex vEntitiesMutex;									//	entity array mutex	for spawning and deleting entities. Otherwise the game will access the std::vector from both functions at the same time.
			inline std::vector<Classes::CEntity*> vPlayerEntities;				//	CEntitySystem::SpawnEntity | CEntitySystem::DeleteEntity	-> our own entity array
			inline std::vector<Classes::CEntity*> vEnemyAIEntities;				//	CEntitySystem::SpawnEntity | CEntitySystem::DeleteEntity	-> our own entity array
			inline std::vector<Classes::CEntity*> vAnimalEntities;				//	CEntitySystem::SpawnEntity | CEntitySystem::DeleteEntity	-> our own entity array
			inline std::vector<Classes::CEntity*> vShipEntities;				//	CEntitySystem::SpawnEntity | CEntitySystem::DeleteEntity	-> our own entity array
			inline std::vector<Classes::CEntity*> vMissionEntities;				//	CEntitySystem::SpawnEntity | CEntitySystem::DeleteEntity	-> our own entity array
			inline std::vector<Classes::CEntity*> vLootEntities;				//	CEntitySystem::SpawnEntity | CEntitySystem::DeleteEntity	-> our own entity array
			inline std::vector<Classes::CEntity*> vRockEntities;				//	CEntitySystem::SpawnEntity | CEntitySystem::DeleteEntity	-> our own entity array
			inline std::vector<Classes::CEntity*> vKioskEntities;				//	CEntitySystem::SpawnEntity | CEntitySystem::DeleteEntity	-> our own entity array
			inline std::vector<Classes::CEntity*> vGoToEntities;				//	CEntitySystem::SpawnEntity | CEntitySystem::DeleteEntity	-> our own entity array
			inline std::vector<Classes::CEntity*> vOrbitEntities;				//	CEntitySystem::SpawnEntity | CEntitySystem::DeleteEntity	-> our own entity array
			inline std::vector<Classes::CEntity*> vAllEntities;					//	CEntitySystem::SpawnEntity | CEntitySystem::DeleteEntity	-> our own entity array

			/* screen points */
			inline std::vector<Structs::SThreadEntity> vScreenEntities;			//	array of entity screen points
			inline std::vector<std::pair<Classes::CEntity* , std::vector<Structs::FVector2D>>> vScreenOrbit;		//	array of orbit screen points

			/* dumps */
			inline std::vector<Classes::CEntityClass*> vEntityClasses;			//	CEntityClassRegistry::RegisterEntityClass -> our own entity class registry array
			inline std::vector<Structs::SXCvar> vConsoleVariables;				//	CXConsole::Register(x)
			inline std::vector<Structs::SXCommand> vConsoleCommands;			//	CXConsole::AddCommand
			inline std::vector<std::string> vStructNames;						//	CDataCore::RegisterStruct
			inline std::vector<Classes::CGoToPoint*> vGoToPoints;				//  array of all found go to points
			inline std::vector<Structs::SWaypoint> vWaypoints;					//  array of all custom player waypoints
			inline Structs::SDataCache cache_ClassNames;						//	entity class cache

			/* thread ctx control flags */
			inline bool bIsCached{ false };										//	is entity class cache valid

			/* cheat vars */
			inline bool cheat_bUFO = false;										//	ufo toggle state
			inline bool cheat_bNoFog = false;									//  no fog toggle state
			inline bool cheat_bDisableArmistice = false;						//	disable armistice toggle state
			inline bool cheat_bTunedMiningFractureBeam = false;					//	toggle for adjusting mining fracture beam properties
			inline bool cheat_bTunedTractorBeam = false;						//  toggle for adjusting tractor beam properties
			inline bool cheat_bTunedSalvageBeam = false;						//  toggle for adjusting salvage beam properties
			inline bool cheat_bTunedMedicalBeam = false;						//  toggle for adjusting healing beam properties 
			inline bool cheat_bEvilMedicalBeam = false;							//  toggle for adjusting healing beam properties 
			inline bool cheat_bCustomWalkSpeed = false;							//  player movement speed toggle state
			inline bool cheat_bFastShields = false;								//  instant shield recharge toggle state
			inline bool cheat_bSuperThrust = false;								//  ship thrust toggle state
			inline bool cheat_bSuperSpeed = false;								//  max ship speed toggle state
			inline bool cheat_bNoGForce = false;								//  disable gforce toggle state
			inline bool cheat_bDisableGlare = false;							//  disable glare toggle state
			inline bool cheat_bShowExtendedStatusHUD = false;					//  show extended status hud toggle state
			inline bool cheat_bInstantWarp = false;								//  insta qt toggle state
			inline bool cheat_bInfiniteFuelHG = false;							//  inf hydrogen fuel toggle state
			inline bool cheat_bInfiniteFuelQT = false;							//  inf quantum fuel toggle state
			inline bool cheat_bDisableAtmostphericResistance = false;			//  no atmo resistance toggle state
			inline bool cheat_bMaxInventoryStorage = false;						//  max inventory storage toggle state ; sets storage containers to the same size as local storage
			inline bool cheat_bInfiniteAmmo = false;							//	infinite ammo toggle state	
			inline bool cheat_bNoSpread = false;								//	no spread toggle state
			inline bool cheat_bNoRecoil = false;								//	no recoil toggle state
			inline bool cheat_bDamageMultiplier = false;						//	damage multiplier toggle state	
			inline bool cheat_bSpaceBrake = false;								//  space brake toggle state	
			inline bool cheat_bEasyJumpGate = false;							//	toggle for easy mode gate jump
			inline bool cheat_bDisableAllShakes = false;						//	disable all shakes toggle state
			inline bool cheat_bDisableLootRestrictions = false;					//	disable loot restrictions toggle state
			inline bool cheat_bBlameUser = false;								//	blame user toggle state
			inline bool cheat_bMirrorForce = false;								//	defends the player from incoming hits
			inline bool cheat_bDemiGod = false;									//	demi god toggle state
			inline bool cheat_bHealSelf = false;								//	heal local player toggle state
			inline bool cheat_bAutoHeal = false;								//	auto heal local player toggle state
			inline bool cheat_bKillSelf = false;								//	kill local player toggle state
			inline bool cheat_bDisablePlayAreaAC{ false };						//	disable play area restriction for arena commander
			inline bool cheat_bDisableTimeLimitAC{ false };						//	disable time limit for arena commander
			inline bool cheat_bDisableATCLandingRestrictions{ false };			//	disable ATC landing restrictions in the PU ( steal other ships )
			inline bool cheat_rage_bGrabAllEnemies{ false };	
			inline bool cheat_bShipBoostMP{ false };							//	ship boost toggle state
			inline float mShipSpeed = 1000.0f;									//  max ship speed scalar			
			inline float mShipThrust = 1.0f;									//  ship thrust scalar
			inline float mUFOSpeedScalar = 1.f;									//	ufo mode speed scalar
			inline float mPlayerWalkSpeed = 5.0f;								//  player movement speed scalar
			inline float mDamageMPScalar = 1.f;									//	damage multiplier value
			inline float cheat_mining_FractureThrottle = 0.01f;					//	custom fracture throttle
			inline float cheat_mining_FractureThrottleLerp = 100.f;				//	mining fractur throttle lerp
			inline bool cheat_mining_bAutoFracture{ false };
			inline bool cheat_mining_mFlag{ false };
			inline __int64 cheat_mining_pMineable{ 0 };


			/* grab entity */
			inline bool grab_bEnable{ false };									//  state for the feature "grab entity"
			inline bool grab_bGrab{ false };									//  
			inline bool grab_bGoTo{ false };									//  
			inline float grab_fDistance{ 5.0f };								//  distance for grabbing an entity
			inline Classes::CEntity* grab_pEntity{ nullptr };					//  entity to grab

			/* ESP properties */
			inline bool esp_bDebug = false;
			inline bool esp_bPlayer = false;									//	esp players toggle state
			inline bool esp_bEnemyAI = false;									//	esp enemy ai toggle state
			inline bool esp_bAnimals = false;									//	esp animals toggle state
			inline bool esp_bShip = false;										//	esp ship toggle state
			inline bool esp_bRock = false;										//	esp toggle state
			inline bool esp_bLoot = false;										//	esp toggle state
			inline bool esp_bOrbit = false;										//	esp toggle state
			inline float esp_DebugRange = 5.f;									//	esp range
			inline float esp_PlayerRange = 10.f;								//	esp range
			inline float esp_EnemyAIRange = 10.f;								//	esp range
			inline float esp_AnimalRange = 10.f;								//	esp range
			inline float esp_ShipRange = 10.f;									//	esp range
			inline float esp_LootRange = 10.f;									//	esp range
			inline float esp_RockRange = 10.f;									//	esp range
			inline float esp_OrbitRange = 10.f;									//	esp range
			inline Structs::FQuat esp_DebugColor = { 1.f, 1.f, 1.f, 1.f };	
			inline Structs::FQuat esp_PlayerColor = { 0.079f, 0.895f, 0.619f, 1.000f };		//	esp color { white }
			inline Structs::FQuat esp_EnemyAIColor = { 0.948f, 0.168f, 0.168f, 1.000f };	//	esp color { red }
			inline Structs::FQuat esp_AnimalColor = { 0.513f, 0.203f, 0.921f, 1.f };		//	esp color { purple }
			inline Structs::FQuat esp_ShipColor = { 0.894f, 0.355f, 0.078f, 1.000f };		//	esp color { purple }
			inline Structs::FQuat esp_RockColor = { 0.881f, 0.894f, 0.078f, 1.000f };		//	esp color { yellow }
			inline Structs::FQuat esp_LootColor = { 0.078f, 0.302f, 0.894f, 1.000f };		//	esp color { blue }
			inline Structs::FQuat esp_OrbitColor = { 0.078f, 0.894f, 0.874f, 1.000f };		//	esp color { cyan }

			/* mining properties */

			/* salvage properties */
			inline bool salvage_bInstantFillRate = false;						//	instant salvage toggle state
			inline float salvage_minSCUContainer{ 1.0f };						//	
			inline float salvage_maxSCUContainer{ 16.0f };						//	


			/* hit result */
			inline Structs::SHitResult hit_lastHit;
			inline Structs::SHitResult hit_lastHitByPlayer;

			/* target entity */
			inline bool target_bSetNewPlayer = false;							//  flag for setting a new target entity
			static Classes::CEntity* target_pSelection = nullptr;				//  target entity for storing structure 
			inline Structs::STargetEntity target_player;						//  target player struct
			inline Structs::SForgeControls target_controls;						//  target forge controls
			inline Structs::FQuat target_Color{ 1.f, 1.f, 1.f, 1.f };			//	target entity Name Color { white }

			/* loadout history */
			inline std::vector<int> loadout_history;							//	loadout history array

			/* waypoints */
			inline Structs::FQuat wp_ColorTags = { 0.921f, 0.717f, 0.203f, 1.f };		//	esp color { yellow }
			inline bool wp_bRenderTags = true;									//  render waypoints toggle state
			inline std::string wp_TargetName;									//  waypoint entry name
			inline bool wp_bSetNewPoint;										//	flag for setting a new waypoint

			/* command executor */
			inline int cmd_count = 1;											//  command count	
			inline bool cmd_bExec = false;										//  executes a command
			inline char* cmd_args[6] = { 0, 0, 0, 0, 0, 0 };					//  command array
			static StarCitizen::Structs::SXCommand cmd_selection;				//  selected command

			/* [DEV] fly mode */
			inline bool fly_bEnable{ false };									//  modules toggle state for the feature "dev fly mode"
			inline bool fly_bSet{ false };										//  used for setting the fly mode state within the star citizen process , this calls a function that should only be fired when setting the fly mode flag needs to take place.
			inline int fly_iState = 0;											//  "" , this is the flag for fly mode that will get verified by the process
			inline int* fly_pFlag = nullptr;									//  pointer to the true fly mode flag

			/* [DEV] goto points */
			inline bool goto_bFindPoint{ false };								//  find go to point toggle state
			inline std::string goto_TargetName;									//  target name for the find go to point feature
			
			/* class names */
			inline int class_names_count = 0;									//	class names count
			inline const int class_names_max_count = 0x8192;					//	max class names count
			inline const char* class_names[class_names_max_count];				//	class names array

			/* entity transforms */
			inline Structs::SLocalPlayer sLocalPlayer;							//	local player info
			inline Structs::STransforms sLocalPlayerTM;							//	local player transforms	
			//	inline Structs::STargetEntity sTargetEntity;					//	target entity info
			inline std::vector<std::vector<Enums::EBones>> BoneVector =
			{
				{ Enums::EBones::l_hand, Enums::EBones::left_elbow,	Enums::EBones::l_shoulder,		Enums::EBones::spine_03 },							// Left hand -> neck
				{ Enums::EBones::r_hand, Enums::EBones::r_elbow,	Enums::EBones::r_shoulder,		Enums::EBones::spine_03 },							// Right hand -> neck
				{ Enums::EBones::l_foot, Enums::EBones::l_knee,		Enums::EBones::l_hip,			Enums::EBones::pelvis,		Enums::EBones::spine_01 },     // Left foot -> bottom spine
				{ Enums::EBones::r_foot, Enums::EBones::r_knee,		Enums::EBones::r_hip,			Enums::EBones::pelvis,		Enums::EBones::spine_01 },		// Right foot -> bottom spine
				{ Enums::EBones::pelvis, Enums::EBones::spine_01,	Enums::EBones::spine_03,		Enums::EBones::head }								// Bottom spine -> head
			};

			inline std::string datacore_TargetName;								//	input string to find
			inline bool datacore_bFindInstance{ false };						//	flag for finding instance
		}

		LRESULT	CALLBACK WndProc_hook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		__int64 __fastcall EAC_HandleDiscipline_hook(void* a1);
		__int64 __fastcall CSystem_Update_hook(void* a1, void* a2, void* a3);																							// 
		__int64 __fastcall C3DEngine_RenderWorld_hook(__int64 a1, unsigned __int8* a2, __int64 a3);																		// 
		__int64 __fastcall CRenderer_MTUpdate_hook(__int64 a1, __int64 a2);																								// 
		__int64 __fastcall CRenderer_DrawTextArgs_hook(void*, void*, void*, __int64, const char*, va_list);																// 
		bool __fastcall CRenderer_ProjectToScreen_hook(void* a1, double x, double y, double z, float* pX, float* pY, float* pZ, char a8, __int64 a9);						//
		__int64 __fastcall CRenderer_FlushTextMessages_hook(void* a1);																										//
		__int64 __fastcall CCameraViewManager_Update_hook(__int64 a1, __int64 a2, double a3);																			//
		__int64 __fastcall CEntitySystem_Update_hook(__int64 a1);
		__int64* __fastcall CEntitySystem_SpawnEntity_hook(__int64 a1, __int64* a2, __int64* a3, __int64 a4);															// 
		__int64 __fastcall CEntitySystem_DeleteEntity_hook(__int64 a1, __int64* a2);																					// 
		__int64 CEntity_Init_hook(__int64 a1, __int64 a2);																										//
		__int64 CEntity_Shutdown_hook(__int64 a1);																												//
		__int64* __fastcall CXConsole_RegisterIntCvars_hook(__int64 a1, const char* a2, unsigned long* a3, int a4, unsigned int a5, const char* a6, __int64 a7);		// dump console variables
		__int64* __fastcall CXConsole_RegisterFloatCvars_hook(__int64 a1, const char* a2, unsigned long* a3, float a4, unsigned int a5, const char* a6, __int64 a7);	// dump console variables
		__int64* __fastcall CXConsole_RegisterInt64Cvars_hook(__int64 a1, const char* a2, unsigned long* a3, __int64 a4, unsigned int a5, const char* a6, __int64 a7);	// dump console variables
		__int64* __fastcall CXConsole_RegisterStringCvars_hook(__int64 a1, const char* a2, unsigned long* a3, void* a4, unsigned int a5, const char* a6, __int64 a7);	// dump console variables
		__int64 __fastcall CXConsole_AddCMD_hook(__int64 a1, const char* a2, __int64 a3, unsigned int a4, const char* a5, char a6);										// dumps console commands
		__int64 __fastcall CDataCore_RegisterStruct_hook(void* a1, const char* a2, void* a3, void* a4, void* a5, void* a6, char a7, void* a8);							// dump structs
		bool __fastcall CEntityClassRegistry_RegisterClass_hook(__int64 a1, __int64* a2);																				// dump structs
		__int64 __fastcall CRigidEntity_VerifyExistingContacts_hook(__int64 a1);																						//
		bool __fastcall CCharacterStateHiearchy_VerifyState_hook(__int64* a1, __int64* a2, __int64 a3, double _XMM3_8, char a5, char a6);								// dev no clip
		__int64 __fastcall CSCAmmoContainerComponent_GetAmmoCount_hook(__int64 a1);																						// set ammo count
		char __fastcall CWeaponActionFireSingle_Update_hook(__int64 a1, double _XMM1_8, __int64 a3);
		char __fastcall CWeaponActionFireBurst_Update_hook(__int64 a1, __int64 a2, __int64 a3, __int64 a4);
		char __fastcall CWeaponActionFireRapid_Update_hook(__int64 a1, double _XMM1_8, __int64 a3, __int64 a4);
		char __fastcall CWeaponActionFireTractorBeam_GetRayCastRequest_hook(__int64 a1, __int64 a2);																	//	tractor beam params
		char __fastcall CWeaponActionFireHealingBeam_GetRayCastRequest_hook(__int64 a1, __int64 a2, double _XMM2_8);													//	healing beam params
		char __fastcall CWeaponActionFireSalvageRepair_GetRayCastRequest_hook(__int64 a1, __int64 a2, double _XMM2_8);													//	salvage repair params	
		char __fastcall CInventoryComponent_AddItem(__int64 _RCX, __int64 _RDX, int szContainer, int a4, int a5, int a6);												//	determines if an item is added to the inventory
		__int64 __fastcall CSCLocalPlayerThoughComponent_OnInventoryMoveItemOntoUnoccupiedPosition_hook(__int64 a1, const void* a2, const void* a3, unsigned int a4, unsigned int a5, int a6);	//	
		__int64 __fastcall CSCActorGForce_Update_hook(__int64 a1, __int64 a2);																										//
		__int64 __fastcall CSCItemQuantumDrive_Update_hook(__int64 a1, __int64 a2);																						//	
		bool __fastcall CSCItemSalvageController_OnRaycastSubmit_hook(__int64 a1, __int64 a2, __int64 a3);															//
		__int64 __fastcall CSCItemMiningController_UpdateLaserThrottle_hook(__int64, double, char, unsigned __int8, char);
		__int64 __fastcall CEntityComponentMineable_OnHitByMiningLaser_hook(__int64 a1, __int64 a2, double _XMM2_8, double _XMM3_8, __int64 a5, __int64 a6);
		__int64 __fastcall FractureMineable_hook(__int64 a1, __int64 a2, __int64 a3);	//
		float __fastcall CSCLocalPlayerMovementSpeed_stub_hook(__int64 a1);																								//	
		void __fastcall CXCommand_MegaMap_hook(__int64 a1);																												//
		void __fastcall CXCommand_LoadMegaMap_hook(__int64 a1, const char* a2, const char* a3);																			//
		__int64 __fastcall LoadMap_hook(double _XMM0_8, __int64 a2);
		__int64 __fastcall AddToHitBox_hook(void* a1, __int64 a2, void* a3, double xmm1, __int8 a5, __int8 a6, void* a7, __int64 a8);									//
		void __fastcall CGameRulesSCDamageHandling_OnHit_hook(__int64 a1, __int64* a2);																					//
		__int64 __fastcall CSCBodyHealthComponent_ApplyHealthChange_hook(unsigned __int64 a1, float a2, unsigned __int64 a3, unsigned __int16 a4);						//
		__int64 __fastcall CSCBodyHealthComponent_ApplyDamageHealing_hook(unsigned __int64 a1, float a2, unsigned __int64 a3, unsigned __int16 a4);						//
		__int64 __fastcall CActor_Kill_hook(__int64* a1, __int64 a2, __int64 a3);																						//
		float __fastcall ShipBoostMP_hook(__int64 a1, unsigned __int8 a2);																								//
	}

	namespace Helpers
	{
		/**/
		bool ShowConsole(const bool& state) noexcept;
		bool IsValidPtr(void* p);	//	checks if the pointer being passed is valid or not
		bool GamePadGetKeyState(WORD vButton) noexcept;	//	gets the gamepad key state
		bool IsESPEnabled() noexcept;	//	checks if the ESP is enabled

		
		/* entity helpers */
		bool IsLocalPlayerSitting() noexcept;	//	Checks if the local player is seated
		Classes::CEntity* GetLocalPlayerEntity() noexcept;	//	Gets local player entity
		Classes::CEntity* GetLocalShipEntity() noexcept;	//	Gets local player entity ( if available )
		Classes::CEntity* GetLocalMechaEntity() noexcept;	//	Gets local player mech suit entity ( if available )
		Classes::CEntity* GetLocalControlledEntity() noexcept;	//	Gets local player controlled entity ( if available )

		/* commands */
		bool GetCVar(const std::string& name, Classes::SXCvar* result) noexcept;	//	Gets console variable by name
		bool GetCmd(const std::string& name, Classes::SXCommand* result) noexcept;	//	Gets console command by name
		bool GetPointByName(const char* pName, StarCitizen::Structs::DVector* pOut) noexcept;	//	Gets a point by name
		StarCitizen::Classes::SGoToPointAZ* GetGoToPointsArray() noexcept;	//	Gets the goto points array
		bool GetEntityClassByName(const char* pName, __int64* pOut) noexcept;	//	Gets entity class by name

		/* entity type helpers */
		bool IsValidEntity(Classes::CEntity* pEntity) noexcept;	//	Checks if the entity is valid
		bool IsPlayerEntity(Classes::CEntity* pEntity) noexcept;	//	Checks if the entity is a player
		bool IsEnemyNPCEntity(Classes::CEntity* pEntity) noexcept;		//	Checks if the entity is an Enemy NPC
		bool IsNPCEntity(Classes::CEntity* pEntity) noexcept;		//	Checks if the entity is an NPC
		bool IsAnimalEntity(Classes::CEntity* pEntity) noexcept;		//	Checks if the entity is an animal
		bool IsShipEntity(Classes::CEntity* pEntity) noexcept;		//	Checks if the entity is a ship
		bool IsMineableEntity(Classes::CEntity* pEntity) noexcept;	//	Checks if the entity is mineable
		bool IsLootableEntity(Classes::CEntity* pEntity) noexcept;	//	Checks if the entity is lootable
		bool IsMissionEntity(Classes::CEntity* pEntity) noexcept;	//	Checks if the entity is a mission entity
		bool IsOrbitEntity(Classes::CEntity* pEntity) noexcept;		//	Checks if the entity is a mission entity
		bool IsGoToEntity(Classes::CEntity* pEntity) noexcept;		//	Checks if the entity is a go to entity
		bool IsShoppingKioskEntity(Classes::CEntity* pEntity) noexcept;	//	Checks if the entity is a kiosk

		/* entity helpers */
		Classes::CEntity* GetEntityShip(Classes::CEntity* pTarget);														//	Gets the entity ship
		bool GetEntityBounds(Classes::CEntity* pTarget, Structs::STransforms* out); 									//	Gets the entity bounds
		bool GetEntityScreenPoints(Classes::CEntity* pTarget, Structs::STransforms& TM, Structs::SDrawEntity* out2D);	//	Gets the entity screen points
		bool GetEntityBoneLocalPosByID(Classes::CEntity* pEntity, const StarCitizen::Enums::EBones& ID, Structs::FVector* out);
		bool GetEntityBoneWorldPosByID(Classes::CEntity* pEntity, const StarCitizen::Enums::EBones& ID, Structs::DVector* out);
		float GetEntityHealth(Classes::CEntity* pTarget) noexcept;
		float GetEntityMaxHealth(Classes::CEntity* pTarget) noexcept;
		void SetEntityHealth(Classes::CEntity* pTarget, float health) noexcept;
		float GetEntityHealthPercent(Classes::CEntity* pTarget) noexcept;	//	Gets the entity health percent

		/* planet / zone helpers */
		bool GetPlanetSphereScreenPoints(Classes::CEntity* pTarget, std::vector<Structs::FVector2D>* outPoints) noexcept;	//	Gets the planet sphere from bounds

		/* cactorentity helpers */
		void SetPlayerActorClipState(int state) noexcept;
		void SetPlayerActorFloatState(int state) noexcept;

		/* string helpers */
		std::string FormatString(const char* fmt, ...);	//	Formats a string
		std::string FormatDistance(const float& distance) noexcept; // "[10km]"
		Structs::FVector2D CalcTextSize(const std::string& text, const float& szFont = 8.f) noexcept;	//	Calculates the size of a text
		void CopyToClipboard(const char* input, ...);

		namespace datacore
		{
			__int64 GetStructDataFields(const std::string& name, Structs::SDataField** outBuffer);
			
			//	example fmt -> MiningGlobalParams.MiningGlobalParams
			__int64 GetStructInstance(const std::string& fmt);	
		}

		bool WorldToScreen(Structs::DVector& pos, const Structs::FVector2D& szScreen, Structs::FVector2D* out, const bool& bIsRelative) noexcept;
	}

	namespace Thread
	{
		static bool WorldToScreen(Structs::DVector& pos, const Structs::FVector2D& szScreen, Structs::FVector2D* out, const bool& bIsRelative) noexcept;	//	World to screen
		static bool CanvasDrawText(const char* text, const DWORD& index, Structs::DVector pos, const Structs::FQuat& color, bool bScreen2D = false);											//	C3DEngine ~ CRenderer
		static bool CanvasDrawTextf(const char* fmt, const DWORD& index, Structs::DVector pos, const Structs::FQuat& color, bool bScreen2D, ...);												//	C3DEngine ~ CRenderer
		static bool CanvasDrawTextf2(const char* fmt, Structs::DVector pos, const Structs::FQuat& color, bool bScreen2D, ...);																	//	C3DEngine ~ CRenderer
	
		/* entity helpers */
		void Teleport(Classes::CEntity* pEntity, const Structs::DVector& pos, const bool& cheat_bUFO = false) noexcept;	//	Teleports an entity to a position
		void Teleport(Classes::CEntity* pEntity, const Structs::DVector& pos, const Structs::DQuat& rotation, const bool& cheat_bUFO = false) noexcept;	//	Teleports an entity to a position
		void LocalTeleport(const Structs::DVector& pos, const bool& cheat_bUFO = false) noexcept;	//	Teleports an entity to a position
		void LocalTeleport(const Structs::DVector& pos, const Structs::DQuat& rotation, const bool& cheat_bUFO = false) noexcept;	//	Teleports an entity to a position
		
		/* features */
		void SetDevFlyModeState(int mFlyModeState) noexcept;	//	bFlyMode
		void UFO(Classes::CEntity* pEntity, const float& speed) noexcept;					//	UFO mode
		void PLAYER_UFO(const float& speed) noexcept;			//	player ufo mode
		void SHIP_UFO(const float& speed) noexcept;				//	UFO mode
		void ForgeEntity(Structs::STargetEntity& pEntity, Structs::SForgeControls& ctx);
		void SpaceShipHandbrake() noexcept;
		void GrabEntity(Classes::CEntity* pEntity, const float& dist) noexcept;	
		void GoToEntity(Classes::CEntity* pEntity, const float& dist) noexcept;	
		void rage_GrabAllPlayers(const float& dist) noexcept;			//	rage grab all players

		/* updates */
		void LocalPlayerUpdate(Classes::CEntity* pEntity) noexcept;	//	local player update
		void TargetEntityUpdate() noexcept;						//	target entity update
		void EntitiesUpdate() noexcept;							//	entities update		-> CEntitySystem::Update
		void ClassesUpdate(Classes::CEntitySystem* pEntitySystem) noexcept;							//	classes update		-> CEntitySystem::Update
		void TransformsUpdate() noexcept;						//	transforms update	-> CRenderer::MT_Update
		void ScreenPointsUpdate(Classes::CRenderer* pRenderer) noexcept;						//	screen points update	-> CRenderer::RT_FlushTextMessages
		void WaypointsUpdate(Classes::CEntity* pLocalEntity) noexcept;						//	waypoints update	-> CRenderer::MT_Update

		/* ~ */
		bool cache_entry(Classes::CEntitySystem* pEntitySystem, vecString& input, vecEntityClassPair* out, bool& bFoundAll) noexcept;					//	generate entity class cache
	}

	namespace Cheats
	{
		void SetDevNoClip(bool state) noexcept;					//	Dev UFO mode
		void SetActorUFO(bool state) noexcept;					//	UFO mode
		void SetNoGForce(bool state) noexcept;					//	No GForce
		void SetNoFog(bool state) noexcept;						//	No Fog
		void SetDisableGlare(bool state) noexcept;				//	Disable Glare
		void SetWeaponNoRecoil(bool state) noexcept;			//	No Recoil
		void SetDisableArmistice(bool state) noexcept;			//	Disable Armistice
		void SetDisableATCRestrictions(bool state) noexcept;	//	Disable ATC Restrictions
		void SetHUDStatusEffects(bool state) noexcept;			//	Show extended status hud
		void SetEasyJumpGate(bool state) noexcept;				//	JumpGate enhancement
		void SetDisableAllShakes(bool state) noexcept;			//	Disable Shake Animations
		void AC_SetDisablePlayableAreaRestriction(bool state);
		void AC_DisableTimeLimit(bool state);
		bool SetDefaultLoadout(const unsigned __int8& loadoutIndex) noexcept;
		void SetDisableCorpseLootingRestrictions(bool state) noexcept;	//	Disable Corpse Looting Restrictions
	}

	namespace Math
	{
		bool IsPointInsideRadius(const float& szRadius, const Structs::FVector2D& radiusCenter, const Structs::FVector2D& point);
		float GetDistance2D(const Structs::FVector2D& from, const Structs::FVector2D& to);
		Structs::DQuat ToQuaternion(const Structs::FVector& angles) noexcept;
		Structs::DQuat MultiplyQuaternions(const Structs::DQuat& q1, const Structs::DQuat& q2);
		void NormalizeQuaternion(Structs::DQuat& q) noexcept;
		void RotatePoint(const Structs::FVector& point, const Structs::FVector& center, const float* angles, Structs::FVector* result);
		void RotatePoint(const Structs::DVector& point, const Structs::DVector& center, const float* angles, Structs::DVector* result);
		void RotateBoxVerts(const Structs::DVector boxVerts[8], const Structs::DVector& center, const Structs::FVector& angles, Structs::DVector boxVertsResult[8]);
	}

	namespace Gui
	{
		void renderMenu();							//	render the imgui menu
		void renderWndw();							//	render the overlay window elements
		void renderWall();							//	render the wallpaper / shroud

		namespace scMenu
		{
			void Header();							//	the head of the imgui menu
			void Body();							//	the body of the imgui menu
			void Footer();							//	the footer of the imgui menu

			namespace scTabs
			{
				void Enhancements();				//	general enhancements for the player
				void ObjectManager();				//	managing entities, missions, go to points, etc.
				void Dumper();						//	analyzing dumped game data such as cvars, commands, structs, etc.
				void Chat();						//	nightcity member chat
				void Config();						//	saving and loading configurations : @todo
			}
		}

		namespace scCanvas
		{
			void draw();					//	draw the overlay window

			namespace scWidgets
			{
				void Radar();						//	radar
				void MiniMap();						//	minimap
				void Skeleton();					//	player skeleton
				void StructData(const std::string& name);					//	struct data
			}
		}

		namespace scWallpaper
		{
			void draw();					//	draw the wallpaper	
		}

	}


	Classes::SSystemGlobalEnvironment* GetGlobalEnvironment() noexcept;

}
inline StarCitizen::Classes::SSystemGlobalEnvironment* gEnv = nullptr;
inline StarCitizen::Classes::CCamera* gCamera = nullptr;
inline StarCitizen::Classes::CGoToPointManager* gGoToMan = nullptr;

namespace NightCity
{
	struct SNCChatMSG
	{
		int msgID;				//	message index
		std::string username;	//	discord username
		std::string message;	//	message from server
		bool isLocalClient;
		bool isAdmin;
	};
	inline std::vector<SNCChatMSG> nightcityChat;
	inline std::string nightCityChatMSG;
	inline bool bNightCitySendMSG;
}