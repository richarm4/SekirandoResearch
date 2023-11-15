#pragma once
#include "Core.h"
#include "Params.h"
#include "mem/mem.h"
#include "./subprojects/minhook/include/MinHook.h"

#define ItemType_Weapon 0
#define ItemType_Protector 1
#define ItemType_Accessory 2
#define ItemType_Goods 4

struct SEquipBuffer;

/// <summary>
/// A struct representing an item received from another world.
/// </summary>
struct SReceivedItem {
	/// <summary>
	/// The Dark Souls 3 ID for this item.
	/// </summary>
	DWORD address;

	/// <summary>
	/// The number of copies of this item that were received.
	/// </summary>
	DWORD count;
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

// A Dark Souls 3 struct containing information about the current character.
struct SSprjChrDataModule {
	uint8_t unk00[0xD8];

	// The character's HP.
	int hp;
};

// A Dark Souls 3 struct containing various SPRJ modules.
struct SChrInsComponentContainer {
	uint8_t unk00[0x18];
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

typedef VOID fEquipItem(DWORD dSlot, SEquipBuffer* E);
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

	DWORD dIsAutoEquip;
	DWORD dLockEquipSlots;
	DWORD dIsNoWeaponRequirements;
	DWORD dIsNoSpellsRequirements;
	DWORD dIsNoEquipLoadRequirements;
	DWORD dIsDeathLink;
	DWORD dEnableDLC;
	UINT_PTR qLocalPlayer = 0x144740178;
	UINT_PTR qWorldChrMan = 0x144768E78;
	UINT_PTR qSprjLuaEvent = 0x14473A9C8;
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

	// Returns a pointer to the location of the given memory pattern in the current executable, or
	// NULL if the pattern asn't found. If offset is passed, the pointer is adjusted by that many
	// bytes if it's found.
	//
	// The name is used for error reporting.
	static mem::pointer FindPattern(const char* name, const char* pattern, ptrdiff_t offset = 0);
	

	
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
	virtual VOID AutoEquipItem(UINT_PTR pItemBuffer, DWORD64 qReturnAddress);
	virtual BOOL SortItem(DWORD dItemID, SEquipBuffer* E);
	virtual BOOL FindEquipType(DWORD dItem, DWORD* pArray);
	virtual DWORD GetInventorySlotID(DWORD dItemID);
	virtual VOID LockUnlockEquipSlots(int iIsUnlock);
	fEquipItem* EquipItem; //0x140AFBBB0
};

struct SEquipBuffer {
	DWORD dUn1;
	DWORD dUn2;
	DWORD dEquipSlot;
	char unkBytes[0x2C];
	DWORD dInventorySlot;
	char paddingBytes[0x60];
};

extern "C" DWORD64 qItemEquipComms;

extern "C" DWORD64 rItemRandomiser;
extern "C" VOID tItemRandomiser();
extern "C" VOID fItemRandomiser(WorldChrMan* qWorldChrMan, SItemBuffer* pItemBuffer, UINT_PTR pItemData, DWORD64 qReturnAddress);

extern "C" ULONGLONG fOnGetItem(UINT_PTR pEquipInventoryData, DWORD qItemCategory, DWORD qItemID, DWORD qCount, UINT_PTR qUnknown2);

extern "C" DWORD64 rAutoEquip;
extern "C" VOID tAutoEquip();
extern "C" VOID fAutoEquip(UINT_PTR pItemBuffer, DWORD64 pItemData, DWORD64 qReturnAddress);

extern "C" DWORD64 rNoWeaponRequirements;
extern "C" VOID tNoWeaponRequirements();
extern "C" VOID fNoWeaponRequirements(DWORD * pRequirementPtr);
