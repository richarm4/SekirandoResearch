#pragma once

#include "GameTypes.h"
#include "Params.h"

// A struct representing an item received from another world.
struct SReceivedItem {
	// The Dark Souls 3 ID for this item.
	DWORD address;

	// The number of copies of this item that were received.
	DWORD count;
};

class CItemRandomiser {
public:
	static void __cdecl HookedItemGib(void* mapItemMan, SItemBuffer* pItemBuffer, int32_t* pItemData);
	static uint64_t __cdecl HookedOnGetItem(void* pEquipInventoryData, uint32_t qItemCategory, uint32_t qItemID, uint32_t qCount, void* qUnknown2);

	decltype(&HookedItemGib) ItemGibOriginal;
	decltype(&HookedOnGetItem) OnGetItemOriginal;

	DWORD dIsAutoEquip;
	std::map<DWORD, DWORD> pApItemsToItemIds = { };
	std::map<DWORD, DWORD> pItemCounts = { };
	std::deque<SReceivedItem> receivedItemsQueue = { };
	std::list<int64_t> checkedLocationsList = { };

private:
	virtual VOID RandomiseItem(SItemBuffer* pItemBuffer);
	virtual VOID OnGetSyntheticItem(EquipParamGoodsRow* row);
};
