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
	virtual VOID RandomiseItem(WorldChrMan* qWorldChrMan, SItemBuffer* pItemBuffer, LPVOID pItemData);
	virtual VOID OnGetSyntheticItem(EquipParamGoodsRow* row);

	DWORD dIsAutoEquip;
	ItemGibType ItemGibOriginal;
	OnGetItemType OnGetItemOriginal;
	std::map<DWORD, DWORD> pApItemsToItemIds = { };
	std::map<DWORD, DWORD> pItemCounts = { };
	std::deque<SReceivedItem> receivedItemsQueue = { };
	std::list<int64_t> checkedLocationsList = { };
};
