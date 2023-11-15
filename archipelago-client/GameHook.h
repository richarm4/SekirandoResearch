#pragma once
#include "Core.h"
#include "Params.h"
#include "mem/mem.h"
#include "./subprojects/minhook/include/MinHook.h"

#define ItemType_Weapon 0
#define ItemType_Protector 1
#define ItemType_Accessory 2
#define ItemType_Goods 4

// A struct representing an item received from another world.
struct SReceivedItem {
	// The Dark Souls 3 ID for this item.
	DWORD address;

	// The number of copies of this item that were received.
	DWORD count;
};

// Constant values used to represent different equip slots in the DS3 inventory.
enum class EquipSlot: DWORD {
	rightHand1 = 0x01,
	head = 0x0C,
	body = 0x0D,
	arms = 0x0E,
	legs = 0x0E,
	ring1 = 0x11,
	ring2 = 0x12,
	ring3 = 0x13,
	ring4 = 0x14,
};

// A Dark Souls 3 struct representing a single item granted to the player.
struct SItemBufferEntry {
	// The DS3 ID of the item being granted.
	DWORD id;

	// The number of items being granted.
	DWORD quantity;

	// The durability of the items being granted. -1 means full durability.
	int durability;
};

// A Dark Souls 3 struct representing a set of items granted to the player.
struct SItemBuffer {
	// The number of items in this buffer.
	DWORD length;

	// The set of items in this buffer.
	SItemBufferEntry items[];
};

// A Dark Souls 3 struct containing information about the current character's available actions.
struct SSprjChrActionFlagModule {
	uint8_t unk00[0x10];
	DWORD chrEquipAnimFlags;
};

// A Dark Souls 3 struct containing information about the current character.
struct SSprjChrDataModule {
	uint8_t unk00[0xD8];

	// The character's HP.
	int hp;
};

// A Dark Souls 3 struct containing various SPRJ modules.
struct SChrInsComponentContainer {
	SSprjChrActionFlagModule* actionModule;
	uint8_t unk00[0x10];
	SSprjChrDataModule* dataModule;
};

// A Dark Souls 3 struct containing information about the current play session.
struct SPlayerIns {
	uint8_t unk00[0x1F90];
	SChrInsComponentContainer* container;
};

// A singleton class containing information about the current play session.
struct WorldChrMan : public FD4Singleton<WorldChrMan, "WorldChrMan"> {
	void** vftable_ptr;
	uint8_t unk00[0x78];
	SPlayerIns* mainCharacter;
};

// A singleton class containing information about the current game world.
struct GameDataMan {
	void** vftable_ptr;
	DWORD unk00;
	LPVOID localPlayerData;

	static GameDataMan* instance();
};

typedef ULONGLONG(*OnGetItemType)(UINT_PTR, DWORD, DWORD, DWORD, UINT_PTR);

class CGameHook {
public:
	virtual BOOL preInitialize();
	virtual BOOL initialize();
	virtual BOOL applySettings();
	virtual VOID updateRuntimeValues();
	virtual VOID giveItems();
	virtual BOOL isSoulOfCinderDefeated();
	virtual VOID manageDeathLink();
	int healthPoint = -1, lastHealthPoint = -1;
	char soulOfCinderDefeated;

	// Whether the game world is currently loaded for the player.
	BOOL isWorldLoaded = false;

	// Displays a banner message to the player. Only works if they're in an active game, not on the
	// menu.
	virtual VOID showMessage(std::wstring message);
	virtual VOID showMessage(std::string message);

	// Sets the event flag with the given ID on or off. Works the same as the DarkScript3 function of
	// the same name.
	virtual VOID setEventFlag(DWORD eventId, BOOL enabled);

	// Grants the player the Path of the Dragon gesture.
	virtual VOID grantPathOfTheDragon();

	// Equips an item for the active player based on its index in their inventory.
	virtual VOID equipItem(EquipSlot equipSlot, DWORD inventorySlot);

	DWORD dLockEquipSlots;
	DWORD dIsNoWeaponRequirements;
	DWORD dIsNoSpellsRequirements;
	DWORD dIsNoEquipLoadRequirements;
	DWORD dIsDeathLink;
	DWORD dEnableDLC;
	HANDLE hHeap;

	BOOL deathLinkData = false;

private:
	static uintptr_t FindExecutableAddress(uintptr_t ptrOffset, std::vector<unsigned int> offsets);
	static uintptr_t GetModuleBaseAddress();
	static uintptr_t FindDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets);
	static BOOL Hook(DWORD64 qAddress, DWORD64 qDetour, DWORD64* pReturn, DWORD dByteLen);
	static BOOL SimpleHook(LPVOID pAddress, LPVOID pDetour, LPVOID* ppOriginal);
	static VOID LockEquipSlots();
	static VOID RemoveSpellsRequirements();
	static VOID RemoveEquipLoad();
	static VOID killThePlayer();
	static const wchar_t* HookedGetActionEventInfoFmg(LPVOID messages, DWORD messageId);
	BOOL checkIsDlcOwned();

	// A hooked function that's run after the data has been loaded for the current game world.
	static LPVOID HookedOnWorldLoaded(ULONGLONG unknown1, ULONGLONG unknown2, DWORD unknown3,
		DWORD unknown4, DWORD unknown5);

	// A hooked function that's run to unload data for the current game world.
	static void HookedOnWorldUnloaded(ULONGLONG unknown1, ULONGLONG unknown2, ULONGLONG unknown3,
		ULONGLONG unknown4);
	

	
	uintptr_t BaseB = -1;
	uintptr_t GameFlagData = -1;
	uintptr_t Param = -1;
	uintptr_t EquipLoad = -1;

	// The next message to send when calling the internal message display function. Not remotely
	// thread-safe, but that shouldn't be an issue as long as we only display banners from the main
	// archipelago thread.
	std::wstring nextMessageToSend;
};

class CItemRandomiser {
public:
	virtual VOID RandomiseItem(WorldChrMan* qWorldChrMan, SItemBuffer* pItemBuffer, UINT_PTR pItemData, DWORD64 qReturnAddress);
	virtual VOID OnGetSyntheticItem(EquipParamGoodsRow* row);

	DWORD dIsAutoEquip;
	OnGetItemType OnGetItemOriginal;
	std::map<DWORD, DWORD> pApItemsToItemIds = { };
	std::map<DWORD, DWORD> pItemCounts = { };
	std::deque<SReceivedItem> receivedItemsQueue = { };
	std::list<int64_t> checkedLocationsList = { };

private:
	int isARandomizedLocation(DWORD dItemID);
	BOOL isReceivedFromServer(DWORD dItemID);
	BOOL isProgressiveLocation(DWORD dItemID);
};

class CAutoEquip {
public:
	virtual VOID AutoEquipItem(SItemBuffer* pItemBuffer);
	virtual BOOL FindEquipType(DWORD dItem, DWORD* pArray);
	virtual DWORD GetInventorySlotID(DWORD dItemID);
	virtual VOID LockUnlockEquipSlots(int iIsUnlock);

	// Returns the equip slot for the given item ID, or std::nullopt if the item shouldn't be
	// auto-equipped.
	std::optional<EquipSlot> SortItem(DWORD dItemID);
};

// Returns a pointer to the location of the given memory pattern in the current executable, or
// NULL if the pattern asn't found. If offset is passed, the pointer is adjusted by that many
// bytes if it's found.
//
// The name is used for error reporting.
static mem::pointer FindPattern(const char* name, const char* pattern, ptrdiff_t offset = 0);

// Given a pointer to the beginning of a MOV instruction whose argument is a relative offset,
// returns the address that offset is pointing to.
static mem::pointer ResolveMov(mem::pointer pointer);

extern "C" DWORD64 qItemEquipComms;

extern "C" DWORD64 rItemRandomiser;
extern "C" VOID tItemRandomiser();
extern "C" VOID fItemRandomiser(WorldChrMan* qWorldChrMan, SItemBuffer* pItemBuffer, UINT_PTR pItemData, DWORD64 qReturnAddress);

extern "C" ULONGLONG fOnGetItem(UINT_PTR pEquipInventoryData, DWORD qItemCategory, DWORD qItemID, DWORD qCount, UINT_PTR qUnknown2);

extern "C" DWORD64 rNoWeaponRequirements;
extern "C" VOID tNoWeaponRequirements();
extern "C" VOID fNoWeaponRequirements(DWORD * pRequirementPtr);
