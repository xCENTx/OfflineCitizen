#include <pch.h>
#include "hack.h"
#include "game.h"
#include "dumper.h"
#include "memory.h"
#include "gui.h"
#include "FileManager.h"


//	fwd
static gui g_gui;
static int LastTick = 0;
static bool bKeyTimer = false;
static const char* wndwTitle = __("StarCitizen ");
void DevFeatures();	//	used for debug development features
DWORD tRenderUpdate();	//	render thread
DWORD tGameUpdate();	//	game data renderWndw thread
DWORD tNetworkUpdate();	//	network data

DWORD hack(LPVOID hModule)
{
	DWORD dwExit = 0;	//	exit code

	init();

	std::thread RenderThread(tRenderUpdate);
	//	std::thread GameUpdateThread(tGameUpdate);
	//	std::thread NetworkUpdateThread(tNetworkUpdate);

	//	Loop
	while (bRunning)
	{
		bKeyTimer = GetTickCount64() - LastTick > 500;

		//	@TODO: relocate to debug namespace
#if _DEBUG
		DevFeatures();
#endif
		//	
		bool bExitModule = (/*((GetAsyncKeyState(VK_END) & 0x8000) && bKeyTimer) || */StarCitizen::Hooks::vars::bFreeModule);
		if (bExitModule)
		{
			bRunning = false;
			break;	//	exit
		}

		//	~
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		std::this_thread::yield();
	}

	shutdown();

	RenderThread.join();
	//	GameUpdateThread.join();
	//	NetworkUpdateThread.join();

	FreeLibraryAndExitThread(HMODULE(hModule), dwExit);
	return dwExit;
}

DWORD tRenderUpdate()
{
	while (bRunning)
	{
		//	@todo: offload to render thread
		if (g_bAppWindow)
		{
			if (GetAsyncKeyState(g_iMenuKey) & 0x8000 && bKeyTimer)
			{
				g_bShowMenu ^= 1;
				StarCitizen::Hooks::vars::bWndwFocus = !g_bShowMenu;
				g_gui.UpdateViewState(StarCitizen::Hooks::vars::pGameWndw, g_bShowMenu);
				LastTick = GetTickCount64();
			}

			g_gui.update(StarCitizen::Hooks::vars::pGameWndw);
		}
		else
		{
			if (GetAsyncKeyState(g_iMenuKey) & 0x8000 && bKeyTimer)
			{
				initAppWindow();
				LastTick = GetTickCount64();
			}
		}


		//	~
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		std::this_thread::yield();
	}

	if (g_bAppWindow)
		shutdownAppWindow();

	return 0;
}

DWORD tGameUpdate()
{
	using namespace StarCitizen;

	while (bRunning)
	{
		if (Hooks::vars::bWndwFocus && Hooks::vars::target_player.bValid && Hooks::vars::target_player.bForge)
		{
			if ((GetAsyncKeyState('Z') & 0x8000 && GetAsyncKeyState('E') & 0x8000))
				Hooks::vars::target_controls.deltaX += 0.0001f;

			if ((GetAsyncKeyState('Z') & 0x8000 && GetAsyncKeyState('Q') & 0x8000))
				Hooks::vars::target_controls.deltaY += 0.0001f;
		}

		if (Hooks::vars::grab_bEnable && !Hooks::vars::grab_bGrab && !Hooks::vars::grab_bGoTo)
		{
			/* GRAB ENTITY */
			if (((GetAsyncKeyState(VK_LMENU) & 0x8000) && (GetAsyncKeyState('X') & 0x8000)))
				Hooks::vars::grab_bGrab = true;

			/* GO TO ENTITY */
			if (((GetAsyncKeyState(VK_LMENU) & 0x8000) && (GetAsyncKeyState('V') & 0x8000)))
				Hooks::vars::grab_bGoTo = true;

			/* SELECT TARGET */
			if (((GetAsyncKeyState(VK_LMENU) & 0x8000) && (GetAsyncKeyState('T') & 0x8000)))
			{
				Hooks::vars::target_player.bValid = false;
				Hooks::vars::target_pSelection = Hooks::vars::grab_pEntity;
				Hooks::vars::target_bSetNewPlayer = true;
			}
		}

		//	~
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		std::this_thread::yield();
	}

	return 0;
}

DWORD tNetworkUpdate()
{
	while (bRunning)
	{
		const auto& tick = GetTickCount64();

		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::this_thread::yield();
	}

	return EXIT_SUCCESS;
}

void DevFeatures()
{
#if _DEBUG

	/* toggle ufo mode */
	if ((GetAsyncKeyState(VK_XBUTTON1) & 0x8000) && bKeyTimer)
	{
		StarCitizen::Cheats::SetActorUFO(!StarCitizen::Hooks::vars::cheat_bUFO);

		LastTick = GetTickCount();
	}

	/* toggle walk speed */
	if ((GetAsyncKeyState(VK_XBUTTON2) & 0x8000) && bKeyTimer)
	{
		StarCitizen::Hooks::vars::cheat_bCustomWalkSpeed ^= 1;

		LastTick = GetTickCount();
	}

	//	TOGGLE WEAPON MODS
	//	if ((GetAsyncKeyState(VK_XBUTTON1) & 0x8000) && bKeyTimer)
	//	{
	//		StarCitizen::Cheats::SetWeaponNoRecoil(!StarCitizen::Hooks::vars::bNoRecoil);
	//		StarCitizen::Hooks::vars::bInfiniteAmmo ^= 1;
	//	
	//		LastTick = GetTickCount();
	//	}	

	///	
	//	if (GetAsyncKeyState(VK_NUMPAD3) & 0x8000 && bKeyTimer)
	//	{
	//		[&]() 
	//		{
	//			const auto& pEnv = gEnv;
	//			if (!pEnv)
	//				return;
	//	
	//			long long outClass;
	//			if (!StarCitizen::Helpers::GetEntityClassByName("OrbitingObjectContainer", &outClass))
	//				return;
	//	
	//			const auto& pClass = reinterpret_cast<StarCitizen::Classes::CEntityClass*>(outClass);
	//			printf("PlayerClass: 0x%llX\n", outClass);
	//		}();
	//	}
	
	//	TOGGLE INSTANT SALVAGE
	//if (GetAsyncKeyState(VK_NUMPAD3) & 0x8000 && bKeyTimer)
	//{
	//	StarCitizen::Hooks::vars::bInstantSalvage ^= 1;
	//}
	
	//	TOGGLE UFO
	//	if (GetAsyncKeyState(VK_NUMPAD5) & 0x8000 && bKeyTimer)
	//	{
	//		StarCitizen::Cheats::SetActorUFO(!StarCitizen::Hooks::vars::bUFO);
	//		bKeyTimer = GetTickCount64();
	//	
	//	}

	///	TOGGLE DEV NO CLIP
	//	if (GetAsyncKeyState(VK_NUMPAD1) & 0x8000 && bTimer)
	//	{
	//		StarCitizen::Hooks::vars::fly_bEnable ^= 1;                                 //  toggle state for fly mode
	//		switch (StarCitizen::Hooks::vars::fly_bEnable)
	//		{
	//		case true: StarCitizen::Hooks::vars::fly_iState = 2; break;      //  set fly mode on
	//		case false: StarCitizen::Hooks::vars::fly_iState = 0; break;     //  set fly mode off
	//		}
	//		StarCitizen::Hooks::vars::fly_bSet = true;                            //  set fly mode state from CSystemUpdate hook
	//	}
	
	///	DUMP
	//	if (GetAsyncKeyState(VK_NUMPAD9) & 0x8000 && bKeyTimer)
	//	{
	//		bKeyTimer = GetTickCount64();
	//	
	//		DumpStructs();
	//		DumpCommands();
	//		DumpCVARS();
	//		DumpClasses();
	//	
	//	}
	
	///	TEST FIND CVAR
	//	if (GetAsyncKeyState(VK_F3) & 0x8000 && bKeyTimer)
	//	{
	//		bKeyTimer = GetTickCount64();
	//	
	//		StarCitizen::Structs::SXCvar cvar{};
	//		if (StarCitizen::Helpers::GetCVar(__("e_DebugDraw"), &cvar))
	//		{
	//			const auto& value = cvar.GetValue<int>();
	//			printf(__("CVar: %s\nValue: %d\n"), cvar.Name, value);
	//		}
	//	
	//		//	const auto& pEnv = gEnv;
	//		//	if (!pEnv)
	//		//		continue;
	//		//	
	//		//	const auto& pConsole = pEnv->pConsole;
	//		//	if (!pConsole)
	//		//		continue;
	//		//	
	//		//	const auto& fn = StarCitizen::Functions::CXConsole_GetCVar_stub(__int64(pConsole), "e_DebugDraw");
	//		//	if (fn)
	//		//	{
	//		//		const auto& pCvar = CleanPointer<StarCitizen::Classes::CXCVar*>(reinterpret_cast<StarCitizen::Classes::CXCVar*>(fn));
	//		//		if (pCvar && pCvar->pValue)
	//		//			*(int*)pCvar->pValue ^= 1;
	//		//	}
	//	}
	
	///	TEST ENTITY ITERATOR
	//	if (GetAsyncKeyState(VK_NUMPAD9) & 0x8000 && bKeyTimer)
	//	{
	// 
	//		bKeyTimer = GetTickCount64();
	//		
	//		const auto& pEnv = gEnv;
	//		if (!pEnv)
	//			continue;
	//	
	//		const auto& pEntitySystem = pEnv->pEntitySystem;
	//		if (!pEntitySystem)
	//			continue;
	//	
	//		long long vIt = 0;
	//		const auto& pResult = pEntitySystem->vf_GetEntityIterator(&vIt);
	//		if (!pResult || !vIt)
	//			continue;
	//	
	//		const auto& czEntIt = reinterpret_cast<Classes::IEntityIt*>(vIt);
	//	
	//		czEntIt->vf_First();
	//		while (!czEntIt->vf_IsEnd())
	//		{
	//			const auto& pEnt = czEntIt->vf_NextEnt();
	//			if (!pEnt || !pEnt->pEntityClass)
	//				continue;
	//	
	//			Hooks::vars::vPlayerEntities.push_back(pEnt);
	//		}
	//	
	//		czEntIt->vf_Remove();
	//	}
	
	///	UNIT TESTS ~ keep last method so can early retun if needed
	//	if (GetAsyncKeyState(VK_F1) & 0x8000 && bKeyTimer)
	//	{
	//		bKeyTimer = GetTickCount64();
	//	
	//		const auto& pEnv = gEnv;
	//		if (!pEnv)
	//			return;
	//		printf(__("pEnv: 0x%llX\n"), pEnv);
	//	
	//		const auto& pGame = CleanPointer<StarCitizen::Classes::CGame*>(pEnv->pGame);
	//		if (!pGame)
	//			return;
	//		printf(__("pGame: 0x%llX\n"), pGame);
	//	
	//		const auto& pLocalPlayer = CleanPointer<StarCitizen::Classes::CSCPlayer*>(pGame->pLocalPlayer);
	//		if (!pLocalPlayer)
	//			return;
	//		printf(__("pLocalPlayer: 0x%llX\n"), pLocalPlayer);
	//	
	//		const auto& pLocalEntity = CleanPointer<StarCitizen::Classes::CEntity*>(pLocalPlayer->pEntity);
	//		if (!pLocalEntity)
	//			return;
	//		printf(__("pLocalEntity: 0x%llX ; %s\n"), pLocalEntity, (char*)pLocalEntity->pName);
	//	}
#endif
}

/*
	//////////////////////////////////////////////////////////////////////////////////////////
*/

// fwd
static memory g_mem;

void init()
{
	bool result = false;

	// set network crash callback
	exCrashHandler::SetCrashHandler(__(L"nc.log"), __(L"nc.dmp"));

	initConsole();

	dwModule = __int64(GetModuleHandle(0));

	g_mem.init();

	gEnv = reinterpret_cast<StarCitizen::Classes::SSystemGlobalEnvironment*>(GetAddr(StarCitizen::Offsets::gEnv));
	gGoToMan = reinterpret_cast<StarCitizen::Classes::CGoToPointManager*>(GetAddr(StarCitizen::Offsets::gGoToPointMan));
	
	initHooks();
	
	bRunning = true;
}

void shutdown()
{
	using namespace StarCitizen;
	memory::hooker::Remove((void*)GetAddr(Offsets::oEAC_HandleDiscipline));
	memory::hooker::Remove((void*)GetAddr(Offsets::oCXConsole_RegisterCvar_Int));
	memory::hooker::Remove((void*)GetAddr(Offsets::oCXConsole_RegisterCvar_Float));
	memory::hooker::Remove((void*)GetAddr(Offsets::oCXConsole_RegisterCvar_Int64));
	memory::hooker::Remove((void*)GetAddr(Offsets::oCXConsole_RegisterCvar_String));
	memory::hooker::Remove((void*)GetAddr(Offsets::oCXConsole_AddCommand));
	memory::hooker::Remove((void*)GetAddr(Offsets::oCXCommand_MegaMap));
	memory::hooker::Remove((void*)GetAddr(Offsets::oCXCommand_LoadMegaMap));
	memory::hooker::Remove((void*)GetAddr(Offsets::oCDataCore_RegisterStruct));
	memory::hooker::Remove((void*)GetAddr(Offsets::oCEntityClassRegistry_RegisterClass));
	memory::hooker::Remove((void*)GetAddr(Offsets::oCEntityClassRegistry_FindClass));
	memory::hooker::Remove((void*)GetAddr(Offsets::oCSystem_Update));
	//	memory::hooker::Remove((void*)GetAddr(Offsets::oCEntitySystem_SpawnEntity));
	//	memory::hooker::Remove((void*)GetAddr(Offsets::oCEntitySystem_DeleteEntity));
	//	memory::hooker::Remove((void*)GetAddr(Offsets::oCRenderer_MTUpdate));
	//	memory::hooker::Remove((void*)GetAddr(Offsets::oC3DEngine_RenderWorld));
	//	memory::hooker::Remove((void*)GetAddr(Offsets::oCCamerViewManager_Update));
	//	memory::hooker::Remove((void*)GetAddr(Offsets::oCSCAmmoContainerComponent_GetAmmoCount));
	//	memory::hooker::Remove((void*)GetAddr(Offsets::oCCharacterStateHiearchy_VerifyState));
	//	memory::hooker::Remove((void*)GetAddr(Offsets::oCWeaponActionFireSalvageRepair_GetRayCastRequest));
	//	memory::hooker::Remove((void*)GetAddr(Offsets::oCWeaponActionFireTractorBeam_GetRayCastRequest));
	//	memory::hooker::Remove((void*)GetAddr(Offsets::oCWeaponActionFireHealingBeam_GetRayCastRequest));
	//	memory::hooker::Remove((void*)GetAddr(Offsets::oCWeaponActionFireSingle_Update));
	//	memory::hooker::Remove((void*)GetAddr(Offsets::oCWeaponActionFireBurst_Update));
	//	memory::hooker::Remove((void*)GetAddr(Offsets::oCWeaponActionFireRapid_Update));
	//	memory::hooker::Remove((void*)GetAddr(Offsets::oCInventoryComponent_AddItem));
	//	memory::hooker::Remove((void*)GetAddr(Offsets::oCSCLocalPlayerPersonalThoughtComponent_OnInventoryMoveItemOntoUnoccupiedPosition));
	//	memory::hooker::Remove((void*)GetAddr(Offsets::oCSCLocalPlayerMovement_Update));
	//	memory::hooker::Remove((void*)GetAddr(Offsets::oCSCItemQuantumDrive_Update));
	//	memory::hooker::Remove((void*)GetAddr(Offsets::oCSCItemMiningController_UpdateLaserThrottle));
	//	memory::hooker::Remove((void*)GetAddr(Offsets::oApplyHealthChange));
	//	memory::hooker::Remove((void*)GetAddr(Offsets::oActorKill));

	//	DetourTransactionBegin();
	//	DetourDetach((void**)&StarCitizen::Functions::CWeaponActionFireHealingBeam_GetRayCastRequest_stub, StarCitizen::Hooks::CWeaponActionFireHealingBeam_GetRayCastRequest_hook);
	//	DetourDetach((void**)&StarCitizen::Functions::CWeaponActionFireSalvageRepair_GetRayCastRequest_stub, StarCitizen::Hooks::CWeaponActionFireSalvageRepair_GetRayCastRequest_hook);
	//	DetourDetach((void**)&StarCitizen::Functions::CEntityComponentMineable_OnHitByMiningLaser_stub, StarCitizen::Hooks::CEntityComponentMineable_OnHitByMiningLaser_hook);
	//	if (DetourTransactionCommit() != NO_ERROR)
	//	{
	//	#if _DEBUG
	//		printf("- DetourTransactionCommit failed\n");
	//	#endif
	//	}

	/* shutdown memory */
	g_mem.shutdown();

	fclose(Hooks::vars::console_output_stream);
	ShowWindow(Hooks::vars::console_wndw, SW_HIDE);			// hide console window
	FreeConsole();								// free console	
}

/*
	//////////////////////////////////////////////////////////////////////////////////////////
*/

// @todo: relocate to console class
void initConsole()
{
	//	CONSOLE OUTPUT
	AllocConsole();
	freopen_s(&StarCitizen::Hooks::vars::console_output_stream, "CONOUT$", "w", stdout);
	StarCitizen::Hooks::vars::console_handle = GetStdHandle(STD_OUTPUT_HANDLE);	// output handle
	StarCitizen::Hooks::vars::console_wndw = GetConsoleWindow();					// console window handle
	ShowWindow(StarCitizen::Hooks::vars::console_wndw, SW_HIDE);					// hide console window
}

// @todo: relocate
void initHooks()
{

	std::vector<unsigned char> patch = { 0x90, 0xE9 };
	if (!memory::Patch(GetAddr(StarCitizen::Offsets::oDisableCrashDumps), patch))
		return;

	/* BYPASS PU CHECKPOINT */
	patch = { 0x74, 0x74 };
	if (!memory::Patch(GetAddr(StarCitizen::Offsets::oBypassPUCheckpoint), patch))
		return;

	/* PATCH EAC DISCIPLINE SERVICE */
	bool bHooked = memory::hooker::Create(
		(void*)GetAddr(StarCitizen::Offsets::oEAC_HandleDiscipline),
		(void**)&StarCitizen::Functions::EAC_HandleDiscipline_stub,
		(void*)StarCitizen::Hooks::EAC_HandleDiscipline_hook
	);

	/* CVARS & COMMANDS */
	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCXConsole_RegisterCvar_Int),
			(void**)&StarCitizen::Functions::CXConsole_RegisterCvar_Int_stub,
			(void*)StarCitizen::Hooks::CXConsole_RegisterIntCvars_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CXConsole::RegisterIntCvars.\n");
#endif

		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCXConsole_RegisterCvar_Float),
			(void**)&StarCitizen::Functions::CXConsole_RegisterCvar_Float_stub,
			(void*)StarCitizen::Hooks::CXConsole_RegisterFloatCvars_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CXConsole::RegisterFloatCvars.\n");
#endif

		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCXConsole_RegisterCvar_Int64),
			(void**)&StarCitizen::Functions::CXConsole_RegisterCvar_Int64_stub,
			(void*)StarCitizen::Hooks::CXConsole_RegisterInt64Cvars_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CXConsole::RegisterInt64Cvars.\n");
#endif

		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCXConsole_RegisterCvar_String),
			(void**)&StarCitizen::Functions::CXConsole_RegisterCvar_String_stub,
			(void*)StarCitizen::Hooks::CXConsole_RegisterStringCvars_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CXConsole::RegisterStringCvars.\n");
#endif

		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCXConsole_AddCommand),
			(void**)&StarCitizen::Functions::CXConsole_AddCommand_stub,
			(void*)StarCitizen::Hooks::CXConsole_AddCMD_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CXConsole::AddCommand.\n");
#endif
	}

	/* DATA CORE */
	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCDataCore_RegisterStruct),
			(void**)&StarCitizen::Functions::CDataCore_RegisterStruct_stub,
			(void*)StarCitizen::Hooks::CDataCore_RegisterStruct_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CDataCore::RegisterStruct.\n");
#endif
	}

	/* CLASSES */
	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCEntityClassRegistry_RegisterClass),
			(void**)&StarCitizen::Functions::CEntityClassRegistry_RegisterClass_stub,
			(void*)StarCitizen::Hooks::CEntityClassRegistry_RegisterClass_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CEntityRegistrySystem::RegisterClass.\n");
#endif
	}

	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCSystem_Update),
			(void**)&StarCitizen::Functions::CSystem_Update_stub,
			(void*)StarCitizen::Hooks::CSystem_Update_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CSystem::Update\n");
#endif
	}

	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCXCommand_LoadMegaMap),
			(void**)&StarCitizen::Functions::CXCommand_LoadMegaMap_stub,
			(void*)StarCitizen::Hooks::CXCommand_LoadMegaMap_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CXCommand::LoadMegaMap\n");
#endif
	}

	return;

	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCEntitySystem_Update),
			(void**)&StarCitizen::Functions::CEntitySystem_Update_stub,
			(void*)StarCitizen::Hooks::CEntitySystem_Update_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CEntitySystem::Update.\n");
#endif
	}


	/* */
	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCEntity_Init),
			(void**)&StarCitizen::Functions::CEntity_Init_stub,
			(void*)StarCitizen::Hooks::CEntity_Init_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CEntity::Init\n");
#endif
	}
	
	{

		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCEntity_Shutdown),
			(void**)&StarCitizen::Functions::CEntity_Shutdown_stub,
			(void*)StarCitizen::Hooks::CEntity_Shutdown_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CEntity::Shutdown\n");
#endif
	}

	/* RENDERING */
	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCRenderer_MTUpdate),
			(void**)&StarCitizen::Functions::CRenderer_MTUpdate_stub,
			(void*)StarCitizen::Hooks::CRenderer_MTUpdate_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CRenderer::MTUpdate\n");
#endif
	}

	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oC3DEngine_RenderWorld),
			(void**)&StarCitizen::Functions::C3DEngine_RenderWorld_stub,
			(void*)StarCitizen::Hooks::C3DEngine_RenderWorld_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- C3DEngine::RenderWorld\n");
#endif
	}

	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCCamerViewManager_Update),
			(void**)&StarCitizen::Functions::CCamerViewManager_Update_stub,
			(void*)StarCitizen::Hooks::CCameraViewManager_Update_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CCameraViewManager::Update\n");
#endif
	}

	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCSCAmmoContainerComponent_GetAmmoCount),
			(void**)&StarCitizen::Functions::CSCAmmoContainerComponent_GetAmmoCount_stub,
			(void*)StarCitizen::Hooks::CSCAmmoContainerComponent_GetAmmoCount_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CSCAmmoContainerComponent::GetAmmoCount.\n");
#endif
	}

	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCWeaponActionFireTractorBeam_GetRayCastRequest),
			(void**)&StarCitizen::Functions::CWeaponActionFireTractorBeam_GetRayCastRequest_stub,
			(void*)StarCitizen::Hooks::CWeaponActionFireTractorBeam_GetRayCastRequest_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CWeaponActionFireTractorBeam::GetRayCastRequest\n");
#endif
	}

	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCWeaponActionFireSingle_Update),
			(void**)&StarCitizen::Functions::CWeaponActionFireSingle_Update_stub,
			(void*)StarCitizen::Hooks::CWeaponActionFireSingle_Update_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CWeaponActionFireSingle::Update\n");
#endif
	}

	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCWeaponActionFireBurst_Update),
			(void**)&StarCitizen::Functions::CWeaponActionFireBurst_Update_stub,
			(void*)StarCitizen::Hooks::CWeaponActionFireBurst_Update_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CWeaponActionFireBurst::Update\n");
#endif
	}

	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCWeaponActionFireRapid_Update),
			(void**)&StarCitizen::Functions::CWeaponActionFireRapid_Update_stub,
			(void*)StarCitizen::Hooks::CWeaponActionFireRapid_Update_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CWeaponActionFireRapid::Update\n");
#endif
	}

	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCSCLocalPlayerMovement_Update),
			(void**)&StarCitizen::Functions::CSCLocalPlayerMovementSpeed_stub,
			(void*)StarCitizen::Hooks::CSCLocalPlayerMovementSpeed_stub_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CSCLocalPlayerMovementSpeed::stub_hook\n");
#endif
	}

	/* MAX INVENTORY */
	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCInventoryComponent_AddItem),
			(void**)&StarCitizen::Functions::CInventoryComponent_AddItem_stub,
			(void*)StarCitizen::Hooks::CInventoryComponent_AddItem
		);
#if _DEBUG
		if (!bHooked)
			printf("- CInventoryComponent::AddItem\n");
#endif
	}

	/* MAX INVENTORY */
	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCSCLocalPlayerPersonalThoughtComponent_OnInventoryMoveItemOntoUnoccupiedPosition),
			(void**)&StarCitizen::Functions::CSCLocalPlayerThoughComponent_OnInventoryMoveItemOntoUnoccupiedPosition_stub,
			(void*)StarCitizen::Hooks::CSCLocalPlayerThoughComponent_OnInventoryMoveItemOntoUnoccupiedPosition_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CSCLocalPlayerThoughComponent::OnInventoryMoveItemOntoUnoccupiedPosition\n");
#endif
	}

	/* NO GFORCE */
	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCSCActorGForce_Update),
			(void**)&StarCitizen::Functions::CSCActorGForce_Update_stub,
			(void*)StarCitizen::Hooks::CSCActorGForce_Update_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CSCActorGForce::Update\n");
#endif
	}

	/* INSTANT QT */
	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCSCItemQuantumDrive_Update),
			(void**)&StarCitizen::Functions::CSCItemQuantumDrive_Update_stub,
			(void*)StarCitizen::Hooks::CSCItemQuantumDrive_Update_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CSCItemQuantumDrive::Update_hook\n");
#endif
	}

	/* DEMI GOD */
	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oAddToHitbox),
			(void**)&StarCitizen::Functions::AddToHitBox_stub,
			(void*)StarCitizen::Hooks::AddToHitBox_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- AddToHitBox::hook\n");
#endif // _DEBUG
	}

	/* DAMAGE HANDLING */
	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCGameRulesSCDamageHandling_OnHit),
			(void**)&StarCitizen::Functions::CGameRulesSCDamageHandling_OnHit_stub,
			(void*)StarCitizen::Hooks::CGameRulesSCDamageHandling_OnHit_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CGameRulesSCDamageHandling::OnHit_hook\n");
#endif // _DEBUG
	}

	/* FLUSH MESSAGES */
	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCRenderer_FlushTextMessages),
			(void**)&StarCitizen::Functions::CRenderer_FlushTextMessages_stub,
			(void*)StarCitizen::Hooks::CRenderer_FlushTextMessages_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CRenderer::FlushTextMessages_hook\n");
#endif // _DEBUG
	}

	/* CLIPPING */
	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCRigidEntity_VerifyExistingContacts),
			(void**)&StarCitizen::Functions::CRigidEntity_VerifyExistingContacts_stub,
			(void*)StarCitizen::Hooks::CRigidEntity_VerifyExistingContacts_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CRigidEntity::VerifyExistingContacts_hook\n");
#endif // _DEBUG
	}

	/* fracture mineable */
	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oFractureMineable),
			(void**)&StarCitizen::Functions::FractureMineable_stub,
			(void*)StarCitizen::Hooks::FractureMineable_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CRigidEntity::VerifyExistingContacts_hook\n");
#endif // _DEBUG
	}

	/* mining controller */
	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oCSCItemMiningController_UpdateLaserThrottle),
			(void**)&StarCitizen::Functions::CSCItemMiningController_UpdateLaserThrottle_stub,
			(void*)StarCitizen::Hooks::CSCItemMiningController_UpdateLaserThrottle_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- CSCItemMiningController::UpdateLaserThrottle_hook\n");
#endif // _DEBUG
	}

	/* ship boost mp */
	{
		bHooked = memory::hooker::Create(
			(void*)GetAddr(StarCitizen::Offsets::oShipBoostMP),
			(void**)&StarCitizen::Functions::ShipBoostMP_stub,
			(void*)StarCitizen::Hooks::ShipBoostMP_hook
		);
#if _DEBUG
		if (!bHooked)
			printf("- ShipBoostMP_hook.\n");
#endif
	}

	/* Weapon Healing , Salvage & OnHitByMiningLaser */
	{
		DetourTransactionBegin();
		DetourAttach((void**)&StarCitizen::Functions::CWeaponActionFireHealingBeam_GetRayCastRequest_stub, StarCitizen::Hooks::CWeaponActionFireHealingBeam_GetRayCastRequest_hook);
		DetourAttach((void**)&StarCitizen::Functions::CWeaponActionFireSalvageRepair_GetRayCastRequest_stub, StarCitizen::Hooks::CWeaponActionFireSalvageRepair_GetRayCastRequest_hook);
		DetourAttach((void**)&StarCitizen::Functions::CEntityComponentMineable_OnHitByMiningLaser_stub, StarCitizen::Hooks::CEntityComponentMineable_OnHitByMiningLaser_hook);
		if (DetourTransactionCommit() != NO_ERROR)
		{
#if _DEBUG
			printf("- DetourTransactionCommit failed\n");
#endif
		}
	}
}

// @todo: relocate 
void initAppWindow()
{
	using namespace StarCitizen;

	Hooks::vars::pGameWndw = memory::GetMainProcWndw();
	if (!Hooks::vars::pGameWndw)
	{
#if _DEBUG
		printf("Failed to find game window.\n");
#endif
		return;
	}

	//	hook game window
	Hooks::vars::origWndProc = (WNDPROC)SetWindowLongPtr(Hooks::vars::pGameWndw, GWLP_WNDPROC, (LONG_PTR)Hooks::WndProc_hook);

	g_bAppWindow = g_gui.init();
}

// @todo: relocate 
void shutdownAppWindow()
{
	using namespace StarCitizen;

	//	restore window hook
	SetWindowLongPtr(Hooks::vars::pGameWndw, GWLP_WNDPROC, (LONG_PTR)Hooks::vars::origWndProc);

	g_gui.shutdown();
	Hooks::vars::pGameWndw = nullptr;
	Hooks::vars::origWndProc = nullptr;
}