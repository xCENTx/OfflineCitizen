#pragma once

#define BIT(x) (1 << (x))
#define GetAddr(x) __int64(reinterpret_cast<__int64>(GetModuleHandleA(0)) + x)
#define GetModAddr(x, y) __int64(reinterpret_cast<__int64>(GetModuleHandleA(x)) + y)

namespace StarCitizen
{
	namespace Enums
	{
		enum EDataFieldType
		{
			FIELDTYPE_BOOL = 0x1,
			FIELDTYPE_UINT32 = 0x4,
			FIELDTYPE_FLOAT = 0xB,
			FIELDTYPE_CONSTCHAR_PTR = 0xD,
			FIELDTYPE_ENUM = 0xF,
			FIELDTYPE_STRUCTPTR_AND_TAG = 0x110,
		};

		enum ECvarType
		{
			CVARTYPE_INT = 0,
			CVARTYPE_FLOAT = 1,
			CVARTYPE_INT64 = 2,
			CVARTYPE_STRING = 3,
			CVARTYPE_FUNCTION = 4
		};

		enum EBones : unsigned int
		{
			root = 0,
			pelvis = 1,
			spine_01 = 2,
			spine_02 = 3,
			spine_03 = 4,
			neck = 5,
			r_eye = 6,
			l_eye = 7,
			r_foot = 10,
			l_foot = 11,
			r_shoulder = 12,
			l_shoulder = 13,
			r_elbow = 14,
			left_elbow = 15,
			r_hand = 16,
			l_hand = 17,
			r_knee = 18,
			l_knee = 19,
			head = 20,
			r_hip = 21,
			l_hip = 22,
		}; 

		enum EQuantumSpoolState : int
		{
			ESPOOL_OFF = 0,
			ESPOOL_SPOOLING,
			ESPOOL_COMPLETE
		};

		enum EQuantumJumpState : int
		{
			EJUMP_NONE = 0,
			EJUMP_NEUTRAL,
			EJUMP_ADJUSTING,
			EJUMP_INPROGRESS,
			EJUMP_INPROGRESS_4,
			EJUMP_UPDATE_5,
			EJUMP_FINISHED,
			EJUMP_UPDATE_7,
			EJUMP_UPDATE_8,
			EJUMP_UPDATE_9,
			EJUMP_COMPLETE,
		};
	}

	namespace Structs
	{
		using namespace Enums;

		struct FVector2D
		{
			float x;	//0x0000
			float y;	//0x0004

			FVector2D() : x(0), y(0) {}
			FVector2D(float x, float y) : x(x), y(y) {}
			FVector2D(const ImVec2& v) : x(v.x), y(v.y) {}

			operator ImVec2() const { return ImVec2(x, y); }

		};	//Size: 0x0008

		struct FVector
		{
			float x;	//0x0000
			float y;	//0x0004
			float z;	//0x0008

			FVector() : x(0), y(0), z(0) {}
			FVector(float x, float y, float z) : x(x), y(y), z(z) {}

			FVector operator+(const FVector& v) const;
			FVector operator-(const FVector& v) const;
			FVector operator*(const FVector& v) const;
			FVector operator*(const float scalar) const;
			FVector& operator+=(const FVector& other);
			FVector& operator-=(const FVector& other);
			FVector& operator+=(const float* other);
			FVector& operator-=(const float* other);
			friend FVector operator*(const float scalar, const FVector& other)
			{
				return { other.x * scalar, other.y * scalar, other.z * scalar };
			}

			float Dot(const FVector& v) const;
			float Length() const;
			float Distance(const FVector& v) const;
			FVector Normalize();
			FVector Cross(const FVector& other);
			bool IsValid();

			FVector RotatePointAroundX(float angleRadians);
			FVector RotatePointAroundY(float angleRadians);
			FVector RotatePointAroundZ(float angleRadians);
		};	//Size: 0x000C

		struct FRotator
		{
			float Pitch;	//0x0000
			float Yaw;	//0x0004
			float Roll;	//0x0008
		};	//Size: 0x000C

		struct FQuat
		{
			float x;	//0x0000
			float y;	//0x0004
			float z;	//0x0008
			float w;	//0x000C

			FQuat() : x(0), y(0), z(0), w(0) {}
			FQuat(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

			FQuat operator+(const FQuat& other) const;
			FQuat operator-(const FQuat& other) const;
			FQuat operator*(float scalar) const;
			FQuat operator/(float scalar) const;
			FQuat operator=(float* other) const;
			FQuat& operator*=(const FQuat& other);
			FQuat operator* (const FQuat& other) const;	// Multiply two quaternions

			FQuat GetConjugate() const;	// Get the conjugate of the quaternion
		};	//Size: 0x0010

		struct DVector
		{
			double x;	//0x0000
			double y;	//0x0008
			double z;	//0x0010

			DVector() : x(0), y(0), z(0) {}
			DVector(double x, double y, double z) : x(x), y(y), z(z) {}

			DVector operator+(const DVector& v) const;
			DVector operator+(float* v) const;
			DVector operator-(const DVector& v) const;
			DVector& operator+=(const DVector& other);
			DVector& operator-=(const DVector& other);
			DVector& operator+=(const float* other);
			DVector& operator-=(const float* other);
			DVector operator+(const FVector& other) const;
			DVector operator-(const FVector& other) const;

			friend DVector operator*(const double scalar, const DVector& other)
			{
				return { other.x * scalar, other.y * scalar, other.z * scalar };
			}

			double Dot(const DVector& v) const;
			double Length() const;
			double Distance(const DVector& v) const;
			DVector Normalize();
			DVector Cross(const DVector& other);
			bool IsValid();
		};	//Size: 0x0018

		struct Vector4
		{
			float x, y, z, w;
		};  //  Size: 0x0010

		struct DRotator
		{
			double Pitch;	//0x0000
			double Yaw;	//0x0008
			double Roll;	//0x0010
		};	//Size: 0x0018

		struct DQuat
		{
			double x;	//0x0000
			double y;	//0x0008
			double z;	//0x0010
			double w;	//0x0018
		};	//Size: 0x0020

		struct DBox
		{
			DVector					m_min;	//0x0000
			DVector					m_max;	//0x0018

			DBox();
			DBox(DVector mmin, DVector mmax);

			double GetWidth();
			double GetHeight();
			double GetDepth();
			DVector GetExtents();
			DVector GetCenter();
			bool IsColliding(const DBox& other);
		};	//Size: 0x0030

		struct Matrix33
		{
			float m[3][3]; // 3x3 matrix representation

			FVector GetColumn0() const;
			FVector GetColumn1() const;
			FVector GetColumn2() const;
		};

		struct Matrix34
		{
			float m[3][4]; // 3x4 matrix representation

			FVector GetColumn0() const;
			FVector GetColumn1() const;
			FVector GetColumn2() const;
			FVector GetColumn3() const;
		};

		struct Matrix44
		{
			//	float m[4][4]; // 4x4 matrix representation
			float m[16];

			FQuat GetColumn0() const;
			FQuat GetColumn1() const;
			FQuat GetColumn2() const;
			FQuat GetColumn3() const;
		};

		struct DMatrix44
		{
			double m[16]; // 4x4 matrix representation
			DQuat GetColumn0() const;
			DQuat GetColumn1() const;
			DQuat GetColumn2() const;
			DQuat GetColumn3() const;
		};

		struct AABB
		{
			FVector mMin;	//0x0000
			FVector mMax;	//0x000C

			AABB();
			AABB(FVector mmin, FVector mmax);

			float GetWidth();
			float GetHeight();
			float GetDepth();
			FVector GetExtents();
			FVector GetCenter();
		};	//Size: 0x0018

		struct VArray
		{
			__int64 max;	//0x0000
			__int64 count;	//0x0008
			__int64 cache;	//0x0010
			__int64 data;	//0x0018
		};	//Size: 0x0020

		struct SDataField
		{
			const char*				fieldName;
			__int64					fieldOffset;
			__int64					fieldSize;
			__int64					fieldType;
		};	//Size: 0x0020
		
		struct StructDataFields
		{
			SDataField*				dataFields[100];
			void*					unk1;
			void*					unk2;
		};	//Size: 0x0140
		
		struct SXCvar
		{
			const char*				Name;
			const char*				Description;
			void*					flagPTR;
			void*					cxClass;
			ECvarType				type;

			template<typename T>
			const T GetValue() noexcept
			{
				if (!flagPTR)
					return T{};

				switch (type)
				{
				case CVARTYPE_INT: return *reinterpret_cast<T*>(flagPTR);
				case CVARTYPE_FLOAT: return *reinterpret_cast<T*>(flagPTR);
				case CVARTYPE_INT64: return *reinterpret_cast<T*>(flagPTR);
				case CVARTYPE_STRING: return *reinterpret_cast<T*>(flagPTR);
				}

				return T{};
			}

			template<typename T>
			void SetValue(T value) noexcept
			{
				if (!flagPTR)
					return;

				switch (type)
				{
				case CVARTYPE_INT: break;
				case CVARTYPE_FLOAT: break;
				case CVARTYPE_INT64: break;
				case CVARTYPE_STRING: return;
				default: return;
				}

				*reinterpret_cast<T*>(flagPTR) = value;
			}

		};	//Size: 0x0020
		
		struct SXCommand
		{
			static char*			g_CmdArgs[100];	//	used by the vtable , do not touch
			static int				g_CmdArgCount;	//	used by the vtable , do not touch
			const char*				Name;
			const char*				Description;
			void*					fnPtr;
			void*					cxClass;
			unsigned int			flags;

			virtual void nullfn();					//	0x0
			virtual int GetArgCount();				//	0x8
			virtual __int64 GetArg(int index);		//	0x10
			virtual __int64 nullfn2();				//	0x18
			virtual __int64 nullfn3(__int64* out);	//	0x20

			void ExecuteCmd(int argc, char* argv[]);
		};	//Size: 0x0020

		struct SDrawText
		{
		public:
			unsigned int flag{ 97 };
			FQuat color{ 1.0f, 1.0f, 1.0f, 1.0f };
			FVector2D scale{ 2.0f, 2.0f };
		};

		struct SGoToPointInfo
		{
		public:
			char* pName;    //0x0000
			char* pDescription;    //0x0008
			char pad_0010[8];    //0x0010
		};    //Size: 0x0018

		struct SGoToPoint
		{
			unsigned int			N00000226;    //0x0000
			unsigned int			N00000230;    //0x0004
			struct SGoToPointInfo*	pPointInfo;    //0x0008
			char					pad_0010[16];   //0x0010
		};    //Size: 0x0020

		struct SGoToPointAZ
		{
			struct SGoToPoint		POINTS[100];    //0x0000
		};    //Size: 0x0BE0

		struct STravelEnvelope
		{
			char					pad_0000[24];	//0x0000
			FVector					min;			//0x0018
			FVector					max;			//0x0030
			Vector4					angles;			//0x0048
		};

		struct SSpreadParams
		{
			unsigned char pad_0[0x8]; // 0x0
			float	min; // 0x8
			float	max; // 0xc
			float	firstAttack; // 0x10
			float	attack; // 0x14
			float	decay; // 0x18
		}; // Size: 0x1c

		struct SProjectileLauncher
		{
			unsigned char pad_0[0x8]; // 0x0
			unsigned char pad_8[0x10]; // 0x8 | Name: fireHelper | Size: 0x10 | Type: 0xa
			unsigned char pad_18[0x10]; // 0x18 | Name: muzzleHelper | Size: 0x10 | Type: 0xa
			__int32	ammoCost; // 0x28
			__int32	pelletCount; // 0x2c
			float	damageMultiplier; // 0x30
			float	soundRadius; // 0x34
			SSpreadParams						spreadParams;			// 0x38 
			unsigned char pad_58[0x4]; // 0x58 | Name: projectileType | Size: 0x4 | Type: 0xf
		}; // Size: 0x5c

		struct SWeaponActionFireTractorBeamParams
		{
			unsigned char pad_0[0x8]; // 0x0
			unsigned char pad_8[0x10]; // 0x8 | Name: name | Size: 0x10 | Type: 0xa
			char* localisedName; // 0x18
			unsigned char pad_20[0x20]; // 0x20 | Name: mannequinTag | Size: 0x20 | Type: 0x10
			unsigned char pad_40[0x18]; // 0x40 | Name: entityTag | Size: 0x18 | Type: 0x310
			unsigned char pad_58[0x38]; // 0x58 | Name: entityTags | Size: 0x38 | Type: 0x10
			unsigned char pad_90[0x18]; // 0x90 | Name: uiBindingsTag | Size: 0x18 | Type: 0x310
			unsigned char pad_a8[0x1]; // 0xa8 | Name: aiShootingMode | Size: 0x1 | Type: 0xf
			unsigned char pad_a9[0x7]; // 0xa9
			unsigned char pad_b0[0x20]; // 0xb0 | Name: switchFireModeAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_d0[0x10]; // 0xd0 | Name: selectableCondition | Size: 0x10 | Type: 0x110
			bool	hasReloadModesOnUI; // 0xe0
			unsigned char pad_e1[0x7]; // 0xe1
			char* localisedLiftingFunctionalityName; // 0xe8
			char* localisedRotationFunctionalityName; // 0xf0
			unsigned char pad_f8[0x18]; // 0xf8 | Name: liftingFunctionalityTag | Size: 0x18 | Type: 0x310
			unsigned char pad_110[0x18]; // 0x110 | Name: rotationFunctionalityTag | Size: 0x18 | Type: 0x310
			unsigned char pad_128[0x10]; // 0x128 | Name: fireHelper | Size: 0x10 | Type: 0xa
			bool	toggle; // 0x138
			unsigned char pad_139[0x3]; // 0x139
			float	minForce; // 0x13c
			float	maxForce; // 0x140
			float	additionalForceDuringZeroGHandholding; // 0x144
			float	minDistance; // 0x148
			float	maxDistance; // 0x14c
			float	fullStrengthDistance; // 0x150
			float	maxAngle; // 0x154
			float	maxVolume; // 0x158
			float	volumeForceCoefficient; // 0x15c
			float	heatPerSecond; // 0x160
			float	wearPerSecond; // 0x164
			float	hitRadius; // 0x168
			float	tetherBreakTime; // 0x16c
			float	safeRangeValueFactor; // 0x170
			float	maxPlayerLookRotationScale; // 0x174
			bool	allowScrollingIntoBreakingRange; // 0x178
			bool	shouldDryFireInGreenZones; // 0x179
			bool	shouldFireInHangars; // 0x17a
			bool	shouldTractorSelf; // 0x17b
			unsigned char pad_17c[0x4]; // 0x17c
			unsigned char pad_180[0x30]; // 0x180 | Name: entityTagBlacklist | Size: 0x30 | Type: 0x10310
			unsigned char pad_1b0[0x4]; // 0x1b0 | Name: ammoType | Size: 0x4 | Type: 0xf
			float	minEnergyDraw; // 0x1b4
			float	maxEnergyDraw; // 0x1b8
			unsigned char pad_1bc[0x4]; // 0x1bc
			unsigned char pad_1c0[0x10]; // 0x1c0 | Name: hitType | Size: 0x10 | Type: 0xa
			unsigned char pad_1d0[0x10]; // 0x1d0 | Name: actorStatusBuff | Size: 0x10 | Type: 0x110
			unsigned char pad_1e0[0x18]; // 0x1e0 | Name: recoil | Size: 0x18 | Type: 0x310
			unsigned char pad_1f8[0x18]; // 0x1f8 | Name: recoilGrappling | Size: 0x18 | Type: 0x310
			float	recoilInterval; // 0x210
			unsigned char pad_214[0x4]; // 0x214
			unsigned char pad_218[0x20]; // 0x218 | Name: fireFragment | Size: 0x20 | Type: 0x10
			unsigned char pad_238[0x20]; // 0x238 | Name: stopFireFragment | Size: 0x20 | Type: 0x10
			unsigned char pad_258[0x20]; // 0x258 | Name: shootTargetFragment | Size: 0x20 | Type: 0x10
			unsigned char pad_278[0x20]; // 0x278 | Name: startFireOneShotAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_298[0x20]; // 0x298 | Name: startFireLoopAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_2b8[0x20]; // 0x2b8 | Name: stopFireOneShotAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_2d8[0x20]; // 0x2d8 | Name: stopFireLoopAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_2f8[0x20]; // 0x2f8 | Name: beamBrokenOneShotAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_318[0x20]; // 0x318 | Name: beamAttachedOneShotAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_338[0x20]; // 0x338 | Name: dryFireAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_358[0x20]; // 0x358 | Name: startRotationModeAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_378[0x20]; // 0x378 | Name: stopRotationModeAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_398[0x20]; // 0x398 | Name: startGrappleAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_3b8[0x20]; // 0x3b8 | Name: stopGrappleAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_3d8[0x20]; // 0x3d8 | Name: shootTargetAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_3f8[0x20]; // 0x3f8 | Name: timeSinceLastFireRTPC | Size: 0x20 | Type: 0x10
			unsigned char pad_418[0x20]; // 0x418 | Name: tractorBeamAccelerationRTPC | Size: 0x20 | Type: 0x10
			unsigned char pad_438[0x20]; // 0x438 | Name: tractorBeamSpeedRTPC | Size: 0x20 | Type: 0x10
			unsigned char pad_458[0x20]; // 0x458 | Name: rotationModeAmountRTPC | Size: 0x20 | Type: 0x10
			unsigned char pad_478[0x20]; // 0x478 | Name: tetherWarningStateRTPC | Size: 0x20 | Type: 0x10
			unsigned char pad_498[0x30]; // 0x498 | Name: fireEffects | Size: 0x30 | Type: 0x10010
			unsigned char pad_4c8[0x20]; // 0x4c8 | Name: tractorBeamNormDistanceRTPC | Size: 0x20 | Type: 0x10
			unsigned char pad_4e8[0x20]; // 0x4e8 | Name: tractorBeamNormForceRTPC | Size: 0x20 | Type: 0x10
			unsigned char pad_508[0x20]; // 0x508 | Name: tractorBeamObjectHeldRTPC | Size: 0x20 | Type: 0x10
			unsigned char pad_528[0x30]; // 0x528 | Name: shootTargetEffects | Size: 0x30 | Type: 0x10010
			unsigned char pad_558[0x10]; // 0x558 | Name: beamGroup | Size: 0x10 | Type: 0x110
			unsigned char pad_568[0x38]; // 0x568 | Name: beamStrengthValues | Size: 0x38 | Type: 0x10
			unsigned char pad_5a0[0x18]; // 0x5a0 | Name: inputParams | Size: 0x18 | Type: 0x10
			unsigned char pad_5b8[0x28]; // 0x5b8 | Name: movementParams | Size: 0x28 | Type: 0x10
			unsigned char pad_5e0[0x208]; // 0x5e0 | Name: attachDetachParams | Size: 0x208 | Type: 0x10
			unsigned char pad_7e8[0x20]; // 0x7e8 | Name: rotationParams | Size: 0x20 | Type: 0x10
			unsigned char pad_808[0x20]; // 0x808 | Name: grappleParams | Size: 0x20 | Type: 0x10
			unsigned char pad_828[0x20]; // 0x828 | Name: vehicleParams | Size: 0x20 | Type: 0x10
			unsigned char pad_848[0x28]; // 0x848 | Name: multitractorParams | Size: 0x28 | Type: 0x10
		}; // Size: 0x870

		struct SWeaponActionFireHealingBeamParams
		{
			unsigned char pad_0[0x8]; // 0x0
			unsigned char pad_8[0x10]; // 0x8 | Name: name | Size: 0x10 | Type: 0xa
			char* localisedName; // 0x18
			unsigned char pad_20[0x20]; // 0x20 | Name: mannequinTag | Size: 0x20 | Type: 0x10
			unsigned char pad_40[0x18]; // 0x40 | Name: entityTag | Size: 0x18 | Type: 0x310
			unsigned char pad_58[0x38]; // 0x58 | Name: entityTags | Size: 0x38 | Type: 0x10
			unsigned char pad_90[0x18]; // 0x90 | Name: uiBindingsTag | Size: 0x18 | Type: 0x310
			unsigned char pad_a8[0x1]; // 0xa8 | Name: aiShootingMode | Size: 0x1 | Type: 0xf
			unsigned char pad_a9[0x7]; // 0xa9
			unsigned char pad_b0[0x20]; // 0xb0 | Name: switchFireModeAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_d0[0x10]; // 0xd0 | Name: selectableCondition | Size: 0x10 | Type: 0x110
			bool	hasReloadModesOnUI; // 0xe0
			unsigned char pad_e1[0x7]; // 0xe1
			unsigned char pad_e8[0x4]; // 0xe8 | Name: healingMode | Size: 0x4 | Type: 0xf
			bool	externalHealingMode; // 0xec
			unsigned char pad_ed[0x3]; // 0xed
			unsigned char pad_f0[0x10]; // 0xf0 | Name: fireHelper | Size: 0x10 | Type: 0xa
			bool	toggle; // 0x100
			unsigned char pad_101[0x3]; // 0x101
			float	maxDistance; // 0x104
			float	maxSensorDistance; // 0x108
			unsigned char pad_10c[0x4]; // 0x10c | Name: medicalAmmoType | Size: 0x4 | Type: 0xf
			float	mSCUPerSec; // 0x110
			float	ammoPerMSCU; // 0x114
			float	wearPerSec; // 0x118
			unsigned char pad_11c[0x4]; // 0x11c | Name: batteryAmmoType | Size: 0x4 | Type: 0xf
			float	batteryDrainPerSec; // 0x120
			float	autoDosageTargetBDLModifier; // 0x124
			float	healingBreakTime; // 0x128
			float	maxDoseForAutoAdjustment; // 0x12c
			unsigned char pad_130[0x10]; // 0x130 | Name: hitType | Size: 0x10 | Type: 0xa
			unsigned char pad_140[0x30]; // 0x140 | Name: consumableTypes | Size: 0x30 | Type: 0x10010
			unsigned char pad_170[0x18]; // 0x170 | Name: recoil | Size: 0x18 | Type: 0x310
			float	recoilInterval; // 0x188
			unsigned char pad_18c[0x4]; // 0x18c
			unsigned char pad_190[0x20]; // 0x190 | Name: fireFragment | Size: 0x20 | Type: 0x10
			unsigned char pad_1b0[0x20]; // 0x1b0 | Name: stopFireFragment | Size: 0x20 | Type: 0x10
			unsigned char pad_1d0[0x20]; // 0x1d0 | Name: startFireOneShotAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_1f0[0x20]; // 0x1f0 | Name: startFireLoopAudioTrigger | Size: 0x20 | Type: 0x10
		}; // Size: 0x210

		struct SWeaponActionFireSalvageRepairParams
		{
			unsigned char pad_0[0x8]; // 0x0
			unsigned char pad_8[0x10]; // 0x8 | Name: name | Size: 0x10 | Type: 0xa
			char*	localisedName; // 0x18
			unsigned char pad_20[0x20]; // 0x20 | Name: mannequinTag | Size: 0x20 | Type: 0x10
			unsigned char pad_40[0x18]; // 0x40 | Name: entityTag | Size: 0x18 | Type: 0x310
			unsigned char pad_58[0x38]; // 0x58 | Name: entityTags | Size: 0x38 | Type: 0x10
			unsigned char pad_90[0x18]; // 0x90 | Name: uiBindingsTag | Size: 0x18 | Type: 0x310
			unsigned char pad_a8[0x1]; // 0xa8 | Name: aiShootingMode | Size: 0x1 | Type: 0xf
			unsigned char pad_a9[0x7]; // 0xa9
			unsigned char pad_b0[0x20]; // 0xb0 | Name: switchFireModeAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_d0[0x10]; // 0xd0 | Name: selectableCondition | Size: 0x10 | Type: 0x110
			bool	hasReloadModesOnUI; // 0xe0
			unsigned char pad_e1[0x7]; // 0xe1
			char*	localisedFunctionalityName; // 0xe8
			unsigned char pad_f0[0x18]; // 0xf0 | Name: functionalityTag | Size: 0x18 | Type: 0x310
			unsigned char pad_108[0x4]; // 0x108 | Name: salvageRepairMode | Size: 0x4 | Type: 0xf
			unsigned char pad_10c[0x4]; // 0x10c
			unsigned char pad_110[0x10]; // 0x110 | Name: fireHelper | Size: 0x10 | Type: 0xa
			bool	toggle; // 0x120
			bool	salvageCanFireOnFull; // 0x121
			unsigned char pad_122[0x2]; // 0x122
			unsigned char pad_124[0x4]; // 0x124 | Name: salvageRepairAmmoType | Size: 0x4 | Type: 0xf
			unsigned char pad_128[0x4]; // 0x128 | Name: batteryAmmoType | Size: 0x4 | Type: 0xf
			float	minEnergyDraw; // 0x12c
			float	maxEnergyDraw; // 0x130
			float	materialEfficiency; // 0x134
			float	maxVehicleDamageRatio; // 0x138
			unsigned char pad_13c[0x4]; // 0x13c
			unsigned char pad_140[0x18]; // 0x140 | Name: maxRepairRatio | Size: 0x18 | Type: 0x10
			unsigned char pad_158[0x18]; // 0x158 | Name: minRepairRatio | Size: 0x18 | Type: 0x10
			float	repairedMaterialRatio; // 0x170
			float	maxHealthRepairRate; // 0x174
			float	healthToAmmoRatio; // 0x178
			unsigned char pad_17c[0x4]; // 0x17c
			unsigned char pad_180[0x18]; // 0x180 | Name: rangeParams | Size: 0x18 | Type: 0x10
			float	heatPerSecond; // 0x198
			float	wearPerSecond; // 0x19c
			float	hitRadius; // 0x1a0
			float	materialGatheringGracePeriod; // 0x1a4
			float	aimPointThicknessDamageThreshold; // 0x1a8
			unsigned char pad_1ac[0x4]; // 0x1ac
			unsigned char pad_1b0[0x18]; // 0x1b0 | Name: startDamageScales | Size: 0x18 | Type: 0x10
			unsigned char pad_1c8[0x18]; // 0x1c8 | Name: endDamageScales | Size: 0x18 | Type: 0x10
			float	rampUpTime; // 0x1e0
			float	rampDownTime; // 0x1e4
			float	damageThreshold; // 0x1e8
			unsigned char pad_1ec[0x4]; // 0x1ec
			unsigned char pad_1f0[0x70]; // 0x1f0 | Name: damageMapValues | Size: 0x70 | Type: 0x10
			unsigned char pad_260[0x30]; // 0x260 | Name: glowParams | Size: 0x30 | Type: 0x10
			unsigned char pad_290[0x10]; // 0x290 | Name: hitType | Size: 0x10 | Type: 0xa
			bool	ignoreSkiplist; // 0x2a0
			bool	shouldDryFireInGreenZones; // 0x2a1
			unsigned char pad_2a2[0x6]; // 0x2a2
			unsigned char pad_2a8[0x18]; // 0x2a8 | Name: recoil | Size: 0x18 | Type: 0x310
			float	recoilInterval; // 0x2c0
			unsigned char pad_2c4[0x4]; // 0x2c4
			unsigned char pad_2c8[0x20]; // 0x2c8 | Name: fireFragment | Size: 0x20 | Type: 0x10
			unsigned char pad_2e8[0x20]; // 0x2e8 | Name: stopFireFragment | Size: 0x20 | Type: 0x10
			unsigned char pad_308[0x20]; // 0x308 | Name: startFireOneShotAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_328[0x20]; // 0x328 | Name: startFireLoopAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_348[0x20]; // 0x348 | Name: stopFireAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_368[0x20]; // 0x368 | Name: dryFireAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_388[0x20]; // 0x388 | Name: startBeamImpactAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_3a8[0x20]; // 0x3a8 | Name: stopBeamImpactAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_3c8[0x20]; // 0x3c8 | Name: timeSinceLastFireRTPC | Size: 0x20 | Type: 0x10
			unsigned char pad_3e8[0x20]; // 0x3e8 | Name: salvageRadiusRTPC | Size: 0x20 | Type: 0x10
			unsigned char pad_408[0x20]; // 0x408 | Name: salvageStateRTPC | Size: 0x20 | Type: 0x10
			unsigned char pad_428[0x20]; // 0x428 | Name: salvageExtractionIntensityRTPC | Size: 0x20 | Type: 0x10
		}; // Size: 0x448

		struct SWeaponActionFireSingleParams
		{
			unsigned char pad_0[0x8]; // 0x0
			unsigned char pad_8[0x10]; // 0x8 | Name: name | Size: 0x10 | Type: 0xa
			char* localisedName; // 0x18
			unsigned char pad_20[0x20]; // 0x20 | Name: mannequinTag | Size: 0x20 | Type: 0x10
			unsigned char pad_40[0x18]; // 0x40 | Name: entityTag | Size: 0x18 | Type: 0x310
			unsigned char pad_58[0x38]; // 0x58 | Name: entityTags | Size: 0x38 | Type: 0x10
			unsigned char pad_90[0x18]; // 0x90 | Name: uiBindingsTag | Size: 0x18 | Type: 0x310
			unsigned char pad_a8[0x1]; // 0xa8 | Name: aiShootingMode | Size: 0x1 | Type: 0xf
			unsigned char pad_a9[0x7]; // 0xa9
			unsigned char pad_b0[0x20]; // 0xb0 | Name: switchFireModeAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_d0[0x10]; // 0xd0 | Name: selectableCondition | Size: 0x10 | Type: 0x110
			bool	hasReloadModesOnUI; // 0xe0
			unsigned char pad_e1[0x7]; // 0xe1
			struct SProjectileLauncher* p_launchParams;	//	0xe8 | Name: launchParams | Size: 0x10 | Type: 0x110
			unsigned char						pad_f0[0x8];	//	0xf0
			__int32	fireRate; // 0xf8
			float	heatPerShot; // 0xfc
			float	wearPerShot; // 0x100
			unsigned char pad_104[0x4]; // 0x104
			unsigned char pad_108[0x18]; // 0x108 | Name: recoil | Size: 0x18 | Type: 0x310
		}; // Size: 0x120

		struct SWeaponActionFireBurstParams
		{
			unsigned char pad_0[0x8]; // 0x0
			unsigned char pad_8[0x10]; // 0x8 | Name: name | Size: 0x10 | Type: 0xa
			char* localisedName; // 0x18
			unsigned char pad_20[0x20]; // 0x20 | Name: mannequinTag | Size: 0x20 | Type: 0x10
			unsigned char pad_40[0x18]; // 0x40 | Name: entityTag | Size: 0x18 | Type: 0x310
			unsigned char pad_58[0x38]; // 0x58 | Name: entityTags | Size: 0x38 | Type: 0x10
			unsigned char pad_90[0x18]; // 0x90 | Name: uiBindingsTag | Size: 0x18 | Type: 0x310
			unsigned char pad_a8[0x1]; // 0xa8 | Name: aiShootingMode | Size: 0x1 | Type: 0xf
			unsigned char pad_a9[0x7]; // 0xa9
			unsigned char pad_b0[0x20]; // 0xb0 | Name: switchFireModeAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_d0[0x10]; // 0xd0 | Name: selectableCondition | Size: 0x10 | Type: 0x110
			bool	hasReloadModesOnUI; // 0xe0
			unsigned char pad_e1[0x7]; // 0xe1
			struct SProjectileLauncher* p_launchParams;	//	0xe8 | Name: launchParams | Size: 0x10 | Type: 0x110
			unsigned char						pad_f0[0x8];	//	0xf0
			unsigned char pad_f8[0x1]; // 0xf8 | Name: shotCount | Size: 0x1 | Type: 0x6
			unsigned char pad_f9[0x3]; // 0xf9
			__int32	fireRate; // 0xfc
			float	heatPerShot; // 0x100
			float	wearPerShot; // 0x104
			float	cooldownTime; // 0x108
			unsigned char pad_10c[0x4]; // 0x10c
			unsigned char pad_110[0x18]; // 0x110 | Name: recoil | Size: 0x18 | Type: 0x310
			unsigned char pad_128[0x18]; // 0x128 | Name: misfire | Size: 0x18 | Type: 0x310
			unsigned char pad_140[0x20]; // 0x140 | Name: fireFragment | Size: 0x20 | Type: 0x10
		}; // Size: 0x160

		struct SWeaponActionFireRapidParams
		{
			unsigned char pad_0[0x8]; // 0x0
			unsigned char pad_8[0x10]; // 0x8 | Name: name | Size: 0x10 | Type: 0xa
			char* localisedName; // 0x18
			unsigned char pad_20[0x20]; // 0x20 | Name: mannequinTag | Size: 0x20 | Type: 0x10
			unsigned char pad_40[0x18]; // 0x40 | Name: entityTag | Size: 0x18 | Type: 0x310
			unsigned char pad_58[0x38]; // 0x58 | Name: entityTags | Size: 0x38 | Type: 0x10
			unsigned char pad_90[0x18]; // 0x90 | Name: uiBindingsTag | Size: 0x18 | Type: 0x310
			unsigned char pad_a8[0x1]; // 0xa8 | Name: aiShootingMode | Size: 0x1 | Type: 0xf
			unsigned char pad_a9[0x7]; // 0xa9
			unsigned char pad_b0[0x20]; // 0xb0 | Name: switchFireModeAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_d0[0x10]; // 0xd0 | Name: selectableCondition | Size: 0x10 | Type: 0x110
			bool	hasReloadModesOnUI; // 0xe0
			unsigned char pad_e1[0x7]; // 0xe1
			struct SProjectileLauncher* p_launchParams;	//	0xe8 | Name: launchParams | Size: 0x10 | Type: 0x110
			unsigned char						pad_f0[0x8];	//	0xf0
			__int32	fireRate; // 0xf8
			float	heatPerShot; // 0xfc
			float	wearPerShot; // 0x100
			float	spinUpTime; // 0x104
			float	spinDownTime; // 0x108
			bool	fireDuringSpinUp; // 0x10c
			unsigned char pad_10d[0x3]; // 0x10d
			unsigned char pad_110[0x18]; // 0x110 | Name: recoil | Size: 0x18 | Type: 0x310
			unsigned char pad_128[0x18]; // 0x128 | Name: misfire | Size: 0x18 | Type: 0x310
			unsigned char pad_140[0x20]; // 0x140 | Name: fireFragment | Size: 0x20 | Type: 0x10
			unsigned char pad_160[0x20]; // 0x160 | Name: stopFireFragment | Size: 0x20 | Type: 0x10
			unsigned char pad_180[0x20]; // 0x180 | Name: oneShotFireFragment | Size: 0x20 | Type: 0x10
			unsigned char pad_1a0[0x20]; // 0x1a0 | Name: spinFragment | Size: 0x20 | Type: 0x10
			unsigned char pad_1c0[0x10]; // 0x1c0 | Name: spinParam | Size: 0x10 | Type: 0xa
			float	audioLoopTimeBetweenFirstShots; // 0x1d0
			unsigned char pad_1d4[0x4]; // 0x1d4
			unsigned char pad_1d8[0x20]; // 0x1d8 | Name: startFireAudioTrigger | Size: 0x20 | Type: 0x10
			unsigned char pad_1f8[0x20]; // 0x1f8 | Name: startFireAudioTriggerOneShot | Size: 0x20 | Type: 0x10
			unsigned char pad_218[0x20]; // 0x218 | Name: stopFireAudioTrigger | Size: 0x20 | Type: 0x10
		}; // Size: 0x238

		struct SSalvageStructuralParams
		{
			unsigned char pad_0[0x8]; // 0x0
			__int32	numFieldSupportersRequired; // 0x8
			unsigned char pad_c[0x4]; // 0xc
			unsigned char pad_10[0x38]; // 0x10 | Name: fieldEmitterArea | Size: 0x38 | Type: 0x10
			unsigned char pad_48[0x38]; // 0x48 | Name: vectorFieldAreaVFX | Size: 0x38 | Type: 0x10
			float	fractureTimePerRadiusMetre; // 0x80
			float	minFracturableRadius; // 0x84
			float	maxFracturableRadius; // 0x88
			float	minDisintegratableRadius; // 0x8c
			float	maxDisintegratableRadius; // 0x90
			float	disintegrationTimePerRadiusMetre; // 0x94
			float	disintegrationSCUPerCubicMetre; // 0x98
			unsigned char pad_9c[0x4]; // 0x9c
			unsigned char pad_a0[0x18]; // 0xa0 | Name: disintegrationResourceType | Size: 0x18 | Type: 0x310
			unsigned char pad_b8[0x10]; // 0xb8 | Name: startGrinderInteraction | Size: 0x10 | Type: 0xa
			unsigned char pad_c8[0x10]; // 0xc8 | Name: stopGrinderInteraction | Size: 0x10 | Type: 0xa
			bool	fieldAlignmentSpeedMultiplierEnabledFracture; // 0xd8
			bool	fieldAlignmentSpeedMultiplierEnabledDisintegration; // 0xd9
			bool	fieldAlignmentYieldMultiplierEnabledDisintegration; // 0xda
			unsigned char pad_db[0x1]; // 0xdb
			float	fieldAlignmentFalloffDistanceForward; // 0xdc
			float	fieldAlignmentFalloffDistanceRadial; // 0xe0
			float	fieldAlignmentSweetSpotForwardFactor; // 0xe4
			float	fieldAlignmentMaxSpeedMultiplier; // 0xe8
			float	fieldAlignmentMaxYieldMultiplier; // 0xec
			float	fieldAlignmentBaselineFactor; // 0xf0
		}; // Size: 0xf4

		struct SSalvageCargoParams
		{
			unsigned char pad_0[0x8]; // 0x0
			unsigned char pad_8[0x18]; // 0x8 | Name: conveyorRetractedStateTag | Size: 0x18 | Type: 0x310
			unsigned char pad_20[0x18]; // 0x20 | Name: conveyorDeployedStateTag | Size: 0x18 | Type: 0x310
			unsigned char pad_38[0x10]; // 0x38 | Name: ejectCargoBoxInteration | Size: 0x10 | Type: 0xa
			unsigned char pad_48[0x10]; // 0x48 | Name: conveyorResetInteration | Size: 0x10 | Type: 0xa
			unsigned char pad_58[0x38]; // 0x58 | Name: obstructionArea | Size: 0x38 | Type: 0x10
			char* cargoTabLocString; // 0x90
			char* createTabLocString; // 0x98
			float	extractionRateUpdateInterval; // 0xa0
			float	extractionRateAveragingInterval; // 0xa4
			float	minCargoBoxSize; // 0xa8
			float	maxCargoBoxSize; // 0xac
			float	boxFillingTimePerSCU; // 0xb0
		}; // Size: 0xb4

		struct SCItemSalvageControllerParams
		{
			unsigned char pad_0[0x18]; // 0x0
			unsigned char pad_18[0x10]; // 0x18 | Name: armParams | Size: 0x10 | Type: 0x110
			unsigned char pad_28[0x10]; // 0x28 | Name: scrapingParams | Size: 0x10 | Type: 0x110
			SSalvageStructuralParams* pStructuralParams; // 0x38
			bool bValidStructuralParams; // 0x40
			unsigned char pad_41[0x7]; // 0x41
			SSalvageCargoParams* pCargoParams; // 0x48
			bool bValidCargoParams; // 0x50
			unsigned char pad_51[0x7]; // 0x51
			unsigned char pad_58[0x10]; // 0x58 | Name: tractorParams | Size: 0x10 | Type: 0x110
			float	sensorRaycastArmingDistance; // 0x68
			__int32	numSupportedSalvageHeads; // 0x6c
			unsigned char pad_70[0x58]; // 0x70 | Name: salvageAudioParams | Size: 0x58 | Type: 0x10
			bool	useControllerToInitalizeControlComponent; // 0xc8
			bool	usesCargoGrid; // 0xc9
			bool	autoEjectRequireManualStart; // 0xca
		}; // Size: 0xcb

		struct SFloatModifier
		{
			char pad_0000[16];	//0x0000
			float level;	//0x0010
			char pad_0014[12];	//0x0014	//	next float modifier
		};	//Size: 0x0020

		struct STypeModifier
		{
			SFloatModifier* pAccessor;	//0x0000
			bool bValid;	//0x0008
			char pad_0009[7];	//0x0009
		};	//Size: 0x0010

		struct SHighliteColor
		{
			float r, g, b, a;//0x0000
		};	//Size: 0x0010

		struct SMiningControllerGlobalParams
		{
			char pad_0000[16];	//0x0000
			struct SHighliteColor highlightColor;	//0x0010
			char pad_0020[72];	//0x0020
			float highlightOccludedAlpha;	//0x0068
			float highlightOutlineWidth;	//0x006C
			float highlightDistantMineablesRange;	//0x0070
			char pad_0074[100];	//0x0074
			bool showChildRockRadarIcon;	//0x00D8
			bool scalePowerGraphMin;	//0x00D9
			char pad_00DA[2];	//0x00DA
			float noProgressHintTime;	//0x00DC
			float noProgressHintPower;	//0x00E0
			float fractureDoneFeedbackDuration;	//0x00E4
			float maxScanRaycastDistance;	//0x00E8
		};	//Size: 0x00EC

		struct SCItemMiningControllerParams
		{
			char pad_0000[80];	//0x0000
			struct SMiningControllerGlobalParams* pMiningControllerGlobalParams;	//0x0050
			char pad_0058[48];	//0x0058
		};	//Size: 0x0088

		struct SMiningGlobalParams
		{
			char pad_0000[8];	//0x0000
			float powerCapacityPerMass;	//0x0008
			float decayPerMass;	//0x000C
			char pad_0010[8];	//0x0010
			float resistanceCurveFactor;	//0x0018
			char pad_001C[528];	//0x001C
			float cSCUPerVolume;	//0x022C
			float defaultMass;	//0x0230
			char pad_0234[596];	//0x0234
		};	//Size: 0x0488

		struct SMineableParams
		{
			char pad_0000[40];	//0x0000
			struct SMiningGlobalParams* pGlobalParams;	//0x0028
			char pad_0030[80];	//0x0030
		};	//Size: 0x0080

		struct MiningLaserGlobalParams
		{
			char pad_0000[8];	//0x0000
			bool blockThrottleChangeWhenNotFiring;	//0x0008
			bool throttleResetOnStopFire;	//0x0009
			char pad_000A[2];	//0x000A
			float throttleChangePerAction;	//0x000C
			float throttleAccPeriod;	//0x0010
			float throttleAccFactor;	//0x0014
			float throttleHoldAccFactor;	//0x0018
			char pad_001C[100];	//0x001C
		};	//Size: 0x0080

		struct MiningLaserModifiers
		{
			char pad_0000[8];	//0x0000
			STypeModifier laserInstability;	//0x0008
			STypeModifier optimalChargeWindowSizeModifier;	//0x0018
			STypeModifier resistanceModifier;	//0x0028
			STypeModifier shatterdamageModifier;	//0x0038
			STypeModifier clusterFactorModifier;	//0x0048
			STypeModifier optimalChargeRateModifier;	//0x0058
			bool isOptimalRateGood;	//0x0068
			char pad_0069[7];	//0x0069
			STypeModifier catastrophicChargeWindowRateMultiplier;	//0x0070
		};	//Size: 0x0080

		struct MiningFilterParams
		{
			char pad_0000[8];	//0x0000
			STypeModifier filterModifier;	//0x0008
		};	//Size: 0x0018

		struct SEntityComponentMiningLaserParams
		{
			char pad_0000[40];	//0x0000
			MiningLaserGlobalParams* pMiningLaserGlobalParams;	//0x0028
			float throttleLerpSpeed;	//0x0030
			float throttleMinimum;	//0x0034
			MiningLaserModifiers miningLaserModifiers;	//0x0038
			MiningFilterParams filterParams;	//0x00B8
			bool usePowerThrottle;	//0x00D0
		};	//Size: 0x00D1

	}

	namespace Classes
	{
		using namespace Structs;

		class CSystem
		{
		public:
			char pad_0008[32];	//0x0008
			//	Matrix4x3 ModelMatrix;	//0x0028

		protected:
			virtual void Function0();
			virtual void Function1();
			virtual void Function2();
			virtual void Function3();
			virtual void CSystem_Update();

		};	//Size: 0x0088

		class CXConsole
		{
			char pad_0008[128];	//0x0008

		protected:
			virtual void Function0();

		};	//Size: 0x0088
    
        class CXCVar
        {
        public:	//	NATIVE OFFSETS      [ DO NOT DISRUPT ]
            char *								pName; 							//0x0008
            char *								pDescription; 					//0x0010
            __int64 							mflag; 							//0x0018
            char 								pad_0020[32]; 					//0x0020
            void *								pConsole; 						//0x0040
            void * 								pValue; 						//0x0048
    
        public:	//	VIRTUAL FUNCTIONS   [ DO NOT DISRUPT ]
            virtual void						vf_Function0();		
    
        }; //Size: 0x0050

		class CGoToPoint
		{
		public:
			char*								m_pPointName;		//0x0000
			char*								m_pEditorName;		//0x0008
			char								pad_0010[32];		//0x0010
			double								m_posX;				//0x0030
			char								pad_0038[24];		//0x0038
			double								m_posY;				//0x0050
			char								pad_0058[24];		//0x0058
			double								m_posZ;				//0x0070
			__int64								m_KeyIndex;			//0x0078
			char								pad_0080[32];		//0x0080
			bool								m_bValid;			//0x00A0
			char								pad_00A1[7];		//0x00A1

		};	//Size: 0x00A8

		class CGoToPointManager
		{
		public:
			char								pad_0000[80];		//0x0000
			struct SGoToPointAZ*				pGoToPoints;		//0x0058

		public:
			virtual void						vFunction0();
		};	//Size: 0x0060

		class CDataCore
		{
		public:

		protected:
			virtual void						vf_Function0();
		};	//	Size:	0x0000

		class CRenderer
		{
		public:
			char pad_0008[128];	//0x0008

		public:
			virtual void Function0();
			virtual void Function1();
			virtual void Function2();
			virtual void Function3();
			virtual void Function4();
			virtual void Function5();
			virtual void Function6();
			virtual void MT_Update();
			virtual void Function8();
			virtual void Function9();
			virtual void Function10();
			virtual void Function11();
			virtual void Function12();
			virtual void Function13();
			virtual void Function14();
			virtual void Function15();
			virtual void Function16();
			virtual void Function17();
			virtual void Function18();
			virtual void Function19();
			virtual void Function20();
			virtual void Function21();
			virtual void Function22();
			virtual void Function23();
			virtual void Function24();
			virtual void Function25();
			virtual void Function26();
			virtual void Function27();
			virtual void Function28();
			virtual void Function29();
			virtual void Function30();
			virtual void Function31();
			virtual void Function32();
			virtual void Function33();
			virtual void Function34();
			virtual void Function35();
			virtual void Function36();
			virtual void Function37();
			virtual void Function38();
			virtual void Function39();
			virtual void Function40();
			virtual void Function41();
			virtual void Function42();
			virtual void Function43();
			virtual void Function44();
			virtual void Function45();
			virtual void Function46();
			virtual void Function47();
			virtual void Function48();
			virtual void Function49();
			virtual void Function50();
			virtual void Function51();
			virtual void Function52();
			virtual void Function53();
			virtual void Function54();
			virtual void Function55();
			virtual void Function56();
			virtual void Function57();
			virtual void Function58();
			virtual void Function59();
			virtual void Function60();
			virtual void Function61();
			virtual void Function62();
			virtual void Function63();
			virtual void Function64();
			virtual void Function65();
			virtual bool ProjectToScreen(double x, double y, double z, float* outX, float* outY, float* outZ, int unk = 1, __int64 unk3 = 0);
		};	//Size: 0x0088

		class CGame
		{
		public:		
			char pad_0008[216];	//0x0008
			class CUnknownData* pUnknownPointer;	//0x00E0
			char pad_00E8[2848];	//0x00E8
			class CSCPlayer* pLocalPlayer;	//0x0C08


		protected:
			virtual void Function0();
			virtual void Function1();
			virtual void Function2();
			virtual void Function3();
			virtual void Function4();
			virtual void Function5();
			virtual void Function6();
			virtual void Function7();
			virtual void Function8();
			virtual void QuitGame();
			virtual void Function10();
			virtual void Function11();
			virtual void GetLocalPlayer();
			virtual void Function13();
			virtual void Function14();
			virtual void Function15();
			virtual void Function16();
			virtual void Function17();
			virtual void Function18();
			virtual void Function19();
			virtual void GetUnknownPointer();	//	return __int64*(this + 0xE0)
		};	//Size: 0x0C08

		class CCamera
		{
		public:
			DVector								WorldPosition;
			FQuat								ViewAngles;

		public:
			void								GetDirections(float* forward, float* right, float* up);
			void								GetForwardDir(float* out);
		};

		class CZone
		{
		public:
			char								pad_0008[16];		//0x0008
			char*								pName;				//0x0018
			DQuat								mAngles;			//0x0020
			DVector								mOrigin;			//0x0040
			char								pad_0058[312];		//0x0058
			class CEntity*						pEntity;			//0x0190
			char								pad_0198[40];		//0x0198
			class CZone*						pParentZone;		//0x01C0

		protected:
			virtual void Function0();

		};	//Size: 0x01C8

		class CRenderProxy
		{
		public:
			class CEntity* pEntity;	//0x0008
			char pad_0010[248];	//0x0010
			AABB mBounds;	//0x0108
			char pad_0120[40];	//0x0120
			//	SContactHighliteShaderParams* ShaderHighliteParams;	//0x????
			bool bGlow;	//0x0148
			bool bVisible;	//0x0149

		protected:
			virtual void Function0();

		};	//Size: 0x014A

		class CEntitySystem
		{
		public:	
			char pad_0008[216]; //0x0008
			int64_t szEntities; //0x00E0
			char pad_00E8[48]; //0x00E8
			VArray EntityArray; //0x0118


		public:
			virtual void vf_Function0();
			virtual void vf_Function1();
			virtual void vf_Function2();
			virtual void vf_Function3();
			virtual void vf_Function4();
			virtual void vf_Function5();
			virtual void vf_Function6();
			virtual void vf_Function7();
			virtual void vf_Function8();
			virtual void vf_Function9();
			virtual void vf_Function10();
			virtual void vf_Update();
			virtual void vf_Function12();
			virtual void vf_Function13();
			virtual void vf_Function14();
			virtual void vf_Function15();
			virtual void vf_Function16();
			virtual void vf_Function17();
			virtual void vf_Function18();
			virtual void vf_Function19();
			virtual void vf_Function20();
			virtual void vf_Function21();
			virtual void vf_Function22();
			virtual void vf_Function23();
			virtual class CEntityClassRegistry* vf_GetEntityClassRegistry();
			virtual void vf_Function25();
			virtual void vf_Function26();
			virtual void vf_Function27();
			virtual void vf_Function28();
			virtual void vf_Function29();
			virtual void vf_Function30();
			virtual void vf_Function31();
			virtual void vf_Function32();
			virtual void vf_Function33();
			virtual class IEntityIt* vf_GetEntityIterator(__int64*);


		};	//Size: 0x0138

		class IEntityIt
		{
		public:
			class CEntitySystem* pEntitySystem;	//0x0008
			char pad_0010[16];	//0x0010
			class IObject* pObject;	//0x0020
			char pad_0028[88];	//0x0028

		public:
			virtual void vf_Function0();
			virtual void vf_Add();					//	adds an entity to the iterator
			virtual void vf_Remove();				//	releases this iterator instance and any allocated memory
			virtual bool vf_IsEnd();				//	returns true if iterator is at end
			virtual class CEntity* vf_NextEnt();	//	returns next entity
			virtual class CEntity* vf_ThisEnt();	//	returns current entity
			virtual void vf_First();				//	positions iterator at first entity

		public:

		};	//Size: 0x0080

		class CEntityClassRegistry
		{
		public:

		protected:

		public:
			virtual void Function0();
			virtual void Function1();
			virtual void Function2();
			virtual void Function3();
			virtual class CEntityClass* FindClass(const char* pClassName);
		};

		class CEntityClass
		{
		public:
			char								pad_0008[8];		//0x0008
			char*								pClassName;			//0x0010
			char*								pScriptName;		//0x0018
			int									mID;				//0x0020

		protected:	
			virtual void Function0();
		    virtual void Function1();
		    virtual void Function2();
		    virtual void GetClassName();
		    virtual void GetScriptName();

		};	//Size: 0x0018

		class CEntity
		{
		public:
			int mID;	//0x0008
			int mGUID;	//0x000C
			__int64 mFlag;	//0x0010
			char pad_0018[8];	//0x0018
			class CEntityClass* pEntityClass;	//0x0020
			DQuat mLocalAngles;	//0x0028
			DVector mLocalOrigin;	//0x0048
			char pad_0060[272];	//0x0060
			unsigned int mSeatFlag;	//0x0170
			char pad_0174[196];	//0x0174
			class IEntityComponents* pComponents;	//0x0238
			char pad_0240[80];	//0x0240
			char* pName;	//0x0290
			char pad_0298[16];	//0x0298
			class CZone* pLocalZone;	//0x02A8
			char pad_02B0[96];	//0x02B0
			class CEntity* pDefaultEntity;	//0x0310
			class CEntity* pParentEntity;	//0x0318
			class CEntity* pSelfEntity;	//0x0320
			char pad_0328[440];	//0x0328
			class CEntity* pSeatEntity;	//0x04E0
			class CEntity* pLocalZoneEntity;	//0x04E8
			char pad_04F0[760]; //0x04F0
			char* pGenderText; //0x07E8
			char pad_07F0[2024]; //0x07F0
			class CActorEntity* pActorEntity; //0x0FD8

		public:
			virtual void vf_Function0();
			virtual void vf_Function1();
			virtual void vf_Function2();
			virtual void vf_Function3();
			virtual void vf_Function4();
			virtual void vf_Function5();
			virtual void vf_Function6();
			virtual void vf_Function7();
			virtual void vf_Function8();
			virtual void vf_Function9();
			virtual void vf_Function10();
			virtual void vf_Function11();
			virtual void vf_Function12();
			virtual void vf_Function13();
			virtual void vf_Function14();
			virtual void vf_Function15();
			virtual void vf_Function16();
			virtual void vf_Function17();
			virtual void vf_Function18();
			virtual void vf_Function19();
			virtual void vf_Function20();
			virtual void vf_Function21();
			virtual void vf_Function22();
			virtual void vf_Function23();
			virtual void vf_Function24();
			virtual void vf_Function25();
			virtual void vf_Function26();
			virtual void vf_Function27();
			virtual void vf_Function28();
			virtual void vf_Function29();
			virtual void vf_Function30();
			virtual void vf_Function31();
			virtual void vf_Function32();
			virtual void vf_Function33();
			virtual void vf_Function34();
			virtual void vf_Function35();
			virtual void vf_Function36();
			virtual void vf_Function37();
			virtual void vf_Function38();
			virtual void vf_Function39();
			virtual void vf_Function40();
			virtual void vf_Function41();
			virtual void vf_Function42();
			virtual void vf_Function43();
			virtual void vf_Function44();
			virtual void vf_Function45();
			virtual void vf_Function46();
			virtual void vf_Function47();
			virtual void vf_Function48();
			virtual void vf_Function49();
			virtual void vf_Function50();
			virtual void vf_Function51();
			virtual void vf_Function52();
			virtual void vf_Function53();
			virtual void vf_Function54();
			virtual void vf_Function55();
			virtual void vf_Function56();
			virtual void vf_Function57();
			virtual void vf_Function58();
			virtual void vf_Function59();
			virtual void vf_Function60();
			virtual void vf_Function61();
			virtual void vf_Function62();
			virtual void vf_Function63();
			virtual void vf_Function64();
			virtual void vf_Function65();
			virtual void vf_Function66();
			virtual void vf_Function67();
			virtual void vf_Function68();
			virtual void vf_Function69();
			virtual void vf_Function70();
			virtual void vf_Function71();
			virtual void vf_Function72();
			virtual void vf_Function73();
			virtual void vf_Function74();
			virtual void vf_Function75();
			virtual void vf_Function76();
			virtual void vf_Function77();
			virtual void vf_Function78();
			virtual void vf_Function79();
			virtual void vf_Function80();
			virtual void vf_Function81();
			virtual void vf_Function82();
			virtual void vf_Function83();
			virtual void vf_Function84();
			virtual void vf_Function85();
			virtual void vf_Function86();
			virtual void vf_Function87();
			virtual __int64 vf_SetWorldPos(DVector* pos, int a3 = 0, int a4 = 0);
			virtual __int64 vf_GetWorldPos(DVector* out, int a3 = 0);
			virtual __int64 vf_GetEularAngles(FVector* a2, int a3 = 1);
			virtual __int64 vf_SetWorldRotation(DQuat* rot, int a3 = 0, int a4 = 0);
			virtual __int64 vf_GetWorldRotation(DQuat* out, int a3 = 1);
			virtual __int64 vf_GetWorldForwardDir(FVector* out, int a3 = 0);
		};	//Size: 0x0FD0

		class CPhysicalEntity
		{
		public:
			char pad_0008[200]; //0x0008
			char* pName; //0x00D0
			char pad_00E0[224];	//0x00E0
			DVector mLocalLocation;	//0x01C0
			FQuat mLocalAngles;	//0x01D8


		protected:
			virtual void Function0();

		};	//Size: 0x01E8

		class CActorEntity
		{
		public:	
			char pad_0008[184];	//0x0008
			class CEntity* pEntity;	//0x00C0
			char pad_00C8[8];	//0x00C8
			char* pName;	//0x00D0
			char pad_00D8[232];	//0x00D8
			DVector mLocalOrigin;	//0x01C0
			FQuat mLocalAngles;	//0x01D8
			char pad_01E8[1930];	//0x01E8
			unsigned char mCollision;	//0x0972
			bool bFloat;	//0x0973
			char pad_0974[132]; //0x0974
			class CPhysicalEntity* pGroundObject; //0x09F8			
			//	char pad_0A00[112];	//0x0A00
			//	class CRenderProxy* pRenderProxy;	//0x0A70
			//	char pad_0A78[200];	//0x0A78
			//	class CSCActorComponent* pActorComponent;	//0x0B40
			//	char pad_0B48[1048];	//0x0B48
			//	float mHealth;	//0x0E88
			//	float mMaxHealth;	//0x0E8C
			//	char pad_0E90[128];	//0x0E90
			//	float mMass;	//0x0F10
			//	char pad_0F14[120];	//0x0F14
			//	float mSpeed;	//0x0F90
			//	char pad_0F94[156];	//0x0F94

		protected:
			virtual void Function0();

		};	//Size: 0x0490

		class CSCActor
		{
		public:
			class CEntity* pEntity;	//0x0008

		protected:
			virtual void Function0();

		};	//Size: 0x0010

		class CSCPlayer
		{
		public:	
			class CEntity* pEntity;	//0x0008
			char pad_0010[936];	//0x0010
			char* pName;	//0x03B8
			char pad_03C0[48];	//0x03C0
			class CSCActor* pActor;	//0x03F0

		protected:
			virtual void Function0();

		};	//Size: 0x03F8

		class CSCActorComponent
		{
		public:
			class CEntity* pEntity;	//0x0008
			char pad_0010[1328];	//0x0010
			class CSCBodyHealthComponent* pHealthComponent;	//0x0540

		protected:
			virtual void Function0();

		};	//Size: 0x0548

		class CSCBodyHealthComponent
		{
		public:
			class CEntity* pEntity;	//0x0008
			char pad_0010[608];	//0x0010
			class CSCActorComponent* pActor;	//0x0270

		protected:
			virtual void Function0();

		};	//Size: 0x0278

		class CSCAmmoContainerComponent
		{
		public:
			class CEntity* pEntity;	//0x0008
			char pad_0010[224];	//0x0010
			int mAmmoMax;	//0x00F0
			int mAmmoCurrent;	//0x00F4
			class CSCAmmoContainerInfo* pAmmoInfo;	//0x00F8
			char pad_0100[264];	//0x0100
			class CSCWeaponAmmoParams* pWeaponAmmoParams;	//0x0208

		protected:
			virtual void Function0();
		};	//Size: 0x0210

		class CSCAmmoContainerInfo
		{
		public:
			char								pad_0008[8];			//0x0008
			char*								pAmmoTypeName;			//0x0010
			char*								pAmmoParamsScript;		//0x0018

		protected:
			virtual void Function0();
		};	//Size: 0x0020

		class CSCWeaponAmmoParams
		{
		public:
			class CSCItemWeaponComponent*		pWeapon;				//0x0008

		protected:
			virtual void Function0();
		};	//Size: 0x0010

		class CSCItemWeaponComponent
		{
		public:
			class CEntity* pEntity;	//0x0008
			char pad_0010[8136];	//0x0010
			class CSCAmmoContainerComponent* pAmmoContainer;	//0x1FD8

		protected:
			virtual void Function0();
		};	//Size: 0x1FE0

		template <typename T>
		class CWeaponAction
		{
		public:
			char								pad_0000[8];			//0x0008
			T*									pWeaponActionParams;	//0x0010
			class CSCItemWeaponComponent*		pWeapon;				//0x0018

		public:
			virtual void						vf_Function0();

		public:
			//	T*									GetWeaponParams() { return reinterpret_cast<T*>(EXTRACT_LOWER_BYTES((__int64)pWeaponActionParams)); }
			//	class CSCItemWeaponComponent*		GetWeaponComponent() { return reinterpret_cast<CSCItemWeaponComponent*>(EXTRACT_LOWER_BYTES((__int64)pWeapon)); }
		};	//Size: 0x0020

		class CSCItemQuantumDrive
		{
		public:
			class CEntity* pEntity;	//0x0008
			char pad_0010[1536];	//0x0010
			DVector targetOrigin;	//0x0610
			class CEntity* pTargetEntity;	//0x0628
			char pad_0630[16];	//0x0630
			char* pGoToName;	//0x0640
			wchar_t* pTargetNameW;	//0x0648
			char pad_0650[48];	//0x0650
			struct STravelEnvelope* pTravelEnvelope;	//0x0680
			char pad_0688[1992];	//0x0688
			EQuantumSpoolState mSpoolState;	//0x0E50
			char pad_0E54[180];	//0x0E54
			float mCalibrationMax;	//0x0F08
			char pad_0F0C[68];	//0x0F0C
			float mSpoolChargeLevel;	//0x0F50
			char pad_0F54[28];	//0x0F54
			float mCalibrationLevel;	//0x0F70
			char pad_0F74[28];	//0x0F74
			EQuantumJumpState mJumpState;	//0x0F90
			char pad_0F94[604];	//0x0F94
			wchar_t* pNameW_2;	//0x11F0

		public:
			virtual void                            Function0();
		};	//Size: 0x1220

		class CEntityComponentMineable
		{
		public:
			char pad_0000[8];	//0x0000
			class CEntity* pHitEntity;	//0x0008
			char pad_0010[48];	//0x0010
			Structs::SMineableParams* pMineableParams;	//0x0040
		};	//Size: 0x0480

		///	some other mining controller
		//	class CEntityComponentMineable
		//	{
		//	public:
		//		char pad_0000[8];	//0x0000
		//		CEntity* pEntity;	//0x0008
		//		char pad_0010[48];	//0x0010
		//		Structs::SCItemMiningControllerParams* pMiningControllerParams;	//0x0040
		//		char pad_0048[688];	//0x0048
		//		class CEntityComponentMineable* pMineableObject;	//0x02F8
		//	};	//Size: 0x0300

		class CSCItemMiningController
		{
		public:
			char pad_0000[8];	//0x0000
			__int64 pEntity;	//0x0008
			char pad_0010[48];	//0x0010
			Structs::SEntityComponentMiningLaserParams* pMiningLaserParams;	//0x0040
			char pad_0048[40];	//0x0048
		};	//Size: 0x0070

		class CSCItemSalvageController
		{
		public:
			char pad_0000[8];	//0x0000
			class CEntity* pEntity;	//0x0008
			char pad_0010[48];	//0x0010
			Structs::SCItemSalvageControllerParams* pControllerParams;	//0x0040
		};	//Size: 0x0048

		struct IEntityComponents
		{
			char								pad_0000[16];		//0x0000
			class CRenderProxy*					pRenderProxy;		//0x0010
			char								pad_0018[64];		//0x0018
			class CSCActorComponent*			pActorComponent;	//0x0058
			char								pad_0060[104];		//0x0060
			class CSCBodyHealthComponent*		pHealthComponent;	//0x00C8
		};	//Size: 0x00D0

		struct SSystemGlobalEnvironment
		{
			char								pad_0000[120];		//0x0000
			class CDataCore* 				    pDataCore;			//0x0078
			char								pad_0080[24];		//0x0080
			class CGame*						pGame;				//0x0098
			class CEntitySystem*				pEntitySystem;		//0x00A0
			char								pad_00A8[8];		//0x00A8
			class CXConsole*					pConsole;			//0x00B0
			char								pad_00B8[8];		//0x00B8
			class CSystem*						pSystem;			//0x00C0
			char								pad_00C8[48];		//0x00C8
			class CRenderer*					pRenderer;			//0x00F8
			char								pad_0100[128];		//0x0100
		};	//Size: 0x0180

	}
}

template <typename T>
inline T CleanPointer(T addr) { return reinterpret_cast<T>((unsigned long long)addr & 0xFFFFFFFFFFFF); }

inline int GetVfIndex(unsigned int offset) { return offset / 8; }

template<typename fnc>
inline fnc GetVFunction(const void* czInstance, size_t vfIndex)
{
	auto vfTbl = *static_cast<const void***>(const_cast<void*>(czInstance));
	return reinterpret_cast<fnc>(const_cast<void(*)>(vfTbl[vfIndex]));
}

template<typename retType, typename... Args>
inline retType CallVFunction(const void* czInstance, size_t vfIndex, Args&&... args)
{
	const auto fn = GetVFunction<retType(*)(const void*, Args...)>(czInstance, vfIndex);
	return fn(czInstance, std::forward<Args>(args)...);
}