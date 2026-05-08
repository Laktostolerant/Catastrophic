#include "CatInventoryComponent.h"

void UCatInventoryComponent::AddItem(FName ItemID, int32 Qty)
{
	if (ItemID.IsNone() || Qty <= 0) return;
	Items.FindOrAdd(ItemID) += Qty;
	OnInventoryChanged.Broadcast();
}

bool UCatInventoryComponent::RemoveItem(FName ItemID, int32 Qty)
{
	if (ItemID.IsNone() || Qty <= 0)
		return false;

	int32* Current = Items.Find(ItemID);
	if (!Current || *Current < Qty)
		return false;

	*Current -= Qty;
	if (*Current <= 0)
		Items.Remove(ItemID);

    OnInventoryChanged.Broadcast();
    
	return true;
}

bool UCatInventoryComponent::HasItem(FName ItemID, int32 Qty) const
{
	const int32* Current = Items.Find(ItemID);
	return Current && *Current >= Qty;
}

int32 UCatInventoryComponent::GetItemCount(FName ItemID) const
{
	const int32* Current = Items.Find(ItemID);
	return Current ? *Current : 0;
}

void UCatInventoryComponent::Clear()
{
	Items.Empty();
	OnInventoryChanged.Broadcast();
}