#pragma once

#include <SQLiteCpp/Database.h>

#include "IDatabase.h"
#include "../Main.h"

class SqlLite : public IDatabase
{
public:
	explicit SqlLite(const std::string& path)
		: db_(path.empty()
			      ? Permissions::GetDbPath()
			      : path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
	{
		try
		{
			db_.exec("PRAGMA journal_mode=WAL;");

			db_.exec("create table if not exists Players ("
				"Id integer primary key autoincrement not null,"
				"EOS_Id text not null COLLATE NOCASE,"
				"Groups text default 'Default,' COLLATE NOCASE"
				");");
			db_.exec("create table if not exists Groups ("
				"Id integer primary key autoincrement not null,"
				"GroupName text not null COLLATE NOCASE,"
				"Permissions text default '' COLLATE NOCASE"
				");");

			// Add default groups

			db_.exec("INSERT INTO Groups(GroupName, Permissions)"
				"SELECT 'Admins', '*,'"
				"WHERE NOT EXISTS(SELECT 1 FROM Groups WHERE GroupName = 'Admins');");
			db_.exec("INSERT INTO Groups(GroupName)"
				"SELECT 'Default'"
				"WHERE NOT EXISTS(SELECT 1 FROM Groups WHERE GroupName = 'Default');");
		}
		catch (const std::exception& exception)
		{
			Log::GetLog()->error("({} {}) Unexpected DB error {}", __FILE__, __FUNCTION__, exception.what());
		}
	}

	bool AddPlayer(const FString& eos_id) override
	{
		try
		{
			SQLite::Statement query(db_, "INSERT INTO Players (eos_id) VALUES (?);");
			query.bind(1, eos_id.ToStringUTF8());
			query.exec();

			return true;
		}
		catch (const std::exception& exception)
		{
			Log::GetLog()->error("({} {}) Unexpected DB error {}", __FILE__, __FUNCTION__, exception.what());
			return false;
		}
	}

	bool IsPlayerExists(const FString& eos_id) override
	{
		int count;

		try
		{
			SQLite::Statement query(db_, "SELECT count(1) FROM Players WHERE EOS_Id = ?;");
			query.bind(1, eos_id.ToStringUTF8());
			query.executeStep();

			count = query.getColumn(0);
		}
		catch (const std::exception& exception)
		{
			Log::GetLog()->error("({} {}) Unexpected DB error {}", __FILE__, __FUNCTION__, exception.what());
			return false;
		}

		return count != 0;
	}

	bool IsGroupExists(const FString& group) override
	{
		int count;

		try
		{
			SQLite::Statement query(db_, "SELECT count(1) FROM Groups WHERE GroupName = ?;");
			query.bind(1, group.ToStringUTF8());
			query.executeStep();

			count = query.getColumn(0);
		}
		catch (const std::exception& exception)
		{
			Log::GetLog()->error("({} {}) Unexpected DB error {}", __FILE__, __FUNCTION__, exception.what());
			return false;
		}

		return count != 0;
	}

	TArray<FString> GetPlayerGroups(const FString& eos_id) override
	{
		TArray<FString> groups;

		try
		{
			SQLite::Statement query(db_, "SELECT Groups FROM Players WHERE EOS_Id = ?;");
			query.bind(1, eos_id.ToStringUTF8());
			if (query.executeStep())
			{
				std::string groups_str = query.getColumn(0);

				FString groups_fstr(groups_str.c_str());

				groups_fstr.ParseIntoArray(groups, L",", true);
			}
		}
		catch (const std::exception& exception)
		{
			Log::GetLog()->error("({} {}) Unexpected DB error {}", __FILE__, __FUNCTION__, exception.what());
		}

		return groups;
	}

	TArray<FString> GetGroupPermissions(const FString& group) override
	{
		if (group.IsEmpty())
			return {};

		TArray<FString> permissions;

		try
		{
			SQLite::Statement query(db_, "SELECT Permissions FROM Groups WHERE GroupName = ?;");
			query.bind(1, group.ToStringUTF8());
			query.executeStep();

			std::string permissions_str = query.getColumn(0);

			FString permissions_fstr(permissions_str.c_str());

			permissions_fstr.ParseIntoArray(permissions, L",", true);
		}
		catch (const std::exception& exception)
		{
			Log::GetLog()->error("({} {}) Unexpected DB error {}", __FILE__, __FUNCTION__, exception.what());
		}

		return permissions;
	}

	TArray<FString> GetAllGroups() override
	{
		TArray<FString> all_groups;

		try
		{
			SQLite::Statement query(db_, "SELECT GroupName FROM Groups;");
			while (query.executeStep())
			{
				all_groups.Add(query.getColumn(0).getText());
			}
		}
		catch (const std::exception& exception)
		{
			Log::GetLog()->error("({} {}) Unexpected DB error {}", __FILE__, __FUNCTION__, exception.what());
		}

		return all_groups;
	}

	TArray<FString> GetGroupMembers(const FString& group) override
	{
		TArray<FString> members;

		try
		{
			SQLite::Statement query(db_, "SELECT EOS_Id FROM Players;");
			while (query.executeStep())
			{
				FString eos_id = FString(query.getColumn(0).getText());
				if (Permissions::IsPlayerInGroup(eos_id, group))
					members.Add(eos_id);
			}
		}
		catch (const std::exception& exception)
		{
			Log::GetLog()->error("({} {}) Unexpected DB error {}", __FILE__, __FUNCTION__, exception.what());
		}

		return members;
	}

	std::optional<std::string> AddPlayerToGroup(const FString& eos_id, const FString& group) override
	{
		if (!IsPlayerExists(eos_id))
			AddPlayer(eos_id);

		if (!IsGroupExists(group))
			return  "Group does not exist";

		if (Permissions::IsPlayerInGroup(eos_id, group))
			return "Player was already added";

		try
		{
			SQLite::Statement query(db_, "UPDATE Players SET Groups = Groups || ? || ',' WHERE EOS_Id = ?;");
			query.bind(1, group.ToStringUTF8());
			query.bind(2, eos_id.ToStringUTF8());
			query.exec();
		}
		catch (const std::exception& exception)
		{
			Log::GetLog()->error("({} {}) Unexpected DB error {}", __FILE__, __FUNCTION__, exception.what());
			return "Unexpected DB error";
		}

		return {};
	}

	std::optional<std::string> RemovePlayerFromGroup(const FString& eos_id, const FString& group) override
	{
		if (!IsPlayerExists(eos_id) || !IsGroupExists(group))
			return "Player or group does not exist";

		if (!Permissions::IsPlayerInGroup(eos_id, group))
			return "Player is not in group";

		TArray<FString> groups = GetPlayerGroups(eos_id);

		FString new_groups;

		for (const FString& current_group : groups)
		{
			if (current_group != group)
				new_groups += current_group + ",";
		}

		try
		{
			SQLite::Statement query(db_, "UPDATE Players SET Groups = ? WHERE EOS_Id = ?;");
			query.bind(1, new_groups.ToStringUTF8());
			query.bind(2, eos_id.ToStringUTF8());
			query.exec();
		}
		catch (const std::exception& exception)
		{
			Log::GetLog()->error("({} {}) Unexpected DB error {}", __FILE__, __FUNCTION__, exception.what());
			return "Unexpected DB error";
		}

		return {};
	}

	std::optional<std::string> AddGroup(const FString& group) override
	{
		if (IsGroupExists(group))
			return "Group already exists";

		try
		{
			SQLite::Statement query(db_, "INSERT INTO Groups (GroupName) VALUES (?);");
			query.bind(1, group.ToStringUTF8());
			query.exec();
		}
		catch (const std::exception& exception)
		{
			Log::GetLog()->error("({} {}) Unexpected DB error {}", __FILE__, __FUNCTION__, exception.what());
			return "Unexpected DB error";
		}

		return {};
	}

	std::optional<std::string> RemoveGroup(const FString& group) override
	{
		if (!IsGroupExists(group))
			return "Group does not exist";

		// Remove all players from this group

		TArray<FString> group_members = GetGroupMembers(group);
		for (const FString player : group_members)
		{
			RemovePlayerFromGroup(player, group);
		}

		// Delete group

		try
		{
			SQLite::Statement query(db_, "DELETE FROM Groups WHERE GroupName = ?;");
			query.bind(1, group.ToStringUTF8());
			query.exec();
		}
		catch (const std::exception& exception)
		{
			Log::GetLog()->error("({} {}) Unexpected DB error {}", __FILE__, __FUNCTION__, exception.what());
			return "Unexpected DB error";
		}

		return {};
	}

	std::optional<std::string> GroupGrantPermission(const FString& group, const FString& permission) override
	{
		if (!IsGroupExists(group))
			return "Group does not exist";

		if (Permissions::IsGroupHasPermission(group, permission))
			return "Group already has this permission";

		try
		{
			SQLite::Statement
				query(db_, "UPDATE Groups SET Permissions = Permissions || ? || ',' WHERE GroupName = ?;");
			query.bind(1, permission.ToStringUTF8());
			query.bind(2, group.ToStringUTF8());
			query.exec();
		}
		catch (const std::exception& exception)
		{
			Log::GetLog()->error("({} {}) Unexpected DB error {}", __FILE__, __FUNCTION__, exception.what());
			return "Unexpected DB error";
		}

		return {};
	}

	std::optional<std::string> GroupRevokePermission(const FString& group, const FString& permission) override
	{
		if (!IsGroupExists(group))
			return "Group does not exist";

		if (!Permissions::IsGroupHasPermission(group, permission))
			return "Group does not have this permission";

		TArray<FString> permissions = GetGroupPermissions(group);

		FString new_permissions;

		for (const FString& current_perm : permissions)
		{
			if (current_perm != permission)
				new_permissions += current_perm + ",";
		}

		try
		{
			SQLite::Statement query(db_, "UPDATE Groups SET Permissions = ? WHERE GroupName = ?;");
			query.bind(1, new_permissions.ToStringUTF8());
			query.bind(2, group.ToStringUTF8());
			query.exec();
		}
		catch (const std::exception& exception)
		{
			Log::GetLog()->error("({} {}) Unexpected DB error {}", __FILE__, __FUNCTION__, exception.what());
			return "Unexpected DB error";
		}

		return {};
	}

private:
	SQLite::Database db_;
};
