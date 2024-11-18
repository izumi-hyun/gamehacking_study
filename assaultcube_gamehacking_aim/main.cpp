#include <Windows.h>
#include <iostream>
#include <cmath>
#define M_PI 3.14159265358979323846
typedef struct Vector2 {
    float x, y;

    // Default constructor
    Vector2() : x(0.0f), y(0.0f) {}

    // Parameterized constructor
    Vector2(float x, float y) : x(x), y(y) {}

    // Overloaded addition operator
    Vector2 operator+ (const Vector2& a) const {
        return Vector2(x + a.x, y + a.y);
    }

    // Overloaded subtraction operator
    Vector2 operator- (const Vector2& a) const {
        return Vector2(x - a.x, y - a.y);
    }

    // Overloaded multiplication operator
    Vector2 operator* (const Vector2& a) const {
        return Vector2(x * a.x, y * a.y);
    }

    // Overloaded division operator
    Vector2 operator/ (const Vector2& a) const {
        // Prevent division by zero
        return Vector2(a.x != 0 ? x / a.x : 0, a.y != 0 ? y / a.y : 0);
    }

    // Calculate the Euclidean norm (magnitude) of the vector
    float sqrt_3d() const {
        return sqrt(x * x + y * y);
    }
} Vector2;

// 3D 벡터 구조체
struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3() {}

    // 벡터 연산자 오버로딩
    Vec3 operator+(const Vec3& a) const {
        return Vec3(x + a.x, y + a.y, z + a.z);
    }

    Vec3 operator-(const Vec3& a) const {
        return Vec3(x - a.x, y - a.y, z - a.z);
    }

    Vec3 operator/(const Vec3& a) const {
        return Vec3(x / a.x, y / a.y, z / a.z);
    }

    Vec3 operator*(const Vec3& a) const {
        return Vec3(x * a.x, y * a.y, z * a.z);
    }
    float dot(const Vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }
    // 3D 거리 계산
    float sqrt_3d() const {
        //return sqrt(x * x + y * y + z * z);
        return dot(*this);
    }
};

// 플레이어 엔티티 구조체
struct Entity {
    char pad_0000[0x4];   // 00 - 04
    Vec3 headPos;         // 04 - 0C
    char pad_000C[0x24];  // 0C - 34
    Vector2 viewAngle;       // 34 - 3C
    char pad_0038[0xB0];  // 3C - EC7
    int health;           // EC - F0
    int armor;            // F0 - F4
    char pad_00F4[0x4c]; // F4 - 205
    int bullet;
    char pad_00F5[0xa3];
    char name[16];        // 205 - 221
    char pad_0219[0xF7];  // 221 - 30C
    int team;             // 30C - 310
};

// DLL 주 입구
BOOL WINAPI OnLoad(HMODULE hmod) {
    // 콘솔 할당 및 출력 리디렉션
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    std::cout << "Injected" << std::endl;

    // 게임 모듈의 베이스 주소를 한 번만 가져와 변수에 저장
    uintptr_t baseAddress = reinterpret_cast<uintptr_t>(GetModuleHandle(L"ac_client.exe"));
    Entity* localplayer = *(Entity**)(baseAddress + 0x17E0A8);
    uintptr_t entitylist = *(uintptr_t*)(baseAddress + 0x191FCC);
    int playerCount = *(int*)(baseAddress + 0x18AC0C);
    uintptr_t targetAddress = baseAddress + 0xC73EA;
    uintptr_t targetAddress2 = baseAddress + 0xC2EC3;
    // 플레이어 정보 출력
    std::cout << "Health: " << localplayer->health << std::endl;
    std::cout << "Armor:  " << localplayer->armor << std::endl;
    std::cout << "Name:   " << localplayer->name << std::endl;
    DWORD oldProtect, oldProtect2;
    if (VirtualProtect(reinterpret_cast<LPVOID>(targetAddress), 2, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        // 해당 주소에 NOP 명령어를 두 번 작성합니다 (0x90은 NOP의 opcode).
        *(BYTE*)targetAddress = 0x90;      // 첫 번째 NOP
        *(BYTE*)(targetAddress + 1) = 0x90; // 두 번째 NOP

        // 메모리 보호를 원래대로 되돌립니다.
        VirtualProtect(reinterpret_cast<LPVOID>(targetAddress), 2, oldProtect, &oldProtect);
    }

    if (VirtualProtect(reinterpret_cast<LPVOID>(targetAddress2), 5, PAGE_EXECUTE_READWRITE, &oldProtect2)) {
        // movss [esi+38], xmm2 명령어를 NOP 처리
        *(BYTE*)targetAddress2 = 0x90;      // NOP
        *(BYTE*)(targetAddress2 + 1) = 0x90; // NOP
        *(BYTE*)(targetAddress2 + 2) = 0x90; // NOP
        *(BYTE*)(targetAddress2 + 3) = 0x90; // NOP
        *(BYTE*)(targetAddress2 + 4) = 0x90; // NOP

        // 메모리 보호를 원래대로 되돌립니다.
        VirtualProtect(reinterpret_cast<LPVOID>(targetAddress2), 5, oldProtect2, &oldProtect2);
    }
    else {
        // VirtualProtect 호출 실패 처리
    }
    // 메인 루프
    while (true) {
        localplayer->armor = 1000;
        localplayer->bullet = 1000;

        playerCount = *(int*)(baseAddress + 0x18AC0C);
        entitylist = *(uintptr_t*)(baseAddress + 0x191FCC);
        if (entitylist) {
            Entity* nearestEnemy = nullptr;
            float closestDist = 1000.0f;

            // 모든 엔티티 확인
            for (int i = 0; i < playerCount-1; i++) {
                // 엔티티 포인터 리스트에서 각 엔티티의 주소를 가져오기
                Entity* entity = *(Entity**)(entitylist + 0x4 * i);

                if (!entity) continue;

                float distance = (localplayer->headPos - entity->headPos).sqrt_3d();
                if (entity->health > 0 && localplayer->team != entity->team && distance < closestDist) {
                    closestDist = distance;
                    nearestEnemy = entity;
                }
            }

            // 가장 가까운 적이 있는 경우
            if (nearestEnemy) {
                Vec3 delta = nearestEnemy->headPos - localplayer->headPos;
                float yaw = atan2f(delta.y, delta.x) * 180.0f / M_PI;
                float hyp = sqrt(delta.x * delta.x + delta.y * delta.y);
                float pitch = atan2f(delta.z, hyp) * 180.0f / M_PI;

                std::cout << "ViewX: " << yaw << std::endl;
                std::cout << "ViewY: " << pitch << std::endl;

                if (GetAsyncKeyState(VK_LBUTTON)) {
                    localplayer->viewAngle.x = yaw + 90.0f; // Assault Cube mechanics
                    localplayer->viewAngle.y = pitch;
                }
            }
        }
        Sleep(5); // CPU 사용량을 줄이기 위해 잠시 대기
    }
    return TRUE;
}
// DLL 메인 함수
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        CloseHandle(CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)OnLoad, hModule, NULL, NULL));
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
