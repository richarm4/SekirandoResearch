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
extern CGameHook* GameHook;

// All of the following hardcoded addresses should really be converted into AOBs that are known to
// be compatible with DS3 1.15 _and_ DS3 1.15.2.

// A singleton object used by DS3 code involving items.
LPVOID* mapItemMan;

// The internal DS3 function that looks up the current localization's message for the given ID. We
// override this to support custom messages with custom IDs.
wchar_t* (*GetActionEventInfoFmgOriginal)(LPVOID messages, DWORD messageId);

// The internal DS3 ItemGib function. The final parameter's use is unknown, but it's definitely
// safe to pass a pointer to a non-negative number.
auto fItemGib = (void (*)(LPVOID mapItemMan, SItemBuffer * items, int* unknown))0x1407BBA70;

// The internal DS3 function that displays the banner with the given message ID. The second
// parameter is probably some sort of enum, but always passing 1 seems to work.
auto fShowBanner = (void (*)(UINT_PTR unused, DWORD unknown, ULONGLONG messageId))0x140473040;

typedef void (*SetEventFlagType)(UINT_PTR unused, DWORD event, BOOL state);

// The internal DS3 function for setting an event ID on or off. Corresponds to the `SetEventFlag`
// function in DarkScript3.
SetEventFlagType fSetEventFlag;

// The function that allocates a bunch of in-game singletons like WorldChrMan. Once this runs, it's
// generally safe to make in-game changes.
LPVOID (*OnWorldLoadedOriginal)(ULONGLONG unknown1, ULONGLONG unknown2, DWORD unknown3,
	DWORD unknown4, DWORD unknown5);

// The deallocator dual of OnWorldLoadedOriginal. Once this runs, it's no longer safe to make
// in-game changes.
void (*OnWorldUnloadedOriginal)(ULONGLONG unknown1, ULONGLONG unknown2, ULONGLONG unknown3,
	ULONGLONG unknown4);

// A singleton class with informatino about installed DLCs.
struct CSDlc : public FD4Singleton<CSDlc, "CSDlc"> {
	void** vftable_ptr;
	uint8_t pad00[0x09];

	// Whether Ashes of Ariandel is installed.
	bool dlc1Installed;

	// Whether The Ringed City is installed.
	bool dlc2Installed;
};

// A singleton class with information about the event system.
struct SprjEventFlagMan : public FD4Singleton<SprjEventFlagMan, "SprjEventFlagMan"> {
	// An array of bit flags that correspond to various states of the local game world.
	uint8_t* worldFlags;
};

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

	// AOB checked against DS3 1.15.0, 1.15.2, and Sekiro
	fSetEventFlag = FindPattern("8b da 45 84 c0 74 ?? 48 85 c9 75", -0xD).as<SetEventFlagType>();
	if (!fSetEventFlag) {
		Core->Logger("Could not locate fSetEventFlag");
		return false;
	}

	// This points to a MOV instruction whose operand (three bytes in) is relative to the next
	// instruction pointer. The resolved value of that operand is what we care about.
	//
	// AOB checked against DS3 1.15.0 and 1.15.2, but does not work with Sekiro
	auto mov = FindPattern("48 8B 0D ?? ?? ?? ?? BB ?? ?? ?? ?? 41 BC");
	mapItemMan = mov.add(7).offset(*mov.add(3).as<int32_t*>()).as<LPVOID*>();

	auto onWorldLoadedAddress = FindPattern(
		"0f 10 00 0f 29 44 24 50 0f 10 48 10 0f 29 4c 24 60 0f 10 40 20 0f 29 44 24 70 0f 10 48 "
		"30 0f 29 8c 24 80 00 00 00",
		-0x60);
	
	auto onWorldUnloadedAddress =
		FindPattern("48 8b 35 ?? ?? ?? ?? 33 ed 48 8b f9 48 85 f6 74 27", -0x14);

	try {
		return Hook(0x1407BBA80, (DWORD64)&tItemRandomiser, &rItemRandomiser, 5)
			&& SimpleHook((LPVOID)0x14058aa20, (LPVOID)&fOnGetItem, (LPVOID*)&ItemRandomiser->OnGetItemOriginal)
			&& SimpleHook((LPVOID)0x140e0c690, (LPVOID)&HookedGetActionEventInfoFmg, (LPVOID*)&GetActionEventInfoFmgOriginal)
			&& SimpleHook(onWorldLoadedAddress.as<LPVOID>(), (LPVOID)&HookedOnWorldLoaded, (LPVOID*)&OnWorldLoadedOriginal)
			&& SimpleHook(onWorldUnloadedAddress.as<LPVOID>(), (LPVOID)&HookedOnWorldUnloaded, (LPVOID*)&OnWorldUnloadedOriginal);
	} catch (const std::exception&) {
		Core->Logger("Cannot hook the game");
	}
	return false;
}

BOOL CGameHook::initialize() {
	Core->Logger("CGameHook::initialize", true, false);

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
	WorldChrMan::instance()->mainCharacter->container->dataModule->hp = 0;
}

VOID debugPrint(const char* prefix, void* data) {
	std::ostringstream stream;
	stream << prefix << std::hex << (ULONGLONG)data;
	Core->Logger(stream.str());
}

VOID CGameHook::updateRuntimeValues() {
	lastHealthPoint = healthPoint;
	healthPoint = WorldChrMan::instance()->mainCharacter->container->dataModule->hp;
	soulOfCinderDefeated = SprjEventFlagMan::instance()->worldFlags[0x5F67];
}

VOID CGameHook::giveItems() {
	//Send the next item in the list
	int size = ItemRandomiser->receivedItemsQueue.size();
	if (size > 0) {
		Core->Logger("Send an item from the list of items", true, false);
		SReceivedItem item = ItemRandomiser->receivedItemsQueue.back();
		if (item.address == 0x40002346) {
			grantPathOfTheDragon();
		} else {
			SItemBuffer items = { 1, {item.address, item.count, -1} };
			int unknown = 1;
			fItemGib(*mapItemMan, &items, &unknown);
		}
	}
}

BOOL CGameHook::isSoulOfCinderDefeated() {
	constexpr std::uint8_t mask7{ 0b1000'0000 };
	return isWorldLoaded && (int)(soulOfCinderDefeated & mask7) == 128;
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

VOID CGameHook::showMessage(std::wstring message) {
	// The way this works is a bit hacky. DS3 looks up all its user-facing text by ID for localization
	// purposes, so we show a banner with an unused ID (0x10000000) and hook into the ID function to
	// return the value of nextMessageToSend. Ideally we'd be able to just set up a 
	nextMessageToSend = message;
	fShowBanner(NULL, 1, 0x10000000);
	nextMessageToSend = std::wstring();
}

VOID CGameHook::showMessage(std::string message) {
	std::wstring wideMessage(message.begin(), message.end());
	showMessage(wideMessage);
}

VOID CGameHook::setEventFlag(DWORD eventId, BOOL enabled) {
	fSetEventFlag(NULL, eventId, enabled);
}

VOID CGameHook::grantPathOfTheDragon() {
	// Archipelago sets up this event flag to grant Path of the Dragon upon being set.
	setEventFlag(100001312, 1);
}

BOOL CGameHook::checkIsDlcOwned() {
	auto dlc = CSDlc::instance();
	return dlc->dlc1Installed && dlc->dlc2Installed;
}

LPVOID CGameHook::HookedOnWorldLoaded(ULONGLONG unknown1, ULONGLONG unknown2, DWORD unknown3,
	DWORD unknown4, DWORD unknown5) {
	auto result = OnWorldLoadedOriginal(unknown1, unknown2, unknown3, unknown4, unknown5);
	GameHook->isWorldLoaded = true;
	return result;
}

void CGameHook::HookedOnWorldUnloaded(ULONGLONG unknown1, ULONGLONG unknown2, ULONGLONG unknown3,
		ULONGLONG unknown4) {
	GameHook->isWorldLoaded = false;
	OnWorldUnloadedOriginal(unknown1, unknown2, unknown3, unknown4);
}

mem::pointer CGameHook::FindPattern(const char* pattern, ptrdiff_t offset) {
	auto main_module = mem::module::main();
	mem::pattern needle(pattern);
	mem::default_scanner scanner(needle);
	mem::pointer result;
	main_module.enum_segments([&](mem::region range, mem::prot_flags prot) {
		scanner(range, [&](mem::pointer address) {
			result = address.offset(offset);
			return (bool)result;
		});

		return (bool)result;
	});

	return result;
}

const wchar_t* CGameHook::HookedGetActionEventInfoFmg(LPVOID messages, DWORD messageId) {
	switch (messageId) {
	case 0x10000000:
		if (GameHook->nextMessageToSend.length() > 0) {
			return GameHook->nextMessageToSend.c_str();
		}
		else {
			return L"[AP mod bug: nextMessageToSend not set]";
		}
	}
	return GetActionEventInfoFmgOriginal(messages, messageId);
}
