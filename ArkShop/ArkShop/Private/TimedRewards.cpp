#include "TimedRewards.h"

#include <Permissions.h>

namespace ArkShop
{
	TimedRewards::TimedRewards()
	{
		AsaApi::GetCommands().AddOnTimerCallback("RewardTimer", std::bind(&TimedRewards::RewardTimer, this));
	}

	TimedRewards& TimedRewards::Get()
	{
		static TimedRewards instance;
		return instance;
	}

	void TimedRewards::AddTask(const FString& id, const FString& eos_id, const std::function<void()>& reward_callback,
	                           int interval)
	{
		const auto now = std::chrono::system_clock::now();
		const auto next_time = now + std::chrono::minutes(interval);

		const auto iter = std::find_if(online_players_.begin(), online_players_.end(),
		                               [eos_id](const std::shared_ptr<OnlinePlayersData>& data) -> bool
		                               {
			                               return data->eos_id == eos_id;
		                               });

		if (iter != online_players_.end())
		{
			const auto iter2 = (*iter)->reward_callbacks.FindByPredicate([&id](const auto& data) -> bool
			{
				return data.id == id;
			});
			if (!iter2)
				(*iter)->reward_callbacks.Add({id, reward_callback, next_time, interval});
		}
		else
		{
			online_players_.push_back(
				std::make_shared<OnlinePlayersData>(eos_id, id, reward_callback, next_time, interval));
		}
	}

	void TimedRewards::RemovePlayer(const FString& eos_id)
	{
		const auto iter = std::find_if(online_players_.begin(), online_players_.end(),
		                               [eos_id](const std::shared_ptr<OnlinePlayersData>& data) -> bool
		                               {
			                               return data->eos_id == eos_id;
		                               });

		if (iter != online_players_.end())
		{
			online_players_.erase(std::remove(online_players_.begin(), online_players_.end(), *iter),
			                      online_players_.end());
		}
	}

	void TimedRewards::RewardTimer()
	{
		const auto now = std::chrono::system_clock::now();

		for (const auto& data : online_players_)
		{
			for (auto& reward_data : data->reward_callbacks)
			{
				const auto next_time = reward_data.next_reward_time;
				if (now >= next_time)
				{
					reward_data.next_reward_time = now + std::chrono::minutes(reward_data.interval);

					reward_data.reward_callback();
				}
			}
		}
	}

	// Free function
	ITimedRewards& GetTimedRewards()
	{
		return TimedRewards::Get();
	}
} // namespace ArkShop
