#pragma once

#include "../Public/Permissions.h"

class IDatabase
{
public:
	virtual ~IDatabase() = default;

	virtual bool AddPlayer(const FString& eos_id) = 0;
	virtual bool IsPlayerExists(const FString& eos_id) = 0;
	virtual bool IsGroupExists(const FString& group) = 0;
	virtual TArray<FString> GetPlayerGroups(const FString& eos_id) = 0;
	virtual TArray<FString> GetGroupPermissions(const FString& group) = 0;
	virtual TArray<FString> GetAllGroups() = 0;
	virtual TArray<FString> GetGroupMembers(const FString& group) = 0;
	virtual std::optional<std::string> AddPlayerToGroup(const FString& eos_id, const FString& group) = 0;
	virtual std::optional<std::string> RemovePlayerFromGroup(const FString& eos_id, const FString& group) = 0;
	virtual std::optional<std::string> AddGroup(const FString& group) = 0;
	virtual std::optional<std::string> RemoveGroup(const FString& group) = 0;
	virtual std::optional<std::string> GroupGrantPermission(const FString& group, const FString& permission) = 0;
	virtual std::optional<std::string> GroupRevokePermission(const FString& group, const FString& permission) = 0;
};
