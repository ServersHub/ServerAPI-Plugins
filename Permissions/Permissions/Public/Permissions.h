#pragma once

#include <API/Ark/Ark.h>

#ifdef ARK_EXPORTS
#define ARK_API __declspec(dllexport)
#else
#define ARK_API __declspec(dllimport)
#endif

namespace Permissions
{
	ARK_API TArray<FString> GetPlayerGroups(const FString& eos_id);
	ARK_API TArray<FString> GetGroupPermissions(const FString& group);
	ARK_API TArray<FString> GetGroupMembers(const FString& group);

	ARK_API bool IsPlayerInGroup(const FString& eos_id, const FString& group);

	ARK_API std::optional<std::string> AddPlayerToGroup(const FString& eos_id, const FString& group);
	ARK_API std::optional<std::string> RemovePlayerFromGroup(const FString& eos_id, const FString& group);

	ARK_API std::optional<std::string> AddPlayerToTimedGroup(const FString& eos_id, const FString& group, int secs, int delaySecs);
	ARK_API std::optional<std::string> RemovePlayerFromTimedGroup(const FString& eos_id, const FString& group);

	ARK_API std::optional<std::string> AddGroup(const FString& group);
	ARK_API std::optional<std::string> RemoveGroup(const FString& group);

	ARK_API bool IsGroupHasPermission(const FString& group, const FString& permission);
	ARK_API bool IsPlayerHasPermission(const FString& eos_id, const FString& permission);

	ARK_API std::optional<std::string> GroupGrantPermission(const FString& group, const FString& permission);
	ARK_API std::optional<std::string> GroupRevokePermission(const FString& group, const FString& permission);


	ARK_API std::optional<std::string> AddTribeToGroup(int tribeId, const FString& group);
	ARK_API std::optional<std::string> RemoveTribeFromGroup(int tribeId, const FString& group);

	ARK_API std::optional<std::string> AddTribeToTimedGroup(int tribeId, const FString& group, int secs, int delaySecs);
	ARK_API std::optional<std::string> RemoveTribeFromTimedGroup(int tribeId, const FString& group);

	ARK_API bool IsTribeInGroup(int tribeId, const FString& group);
	ARK_API bool IsTribeHasPermission(int tribeId, const FString& permission);
	ARK_API TArray<FString> GetTribeGroups(int tribeId);

	ARK_API void AddPlayerPermissionCallback(FString CallbackName, bool onlyCheckOnline, bool cacheBySteamId, bool cacheByTribe, const std::function<TArray<FString>(FString*, int*)>& callback);
	ARK_API void RemovePlayerPermissionCallback(FString CallbackName);
}
