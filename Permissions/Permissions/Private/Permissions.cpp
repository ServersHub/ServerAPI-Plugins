#include "../Public/Permissions.h"

#include "Main.h"

namespace Permissions
{
	TArray<FString> GetPlayerGroups(const FString& eos_id)
	{
		return database->GetPlayerGroups(eos_id);
	}

	TArray<FString> GetGroupPermissions(const FString& group)
	{
		if (group.IsEmpty())
			return {};
		return database->GetGroupPermissions(group);
	}

	TArray<FString> GetAllGroups()
	{
		return database->GetAllGroups();
	}

	TArray<FString> GetGroupMembers(const FString& group)
	{
		return database->GetGroupMembers(group);
	}

	bool IsPlayerInGroup(const FString& eos_id, const FString& group)
	{
		TArray<FString> groups = GetPlayerGroups(eos_id);

		for (const auto& current_group : groups)
		{
			if (current_group == group)
				return true;
		}

		return false;
	}

	std::optional<std::string> AddPlayerToGroup(const FString& eos_id, const FString& group)
	{
		return database->AddPlayerToGroup(eos_id, group);
	}

	std::optional<std::string> RemovePlayerFromGroup(const FString& eos_id, const FString& group)
	{
		return database->RemovePlayerFromGroup(eos_id, group);
	}

	std::optional<std::string> AddGroup(const FString& group)
	{
		return database->AddGroup(group);
	}

	std::optional<std::string> RemoveGroup(const FString& group)
	{
		return database->RemoveGroup(group);
	}

	bool IsGroupHasPermission(const FString& group, const FString& permission)
	{
		if (!database->IsGroupExists(group))
			return false;

		TArray<FString> permissions = GetGroupPermissions(group);

		for (const auto& current_perm : permissions)
		{
			if (current_perm == permission)
				return true;
		}

		return false;
	}

	bool IsPlayerHasPermission(const FString& eos_id, const FString& permission)
	{
		TArray<FString> groups = GetPlayerGroups(eos_id);

		for (const auto& current_group : groups)
		{
			if (IsGroupHasPermission(current_group, permission) || IsGroupHasPermission(current_group, "*"))
				return true;
		}

		return false;
	}

	std::optional<std::string> GroupGrantPermission(const FString& group, const FString& permission)
	{
		return database->GroupGrantPermission(group, permission);
	}

	std::optional<std::string> GroupRevokePermission(const FString& group, const FString& permission)
	{
		return database->GroupRevokePermission(group, permission);
	}
}
