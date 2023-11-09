#include "GameHook.h"

#include "mem/module.h"
#include "mem/pattern.h"

DWORD64 qItemEquipComms = 0;

DWORD64 rItemRandomiser = 0;
DWORD64 rAutoEquip = 0;
DWORD64 rNoWeaponRequirements = 0;
DWORD64 rEquipLock = 0;

LPVOID itemGibDataCodeCave;

extern CItemRandomiser* ItemRandomiser;
extern CArchipelago* ArchipelagoInterface;
extern CCore* Core;

// A singleton object used by DS3 code involving items.
// From https://raw.githubusercontent.com/The-Grand-Archives/Dark-Souls-III-CT-TGA/v2.3.2/DS3_The-Grand-Archives.CT
// This should really be an AOB, but the one in the latest TGA table doesn't work for DS3 1.15.
LPVOID* mapItemMan = (LPVOID*)0x144752300;

typedef void (*ItemGibType)(LPVOID mapItemMan, SItemBuffer* items, int* unknown);

// The internal DS3 ItemGib function. The final parameter's use is unknown, but it's definitely
// safe to pass a pointer to a non-negative number.
ItemGibType fItemGib = (ItemGibType)0x1407BBA70;

/*
* Check if a basic hook is working on this version of the game  
*/
BOOL CGameHook::preInitialize() {
	Core->Logger("CGameHook::preInitialize", true, false);

	try {
		if (MH_Initialize() != MH_OK) return false;
	} catch (const std::exception&) {
		Core->Logger("Cannot initialize MinHook");
		return false;
	}

	try {
		return Hook(0x1407BBA80, (DWORD64)&tItemRandomiser, &rItemRandomiser, 5)
			&& SimpleHook((LPVOID)0x14058aa20, (LPVOID)&fOnGetItem, (LPVOID*)&ItemRandomiser->OnGetItemOriginal);
	} catch (const std::exception&) {
		Core->Logger("Cannot hook the game 0x1407BBA80");
	}
	return false;
}

BOOL CGameHook::initialize() {
	Core->Logger("CGameHook::initialize", true, false);

	// From https://raw.githubusercontent.com/The-Grand-Archives/Dark-Souls-III-CT-TGA/v2.3.2/DS3_The-Grand-Archives.CT
	// This should really be an AOB, but the one in the latest TGA doesn't work for DS3 1.15.
	mapItemMan = (LPVOID*)0x144752300;

	return true;
}

BOOL CGameHook::applySettings() {
	BOOL bReturn = true;

	if (dIsAutoEquip) { bReturn &= Hook(0x1407BBE92, (DWORD64)&tAutoEquip, &rAutoEquip, 6); }
	if (dIsNoWeaponRequirements) { bReturn &= Hook(0x140C073B9, (DWORD64)&tNoWeaponRequirements, &rNoWeaponRequirements, 7); }
	if (dIsNoSpellsRequirements) { RemoveSpellsRequirements(); }
	if (dLockEquipSlots) { LockEquipSlots(); }
	if (dIsNoEquipLoadRequirements) { RemoveEquipLoad(); }
	if (dEnableDLC) {
		if (!checkIsDlcOwned()) {
			Core->Panic("You must own both the ASHES OF ARIANDEL and THE RINGED CITY DLC in order to enable the DLC option in Archipelago", "Missing DLC detected", FE_MissingDLC, 1);
		}
	}
	return bReturn;
}

VOID CGameHook::manageDeathLink() {

	
	if (lastHealthPoint == 0 && healthPoint != 0) {	//The player just respawned
		deathLinkData = false;
	} else if (deathLinkData && lastHealthPoint != 0 && healthPoint != 0 ) { //The player received a deathLink
		killThePlayer();
	} else if(lastHealthPoint != 0 && healthPoint == 0) { //The player just died, ignore the deathLink if received
		if (deathLinkData) {
			Core->Logger("The player just died, a death link has been ignored", true, false);
			deathLinkData = false;
			return;
		}
		ArchipelagoInterface->sendDeathLink();
	}
}

VOID CGameHook::killThePlayer() {
	Core->Logger("Kill the player", true, false);
	DWORD processId = GetCurrentProcessId();
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, processId);
	std::vector<unsigned int> hpOffsets = { 0x80, 0x1F90, 0x18, 0xD8 };
	uintptr_t healthPointAddr = FindExecutableAddress(0x4768E78, hpOffsets); //BaseB + HP Offsets

	int newHP = 0;
	WriteProcessMemory(hProcess, (BYTE*)healthPointAddr, &newHP, sizeof(newHP), nullptr);
}

VOID CGameHook::updateRuntimeValues() {

	DWORD processId = GetCurrentProcessId();
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, processId);

	std::vector<unsigned int> hpOffsets = { 0x80, 0x1F90, 0x18, 0xD8 };
	uintptr_t healthPointAddr = FindExecutableAddress(0x4768E78, hpOffsets); //BaseB + HP Offsets

	std::vector<unsigned int> playTimeOffsets = { 0xA4 };
	uintptr_t playTimeAddr = FindExecutableAddress(0x4740178, playTimeOffsets); //BaseA + PlayTime Offsets	

	std::vector<unsigned int> soulOfCinderDefeatedFlagOffsets = { 0x00, 0x5F67 };
	uintptr_t soulOfCinderDefeatedFlagAddress = FindExecutableAddress(0x473BE28, soulOfCinderDefeatedFlagOffsets); //GameFlagData + Sould of Cinder defeated flag Offsets	

	lastHealthPoint = healthPoint;

	ReadProcessMemory(hProcess, (BYTE*)healthPointAddr, &healthPoint, sizeof(healthPoint), &healthPointRead);
	ReadProcessMemory(hProcess, (BYTE*)playTimeAddr, &playTime, sizeof(playTime), &playTimeRead);
	ReadProcessMemory(hProcess, (BYTE*)soulOfCinderDefeatedFlagAddress, &soulOfCinderDefeated, sizeof(soulOfCinderDefeated), &soulOfCinderDefeatedFlagRead);

	//Enable the Path of The Dragon Gesture manually when receiving the item
	if (ItemRandomiser->enablePathOfTheDragon) {
		ItemRandomiser->enablePathOfTheDragon = false;

		std::vector<unsigned int> pathOfDragonOffsets = { 0x10, 0x7B8, 0x90 };
		uintptr_t gestureAddr = FindExecutableAddress(0x4740178, pathOfDragonOffsets); //BaseA + Path of the dragon Offsets

		char gestureUnlocked = 0x43;
		WriteProcessMemory(hProcess, (BYTE*)gestureAddr, &gestureUnlocked, sizeof(gestureUnlocked), nullptr);
	}
}

VOID CGameHook::giveItems() {
	//Send the next item in the list
	int size = ItemRandomiser->receivedItemsQueue.size();
	if (size > 0) {
		Core->Logger("Send an item from the list of items", true, false);
		SReceivedItem item = ItemRandomiser->receivedItemsQueue.back();
		SItemBuffer items = { 1, {item.address, item.count, -1} };
		int unknown = 1;
		fItemGib(*mapItemMan, &items, &unknown);
	}
}

BOOL CGameHook::isSoulOfCinderDefeated() {
	constexpr std::uint8_t mask7{ 0b1000'0000 };
	return soulOfCinderDefeatedFlagRead != 0 && (int)(soulOfCinderDefeated & mask7) == 128;
}

BOOL CGameHook::Hook(DWORD64 qAddress, DWORD64 qDetour, DWORD64* pReturn, DWORD dByteLen) {

	*pReturn = (qAddress + dByteLen);
	return SimpleHook((LPVOID)qAddress, (LPVOID)qDetour, 0);
}

BOOL CGameHook::SimpleHook(LPVOID pAddress, LPVOID pDetour, LPVOID* ppOriginal) {

	MH_STATUS status = MH_CreateHook(pAddress, pDetour, ppOriginal);
	if (status != MH_OK) return false;
	if (MH_EnableHook(pAddress) != MH_OK) return false;
	return true;
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

uintptr_t CGameHook::FindDMAAddyStandalone(uintptr_t ptr, std::vector<unsigned int> offsets) {

	DWORD processId = GetCurrentProcessId();
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, NULL, processId);

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

VOID CGameHook::RemoveSpellsRequirements() {

	DWORD processId = GetCurrentProcessId();
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, processId);

	std::vector<unsigned int> offsets = { 0x460, 0x68, 0x68, 0x00 };
	uintptr_t magicAddr = FindExecutableAddress(0x4782838, offsets); //Param + Magic
	
	uintptr_t countAddr = magicAddr + 0x0A;
	int count = 0;
	ReadProcessMemory(hProcess, (BYTE*)countAddr, &count, sizeof(char) * 2, nullptr);

	for (int i = 0; i < count; i++) {
		uintptr_t IDOAddr = magicAddr + 0x48 + 0x18 * i;
		int IDOBuffer;
		ReadProcessMemory(hProcess, (BYTE*)IDOAddr, &IDOBuffer, sizeof(IDOBuffer), nullptr);

		uintptr_t spellAddr = magicAddr + IDOBuffer + 0x1E; //Intelligence
		BYTE newValue = 0x00;
		WriteProcessMemory(hProcess, (BYTE*)spellAddr, &newValue, sizeof(newValue), nullptr);

		spellAddr = magicAddr + IDOBuffer + 0x1F;	//Faith
		WriteProcessMemory(hProcess, (BYTE*)spellAddr, &newValue, sizeof(newValue), nullptr);
	}

	return;
}

VOID CGameHook::RemoveEquipLoad() {

	DWORD processId = GetCurrentProcessId();
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, processId);

	std::vector<unsigned int> offsets = { };
	uintptr_t equipLoadAddr = FindExecutableAddress(0x581FCD, offsets); //EquipLoad 
	
	BYTE newValue[4] = {0x0F, (BYTE)0x57, 0xF6, 0x90};
	WriteProcessMemory(hProcess, (BYTE*)equipLoadAddr, &newValue, sizeof(BYTE) * 4, nullptr);

	return;
}

BYTE* CGameHook::findPattern(BYTE* pBaseAddress, BYTE* pbMask, const char* pszMask, size_t nLength) {
	auto DataCompare = [](const BYTE* pData, const BYTE* mask, const char* cmask, BYTE chLast, size_t iEnd) -> bool {
		if (pData[iEnd] != chLast) return false;
		for (; *cmask; ++cmask, ++pData, ++mask) {
			if (*cmask == 'x' && *pData != *mask) {
				return false;
			}
		}

		return true;
	};

	auto iEnd = strlen(pszMask) - 1;
	auto chLast = pbMask[iEnd];

	auto* pEnd = pBaseAddress + nLength - strlen(pszMask);
	for (; pBaseAddress < pEnd; ++pBaseAddress) {
		if (DataCompare(pBaseAddress, pbMask, pszMask, chLast, iEnd)) {
			return pBaseAddress;
		}
	}

	return nullptr;
}

BOOL CGameHook::checkIsDlcOwned() {
	BOOL ret = false;

	int executableSize = 100093 * 1000;
	BYTE* patternAddr = findPattern((BYTE*)GetModuleBaseAddress(), (BYTE*)csDlcPattern, csDlcMask, executableSize);
	if (patternAddr != nullptr) {
		DWORD processId = GetCurrentProcessId();
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, processId);

		int thirdInteger = -1;
		SIZE_T flag;
		ReadProcessMemory(hProcess, (BYTE*)(patternAddr + 3), &thirdInteger, sizeof(thirdInteger), &flag);
		patternAddr = patternAddr + thirdInteger + 7;
		CSDlc = (uintptr_t)patternAddr;

		std::vector<unsigned int> DLC_1_Offsets = { 0x11 };
		uintptr_t dlc_1_addr = FindDMAAddyStandalone((uintptr_t)CSDlc, DLC_1_Offsets);

		std::vector<unsigned int> DLC_2_Offsets = { 0x12 };
		uintptr_t dlc_2_addr = FindDMAAddyStandalone((uintptr_t)CSDlc, DLC_2_Offsets);

		BYTE isDlc_1 = 0x05;
		ReadProcessMemory(hProcess, (BYTE*)dlc_1_addr, &isDlc_1, sizeof(BYTE), nullptr);

		BYTE isDlc_2 = 0x05;
		ReadProcessMemory(hProcess, (BYTE*)dlc_2_addr, &isDlc_2, sizeof(BYTE), nullptr);

		if (isDlc_1 == 1 && isDlc_2 == 1) {
			ret = true;
		} else {
			printf("Missing DLC!\n DLC #1 : %d , DLC #2 : %d\n", isDlc_1, isDlc_2);
		}
	}

	return ret;
}
