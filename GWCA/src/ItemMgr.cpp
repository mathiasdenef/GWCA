#include <GWCA/Managers/ItemMgr.h>

#include <GWCA/Context/GameContext.h>
#include <GWCA/Managers/StoCMgr.h>
#include <GWCA/Managers/CtoSMgr.h>

#include <GWCA/Utilities/Hooker.h>

void GW::Items::OpenXunlaiWindow() {
	GW::Packet::StoC::DataWindow pack;
	pack.agent = 0;
	pack.type = 0;
	pack.data = 3;
	StoC::EmulatePacket(&pack);
}

void GW::Items::PickUpItem(Item *item, DWORD CallTarget /*= 0*/) {
	CtoS::SendPacket(0xC, 0x46, item->AgentId, CallTarget);
}

void GW::Items::DropItem(Item *item, DWORD quantity) {
	CtoS::SendPacket(0xC, 0x33, item->ItemId, quantity);
}

void GW::Items::EquipItem(Item *item) {
	CtoS::SendPacket(0x8, 0x37, item->ItemId);
}

void GW::Items::UseItem(GW::Item *item) {
	CtoS::SendPacket(0x8, 0x84, item->ItemId);
}

GW::Bag **GW::Items::GetBagArray() {
	return GameContext::instance()->items->inventory->Bags;
}

GW::Bag *GW::Items::GetBag(Constants::Bag bag_id) {
	GW::Bag **bags = GetBagArray();
	if (!bags) return nullptr;
	return bags[(unsigned)bag_id];
}

GW::Bag *GW::Items::GetBag(DWORD bag_id) {
	if (bag_id >= GW::Constants::BagMax)
		return nullptr;
	GW::Bag **bags = GetBagArray();
	if (!bags) return nullptr;
	return bags[bag_id];
}

GW::Item *GW::Items::GetItemBySlot(Bag *bag, DWORD slot) {
	if (!bag || slot == 0) return nullptr;
	if (!bag->Items.valid()) return nullptr;
	if (slot > bag->Items.size()) return nullptr;
	return bag->Items[slot - 1];
}

GW::Item *GW::Items::GetItemBySlot(GW::Constants::Bag bag, DWORD slot) {
	GW::Bag *bag_ptr = GetBag(bag);
	return GetItemBySlot(bag_ptr, slot);
}

GW::Item *GW::Items::GetItemBySlot(DWORD bag, DWORD slot) {
	if (bag < 0 || Constants::BagMax <= bag) return nullptr;
	return GetItemBySlot((Constants::Bag)bag, slot);
}

GW::ItemArray GW::Items::GetItemArray() {
	return GameContext::instance()->items->itemarray;
}

void GW::Items::DropGold(DWORD Amount /*= 1*/) {
	CtoS::SendPacket(0x8, 0x36, Amount);
}

DWORD GW::Items::GetGoldAmountOnCharacter() {
	return GameContext::instance()->items->inventory->GoldCharacter;
}

DWORD GW::Items::GetGoldAmountInStorage() {
	return GameContext::instance()->items->inventory->GoldStorage;
}

void GW::Items::OpenLockedChest() {
	CtoS::SendPacket(0x8, 0x5A, 0x2);
}

void GW::Items::MoveItem(Item *item, Bag *bag, int slot, int quantity) {
	if (slot < 0) return;
	if (!item || !bag) return;
	if (bag->Items.size() < (unsigned)slot) return;
	// @Robustness: Check if there is enough space at the destination.
	CtoS::SendPacket(0x10, 0x78, item->ItemId, bag->BagId, slot);
}

void GW::Items::MoveItem(Item *item, Constants::Bag bag_id, int slot, int quantity)
{
	Bag *bag = GetBag(bag_id);
	if (!bag) return;
	MoveItem(item, bag, slot, quantity);
}

void GW::Items::MoveItem(Item *from, Item *to, int quantity) {
	if (!from || !to) return;
	if (!from->Bag || !to->Bag) return;
	if (quantity <= 0) quantity = from->Quantity;
	if (quantity + to->Quantity > 250) return;
	CtoS::SendPacket(0x10, 0x78, from->ItemId, to->Bag->BagId, to->Slot);
}

bool GW::Item::GetIsStackable() {
	switch (Type) {
	case 0:  return false; // Salvage
	case 1:  return false; // LeadHand
	case 2:  return false; // Axe
	case 3:  return false; // Bag
	case 4:  return false; // Feet
	case 5:  return false; // Bow
	case 6:  return false; // Bundle
	case 7:  return false; // Chest
	case 8:  return false; // Rune
	case 9:  return true;  // Consumable
	case 10: return true;  // Dye
	case 11: return true;  // Material
	case 12: return false; // Focus
	case 13: return false; // Arms
	case 14: return true;  // Sigil
	case 15: return false; // Hammer
	case 16: return false; // Head
	case 17: return false; // SalvageItem
	case 18: return true;  // Key
	case 19: return false; // Legs
	case 20: return true;  // Coins
	case 21: return false; // QuestItem
	case 22: return false; // Wand
	case 24: return false; // Shield
	case 26: return false; // Staff
	case 27: return false; // Sword
	case 29: return false; // Kit
	case 30: return true;  // Trophy
	case 31: return true;  // Scroll
	case 32: return false; // Daggers
	case 33: return true;  // Present
	case 34: return false; // Minipet
	case 35: return false; // Scythe
	case 36: return false; // Spear
	case 43: return false; // Handbook
	case 44: return false; // CostumeBody
	case 45: return false; // CostumeHead
	};
	return false;
}

bool GW::Item::GetIsZcoin() {
	if (ModelFileID == 31202) return true; // Copper
	if (ModelFileID == 31203) return true; // Gold
	if (ModelFileID == 31204) return true; // Silver
	return false;
}

bool GW::Item::GetIsMaterial() {
	if (Type == (DWORD)GW::Constants::ItemType::Materials_Zcoins
		&& !GetIsZcoin()) {
		return true;
	}
	return false;
}

int GW::Items::GetMaterialSlot(DWORD model_id) {
	switch (model_id) {
	case 921: return GW::Constants::Bone;
	case 948: return GW::Constants::IronIngot;
	case 940: return GW::Constants::TannedHideSquare;
	case 953: return GW::Constants::Scale;
	case 954: return GW::Constants::ChitinFragment;
	case 925: return GW::Constants::BoltofCloth;
	case 946: return GW::Constants::WoodPlank;
	case 955: return GW::Constants::GraniteSlab;
	case 929: return GW::Constants::PileofGlitteringDust;
	case 934: return GW::Constants::PlantFiber;
	case 933: return GW::Constants::Feather;
	// rare
	case 941: return GW::Constants::FurSquare;
	case 926: return GW::Constants::BoltofLinen;
	case 927: return GW::Constants::BoltofDamask;
	case 928: return GW::Constants::BoltofSilk;
	case 930: return GW::Constants::GlobofEctoplasm;
	case 949: return GW::Constants::SteelIngot;
	case 950: return GW::Constants::DeldrimorSteelIngot;
	case 923: return GW::Constants::MonstrousClaw;
	case 931: return GW::Constants::MonstrousEye;
	case 932: return GW::Constants::MonstrousFang;
	case 937: return GW::Constants::Ruby;
	case 938: return GW::Constants::Sapphire;
	case 935: return GW::Constants::Diamond;
	case 936: return GW::Constants::OnyxGemstone;
	case 922: return GW::Constants::LumpofCharcoal;
	case 945: return GW::Constants::ObsidianShard;
	case 939: return GW::Constants::TemperedGlassVial;
	case 942: return GW::Constants::LeatherSquare;
	case 943: return GW::Constants::ElonianLeatherSquare;
	case 944: return GW::Constants::VialofInk;
	case 951: return GW::Constants::RollofParchment;
	case 952: return GW::Constants::RollofVellum;
	case 956: return GW::Constants::SpiritwoodPlank;
	case 6532: return GW::Constants::AmberChunk;
	case 6533: return GW::Constants::JadeiteShard;
	};
	return -1;
}

int GW::Items::GetMaterialSlot(Item *item) {
	if (!item) return -1;
	if (!item->GetIsMaterial()) return -1;
	return GetMaterialSlot(item->ModelId);
}

bool GW::Items::UseItemByModelId(DWORD modelid, int bagStart, int bagEnd) {
	Bag **bags = GetBagArray();
	if (bags == NULL) return false;

	Bag *bag = NULL;
	Item *item = NULL;

	for (int bagIndex = bagStart; bagIndex <= bagEnd; ++bagIndex) {
		bag = bags[bagIndex];
		if (bag != NULL) {
			ItemArray items = bag->Items;
			if (!items.valid()) return false;
			for (size_t i = 0; i < items.size(); i++) {
				item = items[i];
				if (item != NULL) {
					if (item->ModelId == modelid) {
						UseItem(item);
						return true;
					}
				}
			}
		}
	}

	return false;
}

DWORD GW::Items::CountItemByModelId(DWORD modelid, int bagStart, int bagEnd) {
	DWORD itemcount = 0;
	Bag **bags = GetBagArray();
	Bag  *bag = NULL;

	for (int bagIndex = bagStart; bagIndex <= bagEnd; ++bagIndex) {
		bag = bags[bagIndex];
		if (bag != NULL) {
			ItemArray items = bag->Items;
			for (size_t i = 0; i < items.size(); i++) {
				if (items[i]) {
					if (items[i]->ModelId == modelid) {
						itemcount += items[i]->Quantity;
					}
				}
			}
		}
	}

	return itemcount;
}

GW::Item* GW::Items::GetItemByModelId(DWORD modelid, int bagStart, int bagEnd) {
	Bag **bags = GetBagArray();
	Bag  *bag = NULL;

	for (int bagIndex = bagStart; bagIndex <= bagEnd; ++bagIndex) {
		bag = bags[bagIndex];
		if (bag != NULL) {
			ItemArray items = bag->Items;
			for (size_t i = 0; i < items.size(); i++) {
				if (items[i]) {
					if (items[i]->ModelId == modelid) {
						return items[i];
					}
				}
			}
		}
	}

	return NULL;
}

int GW::Items::GetStoragePage(void) {
	static DWORD *addr;
	if (!addr) {
		DWORD **tmp = (DWORD **)Scanner::Find("\x0F\x84\xFC\x01\x00\x00\x8B\x43\x14", "xxxxxxxxx", -4);
		printf("[SCAN] StoragePannel = %p\n", tmp);
		if (tmp) addr = *tmp;
	}
	assert(addr);
	// @Cleanup: 20 being the position for the storage, but this array hold way more, for instance the current chat channel
	return addr[20];
}

bool GW::Items::IsStorageOpen(void) {
	static DWORD *addr;
	if (!addr) {
		DWORD **tmp = (DWORD **)Scanner::Find("\x40\x85\xD2\xA3\x00\x00\x00\x00\x75\x05", "xxxx????xx", 4);
		printf("[SCAN] StorageOpen = %p\n", tmp);
		assert(tmp);
		addr = *tmp;
	}
	assert(addr);
	return *addr != 0;
}

typedef void (__fastcall *ItemClick_t)(uint32_t *bag_id, uint32_t edx, uint32_t *info);
static GW::THook<ItemClick_t> ItemClickHook;
static std::function<void(uint32_t type, uint32_t slot, GW::Bag *bag)> ItemClickCallback;

static void __fastcall OnItemClick(uint32_t *bag_id, uint32_t edx, uint32_t *info)
{
	// click type:
	//  2  add (when you load / open chest)
	//  5  click
	//  7  release
	//  8  double click
	//  9  move
	//  10 drag-add
	//  12 drag-remove
	uint32_t type = info[2];
	uint32_t slot = info[1] - 2; // for some reason the slot is offset by 2

	GW::Bag *bag = GW::Items::GetBag(*bag_id + 1);
	assert(bag);

	if (ItemClickCallback) {
		ItemClickCallback(type, slot, bag);
	}

	ItemClickHook.Original()(bag_id, edx, info);
}

void GW::Items::SetOnItemClick(std::function<void(uint32_t type, uint32_t slot, GW::Bag *bag)> callback) {
	if (ItemClickHook.Empty()) {
		ItemClick_t ItemClick = (ItemClick_t)Scanner::Find("\x74\x73\x8B\x50\x08", "xxxxx", -25);
		printf("[SCAN] ItemClick = %p\n", ItemClick);
		ItemClickHook.Detour(ItemClick, OnItemClick);
	}
	ItemClickCallback = callback;
}

void GW::Items::RestoreHooks() {
	HookBase::DisableHooks(&ItemClickHook);
}
