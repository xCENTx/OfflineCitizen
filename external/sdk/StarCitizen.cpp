#pragma once
#include <pch.h>
#include "StarCitizen.h"

namespace StarCitizen
{
	namespace Structs
	{

        FVector FVector::operator+(const FVector& v) const { return FVector(x + v.x, y + v.y, z + v.z); }
        FVector FVector::operator-(const FVector& v) const { return FVector(x - v.x, y - v.y, z - v.z); }
        FVector FVector::operator*(const FVector& v) const { return FVector(x * v.x, y * v.y, z * v.z); }
        FVector FVector::operator*(const float scalar) const { return FVector(x * scalar, y * scalar, z * scalar); }
        FVector& FVector::operator+=(const FVector& other)
        {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }

        FVector& FVector::operator-=(const FVector& other)
        {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
        }

        FVector& FVector::operator+=(const float* other)
        {
            x += other[0];
            y += other[1];
            z += other[2];
            return *this;
        }

        FVector& FVector::operator-=(const float* other)
        {
            x -= other[0];
            y -= other[1];
            z -= other[2];
            return *this;
        }
		float FVector::Dot(const FVector& v) const { return x * v.x + y * v.y + z * v.z; }
		float FVector::Length() const { return sqrtf(x * x + y * y + z * z); }
		float FVector::Distance(const FVector& v) const { return (*this - v).Length(); }

        FVector FVector::Normalize() {
            float magnitude = std::sqrt(x * x + y * y + z * z);
            FVector normalized;
            normalized.x = x / magnitude;
            normalized.y = y / magnitude;
            normalized.z = z / magnitude;
            return normalized;
        }

        FVector FVector::Cross(const FVector& other)
        {
            float crossX = y * other.z - z * other.y;
            float crossY = z * other.x - x * other.z;
            float crossZ = x * other.y - y * other.x;

            return FVector(crossX, crossY, crossZ);
        }

        bool FVector::IsValid()
        {
            return x == 0.0 && y == 0.0;
        }

        FVector FVector::RotatePointAroundX(float angleRadians)
        {
            float cosAngle = cos(angleRadians);
            float sinAngle = sin(angleRadians);

            StarCitizen::Structs::FVector rotatedPoint;
            rotatedPoint.x = x;
            rotatedPoint.y = y * cosAngle - z * sinAngle;
            rotatedPoint.z = y * sinAngle + z * cosAngle;

            return rotatedPoint;
        }

        FVector FVector::RotatePointAroundY(float angleRadians)
        {
            float cosAngle = cos(angleRadians);
            float sinAngle = sin(angleRadians);

            StarCitizen::Structs::FVector rotatedPoint;
            rotatedPoint.x = x * cosAngle + z * sinAngle;
            rotatedPoint.y = y;  // Y remains the same
            rotatedPoint.z = -x * sinAngle + z * cosAngle;

            return rotatedPoint;
        }

        FVector FVector::RotatePointAroundZ(float angleRadians)
        {
            float cosAngle = cos(angleRadians);
            float sinAngle = sin(angleRadians);

            StarCitizen::Structs::FVector rotatedPoint;
            rotatedPoint.x = x * cosAngle - y * sinAngle;
            rotatedPoint.y = x * sinAngle + y * cosAngle;
            rotatedPoint.z = z;  // Z remains the same

            return rotatedPoint;
        }


        DVector DVector::operator+(const DVector& v) const { return DVector(x + v.x, y + v.y, z + v.z); }
		DVector DVector::operator+(float* v) const { return DVector(x + v[0], y + v[1], z + v[2]); }
        DVector DVector::operator-(const DVector& v) const { return DVector(x - v.x, y - v.y, z - v.z); }
        DVector& DVector::operator+=(const DVector& other)
        {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }

        DVector& DVector::operator-=(const DVector& other)
        {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
        }

        DVector& DVector::operator+=(const float* other)
        {
            x += other[0];
            y += other[1];
            z += other[2];
            return *this;
        }

        DVector& DVector::operator-=(const float* other)
        {
            x -= other[0];
            y -= other[1];
            z -= other[2];
            return *this;
        }
        DVector DVector::operator+(const FVector& other) const { return DVector{ x + other.x, y + other.y, z + other.z }; }
        DVector DVector::operator-(const FVector& other) const { return DVector{ x - other.x, y - other.y, z - other.z }; }

        double DVector::Dot(const DVector& v) const { return x * v.x + y * v.y + z * v.z; }
        double DVector::Length() const { return sqrtf(x * x + y * y + z * z); }
        double DVector::Distance(const DVector& v) const { return (*this - v).Length(); }

        DVector DVector::Normalize() {
            float magnitude = std::sqrt(x * x + y * y + z * z);
            DVector normalized;
            normalized.x = x / magnitude;
            normalized.y = y / magnitude;
            normalized.z = z / magnitude;
            return normalized;
        }

        DVector DVector::Cross(const DVector& other)
        {
            float crossX = y * other.z - z * other.y;
            float crossY = z * other.x - x * other.z;
            float crossZ = x * other.y - y * other.x;

            return DVector(crossX, crossY, crossZ);
        }

        bool DVector::IsValid()
        {
            return x == 0.0 && y == 0.0;
        }


        FVector Matrix34::GetColumn0() const { return FVector(m[0][0], m[1][0], m[2][0]); }
        FVector Matrix34::GetColumn1() const { return FVector(m[0][1], m[1][1], m[2][1]); }
        FVector Matrix34::GetColumn2() const { return FVector(m[0][2], m[1][2], m[2][2]); }
        FVector Matrix34::GetColumn3() const { return FVector(m[0][3], m[1][3], m[2][3]); }


        AABB::AABB()
        {
            mMin = FVector();
            mMax = FVector();
        }

        AABB::AABB(FVector mmin, FVector mmax)
        {
            mMin = mmin;
            mMax = mmax;
        }

        float AABB::GetWidth() { return mMax.x - mMin.x; }

        float AABB::GetHeight() { return mMax.y - mMin.y; }

        float AABB::GetDepth() { return mMax.z - mMin.z; }

        FVector AABB::GetExtents() { return mMax - mMin; }

        FVector AABB::GetCenter()
        {
            FVector delta = { mMin - mMax };
            return 0.5f * (mMin + delta);
        }

        FQuat FQuat::operator+(const FQuat& other) const { return FQuat{ x + other.x, y + other.y, z + other.z, w + other.w }; }

        FQuat FQuat::operator-(const FQuat& other) const { return FQuat{ x - other.x, y - other.y, z - other.z, w - other.w }; }

        FQuat FQuat::operator*(float scalar) const { return FQuat{ x * scalar, y * scalar, z * scalar, w * scalar }; }

        FQuat FQuat::operator/(float scalar) const { return FQuat{ x / scalar, y / scalar, z / scalar, w / scalar }; }

        FQuat FQuat::operator=(float* other) const { return FQuat{ other[0], other[1], other[2], other[3] }; }

        FQuat& FQuat::operator*=(const FQuat& other)
        {
            x += other.x;
            y += other.y;
            z += other.z;
            w += other.w;
            return *this;
        }

        FQuat FQuat::operator* (const FQuat& other) const
        {
            return {
                w * other.x + x * other.w + y * other.z - z * other.y,
                w * other.y - x * other.z + y * other.w + z * other.x,
                w * other.z + x * other.y - y * other.x + z * other.w,
                w * other.w - x * other.x - y * other.y - z * other.z
            };
        }

        FQuat FQuat::GetConjugate() const { return { -x, -y, -z, w }; }



        DBox::DBox()
        {
            m_min = DVector();
            m_max = DVector();
        }

        DBox::DBox(DVector mmin, DVector mmax)
        {
            m_min = mmin;
            m_max = mmax;
        }

        double DBox::GetWidth() { return m_max.x - m_min.x; }

        double DBox::GetHeight() { return m_max.y - m_min.y; }

        double DBox::GetDepth() { return m_max.z - m_min.z; }

        DVector DBox::GetExtents() { return m_max - m_min; }

        DVector DBox::GetCenter()
        {
            DVector delta = { m_min - m_max };
            return 0.5f * (m_min + delta);
        }

        bool DBox::IsColliding(const DBox& other)
        {
            if (this->m_max.x < other.m_min.x || this->m_min.x > other.m_max.x)
                return false;

            if (this->m_max.y < other.m_min.y || this->m_min.y > other.m_max.y)
                return false;

            if (this->m_max.z < other.m_min.z || this->m_min.z > other.m_max.z)
                return false;

            return true;
        }

        char* SXCommand::g_CmdArgs[100];
        int SXCommand::g_CmdArgCount = 0;

		void SXCommand::nullfn(){ }

        int SXCommand::GetArgCount()
        {
            return g_CmdArgCount;
        }
        
        __int64 SXCommand::GetArg(int index)
        {
            if (index < 0 || index >= g_CmdArgCount)
                return 0;
        
            return reinterpret_cast<__int64>(g_CmdArgs[index]);
        }

        __int64 SXCommand::nullfn2()
        {
            return 0;
        }

        __int64 SXCommand::nullfn3(__int64* out)
        {
            static __int64 v3 = 0;
        
            *out = v3;
            
            return reinterpret_cast<__int64>(&v3);
        }

        //  NOTE: some commands can be executed from gui but others require TLS context. 
        // all commands should be sent via game thread 
        void SXCommand::ExecuteCmd(int argc, char* argv[])
        {
			if (!fnPtr)
				return;

			g_CmdArgCount = argc;
			for (int i = 0; i < argc; i++)
			    g_CmdArgs[i] = argv[i];

            reinterpret_cast<void(__fastcall*)(void*)>(fnPtr)(this);

            /* clear */
			memset(g_CmdArgs, 0, sizeof(g_CmdArgs));
			g_CmdArgCount = 0;
        }

        /*
        
        

            //  static int __fastcall GetArgCount(void* pThis)
            //  {
            //      return 2;
            //  }
            //  
            //  static __int64 __fastcall GetArg(void* pThis, int index)
            //  {
            //      //  if (index < 0 || index >= 2)
            //      //      return 0;
            //      //  return reinterpret_cast<__int64>(vars::g_CommandArgs[index]);
            //  
            //      return 0;
            //  }
            //  static void* g_FakeCmdArgsVTable[] = {
            //      nullptr,
            //      reinterpret_cast<void*>(&GetArgCount),
            //      reinterpret_cast<void*>(&GetArg)
            //  };
            //  struct FakeConsoleArgs
            //  {
            //      void** vftable;
            //  };
            //  FakeConsoleArgs args;
            //  args.vftable = g_FakeCmdArgsVTable;
        
        
        */
	}

	namespace Classes
	{
        void CCamera::GetDirections(float* forward, float* right, float* up)
        {
            float x = ViewAngles.x;
            float y = ViewAngles.y;
            float z = ViewAngles.z;
            float w = ViewAngles.w;
            if (forward)
            {
                forward[0] = 2 * (x * y - w * z);
                forward[1] = 1 - 2 * (x * x + z * z);
                forward[2] = 2 * (y * z + w * x);
            }
            if (right)
            {
                right[0] = 1 - 2 * (y * y + z * z);
                right[1] = 2 * (x * y + w * z);
                right[2] = 2 * (x * z - w * y);
            }
            if (up)
            {
                up[0] = 2 * (x * z + w * y);
                up[1] = 2 * (y * z - w * x);
                up[2] = 1 - 2 * (x * x + y * y);
            }
        }

        void CCamera::GetForwardDir(float* out)
        {
            float x = ViewAngles.x;
            float y = ViewAngles.y;
            float z = ViewAngles.z;
            float w = ViewAngles.w;
            out[0] = 2 * (x * y - w * z);
            out[1] = 1 - 2 * (x * x + z * z);
            out[2] = 2 * (y * z + w * x);
        }
	}
}
