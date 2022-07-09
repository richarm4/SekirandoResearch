#include "GameHook.h"

DWORD64 qItemEquipComms = 0;

DWORD64 rItemRandomiser = 0;
DWORD64 rAutoEquip = 0;
DWORD64 rNoWeaponRequirements = 0;
DWORD64 rEquipLock = 0;

LPVOID itemGibDataCodeCave;

extern CItemRandomiser* ItemRandomiser;

BOOL CGameHook::initialize() {

	BOOL bReturn = true;

	//Inject ItemGibData
	itemGibDataCodeCave = InjectShellCode(nullptr, ItemGibDataShellcode, 17);

	//Modify ItemGibShellcode
	bReturn &= replaceShellCodeAddress(ItemGibShellcode, 15, itemGibDataCodeCave, 0, sizeof(void*));
	bReturn &= replaceShellCodeAddress(ItemGibShellcode, 26, itemGibDataCodeCave, 4, 4);
	bReturn &= replaceShellCodeAddress(ItemGibShellcode, 33, itemGibDataCodeCave, 8, 4);

	//Inject ItemGibShellcode
	LPVOID itemGibCodeCave = InjectShellCode((LPVOID)0x13ffe0000, ItemGibShellcode, 93);

	if (MH_Initialize() != MH_OK) return false;

	bReturn &= Hook(0x1407BBA80, (DWORD64)&tItemRandomiser, &rItemRandomiser, 5);
	if (dIsAutoEquip) { bReturn &= Hook(0x1407BBE92, (DWORD64)&tAutoEquip, &rAutoEquip, 6); }
	if (dIsNoWeaponRequirements) { bReturn &= Hook(0x140C073B9, (DWORD64)&tNoWeaponRequirements, &rNoWeaponRequirements, 7); }
	if (dLockEquipSlots) { LockEquipSlots(); }

	return bReturn;
}

BOOL CGameHook::updateRuntimeValues() {
	DWORD processId = GetCurrentProcessId();
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, processId);

	std::vector<unsigned int> hpOffsets = { 0x80, 0x1F90, 0x18, 0xD8 };
	uintptr_t healthPointAddr = FindExecutableAddress(0x4768E78, hpOffsets); //BaseB + HP Offsets

	std::vector<unsigned int> playTimeOffsets = { 0xA4 };
	uintptr_t playTimeAddr = FindExecutableAddress(0x4740178, playTimeOffsets); //BaseA + PlayTime Offsets	

	ReadProcessMemory(hProcess, (BYTE*)healthPointAddr, &healthPoint, sizeof(healthPoint), &healthPointRead);
	ReadProcessMemory(hProcess, (BYTE*)playTimeAddr, &playTime, sizeof(playTime), &playTimeRead);
}

VOID CGameHook::itemGib(DWORD itemId) {

	DWORD processId = GetCurrentProcessId();
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, processId);

	uintptr_t gibItem = (uintptr_t)itemGibDataCodeCave + 4;

	char* littleEndianItemId = (char*)malloc(sizeof(DWORD));
	ConvertToLittleEndianByteArray((uintptr_t)itemId, littleEndianItemId);

	DWORD memory = 0;
	ReadProcessMemory(hProcess, (BYTE*)gibItem, &memory, sizeof(memory), nullptr);
	DWORD newMemory = itemId;
	WriteProcessMemory(hProcess, (BYTE*)gibItem, &newMemory, sizeof(newMemory), nullptr);

	typedef int func(void);
	func* f = (func*)0x13ffe0000;
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)f, NULL, NULL, NULL);
}

BOOL CGameHook::Hook(DWORD64 qAddress, DWORD64 qDetour, DWORD64* pReturn, DWORD dByteLen) {

	MH_STATUS status = MH_CreateHook((LPVOID)qAddress, (LPVOID)qDetour, 0);
	if (status != MH_OK) return false;
	if (MH_EnableHook((LPVOID)qAddress) != MH_OK) return false;

	*pReturn = (qAddress + dByteLen);

	return true;
}

BOOL CGameHook::replaceShellCodeAddress(BYTE *shellcode, int shellCodeOffset, LPVOID codeCave, int codeCaveOffset, int length) {

	char* addressArray = (char*)malloc(sizeof(void*));
	ConvertToLittleEndianByteArray((uintptr_t)codeCave + codeCaveOffset, addressArray);
	if (addressArray == 0) { return false; }
	memcpy(shellcode + shellCodeOffset, addressArray, length);
	free(addressArray);

	return true;
}

LPVOID CGameHook::InjectShellCode(LPVOID address, BYTE* shellCode, size_t len) {
	
	LPVOID pCodeCave = VirtualAlloc(address, 0x3000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!pCodeCave) {
		return nullptr;
	}

	// copy the machine code into that memory:
	std::memcpy(pCodeCave, shellCode, len);

	// mark the memory as executable:
	DWORD lpflOldProtect;
	VirtualProtect(pCodeCave, len, PAGE_EXECUTE_READ, &lpflOldProtect);

	return pCodeCave;
}

void CGameHook::ConvertToLittleEndianByteArray(uintptr_t address, char* output) {
	for (int i = 0; i < sizeof(void*); ++i) {
		output[i] = address & 0xff;
		address >>= 8;
	}
}

uintptr_t CGameHook::FindExecutableAddress(uintptr_t ptrOffset, std::vector<unsigned int> offsets) {
	DWORD processId = GetCurrentProcessId();
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, processId);

	uintptr_t moduleBase = GetModuleBaseAddress();
	uintptr_t dynamicPtrAddr = moduleBase + ptrOffset;
	return FindDMAAddy(hProcess, dynamicPtrAddr, offsets);
}

uintptr_t CGameHook::FindDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets) {

	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i) {
		ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), 0);
		addr += offsets[i];
	}
	return addr;
}

uintptr_t CGameHook::GetModuleBaseAddress() {
	const char* lpModuleName = "DarkSoulsIII.exe";
	DWORD procId = GetCurrentProcessId();

	MODULEENTRY32 lpModuleEntry = { 0 };
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, procId);
	if (!hSnapShot)
		return NULL;
	lpModuleEntry.dwSize = sizeof(lpModuleEntry);
	BOOL bModule = Module32First(hSnapShot, &lpModuleEntry);
	while (bModule)
	{
		if (!strcmp(lpModuleEntry.szModule, lpModuleName))
		{
			CloseHandle(hSnapShot);
			return (uintptr_t)lpModuleEntry.modBaseAddr;
		}
		bModule = Module32Next(hSnapShot, &lpModuleEntry);
	}
	CloseHandle(hSnapShot);
	return NULL;
}

VOID CGameHook::LockEquipSlots() {

	DWORD dOldProtect = 0;
	DWORD64 qEquip = 0x140B70F45;
	DWORD64 qUnequip = 0x140B736EA;

	if (!VirtualProtect((LPVOID)qEquip, 1, PAGE_EXECUTE_READWRITE, &dOldProtect)) return;
	if (!VirtualProtect((LPVOID)qUnequip, 1, PAGE_EXECUTE_READWRITE, &dOldProtect)) return;

	*(BYTE*)qEquip = 0x30;
	*(BYTE*)qUnequip = 0x30;

	if (!VirtualProtect((LPVOID)qEquip, 1, dOldProtect, &dOldProtect)) return;
	if (!VirtualProtect((LPVOID)qUnequip, 1, dOldProtect, &dOldProtect)) return;

	return;
}