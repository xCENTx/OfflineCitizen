#include <pch.h>
#include <game.h>
#include <gui.h>
#include <sdk/DataCore.h>
#include "dumper.h"


namespace StarCitizen
{
	typedef gui::widget ScGui;

	namespace Structs
	{
		/* must be created from game thread */
		SWaypoint::SWaypoint(const char* label)
		{
			/* Get reference to local player entity */
			pEntity = Helpers::GetLocalPlayerEntity();
			if (!pEntity)
				return;	// early exit , bValid is initialized as false

			/* get local zone for updating waypoint location */
			pZone = pEntity->pLocalZone;
			if (!pZone)
				return;

			/* get reference to zone entity */
			pZoneEntity = pEntity->pLocalZoneEntity;
			if (!pZoneEntity)
				return;

			/* get & store world position */
			if (!pEntity->vf_GetWorldPos(&mWorldPos))
				return;

			/* get & store current rotation */
			if (!pEntity->vf_GetWorldRotation(&mWorldRot))
				return;

			/* set remaining members */
			mName = label;
			mZoneLocalPos = pZoneEntity->transformZoneSpace.mTranslation;
			mLocalPos = pEntity->transformZoneSpace.mTranslation;
			dbg_origin = mWorldPos;
			bValid = true;

			/* update waypoint */
			update();
		}

		/* must be updated from game thread */
		void SWaypoint::update()
		{
			/* is valid ? */
			if (!bValid || !pZone || !pZoneEntity)
			{
				bValid = false;
				return;
			}

			/* get center of zone */
			Structs::DVector center;
			if (!pZoneEntity->vf_GetWorldPos(&center))
			{
				bValid = false;
				return;
			}

			/* get transposed position */
			Structs::DVector displacement = center + mLocalPos;

			/* get angles of the zone */
			Structs::FVector angles;
			if (!pZoneEntity->vf_GetEularAngles(&angles))
			{
				bValid = false;
				return;
			}

			/* rotate waypoint around zone center */
			Structs::DVector tempOut;
			Math::RotatePoint(displacement, center, (float*)&angles, &mWorldPos);

			bValid |= true;
		}

		STransforms::STransforms(Classes::CEntity* entity)
		{
			if (!entity || !entity->pName)
				return;

			pEntity = entity;

			update();
		}
		
		void STransforms::update()
		{
			STransforms newTM;
			if (this->pEntity == nullptr)
				return;

			Helpers::GetEntityBounds(pEntity, &newTM) ? *this = newTM : *this = STransforms();
		}

		STargetEntity::STargetEntity(Classes::CEntity* entity)
		{
			if (!entity || !entity->pName)
				return;

			mName = (char*)entity->pName;
			pEntity = entity;
			TM = STransforms(pEntity);
			bValid = true;

			update();
		}

		void STargetEntity::update()
		{
			if (!bValid || !pEntity || !pEntity->pName)
			{
				bValid = false;
				TM = STransforms();
				return;
			}

			/* update TM */
			TM.update();

			bValid |= true;
		}
	}

	namespace Hooks
	{
		/* VTABLE INDICES */
#define vft_DataCore_GetDataFields 0x58			//	CDataCore::GetStructDataFields
#define vft_MT_GetDisplayWidth 0x188			//	CRenderer::MT_GetDisplayWidth
#define vft_MT_GetDisplayHeight 0x190			//	CRenderer::MT_GetDisplayHeight
#define vft_CEntityComponentSystem_GetComponentID 0x10 // CEntityComponentSystem::GetComponentID
#define vft_Actor_SetHealth 0x6E8 				//	CActorComponent::SetHealth
#define vft_Actor_GetHealth 0x6F0 				//	CActorComponent::GetHealth	// CFlowActorGetHealth - No Entity or Entity not an Actor!
#define vft_Actor_GetMaxHealth 	0x700 			//	CActorComponent::GetMaxHealth
#define vft_CEntity_GetComponent 0x390			//	CEntity::GetComponent
#define vft_CEntity_GetCharacter 0x560			//	CEntity::GetCharacter 

		/* OFFSETS */
#define offset_InventoryComponent 0x2E0			//	CSCLocalPlayerPersonalThoughtComponent::OnInventoryMoveItemOntoUnoccupiedPosition ln148 | v37 = a1 + 0x2A8;
#define offset_PlayerMovementSpeed 0x70			//	CSCLocalPlayerMovement::Update::m_WalkSpeed

		/* flags */
#define FLAG_ENTITY_SHIP 0x8000A000


		LRESULT	CALLBACK WndProc_hook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			static const float fMinFracture = 0.01f;								//
			static const float fMaxFracture = 5.f;								
			static const float fMaxUfoSpeed = 1000000000.f;						//	1,000,000,000.f         //	this float can be found in the .data section of the dll ... all static variables can be.
			static const float fMinUfoSpeed = fMaxUfoSpeed / fMaxUfoSpeed;		//	1.f		//	this is a tricky method so that a float of 1.f is not added to the .data section of the dll ... lol . Just thought i'd share that.
			static const float fMaxWalkSpeed = fMaxUfoSpeed / 50000000;			//	20 m/s
			static const float fMinWalkSpeed = fMaxWalkSpeed / fMaxWalkSpeed;	//	1.f
			static const float fMaxForgeDistance = fMinWalkSpeed * 100.f;		//	100.f
			static const float fMinForgeDistance = fMaxForgeDistance * 0.01f;	//	1.f
			static const float fScalarMP = fMinUfoSpeed / 6;					//	0.166

			auto addScalar = [](float& ref, const float& scalar, const float& max) -> void { ref < max ? ref += (ref * scalar) : ref = max; };
			auto subScalar = [](float& ref, const float& scalar, const float& min) -> void { ref > min ? ref -= (ref * scalar) : ref = min; };

			///	SET UFO MODE SPEED
			if (uMsg == WM_MOUSEWHEEL)
			{
				const auto res = GET_WHEEL_DELTA_WPARAM(wParam);
				if (res > 0)
				{

					if (vars::cheat_bUFO)
					{
						vars::mUFOSpeedScalar < fMaxUfoSpeed
							? vars::mUFOSpeedScalar += (vars::mUFOSpeedScalar * fScalarMP) //	scale speed
							: vars::mUFOSpeedScalar = fMaxUfoSpeed;	//	max speed
					}

					if (GetAsyncKeyState(VK_LSHIFT) & 0x8000 && vars::cheat_bCustomWalkSpeed)
					{
						vars::mPlayerWalkSpeed < fMaxWalkSpeed
							? vars::mPlayerWalkSpeed += (vars::mPlayerWalkSpeed * fScalarMP) //	scale speed
							: vars::mPlayerWalkSpeed = fMaxWalkSpeed;	//	max speed
					}

					if (vars::target_player.bValid && (vars::target_player.bForge || vars::cheat_rage_bGrabAllEnemies))
					{
						if (GetAsyncKeyState('Z') & 0x8000)
							addScalar(vars::target_controls.distance, .05f, fMaxForgeDistance);
						if (GetAsyncKeyState(VK_LMENU) & 0x8000)
							addScalar(vars::target_controls.height, .05f, fMaxForgeDistance);
					}

					if (GetAsyncKeyState('Z') & 0x8000 && vars::cheat_bTunedMiningFractureBeam)
						addScalar(vars::cheat_mining_FractureThrottle, vars::cheat_mining_FractureThrottleLerp * 0.01f, fMaxFracture);
				}
				else if (res < 0)
				{
					if (vars::cheat_bUFO)
					{
						vars::mUFOSpeedScalar > fMinUfoSpeed
							? vars::mUFOSpeedScalar -= (vars::mUFOSpeedScalar * fScalarMP)	//	scale speed
							: vars::mUFOSpeedScalar = fMinUfoSpeed;	//	min speed
					}

					if (GetAsyncKeyState(VK_LSHIFT) & 0x8000 && vars::cheat_bCustomWalkSpeed)
					{
						vars::mPlayerWalkSpeed > fMinWalkSpeed
							? vars::mPlayerWalkSpeed -= (vars::mPlayerWalkSpeed * fScalarMP) //	scale speed
							: vars::mPlayerWalkSpeed = fMinWalkSpeed;	//	min speed
					}

					if (vars::target_player.bValid && (vars::target_player.bForge || vars::cheat_rage_bGrabAllEnemies))
					{
						if (GetAsyncKeyState('Z') & 0x8000)
							subScalar(vars::target_controls.distance, .05f, fMinForgeDistance);
						if (GetAsyncKeyState(VK_LMENU) & 0x8000)
							subScalar(vars::target_controls.height, .05f, fMinForgeDistance);
					}

					if (GetAsyncKeyState('Z') & 0x8000 && vars::cheat_bTunedMiningFractureBeam)
						subScalar(vars::cheat_mining_FractureThrottle, vars::cheat_mining_FractureThrottleLerp * 0.01f, fMinFracture);
				}
			}

			switch (uMsg)
			{
			case WM_SETFOCUS:
				vars::bWndwFocus = true;
				break;

			case WM_KILLFOCUS:
				vars::bWndwFocus = false;
				break;
			}
			return CallWindowProc(vars::origWndProc, vars::pGameWndw, uMsg, wParam, lParam);
		}

		__int64 __fastcall EAC_HandleDiscipline_hook(void* a1)
		{

			//	need to understand how much this is called. i think its constantly

			//	vars::bFreeModule = true;	//	since our user has been detected for cheating , the module should be unloaded. 



			return 0;
		}

		/*
		
			USED FOR LAUNCHING DEDICATED SERVER MODE
		
		*/
		class SSystemInitParams
		{
		public:
			char pad_0000[64]; //0x0000
			bool bEditor; //0x0040
			bool bDedicatedServer; //0x0041
			bool bIsService; //0x0042
			bool bShaderCacheGen; //0x0043
			bool bPSOCacheGen; //0x0044
			bool bAudioOfflineRender; //0x0045
			bool bCIGAudioConsolidate; //0x0046
			bool bIsHeadlessClient; //0x0047
			bool bResourceCompiler; //0x0048
			bool bSupportsSaveLoad; //0x0049
			bool bPreview; //0x004A
			bool bTestMode; //0x004B
			bool bEnableConsoleInShell; //0x004C
			bool bExecuteCommandLine; //0x004D
			bool bMinimal; //0x004E
			bool bSkipFont; //0x004F
			bool bSkipVideoCapture; //0x0050
			bool bSkipCIGBackend; //0x0051
			bool bSkipCIGServices; //0x0052
			bool bSkipInput; //0x0053
			bool bSkipRenderer; //0x0054
			bool bSkipConsole; //0x0055
			bool bSkipNetwork; //0x0056
			bool bSkipLocalization; //0x0057
			void* hInstance; //0x0058
			void* hWnd; //0x0060
			void* pUserCallback; //0x0068
			void* pValidator; //0x0070
			void* pPrintSync; //0x0078
			void* pSystem; //0x0080
			__int64 pStartupConfig; //0x0088
			char pad_0090[32]; //0x0090
		}; //Size: 0x00B0
		static_assert(sizeof(SSystemInitParams) == 0xB0);
		bool __fastcall CSystem_Init_hook(void* a1, void* startupParams)
		{

			__int64 v30;
			__int64 v31;
			const auto fnGetDebugUI = reinterpret_cast<__int64(*)(__int64)>(GetAddr(0x02B4D40));
			const auto fnSpawnDebugUI = reinterpret_cast<__int64(*)(__int64)>(GetAddr(0x07108F00));
			
			v30 = fnGetDebugUI(0x2610i64);
			if (v30)
			{
				v31 = fnSpawnDebugUI(v30);
				*(__int64*)(*(__int64*)((__int64)a1 + 24) + 408) = v31;
				printf("[+] Spawned Debug UI at 0x%llX\n", v31);
			}
			else
				printf("[!] Failed to get Debug UI\n");

			return Functions::CSystem_Init_stub(a1, (__int64)startupParams);
			
			
			//	const auto result = Functions::CSystem_Init_stub(a1, (__int64)startupParams);

			// load dev tools dll
			// EngineModule_Impl = LoadEngineModule_Impl( "CIGDevelopmentTools", "InitializeModule_CIGDevelopmentTools", 40, this, (__int64)startupParams, 0);
			// *(__int64*)(*(__int64*)(a1 + 0x18) + 0x178) = EngineModule_Impl;
			// __int64 __fastcall LoadEngineModule_Impl(const char* sModule, const char* sModuleInitFunction, char eSubsystem, __int64 pSystem, __int64 rInitParams, char bDatacoreComponentsOnly);

			// define function pointer type for LoadEngineModule_Impl
			const auto LoadEngineModule_Impl = reinterpret_cast<__int64* (*)(const char* sModule, const char* sModuleInitFunction, char eSubsystem, __int64 pSystem, __int64 rInitParams, char bDatacoreComponentsOnly)>(GetAddr(0x0712E60));
			
			// load CIGDevelopmentTools module
			auto EngineModule_Impl = LoadEngineModule_Impl("CIGDevelopmentTools", "InitializeModule_CIGDevelopmentTools", 40, (__int64)a1, (__int64)startupParams, 0);
			if (!EngineModule_Impl)
			{
				printf("[!] Failed to load CIGDevelopmentTools module\n");
			}
			else
			{
				*(__int64*)(*(__int64*)((__int64)a1 + 0x18) + 0x178) = (__int64)EngineModule_Impl; // set dev tools pointer for gEnv->pEngineModule
				printf("[+] Loaded CIGDevelopmentTools module at 0x%llX\n", EngineModule_Impl);
			}

			//	return result;

			/*  Dedicated Server Notes
			*
				LogTraceConditional(
					"[CIG] gEnv->IsDedicated[%d] CSystem::m_bDedicatedServer[%d] startupParams.bDedicatedServer[%d]",
					BYTE1(qword_149F9A436),
					*(unsigned __int8 *)(_R15 + 0x3DB),
					*(unsigned __int8 *)(a2 + 0x41));

			* cmd line
				LogTraceConditional("[CIG] startupParams.szSystemCmdLine	[%s]", v179);	0x130

			* flags
				v435[0] = v182;
				v436 = "Online";
				v422[0] = *(_BYTE *)(a2 + 0x46);
				v423 = "Headless";
				v428[0] = *(_BYTE *)(a2 + 0x42);
				v429 = "Service";
				v424[0] = *(_BYTE *)(a2 + 0x41);

						Wiljafor1:
							info( "[+] CSystem_init_hook called ...\n" );

							*(BYTE*)( a2 + 0x41 ) = 1;
							info( "[+] Set startupParams.bDedicatedServer to true\n" );

							*(BYTE*)( a2 + 0x52 ) = 1;
							//*(BYTE*)( a2 + 0x55 ) = 1;

							const auto gEnvFlags = GetAddr( 0x9F9A436 );
							if ( gEnvFlags )
							{
								*(BYTE*)( gEnvFlags + 1 ) = 1;  // BYTE1(gEnvFlags)
								info( "[+] Set gEnvFlags IsDedicated to true at addr: 0x%llX\n", gEnvFlags );
			}
			*/
			{
				const auto& _R15 = a1;
				/// STARTUP PARAMS OLD METHOD
				auto ref = IDA_OFFSET(0x9E8BC85) + 2;
				if (ref && *(bool*)ref == 0)
				{
					*(bool*)ref = 1; // BYTE2(qword_149E8BC85) = 1;
					printf("[+] Set gEnv->IsDedicated to true at addr: 0x%llX\n", ref);
				}

				///	
				//	auto bDedicatedServer = (a2 + 0x41); // startupParams.bDedicatedServer[%d]
				//	if (bDedicatedServer)
				//		*(bool*)(a2 + 0x41) = 1;
				//	
				//	auto szCmdLine = (char*)(*(long long*)(a2 + 0x130)); // startupParams.szSystemCmdLine	[%s]
				//	
				//	
				//	if (ref && m_bDedicatedServer && bDedicatedServer && szCmdLine)
				//	{
				//		printf(__("[*] gEnv->IsDedicated[%d] CSystem::m_bDedicatedServer[%d] startupParams.bDedicatedServer[%d]\n"), 
				//			*(bool*)ref, 
				//			*(bool*)m_bDedicatedServer, 
				//			*(bool*)bDedicatedServer
				//		);
				//		//	printf(__("[*] startupParams.szSystemCmdLine	[%s]\n"), szCmdLine);
				//	}

				//	set dedicated server flag to true
				if (!((SSystemInitParams*)startupParams)->bDedicatedServer)
				{
					((SSystemInitParams*)startupParams)->bDedicatedServer = true;
					printf("[+] Set startupParams.bDedicatedServer to true\n");
				}

				if (!((SSystemInitParams*)startupParams)->bSkipCIGServices)
				{
					((SSystemInitParams*)startupParams)->bSkipCIGServices = true;
					printf("[+] Set startupParams.bSkipCIGServices to true\n");
				}

				auto m_bDedicatedServer = ((long long)_R15 + 0x3DD); // CSystem::m_bDedicatedServer[%d]
				if (m_bDedicatedServer && *(bool*)m_bDedicatedServer == 0)
				{
					*(bool*)m_bDedicatedServer = 1;
					printf("[+] Set CSystem::m_bDedicatedServer to true\n");
				}

				// get system command line
				const auto parma = (SSystemInitParams*)startupParams;
				auto v173 = (**(__int64(__fastcall***)(void*))parma->pStartupConfig)((void*)parma->pStartupConfig);
				if (v173)
				{

					auto v174 = (const char*)(*(__int64(__fastcall**)(__int64))(*(__int64*)v173 + 0x18))(v173);
					printf("[*] startupParams.szSystemCmdLine[%s]", v174);
				}
			}
		}

		__int64 __fastcall CSystem_Update_hook(void* a1, void* a2, void* a3, void* a4)
		{
#if _DEBUG

			if (vars::fly_bSet)
				Thread::SetDevFlyModeState(vars::fly_iState);

#endif	//	dev features

			/*	GO TO POINT */
			if (vars::goto_bFindPoint)
			{
				Structs::DVector pos;
				if (Helpers::GetPointByName(vars::goto_TargetName.c_str(), &pos))
				{
					/* set pos & enable UFO mode */
					Classes::CEntity* pLocalEntity = StarCitizen::Helpers::GetLocalPlayerEntity();
					Thread::LocalTeleport(pos, true);
				}
				else
					printf(__("[!] Failed to find go to point % s\n"), vars::goto_TargetName.c_str());
			
				vars::goto_bFindPoint = false;
				vars::goto_TargetName.clear();
			}

			/* COMMAND EXECUTOR */
			if (vars::cmd_bExec)
			{
				vars::cmd_bExec = false;
			
				Hooks::vars::cmd_selection.ExecuteCmd(Hooks::vars::cmd_count, Hooks::vars::cmd_args);
			
				/* reset for next command */
				Hooks::vars::cmd_count = 1;
				memset(Hooks::vars::cmd_args, 0, sizeof(Hooks::vars::cmd_args));
			}
			
			//	/* heal local player */
			//	if (vars::cheat_bHealSelf || vars::cheat_bAutoHeal)
			//	{
			//		vars::cheat_bHealSelf = false;
			//		auto pLocalEntity = Helpers::GetLocalPlayerEntity();
			//		if (pLocalEntity)
			//		{
			//			float newHealth = 0.0f;
			//	
			//			/* auto heal local player */
			//			if (vars::cheat_bAutoHeal)
			//				newHealth = Helpers::GetEntityHealth(pLocalEntity);
			//			else
			//				newHealth = Helpers::GetEntityMaxHealth(pLocalEntity);
			//	
			//			Helpers::SetEntityHealth(pLocalEntity, newHealth);
			//		}
			//	}
			//	
			//	/* kill local player */
			//	if (vars::cheat_bKillSelf)
			//	{
			//	
			//		vars::cheat_bKillSelf = false;
			//		auto pLocalEntity = Helpers::GetLocalPlayerEntity();
			//		if (pLocalEntity)
			//		{
			//			Helpers::SetEntityHealth(pLocalEntity, Helpers::GetEntityMaxHealth(pLocalEntity) * 0.f);
			//	
			//	
			//			//	const auto& pComponents = CleanPointer<Classes::IEntityComponents*>(pLocalEntity->pComponents);
			//			//	if (pComponents)					{
			//			//	
			//			//	
			//			//		const auto& pActorComponent = CleanPointer<Classes::CSCActorComponent*>(pComponents->pActorComponent);
			//			//		const auto& pHealthComponent = CleanPointer<Classes::CSCBodyHealthComponent*>(pComponents->pHealthComponent);
			//			//		if (pActorComponent && pHealthComponent)
			//			//	
			//			//		{
			//			//			//	Functions::CSCBodyHealthComponent_ApplyHealthChange(
			//			//			//		(__int64)pHealthComponent,
			//			//			//		0.f,
			//			//			//		(__int64)pLocalEntity,
			//			//			//		0
			//			//			//	);
			//			//	
			//			//			//	Functions::CSCBodyHealthComponent_AuthorityRequestHit(
			//			//			//		(__int64)pHealthComponent, 
			//			//			//		(__int64)pLocalEntity, 
			//			//			//		135.0f
			//			//			//	);
			//			//	
			//			//			//	CallVFunction<__int64>(pActorComponent, vft_Actor_SetHealth / 8, 0.f);
			//			//		}
			//			//	
			//			//	}
			//		}
			//	}
			//	
			//	
			//	if (vars::cheat_bSpaceBrake)
			//		Thread::SpaceShipHandbrake();

			/* do ufo mode */
			if (vars::cheat_bUFO)
			{
				if (vars::target_player.bValid && vars::target_player.bForge)
					vars::cheat_bUFO = false;
				else
					Thread::SHIP_UFO(vars::mUFOSpeedScalar);
			}
			
			//	/* auto destroy mineable */
			//	if (vars::cheat_mining_mFlag)
			//	{
			//		const long long success = 0xB700000000;
			//		vars::cheat_mining_mFlag = false;
			//		vars::cheat_mining_bAutoFracture = false;
			//	
			//		Functions::FractureMineable_stub(vars::cheat_mining_pMineable, (__int64)&success, 1);
			//	}
			//	
			//	if (vars::datacore_bFindInstance && !vars::datacore_TargetName.empty())
			//	{
			//		vars::datacore_bFindInstance ^= 1;
			//		if (const auto& pInstance = Helpers::datacore::GetStructInstance(vars::datacore_TargetName))
			//			Helpers::CopyToClipboard(__("%s: 0x%llX"), vars::datacore_TargetName.c_str(), pInstance);
			//		
			//		vars::datacore_TargetName.clear();
			//	
			//		//	const auto& pMiningGlobalParams = Helpers::datacore::GetStructInstance("MiningGlobalParams.MiningGlobalParams");
			//		//	const auto& pGlobalSalvageRepairParams = Helpers::datacore::GetStructInstance("SGlobalSalvageRepairBeamParams.SGlobalSalvageRepairBeamParams");
			//		//	printf("MiningGlobalParams: 0x%llX\n", pMiningGlobalParams);
			//		//	printf("SGlobalSalvageRepairBeamParams: 0x%llX\n", pGlobalSalvageRepairParams);
			//	}

			return Functions::CSystem_Update_stub(a1, a2, a3, a4);
		}


		static void C3DEngine_RenderWorld_helper(__int64 pRenderer, unsigned __int32 fillThreadID, const float text_height, const ImVec2& canvas_bottom_center, const Structs::FQuat text_color)
		{
			// render helper
			auto fn_RenderTags = [](Classes::CEntity* pLocalPlayer, Classes::CEntity* pLocalShip, const DWORD& renderIndex, const std::vector<Classes::SThreadEntity>& ents)
				{
					const float& minDist = 10.f;
					std::string nameTag;
					bool bRender{ false };
					float distScalarMP{ 100.f };
					float renderDistance{ -1.f };
					Structs::FQuat color{ 1.f, 1.f, 1.f, 1.f }; // default white

					if (!gEnv->pRenderer)
						return;

					Structs::DVector pLocalPos = vars::sLocalPlayer.TM.origin;

					const auto entities = ents;	//	copy entities
					for (const auto& x : entities)
					{
						const auto& ent = x.pEntity;

						// skip local player
						if (ent == pLocalPlayer || ent == pLocalShip)
							continue;

						nameTag = ent->pName == nullptr ? __("?") : (char*)ent->pName;
						switch (x.entityType)
						{
						case StarCitizen::Enums::EEntityType::ET_PLAYER:
							bRender = vars::esp_bPlayer;
							renderDistance = vars::esp_PlayerRange;
							color = vars::esp_PlayerColor;
							break;
						case StarCitizen::Enums::EEntityType::ET_SHIP:
							bRender = vars::esp_bShip;
							renderDistance = vars::esp_ShipRange;
							color = vars::esp_ShipColor;
							break;
						case StarCitizen::Enums::EEntityType::ET_ENEMY_NPC:
							bRender = vars::esp_bEnemyAI;
							renderDistance = vars::esp_EnemyAIRange;
							color = vars::esp_EnemyAIColor;
							nameTag = __("HOSTILE");
							distScalarMP = 10.f;
							break;
						case StarCitizen::Enums::EEntityType::ET_ANIMAL:
							bRender = vars::esp_bAnimals;
							renderDistance = vars::esp_AnimalRange;
							color = vars::esp_AnimalColor;
							nameTag = __("ANIMAL");
							distScalarMP = 10.f;
							break;
						case StarCitizen::Enums::EEntityType::ET_CONTAINER:
							bRender = vars::esp_bLoot;
							renderDistance = vars::esp_LootRange;
							color = vars::esp_LootColor;
							nameTag = __("LOOT");
							distScalarMP = 10.f;
							break;
						case StarCitizen::Enums::EEntityType::ET_ROCK:
							bRender = vars::esp_bRock;
							renderDistance = vars::esp_RockRange;
							color = vars::esp_RockColor;
							break;
						case StarCitizen::Enums::EEntityType::ET_ORBIT:
							bRender = vars::esp_bOrbit;
							renderDistance = vars::esp_OrbitRange;
							color = vars::esp_OrbitColor;
							break;
						}
						if (!bRender)
							continue;

						Structs::DVector pos = x.TM.origin;
						float dist = pLocalPos.Distance(pos);	//	get distance
						if (dist > (renderDistance * distScalarMP) && renderDistance < minDist)
							continue;

						//	draw text
						Thread::CanvasDrawTextf(
							__("%s %s"),								//	fmt
							renderIndex, 								//	canvas index
							pos, 										//	world position
							color, 										//	color
							false,
							Helpers::FormatDistance(dist).c_str(), 		//	fmt #1
							nameTag.c_str()								//	fmt #2
						);
					}
				};

			const auto& pLocalPlayer = Helpers::GetLocalPlayerEntity();
			if (!pLocalPlayer)
				return;

			const auto& pLocalShip = Helpers::GetLocalShipEntity();
			Structs::DVector pos_text{ 0.f, 0.f, 0.f };

			/* render player & planet name tags */
			if (const auto& pLocalPlayer = Helpers::GetLocalPlayerEntity())
			{
				const auto& pLocalShip = Helpers::GetLocalShipEntity();

				fn_RenderTags(pLocalPlayer, pLocalShip, fillThreadID, Hooks::vars::vScreenEntities);


				if (vars::esp_bActors)
				{
					std::vector<Classes::CSCActor*> actors;
					Structs::DVector pLocalPos = vars::sLocalPlayer.TM.origin;
					
					// safely retrieve actors from the global actor list
					{
						std::lock_guard<std::mutex> lock(vars::vActorsMutex);
						actors = vars::vAllActors;	//	copy actors
					} // release lock

					/* iterate actor array */
					for (auto actor : actors)
					{
						/* get entity owner */

						Classes::CEntity* ent = reinterpret_cast<Classes::CEntity*>((__int64)actor->pEntity & 0xFFFFFFFFFFFF);

						/* project tag */
						Structs::DVector pos;
						if (!ent->vf_GetWorldPos(&pos))
							continue;

						float dist = pLocalPos.Distance(pos);
						if (dist > vars::esp_ActorRange)
							continue;

						//	draw text
						Thread::CanvasDrawTextf(
							__("%s %s"),								//	fmt
							fillThreadID, 								//	canvas index
							pos, 										//	world position
							vars::esp_ActorColor, 						//	color
							false,
							Helpers::FormatDistance(dist).c_str(), 		//	fmt #1
							(char*)ent->pName							//	fmt #2
						);
					}
				}

				if (vars::esp_bDebug)
				{
					Structs::DVector pLocalPos = vars::sLocalPlayer.TM.origin;
					const auto& entities = vars::vAllEntities;	//	copy entities
					for (auto ent : entities)
					{
						//	if (!Helpers::IsValidEntity(ent))
						//		continue;
				
						if (ent == pLocalPlayer || ent == pLocalShip)
							continue;
				
						if (ent->pName == nullptr)
							continue;
				
						Structs::DVector pos;
						if (!ent->vf_GetWorldPos(&pos))
							continue;
				
						float dist = pLocalPos.Distance(pos);
						if (dist > vars::esp_DebugRange)
							continue;

						//	draw text
						Thread::CanvasDrawTextf(
							__("%s %s"),								//	fmt
							fillThreadID, 								//	canvas index
							pos, 										//	world position
							vars::esp_DebugColor, 						//	color
							false,
							Helpers::FormatDistance(dist).c_str(), 		//	fmt #1
							(char*)ent->pName							//	fmt #2
						);
					}
				}
				
				/* render target name tag */
				if (vars::target_player.bValid && vars::target_player.bRender)
				{
					Structs::DVector pLocalPos;
					if (pLocalPlayer->vf_GetWorldPos(&pLocalPos))
					{
				
						Structs::DVector pos = vars::target_player.TM.origin;
						float dist = pLocalPos.Distance(pos);
				
						const auto& pRenderer = reinterpret_cast<Classes::CRenderer*>(gEnv->pRenderer);
				
						float oout[3] = { 0 };
						int insaneCheck = 1;
						double in[3] = { pos.x, pos.y, pos.z };
						bool bOnScreen = pRenderer->ProjectToScreen(in[0], in[1], in[2], &oout[0], &oout[1], &oout[2], 0);
						Structs::DVector res{ oout[0] * (Hooks::vars::szCanvas.x * 0.01f), oout[1] * (Hooks::vars::szCanvas.y * 0.01f), 0.5f };
						if (bOnScreen)
						{
							Thread::CanvasDrawText(
								__("SUCCESS"),
								fillThreadID,
								res,
								vars::target_Color,
								true
							);
						}
				
						Thread::CanvasDrawTextf(
							__("%s %s"),
							fillThreadID,
							pos,
							vars::target_Color,
							false,
							Helpers::FormatDistance(dist).c_str(),
							vars::target_player.mName.c_str()
						);
					}
				
					/* render forge control settings */
					if (vars::target_player.bForge)
					{
						const std::string& text = __("FORGE MODE: ON");
						const std::string& text2 = Helpers::FormatString(__("Target Distance: %.1f"), vars::target_controls.distance);
						const std::string& text3 = Helpers::FormatString(__("Target Height: %.1f"), vars::target_controls.height);
						const auto& szText = Helpers::CalcTextSize(text);
						const auto& szText2 = Helpers::CalcTextSize(text2);
						const auto& szText3 = Helpers::CalcTextSize(text3);
						pos_text = { canvas_bottom_center.x - (szText.x * .5f), canvas_bottom_center.y * .1f, 0.0f };
				
						Thread::CanvasDrawText(
							text.c_str(),								//	fmt
							fillThreadID, 								//	canvas index
							pos_text, 									//	canvas position
							text_color, 								//	color
							true
						);
						pos_text.x = canvas_bottom_center.x - (szText2.x * .5f);
						pos_text.y += text_height;
				
						Thread::CanvasDrawText(
							text2.c_str(),								//	fmt
							fillThreadID, 								//	canvas index
							pos_text, 									//	canvas position
							text_color, 								//	color
							true
						);
						pos_text.x = canvas_bottom_center.x - (szText3.x * .5f);
						pos_text.y += text_height;
						Thread::CanvasDrawText(
							text3.c_str(),								//	fmt
							fillThreadID, 								//	canvas index
							pos_text, 									//	canvas position
							text_color, 								//	color
							true
						);
				
					}
				}
				
				/* render waypoints */
				if (vars::wp_bRenderTags)
				{
					Structs::DVector pLocalPos;
					if (pLocalPlayer->vf_GetWorldPos(&pLocalPos))
					{
						/* render waypoint name tags */
						const auto waypoints = Hooks::vars::vWaypoints;	//	copy waypoints to avoid race conditions
						for (const auto& wp : waypoints)
						{
							if (!wp.bValid || !wp.bRender)
								continue;
				
							const Structs::DVector pos = wp.mWorldPos;
							float dist = pLocalPos.Distance(pos);
							Thread::CanvasDrawTextf(
								__("%s %s"),
								fillThreadID,
								pos,
								wp.mColor,
								false,
								Helpers::FormatDistance(dist).c_str(),
								wp.mName.c_str()
							);
						}
					}
				}
				
				/* render ufo movement speed */
				if (vars::cheat_bUFO)
				{
					const std::string& text = __("UFO MODE: ON");
					const std::string& text2 = Helpers::FormatString(__("Movement Speed: %.2f"), vars::mUFOSpeedScalar);
					const auto& szText = Helpers::CalcTextSize(text);
					const auto& szText2 = Helpers::CalcTextSize(text2);
					pos_text = { canvas_bottom_center.x - (szText.x * .5f), canvas_bottom_center.y * .75f, 0.0f };
				
					Thread::CanvasDrawText(
						text.c_str(),								//	fmt
						fillThreadID, 								//	canvas index
						pos_text, 									//	canvas position
						text_color, 								//	color
						true
					);
					pos_text.x = canvas_bottom_center.x - (szText2.x * .5f);
					pos_text.y += text_height;
				
					Thread::CanvasDrawText(
						text2.c_str(),								//	fmt
						fillThreadID, 								//	canvas index
						pos_text, 									//	canvas position
						text_color, 								//	color
						true
					);
				}
				
				/* render player walk speed */
				if (vars::cheat_bCustomWalkSpeed)
				{
					const std::string& text = Helpers::FormatString(__("Walk Speed: %.2f"), vars::mPlayerWalkSpeed);
					const auto& szText = Helpers::CalcTextSize(text);
					const ImVec2& canvas_top_right = ImVec2({ vars::szCanvas.x - szText.x, szText.y * 2 });
					pos_text = { canvas_top_right.x - (szText.x * .5f), canvas_top_right.y, 0.f };
				
					Thread::CanvasDrawText(
						text.c_str(),								//	fmt
						fillThreadID, 								//	canvas index
						pos_text, 									//	canvas position
						text_color, 								//	color
						true
					);
				}
				
				/* render mining laser fracture power */
				if (vars::cheat_bTunedMiningFractureBeam)
				{
					const std::string& text = Helpers::FormatString(__("Laser Power: %.2f"), vars::cheat_mining_FractureThrottle * 100.f);
					const auto& szText = Helpers::CalcTextSize(text);
					const ImVec2& canvas_top_right = ImVec2({ vars::szCanvas.x - szText.x, szText.y * 4 });
					pos_text = { canvas_top_right.x - (szText.x * .5f), canvas_top_right.y, 0.f };
				
					Thread::CanvasDrawText(
						text.c_str(),								//	fmt
						fillThreadID, 								//	canvas index
						pos_text, 									//	canvas position
						text_color, 								//	color
						true
					);
				}
			}

		}

		__int64 __fastcall C3DEngine_RenderWorld_hook(__int64 a1, __int64 a2, __int64 a3, __int64 a4)
		{
			const auto& result = Functions::C3DEngine_RenderWorld_stub(a1, a2, a3, a4);
			
			/* capture canvas dimensions */
			if ((vars::szCanvas.x <= 0.f && vars::szCanvas.y <= 0.f) || !gEnv->pRenderer)
				return result;

			const auto& fillThreadID = Functions::CRenderer_GetFillThreadID_stub(gEnv->pRenderer);

			const ImVec2& canvas_center = { vars::szCanvas.x * .5f, vars::szCanvas.y * .5f };
			const ImVec2& canvas_top_center = ImVec2({ canvas_center[0], 0.f });
			const ImVec2& canvas_bottom_center = ImVec2({ canvas_center[0], vars::szCanvas.y });
			const float& szFont = 10.0f;
			const float& text_height = szFont * 3;
			Structs::FQuat text_color = { 1.f, 1.f, 1.f, 1.f };

			/* render watermark */
			const std::string& text_watermark = __("OfflineCitizen DEBUG - This is an early test build and not indicative of features/settings intended for final production");
			const auto& szTextWatermark = Helpers::CalcTextSize(text_watermark, szFont);
			Structs::DVector pos_text = { canvas_bottom_center.x - (szTextWatermark.x * .5f), canvas_bottom_center.y - szTextWatermark.y, 0.0f };
			Thread::CanvasDrawText(
				text_watermark.c_str(),						//	fmt
				fillThreadID, 								//	canvas index
				pos_text, 									//	canvas position
				text_color, 								//	color
				true
			);


			C3DEngine_RenderWorld_helper(a1, fillThreadID, text_height, canvas_bottom_center, text_color);

			return result;
		}

		__int64 __fastcall CRenderer_MTUpdate_hook(__int64 a1, __int64 a2)
		{
			const auto& result = Functions::CRenderer_MTUpdate_stub(a1, a2);
			const auto& pRenderer = reinterpret_cast<Classes::CRenderer*>(a1);
			const auto& pLocalPlayer = Helpers::GetLocalPlayerEntity();

			///	/* relocated to CEntitySystem_Update_hook */
			/* update local player tm */
			//	Thread::LocalPlayerUpdate(pLocalPlayer);

			/* update target entity */
			Thread::TargetEntityUpdate();

			/* update waypoints */
			Thread::WaypointsUpdate(pLocalPlayer);

			/* update transforms */
			Thread::TransformsUpdate();

			/* update canvas size */
			int x = CallVFunction<int>(pRenderer, vft_MT_GetDisplayWidth / 8);
			int y = CallVFunction<int>(pRenderer, vft_MT_GetDisplayHeight / 8);	//	CRenderer::MT_GetDisplayWidth
			if (x > 0 && y > 0)
				vars::szCanvas = { (float)x, (float)y };

			/* do ufo mode */
			if (vars::cheat_bUFO)
			{
				if (vars::target_player.bValid && vars::target_player.bForge)
					vars::cheat_bUFO = false;
				else
					Thread::PLAYER_UFO(vars::mUFOSpeedScalar);
			}

			return result;
		}

		__int64 __fastcall CRenderer_DrawTextArgs_hook(void* a1, void* a2, void* a3, __int64 a4, const char* src, va_list args)
		{
			const auto& result = Functions::CRenderer_DrawTextArgs_stub(a1, a2, a3, a4, src, args);
			return result;
		}

		bool __fastcall CRenderer_ProjectToScreen_hook(void* a1, double x, double y, double z, float* pX, float* pY, float* pZ, __int64 pCamera)
		{
			const auto& result = Functions::CRenderer_ProjectToScreen_stub(a1, x, y, z, pX, pY, pZ, pCamera);
			return result;
		}

		__int64 CRenderer_FlushTextMessages_hook(void* a1, unsigned int a2)
		{
			const auto& pRenderer = reinterpret_cast<Classes::CRenderer*>(a1);

			if (Helpers::IsESPEnabled() && Helpers::GetLocalPlayerEntity() )
			{
				/* update screen TM */
				Thread::ScreenPointsUpdate(pRenderer);
			}

			return Functions::CRenderer_FlushTextMessages_stub(a1, a2);
		}

		__int64 CCameraViewManager_Update_hook(__int64 a1, __int64 a2, double a3)
		{
			gCamera = CleanPointer<Classes::CCamera*>(reinterpret_cast<Classes::CCamera*>(a2));
			
			return Functions::CCamerViewManager_Update_stub(a1, a2, a3);
		}

		__int64 __fastcall CEntitySystem_Update_hook(__int64 a1, __int64 a2, double _XMM2_8)
		{
			const auto& pEntitySystem = reinterpret_cast<Classes::CEntitySystem*>(a1);
			if (pEntitySystem)
			{
				const auto& pLocalPlayer = Helpers::GetLocalPlayerEntity();

				/* update local player tm */
				Thread::LocalPlayerUpdate(pLocalPlayer);

				/* update class registry */
				Thread::ClassesUpdate(pEntitySystem);

				/**/
				if (vars::cheat_rage_bGrabAllEnemies)
					Thread::rage_GrabAllPlayers(Hooks::vars::target_controls.distance);

				/**/
				if (vars::grab_pEntity)
				{
					if (vars::grab_bGoTo)
					{
						vars::grab_bGoTo = false;
						Thread::GoToEntity(vars::grab_pEntity, vars::grab_fDistance);
					}
					else if (vars::grab_bGrab)
					{
						vars::grab_bGrab = false;
						Thread::GrabEntity(vars::grab_pEntity, vars::grab_fDistance);
					}
					vars::grab_pEntity = nullptr;
				}
			}
			return Functions::CEntitySystem_Update_stub(a1, a2, _XMM2_8);
		}

		__int64* __fastcall CEntitySystem_SpawnEntity_hook(__int64 a1, __int64* a2, __int64* a3, __int64 a4)
		{
			// find orbiting container class

			const auto& result = Functions::CEntitySystem_SpawnEntity_stub(a1, a2, a3, a4);
			if (result && gEnv != nullptr)
			{
				[&]() 
				{
					static Classes::CEntityClass* pPlayerClass = nullptr;
					static Classes::CEntityClass* pOrbitingContainer = nullptr;
					static Classes::CEntityClass* pGoToPoint = nullptr;
					static Classes::CEntityClass* pMissionMarker = nullptr;
					
					const auto& pEntity = CleanPointer<Classes::CEntity*>(reinterpret_cast<Classes::CEntity*>(*result));
					if (!pEntity)
						return;

					const auto& pEntityClass = CleanPointer<Classes::CEntityClass*>(pEntity->pEntityClass);
					if (!pEntityClass)
						return;

					std::lock_guard<std::mutex> guard(vars::vEntitiesMutex);

					/* ALL ENTITIES */
					vars::vAllEntities.push_back(pEntity);
					
					/* PLAYER ENTITIES */
					if (Helpers::IsPlayerEntity(pEntity))
					{
						vars::vPlayerEntities.push_back(pEntity);
						vars::vScreenEntities.push_back({ pEntity, Enums::EEntityType::ET_PLAYER, Structs::STransforms(), Structs::SDrawEntity()});
						return; // early exit
					}

					/* ENEMY AI ENTITIES */
					if (Helpers::IsEnemyNPCEntity(pEntity))
					{
						vars::vEnemyAIEntities.push_back(pEntity);
						vars::vScreenEntities.push_back({ pEntity, Enums::EEntityType::ET_ENEMY_NPC, Structs::STransforms(), Structs::SDrawEntity() });
						return; // early exit
					}

					/* ANIMAL ENTITIES */
					if (Helpers::IsAnimalEntity(pEntity))
					{
						vars::vAnimalEntities.push_back(pEntity);
						vars::vScreenEntities.push_back({ pEntity, Enums::EEntityType::ET_ANIMAL, Structs::STransforms(), Structs::SDrawEntity() });
						return; // early exit
					}

					/* SHIP ENTITIES */
					if (Helpers::IsShipEntity(pEntity))
					{
						vars::vShipEntities.push_back(pEntity);
						vars::vScreenEntities.push_back({ pEntity, Enums::EEntityType::ET_SHIP, Structs::STransforms(), Structs::SDrawEntity() });
						return; // early exit
					}

					/* LOOT ENTITIES */
					if (Helpers::IsLootableEntity(pEntity))
					{
						vars::vLootEntities.push_back(pEntity);
						vars::vScreenEntities.push_back({ pEntity, Enums::EEntityType::ET_CONTAINER, Structs::STransforms(), Structs::SDrawEntity() });
						return; // early exit
					}

					/* ROCK ENTITIES */
					if (Helpers::IsMineableEntity(pEntity))
					{
						vars::vRockEntities.push_back(pEntity);
						vars::vScreenEntities.push_back({ pEntity, Enums::EEntityType::ET_ROCK, Structs::STransforms(), Structs::SDrawEntity() });
						return; // early exit
					}

					/* ORBITING CONTAINERS */
					if (Helpers::IsOrbitEntity(pEntity))
					{
						vars::vOrbitEntities.push_back(pEntity);
						//	vars::vScreenOrbit.push_back({ pEntity, {} });
						vars::vScreenEntities.push_back({ pEntity, Enums::EEntityType::ET_ORBIT, Structs::STransforms(), Structs::SDrawEntity() });
						return; // early exit
					}

					/* MISSION MARKERS */
					if (Helpers::IsMissionEntity(pEntity)) 
					{
						vars::vMissionEntities.push_back(pEntity);
						return; // early exit
					}

					/* GO TO ENTITIES */
					if (Helpers::IsGoToEntity(pEntity)) 
					{
						vars::vGoToEntities.push_back(pEntity);
						return; // early exit
					}

					/* KIOSK ENTITIES */
					if (Helpers::IsShoppingKioskEntity(pEntity))
					{
						vars::vKioskEntities.push_back(pEntity);
						return; // early exit
					}
				}();
			}
			return result;
		}

		__int64 __fastcall CEntitySystem_DeleteEntity_hook(__int64 a1, __int64* a2)
		{
			auto result = Functions::CEntitySystem_DeleteEntity_stub(a1, a2);
			if (result)
			{
				Classes::CEntity* pEntity = CleanPointer<Classes::CEntity*>(reinterpret_cast<Classes::CEntity*>(*a2));
				if (pEntity)
				{
					std::lock_guard<std::mutex> guard(vars::vEntitiesMutex);

					/* target entity is gone , reset to avoid a crash */
					if (pEntity == vars::target_player.pEntity)
					{
						vars::target_player.bValid = false;
					}

					/* players */
					auto it_player = std::find(vars::vPlayerEntities.begin(), vars::vPlayerEntities.end(), pEntity);
					if (it_player != vars::vPlayerEntities.end())
						vars::vPlayerEntities.erase(it_player);

					/* planets */
					auto it_orbit = std::find(vars::vOrbitEntities.begin(), vars::vOrbitEntities.end(), pEntity);
					if (it_orbit != vars::vOrbitEntities.end())
						vars::vOrbitEntities.erase(it_orbit);
				
					/* ships */
					auto it_ship = std::find(vars::vShipEntities.begin(), vars::vShipEntities.end(), pEntity);
					if (it_ship != vars::vShipEntities.end())
						vars::vShipEntities.erase(it_ship);

					/* mission markers */ 
					auto it_mission = std::find(vars::vMissionEntities.begin(), vars::vMissionEntities.end(), pEntity);
					if (it_mission != vars::vMissionEntities.end())
						vars::vMissionEntities.erase(it_mission);

					/* go to points */
					auto it_go = std::find(vars::vGoToEntities.begin(), vars::vGoToEntities.end(), pEntity);
					if (it_go != vars::vGoToEntities.end())
						vars::vGoToEntities.erase(it_go);

					/* loot */
					auto it_loot = std::find(vars::vLootEntities.begin(), vars::vLootEntities.end(), pEntity);
					if (it_loot != vars::vLootEntities.end())
						vars::vLootEntities.erase(it_loot);

					/* rocks */
					auto it_rock = std::find(vars::vRockEntities.begin(), vars::vRockEntities.end(), pEntity);
					if (it_rock != vars::vRockEntities.end())
						vars::vRockEntities.erase(it_rock);

					/* kiosks */
					auto it_kiosk = std::find(vars::vKioskEntities.begin(), vars::vKioskEntities.end(), pEntity);
					if (it_kiosk != vars::vKioskEntities.end())
						vars::vKioskEntities.erase(it_kiosk);

					/* animals */
					auto it_animal = std::find(vars::vAnimalEntities.begin(), vars::vAnimalEntities.end(), pEntity);
					if (it_animal != vars::vAnimalEntities.end())
						vars::vAnimalEntities.erase(it_animal);

					/* enemy npc */
					auto it_enemy = std::find(vars::vEnemyAIEntities.begin(), vars::vEnemyAIEntities.end(), pEntity);
					if (it_enemy != vars::vEnemyAIEntities.end())
						vars::vEnemyAIEntities.erase(it_enemy);


					/* orbit screen */
					//	auto it_screen_orbit = std::find_if(vars::vScreenOrbit.begin(), vars::vScreenOrbit.end(),
					//		[&](const auto& pair) { return pair.first == pEntity; });
					//	if (it_screen_orbit != vars::vScreenOrbit.end())
					//		vars::vScreenOrbit.erase(it_screen_orbit);

					/* render screen */
					auto it_screen = std::find_if(vars::vScreenEntities.begin(), vars::vScreenEntities.end(),
						[&](const Structs::SThreadEntity& x) { return x.pEntity == pEntity; });
					if (it_screen != vars::vScreenEntities.end())
						vars::vScreenEntities.erase(it_screen);

					auto it_all = std::find(vars::vAllEntities.begin(), vars::vAllEntities.end(), pEntity);
					if (it_all != vars::vAllEntities.end())
						vars::vAllEntities.erase(it_all);

				}
			}
			return result;
		}

		__int64 CEntity_Init_hook(__int64 a1, __int64 a2)
		{
			/*
			
			  if ( *(_BYTE *)(this + 13) )
			  {
			    sub_1405BD370(
			      "Entity %llu %s is being initialized from an unexpected state (%u)",
			      *(_QWORD *)(this + 0x10),
			      *(const char **)(this + 0x290),
			      *(unsigned __int8 *)(this + 13));
			  }
			
			*/


			auto detour = [](__int64 a1, __int64 a2)
			{
				static Classes::CEntityClass* pPlayerClass = nullptr;
				static Classes::CEntityClass* pOrbitingContainer = nullptr;
				static Classes::CEntityClass* pGoToPoint = nullptr;
				static Classes::CEntityClass* pMissionMarker = nullptr;

				const auto& pEntity = CleanPointer<Classes::CEntity*>(reinterpret_cast<Classes::CEntity*>(a1));
				if (!pEntity)
					return;

				const auto& pEntityClass = CleanPointer<Classes::CEntityClass*>(pEntity->pEntityClass);
				if (!pEntityClass)
					return;

				std::lock_guard<std::mutex> guard(vars::vEntitiesMutex);

				/* PLAYER ENTITIES */
				if (Helpers::IsPlayerEntity(pEntity))
				{
					vars::vPlayerEntities.push_back(pEntity);
					vars::vAllEntities.push_back(pEntity);
					vars::vScreenEntities.push_back({ pEntity, Enums::EEntityType::ET_PLAYER, Structs::STransforms(), Structs::SDrawEntity() });
					return; // early exit
				}

				/* ENEMY AI ENTITIES */
				if (Helpers::IsEnemyNPCEntity(pEntity))
				{
					vars::vEnemyAIEntities.push_back(pEntity);
					vars::vAllEntities.push_back(pEntity);
					vars::vScreenEntities.push_back({ pEntity, Enums::EEntityType::ET_ENEMY_NPC, Structs::STransforms(), Structs::SDrawEntity() });
					return; // early exit
				}

				/* ANIMAL ENTITIES */
				if (Helpers::IsAnimalEntity(pEntity))
				{
					vars::vAnimalEntities.push_back(pEntity);
					vars::vAllEntities.push_back(pEntity);
					vars::vScreenEntities.push_back({ pEntity, Enums::EEntityType::ET_ANIMAL, Structs::STransforms(), Structs::SDrawEntity() });
					return; // early exit
				}

				/* SHIP ENTITIES */
				if (Helpers::IsShipEntity(pEntity))
				{
					vars::vShipEntities.push_back(pEntity);
					vars::vAllEntities.push_back(pEntity);
					vars::vScreenEntities.push_back({ pEntity, Enums::EEntityType::ET_SHIP, Structs::STransforms(), Structs::SDrawEntity() });
					return; // early exit
				}

				/* LOOT ENTITIES */
				if (Helpers::IsLootableEntity(pEntity))
				{
					vars::vLootEntities.push_back(pEntity);
					vars::vAllEntities.push_back(pEntity);
					vars::vScreenEntities.push_back({ pEntity, Enums::EEntityType::ET_CONTAINER, Structs::STransforms(), Structs::SDrawEntity() });
					return; // early exit
				}

				/* ROCK ENTITIES */
				if (Helpers::IsMineableEntity(pEntity))
				{
					vars::vRockEntities.push_back(pEntity);
					vars::vAllEntities.push_back(pEntity);
					vars::vScreenEntities.push_back({ pEntity, Enums::EEntityType::ET_ROCK, Structs::STransforms(), Structs::SDrawEntity() });
					return; // early exit
				}

				/* ORBITING CONTAINERS */
				if (Helpers::IsOrbitEntity(pEntity))
				{
					vars::vOrbitEntities.push_back(pEntity);
					//	vars::vScreenOrbit.push_back({ pEntity, {} });
					vars::vAllEntities.push_back(pEntity);
					vars::vScreenEntities.push_back({ pEntity, Enums::EEntityType::ET_ORBIT, Structs::STransforms(), Structs::SDrawEntity() });
					return; // early exit
				}

				/* MISSION MARKERS */
				if (Helpers::IsMissionEntity(pEntity))
				{
					vars::vMissionEntities.push_back(pEntity);
					vars::vAllEntities.push_back(pEntity);
					return; // early exit
				}

				/* GO TO ENTITIES */
				if (Helpers::IsGoToEntity(pEntity))
				{
					vars::vGoToEntities.push_back(pEntity);
					vars::vAllEntities.push_back(pEntity);
					return; // early exit
				}

				/* KIOSK ENTITIES */
				if (Helpers::IsShoppingKioskEntity(pEntity))
				{
					vars::vKioskEntities.push_back(pEntity);
					vars::vAllEntities.push_back(pEntity);
					return; // early exit
				}

				//	ALL ENTS
				vars::vAllEntities.push_back(pEntity);
			};

			if (*(BYTE*)(a1 + 0xD) == 0)	//	check state is valid
				detour(a1, a2);

			return Functions::CEntity_Init_stub(a1, a2);
		}

		__int64 CEntity_Shutdown_hook(__int64 a1, __int64 a2, double a3)
		{
			auto detour = [](__int64 a1)
			{
				Classes::CEntity* pEntity = CleanPointer<Classes::CEntity*>(reinterpret_cast<Classes::CEntity*>(a1));
				if (!pEntity)
					return;

				std::lock_guard<std::mutex> guard(vars::vEntitiesMutex);

				/* target entity is gone , reset to avoid a crash */
				if (pEntity == vars::target_player.pEntity)
				{
					vars::target_player.bValid = false;
				}

				/* players */
				auto it_player = std::find(vars::vPlayerEntities.begin(), vars::vPlayerEntities.end(), pEntity);
				if (it_player != vars::vPlayerEntities.end())
					vars::vPlayerEntities.erase(it_player);

				/* planets */
				auto it_orbit = std::find(vars::vOrbitEntities.begin(), vars::vOrbitEntities.end(), pEntity);
				if (it_orbit != vars::vOrbitEntities.end())
					vars::vOrbitEntities.erase(it_orbit);

				/* ships */
				auto it_ship = std::find(vars::vShipEntities.begin(), vars::vShipEntities.end(), pEntity);
				if (it_ship != vars::vShipEntities.end())
					vars::vShipEntities.erase(it_ship);

				/* mission markers */
				auto it_mission = std::find(vars::vMissionEntities.begin(), vars::vMissionEntities.end(), pEntity);
				if (it_mission != vars::vMissionEntities.end())
					vars::vMissionEntities.erase(it_mission);

				/* go to points */
				auto it_go = std::find(vars::vGoToEntities.begin(), vars::vGoToEntities.end(), pEntity);
				if (it_go != vars::vGoToEntities.end())
					vars::vGoToEntities.erase(it_go);

				/* loot */
				auto it_loot = std::find(vars::vLootEntities.begin(), vars::vLootEntities.end(), pEntity);
				if (it_loot != vars::vLootEntities.end())
					vars::vLootEntities.erase(it_loot);

				/* rocks */
				auto it_rock = std::find(vars::vRockEntities.begin(), vars::vRockEntities.end(), pEntity);
				if (it_rock != vars::vRockEntities.end())
					vars::vRockEntities.erase(it_rock);

				/* kiosks */
				auto it_kiosk = std::find(vars::vKioskEntities.begin(), vars::vKioskEntities.end(), pEntity);
				if (it_kiosk != vars::vKioskEntities.end())
					vars::vKioskEntities.erase(it_kiosk);

				/* animals */
				auto it_animal = std::find(vars::vAnimalEntities.begin(), vars::vAnimalEntities.end(), pEntity);
				if (it_animal != vars::vAnimalEntities.end())
					vars::vAnimalEntities.erase(it_animal);

				/* enemy npc */
				auto it_enemy = std::find(vars::vEnemyAIEntities.begin(), vars::vEnemyAIEntities.end(), pEntity);
				if (it_enemy != vars::vEnemyAIEntities.end())
					vars::vEnemyAIEntities.erase(it_enemy);


				/* orbit screen */
				//	auto it_screen_orbit = std::find_if(vars::vScreenOrbit.begin(), vars::vScreenOrbit.end(),
				//		[&](const auto& pair) { return pair.first == pEntity; });
				//	if (it_screen_orbit != vars::vScreenOrbit.end())
				//		vars::vScreenOrbit.erase(it_screen_orbit);

				/* render screen */
				auto it_screen = std::find_if(vars::vScreenEntities.begin(), vars::vScreenEntities.end(),
					[&](const Structs::SThreadEntity& x) { return x.pEntity == pEntity; });
				if (it_screen != vars::vScreenEntities.end())
					vars::vScreenEntities.erase(it_screen);

				auto it_all = std::find(vars::vAllEntities.begin(), vars::vAllEntities.end(), pEntity);
				if (it_all != vars::vAllEntities.end())
					vars::vAllEntities.erase(it_all);

			};


			detour(a1);

			return Functions::CEntity_Shutdown_stub(a1, a2, a3);
		}

		__int64 CActorSystem_Init_hook(__int64 this_ptr, __int64 pSystem, __int64 pEntitySystem)
		{
			__int64 pActorSystem = Functions::CActorSystem_Init_stub(this_ptr, pSystem, pEntitySystem);
			
			if (!gActorSystem || gActorSystem != (Classes::CActorSystem*)pActorSystem)
			{
				gSystem = (Classes::CSystem*)pSystem;
				gEntitySystem = (Classes::CEntitySystem*)pEntitySystem;
				gActorSystem = (Classes::CActorSystem*)pActorSystem;
				printf("[*] obtained game framework\n\t- gSystem: 0x%p\n\t- gEntitySystem: 0x%p\n\t- gActorSystem: 0x%p\n", gSystem, gEntitySystem, gActorSystem);
			}
			return pActorSystem;
		}

		__int64 CActorSystem_AddActor_hook(__int64 a1, __int64 entityId, __int64 pActor)
		{
			const auto result = Functions::CActorSystem_AddActor_stub(a1, entityId, pActor);
		
			/* enumerate actor array and update our reference */
			auto* pActorSystem = reinterpret_cast<Classes::CActorSystem*>(a1);
			if (!pActorSystem)
				return result;

			/* setup dispatcher reference */
			std::vector<Classes::CSCActor*> vActors;
			StarCitizen::ActorEnumerator enumerator{};
			enumerator.m_pFunctor = &vActors;
			enumerator.m_pDispatcher = &StarCitizen::ActorDispatcher;
			
			/* enumerate actors */
			pActorSystem->ForEachActor(&enumerator);

			// lock container and swap with new actor list
			{
				std::lock_guard<std::mutex> lock(Hooks::vars::vActorsMutex);
				Hooks::vars::vAllActors.swap(vActors);
			} // release lock

			return result;
		}

		__int64 CActorSystem_RemoveActor_hook(__int64 a1, __int64 entityId)
		{
			const auto result = Functions::CActorSystem_RemoveActor_stub(a1, entityId);
			
			auto* pActorSystem = reinterpret_cast<Classes::CActorSystem*>(a1);
			if (!pActorSystem)
				return result;

			/* setup dispatcher reference */
			std::vector<Classes::CSCActor*> vActors;
			StarCitizen::ActorEnumerator enumerator{};
			enumerator.m_pFunctor = &vActors;
			enumerator.m_pDispatcher = &StarCitizen::ActorDispatcher;

			/* enumerate actors */
			pActorSystem->ForEachActor(&enumerator);

			// lock container and swap with new actor list
			{
				std::lock_guard<std::mutex> lock(Hooks::vars::vActorsMutex);
				Hooks::vars::vAllActors.swap(vActors);
			} // release lock

			return result;

		}

		bool __fastcall CCharacterStateHiearchy_VerifyState_hook(__int64* a1, __int64* a2, __int64 a3, double _XMM3_8, char a5, char a6)
		{
			vars::fly_pFlag = *(int**)(a3 + 0x88);
			if (vars::fly_bEnable)
				*vars::fly_pFlag = 2;

			return Functions::CCharacterStateHiearchy_VerifyState_stub(a1, a2, a3, _XMM3_8, a5, a6);
		}

		__int64* __fastcall CXConsole_RegisterIntCvars_hook(__int64 a1, const char* a2, unsigned long* a3, int a4, unsigned int a5, const char* a6, __int64 a7)
		{
			Structs::SXCvar cvar{};
			cvar.Name = a2;
			cvar.Description = a6;
			cvar.flagPTR = a3;
			cvar.cxClass = reinterpret_cast<void*>(a1);
			cvar.type = Structs::ECvarType::CVARTYPE_INT;
			vars::vConsoleVariables.push_back(cvar);

			return Functions::CXConsole_RegisterCvar_Int_stub(a1, a2, a3, a4, a5, a6, a7);
		}

		__int64* __fastcall CXConsole_RegisterFloatCvars_hook(__int64 a1, const char* a2, unsigned long* a3, float a4, unsigned int a5, const char* a6, __int64 a7)
		{
			Structs::SXCvar cvar{};
			cvar.Name = a2;
			cvar.Description = a6;
			cvar.flagPTR = a3;
			cvar.cxClass = reinterpret_cast<void*>(a1);
			cvar.type = Structs::ECvarType::CVARTYPE_FLOAT;
			vars::vConsoleVariables.push_back(cvar);

			return Functions::CXConsole_RegisterCvar_Float_stub(a1, a2, a3, a4, a5, a6, a7);
		}

		__int64* __fastcall CXConsole_RegisterInt64Cvars_hook(__int64 a1, const char* a2, unsigned long* a3, __int64 a4, unsigned int a5, const char* a6, __int64 a7)
		{
			Structs::SXCvar cvar{};
			cvar.Name = a2;
			cvar.Description = (const char*)a6;
			cvar.flagPTR = a3;
			cvar.cxClass = reinterpret_cast<void*>(a1);
			cvar.type = Structs::ECvarType::CVARTYPE_INT64;
			vars::vConsoleVariables.push_back(cvar);

			return Functions::CXConsole_RegisterCvar_Int64_stub(a1, a2, a3, a4, a5, a6, a7);
		}

		__int64* __fastcall CXConsole_RegisterStringCvars_hook(__int64 a1, const char* a2, unsigned long* a3, void* a4, unsigned int a5, const char* a6, __int64 a7)
		{
			Structs::SXCvar cvar{};
			cvar.Name = a2;
			cvar.Description = (const char*)a6;
			cvar.flagPTR = a3;
			cvar.cxClass = reinterpret_cast<void*>(a1);
			cvar.type = Structs::ECvarType::CVARTYPE_STRING;
			vars::vConsoleVariables.push_back(cvar);

			return Functions::CXConsole_RegisterCvar_String_stub(a1, a2, a3, a4, a5, a6, a7);
		}

		__int64 __fastcall CXConsole_AddCMD_hook(__int64 a1, const char* a2, __int64 a3, unsigned int a4, const char* a5, char a6)
		{
			Structs::SXCommand cmd{};
			cmd.Name = a2;
			cmd.Description = a5;
			cmd.flags = a4;
			cmd.fnPtr = reinterpret_cast<void*>(a3);
			cmd.cxClass = reinterpret_cast<void*>(a1);
			vars::vConsoleCommands.push_back(cmd);

			return Functions::CXConsole_AddCommand_stub(a1, a2, a3, a4, a5, a6);
		}

		__int64 __fastcall CDataCore_RegisterStruct_hook(void* a1, const char* a2, void* a3, void* a4, void* a5, void* a6, char a7, void* a8)
		{
			vars::vStructNames.push_back(a2);	//	capture struct name
			return Functions::CDataCore_RegisterStruct_stub(a1, a2, a3, a4, a5, a6, a7, a8);
		}

		bool __fastcall CEntityClassRegistry_RegisterClass_hook(__int64 a1, __int64* a2)
		{
			//	/*
			//		_R13 = a2;
			//		v123 = a1;
			//		Class = x__CEntityClassRegistry_FindClass(a1, a2[2]);
			//		if ( Class )
			//		{
			//		  sub_67C1050(
			//			"CEntityClassRegistry::RegisterClass failed, class with name %s already registered",
			//			*(const char **)(Class + 16));
			//		  return 0;
			//		}
			//	*/

			bool result = Functions::CEntityClassRegistry_RegisterClass_stub(a1, a2);

			if (result && a2 && vars::class_names_count < vars::class_names_max_count)
			{
				std::string res = (char*)a2[2];
				if (!res.empty())
				{
					//	vars::ClassNames.push_back(res.c_str());	//	capture class name
					vars::class_names[vars::class_names_count] = (char*)a2[2];
					vars::class_names_count++;
				}
			}

			return result;
		}

		__int64 __fastcall CRigidEntity_VerifyExistingContacts_hook(__int64 a1)
		{

			auto detour = [](__int64 a1)
			{

				int result = 0;
				if (*(int*)(a1 + 0x148) <= 0)
					return;

				int v8 = 0;
				do
				{
					auto v9 = *(__int64**)(*(INT64*)(a1 + 0x4C0) + v8);
					if (v9)
					{
						while (1)
						{
							if ((*((BYTE*)v9 + 0x94) & 0x40) == 0)	// collision check
								*((BYTE*)v9 + 0x94) |= 0x40;	// set flag

							*((DWORD*)v9 + 0x25) &= 0xFFDFFF3F;
							if ((*((DWORD*)v9 + 0x25) & 0x80000) != 0)
								break;
							v9 = (__int64*)*v9;
						}
					}
					result = (unsigned int)(result + 1);
					v8 += 0x8;
				
				} while (result < *(int*)(a1 + 0x148));


			};

			if (vars::cheat_bTunedTractorBeam && GetAsyncKeyState(VK_LMENU) & 0x8000)
				detour(a1);

			return Functions::CRigidEntity_VerifyExistingContacts_stub(a1);
		}

		__int64 __fastcall CSCAmmoContainerComponent_GetAmmoCount_hook(__int64 a1)
		{
			if (vars::cheat_bInfiniteAmmo && !vars::salvage_bInstantFillRate)
			{
				//	get current ammo count
				const auto& pAmmoContainer = reinterpret_cast<Classes::CSCAmmoContainerComponent*>(a1);
				if (pAmmoContainer)
				{
					const auto maxAmmo = pAmmoContainer->mAmmoMax;

					if (pAmmoContainer->mAmmoCurrent != maxAmmo)
						pAmmoContainer->mAmmoCurrent = maxAmmo;

					return maxAmmo;
				}
			}
			return Functions::CSCAmmoContainerComponent_GetAmmoCount_stub(a1);
		}
		
		char __fastcall CWeaponActionFireSingle_Update_hook(__int64 a1, double _XMM1_8, __int64 a3)
		{
			auto mWeapon = reinterpret_cast<Classes::CWeaponAction<Structs::SWeaponActionFireSingleParams>*>(a1);
			if (mWeapon)
			{
				[&]()
				{
					//	get local player
					Classes::CEntity* pLocalPlayer = Helpers::GetLocalPlayerEntity();
					if (!pLocalPlayer)
						return;

					//	get local ship
					Classes::CEntity* plocalShip = Helpers::GetLocalShipEntity();	//	this can be null as its only used as a comparison and never accessed directly.

					//	Get the weapon component
					const auto& pWeaponComponent = CleanPointer<Classes::CSCItemWeaponComponent*>(mWeapon->pWeapon);
					if (!pWeaponComponent)
						return;

					//	get the weapon entity
					const auto& pWeaponEntity = CleanPointer<Classes::CEntity*>(pWeaponComponent->pEntity);
					if (!pWeaponEntity)
						return;

					//	get the weapon parent entity
					const auto& pWp = CleanPointer<Classes::CEntity*>(pWeaponEntity->pParentEntity);
					if (!pWp)
						return;

					//	get the weapon owner ( should be our local player or ship )
					const auto pWeaponOwner = CleanPointer<Classes::CEntity*>(pWp->pParentEntity);
					if (!pWeaponOwner)
						return;

					if (pWeaponOwner != pLocalPlayer && pWeaponOwner != plocalShip)
						return;

					const auto& pWeaponFireParams = CleanPointer<Classes::SWeaponActionFireSingleParams*>(mWeapon->pWeaponActionParams);		//	weapon action params
					if (!pWeaponFireParams)
						return;


					const auto& pLaunchParams = CleanPointer<Structs::SProjectileLauncher*>(pWeaponFireParams->p_launchParams);		//	projectile params
					if (!pLaunchParams)
						return;

					/* edit params */
					auto& mSpreadParams = pLaunchParams->spreadParams;		//	spread params

					if (vars::cheat_bInfiniteAmmo)
					{
						//	pWeaponFireParams->heatPerShot = 0.f;	//	set heat per shot to 0

						//	if (!vars::bInstantSalvage)
						//		pLaunchParams->ammoCost = 0.0f;
					}

					if (vars::cheat_bNoSpread)
					{
						mSpreadParams.min = 0.0f;
						mSpreadParams.max = 0.0f;
						mSpreadParams.decay = 0.0f;
					}

					if (vars::cheat_bDamageMultiplier)
						pLaunchParams->damageMultiplier = vars::mDamageMPScalar;


					/// @TODO
					//	//	DATA CACHE ENTRY
					//	SWeaponActionFireDefaults cache_entry;
					//	cache_entry.mWeaponType = EFIRESINGLE;												//	WeaponAction Type
					//	cache_entry.mWeaponName = std::string(__("[SINGLE] ")).append(weapon_entity->GetName());	//	weapon name
					//	cache_entry.pAddr = reinterpret_cast<__int64>(mFireParams);							//	Params Address
					//	
					//	bool bFound{ false };
					//	DWORD index = -1;
					//	for (auto SWeaponCacheEntry : g_Engine->sGameData.sWeaponActionFireDefaultsCache)
					//	{
					//		index++;
					//		if (SWeaponCacheEntry.mWeaponType != EFIRESINGLE)
					//			continue;
					//	
					//		if (SWeaponCacheEntry.pAddr == (__int64)mFireParams)
					//			bFound = true;
					//	
					//		//	Some wepaons have the same pointer but different names
					//		if (SWeaponCacheEntry.mWeaponName == cache_entry.mWeaponName)
					//			bFound = true;
					//	}
					//	
					//	if (!bFound)
					//	{
					//		cache_entry.pWeaponParams = mFireParams;												//	store pointer to params
					//		cache_entry.GetWeaponDefaultParams();													//	store default params
					//		g_Engine->sGameData.sWeaponActionFireDefaultsCache.push_back(cache_entry);				//	store new entry
					//	}

				}();
			}

			return Functions::CWeaponActionFireSingle_Update_stub(a1, _XMM1_8, a3);
		}

		char __fastcall CWeaponActionFireBurst_Update_hook(__int64 a1, __int64 a2, __int64 a3, __int64 a4)
		{
			auto mWeapon = reinterpret_cast<Classes::CWeaponAction<Structs::SWeaponActionFireSingleParams>*>(a1);
			if (mWeapon)
			{
				[&]()
				{
					//	get local player
					Classes::CEntity* pLocalPlayer = Helpers::GetLocalPlayerEntity();
					if (!pLocalPlayer)
						return;

					//	get local ship
					Classes::CEntity* plocalShip = Helpers::GetLocalShipEntity();	//	this can be null as its only used as a comparison and never accessed directly.

					//	Get the weapon component
					const auto& pWeaponComponent = CleanPointer<Classes::CSCItemWeaponComponent*>(mWeapon->pWeapon);
					if (!pWeaponComponent)
						return;

					//	get the weapon entity
					const auto& pWeaponEntity = CleanPointer<Classes::CEntity*>(pWeaponComponent->pEntity);
					if (!pWeaponEntity)
						return;

					//	get the weapon parent entity
					const auto& pWp = CleanPointer<Classes::CEntity*>(pWeaponEntity->pParentEntity);
					if (!pWp)
						return;

					//	get the weapon owner ( should be our local player or ship )
					const auto pWeaponOwner = CleanPointer<Classes::CEntity*>(pWp->pParentEntity);
					if (!pWeaponOwner)
						return;

					if (pWeaponOwner != pLocalPlayer && pWeaponOwner != plocalShip)
						return;

					const auto& pWeaponFireParams = CleanPointer<Classes::SWeaponActionFireSingleParams*>(mWeapon->pWeaponActionParams);		//	weapon action params
					if (!pWeaponFireParams)
						return;
					const auto& pLaunchParams = CleanPointer<Structs::SProjectileLauncher*>(pWeaponFireParams->p_launchParams);		//	projectile params
					auto& mSpreadParams = pLaunchParams->spreadParams;		//	spread params



					if (vars::cheat_bInfiniteAmmo)
					{
						//	pWeaponFireParams->heatPerShot = 0.f;	//	set heat per shot to 0

						//	if (!vars::bInstantSalvage)
						//		pLaunchParams->ammoCost = 0.0f;
					}

					if (vars::cheat_bNoSpread)
					{
						mSpreadParams.min = 0.0f;
						mSpreadParams.max = 0.0f;
						mSpreadParams.decay = 0.0f;
					}

					if (vars::cheat_bDamageMultiplier)
						pLaunchParams->damageMultiplier = vars::mDamageMPScalar;


					/// @TODO
					//	//	DATA CACHE ENTRY
					//	SWeaponActionFireDefaults cache_entry;
					//	cache_entry.mWeaponType = EFIRESINGLE;												//	WeaponAction Type
					//	cache_entry.mWeaponName = std::string(__("[SINGLE] ")).append(weapon_entity->GetName());	//	weapon name
					//	cache_entry.pAddr = reinterpret_cast<__int64>(mFireParams);							//	Params Address
					//	
					//	bool bFound{ false };
					//	DWORD index = -1;
					//	for (auto SWeaponCacheEntry : g_Engine->sGameData.sWeaponActionFireDefaultsCache)
					//	{
					//		index++;
					//		if (SWeaponCacheEntry.mWeaponType != EFIRESINGLE)
					//			continue;
					//	
					//		if (SWeaponCacheEntry.pAddr == (__int64)mFireParams)
					//			bFound = true;
					//	
					//		//	Some wepaons have the same pointer but different names
					//		if (SWeaponCacheEntry.mWeaponName == cache_entry.mWeaponName)
					//			bFound = true;
					//	}
					//	
					//	if (!bFound)
					//	{
					//		cache_entry.pWeaponParams = mFireParams;												//	store pointer to params
					//		cache_entry.GetWeaponDefaultParams();													//	store default params
					//		g_Engine->sGameData.sWeaponActionFireDefaultsCache.push_back(cache_entry);				//	store new entry
					//	}

				}();
			}

			return Functions::CWeaponActionFireBurst_Update_stub(a1, a3, a3, a4);
		}

		char __fastcall CWeaponActionFireRapid_Update_hook(__int64 a1, double _XMM1_8, __int64 a3, __int64 a4)
		{
			auto mWeapon = reinterpret_cast<Classes::CWeaponAction<Structs::SWeaponActionFireSingleParams>*>(a1);
			if (mWeapon)
			{
				[&]()
				{
					//	get local player
					Classes::CEntity* pLocalPlayer = Helpers::GetLocalPlayerEntity();
					if (!pLocalPlayer)
						return;

					//	get local ship
					Classes::CEntity* plocalShip = Helpers::GetLocalShipEntity();	//	this can be null as its only used as a comparison and never accessed directly.

					//	Get the weapon component
					const auto& pWeaponComponent = CleanPointer<Classes::CSCItemWeaponComponent*>(mWeapon->pWeapon);
					if (!pWeaponComponent)
						return;

					//	get the weapon entity
					const auto& pWeaponEntity = CleanPointer<Classes::CEntity*>(pWeaponComponent->pEntity);
					if (!pWeaponEntity)
						return;

					//	get the weapon parent entity
					const auto& pWp = CleanPointer<Classes::CEntity*>(pWeaponEntity->pParentEntity);
					if (!pWp)
						return;

					//	get the weapon owner ( should be our local player or ship )
					const auto pWeaponOwner = CleanPointer<Classes::CEntity*>(pWp->pParentEntity);
					if (!pWeaponOwner)
						return;

					if (pWeaponOwner != pLocalPlayer && pWeaponOwner != plocalShip)
						return;

					const auto& pWeaponFireParams = CleanPointer<Classes::SWeaponActionFireSingleParams*>(mWeapon->pWeaponActionParams);		//	weapon action params
					if (!pWeaponFireParams)
						return;
					const auto& pLaunchParams = CleanPointer<Structs::SProjectileLauncher*>(pWeaponFireParams->p_launchParams);		//	projectile params
					auto& mSpreadParams = pLaunchParams->spreadParams;		//	spread params

					if (vars::cheat_bInfiniteAmmo)
					{
						//	pWeaponFireParams->heatPerShot = 0.f;	//	set heat per shot to 0

						//	if (!vars::bInstantSalvage)
						//		pLaunchParams->ammoCost = 0.0f;
					}

					if (vars::cheat_bNoSpread)
					{
						mSpreadParams.min = 0.0f;
						mSpreadParams.max = 0.0f;
						mSpreadParams.decay = 0.0f;
					}

					if (vars::cheat_bDamageMultiplier)
						pLaunchParams->damageMultiplier = vars::mDamageMPScalar;


					/// @TODO
					//	//	DATA CACHE ENTRY
					//	SWeaponActionFireDefaults cache_entry;
					//	cache_entry.mWeaponType = EFIRESINGLE;												//	WeaponAction Type
					//	cache_entry.mWeaponName = std::string(__("[SINGLE] ")).append(weapon_entity->GetName());	//	weapon name
					//	cache_entry.pAddr = reinterpret_cast<__int64>(mFireParams);							//	Params Address
					//	
					//	bool bFound{ false };
					//	DWORD index = -1;
					//	for (auto SWeaponCacheEntry : g_Engine->sGameData.sWeaponActionFireDefaultsCache)
					//	{
					//		index++;
					//		if (SWeaponCacheEntry.mWeaponType != EFIRESINGLE)
					//			continue;
					//	
					//		if (SWeaponCacheEntry.pAddr == (__int64)mFireParams)
					//			bFound = true;
					//	
					//		//	Some wepaons have the same pointer but different names
					//		if (SWeaponCacheEntry.mWeaponName == cache_entry.mWeaponName)
					//			bFound = true;
					//	}
					//	
					//	if (!bFound)
					//	{
					//		cache_entry.pWeaponParams = mFireParams;												//	store pointer to params
					//		cache_entry.GetWeaponDefaultParams();													//	store default params
					//		g_Engine->sGameData.sWeaponActionFireDefaultsCache.push_back(cache_entry);				//	store new entry
					//	}

				}();
			}

			return Functions::CWeaponActionFireRapid_Update_stub(a1, _XMM1_8, a3, a4);
		}

		char __fastcall CWeaponActionFireTractorBeam_GetRayCastRequest_hook(__int64 a1, __int64 a2)
		{
			auto fn_TuneTractorBeam = [](const __int64& a1, const __int64& a2)
			{
				auto mWeapon = reinterpret_cast<Classes::CWeaponAction<Structs::SWeaponActionFireTractorBeamParams>*>(a1);
				if (!mWeapon)
					return;

				//	get local player
				Classes::CEntity* pLocalPlayer = Helpers::GetLocalPlayerEntity();
				if (!pLocalPlayer)
					return;

				//	get local ship
				Classes::CEntity* plocalShip = Helpers::GetLocalShipEntity();	//	this can be null as its only used as a comparison and never accessed directly.

				//	Get the weapon component
				const auto& pWeaponComponent = CleanPointer<Classes::CSCItemWeaponComponent*>(mWeapon->pWeapon);
				if (!pWeaponComponent)
					return;

				//	get the weapon entity
				const auto& pWeaponEntity = CleanPointer<Classes::CEntity*>(pWeaponComponent->pEntity);
				if (!pWeaponEntity)
					return;

				//	get the weapon parent entity
				const auto& pWp = CleanPointer<Classes::CEntity*>(pWeaponEntity->pParentEntity);
				if (!pWp)
					return;

				//	get the weapon owner ( should be our local player or ship )
				const auto pWeaponOwner = CleanPointer<Classes::CEntity*>(pWp->pParentEntity);
				if (!pWeaponOwner)
					return;

				///	unsure if needed
				//	const auto& pWeaponAmmoContainer = CleanPointer<Classes::CSCAmmoContainerComponent*>(pWeaponComponent->pAmmoContainer);

				if (pWeaponOwner != pLocalPlayer && pWeaponOwner != plocalShip)
					return;

				//	finally get & set salvage beam params
				const auto& mWeaponParams = mWeapon->pWeaponActionParams = reinterpret_cast<Structs::SWeaponActionFireTractorBeamParams*>(mWeapon->pWeaponActionParams);
				if (!mWeaponParams)
					return;

				/* cheats */

				if (vars::cheat_bTunedTractorBeam)
				{
					mWeaponParams->minForce = 0.f;
					mWeaponParams->maxForce = 1200000.f;
					mWeaponParams->maxDistance = 150.f;			
					mWeaponParams->fullStrengthDistance = 75.f;	
					mWeaponParams->maxAngle = 80.f;				
					mWeaponParams->maxVolume = 300000;			
					mWeaponParams->tetherBreakTime = 99.f;		
				}
			};

			fn_TuneTractorBeam(a1, a2);

			return Functions::CWeaponActionFireTractorBeam_GetRayCastRequest_stub(a1, a2);
		}

		char __fastcall CWeaponActionFireHealingBeam_GetRayCastRequest_hook(__int64 a1, __int64 a2, double a3)
		{
			auto mWeapon = reinterpret_cast<Classes::CWeaponAction<Structs::SWeaponActionFireHealingBeamParams>*>(a1);
			if (mWeapon && vars::cheat_bTunedMedicalBeam )
			{
				[&]()
				{
					//	get local player
					Classes::CEntity* pLocalPlayer = Helpers::GetLocalPlayerEntity();
					if (!pLocalPlayer)
						return;

					//	get local ship
					Classes::CEntity* plocalShip = Helpers::GetLocalShipEntity();	//	this can be null as its only used as a comparison and never accessed directly.

					//	Get the weapon component
					const auto& pWeaponComponent = CleanPointer<Classes::CSCItemWeaponComponent*>(mWeapon->pWeapon);
					if (!pWeaponComponent)
						return;

					//	get the weapon entity
					const auto& pWeaponEntity = CleanPointer<Classes::CEntity*>(pWeaponComponent->pEntity);
					if (!pWeaponEntity)
						return;

					//	get the weapon parent entity
					const auto& pWp = CleanPointer<Classes::CEntity*>(pWeaponEntity->pParentEntity);
					if (!pWp)
						return;

					//	get the weapon owner ( should be our local player or ship )
					const auto pWeaponOwner = CleanPointer<Classes::CEntity*>(pWp->pParentEntity);
					if (!pWeaponOwner)
						return;

					///	unsure if needed
					//	const auto& pWeaponAmmoContainer = CleanPointer<Classes::CSCAmmoContainerComponent*>(pWeaponComponent->pAmmoContainer);

					if (pWeaponOwner != pLocalPlayer && pWeaponOwner != plocalShip)
						return;

					//	finally get & set salvage beam params
					const auto& mWeaponParams = mWeapon->pWeaponActionParams = reinterpret_cast<Structs::SWeaponActionFireHealingBeamParams*>(mWeapon->pWeaponActionParams);
					if (!mWeaponParams)
						return;

					if (vars::cheat_bTunedMedicalBeam || vars::cheat_bEvilMedicalBeam)
					{
						float value = vars::cheat_bEvilMedicalBeam ? -1337.f : 1337.f;

						mWeaponParams->maxDistance = 150.f;			//	set max distance
						mWeaponParams->maxSensorDistance = 150.f;	//	set max sensor distance
						mWeaponParams->mSCUPerSec = value;
						mWeaponParams->ammoPerMSCU = 0.f;
						mWeaponParams->wearPerSec = 0.f;
					}
				}();
			}

			return Functions::CWeaponActionFireHealingBeam_GetRayCastRequest_stub(a1, a2, a3);
		}

		char __fastcall CWeaponActionFireSalvageRepair_GetRayCastRequest_hook(__int64 a1, __int64 a2, double a3)
		{
			auto detour = [](__int64 a1, __int64 a2, double a3) -> void
			{
				auto mWeapon = reinterpret_cast<Classes::CWeaponAction<Structs::SWeaponActionFireSalvageRepairParams>*>(a1);
				if (!mWeapon)
					return;

				//	get local player
				Classes::CEntity* pLocalPlayer = Helpers::GetLocalPlayerEntity();
				if (!pLocalPlayer)
					return;

				//	get local ship
				Classes::CEntity* plocalShip = Helpers::GetLocalShipEntity();	//	this can be null as its only used as a comparison and never accessed directly.

				//	Get the weapon component
				const auto& pWeaponComponent = CleanPointer<Classes::CSCItemWeaponComponent*>(mWeapon->pWeapon);
				if (!pWeaponComponent)
					return;

				//	get the weapon entity
				const auto& pWeaponEntity = CleanPointer<Classes::CEntity*>(pWeaponComponent->pEntity);
				if (!pWeaponEntity)
					return;

				//	get the weapon parent entity
				const auto& pWp = CleanPointer<Classes::CEntity*>(pWeaponEntity->pParentEntity);
				if (!pWp)
					return;

				//	get the weapon owner ( should be our local player or ship )
				const auto pWeaponOwner = CleanPointer<Classes::CEntity*>(pWp->pParentEntity);
				if (!pWeaponOwner)
					return;

				if (pWeaponOwner != pLocalPlayer && pWeaponOwner != plocalShip)
					return;

				//	finally get & set salvage beam params
				const auto& mWeaponParams = mWeapon->pWeaponActionParams = reinterpret_cast<Structs::SWeaponActionFireSalvageRepairParams*>(mWeapon->pWeaponActionParams);
				if (!mWeaponParams)
					return;

				/* set modification */
				mWeaponParams->materialEfficiency = 9999.f;	//	set material efficiency to 99999
			};

			if (vars::salvage_bInstantFillRate && !vars::cheat_bInfiniteAmmo)
				detour(a1, a2, a3);

			return Functions::CWeaponActionFireSalvageRepair_GetRayCastRequest_stub(a1, a2, a3);
		}

		static int _szItem = 0;
		static int _szContainer = 1000000000;
		char __fastcall CInventoryComponent_AddItem(__int64 _RCX, __int64 _RDX, int szContainer, int a4, int szItem, int a6)
		{
			const auto& result = Functions::CInventoryComponent_AddItem_stub(_RCX, _RDX, _szContainer, a4, _szItem, a6);
			return true;
		}

		__int64 __fastcall CSCLocalPlayerThoughComponent_OnInventoryMoveItemOntoUnoccupiedPosition_hook(__int64 a1, const void* a2, const void* a3, unsigned int a4, unsigned int a5, int a6)
		{

			/*	PSUEDOCODE
				CInventoryComponent::AddItem is called with v19 + 0x60 as the 3rd parameter for szContainer
				[v19 + 0x60 = szContainer]

				// REBUILD v19
				- v65 = (struct __crt_stdio_stream *)sub_1436B0E10(v62[0], a3);		//	fna
				- _lambdaa::_lambda_fna((_lambda_*)v47, v65);						//	*(_QWORD *)this = a2; return this;
				- v19 = v47[0] + 0x38;												//	
			*/


			auto fnb = [](__int64 a1, __int64 a2) -> bool
			{
				unsigned int v2; // eax

				if (*(__int64*)a1 >= *(__int64*)a2)
				{
					if (*(__int64*)a1 > *(__int64*)a2)
						return 0;
					v2 = *(DWORD*)(a1 + 8);
					if (v2 >= *(DWORD*)(a2 + 8))
					{
						if (v2 <= *(DWORD*)(a2 + 8))
							return *(__int64*)(a1 + 16) < *(__int64*)(a2 + 16);
						return 0;
					}
				}
				return 1;
			};

			auto fna = [fnb](__int64* a1, __int64 a2)
			{
				__int64* v2; // rdi
				__int64* v5; // rbx
				bool v6; // zf
				__int64* result; // rax

				v2 = (__int64*)*a1;
				v5 = *(__int64**)(*a1 + 0x8);

				if (!v5)
					return (__int64*)*a1;

				while (!*((bool*)v5 + 0x19))
				{

					if (fnb((__int64)(v5 + 4), a2))
					{
						v5 = (__int64*)v5[2];
					}
					else
					{
						v2 = v5;
						v5 = (__int64*)*v5;
					}
				}

				if (*((bool*)v2 + 0x19))                  // invalid = 1
					return (__int64*)*a1;                      // return origin
				
				v6 = !fnb(a2, (__int64)(v2 + 4));
				
				result = v2;
				
				if (!v6)
					return (__int64*)*a1;
				
				return result;
			};

			/* gain main pointer to container */
			__int64* v57 = (__int64*)(a1 + offset_InventoryComponent);
			if (v57 && vars::cheat_bMaxInventoryStorage)
			{
				__int64* v44;
				const auto& v60 = fna(v57, reinterpret_cast<__int64>(a3));
				v44 = v60;
				const auto& pInventory = __int64(v44) + 0x38;
				if (pInventory)
				{
					*(__int64*)(pInventory + 0x60) = _szContainer;	//	szContainer
					*(__int64*)(pInventory + 0x68) = _szItem;			//	szItem
				}
			}

			return Functions::CSCLocalPlayerThoughComponent_OnInventoryMoveItemOntoUnoccupiedPosition_stub(a1, a2, a3, a4, a5, a6);
		}

		__int64 __fastcall CSCActorGForce_Update_hook(__int64 a1, __int64 a2)
		{
			if (vars::cheat_bNoGForce)
				return 0;
	
			return Functions::CSCActorGForce_Update_stub(a1, a2);
		}

		struct CPlayerMovementComponent
		{
			unsigned char pad_0000[offset_PlayerMovementSpeed];
			float mSpeed;
		};
		float __fastcall CSCLocalPlayerMovementSpeed_stub_hook(__int64 a1)
		{
			auto result = StarCitizen::Functions::CSCLocalPlayerMovementSpeed_stub(a1);

			auto rax = reinterpret_cast<CPlayerMovementComponent*>(a1);
			if (rax && vars::cheat_bCustomWalkSpeed)	//	should only set if the flag is enabled
			{
				rax->mSpeed = StarCitizen::Hooks::vars::mPlayerWalkSpeed;
			}

			return result;
		}

		void __fastcall CXCommand_MegaMap_hook(__int64 a1)
		{
			return Functions::CXCommand_MegaMap_stub(a1);
		}

		void __fastcall CXCommand_LoadMegaMap_hook(__int64 a1, const char* a2, const char* a3)
		{
			static bool bFirstTime = true;

			/* Main Menu: Frontend_Main , SC_Frontend */
			if (bFirstTime && a2 && a3 && !strcmp(a2, __("Frontend_Main")) && !strcmp(a3, __("SC_Frontend")))
			{
				bFirstTime ^= 1; // only once
				return Functions::CXCommand_LoadMegaMap_stub(a1, __("PU"), __("SC_Default")); // Mode , Rules
			}

			return Functions::CXCommand_LoadMegaMap_stub(a1, a2, a3);
		}

		__int64 __fastcall LoadMap_hook(double _XMM0_8, __int64 a2)
		{
			return 0;
		}

		__int64 __fastcall AddToHitBox_hook(void* a1, __int64 a2, void* a3, double xmm1, __int8 a5, __int8 a6, void* a7, __int64 a8)
		{

			/*
			* _R8 = a3
			* set first float of r8 to 100.f
				_R12 = _R8;
			
				LABEL_17:
					v30 = _R12[0x13]; // _R8 + 0x98
					v31 = a8;
					__asm { vmovss  dword ptr [r12], xmm7 }	// storing value to health after hit
					if ( *(_BYTE *)(v30 + 0x2A) )
					// do other stuff
					
			
			
			*/


			/* @TODO: check if is local player or child of local entity */

			if (a3 && vars::cheat_bDemiGod)
			{
				auto pFloats = reinterpret_cast<float*>(a3);	//	convert to float pointer
				auto& max_health = pFloats[3];				//	should be 100.f

				pFloats[0] = max_health;	//	adjustment to not receive damage
				pFloats[1] = max_health;	//	adjustment to not receive damage

				return 0;	//	this result should be null to prevent damage / death event


				/* @TODO: unresolved tests */
				//	const auto& pLocalPlayer = Helpers::GetLocalPlayerEntity();
				//	if (pLocalPlayer && Helpers::GetEntityHealth(pLocalPlayer) <= 50.0f)
				//	{
				//	
				//		Helpers::SetEntityHealth(pLocalPlayer, max_health);
				//	
				//		pFloats[0] = max_health;	//	adjustment to not receive damage
				//		pFloats[1] = max_health;	//	adjustment to not receive damage
				//	
				//		return 0;	//	this result should be null to prevent damage / death event
				//	}
			}

			return Functions::AddToHitBox_stub(a1, a2, a3, xmm1, a5, a6, a7, a8);
		}

		__int64 __fastcall CSCItemQuantumDrive_Update_hook(__int64 a1, __int64 a2)
		{
			static auto InstantCalibrateDrive = [](StarCitizen::Classes::CSCItemQuantumDrive* p) -> void
			{
				using namespace StarCitizen::Enums;

				if (!p)
					return;

				const float ref = 0.1f; // displacement reference
				const float currentCalibration = p->mCalibrationLevel;
				const float maxCalibration = p->mCalibrationMax;
				const auto spoolState = p->mSpoolState;

				if (spoolState == ESPOOL_OFF || maxCalibration <= 0.0f || currentCalibration >= maxCalibration)
					return;

				if (spoolState == ESPOOL_SPOOLING)
					p->mSpoolChargeLevel = 1.0f - ref;

				if (!p->pTargetEntity || p->mCalibrationMax <= 0.0f)
					return;

				p->mCalibrationLevel = maxCalibration - ref;
			};

			static auto InstantWarp = [](StarCitizen::Classes::CSCItemQuantumDrive* p) -> void
			{
				using namespace StarCitizen::Enums;
				static clock_t timer = 0;
				static bool bShouldReset = false;
				clock_t now = clock();

				if (!p || !p->pTravelEnvelope)
				{
					timer = 0;
					bShouldReset = false;
					return;
				}

				if (!bShouldReset)
				{
					p->mJumpState = EJUMP_COMPLETE;
					
					timer == 0 ? timer = now : bShouldReset = now - timer > 2000;
					
					return;
				}

				p->mJumpState = EJUMP_FINISHED;
				if (now - timer <= 3500)
					return;

				p->pTravelEnvelope = nullptr;
				bShouldReset = false;
				timer = 0;
				StarCitizen::Hooks::vars::cheat_bInstantWarp = false;
			};

			auto result = StarCitizen::Functions::CSCItemQuantumDrive_Update_stub(a1, a2);

			StarCitizen::Classes::CSCItemQuantumDrive* qDrive = reinterpret_cast<StarCitizen::Classes::CSCItemQuantumDrive*>(a1);
			//	if (StarCitizen::Hooks::vars::bInstantWarpCalibration)

			if (StarCitizen::Hooks::vars::cheat_bInstantWarp)
			{
				InstantCalibrateDrive(qDrive);

				InstantWarp(qDrive);
			}

			return result;
		}

		bool __fastcall CSCItemSalvageController_OnRaycastSubmit_hook(__int64 a1, __int64 a2, __int64 a3)
		{
			auto pController = reinterpret_cast<StarCitizen::Classes::CSCItemSalvageController*>(a1);
			if (pController && vars::salvage_bInstantFillRate)
			{
				auto pParams = pController->pControllerParams;
				if (pParams)
				{
					if (pParams->bValidCargoParams)
					{
						auto pCargoParams = pParams->pCargoParams;
						pCargoParams->boxFillingTimePerSCU = 0.1f;	//	set instant fill speed
						pCargoParams->minCargoBoxSize = vars::salvage_minSCUContainer;
						pCargoParams->maxCargoBoxSize = vars::salvage_maxSCUContainer;
					}

					if (pParams->bValidStructuralParams)
					{
						auto pStructuralParams = pParams->pStructuralParams;
						pStructuralParams->fractureTimePerRadiusMetre = 0.1f;	//	set instant fracture speed

						/* @TODO: */
						//	pStructuralParams->disintegrationSCUPerCubicMetre = 0.1f;	//	set instant disintegration scu size
						//	pStructuralParams->disintegrationTimePerRadiusMetre = 0.1f;	//	set instant disintegration speed
					}
				}
			}

			return Functions::CSCItemSalvageController_OnRaycastSubmit_stub(a1, a2, a3);
		}

		__int64 __fastcall CSCItemMiningController_UpdateLaserThrottle_hook(__int64 a1, double a2, char a3, unsigned __int8 a4, char a5)
		{
			const auto& detour = [](__int64 pController)
			{
				static const float& MAX_CHARGE = 99999.f;
				static const float& MIN_CHARGE = MAX_CHARGE * 0.f;

				const auto& pMiningController = reinterpret_cast<Classes::CSCItemMiningController*>(pController);
				if (!pMiningController)
					return;

				Structs::SEntityComponentMiningLaserParams* pLaserParams = pMiningController->pMiningLaserParams;
				if (!pLaserParams)
					return;

				if (vars::cheat_bTunedMiningFractureBeam)
				{
					const Structs::MiningLaserModifiers& modifiers = pLaserParams->miningLaserModifiers;

					/* set throttle adjustment speed */
					vars::cheat_mining_FractureThrottleLerp = pLaserParams->throttleLerpSpeed;
					
					/* set fracture power */
					pLaserParams->throttleMinimum = vars::cheat_mining_FractureThrottle;

					/* laser instability */
					auto& instability = modifiers.laserInstability;
					if (instability.bValid)
						instability.pAccessor->level = -MAX_CHARGE;

					/* laser resistance */
					auto& resistance = modifiers.resistanceModifier;
					if (resistance.bValid)
						resistance.pAccessor->level = -MAX_CHARGE;

					/* laser optimal charge */
					auto& optimalCharge = modifiers.optimalChargeWindowSizeModifier;
					if (optimalCharge.bValid)
						optimalCharge.pAccessor->level = MAX_CHARGE;

				}
				else if (pLaserParams->throttleMinimum != MIN_CHARGE)
					pLaserParams->throttleMinimum = MIN_CHARGE;
			};

			const auto& result = Functions::CSCItemMiningController_UpdateLaserThrottle_stub(a1, a2, a3, a4, a5);
			
			detour(a1);
			
			return result;
		}

		__int64 __fastcall CEntityComponentMineable_OnHitByMiningLaser_hook(__int64 a1, __int64 a2, double _XMM2_8, double _XMM3_8, __int64 a5, __int64 a6)
		{
			if (vars::cheat_mining_bAutoFracture)
			{
				a6 = 23;
				//	vars::cheat_mining_mFlag = true;
				//	vars::cheat_mining_pMineable = a1;
			}
			const auto& result = Functions::CEntityComponentMineable_OnHitByMiningLaser_stub(a1, a2, _XMM2_8, _XMM3_8, a5, a6);



			return result;
		}

		__int64 __fastcall FractureMineable_hook(__int64 a1, __int64 a2, __int64 a3)
		{
			/*
			* RCX = CSCItemMiningController
			* 
			*/
			printf("params: 0x%llx , 0x%llx , 0x%llx\n", a1, a2, a3);
			return Functions::FractureMineable_stub(a1, a2, a3);
		}

		struct _fn_OnHit
		{
			class Classes::CEntity* pShooter;	//0x0000
			class Classes::CEntity* pTarget;	//0x0008
			class Classes::CEntity* pWeapon;	//0x0010
			char pad_0018[24];	//0x0018
			class Classes::CEntity* pTarget_HitPart;	//0x0030
		};	//Size: 0x0038

		void __fastcall CGameRulesSCDamageHandling_OnHit_hook(__int64 a1, __int64* a2)
		{
			auto hitParams = reinterpret_cast<_fn_OnHit*>(a2);

			Classes::CEntity* pLocalEntity = Helpers::GetLocalPlayerEntity();
			if (pLocalEntity)
			{
				bool bShooterIsLocalPlayer = hitParams->pShooter == pLocalEntity;
				bool bTargetIsLocalPlayer = hitParams->pTarget == pLocalEntity;
				auto pShooter = CleanPointer<Classes::CEntity*>(hitParams->pShooter);
				auto pTarget = CleanPointer<Classes::CEntity*>(hitParams->pTarget);
				auto pTargetPart = CleanPointer<Classes::CEntity*>(hitParams->pTarget_HitPart);
				auto pWeapon = CleanPointer<Classes::CEntity*>(hitParams->pWeapon);

				if (bShooterIsLocalPlayer && vars::cheat_bBlameUser)
				{
					hitParams->pShooter = hitParams->pTarget;		//	blame shooter on self

					/* return result */
					Functions::CGameRulesSCDamageHandling_OnHit_stub(a1, a2);
					return;
				}

				if (bTargetIsLocalPlayer && vars::cheat_bMirrorForce)
				{
					hitParams->pTarget = hitParams->pShooter;		//	blame target on self

					/* return result */
					Functions::CGameRulesSCDamageHandling_OnHit_stub(a1, a2);
					return;
				}

				/* copy hit result */
				Structs::SHitResult hit;
				{
					hit.pShooter = (__int64)pShooter;
					hit.pEntity = (__int64)pTarget;
					hit.pEntity_HitPart = (__int64)pTargetPart;
					hit.pWeapon = (__int64)pWeapon;

					if (hit.pShooter && pShooter->pName)
						hit.mShooterEntityName = (char*)pShooter->pName;

					if (hit.pEntity && pTarget->pName)
						hit.mHitEntityName = (char*)pTarget->pName;

					if (hit.pEntity_HitPart && pTargetPart->pName)
						hit.mHitPartName = (char*)pTargetPart->pName;

					if (hit.pWeapon && pWeapon->pName)
						hit.mWeaponName = (char*)pWeapon->pName;

					vars::hit_lastHit = hit;
				}

				/* check if the shooter is the local player */
				if (bShooterIsLocalPlayer)
					vars::hit_lastHitByPlayer = hit;	//	 copy local player hit result
			}

			Functions::CGameRulesSCDamageHandling_OnHit_stub(a1, a2);
		}

		__int64 __fastcall CSCBodyHealthComponent_ApplyHealthChange_hook(unsigned __int64 a1, float a2, unsigned __int64 a3, unsigned __int16 a4)
		{
			const auto& pLocalPlayer = Helpers::GetLocalPlayerEntity();
			if (a3 == (__int64)pLocalPlayer)
				printf("ApplyHealthChange: 0x%llX , %.2f, 0x%llX , 0x%x\n", a1, a2, a3, a4);

			return Functions::CSCBodyHealthComponent_ApplyHealthChange(a1, a2, a3, a4);
		}

		__int64 __fastcall CSCBodyHealthComponent_ApplyDamageHealing_hook(unsigned __int64 a1, float a2, unsigned __int64 a3, unsigned __int16 a4)
		{
			const auto& pLocalPlayer = Helpers::GetLocalPlayerEntity();
			if (a3 == (__int64)pLocalPlayer)
				printf("ApplyDamageHealing: 0x%llX , %.2f, 0x%llX , 0x%x\n", a1, a2, a3, a4);

			return Functions::CSCBodyHealthComponent_ApplyDamageHealing(a1, a2, a3, a4);
		}

		__int64 __fastcall CActor_Kill_hook(__int64* a1, __int64 a2, __int64 a3)
		{
			return Functions::CActor_Kill_stub(a1, a2, a3);
		}

		float __fastcall ShipBoostMP_hook(__int64 a1, unsigned __int8 a2)
		{
			float boost = 1.f;
			const auto pBoost = (float*)(a1 + 0x170);
			if (vars::cheat_bShipBoostMP && pBoost)
				boost = 35.f;
			
			for (int i = 0; i < 2; i++)
				pBoost[i] = boost;
			
			return Functions::ShipBoostMP_stub(a1, a2);
		}

	}

	namespace Helpers
	{

		__int64 datacore::GetStructDataFields(const std::string& name, Structs::SDataField** outBuffer)
		{
			const auto& pEnv = gEnv;
			if (!pEnv)
				return 0;

			const auto& pData = pEnv->pDataCore;
			if (!pData)
				return 0;

			return CallVFunction<__int64>(pData, vft_DataCore_GetDataFields / 8, name.c_str(), outBuffer, 1);
		}

		__int64 datacore::GetStructInstance(const std::string& fmt)
		{
			__int64 System; // rax
			__int64 v2; // [rsp+30h] [rbp-48h]
			char v3[24]; // [rsp+50h] [rbp-28h] BYREF

			const auto& pEnv = gEnv;
			if (!pEnv)
				return 0;

			System = (__int64)pEnv->pSystem;
			if (!System)
				return 0;

			v2 = (*(__int64(__fastcall**)(__int64))(*(__int64*)System + 0x240))(System);
			return *(__int64*)(Functions::GetStructInstance_stub(
				v2,
				(__int64)v3,
				(__int64)fmt.c_str()) + 0x10
			);
		}

		bool ShowConsole(const bool& state) noexcept
		{
			//	if (!IsWindow(Hooks::vars::console_wndw))
			//		return false;

			/* update console handle */
			StarCitizen::Hooks::vars::console_wndw = GetConsoleWindow();					
			
			/* show or hide console */
			return ShowWindow(Hooks::vars::console_wndw, state ? SW_SHOW : SW_HIDE);
		}

		bool IsValidPtr(void* p)
		{
			__try
			{
				if (!p)
					return false;

				volatile auto v = *reinterpret_cast<char*>(p);
				
				return true;
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				return false;
			}
			return false;
		}

		bool GamePadGetKeyState(WORD vButton) noexcept
		{
			XINPUT_STATE state;
			ZeroMemory(&state, sizeof(XINPUT_STATE));
			if (XInputGetState(0, &state) == ERROR_SUCCESS && (state.Gamepad.wButtons & vButton) == vButton)
				return true;
			return false;
		}

		bool IsESPEnabled() noexcept
		{
			return (
				Hooks::vars::esp_bPlayer  ||
				Hooks::vars::esp_bEnemyAI ||
				Hooks::vars::esp_bAnimals ||	
				Hooks::vars::esp_bShip	  ||
				Hooks::vars::esp_bRock	  ||
				Hooks::vars::esp_bLoot	  ||
				Hooks::vars::esp_bOrbit	
			);
		}

		bool IsLocalPlayerSitting() noexcept
		{
			const auto& pLocalPlayer = GetLocalPlayerEntity();
			if (!pLocalPlayer)
				return false;

			return CleanPointer<Classes::CEntity*>(pLocalPlayer->pSeatEntity) != nullptr;
		}

		Classes::CEntity* GetLocalPlayerEntity() noexcept
		{
			const auto& pEnv = gEnv;
			if (!pEnv)
				return nullptr;

			const auto& pGame = CleanPointer<Classes::CGame*>(pEnv->pGame);
			if (!pGame)
				return nullptr;

			const auto& pLocalPlayer = CleanPointer<Classes::CSCPlayer*>(pGame->pLocalPlayer);
			if (!pLocalPlayer)
				return nullptr;

			return CleanPointer<Classes::CEntity*>(pLocalPlayer->pEntity);
		}

		Classes::CEntity* GetLocalShipEntity() noexcept
		{
			if (GetLocalMechaEntity())
				return nullptr;

			const auto& pLocalEntity = GetLocalPlayerEntity();
			if (!pLocalEntity)
				return nullptr;

			const auto& pSeatEntity = CleanPointer<Classes::CEntity*>(pLocalEntity->pSeatEntity);
			if (!pLocalEntity->pSeatEntity)
				return nullptr;

			const auto& pParentEntity = CleanPointer<Classes::CEntity*>(pLocalEntity->pParentEntity);
			if (!pParentEntity)
				return nullptr;

			if (pParentEntity != pSeatEntity)
				return nullptr;

			const auto& pShipEntity = CleanPointer<Classes::CEntity*>(pSeatEntity->pLocalZoneEntity);
			if (!pShipEntity)
				return nullptr;

			return pShipEntity;
		}

		Classes::CEntity* GetLocalMechaEntity() noexcept
		{
			const auto& pLocalEntity = GetLocalPlayerEntity();
			if (!pLocalEntity)
				return nullptr;

			const auto& pSeatEntity = CleanPointer<Classes::CEntity*>(pLocalEntity->pSeatEntity);
			if (!pLocalEntity->pSeatEntity)
				return nullptr;

			const auto& pParentEntity = CleanPointer<Classes::CEntity*>(pLocalEntity->pParentEntity);
			if (!pParentEntity)
				return nullptr;

			const auto& pUnk = CleanPointer<Classes::CEntity*>(pParentEntity->pSelfEntity);
			if (!pUnk || pUnk != pParentEntity)
				return nullptr;

			const auto& pSeatZoneEntity = CleanPointer<Classes::CEntity*>(pSeatEntity->pLocalZoneEntity);
			if (!pSeatZoneEntity)
				return nullptr;

			const auto& pGrandParentEntity = CleanPointer<Classes::CEntity*>(pSeatEntity->pParentEntity);
			if (!pGrandParentEntity)
				return nullptr;
			
			if (pGrandParentEntity != pSeatZoneEntity)
				return nullptr;

			return pParentEntity;
		}

		Classes::CEntity* GetLocalControlledEntity() noexcept
		{
			Classes::CEntity* pTargetEntity = nullptr;

			const auto& pLocalPlayer = Helpers::GetLocalPlayerEntity();
			if (!pLocalPlayer)
				return nullptr;

			const auto& pLocalShipEntity = Helpers::GetLocalShipEntity();
			if (pLocalShipEntity)
				pTargetEntity = pLocalShipEntity;

			const auto& pMechEntity = Helpers::GetLocalMechaEntity();
			if (pMechEntity)
				pTargetEntity = pMechEntity;

			if (!pTargetEntity)
				pTargetEntity = pLocalPlayer;

			return pTargetEntity;
		}

		///	NOTE: these are slow , always cache cvars that are looked for as they never change.
		bool GetCVar(const std::string& name, Structs::SXCvar* out) noexcept
		{
			const auto& pEnv = gEnv;
			if (!pEnv)
				return false;

			//	check hooked entries
			if (!Hooks::vars::vConsoleVariables.empty())
			{
				const auto& entries = Hooks::vars::vConsoleVariables;

				auto it = std::find_if(
					entries.begin(), 
					entries.end(), 
					[&](const Structs::SXCvar& entry) 
					{ 
						return name == std::string(entry.Name);
					}
				); 


				if (it != entries.end())
				{
					*out = *it;
					return true;
				}
			}
		
			//	default to using game function to find cvar
			const auto& pConsole = CleanPointer<StarCitizen::Classes::CXConsole*>(pEnv->pConsole);
			if (!pConsole)
				return false;
			
			const auto& fn = Functions::CXConsole_GetCVar_stub(__int64(pConsole), name.c_str());
			if (!fn)
				return false;

			const auto& pCvar = CleanPointer<StarCitizen::Classes::CXCVar*>(reinterpret_cast<StarCitizen::Classes::CXCVar*>(fn));
			if (!pCvar)
				return false;

			Structs::SXCvar cvar{};
			cvar.Name = pCvar->pName;
			cvar.Description = pCvar->pDescription;
			cvar.cxClass = pCvar->pConsole;
			cvar.flagPTR = pCvar->pValue;

			*out = cvar;

			return true;
		}

		bool GetCmd(const std::string& name, Structs::SXCommand* out) noexcept
		{
			const auto& pEnv = gEnv;
			if (!pEnv)
				return false;

			//	check hooked entries
			if (Hooks::vars::vConsoleCommands.empty())
				return false;

			const auto& entries = Hooks::vars::vConsoleCommands;
			auto it = std::find_if(
				entries.begin(),
				entries.end(),
				[&](const Structs::SXCommand& entry)
				{
					return name == std::string(entry.Name);
				}
			);

			if (it == entries.end())
				return false;

			*out = *it;
			return true;
		}

		bool GetPointByName(const char* pName, StarCitizen::Structs::DVector* pOut) noexcept
		{
			auto pGoTo = *(__int64*)gGoToMan;
			auto pSystem = gEnv;
			if (!pSystem)
				return false;

			auto pEntitySystem = gEnv ? gEnv->pEntitySystem : nullptr;
			if (!pEntitySystem)
				return false;

			// Retrieve go-to point
			auto pPoint = reinterpret_cast<StarCitizen::Classes::CGoToPoint*>(StarCitizen::Functions::CGoToPointManager_GetPointByName_stub(pGoTo, pName));
			if (!pPoint)
				return false;

			/* store the go to point */
			if (std::find(Hooks::vars::vGoToPoints.begin(), Hooks::vars::vGoToPoints.end(), pPoint) == Hooks::vars::vGoToPoints.end())
				Hooks::vars::vGoToPoints.push_back(pPoint);

			// Retrieve zone entity
			__int64 pPointZoneEntity;
			auto result = StarCitizen::Functions::CEntitySystem_GetEntityZoneByID_stub((__int64)pEntitySystem, (__int64)&pPointZoneEntity, pPoint->m_KeyIndex);
			if (!result)
				return false;

			//	Get World Position of Entity
			auto ent = reinterpret_cast<StarCitizen::Classes::CEntity*>(EXTRACT_LOWER_BYTES(pPointZoneEntity));
			if (!ent)
				return false;

			StarCitizen::Structs::DVector localPos{ pPoint->m_posX, pPoint->m_posY, pPoint->m_posZ };         //  point local position within the zone
			StarCitizen::Structs::DVector root;
			if (!ent->vf_GetWorldPos(&root))																//  Get world position of the entity					
				return false;
			StarCitizen::Structs::DVector WorldOrigin = localPos + root;										//  translate position

			// Retrieve entity angles and rotate point
			StarCitizen::Structs::FVector angles;
			if (!ent->vf_GetEularAngles(&angles))
				return false;


			// Call RotatePoint with FVector parameters
			StarCitizen::Structs::DVector tempOut;
			Math::RotatePoint(WorldOrigin, root, &angles.x, &tempOut);

			*pOut = tempOut;

			return true;
		}	
		
		StarCitizen::Classes::SGoToPointAZ* GetGoToPointsArray() noexcept
		{
			auto pGoTo = *(StarCitizen::Classes::CGoToPointManager**)gGoToMan;
			if (!pGoTo)
				return nullptr;

			return reinterpret_cast<StarCitizen::Structs::SGoToPointAZ*>(EXTRACT_LOWER_BYTES((__int64)pGoTo->pGoToPoints));
		}


		bool GetEntityClassByName(const char* pName, __int64* pOut) noexcept
		{
			const auto& pEnv = gEnv;
			if (!pEnv)
				return false;

			const auto& pEntSystem = pEnv->pEntitySystem;
			if (!pEntSystem)
				return false;
			
			static auto pEntityClassRegistry = pEntSystem->vf_GetEntityClassRegistry();
			if (!pEntityClassRegistry)
				return false;

			const auto& pEntityClass = pEntityClassRegistry->FindClass(pName);
			if (!pEntityClass)
			{
#if _DEBUG
				printf("failed to find entity class %s\n", pName);
#endif
				return false;
			}

			*pOut = __int64(pEntityClass);

			return true;
		}

		bool IsValidEntity(Classes::CEntity* pEntity) noexcept
		{
			if (!pEntity)
				return false;

			INT64 pRenderProxy[2];
			if (!Functions::GetRenderProxy_stub((unsigned long long*)pEntity, (unsigned long long*)pRenderProxy))
				return false;

			if (!Functions::IsValidRenderProxy_stub((unsigned long long*)pRenderProxy))
				return false;

			const auto& pRender = reinterpret_cast<Classes::CRenderProxy*>(pRenderProxy[0] & 0xFFFFFFFFFFFF);
			if (!pRender || !pRender->pEntity)
				return false;

			// addr: 2D43A20
			// __int64* __fastcall IComponent::GetComponent_CSCBodyHealthComponent(_QWORD * resulta, unsigned __int64 pEntityPtr)
			//	INT64 pCSCBodyHealth[2];
			//	reinterpret_cast<__int64(*)(unsigned __int64*, unsigned __int64)>(GetAddr(0x2D43A20))((unsigned __int64*)pCSCBodyHealth, (__int64)pEntity);
			//	if (!pCSCBodyHealth)
			//		return false;

			//	auto res = Helpers::GetEntityComponentByName(pEntity, "Actor");

			return Functions::IsValidEntity_stub((unsigned __int64*)&pRender->pEntity);

			//	const auto& pComponents = CleanPointer<Classes::IEntityComponents*>(pEntity->pComponents);
			//	if (!pComponents)
			//		return false;
			//	
			//	const auto& pRenderProxy = CleanPointer<Classes::CRenderProxy*>(pComponents->pRenderProxy);
			//	if (!pRenderProxy)
			//		return false;
			//	
			//	return Functions::IsValidEntity_stub((unsigned __int64*)&pRenderProxy->pEntity);
		}

		bool IsPlayerEntity(Classes::CEntity* pEntity) noexcept
		{
			if (!pEntity)
				return false;	

			const auto& pEntityClass = CleanPointer<Classes::CEntityClass*>(pEntity->pEntityClass);
			if (!pEntityClass)
				return false;

			const auto& pPlayerClass = Hooks::vars::cache_ClassNames.pPlayer;
			if (!pPlayerClass)
				return false;

			return pEntityClass == pPlayerClass;
		}

		bool IsEnemyNPCEntity(Classes::CEntity* pEntity) noexcept
		{
			if (!pEntity)
				return false;

			const auto& pEntityClass = CleanPointer<Classes::CEntityClass*>(pEntity->pEntityClass);
			if (!pEntityClass)
				return false;

			for (const auto& x : Hooks::vars::cache_ClassNames.vEnemyNPC)
			{
				if (pEntityClass == x.second)
					return true;
			}

			return false;
		}

		bool IsNPCEntity(Classes::CEntity* pEntity) noexcept
		{
			if (!pEntity)
				return false;

			const auto& pEntityClass = CleanPointer<Classes::CEntityClass*>(pEntity->pEntityClass);
			if (!pEntityClass)
				return false;

			for (const auto& x : Hooks::vars::cache_ClassNames.vNPC)
			{
				if (pEntityClass == x.second)
					return true;
			}

			return false;
		}

		bool IsAnimalEntity(Classes::CEntity* pEntity) noexcept
		{
			if (!pEntity)
				return false;

			const auto& pEntityClass = CleanPointer<Classes::CEntityClass*>(pEntity->pEntityClass);
			if (!pEntityClass)
				return false;

			for (const auto& x : Hooks::vars::cache_ClassNames.vAnimals)
			{
				if (pEntityClass == x.second)
					return true;
			}

			return false;
		}

		bool IsShipEntity(Classes::CEntity* pEntity) noexcept
		{
			if (!pEntity)
				return false;

			const auto& pEntityClass = CleanPointer<Classes::CEntityClass*>(pEntity->pEntityClass);
			if (!pEntityClass)
				return false;

			for (const auto& x : Hooks::vars::cache_ClassNames.vShips)
			{
				if (pEntityClass == x.second)
					return true;
			}

			return false;
		}

		bool IsMineableEntity(Classes::CEntity* pEntity) noexcept
		{
			if (!pEntity)
				return false;

			const auto& pEntityClass = CleanPointer<Classes::CEntityClass*>(pEntity->pEntityClass);
			if (!pEntityClass)
				return false;

			for (const auto& x : Hooks::vars::cache_ClassNames.vRocks)
			{
				if (pEntityClass == x.second)
					return true;
			}

			return false;
		}

		bool IsLootableEntity(Classes::CEntity* pEntity) noexcept
		{
			if (!pEntity)
				return false;

			const auto& pEntityClass = CleanPointer<Classes::CEntityClass*>(pEntity->pEntityClass);
			if (!pEntityClass)
				return false;

			for (const auto& x : Hooks::vars::cache_ClassNames.vContainers)
			{
				if (pEntityClass == x.second)
					return true;
			}

			return false;
		}

		bool IsMissionEntity(Classes::CEntity* pEntity) noexcept
		{
			if (!pEntity)
				return false;

			const auto& pEntityClass = CleanPointer<Classes::CEntityClass*>(pEntity->pEntityClass);
			if (!pEntityClass)
				return false;

			const auto& pMissionClass = Hooks::vars::cache_ClassNames.pMissionMarker;
			if (!pMissionClass)
				return false;

			return pEntityClass == pMissionClass;
		}

		bool IsOrbitEntity(Classes::CEntity* pEntity) noexcept
		{
			if (!pEntity)
				return false;

			const auto& pEntityClass = CleanPointer<Classes::CEntityClass*>(pEntity->pEntityClass);
			if (!pEntityClass)
				return false;

			const auto& pOrbitClass = Hooks::vars::cache_ClassNames.pOrbitingContainer;
			if (!pOrbitClass)
				return false;

			return pEntityClass == pOrbitClass;
		}

		bool IsGoToEntity(Classes::CEntity* pEntity) noexcept
		{
			if (!pEntity)
				return false;

			const auto& pEntityClass = CleanPointer<Classes::CEntityClass*>(pEntity->pEntityClass);
			if (!pEntityClass)
				return false;

			const auto& pGoToClass = Hooks::vars::cache_ClassNames.pGoToPoint;
			if (!pGoToClass)
				return false;

			return pEntityClass == pGoToClass;
		}

		bool IsShoppingKioskEntity(Classes::CEntity* pEntity) noexcept
		{
			if (!pEntity)
				return false;

			const auto& pEntityClass = CleanPointer<Classes::CEntityClass*>(pEntity->pEntityClass);
			if (!pEntityClass)
				return false;

			for (const auto& x : Hooks::vars::cache_ClassNames.vKiosks)
			{
				if (pEntityClass == x.second)
					return true;
			}

			return false;
		}

		Classes::CEntity* GetEntityShip(Classes::CEntity* pTarget)
		{
			if (!pTarget)
				return nullptr;

			const auto& pSeatEntity = CleanPointer<Classes::CEntity*>(pTarget->pSeatEntity);
			if (!pSeatEntity)
				return nullptr;

			const auto& pParentEntity = CleanPointer<Classes::CEntity*>(pTarget->pParentEntity);
			if (!pParentEntity)
				return nullptr;

			if (pParentEntity != pSeatEntity)
				return nullptr;

			const auto& pShipEntity = CleanPointer<Classes::CEntity*>(pSeatEntity->pLocalZoneEntity);
			if (!pShipEntity)
				return nullptr;

			return pShipEntity;
		}

		bool GetEntityBounds(Classes::CEntity* pTarget, Structs::STransforms* out)
		{
			if (IsValidEntity(pTarget) == false)
				return false;

			Structs::STransforms trans;
			trans.pEntity = pTarget;

			if (!pTarget->vf_GetWorldPos(&trans.origin))
				return false;

			if (trans.origin.IsValid())
				return false;

			if (!pTarget->vf_GetWorldRotation(&trans.rotation))
				return false;

			if (!pTarget->vf_GetEularAngles(&trans.angles))
				return false;

			if (!pTarget->vf_GetWorldForwardDir(&trans.direction))
				return false;

			///	@TODO: ensure bones are only obtained on player entities
			const auto pComponents = CleanPointer<Classes::IEntityComponents*>(pTarget->pComponents);
			if ( pComponents )
			{
				INT64 pRenderProxy_h[2];
				if (Functions::GetRenderProxy_stub((unsigned long long*)pTarget, (unsigned long long*)pRenderProxy_h)
					&& Functions::IsValidRenderProxy_stub((unsigned long long*)pRenderProxy_h))
				{
					const auto& pRenderProxy = reinterpret_cast<Classes::CRenderProxy*>(pRenderProxy_h[0] & 0xFFFFFFFFFFFF);
					//	const auto& pRenderProxy = CleanPointer<Classes::CRenderProxy*>(pComponents->pRenderProxy);
					const auto& pActorComponent = CleanPointer<Classes::CSCActorComponent*>(pComponents->pActorComponent);
					const bool& bIsPlayer = Helpers::IsPlayerEntity(pTarget);
					const bool& bIsShip = Helpers::IsShipEntity(pTarget);

					/* get bounds */
					if (pRenderProxy /* && pRenderProxy->bVisible*/)
					{
						//	float outAABB[6];
						//	bool result = Functions::CRenderProxy_GetLocalBounds_stub(pRenderProxy, outAABB);	// @TODO: crash on some entities
						//	if (result)
						//	{
						//		const auto m_min = Structs::FVector(outAABB[0], outAABB[1], outAABB[2]);
						//		const auto m_max = Structs::FVector(outAABB[3], outAABB[4], outAABB[5]);
						//		const auto& localBounds = Structs::AABB(m_min, m_max);
						//		trans.box = Structs::DBox(trans.origin + localBounds.mMin, trans.origin + localBounds.mMax);
						//	}
						const auto& localBounds = pRenderProxy->mBounds;
						trans.box = Structs::DBox(trans.origin + localBounds.mMin, trans.origin + localBounds.mMax);
					}

					/* get bone tm */
					if (bIsPlayer && pActorComponent && CleanPointer<Classes::CEntity*>(pActorComponent->pEntity) == pTarget)
					{

						///	LOCAL POINTS
						//	std::vector<StarCitizen::Structs::FVector> points;
						//	for (int i = 0; i < 23; i++)
						//	{
						//		Structs::FVector bone[3];
						//		if (!Functions::CActor_GetBoneTransform_stub(__int64(pActorComponent), &bone, (unsigned int)i))
						//			continue;
						//	
						//		points.push_back(Structs::FVector(bone[1].y, bone[1].z, bone[2].x));
						//	}

						std::vector<StarCitizen::Structs::DVector> points;
						for (int i = 0; i < 23; i++)
						{
							Structs::DVector point;
							if (!Helpers::GetEntityBoneWorldPosByID(pTarget, Enums::EBones(i), &point))
								continue;

							points.push_back(point);
						}
						trans.bones = points;
						trans.bValidBones = trans.bones.size() > 0;
						trans.bIsActor = true;
						trans.health = (Helpers::GetEntityHealth(pTarget) / 100.f) * 100.f;	//	health is in percent
					}
				}
			}

			*out = trans;

			return true;
		}

		bool GetEntityScreenPoints(Classes::CEntity* pTarget, Structs::STransforms& TM, Structs::SDrawEntity* out2D)
		{

			Structs::SDrawEntity screen2D;
			screen2D.pEntity = pTarget;

			if (!pTarget || !Helpers::IsValidEntity(pTarget))
			{
				*out2D = screen2D;
				return false;
			}

			Structs::FVector2D szWndw = Hooks::vars::szCanvas;
			Structs::DVector min = TM.box.m_min;
			Structs::DVector max = TM.box.m_max;
			const float& height = TM.box.GetHeight();
			Structs::DVector root = TM.origin;
			Structs::DVector origin = TM.box.GetCenter();
			Structs::DVector head = { root.x, root.y, root.z + height };
			Structs::DVector fwd = root + (TM.direction * (TM.box.GetWidth() * .5f));

			screen2D.pEntity = pTarget;
			screen2D.bOrigin = Thread::WorldToScreen(TM.origin, szWndw, &screen2D.origin, true);
			screen2D.bOriginCenter = Thread::WorldToScreen(origin, szWndw, &screen2D.originCenter, true);
			screen2D.bOriginTop = Thread::WorldToScreen(head, szWndw, &screen2D.originTop, true);
			screen2D.bOriginFWD = Thread::WorldToScreen(fwd, szWndw, &screen2D.originFWD, true);

			/* entity probably is not on screen */
			if (!screen2D.bOrigin && !screen2D.bOriginCenter && !screen2D.bOriginTop && !screen2D.bOriginFWD)
			{
				*out2D = screen2D;
				return false;
			}

			Structs::DVector boxVerts[8] =
			{
				{ TM.box.m_min.x, TM.box.m_min.y, TM.box.m_min.z },	//	vert0	//		   7--------------6
				{ TM.box.m_max.x, TM.box.m_min.y, TM.box.m_min.z },	//	vert1	//	   	 / |            / |
				{ TM.box.m_max.x, TM.box.m_min.y, TM.box.m_max.z },	//	vert2	//	    3--------------2  |
				{ TM.box.m_min.x, TM.box.m_min.y, TM.box.m_max.z },	//	vert3	//	    |  |           |  |
				{ TM.box.m_min.x, TM.box.m_max.y, TM.box.m_min.z },	//	vert5	//	    |  4-----------|--5
				{ TM.box.m_max.x, TM.box.m_max.y, TM.box.m_min.z },	//	vert6	//	    | /            | /
				{ TM.box.m_max.x, TM.box.m_max.y, TM.box.m_max.z },	//	vert7	//	    |/             |/
				{ TM.box.m_min.x, TM.box.m_max.y, TM.box.m_max.z },	//	vert4	//	    0--------------1
			};

			Math::RotateBoxVerts(boxVerts, TM.origin, TM.angles, boxVerts);
			for (int i = 0; i < 8; i++)
				screen2D.bBoxVerts[i] = Thread::WorldToScreen(boxVerts[i], szWndw, &screen2D.boxVerts[i], true);

			if (TM.bIsActor)
			{
				std::vector<Structs::FQuat> bone_points;
				for (auto& boneID : Hooks::vars::BoneVector)
				{
					Structs::DVector xPoint, yPoint;
					for (int i = 0; i < boneID.size(); i++)
					{
						const auto& index = boneID.at(i);
						if (TM.bones.size() <= index)
							continue;

						xPoint = TM.bones[index];

						Structs::FVector2D xScreen;
						if (!Thread::WorldToScreen(xPoint, szWndw, &xScreen, true))
							continue;

						if (yPoint.IsValid())
						{
							yPoint = xPoint;
							continue;
						}

						Structs::FVector2D yScreen;
						if (!Thread::WorldToScreen(yPoint, szWndw, &yScreen, true))
							continue;

						bone_points.push_back(Structs::FQuat(xScreen.x, xScreen.y, yScreen.x, yScreen.y));

						yPoint = xPoint;
					}
					screen2D.bIsActor = true;
				}

				screen2D.bones = bone_points;


				if (TM.bones.size() >= Enums::EBones::head)
				{
					Structs::DVector headBone = TM.bones[Enums::EBones::head];
					Structs::DVector head_fwd = headBone + (TM.direction * (TM.box.GetWidth() * .5f));
					Structs::FVector2D screenHead;
					if (Thread::WorldToScreen(headBone, szWndw, &screenHead, true))
					{
						screen2D.bBoneHead = true;
						screen2D.boneHeadRadius = 100.f / float(gCamera->WorldPosition.Distance(root));
						screen2D.boneHead = screenHead;
						screen2D.bBoneHeadFWD = Thread::WorldToScreen(head_fwd, szWndw, &screen2D.boneHeadFWD, true);
					}
				}
				//	pTarget->GetBoneLocation(EBones::l_eye, &head_bone);
				//	OfflineCitizenRenderer::WorldToScreen(head_bone, &screen2D.bonesEyes[0], _size, true);
				//	
				//	pTarget->GetBoneLocation(EBones::r_eye, &head_bone);
				//	OfflineCitizenRenderer::WorldToScreen(head_bone, &screen2D.bonesEyes[1], _size, true);



				//	TM.health = Helpers::GetEntityHealthPercent(pTarget);	// wrong thread ??
			}

			screen2D.bValid = true;

			*out2D = screen2D;

			return true;
		}

		bool GetEntityBoneLocalPosByID(Classes::CEntity* pEntity, const StarCitizen::Enums::EBones& ID, Structs::FVector* out)
		{
			if (!pEntity)
				return false;

			const auto& pComponents = CleanPointer<Classes::IEntityComponents*>(pEntity->pComponents);
			if (!pComponents)
				return false;

			const auto& pActorComponent = CleanPointer<Classes::CSCActorComponent*>(pComponents->pActorComponent);
			if (!pActorComponent || !pActorComponent->pHealthComponent)
				return false;

			// check if valid bone index
			//	if (!Functions::CActor_HasBoneID_stub(__int64(pActorComponent), (unsigned int)ID))
			//		return false;

			Structs::FVector bone[3];
			if (!Functions::CActor_GetBoneTransform_stub(__int64(pActorComponent), &bone, (unsigned int)ID))
				return false;

			*out = Structs::FVector(bone[1].y, bone[1].z, bone[2].x);

			return true;
		}

		bool GetEntityBoneWorldPosByID(Classes::CEntity* pEntity, const StarCitizen::Enums::EBones& ID, Structs::DVector* out)
		{
			Structs::FVector boneLocalPos;
			if (!pEntity || !GetEntityBoneLocalPosByID(pEntity, ID, &boneLocalPos))
				return false;

			Structs::DVector origin;
			pEntity->vf_GetWorldPos(&origin);

			Structs::DVector result = origin + boneLocalPos;

			Structs::FVector angles;
			pEntity->vf_GetEularAngles(&angles);

			Math::RotatePoint(result, origin, &angles.x, out);

			return true;
		}

		float GetEntityHealth(Classes::CEntity* pEntity) noexcept
		{
			static const float& result = -1.0f;
			if (!pEntity)
				return result;

			const auto& pComponents = CleanPointer<Classes::IEntityComponents*>(pEntity->pComponents);
			if (!pComponents)
				return result;

			const auto& pActorComponent = CleanPointer<Classes::CSCActorComponent*>(pComponents->pActorComponent);
			if (!pActorComponent)
				return result;

			return CallVFunction<float>(pActorComponent, vft_Actor_GetHealth / 8);
		}

		float GetEntityMaxHealth(Classes::CEntity* pEntity) noexcept
		{
			static const float& result = -1.0f;
			if (!pEntity)
				return result;

			const auto& pComponents = CleanPointer<Classes::IEntityComponents*>(pEntity->pComponents);
			if (!pComponents)
				return result;

			const auto& pActorComponent = CleanPointer<Classes::CSCActorComponent*>(pComponents->pActorComponent);
			if (!pActorComponent)
				return result;

			return CallVFunction<float>(pActorComponent, vft_Actor_GetMaxHealth / 8);
		}

		void SetEntityHealth(Classes::CEntity* pEntity, float health) noexcept
		{
			if (!pEntity)
				return;

			const auto& pComponents = CleanPointer<Classes::IEntityComponents*>(pEntity->pComponents);
			if (!pComponents)
				return;

			const auto& pActorComponent = CleanPointer<Classes::CSCActorComponent*>(pComponents->pActorComponent);
			if (!pActorComponent)
				return;

			const auto& pHealthComponent = CleanPointer<Classes::CSCBodyHealthComponent*>(pComponents->pHealthComponent);
			if (!pActorComponent)
				return;

			CallVFunction<__int64>(pActorComponent, vft_Actor_SetHealth / 8, health);

			//	Functions::CSCBodyHealthComponent_ApplyHealthChange(
			//		(__int64)pHealthComponent,
			//		health,
			//		(__int64)pEntity,
			//		0
			//	);
		}

		float GetEntityHealthPercent(Classes::CEntity* pTarget) noexcept
		{
			float result = 0.f;
			if (!pTarget)
				return result;

			const auto& pComponents = CleanPointer<Classes::IEntityComponents*>(pTarget->pComponents);
			if (!pComponents)
				return result;

			const auto& pActorComponent = CleanPointer<Classes::CSCActorComponent*>(pComponents->pActorComponent);
			if (!pActorComponent)
				return result;

			const auto& pHealthComponent = CleanPointer<Classes::CSCBodyHealthComponent*>(pComponents->pHealthComponent);
			if (!pActorComponent)
				return result;

			float curHP = CallVFunction<float>(pActorComponent, vft_Actor_GetHealth / 8);
			float maxHP = CallVFunction<float>(pActorComponent, vft_Actor_GetMaxHealth / 8);

			return (maxHP > 0.f) ? (curHP / maxHP) * 100.f : 0.f;
		}

		__int64 GetCharacter(Classes::CEntity* pEntity) noexcept
		{
			if (!pEntity)
				return 0;

			return CallVFunction<__int64>(pEntity, vft_CEntity_GetCharacter / 8);
		}

		__int64 GetEntityComponentByName(Classes::CEntity* pEntity, std::string name) noexcept
		{
			// verify entity is valid
			// get entity component system
			// get virtual function responsible for obtaining the component id
			// call centity vft responsible for obtaining the component by id
			// return the result
			//	
			//	char v82;
			//	__int16 v58;
			//	__int64 v78;
			//	v58 = Functions::CEntityComponentSystem_GetEntityComponentIDByName_stub((__int64)pEntityComponentSystem, (int*)&v82, name.c_str());
			//	LOWORD(v78) = v58;
			//	
			//	char v83;
			//	unsigned __int64 v59;
			//	v59 = Functions::CEntity_GetComponentByName_stub((__int64)pEntity, (int*)&v83, v78);


			if (!gEnv || !gEnv->pEntitySystem || !gEnv->pEntityComponentSystem)
				return 0;
			
			const auto& pEntityComponentSystem = gEnv->pEntityComponentSystem;
			__int64 mComponentID;
			auto res = CallVFunction<__int64>(pEntityComponentSystem, vft_CEntityComponentSystem_GetComponentID / 8, mComponentID, name.c_str());
			if (!res)
				return 0;
			
			__int64 pResult;
			auto resa = CallVFunction<__int64>(pEntity, vft_CEntity_GetComponent / 8, pResult, mComponentID);
			if (!resa)
				return 0;

			return pResult;
		}

		bool GetPlanetSphereScreenPoints(Classes::CEntity* pTarget, std::vector<Structs::FVector2D>* outPoints) noexcept
		{
			if (!pTarget || !pTarget->pLocalZoneEntity)
				return false;

			const auto& pEnt = CleanPointer<Classes::CEntity*>(pTarget->pLocalZoneEntity);
			Structs::STransforms TM;
			if (!GetEntityBounds(pEnt, &TM))
				return false;

			Structs::DVector center = TM.origin;

			float radius = TM.box.m_min.Distance(TM.box.m_max) * 0.5f;

			std::vector<Structs::FVector2D> points;
			for (int i = 0; i < 32; i++)
			{
				float theta = (2.0f * M_PI * i) / 32.0f;
				Structs::DVector point = center + Structs::DVector(radius * cosf(theta), radius * sinf(theta), 0.0f);
				
				Structs::FVector2D screenPoint;
				if (!Thread::WorldToScreen(point, Hooks::vars::szCanvas, &screenPoint, true))
					continue;

				points.push_back(screenPoint);
			}

			*outPoints = points;

			return points.size() > 0;
		}

		void SetPlayerActorClipState(int state) noexcept
		{

			const auto& pLocalEntity = GetLocalPlayerEntity();
			if (!pLocalEntity)
				return;

			const auto pLocalActorEntity = CleanPointer<Classes::CActorEntity*>(pLocalEntity->pActorEntity);
			if (!pLocalActorEntity)
				return;

			pLocalActorEntity->mCollision = state ? 0 : 34;
		}

		void SetPlayerActorFloatState(int state) noexcept
		{
			static Structs::SXCvar cvar{};
			static bool bFound = GetCVar(__("p_fly_mode"), &cvar);
			if (!bFound)
				return;

			if (!cvar.flagPTR)
				return;

			cvar.SetValue<int>(state ? 1 : 0);
		}

		void SetMiningParams() noexcept
		{
			datacore::GetStructInstance(__("MiningGlobalParams.MiningGlobalParams"));
		}

		std::string FormatString(const char* fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			char buffer[1024];
			vsnprintf(buffer, sizeof(buffer), fmt, args);
			va_end(args);
			return std::string(buffer);
		}

		std::string FormatDistance(const float& distance) noexcept
		{
			const char* units[] = { "m", "km", "Mm", "Gm", "Tm" };
			float dist = distance;
			int unitIndex = 0;

			while (dist >= 1000.f && unitIndex < 4) 
			{
				dist *= 0.001f;
				++unitIndex;
			}

			char buffer[32];
			snprintf(buffer, sizeof(buffer), "[%.0f%s]", dist, units[unitIndex]);
			return std::string(buffer);
		}

		Structs::FVector2D CalcTextSize(const std::string& text, const float& szFont) noexcept
		{
			const float& text_height = szFont * 3;
			float text_width = text.length() * szFont;
			return { text_width, text_height };
		}

		void CopyToClipboard(const char* fmt, ...)
		{
			/* format input */
			char buffer[1024];
			va_list args;
			va_start(args, fmt);
			vsnprintf(buffer, sizeof(buffer), fmt, args);
			va_end(args);
			std::string input = buffer;

			/* open clipboard for input */
			if (!OpenClipboard(nullptr))
				return;

			/* Clear the clipboard before setting new data */
			if (!EmptyClipboard()) 
			{
				CloseClipboard();
				return;
			}

			/* Allocate global memory for the text */
			HGLOBAL hClipboardData = GlobalAlloc(GMEM_MOVEABLE, input.size() + 1);
			if (hClipboardData == NULL) 
			{
				CloseClipboard();
				return;
			}
			
			char* pClipboardText = static_cast<char*>(GlobalLock(hClipboardData));
			if (!pClipboardText) 
			{
				GlobalFree(hClipboardData);
				CloseClipboard();
				return;
			}

			strcpy_s(pClipboardText, input.size() + 1, input.c_str());
			GlobalUnlock(hClipboardData);

			SetClipboardData(CF_TEXT, hClipboardData);
			CloseClipboard();
		} 
		
		///	
		//	bool WorldToScreen(Structs::DVector& pos, const Structs::FVector2D& szScreen, Structs::FVector2D* out, const bool& bIsRelative) noexcept
		//	{
		//		//Matrix-vector Product, multiplying world(eye) coordinates by projection matrix = clipCoords
		//	
		//		const auto& matrix = *(Structs::DMatrix44*)GetAddr(Offsets::gViewMatrix);
		//	
		//		Structs::DQuat clip;
		//		clip.x = pos.x * matrix.m[0] + pos.y * matrix.m[1] + pos.z * matrix.m[2] + matrix.m[3];
		//		clip.y = pos.x * matrix.m[4] + pos.y * matrix.m[5] + pos.z * matrix.m[6] + matrix.m[7];
		//		clip.z = pos.x * matrix.m[8] + pos.y * matrix.m[9] + pos.z * matrix.m[10] + matrix.m[11];
		//		clip.w = pos.x * matrix.m[12] + pos.y * matrix.m[13] + pos.z * matrix.m[14] + matrix.m[15];
		//	
		//		if (clip.w < 0.1f)
		//			return false;
		//	
		//		//perspective division, dividing by clip.W = Normalized Device Coordinates
		//		Structs::FVector NDC;
		//		NDC.x = clip.x / clip.w;
		//		NDC.y = clip.y / clip.w;
		//		NDC.z = clip.z / clip.w;
		//	
		//		Structs::FVector2D result = 
		//		{
		//			(szScreen.x / 2 * NDC.x) + (NDC.x + szScreen.x / 2),
		//			-(szScreen.y / 2 * NDC.y) + (NDC.y + szScreen.y / 2)
		//		};
		//	
		//		*out = result;
		//		
		//		return true;
		//	}
	}

	namespace Thread
	{
		static bool WorldToScreen(Structs::DVector& pos, const Structs::FVector2D& szScreen, Structs::FVector2D* out, const bool& isViewportRelative) noexcept
		{
			
			if (!gEnv || !gEnv->pSystem || !gEnv->pRenderer)
				return false;
			
			auto pRenderer = gEnv->pRenderer;
			auto pSystem = gEnv->pSystem;

			auto pCamera = CallVFunction<__int64>(pSystem, 0x3E0 / 8);

			float result[3];
			Structs::FVector screen2D;
			if (!pRenderer->ProjectToScreen(pos.x, pos.y, pos.z, &screen2D.x, &screen2D.y, &screen2D.z, pCamera))
				return false;

			//	if (!Functions::CRenderer_ProjectToScreen_stub(pRenderer, pos.x, pos.y, pos.z, &screen2D.x, &screen2D.y, &screen2D.z, pCamera))
			//		return false;

			/// 0-100 screen % scaling
			//	screen2D.x *= (szScreen.x * 0.01f);
			//	screen2D.y *= (szScreen.y * 0.01f);

			/// 0-1 normalized coordinates scaling
			screen2D.x *= szScreen.x;
			screen2D.y *= szScreen.y;

			/// check if the point is behind the camera or outside the screen bounds
			if (screen2D.z <= 0.0f || screen2D.z >= 1.0f)
				return false;
			
			/// 0-100 screen %
			out->x = screen2D.x;
			out->y = screen2D.y;

			return true;
		}

		bool CanvasDrawText(const char* text, const DWORD& index, Structs::DVector pos, const Structs::FQuat& color, bool bScreen2D)
		{
			auto pRenderer = gEnv->pRenderer;
			if (!pRenderer)
				return false;

			Structs::SDrawText draw;
			draw.color = color;
			if (bScreen2D)
				draw.flag = 0x70;

			return Functions::CRenderer_DrawText_stub(pRenderer, &pos, &draw, index, text);
		}

		bool CanvasDrawTextf(const char* text, const DWORD& index, Structs::DVector pos, const Structs::FQuat& color, bool bScreen2D, ...)
		{
			//	return false;
			auto pRenderer = gEnv->pRenderer;
			if (!pRenderer)
				return false;

			Structs::SDrawText draw;
			draw.color = color;
			if (bScreen2D)
				draw.flag = 0x70;

			va_list args;
			va_start(args, bScreen2D);
			return Functions::CRenderer_DrawTextArgs_stub(pRenderer, &pos, &draw, index, text, args);
		}

		bool CanvasDrawTextf2(const char* text, Structs::DVector pos, const Structs::FQuat& color, bool bScreen2D, ...)
		{
			auto pRenderer = gEnv->pRenderer;
			if (!pRenderer)
				return false;

			Structs::SDrawText draw;
			draw.color = color;
			if (bScreen2D)
				draw.flag = 0x70;

			va_list args;
			va_start(args, bScreen2D);
			return Functions::CRenderer_DrawTextArgs2_stub(pRenderer, &pos, &draw, text, args);
		}

		void Teleport(Classes::CEntity* pEntity, const Structs::DVector& pos, const bool& cheat_bUFO) noexcept
		{
			if (!pEntity)
				return;

			Structs::DQuat newRot;
			if (!pEntity->vf_GetWorldRotation(&newRot))
				return;

			Teleport(pEntity, pos, newRot, cheat_bUFO);
		}

		void Teleport(Classes::CEntity* pEntity, const Structs::DVector& pos, const Structs::DQuat& rotation, const bool& cheat_bUFO) noexcept
		{
			Structs::DQuat newRot = rotation;
			Structs::DVector newPos = pos;
			if (!pEntity->vf_SetWorldPos(&newPos) || !pEntity->vf_SetWorldRotation(&newRot))
				return;
			
			if (cheat_bUFO)
				Cheats::SetActorUFO(true);
		}

		void LocalTeleport(const Structs::DVector& pos, const bool& cheat_bUFO) noexcept
		{
			Classes::CEntity* pEntity = Helpers::GetLocalControlledEntity();
			if (!pEntity)
				return;

			Structs::DQuat newRot;
			if (!pEntity->vf_GetWorldRotation(&newRot))
				return;

			Teleport(pEntity, pos, newRot, cheat_bUFO);

			Cheats::SetActorUFO(true);
		}

		void LocalTeleport(const Structs::DVector& pos, const Structs::DQuat& rotation, const bool& cheat_bUFO) noexcept
		{
			Classes::CEntity* pEntity = Helpers::GetLocalControlledEntity();
			if (!pEntity)
				return;

			Teleport(pEntity, pos, rotation, cheat_bUFO);

			Cheats::SetActorUFO(true);
		}

		void SetDevFlyModeState(int mFlyModeState) noexcept
		{
			using namespace StarCitizen;

			const auto& pEnv = gEnv;
			if (!pEnv)
				return;

			const auto& pGame = CleanPointer<Classes::CGame*>(pEnv->pGame);	//	gEnv + 0x98
			if (!pGame)
				return;

			///	OLD METHOD
			//	long long pUnknown = CallVFunction<long long>((void*)pGame, GetVfIndex(0xA0), pGame);	//	Get a pointer @ 0xE0 in CGame
			//	if (!pUnknown)
			//		return;
			//	
			//	long long pActor = CallVFunction<long long>((void*)pUnknown, GetVfIndex(0x2D8), pUnknown);	//	CEntity Actor Component
			//	if (!pActor)
			//		return;

			const auto& pLocalPlayer = CleanPointer<Classes::CSCPlayer*>(pGame->pLocalPlayer);	//	pGame + 0xC00 ; also a vtable function ( index 11 )
			if (!pLocalPlayer)
				return;

			const auto& pLocalEntity = CleanPointer<Classes::CEntity*>(pLocalPlayer->pEntity);	//	pLocalPlayer + 0x8
			if (!pLocalEntity)
				return;

			const auto& pEntityComponents = CleanPointer<Classes::IEntityComponents*>(pLocalEntity->pComponents);	//	pLocalEntity + 0x238
			if (!pEntityComponents)
				return;

			const auto& pActorComponent = CleanPointer<Classes::CSCActorComponent*>(pEntityComponents->pActorComponent);	//	pEntityComponents + 0x58
			if (!pActorComponent)
				return;

			const auto& pResult = Functions::GetIActor_stub((__int64*)pActorComponent);
			if (!pResult)
				return;

			if (Hooks::vars::fly_pFlag != nullptr)
			{
				switch (mFlyModeState)
				{
				case 0: *Hooks::vars::fly_pFlag = 2; break;
				case 2: *Hooks::vars::fly_pFlag = 0; break;
				default: break;
				}
			}

			Functions::CCharacterStateHiearchy_SetState_stub(reinterpret_cast<__int64*>(pResult), mFlyModeState);
		}

		void UFO(Classes::CEntity* pEntity, const float& speed) noexcept
		{
			Classes::CCamera* pCamera = gCamera;
			if (!pEntity || !pCamera || !Hooks::vars::bWndwFocus)
				return;

			////	check if dev Fly Mode is active
			//	static ConsoleVariable* cvar_fly = g_Cvars->GetCvarByName(__("p_fly_mode"));
			//	if (cvar_fly && cvar_fly->flagPTR)
			//		*(bool*)cvar_fly->flagPTR = true;
			//	else return;

			int	 flyDir = 0;
			bool bForward	= (GetAsyncKeyState('W') != NULL);
			bool bLeft		= (GetAsyncKeyState('A') != NULL);
			bool bBack		= (GetAsyncKeyState('S') != NULL);
			bool bRight = (GetAsyncKeyState('D') != NULL);
			bool bLeftRoll = (GetAsyncKeyState('Q') != NULL);
			bool bRightRoll = (GetAsyncKeyState('E') != NULL);
			bool bAscend = (GetAsyncKeyState(VK_SPACE) != NULL);
			bool bDescend = (GetAsyncKeyState(VK_LCONTROL) != NULL);
			if (bForward) { flyDir = 1; }
			else if (bBack) { flyDir = 2; }
			if (bLeft) { flyDir = 3; }
			else if (bRight) { flyDir = 4; }
			if (bAscend) { flyDir = 5; }
			else if (bDescend) { flyDir = 6; }
			if (!flyDir)
				return;

			//	Get World Position
			Structs::DVector cPos;
			pEntity->vf_GetWorldPos(&cPos);

			//	Get Camera Angles
			float mforwardDir[3];
			float mRightDir[3];
			float mUpDir[3];
			pCamera->GetDirections(mforwardDir, mRightDir, mUpDir);

			for (int i = 0; i < 3; i++)
			{
				mforwardDir[i] *= (speed * .10f);
				mRightDir[i] *= (speed * .10f);
				mUpDir[i] *= (speed * .10f);
			}

			switch (flyDir)
			{
			case 1:	cPos += mforwardDir; break;
			case 3:	cPos -= mRightDir;	 break;
			case 2:	cPos -= mforwardDir; break;
			case 4:	cPos += mRightDir;	 break;
			case 5:	cPos += mUpDir;		 break;
			case 6:	cPos -= mUpDir;		 break;
			}
			pEntity->vf_SetWorldPos(&cPos);
		}

		void PLAYER_UFO(const float& speed) noexcept
		{
			Classes::CEntity* pLocalEntity = Helpers::GetLocalPlayerEntity();
			if (!pLocalEntity || !Hooks::vars::bWndwFocus)
				return;

			Classes::CEntity* pShipEntity = Helpers::GetLocalShipEntity();
			Classes::CEntity* pMechEntity = Helpers::GetLocalMechaEntity();
			if (pShipEntity || pMechEntity)
				return;

			UFO(pLocalEntity, speed);
		}

		void SHIP_UFO(const float& speed) noexcept
		{
			Classes::CEntity* pLocalEntity = Helpers::GetLocalPlayerEntity();
			if (!pLocalEntity || !Hooks::vars::bWndwFocus)
				return;

			Classes::CEntity* pShipEntity = Helpers::GetLocalShipEntity();
			Classes::CEntity* pMechEntity = Helpers::GetLocalMechaEntity();
			if (!pShipEntity && !pMechEntity)
				return;

			Classes::CEntity* pTarget = pLocalEntity;
			if (pShipEntity != nullptr)
				pTarget = pShipEntity;
			else if (pMechEntity != nullptr)
				pTarget = pMechEntity;

			UFO(pTarget, speed);
		}

		void ForgeEntity(Structs::STargetEntity& forgeEntity, Structs::SForgeControls& ctx)
		{
			/* get entity ref */
			const auto& pEntity = forgeEntity.pEntity;
			const auto& forgeTM = forgeEntity.TM;
			if (!pEntity || !gCamera || !Hooks::vars::bWndwFocus)
				return;

			/* get ref to local entity */
			const auto& pLocalEntity = Hooks::vars::sLocalPlayer;
			const auto& localTM = pLocalEntity.TM;
			
			/* get ref to camera */
			const auto& pCamera = gCamera;
			
			/* get camera tm */
			Structs::DVector camPos = gCamera->WorldPosition;
			float camFWD[3], camRIGHT[3], camUP[3];
			pCamera->GetDirections(camFWD, camRIGHT, camUP);
			for (int i = 0; i < 3; i++)
			{
				camFWD[i] *= ctx.distance;
				camRIGHT[i] *= ctx.distance;
				camUP[i] *= ctx.distance;
			}

			/* get updated tm */
			//Structs::DVector newWorldPos = localTM.origin + camFWD;
			Structs::DVector newWorldPos = camPos + camFWD;

			Structs::DVector newLocalPos = pEntity->transformZoneSpace.mTranslation;
			newLocalPos.z = (camPos.z + ctx.height);

			/* set new tm */
			pEntity->vf_SetWorldPos(&newWorldPos);	//	always update the position

			//	if (ctx.deltaX != 0.f || ctx.deltaY != 0.f || ctx.deltaZ != 0.f)
			//	{
			//		Structs::FVector deltaAngles = forgeTM.angles + Structs::FVector( ctx.deltaX * .1f, ctx.deltaY * .1f, ctx.deltaZ * .1f);
			//		Structs::DQuat deltaRot = Math::ToQuaternion(deltaAngles);
			//		Structs::DQuat newRot = Math::MultiplyQuaternions(deltaRot, forgeTM.rotation);
			//		Math::NormalizeQuaternion(newRot);
			//	
			//	
			//		/* set new rotation */
			//		pEntity->vf_SetWorldRotation(&newRot);
			//	
			//		/* restore deltas */
			//		ctx.deltaX = 0; 
			//		ctx.deltaY = 0;
			//		ctx.deltaZ = 0;
			//	}
			//	else
			//	{
			//		/* set new rotation */
			//		Structs::DQuat newRot = forgeTM.rotation;
			//		pEntity->vf_SetWorldRotation(&newRot);
			//	}

		}

		void SpaceShipHandbrake() noexcept
		{
			auto pShip = StarCitizen::Helpers::GetLocalShipEntity();
			if (!pShip || !Hooks::vars::bWndwFocus)
				return;

			bool bAction = (GetAsyncKeyState('X') & 0x8000) != 0;
			if (!bAction)
				return;

			Structs::DVector pos;
			pShip->vf_GetWorldPos(&pos);
			pos.z += .1f;
			pShip->vf_SetWorldPos(&pos);
		}

		void GrabEntity(Classes::CEntity* pEntity, const float& dist) noexcept
		{
			/* get ref to local entity */
			const auto& pLocalEntity = Hooks::vars::sLocalPlayer;
			if (!pEntity || !pLocalEntity.bValid || !gCamera || !Hooks::vars::bWndwFocus)
				return;

			/* get ref to camera */
			const auto& pCamera = gCamera;

			/* get camera tm */
			Structs::DVector camPos = gCamera->WorldPosition;
			float camFWD[3], camRIGHT[3], camUP[3];
			pCamera->GetDirections(camFWD, camRIGHT, camUP);
			for (int i = 0; i < 3; i++)
			{
				camFWD[i] *= dist;
				camRIGHT[i] *= dist;
				camUP[i] *= dist;
			}

			/* get updated tm */
			Structs::DVector newWorldPos = camPos + camFWD;
			pEntity->vf_SetWorldPos(&newWorldPos);
		}

		void GoToEntity(Classes::CEntity* pEntity, const float& dist) noexcept
		{
			/* get ref to local entity */
			const auto& pLocalEntity = Hooks::vars::sLocalPlayer;
			if (!pEntity || !pLocalEntity.bValid || !Hooks::vars::bWndwFocus)
				return;

			/* get location */
			Structs::DVector entPos;
			Structs::FVector entAngles;
			Structs::DQuat entQuat;
			if (!pEntity->vf_GetWorldPos(&entPos) ||
				!pEntity->vf_GetWorldRotation(&entQuat);
				!pEntity->vf_GetWorldForwardDir(&entAngles))
				return;

			/* set updated tm */
			Structs::DVector newWorldPos = entPos - (entAngles * dist);
			pLocalEntity.pEntity->vf_SetWorldPos(&newWorldPos);
			pLocalEntity.pEntity->vf_SetWorldRotation(&entQuat);
		}

		void rage_GrabAllPlayers(const float& dist) noexcept
		{
			//	Get all player entities
			if (!(GetAsyncKeyState('Z') & 0x8000))
				return;

			/* get ref to local entity */
			const auto& pLocalEntity = Hooks::vars::sLocalPlayer;
			if (!pLocalEntity.bValid || !gCamera || !Hooks::vars::bWndwFocus)
				return;

			/* get ref to camera */
			const auto& pCamera = gCamera;

			/* get camera tm */
			Structs::DVector camPos = gCamera->WorldPosition;
			float camFWD[3], camRIGHT[3], camUP[3];
			pCamera->GetDirections(camFWD, camRIGHT, camUP);
			for (int i = 0; i < 3; i++)
			{
				camFWD[i] *= dist;
				camRIGHT[i] *= dist;
				camUP[i] *= dist;
			}

			/* get updated tm */
			Structs::DVector newWorldPos = camPos + camFWD;

			/* iterate all players */
			const auto enemies = Hooks::vars::vPlayerEntities;
			for (const auto& ent : enemies)
			{
				if (ent == pLocalEntity.pEntity)
					continue;

				ent->vf_SetWorldPos(&newWorldPos);
			}
		}

		void LocalPlayerUpdate(Classes::CEntity* pEntity) noexcept
		{
			auto& localPlayer = Hooks::vars::sLocalPlayer;
			
			/* update entity ref */
			localPlayer.pEntity = pEntity;
			localPlayer.pShipEntity = Helpers::GetEntityShip(pEntity);
			localPlayer.bValid = pEntity != 0;
			if (!localPlayer.bValid)
			{
				localPlayer.TM = Structs::STransforms();
				return;
			}

			auto& localTM = localPlayer.TM;

			/* update TM entity ref */
			localTM.pEntity = pEntity;

			/* update TM */
			localTM.update();

			localPlayer.health = Helpers::GetEntityHealthPercent(pEntity);
		}

		void TargetEntityUpdate() noexcept
		{
			/* set new instance */
			if (Hooks::vars::target_bSetNewPlayer)
			{
				/* clear flag */
				Hooks::vars::target_bSetNewPlayer = false;
				
				/* set new target */
				Hooks::vars::target_player = Structs::STargetEntity(Hooks::vars::target_pSelection);

				/* clear selection */
				Hooks::vars::target_pSelection = nullptr;
			}

			/* update instance */
			Hooks::vars::target_player.update();

			/* flags for events */
			auto& target = Hooks::vars::target_player;
			if (!target.bValid)
			{
				/* clear target */
				Hooks::vars::target_player = Structs::STargetEntity();
			}

			/* teleport to target */
			if (target.bTeleport || target.bSticky)
			{
				/* @TODO: get size of both entities and set 5m from bounds */
				Structs::DVector pos = target.TM.origin - (target.TM.direction * 5.f);
				
				LocalTeleport(pos, target.TM.rotation, true);
				//	Teleport(Helpers::GetLocalPlayerEntity(), pos, target.TM.rotation, true);
				
				target.bTeleport = false;
			}

			/* kill target */
			if (target.bKill)
			{
				
			}

			/* forge mode */
			if (target.bForge)
			{
				ForgeEntity(target, Hooks::vars::target_controls);
			}
		}

		void TransformsUpdate() noexcept
		{
			auto fn = [](std::vector<Structs::SThreadEntity>& p)
			{
				for (auto& it : p)
				{
					const auto& player = it.pEntity;
					if (!player)
						continue;
					
					bool bUpdate = false;

					switch (it.entityType)
					{
						case Enums::EEntityType::ET_PLAYER: bUpdate = Hooks::vars::esp_bPlayer; break;
						case Enums::EEntityType::ET_SHIP: bUpdate = Hooks::vars::esp_bShip; break;
						case Enums::EEntityType::ET_ENEMY_NPC: bUpdate = Hooks::vars::esp_bEnemyAI; break;
						case Enums::EEntityType::ET_ANIMAL: bUpdate = Hooks::vars::esp_bAnimals; break;
						case Enums::EEntityType::ET_CONTAINER: bUpdate = Hooks::vars::esp_bLoot; break;
						case Enums::EEntityType::ET_ROCK: bUpdate = Hooks::vars::esp_bRock; break;
						case Enums::EEntityType::ET_ORBIT: bUpdate = Hooks::vars::esp_bOrbit; break;
						default: break;
					}

					if (!bUpdate)
						continue;

					/* get transforms */
					Structs::STransforms TM;
					if (!Helpers::GetEntityBounds(player, &TM))
						continue;

					/* set tm */
					it.TM = TM;
				}
			};

			fn(Hooks::vars::vScreenEntities);
		}

		void ScreenPointsUpdate(Classes::CRenderer* pRenderer) noexcept
		{
			if (!pRenderer)
				return;

			auto fn = [](std::vector<Structs::SThreadEntity>& p)
			{
				for (auto& it : p)
				{
					const auto& player = it.pEntity;
					if (!player)
						continue;

					/* get screen points */
					Helpers::GetEntityScreenPoints(player, it.TM, &it.screenTM);
				}
			};

			if (Hooks::vars::target_player.bValid)
			{
				/* get target player screen points */
				auto& target = Hooks::vars::target_player;
				Helpers::GetEntityScreenPoints(target.pEntity, target.TM, &target.screenTM);
			}

			/* @TODO: fix by getting zone bounds */
			//	for (auto& it : Hooks::vars::vScreenOrbit)
			//	{
			//		const auto& planet = it.first;
			//		if (!planet)
			//			continue;
			//	
			//		/* get planet screen points */
			//		Helpers::GetPlanetSphereScreenPoints(planet, &it.second);
			//	}

			fn(Hooks::vars::vScreenEntities);
		}

		void WaypointsUpdate(Classes::CEntity* pLocalEntity) noexcept
		{
			/* check for new entry */
			if (Hooks::vars::wp_bSetNewPoint)
			{
				Hooks::vars::wp_bSetNewPoint = false;
				if (Hooks::vars::wp_TargetName.length() > 0)
				{
					const auto& waypoint = Structs::SWaypoint(Hooks::vars::wp_TargetName.c_str());
					Hooks::vars::vWaypoints.push_back(waypoint);
				}
				Hooks::vars::wp_TargetName.clear();
			}

			/* update waypoints */
			for (auto& waypoint : Hooks::vars::vWaypoints)
			{
				if (!waypoint.bValid)
				{
					/* @TODO: remove waypoint */
					continue;
				}

				/* update entity instance */
				waypoint.pEntity = pLocalEntity;

				/* update the waypoint position */
				waypoint.update();	

				/* handle teleport event */
				if (waypoint.bTeleport || waypoint.bLockToWaypoint)
				{

					/* set player position & rotation */
					waypoint.pEntity->vf_SetWorldPos(&waypoint.mWorldPos);
					waypoint.pEntity->vf_SetWorldRotation(&waypoint.mWorldRot);

					/* set teleport to false if it was true */
					waypoint.bTeleport = false;
				}
			}
		}
		
		void ClassesUpdate(Classes::CEntitySystem* pEntitySystem) noexcept
		{
			static bool bFoundAllShips{ false };
			static bool bFoundAllContainers{ false };
			static bool bFoundAllRocks{ false };
			static bool bFoundAllKiosks{ false };
			static bool bFoundAllAnimals{ false };
			static bool bFoundAllEnemyNPC{ false };

			/* cache entity classes */
			auto& cache = Hooks::vars::cache_ClassNames;

			if (!pEntitySystem)
				return;

			static auto pEntityClassRegistry = pEntitySystem->vf_GetEntityClassRegistry();
			if (!pEntityClassRegistry)
				return;

			if (!cache.pPlayer)
				cache.pPlayer = pEntityClassRegistry->FindClass(__("Player"));

			if (!cache.pOrbitingContainer)
				cache.pOrbitingContainer = pEntityClassRegistry->FindClass(__("OrbitingObjectContainer"));

			if (!cache.pGoToPoint)
				cache.pGoToPoint = pEntityClassRegistry->FindClass(__("GoToPointEntity"));

			if (!cache.pMissionMarker)
				cache.pMissionMarker = pEntityClassRegistry->FindClass(__("MissionObjectiveMarker"));

			if (!bFoundAllShips)
				cache_entry(pEntitySystem, ClassNames::v_spaceships, &cache.vShips, bFoundAllShips);

			if (!bFoundAllContainers)
				cache_entry(pEntitySystem, ClassNames::v_lootables, &cache.vContainers, bFoundAllContainers);

			if (!bFoundAllRocks)
				cache_entry(pEntitySystem, ClassNames::v_mineable, &cache.vRocks, bFoundAllRocks);

			if (!bFoundAllKiosks)
				cache_entry(pEntitySystem, ClassNames::v_kiosks, &cache.vKiosks, bFoundAllKiosks);

			if (!bFoundAllAnimals)
				cache_entry(pEntitySystem, ClassNames::v_Animals, &cache.vAnimals, bFoundAllAnimals);

			if (!bFoundAllEnemyNPC)
				cache_entry(pEntitySystem, ClassNames::v_EnemyAI, &cache.vEnemyNPC, bFoundAllEnemyNPC);
		}

		bool cache_entry(Classes::CEntitySystem* pEntitySystem, vecString& names, vecEntityClassPair* out, bool& bFoundAll) noexcept
		{
			if (!out || !pEntitySystem)
				return false;

			static auto pEntityClassRegistry = pEntitySystem->vf_GetEntityClassRegistry();
			if (!pEntityClassRegistry)
				return false;

			int count = -1;
			bool bFound = false;
			vecEntityClassPair tmpCache;
			for (const auto& name : names)
			{
				count++;
				Classes::CEntityClass* pEntityClass = pEntityClassRegistry->FindClass(name.c_str());
				if (!pEntityClass)
					continue;

				tmpCache.push_back({ name, pEntityClass });
				bFound |= true; // found at least one
			}

			*out = tmpCache;

			bFoundAll = bFound && (tmpCache.size() == names.size());	//	@TODO: this will always return true ... lol

			return bFound;
		}
	}

	namespace Cheats
	{
		void SetActorUFO(bool state) noexcept
		{
			Hooks::vars::cheat_bUFO = state;
			Hooks::vars::mUFOSpeedScalar = 1.f;
			Helpers::SetPlayerActorFloatState(state);
			//Helpers::SetPlayerActorClipState(state);
		}

		void SetDevNoClip(bool state) noexcept
		{
			StarCitizen::Hooks::vars::fly_bEnable = state;                             //  toggle state for fly mode
			switch (state)
			{
			case true: StarCitizen::Hooks::vars::fly_iState = 2; break;				//  set fly mode on
			case false: StarCitizen::Hooks::vars::fly_iState = 0; break;			//  set fly mode off
			}
			StarCitizen::Hooks::vars::fly_bSet = true;								//  set fly mode state from CSystemUpdate hook
		}

		void SetNoFog(bool state) noexcept
		{
			Hooks::vars::cheat_bNoFog = state;

			static Structs::SXCvar cvar{};
			static bool bFoundVar = Helpers::GetCVar(__("e_Fog"), &cvar);
			if (!bFoundVar)
				return;

			if (!cvar.flagPTR)
				return;

			cvar.SetValue<int>(!state);
		}

		void SetDisableGlare(bool state) noexcept
		{
			Hooks::vars::cheat_bDisableGlare = state;
			
			static Structs::SXCvar cvar{};
			static bool bFoundVar = Helpers::GetCVar(__("r_glareFilterThreshold"), &cvar);
			if (!bFoundVar)
				return;
			
			if (!cvar.flagPTR)
				return;

			cvar.SetValue<float>(state ? 0.f : 500.f);
		}

		void SetNoGForce(bool state) noexcept
		{
			Hooks::vars::cheat_bNoGForce = state;

			static Structs::SXCvar cvar{};
			static bool bFoundVar = Helpers::GetCVar(__("pl_gforce.enabled"), &cvar);
			if (bFoundVar && cvar.flagPTR)
				cvar.SetValue<int>(state ? 0 : 1);

			static Structs::SXCvar cvar1{};
			static bool bFoundVar1 = Helpers::GetCVar(__("v_gforce_animations"), &cvar1);
			if (bFoundVar1 && cvar1.flagPTR)
				cvar1.SetValue<int>(state ? 0 : 1);			
		}

		void SetWeaponNoRecoil(bool state) noexcept
		{
			Hooks::vars::cheat_bNoRecoil = state;

			static Structs::SXCvar cvar{};
			static bool bFoundVar = Helpers::GetCVar(__("pl_proceduralRecoil.enable"), &cvar);
			if (!bFoundVar)
				return;

			if (!cvar.flagPTR)
				return;

			cvar.SetValue<int>(state ? 0 : 1);
		}

		void SetDisableArmistice(bool state) noexcept
		{
			///	removed
			//	Hooks::vars::cheat_bDisableArmistice = state;
			//	
			//	static Structs::SXCvar cvar{};
			//	static bool bFoundVar = Helpers::GetCVar(__("g_disable_green_zones"), &cvar);
			//	if (!bFoundVar)
			//		return;
			//	
			//	if (!cvar.flagPTR)
			//		return;
			//	
			//	cvar.SetValue<int>(state);

			if (!gMissionSystemCVars || !gMissionSystemCVars->p)
				return;

			gMissionSystemCVars->p->green_zone.disable = state;
		}

		void SetDisableATCRestrictions(bool state) noexcept
		{
			///	removed
			//	Hooks::vars::cheat_bDisableATCLandingRestrictions = state;
			//	
			//	static Structs::SXCvar cvar{};
			//	static bool bFoundVar = Helpers::GetCVar(__("g_ATCDisableLandingRestrictions"), &cvar);
			//	if (!bFoundVar)
			//		return;
			//	
			//	if (!cvar.flagPTR)
			//		return;
			//	
			//	cvar.SetValue<int>(!state);
		}

		void SetHUDStatusEffects(bool state) noexcept
		{
			Hooks::vars::cheat_bShowExtendedStatusHUD = state;

			static Structs::SXCvar cvar{};
			static bool bFoundVar = Helpers::GetCVar(__("pl_ActivatePlayerStatusDisplay"), &cvar);
			if (!bFoundVar)
				return;

			if (!cvar.flagPTR)
				return;

			cvar.SetValue<int>(state ? 3 : 0);
		}

		void SetEasyJumpGate(bool state) noexcept
		{
			/*
				g_jump_drive.disableFailures
				g_jump_drive.disableFuelBurn
				g_jump_drive.disableTunnelDistortion
				g_jump_drive.disableVibrations
				g_jump_tunnel.easyMode
				g_jump_tunnel.enableObstacles
				g_jump_tunnel.enableTunnelKillingActors
			
			*/
			Hooks::vars::cheat_bEasyJumpGate = state;



			static Structs::SXCvar cvar{};
			static bool bFoundVar = Helpers::GetCVar(__("g_jump_drive.disableFailures"), &cvar);
			if (bFoundVar && cvar.flagPTR)
				cvar.SetValue<int>(state ? 1 : 0);

			static Structs::SXCvar cvar1{};
			static bool bFoundVar1 = Helpers::GetCVar(__("g_jump_drive.disableFuelBurn"), &cvar1);
			if (bFoundVar1 && cvar1.flagPTR)
				cvar1.SetValue<int>(state ? 1 : 0);

			//	static Structs::SXCvar cvar2{};
			//	static bool bFoundVar2 = Helpers::GetCVar(__("g_jump_drive.disableTunnelDistortion"), &cvar2);
			//	if (bFoundVar2 && cvar2.flagPTR)
			//		cvar2.SetValue<int>(state ? 1 : 0);

			//	static Structs::SXCvar cvar3{};
			//	static bool bFoundVar3 = Helpers::GetCVar(__("g_jump_drive.disableVibrations"), &cvar3);
			//	if (bFoundVar3 && cvar3.flagPTR)
			//		cvar3.SetValue<int>(state ? 1 : 0);

			//	static Structs::SXCvar cvar4{};
			//	static bool bFoundVar4 = Helpers::GetCVar(__("g_jump_tunnel.easyMode"), &cvar4);
			//	if (bFoundVar4 && cvar4.flagPTR)
			//		cvar4.SetValue<int>(state ? 1 : 0);	//	1 = straight tunnel : 2 = sine wave tunnel

			//	static Structs::SXCvar cvar5{};
			//	static bool bFoundVar5 = Helpers::GetCVar(__("g_jump_tunnel.enableObstacles"), &cvar5);
			//	if (bFoundVar5 && cvar5.flagPTR)
			//		cvar5.SetValue<int>(state ? 0 : 1);

			static Structs::SXCvar cvar6{};
			static bool bFoundVar6 = Helpers::GetCVar(__("g_jump_tunnel.enableTunnelKillingActors"), &cvar6);
			if (bFoundVar6 && cvar6.flagPTR)
				cvar6.SetValue<int>(state ? 0 : 1);

			//	static Structs::SXCvar cvar7{};
			//	static bool bFoundVar7 = Helpers::GetCVar(__("fx_jump.tunnel.enableSplineEffect"), &cvar7);
			//	if (bFoundVar7 && cvar7.flagPTR)
			//		cvar7.SetValue<int>(state ? 0 : 1);

			//	static Structs::SXCvar cvar8{};
			//	static bool bFoundVar8 = Helpers::GetCVar(__("fx_jump.tunnel.enableTransitFailEffect"), &cvar8);
			//	if (bFoundVar8 && cvar8.flagPTR)
			//		cvar8.SetValue<int>(state ? 0 : 1);
		}

		void SetDisableAllShakes(bool state) noexcept
		{
			/*
				g_screenShakeArea.enabled	: 0 disables hud shake
				hud_shake_scale				: 1 disables screen shake areas
				pl_breath.proceduralBreathingAnim.enabled	: 0 disables breathing animations
			*/
			Hooks::vars::cheat_bDisableAllShakes = state;

			static Structs::SXCvar cvar{};
			static bool bFoundVar = Helpers::GetCVar(__("g_screenShakeArea.enabled"), &cvar);
			if (bFoundVar && cvar.flagPTR)
				cvar.SetValue<int>(state ? 0 : 1);

			static Structs::SXCvar cvar1{};
			static bool bFoundVar1 = Helpers::GetCVar(__("hud_shake_scale"), &cvar1);
			if (bFoundVar1 && cvar1.flagPTR)
				cvar1.SetValue<int>(state ? 1 : 0);

			static Structs::SXCvar cvar2{};
			static bool bFoundVar2 = Helpers::GetCVar(__("pl_breath.proceduralBreathingAnim.enabled"), &cvar2);
			if (bFoundVar2 && cvar2.flagPTR)
				cvar2.SetValue<int>(state ? 0 : 1);
		}

		void AC_SetDisablePlayableAreaRestriction(bool state)
		{
			/*
				ea_playablearea.svenabled	: 0 disables playable area restriction
			*/
			static Structs::SXCvar cvar{};
			static bool bFoundVar = Helpers::GetCVar(__("ea_playablearea.svenabled"), &cvar);
			if (bFoundVar && cvar.flagPTR)
				cvar.SetValue<int>(state ? 0 : 1);
		}

		void AC_DisableTimeLimit(bool state)
		{
			/*
				g_timeLimitOverride	: 0 disables time limit
			*/
			static Structs::SXCvar cvar{};
			static bool bFoundVar = Helpers::GetCVar(__("g_timeLimitOverride"), &cvar);
			if (bFoundVar && cvar.flagPTR)
				cvar.SetValue<int>(state ? 0 : 1);
		}

		bool SetDefaultLoadout(const unsigned __int8& loadoutIndex) noexcept
		{
			Hooks::vars::cmd_selection = {};	//	clear
			if (!StarCitizen::Helpers::GetCmd(__("pl_loadLoadout"), &Hooks::vars::cmd_selection))
				return false;

			Hooks::vars::cmd_args[0] = (char*)Hooks::vars::cmd_selection.Name;

			if (loadoutIndex < ClassNames::v_Loadouts.size()) {
				Hooks::vars::cmd_args[Hooks::vars::cmd_count++] = (char*)(ClassNames::v_Loadouts[loadoutIndex].c_str());
				Hooks::vars::cmd_bExec = true;
				return true;
			}
			//	switch (p)
			//	{
			//	case StarCitizen::Enums::EDefaultLoadout::EDEFAULTLOADOUT_PU_DEFAULT:					Hooks::vars::cmd_args[Hooks::vars::cmd_count++] = (char*)("DefaultLoadouts/PU_Default.xml");	break;
			//	case StarCitizen::Enums::EDefaultLoadout::EDEFAULTLOADOUT_PU_HEALING_TOOL:				Hooks::vars::cmd_args[Hooks::vars::cmd_count++] = (char*)("DefaultLoadouts/PU_HealingTool.xml");	break;
			//	case StarCitizen::Enums::EDefaultLoadout::EDEFAULTLOADOUT_PU_MINING_TOOL:				Hooks::vars::cmd_args[Hooks::vars::cmd_count++] = (char*)("DefaultLoadouts/PU_MiningTool.xml");	break;
			//	case StarCitizen::Enums::EDefaultLoadout::EDEFAULTLOADOUT_ZEUS:							Hooks::vars::cmd_args[Hooks::vars::cmd_count++] = (char*)("DefaultLoadouts/zeus.xml");	break;
			//	case StarCitizen::Enums::EDefaultLoadout::EDEFAULTLOADOUT_WHALEMAN:						Hooks::vars::cmd_args[Hooks::vars::cmd_count++] = (char*)("DefaultLoadouts/WhaleMan.xml");	break;
			//	case StarCitizen::Enums::EDefaultLoadout::EDEFAULTLOADOUT_THE_DIRECTOR:					Hooks::vars::cmd_args[Hooks::vars::cmd_count++] = (char*)("DefaultLoadouts/TheDirector.xml");	break;
			//	case StarCitizen::Enums::EDefaultLoadout::EDEFAULTLOADOUT_PU_TRACTOR_BEAM:				Hooks::vars::cmd_args[Hooks::vars::cmd_count++] = (char*)("DefaultLoadouts/PU_TractorBeam.xml");	break;
			//	case StarCitizen::Enums::EDefaultLoadout::EDEFAULTLOADOUT_MFT_LOADOUT_CQC:				Hooks::vars::cmd_args[Hooks::vars::cmd_count++] = (char*)("DefaultLoadouts/MFTLoadout_CQC.xml");	break;
			//	case StarCitizen::Enums::EDefaultLoadout::EDEFAULTLOADOUT_MFT_LOADOUT_SALVAGE:			Hooks::vars::cmd_args[Hooks::vars::cmd_count++] = (char*)("DefaultLoadouts/MFTLoadout_Salvage.xml");	break;
			//	case StarCitizen::Enums::EDefaultLoadout::EDEFAULTLOADOUT_DEFAULT_EVERYTHING_NEW_MOBI:	Hooks::vars::cmd_args[Hooks::vars::cmd_count++] = (char*)("default_everything_loadout_newMobi.xml");	break;
			//	case StarCitizen::Enums::EDefaultLoadout::EDEFAULTLOADOUT_DEV_INVIS_PLAYER_01:			Hooks::vars::cmd_args[Hooks::vars::cmd_count++] = (char*)("dev/invis_player_01.xml");	break;
			//	case StarCitizen::Enums::EDefaultLoadout::EDEFAULTLOADOUT_MEDICAL_BODY:					Hooks::vars::cmd_args[Hooks::vars::cmd_count++] = (char*)("UI/MedicalBody.xml");	break;
			//	case StarCitizen::Enums::EDefaultLoadout::EDEFAULTLOADOUT_MEDICAL_SKELETON:				Hooks::vars::cmd_args[Hooks::vars::cmd_count++] = (char*)("UI/MedicalSkeleton.xml");	break;
			//	default: return false;
			//	}
			//	Hooks::vars::cmd_bExec = true;
			return false;
		}

		void SetDisableCorpseLootingRestrictions(bool state) noexcept
		{
			static Structs::SXCvar cvar{};
			static bool bFoundVar = Helpers::GetCVar(__("pl_choice.enable_corpse_looting_restrictions"), &cvar);
			if (bFoundVar && cvar.flagPTR)
				cvar.SetValue<int>(state ? 0 : 1);
		}
	}

	namespace Math
	{
		bool IsPointInsideRadius(const float& szRadius, const Structs::FVector2D& radiusCenter, const Structs::FVector2D& point)
		{
			return (GetDistance2D(radiusCenter, point) <= szRadius);
		}

		float GetDistance2D(const Structs::FVector2D& from, const Structs::FVector2D& to)
		{
			return sqrt(abs(from.x - to.x) * abs(from.x - to.x) + abs(from.y - to.y) * abs(from.y - to.y));
		}

		Structs::DQuat ToQuaternion(const Structs::FVector& angles) noexcept
		{
			double cp = cos(angles.x * 0.5);
			double sp = sin(angles.x * 0.5);
			double cy = cos(angles.y * 0.5);
			double sy = sin(angles.y * 0.5);
			double cr = cos(angles.z * 0.5);
			double sr = sin(angles.z * 0.5);

			Structs::DQuat q;
			q.w = cr * cp * cy + sr * sp * sy;
			q.x = sr * cp * cy - cr * sp * sy;
			q.y = cr * sp * cy + sr * cp * sy;
			q.z = cr * cp * sy - sr * sp * cy;

			return q;
		}

		Structs::DQuat MultiplyQuaternions(const Structs::DQuat& q1, const Structs::DQuat& q2)
		{
			return Structs::DQuat(
				q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z, // W
				q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y, // X
				q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z, // Y
				q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x  // Z
			);
		}

		void NormalizeQuaternion(Structs::DQuat& q) noexcept
		{
			double magnitude = sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);

			if (magnitude > 0.0)  // Prevent division by zero
			{
				double invMag = 1.0 / magnitude;
				q.w *= invMag;
				q.x *= invMag;
				q.y *= invMag;
				q.z *= invMag;
			}
		}

		void RotatePoint(const Structs::FVector& point, const Structs::FVector& center, const float* angles, Structs::FVector* result)
		{
			double sx, cx, sy, cy, sz, cz;
			sincos(angles[1], sx, cx);
			sincos(angles[2], sy, cy);
			sincos(angles[0], sz, cz);
			Structs::FVector rotatedPoint;
			Structs::FVector dir;

			dir = point - center;
			rotatedPoint.x = point.x;
			rotatedPoint.y = center.y + (dir.y * cz - dir.z * sz);
			rotatedPoint.z = center.z + (dir.y * sz + dir.z * cz);

			dir = rotatedPoint - center;
			rotatedPoint.x = center.x + (dir.x * cx + dir.z * sx);
			rotatedPoint.z = center.z + (dir.z * cx - dir.x * sx);

			dir = rotatedPoint - center;

			result->x = center.x + (dir.x * cy - dir.y * sy);
			result->y = center.y + (dir.x * sy + dir.y * cy);
			result->z = rotatedPoint.z;
		}

		void RotatePoint(const Structs::DVector& point, const Structs::DVector& center, const float* angles, Structs::DVector* result)
		{
			double sx, cx, sy, cy, sz, cz;
			sincos(angles[1], sx, cx);
			sincos(angles[2], sy, cy);
			sincos(angles[0], sz, cz);
			Structs::DVector rotatedPoint;
			Structs::DVector dir;

			dir = point - center;
			rotatedPoint.x = point.x;
			rotatedPoint.y = center.y + (dir.y * cz - dir.z * sz);
			rotatedPoint.z = center.z + (dir.y * sz + dir.z * cz);

			dir = rotatedPoint - center;
			rotatedPoint.x = center.x + (dir.x * cx + dir.z * sx);
			rotatedPoint.z = center.z + (dir.z * cx - dir.x * sx);

			dir = rotatedPoint - center;

			result->x = center.x + (dir.x * cy - dir.y * sy);
			result->y = center.y + (dir.x * sy + dir.y * cy);
			result->z = rotatedPoint.z;
		}

		void RotateBoxVerts(const Structs::DVector boxVerts[8], const Structs::DVector& center, const Structs::FVector& angles, Structs::DVector boxVertsResult[8])
		{
			double sx, cx, sy, cy, sz, cz;
			sincos(angles.y, sx, cx);
			sincos(angles.z, sy, cy);
			sincos(angles.x, sz, cz);
			Structs::DVector rotatedPoint;
			Structs::DVector dir;
			for (int b = 0; b < 8; b++)
			{
				const Structs::DVector& point = boxVerts[b];
				Structs::DVector& result = boxVertsResult[b];
				dir = point - center;
				rotatedPoint.x = point.x;
				rotatedPoint.y = center.y + (dir.y * cz - dir.z * sz);
				rotatedPoint.z = center.z + (dir.y * sz + dir.z * cz);

				dir = rotatedPoint - center;
				rotatedPoint.x = center.x + (dir.x * cx + dir.z * sx);
				rotatedPoint.z = center.z + (dir.z * cx - dir.x * sx);

				dir = rotatedPoint - center;

				result.x = center.x + (dir.x * cy - dir.y * sy);
				result.y = center.y + (dir.x * sy + dir.y * cy);
				result.z = rotatedPoint.z;
			}
		}
	}

	namespace Gui
	{
		namespace scMenu
		{
			namespace scTabs
			{

				void Enhancements()
				{
					ImVec2 main_szWndw = ImGui::GetContentRegionAvail();
					const auto& main_wndwWidth = main_szWndw.x;
					const auto& main_pDraw = ImGui::GetWindowDrawList();
					const auto& main_posWndw = ImGui::GetWindowPos();
					const auto& child_resize_pad_x = ImGuiChildFlags_ResizeX | ImGuiChildFlags_AlwaysUseWindowPadding;
					const auto& child_resize_pad_y = ImGuiChildFlags_ResizeY | ImGuiChildFlags_AlwaysUseWindowPadding;

					/* PLAYER , SHIP , ARENA COMMANDER */
					ImGui::SeparatorText(__("PLAYER , SHIP & ARENA COMMANDER"));
					ImGui::BeginChild(__("##player_ship_main"), ImVec2(main_szWndw.x, main_szWndw.y * .33f), child_resize_pad_y);
					{
						ImVec2 child_player_szWndw = ImGui::GetContentRegionAvail();
						ImGui::BeginChild(__("##player_child_player"), ImVec2(child_player_szWndw.x * .3f, child_player_szWndw.y), child_resize_pad_x);
						{
							if (ImGui::Checkbox(__("UFO Mode"), &StarCitizen::Hooks::vars::cheat_bUFO))
								StarCitizen::Cheats::SetActorUFO(StarCitizen::Hooks::vars::cheat_bUFO);
							ImGui::SameLine(); gui::widget::HelpMarker(__("free cam movement style. [Scroll mousewheel] to adjust. UFO Speed is also rendered on the display"));

							ImGui::Checkbox((__("Auto Heal")), &Hooks::vars::cheat_bAutoHeal);
							ImGui::SameLine(); gui::widget::HelpMarker(__("automatically heals the local player upon taking damage. will also revive the player. server does not accept the new value so players should use a real healing item when available."));

							ImGui::Checkbox(__("Unlock Walk Speed"), &StarCitizen::Hooks::vars::cheat_bCustomWalkSpeed);
							ImGui::SameLine(); gui::widget::HelpMarker(__("unlocks the restriction set on player movement speed. Hold [SHIFT + scroll mousewheel] to adjust. Walk Speed is also rendered on the display."));

							ImGui::Checkbox(__("Max Inventory Capacity"), &StarCitizen::Hooks::vars::cheat_bMaxInventoryStorage);
							ImGui::SameLine(); gui::widget::HelpMarker(__("sets the players inventory capacity for all storage containers to match local storage capacity. place an item from a storage access kiosk into a container to clone the local capacity size."));

							if (ImGui::Checkbox(__("Relaxed Armistice Zones"), &StarCitizen::Hooks::vars::cheat_bDisableArmistice))
								StarCitizen::Cheats::SetDisableArmistice(StarCitizen::Hooks::vars::cheat_bDisableArmistice);
							ImGui::SameLine(); gui::widget::HelpMarker(__("remove some armistice restrictions."));

							//	if (ImGui::Checkbox(__("Disable ATC Landing Restrictions"), &StarCitizen::Hooks::vars::cheat_bDisableATCLandingRestrictions))
							//		StarCitizen::Cheats::SetDisableATCRestrictions(StarCitizen::Hooks::vars::cheat_bDisableATCLandingRestrictions);
							//	ImGui::SameLine(); gui::widget::HelpMarker(__("disable the ATC landing restrictions. allows for stealing ships.\nNOTE: ships are lost once destroyed."));

							//	ImGui::Checkbox((__("Demi God")), &Hooks::vars::cheat_bDemiGod);
							//	ImGui::SameLine(); gui::widget::HelpMarker(__("prevents the player from dying. also affects other entities."));

							ImGui::Checkbox(__("Blame Shooter"), &Hooks::vars::cheat_bBlameUser);
							ImGui::SameLine(); gui::widget::HelpMarker(__("spoof the shooter of a projectile. protects the local player from crimes."));

							ImGui::Checkbox(__("Mirror Force"), &Hooks::vars::cheat_bMirrorForce);
							ImGui::SameLine(); gui::widget::HelpMarker(__("mirror the force of a projectile. sends it back to shooter."));

							if (ImGui::Checkbox(__("Disable Loot Restrictions"), &Hooks::vars::cheat_bDisableLootRestrictions))
								Cheats::SetDisableCorpseLootingRestrictions(Hooks::vars::cheat_bDisableLootRestrictions);
							ImGui::SameLine(); gui::widget::HelpMarker(__("disable loot restrictions."));

							ImGui::Checkbox(__("Force Hold Enemies"), &Hooks::vars::cheat_rage_bGrabAllEnemies);
							ImGui::SameLine(); gui::widget::HelpMarker(__("grabs all NPC enemies in the area and holds them in front of the local player.\nKEY: Z"));

							if (ImGui::Button(__("Heal")))
								Hooks::vars::cheat_bHealSelf = true;
							ImGui::SameLine(); gui::widget::HelpMarker(__("heal the local player."));
							//	ImGui::SameLine();
							//	if (ImGui::Button(__("Respawn")))
							//		Hooks::vars::cheat_bKillSelf = true;
							//	ImGui::SameLine(); gui::widget::HelpMarker(__("respawn the local player."));
						}
						ImGui::EndChild();

						ImGui::SameLine();
						child_player_szWndw = ImGui::GetContentRegionAvail();
						ImGui::BeginChild(__("##player_child_ship"), ImVec2(child_player_szWndw.x * .5f, child_player_szWndw.y), child_resize_pad_x);
						{
							if (ImGui::Checkbox(__("No GForce"), &StarCitizen::Hooks::vars::cheat_bNoGForce))
								StarCitizen::Cheats::SetNoGForce(StarCitizen::Hooks::vars::cheat_bNoGForce);
							ImGui::SameLine(); gui::widget::HelpMarker(__("disable the effects of GForce."));

							ImGui::Checkbox(__("Instant QT"), &StarCitizen::Hooks::vars::cheat_bInstantWarp);
							ImGui::SameLine(); gui::widget::HelpMarker(__("instantly calibrate quantum drive and arrive at destination when warp initiates. automatically disabled upon reaching destination."));

							ImGui::Checkbox(__("Boost Multiplier"), &StarCitizen::Hooks::vars::cheat_bShipBoostMP);
							ImGui::SameLine(); gui::widget::HelpMarker(__("instantly boost the ship to maximum speed."));
							//	if (ImGui::Checkbox(__("EZ Jump Gate Tunnel"), &StarCitizen::Hooks::vars::cheat_bEasyJumpGate))
							//		Cheats::SetEasyJumpGate(Hooks::vars::cheat_bEasyJumpGate);
							//	ImGui::SameLine(); gui::widget::HelpMarker(__("makes the jump sequence between solar systems easier."));

							ImGui::Checkbox(__("Spaceship Handbrake"), &StarCitizen::Hooks::vars::cheat_bSpaceBrake);
							ImGui::SameLine(); gui::widget::HelpMarker(__("pressing 'X' will instantly freeze the ships position."));
						}
						ImGui::EndChild();

						ImGui::SameLine();
						child_player_szWndw = ImGui::GetContentRegionAvail();
						ImGui::BeginChild(__("##player_child_arena_commander"), child_player_szWndw, ImGuiChildFlags_AlwaysUseWindowPadding);
						{
							ImGui::Checkbox(__("[AC] Disable Play Area Restriction"), &StarCitizen::Hooks::vars::cheat_bDisablePlayAreaAC);
							StarCitizen::Cheats::AC_SetDisablePlayableAreaRestriction(StarCitizen::Hooks::vars::cheat_bDisablePlayAreaAC);
							ImGui::SameLine(); gui::widget::HelpMarker(__("disable the playable area restriction in Arena Commander."));

							ImGui::Checkbox(__("[AC] Disable Time Limit"), &Hooks::vars::cheat_bDisableTimeLimitAC);
							StarCitizen::Cheats::AC_DisableTimeLimit(Hooks::vars::cheat_bDisableTimeLimitAC);
							ImGui::SameLine(); gui::widget::HelpMarker(__("disable the time limit in Arena Commander."));
						}
						ImGui::EndChild();

					}
					ImGui::EndChild();

					/* WEAPONS */
					main_szWndw = ImGui::GetContentRegionAvail();
					ImGui::SeparatorText(__("WEAPONS , GEAR & LOADOUT"));
					ImGui::BeginChild(__("##weapons_main"), ImVec2(main_szWndw.x, main_szWndw.y * .5f), child_resize_pad_y);
					{
						ImVec2 child_player_szWndw = ImGui::GetContentRegionAvail();
						ImGui::BeginChild(__("##weapons_child_weapon"), ImVec2(child_player_szWndw.x * .5f, child_player_szWndw.y), child_resize_pad_x);
						{
							ImGui::Checkbox(__("Infinite Ammo"), &StarCitizen::Hooks::vars::cheat_bInfiniteAmmo);
							ImGui::SameLine(); gui::widget::HelpMarker(__("infinite ammo for most weapon types."));

							if (ImGui::Checkbox(__("No Recoil"), &StarCitizen::Hooks::vars::cheat_bNoRecoil))
								StarCitizen::Cheats::SetWeaponNoRecoil(StarCitizen::Hooks::vars::cheat_bNoRecoil);
							ImGui::SameLine(); gui::widget::HelpMarker(__("remove weapon recoil for most weapon types."));

							ImGui::Checkbox(__("No Spread"), &StarCitizen::Hooks::vars::cheat_bNoSpread);
							ImGui::SameLine(); gui::widget::HelpMarker(__("remove weapon spread for most weapon types.\n*persists for the weapon until game restart."));

							ImGui::Checkbox(__("Tuned Mining Beam"), &StarCitizen::Hooks::vars::cheat_bTunedMiningFractureBeam);
							ImGui::SameLine(); gui::widget::HelpMarker(__("mining fracture beam with increased power & optimal window size."));
							ImGui::Checkbox(__("Auto Fracture"), &StarCitizen::Hooks::vars::cheat_mining_bAutoFracture);

							ImGui::Checkbox(__("Tuned Tractor Beam"), &StarCitizen::Hooks::vars::cheat_bTunedTractorBeam);
							ImGui::SameLine(); gui::widget::HelpMarker(__("tractor beam with increased range , capacity & no breakage. Hold 'L-ALT' to disable collision for the attached object."));

							ImGui::Checkbox(__("Tuned Medical Beam"), &StarCitizen::Hooks::vars::cheat_bTunedMedicalBeam);
							ImGui::SameLine(); gui::widget::HelpMarker(__("medical beam with increased range & healing properties."));
							if (StarCitizen::Hooks::vars::cheat_bTunedMedicalBeam)
							{
								ImGui::SameLine();
								ImGui::Checkbox(__("##TUNED_MEDICAL_BEAM_KILL"), &StarCitizen::Hooks::vars::cheat_bEvilMedicalBeam);
								ImGui::SameLine(); gui::widget::HelpMarker(__("reverses the medical beam properties. can be used to remove drug level and health."));
							}

							ImGui::Checkbox(__("Tuned Salvage Beam"), &StarCitizen::Hooks::vars::salvage_bInstantFillRate);
							ImGui::SameLine(); gui::widget::HelpMarker(__("salvage beam with increased range & scraping efficiency.\n*cancels out infinite ammo."));

							ImGui::Checkbox(__("Damage Multiplier"), &StarCitizen::Hooks::vars::cheat_bDamageMultiplier);
							ImGui::SameLine(); gui::widget::HelpMarker(__("projectile damage multiplier. applies to current equipped weapon , anybody with the same weapon will have the same buff applied.\n*persists until restart unless manually reset by the player."));
							if (StarCitizen::Hooks::vars::cheat_bDamageMultiplier)
							{
								ImGui::SameLine();
								ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
								ImGui::SliderFloat(__("##DAMAGE_MP_SCALAR"), &StarCitizen::Hooks::vars::mDamageMPScalar, 0.0f, 3.f, "%.1f");
							}
						}
						ImGui::EndChild();

						ImGui::SameLine();
						child_player_szWndw = ImGui::GetContentRegionAvail();
						ImGui::BeginChild(__("##weapons_child_loadout"), child_player_szWndw, ImGuiChildFlags_AlwaysUseWindowPadding);
						{
							//	const char* charDefaultLoadoutouts[] = {
							//		__("DEFAULT"),
							//		__("HEALER"),
							//		__("MINER"),
							//		__("ZEUS"),
							//		__("WHALE MAN"),
							//		__("THE DIRECTOR"),
							//		__("TRACTOR BEAM"),
							//		__("COMBAT"),
							//		__("SALVAGER"),
							//		__("DEFAULT_1"),
							//		__("INVISIBLE"),
							//		__("HOLOGRAPHIC"),
							//		__("SKELETON")
							//	};
							static int charSelectedLoadout;
							bool bSetLoadout{ false };

							if (ImGui::Button(__("SET LOADOUT")))
							{
								if (std::find_if(Hooks::vars::loadout_history.begin(), Hooks::vars::loadout_history.end(),
									[&](const int& loadout)
									{
										return loadout == charSelectedLoadout;
									}) == Hooks::vars::loadout_history.end())
								{
									Hooks::vars::loadout_history.push_back(charSelectedLoadout);
								}
								
								bSetLoadout = true;
							}
							ImGui::SameLine();
							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
							//	ImGui::Combo(__("##LOADOUT_SELECTOR"), &charSelectedLoadout, ClassNames::v_LoadoutNames.data(), IM_ARRAYSIZE(charDefaultLoadoutouts));

							if (ImGui::BeginCombo("##LOADOUT_SELECTOR", ClassNames::v_LoadoutNames[charSelectedLoadout].c_str()))
							{
								for (int i = 0; i < ClassNames::v_LoadoutNames.size(); ++i)
								{
									bool isSelected = (charSelectedLoadout == i);
									if (ImGui::Selectable(ClassNames::v_LoadoutNames[i].c_str(), isSelected))
										charSelectedLoadout = i;

									if (isSelected)
										ImGui::SetItemDefaultFocus();
								}
								ImGui::EndCombo();
							}

							ImGui::SeparatorText(__("LOADOUT HISTORY"));

							ImGui::BeginChild(__("##weapons_child_loadout_history"), ImGui::GetContentRegionAvail(), ImGuiChildFlags_AlwaysUseWindowPadding);
							{
								for (auto loadout : Hooks::vars::loadout_history)
								{
									const auto& dw_selFlags = ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SelectOnClick;
									if (ImGui::Selectable(ClassNames::v_LoadoutNames[loadout].c_str(), false, dw_selFlags))
									{
										charSelectedLoadout = loadout;
										bSetLoadout = true;
									}
								}
							}
							ImGui::EndChild();

							if (bSetLoadout)
								StarCitizen::Cheats::SetDefaultLoadout(charSelectedLoadout);
						}
						ImGui::EndChild();
					}
					ImGui::EndChild();

					/* ENVIRONMENT */
					main_szWndw = ImGui::GetContentRegionAvail();
					ImGui::SeparatorText(__("ENVIRONMENT & VISUALS"));
					ImGui::BeginChild(__("##environment_main"), main_szWndw, child_resize_pad_y);
					{
						ImVec2 child_env_szWndw = ImGui::GetContentRegionAvail();
						ImGui::BeginChild(__("##environment_child_visuals"), ImVec2(child_env_szWndw.x * .3f, child_env_szWndw.y), child_resize_pad_x);
						{
							if (ImGui::Checkbox(__("ESP Interaction"), &Hooks::vars::grab_bEnable) && !Hooks::vars::grab_bEnable)
							{
								Hooks::vars::grab_pEntity = nullptr;
								Hooks::vars::grab_bGrab = false;
								Hooks::vars::grab_bGoTo = false;
							}
							ImGui::SameLine(); gui::widget::HelpMarker(__("highlites the closest rendered actor to crosshair. A line will be drawn to the actor if forge controls are available.\nGRAB: R-ALT + X\nGOTO: LALT + V\nSET TARGET: LALT + T\nNOTE: ESP of Type must be enabled"));
							ImGui::SameLine(); ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
							ImGui::SliderFloat(__("##grab_range"), &Hooks::vars::grab_fDistance, 1.0f, 10.f, "%.1f");
							ImGui::SameLine(); gui::widget::Tooltip(__("GoTo / Grab Distance modifier."));

							if (ImGui::Checkbox(__("Glare"), &StarCitizen::Hooks::vars::cheat_bDisableGlare))
								StarCitizen::Cheats::SetDisableGlare(StarCitizen::Hooks::vars::cheat_bDisableGlare);
							ImGui::SameLine(); gui::widget::HelpMarker(__("toggle glare effects."));

							if (ImGui::Checkbox(__("Fog"), &StarCitizen::Hooks::vars::cheat_bNoFog))
								StarCitizen::Cheats::SetNoFog(StarCitizen::Hooks::vars::cheat_bNoFog);
							ImGui::SameLine(); gui::widget::HelpMarker(__("toggle fog effects."));

							if (ImGui::Checkbox(__("Remove Camera Shakes"), &StarCitizen::Hooks::vars::cheat_bDisableAllShakes))
								StarCitizen::Cheats::SetDisableAllShakes(StarCitizen::Hooks::vars::cheat_bDisableAllShakes);
							ImGui::SameLine(); gui::widget::HelpMarker(__("disables most screen shake effects."));

							if (ImGui::Checkbox(__("Player Status"), &StarCitizen::Hooks::vars::cheat_bShowExtendedStatusHUD))
								StarCitizen::Cheats::SetHUDStatusEffects(StarCitizen::Hooks::vars::cheat_bShowExtendedStatusHUD);
							ImGui::SameLine(); gui::widget::HelpMarker(__("Show player status effects on HUD."));

							//  if (ImGui::Checkbox(__("DISABLE ATMOSPHERIC RESISTANCE"), &StarCitizen::Hooks::vars::bDisableAtmostphericResistance))
							//  	StarCitizen::Cheats::SetDisableAtmostphericResistance(StarCitizen::Hooks::vars::bDisableAtmostphericResistance);
							//  ImGui::SameLine(); gui::widget::HelpMarker(__("toggle atmospheric resistance effects."));
						}
						ImGui::EndChild();
						ImGui::SameLine();
						child_env_szWndw = ImGui::GetContentRegionAvail();
						ImGui::BeginChild(__("##environment_child_esp"), child_env_szWndw, ImGuiChildFlags_AlwaysUseWindowPadding);
						{
							// const ImGuiColorEditFlags dwColorEditFlags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel;

							ImVec2 szESPChild = ImGui::GetContentRegionAvail();

							auto fn = [&](const char* help, bool* bState, float* v, float* color, const char* fmt, const char* ceTag, const char* cbTag, const char* sfTag, ImVec2 minMax = ImVec2(0.0f, 10.0f))
							{
								szESPChild = ImGui::GetContentRegionAvail();
								float ESPChildSliderPos = szESPChild.x * .5f;
								const ImGuiColorEditFlags dwColorEditFlags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel;
								gui::widget::HelpMarker(help);
								ImGui::SameLine();
								ImGui::Checkbox(cbTag, bState);
								ImGui::SameLine();
								ImGui::ColorEdit4(ceTag, color, dwColorEditFlags);
								ImGui::SameLine();
								//	ImGui::SetCursorPosX(ESPChildSliderPos);
								ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
								ImGui::SliderFloat(sfTag, v, minMax.x, minMax.y, fmt);
							};


							fn(__("Player ESP"), 
								&StarCitizen::Hooks::vars::esp_bPlayer, 
								&StarCitizen::Hooks::vars::esp_PlayerRange, 
								(float*)&StarCitizen::Hooks::vars::esp_PlayerColor, 
								__("%.1f"), 
								__("##playerESP_COLOR"), 
								__("##playerESP"), 
								__("##playerESP_PLAYER_DIST")
							);

							fn(__("Enemy ESP"),
								&StarCitizen::Hooks::vars::esp_bEnemyAI,
								&StarCitizen::Hooks::vars::esp_EnemyAIRange,
								(float*)&StarCitizen::Hooks::vars::esp_EnemyAIColor,
								__("%.1f"),
								__("##enemyESP_COLOR"),
								__("##enemyESP"),
								__("##ESP_ENEMY_DIST")
							);

							fn(__("Actor ESP"),
								&StarCitizen::Hooks::vars::esp_bActors,
								&StarCitizen::Hooks::vars::esp_ActorRange,
								(float*)&StarCitizen::Hooks::vars::esp_ActorColor,
								__("%.1f"),
								__("##actorESP_COLOR"),
								__("##actorESP"),
								__("##actorESP_PLAYER_DIST")
							);

							fn(__("Animal ESP"),
								&StarCitizen::Hooks::vars::esp_bAnimals,
								&StarCitizen::Hooks::vars::esp_AnimalRange,
								(float*)&StarCitizen::Hooks::vars::esp_AnimalColor,
								__("%.1f"),
								__("##animalESP_COLOR"),
								__("##animalESP"),
								__("##ESP_ANIMAL_DIST")
							);

							fn(__("Ship ESP"),
								&StarCitizen::Hooks::vars::esp_bShip,
								&StarCitizen::Hooks::vars::esp_ShipRange,
								(float*)&StarCitizen::Hooks::vars::esp_ShipColor,
								__("%.1f"),
								__("##ShipESP_COLOR"),
								__("##ShipESP"),
								__("##ESP_SHIP_DIST")
							);

							fn(__("Mining ESP"),
								&StarCitizen::Hooks::vars::esp_bRock,
								&StarCitizen::Hooks::vars::esp_RockRange,
								(float*)&StarCitizen::Hooks::vars::esp_RockColor,
								__("%.1f"),
								__("##RockESP_COLOR"),
								__("##RockESP"),
								__("##ESP_ROCK_DIST")
							);

							fn(__("Loot ESP"),
								&StarCitizen::Hooks::vars::esp_bLoot,
								&StarCitizen::Hooks::vars::esp_LootRange,
								(float*)&StarCitizen::Hooks::vars::esp_LootColor,
								__("%.1f"),
								__("##LootESP_COLOR"),
								__("##LootESP"),
								__("##ESP_LOOT_DIST")
							);

							fn(__("Planet ESP"),
								&StarCitizen::Hooks::vars::esp_bOrbit,
								&StarCitizen::Hooks::vars::esp_OrbitRange,
								(float*)&StarCitizen::Hooks::vars::esp_OrbitColor,
								__("%.1f"),
								__("##OrbitESP_COLOR"),
								__("##OrbitESP"),
								__("##ESP_ORBIT_DIST")
							);

#if _DEBUG
							fn(__("DEBUG ESP"),
								&StarCitizen::Hooks::vars::esp_bDebug,
								&StarCitizen::Hooks::vars::esp_DebugRange,
								(float*)&StarCitizen::Hooks::vars::esp_DebugColor,
								__("%.1f"),
								__("##DebugESP_COLOR"),
								__("##DebugESP"),
								__("##ESP_DEBUG_DIST"),
								{ 0.0f, 100.f }
							);
#endif
						}
						ImGui::EndChild();
					}
					ImGui::EndChild();

					/* DEVELOPER */
#if _DEBUG
					ImGui::SeparatorText(__("DEVELOPER"));
					ScGui::BeginBlock(__("##dev_main"), ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 10.f), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY);
					{
						if (ImGui::Checkbox(__("Show Console"), &Hooks::vars::console_bShow))
							Helpers::ShowConsole(Hooks::vars::console_bShow);
						ImGui::SameLine(); gui::widget::HelpMarker(__("show / hide the console window."));

						ImGui::Checkbox((__("Demi God")), &Hooks::vars::cheat_bDemiGod);
						ImGui::SameLine(); gui::widget::HelpMarker(__("prevents the player from dying. also affects other entities."));

						ImGui::Checkbox(__("Blame Shooter"), &Hooks::vars::cheat_bBlameUser);
						ImGui::SameLine(); gui::widget::HelpMarker(__("spoof the shooter of a projectile. protects the local player from crimes."));

						ImGui::Checkbox(__("Mirror Force"), &Hooks::vars::cheat_bMirrorForce);
						ImGui::SameLine(); gui::widget::HelpMarker(__("mirror the force of a projectile. sends it back to shooter."));

						if (ImGui::Checkbox(__("Disable Loot Restrictions"), &Hooks::vars::cheat_bDisableLootRestrictions))
							Cheats::SetDisableCorpseLootingRestrictions(Hooks::vars::cheat_bDisableLootRestrictions);
						ImGui::SameLine(); gui::widget::HelpMarker(__("disable loot restrictions."));

						if (ImGui::Button(__("Heal")))
							Hooks::vars::cheat_bHealSelf = true;
						ImGui::SameLine(); gui::widget::HelpMarker(__("heal the local player."));

						if (ImGui::Button(__("Respawn")))
							Hooks::vars::cheat_bKillSelf = true;
						ImGui::SameLine(); gui::widget::HelpMarker(__("respawn the local player."));

						if (ImGui::Button(__("LAUNCH FREE FLIGHT")))
						{
						    //  StarCitizen::Hooks::vars::bLaunchFreeFlight = true;
						    StarCitizen::Structs::SXCommand cmd;
						    if (StarCitizen::Helpers::GetCmd(__("megamap"), &cmd))
						    {
						        char* cmds[3] = { (char*)cmd.Name, __("EA_Kareah_FreeFlight") };
						        cmd.ExecuteCmd(2, cmds);
						    }
						}

						if (ImGui::Button(__("DUMP PROCESS"), ImVec2(ImGui::GetContentRegionAvail().x, 0)))
						{
							DumpStructs();
							DumpCommands();
							DumpCVARS();
							DumpClasses();
						}

						ImGui::SeparatorText(__("CARGO SETTINGS"));
						ScGui::BeginBlock(__("##cargo_params"), { 0.0f, ImGui::GetTextLineHeightWithSpacing() * 4.f }, ImGuiChildFlags_Border);
						{
							ImGui::Checkbox(__("Fast Scraping"), &StarCitizen::Hooks::vars::salvage_bInstantFillRate);
							ImGui::SliderFloat(__("minSCU"), &Hooks::vars::salvage_minSCUContainer, 1.f, 32.f);
							ImGui::SliderFloat(__("maxSCU"), &Hooks::vars::salvage_maxSCUContainer, 1.f, 32.f);
						}
						ScGui::EndBlock();

						ImGui::SeparatorText(__("HIT RESULTS"));
						ScGui::BeginBlock(__("##HIT_RESULT_CHILD"), { 0.0f, ImGui::GetTextLineHeightWithSpacing() * 8.f }, ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY);
						{
							ImVec2 szChild = ImGui::GetContentRegionAvail();
							ImGui::BeginChild(__("##hit_result_child_child"), { szChild.x * .5f, szChild.y }, ImGuiChildFlags_ResizeX | ImGuiChildFlags_AlwaysUseWindowPadding);
							{
								const auto& hit = Hooks::vars::hit_lastHit;
								gui::widget::TextCentered(__("LAST HIT RESULT"));
								ImGui::Text("SHOOTER: %s : 0x%llX", hit.mShooterEntityName.c_str(), hit.pShooter);
								ImGui::Text("WEAPON: %s : 0x%llX", hit.mWeaponName.c_str(), hit.pWeapon);
								ImGui::Text("ENTITY HIT: %s : 0x%llX", hit.mHitEntityName.c_str(), hit.pEntity);
								ImGui::Text("HIT PART: %s : 0x%llX", hit.mHitPartName.c_str(), hit.pEntity_HitPart);
								if (ImGui::Button(__("COPY TO CLIPBOARD")))
								{
									Helpers::CopyToClipboard(__("%s : 0x%llX\n%s : 0x%llX\n%s : 0x%llX\n%s : 0x%llX"),
										hit.mHitEntityName.c_str(), hit.pEntity,
										hit.mHitPartName.c_str(), hit.pEntity_HitPart,
										hit.mShooterEntityName.c_str(), hit.pShooter,
										hit.mWeaponName.c_str(), hit.pWeapon
									);
								}
							}
							ImGui::EndChild();
							ImGui::SameLine();
							ImGui::BeginChild(__("##hit_result_child_child_child"), ImGui::GetContentRegionAvail(), ImGuiChildFlags_AlwaysUseWindowPadding);
							{
								const auto& hit = Hooks::vars::hit_lastHitByPlayer;
								gui::widget::TextCentered(__("LAST HIT RESULT BY PLAYER"));
								ImGui::Text("SHOOTER: %s : 0x%llX", hit.mShooterEntityName.c_str(), hit.pShooter);
								ImGui::Text("WEAPON: %s : 0x%llX", hit.mWeaponName.c_str(), hit.pWeapon);
								ImGui::Text("ENTITY HIT: %s : 0x%llX", hit.mHitEntityName.c_str(), hit.pEntity);
								ImGui::Text("HIT PART: %s : 0x%llX", hit.mHitPartName.c_str(), hit.pEntity_HitPart);
								if (ImGui::Button(__("COPY TO CLIPBOARD")))
								{
									Helpers::CopyToClipboard(__("%s : 0x%llX\n%s : 0x%llX\n%s : 0x%llX\n%s : 0x%llX"),
										hit.mHitEntityName.c_str(), hit.pEntity,
										hit.mHitPartName.c_str(), hit.pEntity_HitPart,
										hit.mShooterEntityName.c_str(), hit.pShooter,
										hit.mWeaponName.c_str(), hit.pWeapon
									);
								}
							}
							ImGui::EndChild();

						}
						ScGui::EndBlock();
					}
					ScGui::EndBlock();
#endif
				}



				struct SCDumperVarHistory
				{
					Structs::SXCvar var;			// variable that will be passed to hooks::vars::cvar_selection
				};
				struct SCDumperCommandHistory 
				{
					struct args
					{
						int vCount = 0;
						char v[6] = { 0, 0, 0, 0, 0, 0 };
					};
					Structs::SXCommand cmd;			// command that will be passed to hooks::vars::cmd_selection
					std::vector<args> vArgs;		// history of arguments for this command
				};
				void Dumper()
				{
					///	DESIGN
					/*
						 ____________________________________
						|              OfflineCitizen             |
						|____________________________________|
						|____________|___________|___________|
						|  ________________________________  |
						| |VAR|                   |__MOD___| |
						| | ~~~~~~~~~~~~~~~~~~~~~ | ~~~~~~ | |
						| | ~~~~~~~~~~~~~~~~~~~~~ | ~~~~~~ | |
						| | ~~~~~~~~~~~~~~~~~~~~~ | ~~~~~~ | |
						| |_______________________|________| |
						| |CMD                    |__EXEC__| |
						| | ~~~~~~~~~~~~~~~~~~~~~ | ~~~~~~ | |
						| | ~~~~~~~~~~~~~~~~~~~~~ | ~~~~~~ | |
						| | ~~~~~~~~~~~~~~~~~~~~~ | ~~~~~~ | |
						| |_______________________|________| |
						|____________________________________|

					*/

					/* window dimensions */
					auto& style = ImGui::GetStyle();
					auto width = ImGui::GetContentRegionAvail().x;
					auto draw = ImGui::GetWindowDrawList();
					auto pos = ImGui::GetWindowPos();
					auto size = ImGui::GetWindowSize();

					/* declare arrays */
					auto& ents_all = StarCitizen::Hooks::vars::vAllEntities;					//	all entities
					auto& players = StarCitizen::Hooks::vars::vPlayerEntities;					//	player entities
					auto& ents_ships = StarCitizen::Hooks::vars::vShipEntities;					//	ship entities
					auto& ents_loot = StarCitizen::Hooks::vars::vLootEntities;					//	ship entities
					auto& ents_rocks = StarCitizen::Hooks::vars::vRockEntities;					//	ship entities
					auto& ents_kiosks = StarCitizen::Hooks::vars::vKioskEntities;				//	ship entities
					auto& ents_missions = StarCitizen::Hooks::vars::vMissionEntities;			//	ship entities
					auto& waypoints = StarCitizen::Hooks::vars::vWaypoints;						//	waypoints
					auto& vars = StarCitizen::Hooks::vars::vConsoleVariables;					//	cvars
					auto& commands = StarCitizen::Hooks::vars::vConsoleCommands;				//	commands
					auto& names = StarCitizen::Hooks::vars::class_names;						//	class names
					auto& class_names_count = StarCitizen::Hooks::vars::class_names_count;		//	class names count
					auto& structs = StarCitizen::Hooks::vars::vStructNames;						//	structs

					/* declare selections */
					static bool bSelectedEntity = false;
					static bool bSelectedCvar = false;
					static bool bSelectedWaypoint = false;
					static bool bSelectedCmd = false;
					static bool bSelectedStruct = false;
					static bool bObtainedStructData = false;
					static char waypoint_input_buffer[MAX_PATH];
					static char selection_input_buffer[MAX_PATH];
					static char cmd_params_input_buffer[MAX_PATH];
					static char goto_input_buffer[MAX_PATH];
					static std::string selected_struct;
					static StarCitizen::Structs::SWaypoint selected_wp;
					static StarCitizen::Structs::SXCvar selected_cvar;
					static std::vector<Structs::STargetEntity> player_history;
					static std::vector<std::string> struct_history;
					static std::vector<Structs::SWaypoint> wp_history;
					static std::vector<SCDumperVarHistory> var_history;
					static std::vector<SCDumperCommandHistory> command_history;

					/* FINDER */
					static int selected_category = 0;
					ImVec2 szwndw = ImVec2(ImGui::GetContentRegionAvail().x * .40f, ImGui::GetContentRegionAvail().y);

					bool bSelectedPlayer = Hooks::vars::target_player.bValid;
					ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f); // Disable rounding
					ImGui::BeginChild(__("##CHILD_DUMPER_FINDER"), szwndw, ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
					{
						ImVec2 szwndw = ImGui::GetContentRegionAvail();
						ImGui::BeginChild(__("##TABS_CHILD"), { szwndw.x, szwndw.y * .05f }, false);
						{
#if _DEBUG
							static const char* categories[] = { "ENTITIES", "POI", "CVARS", "COMMANDS", "STRUCTS", "NAMES" };
#else
							static const char* categories[] = { "ENTITIES", "POI", "CVARS", "COMMANDS" };
#endif
							const ImVec2 child_pos = ImGui::GetCursorScreenPos();
							const ImVec2 child_szWndw = ImGui::GetContentRegionAvail();
							const ImVec2 child_size(ImGui::GetContentRegionAvail().x, child_szWndw.y);

							ImU32 glowColor = IM_COL32(255, 255, 255, 120); // Semi-transparent blue glow
							for (float offset = 4.0f; offset > 0.0f; offset -= 1.0f)
							{
								ImVec2 glowMin = child_pos + ImVec2(-4, -4);
								ImVec2 glowMax = child_pos + child_size + ImVec2(4, 4);
								draw->AddRect(
									glowMin + ImVec2(offset, offset),	//	min
									glowMax - ImVec2(offset, offset),	//	max
									glowColor,							// color
									2.f,								// rounding
									0,									// flags	
									1.f									// thickness						
								);
							}

#if _DEBUG
							for (int i = 0; i < 6; i++)
#else
							for (int i = 0; i < 4; i++)
#endif
							{
								auto szWndw = ImGui::GetContentRegionAvail();
								ImVec2 buttonPos = ImGui::GetCursorScreenPos();
#if _DEBUG
								float buttonWidth = szWndw.x * ((i == 0) ? 0.16f : (i == 1) ? 0.20f : (i == 2) ? 0.25f : (i == 3) ? 0.33f : (i  == 4) ? 0.5f : 1.0f);
#else
								float buttonWidth = szWndw.x * ((i == 0) ? 0.25f : (i == 1) ? 0.33f : (i == 2) ? 0.5f : 1.f);
#endif
								ImVec2 buttonSize(buttonWidth, szWndw.y);

								if (ImGui::Button(categories[i], buttonSize))
									selected_category = i;

								if (selected_category == i)
								{
									ImDrawList* drawList = ImGui::GetWindowDrawList();
									ImVec2 glowMin = buttonPos;
									ImVec2 glowMax = buttonPos + buttonSize;

									ImU32 glowColor = IM_COL32(0, 150, 255, 120); // Semi-transparent blue glow

									for (float offset = 4.0f; offset > 0.0f; offset -= 1.0f)
									{
										drawList->AddRect(glowMin + ImVec2(offset, offset),
											glowMax - ImVec2(offset, offset),
											glowColor,
											0.0f, 
											0,
											1.0f
										); // Rounded corners & thickness
									}
								}

#if _DEBUG
								if (i < 5) ImGui::SameLine(0, 0);
#else
								if (i < 3) ImGui::SameLine(0, 0);
#endif
							}
						}
						ImGui::EndChild();

						/* draw rect border for input text field */
						const auto& dwFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll;
						ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
						ImGui::InputTextWithHint(__("##DUMPER_FIND_ITEM_INPUT"), __("start typing to search list"), selection_input_buffer, MAX_PATH, dwFlags);

						ImGui::PopStyleVar();	//	Enable frame rounding
						ImGui::Separator();

						std::string search = selection_input_buffer;
						std::transform(search.begin(), search.end(), search.begin(), ::tolower);
						ImGui::BeginChild(__("##DUMPER_ITEMS_LIST"), ImGui::GetContentRegionAvail(), ImGuiChildFlags_FrameStyle);
						{
							ImVec2 szFinderWndw = ImGui::GetContentRegionAvail();

							int i = 0;
							switch (selected_category)
							{
								/* ENTITIES */
							case 0:
								ImGui::SeparatorText(__("PLAYERS"));
								szFinderWndw = ImGui::GetContentRegionAvail();
								ImGui::BeginChild(__("finder_entities_players"), { szFinderWndw.x, szFinderWndw.y * .16f }, ImGuiChildFlags_ResizeY);
								{
									int playerID = -1;
									for (Classes::CEntity* pPlayerEntity : players)
									{
										playerID++;
										if (!Helpers::IsValidPtr(pPlayerEntity) || !pPlayerEntity->pName)
											continue;

										/* compare name with search string */
										std::string name = (char*)pPlayerEntity->pName;
										std::string& lowerName = name;
										std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
										if (!search.empty() && lowerName.find(search) == std::string::npos)
											continue;

										ImGui::PushID(playerID);
										if (ImGui::Selectable(name.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SelectOnClick))
										{
											Hooks::vars::target_pSelection = pPlayerEntity;
											Hooks::vars::target_bSetNewPlayer = true;
											bSelectedPlayer = true;
										}

										///	UPDATE SELECTION HISTORY
										//	/* find entity in history array */
										//	const auto& it = std::find_if(player_history.begin(), player_history.end(), [&](const Structs::STargetEntity& xWp)
										//		{ return pPlayerEntity->pName == pPlayerEntity->pName; });
										//	
										//	/* update entity history */
										//	if (it == wp_history.end())
										//	{
										//		/* store new var to the history */
										//		player_history.push_back(pPlayerEntity);
										//	}
										ImGui::PopID();

									}
								}
								ImGui::EndChild();
								ImGui::SeparatorText(__("SHIPS"));
								szFinderWndw = ImGui::GetContentRegionAvail();
								ImGui::BeginChild(__("##finder_entities_ships"), { szFinderWndw.x, szFinderWndw.y * .20f }, ImGuiChildFlags_ResizeY);
								{
									int shipID = -1;
									for (Classes::CEntity* pShipEntity : ents_ships)
									{
										shipID++;
										if (!Helpers::IsValidPtr(pShipEntity) || !pShipEntity->pName)
											continue;

										/* compare name with search string */
										std::string name = (char*)pShipEntity->pName;
										std::string& lowerName = name;
										std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
										if (!search.empty() && lowerName.find(search) == std::string::npos)
											continue;

										ImGui::PushID(shipID);
										if (ImGui::Selectable(name.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SelectOnClick))
										{
											Hooks::vars::target_pSelection = pShipEntity;
											Hooks::vars::target_bSetNewPlayer = true;
											bSelectedPlayer = true;
										}
										ImGui::PopID();
									}
								}
								ImGui::EndChild();
								ImGui::SeparatorText(__("KIOSKS"));
								szFinderWndw = ImGui::GetContentRegionAvail();
								ImGui::BeginChild(__("##finder_entities_kiosks"), { szFinderWndw.x, szFinderWndw.y * .25f }, ImGuiChildFlags_ResizeY);
								{
									int shipID = -1;
									for (Classes::CEntity* pKioskEntity : ents_kiosks)
									{
										shipID++;
										if (!Helpers::IsValidPtr(pKioskEntity) || !pKioskEntity->pName)
											continue;

										/* compare name with search string */
										std::string name = (char*)pKioskEntity->pName;
										std::string& lowerName = name;
										std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
										if (!search.empty() && lowerName.find(search) == std::string::npos)
											continue;

										ImGui::PushID(shipID);
										if (ImGui::Selectable(name.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SelectOnClick))
										{
											Hooks::vars::target_pSelection = pKioskEntity;
											Hooks::vars::target_bSetNewPlayer = true;
											bSelectedPlayer = true;
										}
										ImGui::PopID();
									}
								}
								ImGui::EndChild();
								ImGui::SeparatorText(__("LOOT"));
								szFinderWndw = ImGui::GetContentRegionAvail();
								ImGui::BeginChild(__("##finder_entities_loot"), { szFinderWndw.x, szFinderWndw.y * .33f }, ImGuiChildFlags_ResizeY);
								{
									int shipID = -1;
									for (Classes::CEntity* pLootEntity : ents_loot)
									{
										shipID++;
										if (!Helpers::IsValidPtr(pLootEntity) || !pLootEntity->pName)
											continue;

										/* compare name with search string */
										std::string name = (char*)pLootEntity->pName;
										std::string& lowerName = name;
										std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
										if (!search.empty() && lowerName.find(search) == std::string::npos)
											continue;

										ImGui::PushID(shipID);
										if (ImGui::Selectable(name.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SelectOnClick))
										{
											Hooks::vars::target_pSelection = pLootEntity;
											Hooks::vars::target_bSetNewPlayer = true;
											bSelectedPlayer = true;
										}
										ImGui::PopID();
									}
								}
								ImGui::EndChild();
								ImGui::SeparatorText(__("MINEABLES"));
								szFinderWndw = ImGui::GetContentRegionAvail();
								ImGui::BeginChild(__("##finder_entities_mining"), { szFinderWndw.x, szFinderWndw.y * .50f }, ImGuiChildFlags_ResizeY);
								{
									int shipID = -1;
									for (Classes::CEntity* pRockEntity : ents_rocks)
									{
										shipID++;
										if (!Helpers::IsValidPtr(pRockEntity) || !pRockEntity->pName)
											continue;

										/* compare name with search string */
										std::string name = (char*)pRockEntity->pName;
										std::string& lowerName = name;
										std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
										if (!search.empty() && lowerName.find(search) == std::string::npos)
											continue;

										ImGui::PushID(shipID);
										if (ImGui::Selectable(name.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SelectOnClick))
										{
											Hooks::vars::target_pSelection = pRockEntity;
											Hooks::vars::target_bSetNewPlayer = true;
											bSelectedPlayer = true;
										}
										ImGui::PopID();
									}
								}
								ImGui::EndChild();
								ImGui::SeparatorText(__("MISSION OBJECTIVES"));
								szFinderWndw = ImGui::GetContentRegionAvail();
								ImGui::BeginChild(__("##finder_entities_missions"), szFinderWndw, ImGuiChildFlags_ResizeY);
								{
									int shipID = -1;
									for (Classes::CEntity* pMissionEntity : ents_missions)
									{
										shipID++;
										if (!Helpers::IsValidPtr(pMissionEntity) || !pMissionEntity->pName)
											continue;

										/* compare name with search string */
										std::string name = (char*)pMissionEntity->pName;
										std::string& lowerName = name;
										std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
										if (!search.empty() && lowerName.find(search) == std::string::npos)
											continue;

										ImGui::PushID(shipID);
										if (ImGui::Selectable(name.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SelectOnClick))
										{
											Hooks::vars::target_pSelection = pMissionEntity;
											Hooks::vars::target_bSetNewPlayer = true;
											bSelectedPlayer = true;
										}
										ImGui::PopID();
									}
								}
								ImGui::EndChild();

								///	@TODO: SLOW
								//	ImGui::SeparatorText(__("ALL"));
								//	ImGui::BeginChild(__("##finder_entities_all"), ImGui::GetContentRegionAvail());
								//	{
								//		int entityID = -1;
								//		for (const auto& ent : ents_all)
								//		{
								//			entityID++;
								//			if (!ent || !ent->pName)
								//				continue;
								//	
								//			/* compare name with search string */
								//			std::string name = (char*)ent->pName;
								//			if (!search.empty())
								//			{
								//				std::string& lowerName = name;
								//				std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
								//				if (lowerName.find(search) == std::string::npos)
								//					continue;
								//			}
								//	
								//			ImGui::PushID(entityID);
								//			if (ImGui::Selectable(name.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SelectOnClick))
								//			{
								//				Hooks::vars::target_pSelection = ent;
								//				Hooks::vars::target_bSetNewPlayer = true;
								//				bSelectedPlayer = true;
								//			}
								//			ImGui::PopID();
								//		}
								//	}
								//	ImGui::EndChild();

								break;

								/* POI */
							case 1:
								for (const auto& wp : waypoints)
								{
									std::string lowerName = wp.mName;
									std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
									if (!search.empty() && lowerName.find(search) == std::string::npos)
										continue;

									if (ImGui::Selectable(wp.mName.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SelectOnClick))
									{
										selected_wp = wp;
										bSelectedWaypoint = true;
									}

									/* find command in history array */
									const auto& it = std::find_if(wp_history.begin(), wp_history.end(), [&](const Structs::SWaypoint& xWp)
										{ return xWp.mName == wp.mName && xWp.pZoneEntity == wp.pZoneEntity; });

									/* update command history */
									if (it == wp_history.end())
									{
										/* store new var to the history */
										wp_history.push_back(wp);
									}
								}
								break;

								/* CVAR */
							case 2:
								for (const auto& var : vars)
								{
									if (var.Name == nullptr)
										continue;

									std::string lowerName = var.Name;
									std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
									if (!search.empty() && lowerName.find(search) == std::string::npos)
										continue;

									if (ImGui::Selectable(var.Name, false, ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SelectOnClick))
									{
										selected_cvar = var;
										bSelectedCvar = true;

										/* find var in history array */
										const auto& it = std::find_if(var_history.begin(), var_history.end(), [&](const SCDumperVarHistory& xvar)
											{ return xvar.var.Name == var.Name; });

										/* update var history */
										if (it == var_history.end())
										{
											/* store new var to the history */
											SCDumperVarHistory lastVar;
											lastVar.var = var;

											///	@TODO : store params
											//	for (int i = 0; i < Hooks::vars::cmd_count; i++)
											//	{
											//		lastArgs.v[i] = Hooks::vars::cmd_args[i];
											//	}
											//	lastCommand.vArgs.push_back(lastArgs);
											var_history.push_back(lastVar);
										}
									}
								}
								break;

								/* COMMAND */
							case 3:
								for (const auto& cmd : commands)
								{
									if (cmd.Name == nullptr)
										continue;
									std::string lowerName = cmd.Name;
									std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
									if (!search.empty() && lowerName.find(search) == std::string::npos)
										continue;

									if (ImGui::Selectable(cmd.Name, false, ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SelectOnClick))
									{
										Hooks::vars::cmd_selection = cmd;
										bSelectedCmd = true;
									}
								}
								break;

#if _DEBUG
								/* STRUCT */
							case 4:
								for (const auto& str : structs)
								{
									std::string lowerName = str;
									std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
									if (!search.empty() && lowerName.find(search) == std::string::npos)
										continue;

									if (ImGui::Selectable(str.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SelectOnClick))
									{
										bObtainedStructData = false;
										bSelectedStruct = true;
										selected_struct = str;

										/* find struct in history array */
										const auto& it = std::find_if(struct_history.begin(), struct_history.end(), [&](const std::string& x)
											{ return x == str; });

										/* update struct history */
										if (it == struct_history.end())
										{
											struct_history.push_back(str);
										}
									}
								}
								break;

								/* NAME */
							case 5:
								do
								{
									if (names[i] == nullptr)
										break;

									std::string lowerName = names[i];
									std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
									if (!search.empty() && lowerName.find(search) == std::string::npos)
									{
										i++;
										continue;
									}

									if (ImGui::Selectable(names[i], false, ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SelectOnClick))
									{
										// @TODO: implement selection
									}
									i++;
								} while (i < class_names_count);
								break;
#endif
							default: break;
							}
						}
						ImGui::EndChild();
					}
					ImGui::EndChild();

					ImGui::SameLine();

					/* EXECUTOR */
					ImGui::BeginChild(__("##CHILD_DUMPER_EXEC"), ImGui::GetContentRegionAvail(), false);
					{
						static int input_int{ 0 };
						static float input_float{ 0.0f };

						switch (selected_category)
						{
							/* PLAYERS */
						case 0:
							[&]()
							{
								if (!bSelectedPlayer)
									return;

								auto& selected_player = Hooks::vars::target_player;
								if (selected_player.bValid)
								{
									/* display player editor */
									auto szExecWndw = ImGui::GetContentRegionAvail();
									ImVec2 szExecHeaderWndw = { szExecWndw.x, szExecWndw.y * .6f };
									ImGui::BeginChild(__("##player_EXECUTOR_HEADER"), szExecHeaderWndw, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_ResizeY);
									{
										gui::widget::TextCentered(selected_player.mName.c_str());
										const ImGuiColorEditFlags dwColorEditFlags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel;
										ImGui::ColorEdit4(__("##player_TAG_COLOR"), (float*)&Hooks::vars::target_Color, dwColorEditFlags);
										ImGui::SameLine(); gui::widget::HelpMarker(__("adjusts the player name tag color."));

										ImGui::Checkbox(__("RENDER"), &selected_player.bRender);
										ImGui::SameLine(); gui::widget::HelpMarker(__("shows the target player nametag in the game world."));

										ImGui::Checkbox(__("STICK"), &selected_player.bSticky);
										ImGui::SameLine(); gui::widget::HelpMarker(__("sticks the local player to the target player."));

										ImGui::Checkbox(__("FORGE"), &selected_player.bForge);
										ImGui::SameLine(); gui::widget::HelpMarker(__("transform the target entity position & rotation."));

										if (ImGui::Button(__("TELEPORT")))
											selected_player.bTeleport |= true;
										gui::widget::Tooltip(__("teleports to the target player. attempts to place the local player behind the target."));

										if (ImGui::Button(__("CLEAR SELECTION")))
										{
											bSelectedPlayer = false;
											selected_player.bValid = false;
										}
#if _DEBUG
										ImGui::BeginChild(__("##player_EXECUTE_debug"), ImGui::GetContentRegionAvail(), ImGuiChildFlags_Border | ImGuiChildFlags_AlwaysUseWindowPadding);
										{
											auto SetClipboard = [](const char* buff, ...)
												{
													char buffer[1024];
													va_list args;
													va_start(args, buff);
													vsnprintf(buffer, sizeof(buffer), buff, args);
													va_end(args);
													ImGui::SetClipboardText(buffer);
												};

											if (const auto& pEntity = selected_player.pEntity)
											{
												if (ImGui::Selectable(__("mID")))
												{
													SetClipboard(__("EntityID: 0x%04X"), pEntity->nID);
												}
												//	if (ImGui::Selectable(__("mGUID")))
												//	{
												//		SetClipboard(__("mGUID: 0x%04X"), pEntity->mGUID);
												//	}
												if (ImGui::Selectable(__("mFlag")))
												{
													SetClipboard(__("mFlag: 0x%08X"), pEntity->flags);
												}
												//	if (ImGui::Selectable(__("mEntFlag")))
												//	{
												//		SetClipboard(__("mEntFlag: 0x%04X"), pEntity->mSeatFlag);
												//	}
												if (ImGui::Selectable(__("pEntityClass")))
												{
													SetClipboard(__("pEntityClass: 0x%llX"), EXTRACT_LOWER_BYTES((__int64)pEntity->pEntityClass));
												}
												if (ImGui::Selectable(__("pComponents")))
												{
													SetClipboard(__("pComponents: 0x%llX"), EXTRACT_LOWER_BYTES((__int64)pEntity->pComponents));
												}
												if (ImGui::Selectable(__("pLocalZone")))
												{
													SetClipboard(__("pLocalZone: 0x%llX"), EXTRACT_LOWER_BYTES((__int64)pEntity->pLocalZone));
												}
												//	if (ImGui::Selectable(__("pParentEntity")))
												//	{
												//		SetClipboard(__("pParentEntity: 0x%llX"), EXTRACT_LOWER_BYTES((__int64)pEntity->pParentEntity));
												//	}
												//	if (ImGui::Selectable(__("pUnkEntity")))
												//	{
												//		SetClipboard(__("pUnkEntity: 0x%llX"), EXTRACT_LOWER_BYTES((__int64)pEntity->pSelfEntity));
												//	}
												if (ImGui::Selectable(__("pSeatEntity")))
												{
													SetClipboard(__("pSeatEntity: 0x%llX"), EXTRACT_LOWER_BYTES((__int64)pEntity->pSeatEntity));
												}
												if (ImGui::Selectable(__("pLocalZoneEntity")))
												{
													SetClipboard(__("pLocalZoneEntity: 0x%llX"), EXTRACT_LOWER_BYTES((__int64)pEntity->pLocalZoneEntity));
												}
												//	if (ImGui::Selectable(__("pActorEntity")))
												//	{
												//		SetClipboard(__("pActorEntity: 0x%llX"), EXTRACT_LOWER_BYTES((__int64)pEntity->pActorEntity));
												//	}
											}
										}
										ImGui::EndChild();
#endif
									}
									ImGui::EndChild();
								}

								ImGui::SeparatorText(__("HISTORY"));

								/* display target player history */
								ImGui::BeginChild(__("##player_EXECUTE_FOOTER"), ImGui::GetContentRegionAvail(), false);
								{
									const ImVec2 szWndwAvail = ImGui::GetContentRegionAvail();
									const ImVec2& szChildHisCMD = ImVec2(szWndwAvail.x * .3f, szWndwAvail.y);
									ImGui::BeginChild(__("##player_history"), szChildHisCMD, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_ResizeX);
									{
										for (auto& pPlayer : player_history)
										{
											if (!pPlayer.bValid || !pPlayer.pEntity)
											{
												//	@TODO: flag for removal from original array ?
												continue;
											}

											const auto& dw_selFlags = ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SelectOnClick;
											if (ImGui::Selectable(pPlayer.mName.c_str(), false, dw_selFlags))
											{
												Hooks::vars::target_pSelection = pPlayer.pEntity;
												Hooks::vars::target_bSetNewPlayer = true;
												bSelectedPlayer = true;
											}
										}
									}
									ImGui::EndChild();

									ImGui::SameLine();

									ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
									ImGui::BeginChild(__("##player_edit_history"), ImGui::GetContentRegionAvail(), true);
									{
									}
									ImGui::EndChild();
									ImGui::PopStyleVar();
								}
								ImGui::EndChild();
							}();
							break;

							/* POI */
						case 1:
							[&]()
							{
								/* display new waypoint input */
								auto szExecWndw = ImGui::GetContentRegionAvail();
								ImGui::BeginChild(__("##waypoint__EXECUTE_HEADER_1"), {szExecWndw.x, ImGui::GetTextLineHeightWithSpacing() * 3.f}, true);
								{
									gui::widget::TextCentered(__("CREATE NEW WAYPOINT"));
									const auto& dwFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll;
									ImVec2 szWaypointChild = ImGui::GetContentRegionAvail();
									ImGui::SetNextItemWidth(szWaypointChild.x * .75f);
									if (ImGui::InputTextWithHint(__("##waypoint_input_entry_field"), __("enter waypoint name"), waypoint_input_buffer, MAX_PATH, dwFlags))
									{
										std::string name = waypoint_input_buffer;
										if (!name.empty())
										{
											Hooks::vars::wp_TargetName = name;
											Hooks::vars::wp_bSetNewPoint = true;
										}
										memset(waypoint_input_buffer, 0, MAX_PATH);
									}
									ImGui::SameLine();
									if (ImGui::Button(__("SET"), ImVec2(ImGui::GetContentRegionAvail().x, 0.f)))
									{
										std::string name = waypoint_input_buffer;
										if (!name.empty())
										{
											Hooks::vars::wp_TargetName = name;
											Hooks::vars::wp_bSetNewPoint = true;
										}
										memset(waypoint_input_buffer, 0, MAX_PATH);
									}
								}
								ImGui::EndChild();

								/* display waypoint editor */
								szExecWndw = ImGui::GetContentRegionAvail();
								ImVec2 szExecHeaderWndw = { szExecWndw.x, szExecWndw.y * .25f };
								if (bSelectedWaypoint)
								{
									int updateType = 0;
									bool bRequiresUpdate = false;
									ImGui::BeginChild(__("##waypoint_EXECUTE_HEADER"), szExecHeaderWndw, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_ResizeY);
									{
										gui::widget::TextCentered(selected_wp.mName.c_str());
										const ImGuiColorEditFlags dwColorEditFlags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel;
										bRequiresUpdate = ImGui::ColorEdit4(__("##WAYPOINT_COLOR"), (float*)&selected_wp.mColor, dwColorEditFlags);
										ImGui::SameLine(); gui::widget::HelpMarker(__("adjusts the waypoint name tag color."));

										bRequiresUpdate = ImGui::Checkbox(__("RENDER"), &selected_wp.bRender);
										ImGui::SameLine(); gui::widget::HelpMarker(__("shows the waypoint nametag in the game world."));

										bRequiresUpdate = ImGui::Checkbox(__("LOCK"), &selected_wp.bLockToWaypoint);
										ImGui::SameLine(); gui::widget::HelpMarker(__("locks the player to the waypoint position."));

										if (ImGui::Button(__("TELEPORT")))
										{
											selected_wp.bTeleport |= true;
											bRequiresUpdate = true;
										}
										gui::widget::Tooltip(__("teleports to waypoint."));
									}
									ImGui::EndChild();

									/* update entry in waypoints array */
									if (bRequiresUpdate)
									{
										auto it = std::find_if(waypoints.begin(), waypoints.end(), [&](Structs::SWaypoint& wp)
											{
												return (wp.mName == selected_wp.mName && wp.pZoneEntity == selected_wp.pZoneEntity);
											});

										if (it != waypoints.end())
										{
											 it->mColor = selected_wp.mColor;
											 it->bRender = selected_wp.bRender;
											 it->bLockToWaypoint = selected_wp.bLockToWaypoint;
											 it->bTeleport = selected_wp.bTeleport;
											
										}
									}
										
								}
								ImGui::SeparatorText(__("HISTORY"));

								/* display waypoint history */
								ImGui::BeginChild(__("##waypoint_EXECUTE_FOOTER"), ImGui::GetContentRegionAvail(), false);
								{

									const ImVec2 szWndwAvail = ImGui::GetContentRegionAvail();
									const ImVec2& szChildHisCMD = ImVec2(szWndwAvail.x * .3f, szWndwAvail.y);
									ImGui::BeginChild(__("##waypoint_history"), szChildHisCMD, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_ResizeX);
									{
										for (auto& wp : wp_history)
										{
											const auto& dw_selFlags = ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SelectOnClick;
											if (ImGui::Selectable(wp.mName.c_str(), false, dw_selFlags))
											{
												selected_wp = wp;
												bSelectedWaypoint = true;
											}
										}
									}
									ImGui::EndChild();

									ImGui::SameLine();

									ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
									ImGui::BeginChild(__("##waypoint_edit_history"), ImGui::GetContentRegionAvail(), true);
									{
										ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
										if (ImGui::InputTextWithHint(xorstr_("##go_to_point_entry"), xorstr_("GO TO POINT NAME"), goto_input_buffer, MAX_PATH, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll) && !std::string(goto_input_buffer).empty())
										{
											StarCitizen::Hooks::vars::goto_TargetName = goto_input_buffer;
											StarCitizen::Hooks::vars::goto_bFindPoint = true;
											memset(goto_input_buffer, 0, MAX_PATH);
										}

										ImGui::Separator();

										for (auto point : StarCitizen::Hooks::vars::vGoToPoints)
										{
											if (!point->m_pPointName)
												continue;

											const auto& point_name = (char*)point->m_pPointName;
											if (ImGui::Selectable(point_name) || ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter))
											{
												StarCitizen::Hooks::vars::goto_TargetName = point_name;
												StarCitizen::Hooks::vars::goto_bFindPoint = true;
											}
										}
									}
									ImGui::EndChild();
									ImGui::PopStyleVar();
								}
								ImGui::EndChild();
							}();
							break;

							/* CVARS */
						case 2:
							[&]()
							{
								if (!bSelectedCvar)
									return;

								/* display command editor */
								auto szExecWndw = ImGui::GetContentRegionAvail();
								ImVec2 szExecHeaderWndw = { szExecWndw.x, szExecWndw.y * .25f };
								ImGui::BeginChild(__("##cvar_EXECUTOR_HEADER"), szExecHeaderWndw, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_ResizeY);
								{
									gui::widget::TextCentered(selected_cvar.Name);
									if (selected_cvar.Description != nullptr)
										ImGui::TextWrapped(selected_cvar.Description);

									switch (selected_cvar.type)
									{
									case StarCitizen::Enums::ECvarType::CVARTYPE_INT:
										ImGui::Text("Value: %d", selected_cvar.GetValue<int>());
										if (ImGui::InputInt(__("##CVAR_EDIT_INT"), &input_int))
											selected_cvar.SetValue<int>(input_int);
										ImGui::SameLine();
										if (ImGui::Button(__("FORCE")))
											selected_cvar.SetValue<int>(input_int);
										ImGui::SameLine();  gui::widget::HelpMarker(__("applies current value."));
										break;

									case StarCitizen::Enums::ECvarType::CVARTYPE_FLOAT:
										ImGui::Text("Value: %.2f", selected_cvar.GetValue<float>());
										if (ImGui::InputFloat(__("##CVAR_EDIT_FLOAT"), &input_float, 1.f, 10.0f, "%.2f"))
											selected_cvar.SetValue<float>(input_float);
										ImGui::SameLine();
										if (ImGui::Button(__("FORCE")))
											selected_cvar.SetValue<float>(input_int);
										ImGui::SameLine();  gui::widget::HelpMarker(__("applies current value."));
										break;

									default: break;

										//  case StarCitizen::Enums::ECvarType::CVARTYPE_INT64:
										//  	ImGui::Text("Value: %lld", selected_cvar.GetValue<int64_t>());
										//  	break;
										//  case StarCitizen::Enums::ECvarType::CVARTYPE_STRING:
										//  	ImGui::Text("Value: %s", selected_cvar.GetValue<char*>());
										//  	break;
									}
								}
								ImGui::EndChild();

								ImGui::SeparatorText(__("HISTORY"));

								/* display command history */
								ImGui::BeginChild(__("##cvar_EXECUTOR_FOOTER"), ImGui::GetContentRegionAvail(), false);
								{
									const ImVec2 szWndwAvail = ImGui::GetContentRegionAvail();
									const ImVec2& szChildHisCMD = ImVec2(szWndwAvail.x * .3f, szWndwAvail.y);
									ImGui::BeginChild(__("##CVAR_HISTORY"), szChildHisCMD, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_ResizeX);
									{
										int cmdID = 0;
										int argID = 0;
										for (auto& hVar : var_history)
										{
											const auto& dw_selFlags = ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SelectOnClick;
											if (ImGui::Selectable(hVar.var.Name, false, dw_selFlags))
											{
												selected_cvar = hVar.var;
												bSelectedCvar = true;
											}

											///	load old values
											//	ImGui::PushID(cmdID);
											//	if (ImGui::CollapsingHeader(hCmd.cmd.Name, ImGuiTreeNodeFlags_None))
											//	{
											//		for (auto arg : cmd.vArgs)
											//		{
											//			ImGui::PushID(argID);
											//			if (ImGui::Selectable((char*)arg.v[1], false, dw_selFlags))
											//			{
											//				for (int i = 0; i < arg.vCount; i++)
											//				{
											//					Hooks::vars::cmd_count++;			//	increment arg count
											//					Hooks::vars::cmd_args[i] = arg.v[i];	//	store args
											//				}
											//				Hooks::vars::cmd_selection = cmd.cmd;	// store command
											//	
											//				Hooks::vars::cmd_bExec = true;	//	command is executed from game thread
											//			}
											//			ImGui::PopID();
											//	
											//			argID++;
											//		}
											//	}
											//	ImGui::PopID();
											//	
											//	cmdID++;
										}
									}
									ImGui::EndChild();

									ImGui::SameLine();

									ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
									ImGui::BeginChild(__("##VAR_VALUE_HISTORY"), ImGui::GetContentRegionAvail(), true);
									{
									}
									ImGui::EndChild();
									ImGui::PopStyleVar();
								}
								ImGui::EndChild();
							}();
							break;

							/* COMMANDS */
						case 3:
							[&]()
							{
								auto fnCMDexec = [&]() -> void
								{
									Hooks::vars::cmd_args[0] = (char*)Hooks::vars::cmd_selection.Name;
									if (cmd_params_input_buffer[0] != '\0')
									{
										char* str_start = cmd_params_input_buffer;
										char* str_end = nullptr;

										while ((str_end = strchr(str_start, ',')) != nullptr)   // Find next comma
										{
											*str_end = '\0';                                    // Null-terminate the token
											if (Hooks::vars::cmd_count < 6)
												Hooks::vars::cmd_args[Hooks::vars::cmd_count++] = str_start;                  // Store the argument

											str_start = str_end + 1;                            // Move to next part
										}

										// Store the last part after the final comma (or full string if no commas)
										if (*str_start != '\0' && Hooks::vars::cmd_count < 6)
											Hooks::vars::cmd_args[Hooks::vars::cmd_count++] = str_start;
									}

									/* find command in history array */
									const auto& it = std::find_if(command_history.begin(), command_history.end(), [&](const SCDumperCommandHistory& cmd)
										{ return cmd.cmd.Name == Hooks::vars::cmd_selection.Name; });
									
									/* update command history */
									if (it == command_history.end())
									{
										/* store new command to the history */
										SCDumperCommandHistory lastCommand;
										SCDumperCommandHistory::args lastArgs;
										lastCommand.cmd = Hooks::vars::cmd_selection;
									
										///	@TODO : store params
										//	for (int i = 0; i < Hooks::vars::cmd_count; i++)
										//	{
										//		lastArgs.v[i] = Hooks::vars::cmd_args[i];
										//	}
										//	lastCommand.vArgs.push_back(lastArgs);
										command_history.push_back(lastCommand);
									}

									///	update args history
									//	else
									//	{
									//		SCDumperCommandHistory::args lastArgs;
									//	
									//		for (int i = 0; i < Hooks::vars::cmd_count; i++)
									//		{
									//			lastArgs.v[i] = Hooks::vars::cmd_args[i];
									//		}
									//		// @ TODO: prevent storage of duplicates
									//		it->vArgs.push_back(lastArgs);
									//	}
									//	
									//	/* clear input buffer */ 
									//	memset(cmd_params_input_buffer, 0, sizeof(cmd_params_input_buffer));

									/* execute command */
									Hooks::vars::cmd_bExec = true;	//	command is executed from game thread
								};

								if (!bSelectedCmd)
									return;

								/* display command editor */
								auto szExecWndw = ImGui::GetContentRegionAvail();
								ImVec2 szExecHeaderWndw = { szExecWndw.x, szExecWndw.y * .25f };
								ImGui::BeginChild(__("##EXECUTOR_HEADER"), szExecHeaderWndw, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_ResizeY);
								{
									ImGui::SeparatorText(Hooks::vars::cmd_selection.Name);
									ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
									ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * .75f);
									if (ImGui::InputTextWithHint(__("##CMD_PARAM_INPUT"), __("args separated by ','"), cmd_params_input_buffer, MAX_PATH, ImGuiInputTextFlags_EnterReturnsTrue))
										fnCMDexec();

									ImGui::SameLine();
									ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
									if (ImGui::Button(__("EXECUTE"), ImVec2(ImGui::GetContentRegionAvail().x, 0)))
										fnCMDexec();

									ImGui::SeparatorText(__("DESCRIPTION"));

									if (Hooks::vars::cmd_selection.Description != nullptr)
										ImGui::TextWrapped(Hooks::vars::cmd_selection.Description);
									ImGui::PopStyleVar();
								}
								ImGui::EndChild();

								ImGui::SeparatorText(__("HISTORY"));

								/* display command history */
								ImGui::BeginChild(__("##EXECUTOR_FOOTER"), ImGui::GetContentRegionAvail(), false);
								{
									const ImVec2 szWndwAvail = ImGui::GetContentRegionAvail();
									const ImVec2& szChildHisCMD = ImVec2(szWndwAvail.x * .3f, szWndwAvail.y);
									ImGui::BeginChild(__("##COMMAND_HISTORY"), szChildHisCMD, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_ResizeX);
									{
										int cmdID = 0;
										int argID = 0;
										for (auto hCmd : command_history)
										{
											const auto& dw_selFlags = ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SelectOnClick;
											if (ImGui::Selectable(hCmd.cmd.Name, false, dw_selFlags))
											{
												Hooks::vars::cmd_selection = hCmd.cmd;
												bSelectedCmd = true;
											}

											///	load params
											//	ImGui::PushID(cmdID);
											//	if (ImGui::CollapsingHeader(hCmd.cmd.Name, ImGuiTreeNodeFlags_None))
											//	{
											//		for (auto arg : cmd.vArgs)
											//		{
											//			ImGui::PushID(argID);
											//			if (ImGui::Selectable((char*)arg.v[1], false, dw_selFlags))
											//			{
											//				for (int i = 0; i < arg.vCount; i++)
											//				{
											//					Hooks::vars::cmd_count++;			//	increment arg count
											//					Hooks::vars::cmd_args[i] = arg.v[i];	//	store args
											//				}
											//				Hooks::vars::cmd_selection = cmd.cmd;	// store command
											//	
											//				Hooks::vars::cmd_bExec = true;	//	command is executed from game thread
											//			}
											//			ImGui::PopID();
											//	
											//			argID++;
											//		}
											//	}
											//	ImGui::PopID();
											//	
											//	cmdID++;
										}
									}
									ImGui::EndChild();

									ImGui::SameLine();
									ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
									ImGui::BeginChild(__("##COMMAND_ARG_HISTORY"), ImGui::GetContentRegionAvail(), true);
									{
									}
									ImGui::EndChild();
									ImGui::PopStyleVar();
								};
								ImGui::EndChild();
							}();
							break;

#if _DEBUG
							/* STRUCTS */
						case 4:
							[&]()
							{
								if (!bSelectedStruct)
									return;

								static long long szFields = 0;									//	
								static constexpr size_t MAX_FIELDS = 256;						//	
								static Structs::SDataField* outFields[MAX_FIELDS];				//	
								static const std::unordered_map<int, std::string> typeMap =
								{
									{StarCitizen::Enums::FIELDTYPE_BOOL, "bool"},
									{StarCitizen::Enums::FIELDTYPE_UINT32, "__int32"},
									{StarCitizen::Enums::FIELDTYPE_FLOAT, "float"},
									{StarCitizen::Enums::FIELDTYPE_CONSTCHAR_PTR, "char*"}
								};


								/* get data fields */
								if (!bObtainedStructData)
								{
									/* clear data */
									szFields = 0;
									memset(outFields, 0, sizeof(outFields));

									/* get new data */
									szFields = Helpers::datacore::GetStructDataFields(selected_struct, outFields);	//	get struct data and size of struct , field count
									if (!szFields || szFields > MAX_FIELDS)
										return;

									bObtainedStructData = true;
									return;	//	early return as the data shall persist
								}

								ImVec2 szWndw = ImGui::GetContentRegionAvail();
								ImGui::BeginChild(__("##structs_editor_child_header"), { szWndw.x, szWndw.y * .5f }, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_ResizeY);
								{
									//	const auto& pWndw = ImGui::GetCurrentWindow();
									//	const ImDrawList* pDraw = ImGui::GetWindowDrawList();	// draw ref
									//	const ImRect& szWndw = pWndw->Rect();	//	window size

									ImGui::Text(selected_struct.c_str());
									ImGui::Separator();

									for (int i = 0; i < szFields; i++)
									{
										if (!outFields[i])
											continue;

										const auto& data = outFields[i];
										const auto& it = typeMap.find(data->fieldType);

										ImGui::PushID(i);
										if (ImGui::CollapsingHeader(data->fieldName, ImGuiTreeNodeFlags_None))
										{
											ImGui::BulletText("Offset: 0x%04X", data->fieldOffset);
											ImGui::BulletText("Size: 0x%04X", data->fieldSize);
											it == typeMap.end() ? ImGui::BulletText("Type: 0x%04X", data->fieldType) : ImGui::BulletText("Type: %s", it->second.c_str());
										}
										ImGui::PopID();
									}
								}
								ImGui::EndChild();								
								
								/* display structs history */
								ImGui::BeginChild(__("##structs_EXECUTE_FOOTER"), ImGui::GetContentRegionAvail(), false);
								{
									const ImVec2 szWndwAvail = ImGui::GetContentRegionAvail();
									const ImVec2& szChildHisCMD = ImVec2(szWndwAvail.x * .3f, szWndwAvail.y);
									ImGui::BeginChild(__("##structs_history"), szChildHisCMD, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_ResizeX);
									{
										for (auto& s_struct : struct_history)
										{
											const auto& dw_selFlags = ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SelectOnClick;
											if (ImGui::Selectable(s_struct.c_str(), false, dw_selFlags))
											{
												bObtainedStructData = false;
												selected_struct = s_struct;
												bSelectedStruct = true;
											}
										}
									}
									ImGui::EndChild();

									ImGui::SameLine();

									ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
									ImGui::BeginChild(__("##structs_edit_history"), ImGui::GetContentRegionAvail(), true);
									{

									}
									ImGui::EndChild();
									ImGui::PopStyleVar();
								}
								ImGui::EndChild();


							}();
							break;

							/* NAMES */
						case 5:
							[&]()
							{

							}();
							break;
#endif
						default: break;
						}

					}
					ImGui::EndChild();
				}

				void Config()
				{
#if _DEBUG
					const auto& dwFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll;
					static char input_buffer[MAX_PATH];
					if (ImGui::InputTextWithHint(__("##input_get_struct_instance"), __("example: MiningGlobalParams.MiningGlobalParams"), input_buffer, MAX_PATH, dwFlags))
					{
						Hooks::vars::datacore_TargetName = std::string(input_buffer);
						Hooks::vars::datacore_bFindInstance = true;
						memset(input_buffer, 0, MAX_PATH);
					}
#endif
				}
			

			}

			/* scMenu */
			
			static int selected_tab = 0;

			void Header()
			{

				/*
					* ~ |                                    |
					* ~ |              OfflineCitizen             |
					* ~ |____________________________________|

					 ____________________________________
					|____________|___________|___________|
					|  ________________________________  |
					| |                                | |
					| |  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~  | |
					| |  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~  | |
					| |  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~  | |
					| |  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~  | |
					| |  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~  | |
					| |  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~  | |
					| |  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~  | |
					| |  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~  | |
					| |________________________________| |
					|____________________________________|

				*/

				auto style = ImGui::GetStyle();
				auto draw = ImGui::GetWindowDrawList();
				auto pos = ImGui::GetWindowPos();
				auto size = ImGui::GetWindowSize();
				auto szMainWindow = ImGui::GetContentRegionAvail();
				auto width = szMainWindow.x;


				//	gui::scene::Particles(pos, size, IM_COL32_CYAN);

				ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.f);

				/// TABS
				ImGui::BeginChild(__("##TABS_CHILD"), { szMainWindow.x, szMainWindow.y * .05f }, false);
				{
					static const char* tabs[] = { "ENHANCEMENTS", "DUMPER", "CONFIG" };



					const ImVec2 child_pos = ImGui::GetCursorScreenPos();
					const ImVec2 child_szWndw = ImGui::GetContentRegionAvail();
					const ImVec2 child_size(ImGui::GetContentRegionAvail().x, child_szWndw.y);

					ImU32 glowColor = IM_COL32(255, 255, 255, 120); // Semi-transparent blue glow
					for (float offset = 4.0f; offset > 0.0f; offset -= 1.0f)
					{
						ImVec2 glowMin = child_pos + ImVec2(-4, -4);
						ImVec2 glowMax = child_pos + child_size + ImVec2(4, 4);
						draw->AddRect(
							glowMin + ImVec2(offset, offset),	//	min
							glowMax - ImVec2(offset, offset),	//	max
							glowColor,							// color
							2.f,								// rounding
							0,									// flags	
							1.f									// thickness						
						);
					}

					ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f); // Disable rounding
					for (int i = 0; i < 3; i++)
					{
						ImVec2 szWndw = ImGui::GetContentRegionAvail();
						ImVec2 buttonPos = ImGui::GetCursorScreenPos();
						//	float buttonWidth = szWndw.x * ((i == 0) ? 0.33f : (i == 1) ? 0.5f : 1.0f);
						float buttonWidth = szWndw.x / static_cast<float>(3 - i);
						ImVec2 buttonSize(buttonWidth, szWndw.y);

						if (ImGui::Button(tabs[i], buttonSize))
							selected_tab = i;

						if (selected_tab == i)
						{
							ImDrawList* drawList = ImGui::GetWindowDrawList();
							ImVec2 glowMin = buttonPos;
							ImVec2 glowMax = buttonPos + buttonSize;

							ImU32 glowColor = IM_COL32(0, 150, 255, 120); // Semi-transparent blue glow

							for (float offset = 4.0f; offset > 0.0f; offset -= 1.0f)
							{
								drawList->AddRect(glowMin + ImVec2(offset, offset),
									glowMax - ImVec2(offset, offset),
									glowColor,
									0.0f, 
									0,
									1.0f
								); 
							}
						}

						if (i < 2) ImGui::SameLine(0, 0);
					}
					ImGui::PopStyleVar();   //  ImGuiStyleVar_FrameRounding
				}
				ImGui::EndChild();
				ImGui::PopStyleVar();

			}

			void Body()
			{
				ImGui::BeginChild(__("##BODY_CHILD"), ImGui::GetContentRegionAvail(), false);
				{
					switch (selected_tab)
					{
					case 0: scTabs::Enhancements(); break;
					case 1: scTabs::Dumper(); break;
#if _DEBUG
					case 2: scTabs::Config(); break;
#endif
					default: break;
					}
				}
				ImGui::EndChild();
			}

			void Footer()
			{

			}
		}
		
		namespace scCanvas
		{
			namespace scWidgets 
			{
				void MiniMap()
				{
					///	@TODO:	Implement MiniMap
					//	const char* fmt_bones_face[] = { xorstr_("^"), xorstr_("^"), xorstr_("_") };
					//	static EBones bones_face[] = { EBones::l_eye, EBones::r_eye, EBones::head };
					//	
					//	CSystem* pSystem = *CSystem::g_System;
					//	CCamera* pCamera = CCamera::g_pCamera;
					//	if (!pSystem)
					//		return;
					//	
					//	CEntity* pEntity = g_localPlayerTM.pEntity;
					//	if (!pEntity || !pEntity->IsValid() /*|| pEntity->GetShipEntity()*/)
					//		return;
					//	
					//	CZone* pZone = pEntity->GetLocalZone();
					//	if (!pZone)
					//		return;
					//	
					//	ImVec2 _size = { g_WndwVars->s_CurrentWindow.clientSize[0], g_WndwVars->s_CurrentWindow.clientSize[1] };
					//	ImVec2 _pos = { g_WndwVars->s_CurrentWindow.clientPos[0], g_WndwVars->s_CurrentWindow.clientPos[1] };
					//	ImVec2 _szWndw{ _size.x * .5f, _size.y * .5f };
					//	ImVec2 _posWndw{ ((_size.x / 2) - _szWndw.x * .5f) + _pos.x, ((_size.y / 2) - _szWndw.y * .5f) + _pos.y };	//	center
					//	
					//	static bool map_bEdit_Angles{ false };
					//	static bool map_bMatch_Angles{ false };
					//	static float map_head_radius = 5.f;
					//	static float map_rotation_angle_x = 0.0f;
					//	static float map_rotation_angle_y = 0.0f;
					//	static float map_rotation_angle_z = 0.0f;
					//	static float map_scale_factor = 25.f; // Adjust this to scale the bone structure preview
					//	static float map_height_factor = 0.5f; // Adjust this to scale the bone structure preview
					//	static float map_ZoomLevel{ 100.f };		//	
					//	static float map_ZoomScale{ 2.f };		//	
					//	static float map_LineSpacing{ 20.f };	//	zoom => 100 ; scale => 2 ; spacing => 20 ; == 10m block
					//	
					//	ImGui::SetNextWindowPos(_posWndw, ImGuiCond_Always);
					//	ImGui::SetNextWindowSize(_szWndw, ImGuiCond_Always);
					//	if (ImGui::Begin("MINI MAP", (bool*)false, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus))
					//	{
					//	
					//		auto& style = ImGui::GetStyle();
					//		ImDrawList* draw_list = ImGui::GetWindowDrawList();
					//		ImVec2 window_pos = ImGui::GetWindowPos();				// Get the top-left position of the window
					//		ImVec2 window_size = ImGui::GetWindowSize();			// Define the preview area size
					//		ImVec2 center = ImVec2(window_pos.x + window_size.x * .5f, window_pos.y + window_size.y * map_height_factor);// Center the object in the preview window
					//		ImVec2 radar_pos = center;
					//		float radar_size = (window_size.x + window_size.y) * .5;
					//		auto radar_zoom = map_ZoomLevel * map_ZoomScale;
					//	
					//		/// Draw Radar Grid Background
					//		{
					//			//  Draw Grid Lines
					//			float line_spacing = radar_size / map_LineSpacing;                                          // Determine the spacing between the grid lines based on the radius
					//			int num_lines = static_cast<int>(radar_size / line_spacing) * 2;                            // Determine the number of grid lines based on the spacing and radius
					//			for (int i = 0; i <= num_lines; i++)
					//			{
					//				float y = radar_pos.y - radar_size + i * line_spacing;
					//				float x = radar_pos.x - radar_size + i * line_spacing;
					//				gui::draw::CleanLine({ radar_pos.x - radar_size, y }, { radar_pos.x + radar_size, y }, style.Colors[ImGuiCol_PopupBg], 1.0f);
					//				gui::draw::CleanLine({ x, radar_pos.y - radar_size }, { x, radar_pos.y + radar_size }, style.Colors[ImGuiCol_PopupBg], 1.0f);
					//			}
					//		}
					//	
					//	
					//		///	DRAW ENTITIY ARRAY
					//		//	int index{ -1 };
					//	//	auto ent_array = g_imgui_trans_cache;
					//	//	auto local_player_pos = pEntity->GetLocalPos();
					//	//	for (auto ent : ent_array)
					//	//	{
					//	//		index++;
					//	//		CEntity* player = ent.pEntity;
					//	//		if (!player->IsValid() || player->GetLocalZone() != pZone)
					//	//			continue;
					//	//	
					//	//		if (player == pEntity)
					//	//			continue;
					//	//	
					//	//		//	determine if entity has the bones
					//	//		if (ent.bones.size() < 22)
					//	//			continue;
					//	//	
					//	//		//	Get 2D position
					//	//		auto ent_pos = player->GetLocalPos();
					//	//		auto pos = local_player_pos - ent_pos;
					//	//		Vector2 pos2D = { (float)pos.Y, (float)pos.X };
					//	//		auto dist2D = std::sqrt(pos2D.x * pos2D.x + pos2D.y * pos2D.y);
					//	//		if (dist2D > radar_zoom)
					//	//			continue;
					//	//	
					//	//		float eular_angle = player->GetNormalizedAngleRadians();			//	radians
					//	//		//	float player_angle_radians = pEntity->GetNormalizedAngleRadians();	//	radians
					//	//		auto rotatedPos2D = Vector2(pos2D.x, pos2D.y);
					//	//		rotatedPos2D *= radar_size / radar_zoom;
					//	//		Vector2 rotScreen2D = Vector2(radar_pos.x, radar_pos.y) - rotatedPos2D;
					//	//		ImVec2 rotDraw_pos = ImVec2(rotScreen2D.x, rotScreen2D.y);
					//	//	
					//	//	
					//	//		auto bone_head = ent.bones.at(EBones::head);
					//	//		if (map_bEdit_Angles)
					//	//		{
					//	//			if (map_bMatch_Angles)
					//	//				map_rotation_angle_z = eular_angle;
					//	//			bone_head.RotatePointAroundX(map_rotation_angle_x);	//	yaw
					//	//			bone_head.RotatePointAroundY(map_rotation_angle_y);	//	yaw
					//	//			bone_head.RotatePointAroundZ(map_rotation_angle_z);	//	yaw
					//	//		}
					//	//		else
					//	//			bone_head.RotatePointAroundZ(eular_angle);	//	yaw
					//	//		ImVec2 head_bone_pos2D = ImVec2(rotDraw_pos.x + bone_head.x * map_scale_factor, rotDraw_pos.y - bone_head.y * map_scale_factor);
					//	//		//	draw_list->AddCircleFilled(rotDraw_pos, 5.f, IM_COL32_RED, 100);
					//	//		GUI::Draw::TriangleFilled(head_bone_pos2D, { radar_size * .05f, 1.1f * 10.f }, ImColor(1.0f, 1.0f, 1.0f, 0.25f), GUI::BOTTOM, eular_angle, -1.f);	//	Player FOV
					//	//		//	GUI::Draw::TriangleFilled(head_bone_pos2D, { radar_size * .25f, 1.1f * 45.f }, ImColor(1.0f, 1.0f, 1.0f, 0.05f), GUI::BOTTOM, eular_angle, -1.f);	//	Player FOV
					//	//	
					//	//		for (auto& boneID : BoneVector)
					//	//		{
					//	//			StarCitizen::Vector3 xPoint, yPoint;
					//	//			for (int i = 0; i < boneID.size(); i++)
					//	//			{
					//	//				//	Get Bone
					//	//				auto index = boneID.at(i);
					//	//				xPoint = ent.bones.at(index);
					//	//				if (map_bEdit_Angles)
					//	//				{
					//	//					if (map_bMatch_Angles)
					//	//						map_rotation_angle_z = eular_angle;
					//	//					xPoint = RotatePointAroundX(xPoint, map_rotation_angle_x);					//	yaw
					//	//					xPoint = RotatePointAroundY(xPoint, map_rotation_angle_y);					//	yaw
					//	//					xPoint = RotatePointAroundZ(xPoint, map_rotation_angle_z);					//	yaw
					//	//				}
					//	//				else
					//	//					xPoint = RotatePointAroundZ(xPoint, eular_angle);							//	yaw
					//	//	
					//	//				if (yPoint.IsValid())
					//	//				{
					//	//					yPoint = xPoint;
					//	//					continue;
					//	//				}
					//	//				ImVec2 bone_pos_2D_a = ImVec2(rotDraw_pos.x + xPoint.x * map_scale_factor, rotDraw_pos.y - xPoint.y * map_scale_factor); // Invert Y for top-down view
					//	//				ImVec2 bone_pos_2D_b = ImVec2(rotDraw_pos.x + yPoint.x * map_scale_factor, rotDraw_pos.y - yPoint.y * map_scale_factor); // Invert Y for top-down view
					//	//				draw_list->AddCircleFilled(bone_pos_2D_a, 1.0f, IM_COL32_WHITE);
					//	//				draw_list->AddCircleFilled(bone_pos_2D_b, 1.0f, IM_COL32_WHITE);
					//	//				draw_list->AddLine(bone_pos_2D_a, bone_pos_2D_b, IM_COL32_WHITE, 3.f);
					//	//				yPoint = xPoint;
					//	//			}
					//	//		}
					//	//		draw_list->AddCircleFilled(head_bone_pos2D, map_head_radius, IM_COL32_WHITE, 100.f);
					//	//	}
					//	
					//		///	DRAW LOCAL PLAYER
					//		//	auto ent_angle = NormalizeAngleRadians(pEntity->GetLocalAngles().z);
					//	//	float eular_angle = pEntity->GetNormalizedLocalRotation();
					//	//	for (auto& boneID : BoneVector)
					//	//	{
					//	//		StarCitizen::Vector3 xPoint, yPoint;
					//	//		for (int i = 0; i < boneID.size(); i++)
					//	//		{
					//	//			///	GET BONE
					//	//			auto index = boneID.at(i);
					//	//			xPoint = g_localPlayerTM.bones.at(index);
					//	//	
					//	//			///	APPLY BONE TM
					//	//			//	xPoint = RotatePointAroundZ(xPoint, eular_angle);	//	roll
					//	//			if (yPoint.IsValid())
					//	//			{
					//	//				yPoint = xPoint;
					//	//				continue;
					//	//			}
					//	//			///	GET SCREEN COORDS
					//	//			ImVec2 bone_pos_2D_a = ImVec2(center.x + xPoint.x * map_scale_factor, center.y - xPoint.y * map_scale_factor); // Invert Y for top-down view
					//	//			ImVec2 bone_pos_2D_b = ImVec2(center.x + yPoint.x * map_scale_factor, center.y - yPoint.y * map_scale_factor); // Invert Y for top-down view
					//	//	
					//	//			///	DRAW
					//	//			draw_list->AddCircleFilled(bone_pos_2D_a, 1.0f, IM_COL32_WHITE);
					//	//			draw_list->AddCircleFilled(bone_pos_2D_b, 1.0f, IM_COL32_WHITE);
					//	//			draw_list->AddLine(bone_pos_2D_a, bone_pos_2D_b, IM_COL32_WHITE, 3.f);
					//	//	
					//	//			///	SET PREV BONE
					//	//			yPoint = xPoint;
					//	//		}
					//	//	}
					//	//	
					//	//	auto bone_head = g_localPlayerTM.bones.at(EBones::head);
					//	//	//	bone_head = RotatePointAroundZ(bone_head, eular_angle);	//	roll
					//	//	ImVec2 bone_pos_2D_a = ImVec2(center.x + bone_head.x * map_scale_factor, center.y - bone_head.y * map_scale_factor);
					//	//	draw_list->AddCircleFilled(bone_pos_2D_a, map_scale_factor / 5, IM_COL32_WHITE, 100.f);
					//		
					//		///	FACE
					//		//	GUI::Draw::TriangleFilled(center, { radar_size * .05f, 1.1f * 10.f }, ImColor(1.0f, 1.0f, 1.0f, 0.25f), GUI::BOTTOM, pEntity->GetNormalizedAngleRadians(), -1.f);	//	Player FOV
					//		//	draw_list->AddCircleFilled(center, 5.f, IM_COL32_ORANGE, 100.f);
					//	
					//		ImGui::End();	//	end minimap
					//	}
					//	
					//	
					//	ImVec2 _szWndw_widget{ _szWndw.x * .3f, _szWndw.y * .5f };
					//	ImVec2 _posWndw_target{ (_posWndw.x + (_szWndw.x + 5.f)) + _pos.x, _posWndw.y + _pos.y };							//	TARGET PLAYER WIDGET ; TOP
					//	ImVec2 _posWndw_player{ (_posWndw.x + (_szWndw.x + 5.f)) + _pos.x, (_posWndw.y + _szWndw_widget.y) + _pos.y };	//	Local Player Widget ; TOP
					//	static float _wgt_scale_factor = 150.f;
					//	
					//	ImGui::SetNextWindowPos(_posWndw_target, ImGuiCond_Always);
					//	ImGui::SetNextWindowSize(ImVec2(_szWndw_widget.x, _szWndw_widget.y * .9f), ImGuiCond_Always);
					//	if (/*g_WndwVars->m_bShowMenu && */ImGui::Begin("MiniMap Debug Tools Widget", (bool*)false, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNav))
					//	{
					//		//	//	LOCAL PLAYER
					//		//	STransforms TM = g_localPlayerTM;
					//		//	auto world_pos = TM.origin;
					//		//	auto world_rot = TM.rotation;
					//		//	auto world_angles = TM.angles;
					//		//	auto world_angle_degrees = world_angles.z;
					//		//	auto local_Pos = pEntity->GetLocalPos();
					//		//	auto local_Rot = pEntity->GetLocalRotation();
					//		//	auto local_angles = pEntity->GetLocalEularAngles();
					//		//	float local_angle_radians = pEntity->GetNormalizedAngleRadians();
					//		//	float local_angle_degrees = local_angle_radians * 180.f / M_PI;
					//		//	//	auto zone_rot = pZone->GetRotation();
					//		//	
					//		//	//	CAMERA
					//		//	auto pCamera = CCamera::g_pCamera;
					//		//	auto camera_pos = pCamera->GetWorldPos();
					//		//	auto camera_rotation = pCamera->GetViewAngles();
					//		//	auto camera_angles = camera_rotation.GetEularAngles();
					//		//	
					//		//	//	ZONE
					//		//	auto pZone = pEntity->GetLocalZone();
					//		//	auto zone_pos = pZone->GetPosition();
					//		//	
					//		//	//	OTHERS
					//		//	auto tracked_players = g_imgui_trans_cache;
					//		//	int zone_players{ 0 };
					//		//	
					//		//	for (auto ent : tracked_players)
					//		//	{
					//		//		CEntity* player = ent.pEntity;
					//		//		if (!player->IsValid() || player->GetLocalZone() != pZone)
					//		//			continue;
					//		//	
					//		//		zone_players++;
					//		//	}
					//		//	
					//		//	if (ImGui::BeginTabBar("##MINIMAPDEBUG", ImGuiTabBarFlags_None))
					//		//	{
					//		//		if (ImGui::BeginTabItem("INFO"))
					//		//		{
					//		//			ImGui::BeginChild("##MINIMAPDEBUG_INFO", ImGui::GetContentRegionAvail());
					//		//	
					//		//			ImGui::SeparatorText(xorstr_("LOCAL TM"));
					//		//			ImGui::Text(xorstr_("Name: %s"), pEntity->GetName().c_str());
					//		//			ImGui::Text(xorstr_("Position: { %.2f , %.2f , %.2f }"), local_Pos.X, local_Pos.Y, local_Pos.Z);
					//		//			ImGui::Text(xorstr_("Quat Rotation: { %.2f , %.2f , %.2f , %.2f }"), local_Rot.X, local_Rot.Y, local_Rot.Z, local_Rot.W);
					//		//			ImGui::Text(xorstr_("Eular Angles: { %.2f , %.2f , %.2f }"), local_angles.x, local_angles.y, local_angles.z);
					//		//			ImGui::Text(xorstr_("Angle Radians: { %.2f }"), local_angle_radians);
					//		//			ImGui::Text(xorstr_("Angle Degrees: { %.2f }"), local_angle_degrees);
					//		//	
					//		//			ImGui::SeparatorText(xorstr_("WORLD TM"));
					//		//			ImGui::TextWrapped(xorstr_("Position: { %.2f , %.2f , %.2f }"), world_pos.X, world_pos.Y, world_pos.Z);
					//		//			ImGui::Text(xorstr_("Rotation: { %.2f , %.2f , %.2f }"), world_rot.X, world_rot.Y, world_rot.Z);
					//		//			ImGui::Text(xorstr_("Eular Angles: { %.2f , %.2f , %.2f }"), world_angles.x, world_angles.y, world_angles.z);
					//		//	
					//		//			ImGui::SeparatorText(xorstr_("CAMERA TM"));
					//		//			ImGui::TextWrapped(xorstr_("Position: { %.2f , %.2f , %.2f }"), camera_pos.X, camera_pos.Y, camera_pos.Z);
					//		//			ImGui::Text(xorstr_("View Angles: { %.2f , %.2f , %.2f , %.2f }"), camera_rotation.x, camera_rotation.y, camera_rotation.z, camera_rotation.w);
					//		//			ImGui::Text(xorstr_("Eular Angles: { %.2f , %.2f , %.2f }"), camera_angles.x, camera_angles.y, camera_angles.z);
					//		//	
					//		//			ImGui::SeparatorText(xorstr_("ZONE TM"));
					//		//			ImGui::Text(xorstr_("Zone: %s"), pZone->GetName().c_str());
					//		//			ImGui::Text(xorstr_("Rendererd Players: %d / %d"), zone_players, tracked_players.size());
					//		//			ImGui::TextWrapped(xorstr_("Zone Position: { %.2f , %.2f , %.2f }"), zone_pos.X, zone_pos.Y, zone_pos.Z);
					//		//	
					//		//	
					//		//			ImGui::EndChild();
					//		//			ImGui::EndTabItem();
					//		//		}
					//		//	
					//		//		if (ImGui::BeginTabItem("CONTROLS"))
					//		//		{
					//		//			ImGui::BeginChild("##MINIMAPDEBUG_CONTROLS", ImGui::GetContentRegionAvail());
					//		//	
					//		//			//	MINI MAP DEBUG CONTROLS
					//		//			if (ImGui::CollapsingHeader(xorstr_("DEBUG MAP OPTIONS")))
					//		//			{
					//		//				ImGui::SliderFloat("Zoom Level", &map_ZoomLevel, 0.0f, 100.f, "%.0f");
					//		//				ImGui::SliderFloat("Zoom Scale", &map_ZoomScale, 0.0f, 100.f, "%.0f");
					//		//				ImGui::SliderFloat("Line Spacing", &map_LineSpacing, 0.0f, 100.f, "%.0f");
					//		//	
					//		//				ImGui::Checkbox("Edit Angles", &map_bEdit_Angles);
					//		//				if (map_bEdit_Angles)
					//		//				{
					//		//	
					//		//					ImGui::Checkbox("match entity roll", &map_bMatch_Angles);
					//		//					ImGui::SliderFloat("Ent-Rotation X", &map_rotation_angle_x, 0.f, M_PI * 2.f, "%.2f");
					//		//					ImGui::SliderFloat("Ent-Rotation Y", &map_rotation_angle_y, 0.f, M_PI * 2.f, "%.2f");
					//		//					ImGui::SliderFloat("Ent-Rotation Z", &map_rotation_angle_z, 0.f, M_PI * 2.f, "%.2f");
					//		//				}
					//		//			}
					//		//	
					//		//			//---------------------------------------------------------------------------------------------------
					//		//			if (ImGui::CollapsingHeader(xorstr_("DEBUG RADAR OPTIONS")))
					//		//			{
					//		//				GUI::ToggleWithToolTip(xorstr_("HELMET MODE"), xorstr_("Having a helmet on will change the radars position in the game."), &g_radar_bHelmet);
					//		//				ImGui::SliderFloat(xorstr_("RADAR WIDTH"), &g_debug_radar_width, 0.0, 500.f, xorstr_("%.2f"));
					//		//				ImGui::SliderFloat(xorstr_("RADAR HEIGHT"), &g_debug_radar_height, 0.0, 500.f, xorstr_("%.2f"));
					//		//				ImGui::SliderFloat(xorstr_("RADAR X"), &g_debug_radar_width_scale, 0.0, 1.f, xorstr_("%.2f"));
					//		//				ImGui::SliderFloat(xorstr_("RADAR Y"), &g_debug_radar_height_scale, 0.f, 1.f, xorstr_("%.2f"));
					//		//				ImGui::SliderFloat(xorstr_("ZOOM LEVEL"), &g_radar_ZoomLevel, 0.f, 500.f, xorstr_("%.2f"));
					//		//				ImGui::SliderFloat(xorstr_("ZOOM SCALE"), &g_radar_ZoomScale, 0.f, 100.f, xorstr_("%.2f"));
					//		//			}
					//		//	
					//		//			ImGui::EndChild();
					//		//			ImGui::EndTabItem();
					//		//		}
					//		//	
					//		//		ImGui::EndTabBar();
					//		//	}
					//	
					//	
					//		ImGui::End();
					//	}
					//	
					//	///	
					//	ImGui::SetNextWindowPos(_posWndw_player, ImGuiCond_Always);
					//	ImGui::SetNextWindowSize(_szWndw_widget, ImGuiCond_Always);
					//	if (ImGui::Begin("Local Player Widget", (bool*)false, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDocking))
					//	{
					//	
					//		///	DEBUG PLAYER PREVIEW WIDGET ; MINIMAP
					//		// - Skeleton
					//		// - Health
					//		// - Name
					//		//	TODO: Dispaly ESP Feature Types
					//		ImGui::BeginChild("PLAYER_CHILD_PREVIEW", ImGui::GetContentRegionAvail());
					//		{
					//			static constexpr auto pitch_angle = -M_PI * 0.5f;
					//			auto draw_list = ImGui::GetWindowDrawList();
					//			auto window_pos = ImGui::GetWindowPos();
					//			auto window_size = ImGui::GetWindowSize();
					//			auto center = ImVec2(window_pos.x + window_size.x * .5f, window_pos.y + window_size.y * 0.9f);
					//	
					//	
					//			if (g_localPlayerTM.bones.size() >= 22)
					//			{
					//				///	DRAW SKELETON
					//				for (auto& boneID : BoneVector)
					//				{
					//					StarCitizen::Vector3 xPoint, yPoint;
					//					for (int i = 0; i < boneID.size(); i++)
					//					{
					//						xPoint = RotatePointAroundX(g_localPlayerTM.bones.at(boneID.at(i)), pitch_angle);
					//						if (yPoint.IsValid())
					//						{
					//							yPoint = xPoint;
					//							continue;
					//						}
					//						ImVec2 bone_screen_a = ImVec2(center.x + xPoint.x * _wgt_scale_factor, center.y - xPoint.y * _wgt_scale_factor);
					//						ImVec2 bone_screen_b = ImVec2(center.x + yPoint.x * _wgt_scale_factor, center.y - yPoint.y * _wgt_scale_factor);
					//						draw_list->AddCircleFilled(bone_screen_a, 1.0f, IM_COL32_WHITE);
					//						draw_list->AddCircleFilled(bone_screen_b, 1.0f, IM_COL32_WHITE);
					//						draw_list->AddLine(bone_screen_a, bone_screen_b, IM_COL32_WHITE, 3.f);
					//						yPoint = xPoint;
					//					}
					//				}
					//	
					//				///	DRAW HEAD
					//				StarCitizen::Vector3 bone_head = RotatePointAroundX(g_localPlayerTM.bones.at(EBones::head), pitch_angle);
					//				ImVec2 bone_head_pos = ImVec2(center.x + bone_head.x * _wgt_scale_factor, center.y - bone_head.y * _wgt_scale_factor);
					//				draw_list->AddCircleFilled(bone_head_pos, _wgt_scale_factor * .1f, IM_COL32_WHITE, 100.f);
					//	
					//				///	^_^
					//				for (int i = 0; i < 3; i++)
					//				{
					//					StarCitizen::Vector3 bones_eye = RotatePointAroundX(g_localPlayerTM.bones.at(bones_face[i]), pitch_angle);
					//					ImVec2 bone_eye_pos = ImVec2(center.x + bones_eye.x * _wgt_scale_factor, center.y - bones_eye.y * _wgt_scale_factor);
					//					draw_list->AddText(bone_eye_pos, IM_COL32_BLACK, fmt_bones_face[i]);
					//				}
					//			}
					//		}
					//		ImGui::EndChild();
					//	
					//		ImGui::End();
					//	}
					//	
					//	ImGui::End();
				}

				void Skeleton()
				{
					//	const auto& pWindow = ImGui::GetCurrentWindow();
					//	if (!pWindow)
					//		return;
					//	const auto& szMenu = pWindow->Rect();
					//	//	const auto& szMenu = *reinterpret_cast<ImRect*>(rect);
					//	auto center = szMenu.GetCenter();
					//	
					//	const float& flWndwSz = szMenu.GetSize().x * 0.125f;
					//	ImVec2 _szWndw{ flWndwSz , flWndwSz };
					//	ImVec2 _posWndw{ 0.0f, 0.0f }; // { _szWndw.x * .08f + szMenu.Min.x, _szWndw.x * .0f + szMenu.Max.y };	//	top left	;	no helmet
					//	ImGui::SetNextWindowPos(_posWndw, ImGuiCond_Always);
					//	ImGui::SetNextWindowSize(_szWndw, ImGuiCond_Always);
					//	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 100.f);
					//	ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32_BLACK_TRANS);
					//	ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32_BLACK_TRANS);
					//	if (!ImGui::Begin("##PLAYER STATUS", (bool*)false, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDecoration))
					//	{
					//		ImGui::PopStyleColor(2);
					//		ImGui::PopStyleVar();
					//		ImGui::End();
					//		return;
					//	}
					//	ImGui::PopStyleColor(2);
					//	ImGui::PopStyleVar();
					//	
					//	ImDrawList* pDraw = ImGui::GetWindowDrawList();
					//	
					//	//	Draw Local Player 
					//	ImVec2 window_pos = ImGui::GetWindowPos();
					//	ImVec2 window_size = ImGui::GetWindowSize();
					//	center = ImVec2(window_pos.x + window_size.x * .5f, window_pos.y + window_size.y * 0.5f);
					//	ImVec2 radar_pos = center;
					//	float radar_size = (window_size.x + window_size.y) * .5;
					//	auto radar_zoom = 30.f * 1.f;
					//	static constexpr float rotation_angle_x{ 0.f };
					//	static constexpr float scale_factor = 25.f;
					//	const auto sLocalTM = StarCitizen::Hooks::vars::sLocalPlayerTM;
					//	
					//	if (sLocalTM.bones.size() > 0)
					//	{
					//		for (auto& boneID : StarCitizen::Hooks::vars::BoneVector)
					//		{
					//			StarCitizen::Structs::FVector xPoint, yPoint;
					//			for (int i = 0; i < boneID.size(); i++)
					//			{
					//				//	Get Bone
					//				auto index = boneID.at(i);
					//				xPoint = sLocalTM.bones.at(index);
					//				xPoint.RotatePointAroundX(-M_PI / 2);	//	pitch
					//				xPoint.RotatePointAroundY(0.0f);	    //	yaw
					//	
					//				if (yPoint.IsValid())
					//				{
					//					yPoint = xPoint;
					//					continue;
					//				}
					//				ImVec2 bone_pos_2D_a = ImVec2(center.x + xPoint.x * scale_factor, center.y - xPoint.y * scale_factor); // Invert Y for top-down view
					//				ImVec2 bone_pos_2D_b = ImVec2(center.x + yPoint.x * scale_factor, center.y - yPoint.y * scale_factor); // Invert Y for top-down view
					//	
					//				pDraw->AddCircleFilled(bone_pos_2D_a, 1.0f, IM_COL32_WHITE);
					//				pDraw->AddCircleFilled(bone_pos_2D_b, 1.0f, IM_COL32_WHITE);
					//				pDraw->AddLine(bone_pos_2D_a, bone_pos_2D_b, IM_COL32_WHITE, 3.f);
					//	
					//				yPoint = xPoint;
					//			}
					//		}
					//	
					//		auto bone_head = sLocalTM.bones.at(StarCitizen::Enums::EBones::head);
					//		bone_head.RotatePointAroundX(-M_PI / 2);	//	pitch
					//		bone_head.RotatePointAroundY(0.0f);	//	yaw
					//		ImVec2 bone_pos_2D_a = ImVec2(center.x + bone_head.x * scale_factor, center.y - bone_head.y * scale_factor);
					//		pDraw->AddCircleFilled(bone_pos_2D_a, scale_factor / 5.f, IM_COL32_WHITE, 100.f);
					//	}
					//	
					//	ImGui::End();


				}

				void StructData(const std::string& name)
				{



				}
			}

			void draw()
			{
				auto drawIT = [&](Structs::SDrawEntity entTM, const ImColor& target_color, const ImColor& fwd_color, const float& health, const int& fwdIndex = 0)
				{
					const auto& pDraw = ImGui::GetWindowDrawList();
					if (!pDraw)
						return;


					const ImVec2& posWndw = g_gui.GetCloneRect().Min;

					//	if (entTM.bIsActor && health <= 0.f)
					//		return;

					ImVec2 fwd = ImVec2(entTM.originFWD.x, entTM.originFWD.y) + posWndw;
					ImVec2 root = ImVec2(entTM.origin.x, entTM.origin.y) + posWndw;

					//	3D Bounding Box
					ImVec2 boxVerts[8];
					for (int i = 0; i < 8; i++)
					{
						boxVerts[i] = ImVec2(entTM.boxVerts[i].x, entTM.boxVerts[i].y) + posWndw;
//	#if _DEBUG
//							gui::draw::Text(boxVerts[i], ImColor(255, 255, 255, 50), std::to_string(i).c_str(), true);
//	#endif
					}
					for (int i = 0; i < 4; i++)
					{
						if (entTM.bBoxVerts[i] && entTM.bBoxVerts[(i + 1) % 4])
							gui::draw::CleanLine(boxVerts[i], boxVerts[(i + 1) % 4], /*fwdIndex == 0 ? fwd_color :*/ target_color, 1.f);
					
						if (entTM.bBoxVerts[i + 4] && entTM.bBoxVerts[((i + 1) % 4) + 4])
							gui::draw::CleanLine(boxVerts[i + 4], boxVerts[((i + 1) % 4) + 4], /*fwdIndex == 1 ? fwd_color :*/ target_color, 1.0f);
					
						if (entTM.bBoxVerts[i] && entTM.bBoxVerts[i + 4])
							gui::draw::CleanLine(boxVerts[i], boxVerts[i + 4], /*fwdIndex == 2 ? fwd_color :*/ target_color, 1.0f);
					}


					ImVec2 bottomFaceVerts[4]	= { boxVerts[0], boxVerts[1] , boxVerts[5] , boxVerts[4] };
					ImVec2 topFaceVerts[4]		= { boxVerts[3], boxVerts[2] , boxVerts[6] , boxVerts[7] };
					ImVec2 rightFaceVerts[4]	= { boxVerts[0], boxVerts[3] , boxVerts[7] , boxVerts[4] };
					ImVec2 leftFaceVerts[4]		= { boxVerts[1], boxVerts[5] , boxVerts[6] , boxVerts[2] };
					ImVec2 backFaceVerts[4]		= { boxVerts[0], boxVerts[1] , boxVerts[2] , boxVerts[3] }; // ~
					ImVec2 frontFaceVerts[4]	= { boxVerts[5], boxVerts[4] , boxVerts[7] , boxVerts[6] }; // ~
					//	for ( int i = 0; i < 4; i++)
					//		gui::draw::CleanLine(frontFaceVerts[i], boxVerts[(i + 1) % 4], fwdIndex == 1 ? fwd_color : target_color, 1.0f);
					//	ImVec2* faces[6] = {
					//		bottomFaceVerts, // 0
					//		topFaceVerts,    // 1
					//		frontFaceVerts,  // 2
					//		backFaceVerts,   // 3
					//		leftFaceVerts,   // 4
					//		rightFaceVerts   // 5
					//	};
					//	bool bValidBox[6] =
					//	{
					//		{ entTM.bBoxVerts[0] && entTM.bBoxVerts[1] && entTM.bBoxVerts[5] && entTM.bBoxVerts[4] },	
					//		{ entTM.bBoxVerts[3] && entTM.bBoxVerts[2] && entTM.bBoxVerts[6] && entTM.bBoxVerts[7] },	
					//		{ entTM.bBoxVerts[5] && entTM.bBoxVerts[4] && entTM.bBoxVerts[7] && entTM.bBoxVerts[6] },
					//		{ entTM.bBoxVerts[0] && entTM.bBoxVerts[1] && entTM.bBoxVerts[2] && entTM.bBoxVerts[3] },	
					//		{ entTM.bBoxVerts[1] && entTM.bBoxVerts[5] && entTM.bBoxVerts[6] && entTM.bBoxVerts[2] },	
					//		{ entTM.bBoxVerts[0] && entTM.bBoxVerts[3] && entTM.bBoxVerts[7] && entTM.bBoxVerts[4] }	
					//	};

					ImColor shadedTargetColor = target_color;
					ImColor shadedTargetFrontColor = fwd_color;

					if (health > 0.f)
						shadedTargetColor = ImColor(255 - health * 2.55, health * 2.55, 0);		//	health color
					
					shadedTargetColor.Value.w = target_color.Value.w * .1f;
					shadedTargetFrontColor.Value.w = target_color.Value.w * .1f;
					
					//	for (int f = 0; f < 6; f++)
					//	{
					//		if (!bValidBox)
					//			continue;
					//	
					//		ImU32 col = (f == fwdIndex) ? fwd_color : target_color;
					//		ImU32 shadedCol = (f == fwdIndex) ? shadedTargetFrontColor : shadedTargetColor;
					//	
					//		// Filled shading
					//		if (bValidBox[f])
					//			pDraw->AddConvexPolyFilled(faces[f], 4, shadedCol);
					//	
					//		// Outline for clarity
					//		for (int i = 0; i < 4; i++)
					//		{
					//			gui::draw::CleanLine(
					//				faces[f][i],
					//				faces[f][(i + 1) % 4],
					//				col,
					//				1.0f
					//			);
					//		}
					//	}

					if (entTM.bBoxVerts[0] &&
						entTM.bBoxVerts[1] &&
						entTM.bBoxVerts[5] &&
						entTM.bBoxVerts[4]
						)
						pDraw->AddConvexPolyFilled(bottomFaceVerts, 4, shadedTargetColor);
					
					if (entTM.bBoxVerts[3] &&
						entTM.bBoxVerts[2] &&
						entTM.bBoxVerts[6] &&
						entTM.bBoxVerts[7]
						)
						pDraw->AddConvexPolyFilled(topFaceVerts, 4, shadedTargetColor);
					
					if (entTM.bBoxVerts[0] &&
						entTM.bBoxVerts[3] &&
						entTM.bBoxVerts[7] &&
						entTM.bBoxVerts[4]
						)
						pDraw->AddConvexPolyFilled(rightFaceVerts, 4, shadedTargetColor);
					
					if (entTM.bBoxVerts[1] &&
						entTM.bBoxVerts[5] &&
						entTM.bBoxVerts[6] &&
						entTM.bBoxVerts[2]
						)
						pDraw->AddConvexPolyFilled(leftFaceVerts, 4, shadedTargetColor);
					
					if (entTM.bBoxVerts[0] &&
						entTM.bBoxVerts[1] &&
						entTM.bBoxVerts[2] &&
						entTM.bBoxVerts[3]
						)
						pDraw->AddConvexPolyFilled(frontFaceVerts, 4, shadedTargetColor);
					
					if (entTM.bBoxVerts[5] &&
						entTM.bBoxVerts[4] &&
						entTM.bBoxVerts[7] &&
						entTM.bBoxVerts[6]
						)
						pDraw->AddConvexPolyFilled(backFaceVerts, 4, shadedTargetColor);


					if (entTM.bIsActor)
					{
						/* BONES */
						for (auto point : entTM.bones)
						{
							ImVec2 r1{ point.x, point.y };
							ImVec2 r2{ point.z, point.w };
							gui::draw::CleanLine(r1 + posWndw, r2 + posWndw, target_color, 1.0f);
						}
						if (entTM.bBoneHead)
						{
							// draw head bone as a circle
							gui::draw::Circle(ImVec2(entTM.boneHead.x, entTM.boneHead.y) + posWndw, target_color, entTM.boneHeadRadius, 1.f);

							//	draw look direction line from head
							if (entTM.bBoneHeadFWD)
							{
								ImVec2 head_root = ImVec2(entTM.boneHead.x, entTM.boneHead.y) + posWndw;
								ImVec2 head_fwd = ImVec2(entTM.boneHeadFWD.x, entTM.boneHeadFWD.y) + posWndw;
								gui::draw::CleanLine(head_root, head_fwd, fwd_color, 1.0f);
							}
						}

						/* HEALTH BAR */
						//	float corner_height = abs(entTM.originTop.y - entTM.origin.y);												//	Width
						//	float corner_width = corner_height * 0.65;																	//	Height
						//	ImVec2 pos_box(entTM.originTop.x - (corner_width / 2), entTM.originTop.y);									//	Top Left Corner
						//	ImRect bbox(pos_box, ImVec2(pos_box.x + corner_width, pos_box.y + corner_height));							//	screen bounding box
						//	float baseY = bbox.Max.y + 2.0f; 
						//	float stackedYOffset = 0.0f;
						//	ImColor mColHealth(255 - health * 2.55, health * 2.55, 0);		//	health color
						//	float height = entTM.originTop.y - entTM.origin.y;
						//	float barHeight = std::clamp(height * 0.06f, 2.0f, 6.0f);
						//	float barWidth = bbox.GetWidth();
						//	float filledWidth = barWidth * (health / 100.f);
						//	float rounding = 0.f; // barHeight * 0.5f; // for nice pill shape
						//	
						//	// Position: first element in stack
						//	ImVec2 barTopLeft(bbox.Min.x, baseY + stackedYOffset);
						//	ImVec2 barBottomRight(barTopLeft.x + barWidth, barTopLeft.y + barHeight);
						//	ImVec2 filledEnd(barTopLeft.x + filledWidth, barBottomRight.y);
						//	
						//	// background
						//	pDraw->AddRect(barTopLeft, barBottomRight, IM_COL32_BLACK);
						//	pDraw->AddRectFilled(barTopLeft, filledEnd, mColHealth);
						//	
						//	// Gradient gloss overlay (top half of filled bar)
						//	ImU32 glossTop = IM_COL32(255, 255, 255, 40);   // top: faint white
						//	ImU32 glossBot = IM_COL32(255, 255, 255, 10);   // bottom: faint fade
						//	float glossHeight = barHeight * 0.5f;
						//	
						//	ImVec2 glossTopStart = barTopLeft;
						//	ImVec2 glossTopEnd = ImVec2(barTopLeft.x + filledWidth, barTopLeft.y + glossHeight);
						//	pDraw->AddRectFilledMultiColor(
						//		glossTopStart, glossTopEnd,
						//		glossTop, glossTop, glossBot, glossBot
						//	);
						//	
						//	// Inner top white line for shine
						//	ImVec2 shineStart = ImVec2(barTopLeft.x + 1, barTopLeft.y + 1);
						//	ImVec2 shineEnd = ImVec2(barTopLeft.x + filledWidth - 1, barTopLeft.y + 1);
						//	pDraw->AddLine(shineStart, shineEnd, IM_COL32(255, 255, 255, 60), 1.0f);
					}
					else
					{
						//	Forward Projection
						if (entTM.bOriginFWD && entTM.bOrigin)
							gui::draw::CleanLine(root, fwd, fwd_color, 1.0f);
					}

				}; // end fnDrawIT


				ImVec2 szWndw = ImGui::GetWindowSize();
				ImVec2 wndwCenter = szWndw * .5f;
				Classes::CEntity* pClosestFovEntity = nullptr;
				Structs::SDrawEntity mClosestTM;
				float szFov = szWndw.y * Hooks::vars::gui_FOV;
				float mClosestDist = szFov;
				bool bShouldDrawClosestEntity = false;
				const auto& pWindow = ImGui::GetCurrentWindow();
				if (!pWindow)
					return;

				ImGuiStyle style = ImGui::GetStyle();
				ImDrawList* pDraw = ImGui::GetWindowDrawList();
				const auto& szMenu = pWindow->Rect();
				auto center = szMenu.GetCenter();
				auto top_center = ImVec2({ center.x, szMenu.Min.y });
				ImColor fwd_color = ImColor(0.0f, 1.0f, 0.0f, 1.0f);

				/* render cached screen transforms */
				for (auto it : Hooks::vars::vScreenEntities)
				{
					const float& minDist = 10.f;
					bool bShouldRender{ false };
					const auto& entType = it.entityType;
					float distScalarMP{ 100.f };
					float renderDistance{ -1.f };
					ImColor entColor = ImColor(IM_COL32_WHITE);
					Structs::DVector pLocalPos = Hooks::vars::sLocalPlayer.TM.origin;

					if (it.pEntity == Hooks::vars::sLocalPlayer.pEntity || it.pEntity == Hooks::vars::sLocalPlayer.pShipEntity)
						continue;
					
					switch (entType)
					{
					case StarCitizen::Enums::EEntityType::ET_PLAYER:
						bShouldRender = Hooks::vars::esp_bPlayer;
						renderDistance = Hooks::vars::esp_PlayerRange;
						entColor = ImColor(Hooks::vars::esp_PlayerColor.x, Hooks::vars::esp_PlayerColor.y, Hooks::vars::esp_PlayerColor.z, Hooks::vars::esp_PlayerColor.w);
						break;

					case StarCitizen::Enums::EEntityType::ET_SHIP:
						bShouldRender = Hooks::vars::esp_bShip;
						renderDistance = Hooks::vars::esp_ShipRange;
						entColor = ImColor(Hooks::vars::esp_ShipColor.x, Hooks::vars::esp_ShipColor.y, Hooks::vars::esp_ShipColor.z, Hooks::vars::esp_ShipColor.w);
						break;

					case StarCitizen::Enums::EEntityType::ET_ENEMY_NPC:
						distScalarMP = 10.f;
						bShouldRender = Hooks::vars::esp_bEnemyAI;
						renderDistance = Hooks::vars::esp_EnemyAIRange;
						entColor = ImColor(Hooks::vars::esp_EnemyAIColor.x, Hooks::vars::esp_EnemyAIColor.y, Hooks::vars::esp_EnemyAIColor.z, Hooks::vars::esp_EnemyAIColor.w);
						break;

					case StarCitizen::Enums::EEntityType::ET_ANIMAL:
						distScalarMP = 10.f;
						bShouldRender = Hooks::vars::esp_bAnimals;
						renderDistance = Hooks::vars::esp_AnimalRange;
						entColor = ImColor(Hooks::vars::esp_AnimalColor.x, Hooks::vars::esp_AnimalColor.y, Hooks::vars::esp_AnimalColor.z, Hooks::vars::esp_AnimalColor.w);
						break;

					case StarCitizen::Enums::EEntityType::ET_CONTAINER:
						distScalarMP = 10.f;
						bShouldRender = Hooks::vars::esp_bLoot;
						renderDistance = Hooks::vars::esp_LootRange;
						entColor = ImColor(Hooks::vars::esp_LootColor.x, Hooks::vars::esp_LootColor.y, Hooks::vars::esp_LootColor.z, Hooks::vars::esp_LootColor.w);
						break;

					case StarCitizen::Enums::EEntityType::ET_ROCK:
						bShouldRender = Hooks::vars::esp_bRock;
						renderDistance = Hooks::vars::esp_RockRange;
						entColor = ImColor(Hooks::vars::esp_RockColor.x, Hooks::vars::esp_RockColor.y, Hooks::vars::esp_RockColor.z, Hooks::vars::esp_RockColor.w);
						break;

					case StarCitizen::Enums::EEntityType::ET_ORBIT:
						bShouldRender = Hooks::vars::esp_bOrbit;
						renderDistance = Hooks::vars::esp_OrbitRange;
						entColor = ImColor(Hooks::vars::esp_OrbitColor.x, Hooks::vars::esp_OrbitColor.y, Hooks::vars::esp_OrbitColor.z, Hooks::vars::esp_OrbitColor.w);
						break;

					default: break;
					};

					if (!bShouldRender)
						continue;

					Structs::DVector pos = it.TM.origin;
					float dist = pLocalPos.Distance(pos);	//	get distance
					if (dist > (renderDistance * distScalarMP) && renderDistance < minDist)
						continue;

					/* highlite grab entity */
					if (Hooks::vars::grab_bEnable && Hooks::vars::grab_pEntity && it.pEntity == Hooks::vars::grab_pEntity)
						entColor = ImColor(0.988, 0.760, 0.011, 1.0f);	//	grab color

					drawIT(it.screenTM, entColor, fwd_color, it.TM.health, 2);



					/* get closest target */
					if (Hooks::vars::grab_bEnable)
					{
						float distance2D = Math::GetDistance2D(wndwCenter, it.screenTM.origin);
						if (distance2D <= mClosestDist)
						{
							mClosestDist = distance2D;
							pClosestFovEntity = it.pEntity;
							mClosestTM = it.screenTM;
						}
					}
				}


				/* render target */
				if (Hooks::vars::target_player.bValid && Hooks::vars::target_player.bRender && Hooks::vars::target_player.screenTM.bValid)
				{
					const auto& ent = Hooks::vars::target_player;
					const ImColor& target_color = ImColor(Hooks::vars::target_Color.x, Hooks::vars::target_Color.y, Hooks::vars::target_Color.z, Hooks::vars::target_Color.w);

					drawIT(ent.screenTM, target_color, fwd_color, 2);
				}

				/* render line to grab entity */
				if (Hooks::vars::grab_bEnable && pClosestFovEntity)
				{
					Hooks::vars::grab_pEntity = pClosestFovEntity;
					gui::draw::CleanLine(wndwCenter + g_gui.GetCloneRect().Min, mClosestTM.origin, ImColor(0.988, 0.760, 0.011, 1.0f), 1.5f);
				}

				//	if (Hooks::vars::esp_bOrbit)
				//	{
				//		const ImColor& target_color = ImColor(Hooks::vars::esp_OrbitColor.x, Hooks::vars::esp_OrbitColor.y, Hooks::vars::esp_OrbitColor.z, Hooks::vars::esp_OrbitColor.w);
				//		for (auto it : Hooks::vars::vScreenOrbit)
				//		{
				//			const auto& segments = it.second;
				//	
				//			for (int i = 0; i < segments.size(); i++)
				//			{
				//				size_t next = (i + 1) % segments.size();
				//				ImVec2 pos = { segments[i].x, segments[i].y };
				//				ImVec2 nextPos = { segments[next].x, segments[next].y };
				//				gui::draw::CleanLine(pos, nextPos, target_color, 1.0f);
				//			}
				//		}
				//	}

				/* render watermark */
				//	gui::draw::BGTextCentered(top_center, IM_COL32_WHITE, __("OfflineCitizen - DEBUG"), IM_COL32_BLACK, 24.0f);

				/* render ufo speed */
				//	if (StarCitizen::Hooks::vars::bUFO)
				//	{
				//		char buff[32];  //  should not overflow
				//		sprintf_s(buff, __("Movement Speed: %.2f"), StarCitizen::Hooks::vars::mUFOSpeedScalar);
				//		auto bottom_center = ImVec2({ center.x, szMenu.Max.y * .75f });
				//		gui::draw::TextCentered(bottom_center, IM_COL32_WHITE, __("UFO MODE: ON"), 24.0f);
				//		bottom_center.y += 24.0f;
				//		gui::draw::TextCentered(bottom_center, IM_COL32_WHITE, buff, 24.0f);
				//	}

				/* render walk speed */
				//	if (StarCitizen::Hooks::vars::bSuperWalkSpeed)
				//	{
				//		char buff[32];  //  should not overflow
				//		sprintf_s(buff, __("Walk Speed: %.2f"), StarCitizen::Hooks::vars::mPlayerWalkSpeed);
				//		ImRect szText = gui::tools::CalcTextSize(buff, 24.f);
				//	
				//		ImVec2 top_right = ImVec2({ szMenu.GetWidth() - szText.GetWidth(), szText.Min.y + szText.GetHeight() });
				//		gui::draw::TextCentered(top_right, IM_COL32_WHITE, buff, 24.0f);
				//	}

				/* render skeleton dude */
				//	scWidgets::Skeleton(rect);
			}
		}

		namespace scWallpaper
		{
			void draw();
		}

		/* gui */

		void renderMenu()
		{
			scMenu::Header();	//	 render the main window header
			scMenu::Body();	//	render the main window body

			//	@ TODO: ?
		}

		void renderWndw()
		{
			scCanvas::draw();

		}

		void renderWall()
		{

		}

	}
}
