// Christopher Naglik All Rights Reserved


#include "ChrisGameplayTags.h"

namespace ChrisGameplayTags
{
	// ── UI 2D ──────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_UI_Navigate_Soft, "audio.ui.navigate.soft", "Generic button hover");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_UI_Navigate_Main, "audio.ui.navigate.main", "Main menu hover");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_UI_Navigate_Shop, "audio.ui.navigate.shop", "Shop hover");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_UI_Confirm, "audio.ui.confirm", "Continue / Yes / No");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_UI_Page_Change, "audio.ui.page.change", "Page switch");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_UI_Page_Leave, "audio.ui.page.leave", "Panel close animation");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_UI_Tab_Change, "audio.ui.tab.change", "Tab switch");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_UI_Book_Open, "audio.ui.book.open", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_UI_Book_Close, "audio.ui.book.close", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_UI_Book_Flip, "audio.ui.book.flip", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_UI_Leaderboard_Open, "audio.ui.leaderboard.open", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_UI_Leaderboard_Close, "audio.ui.leaderboard.close", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_UI_Lobby_Continue, "audio.ui.lobby.continue", "Ready up / continue to arena");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_UI_Lobby_TeamSlot, "audio.ui.lobby.teamslot", "Confirming red/blue slot");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_UI_Shop_Unlock_Skill, "audio.ui.shop.unlock.skill", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_UI_Shop_Unlock_Consumable, "audio.ui.shop.unlock.consumable", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_UI_Shop_Unlock_Upgrade, "audio.ui.shop.unlock.upgrade", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_UI_Reject, "audio.ui.reject", "Action refused — locked in, readied up, invalid");


	// ── Local player 2D ──────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_Player_Attack_Grunt, "audio.player.attack.grunt", "Effort vocal on a swing — per-character voice");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_Player_Attack_Swoosh, "audio.player.attack.swoosh", "Blade through air — same for all characters");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_Player_Hit_Light, "audio.player.hit.light", "Taking a light hit — per-character voice");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_Player_Hit_Heavy, "audio.player.hit.heavy", "Taking a heavy hit — per-character voice");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_Player_Combo_Hit, "audio.player.combo.hit", "Combo landed successfully");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_Player_Roll_Forward, "audio.player.roll.forward", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_Player_Roll_Backward, "audio.player.roll.backward", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_Player_Roll_Left, "audio.player.roll.left", "Also used for forward-left and backward-left");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_Player_Roll_Right, "audio.player.roll.right", "Also used for forward-right and backward-right");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_Player_LowHealth, "audio.player.lowhealth", "Looping heartbeat while critically injured");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_Player_LevelUp, "audio.player.levelup", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_Player_Death, "audio.player.death", "Death screen sting, owner only");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_Player_Skill_Available, "audio.player.skill.available", "Cooldown finished");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_Player_Skill_Upgrade, "audio.player.skill.upgrade", "Skill upgraded in-arena, silent when maxed");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_HeavyAttack_Cooldown, "ability.heavyattack.cooldown", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_HeavyAttack1_Cooldown, "ability.heavyattack1.cooldown", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_HeavyAttack2_Cooldown, "ability.heavyattack2.cooldown", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_HeavyAttack3_Cooldown, "ability.heavyattack3.cooldown", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_Player_Blinded, "audio.player.blinded", "Nightflare blind, victim only");


	// ── World 3D ─────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_World_Equip, "audio.world.equip", "Weapon drawn — 3D, everyone in range");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_World_Unequip, "audio.world.unequip", "Weapon sheathed");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_World_GroundHit, "audio.world.groundhit", "Weapon striking the ground");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_World_Skill_Smash, "audio.world.skill.smash", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_World_Skill_Stun, "audio.world.skill.stun", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_World_Skill_Thrust, "audio.world.skill.thrust", "Follows the actor, so it sweeps past stationary players");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_World_Skill_Shoot, "audio.world.skill.shoot", "Firing, at the shooter");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_World_Skill_Shoot_Impact, "audio.world.skill.shoot.impact", "Projectile landing, at the target");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_World_Consumable_ElixirOfLife, "audio.world.consumable.elixiroflife", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_World_Consumable_BloodSerum, "audio.world.consumable.bloodserum", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_World_Consumable_WardensPhial, "audio.world.consumable.wardensphial", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_World_Consumable_Quicksilver, "audio.world.consumable.quicksilver", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_World_Consumable_Nightflare, "audio.world.consumable.nightflare", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_World_Skeleton_Death, "audio.world.skeleton.death", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_World_Skeleton_Scream, "audio.world.skeleton.scream", "Idle chatter, not the death scream");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_World_Skeleton_DeathScream, "audio.world.skeleton.deathscream", "Always plays on death, separate from the idle pool");



	// ── General ─────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_Ambience_Arena, "audio.ambience.arena", "Looping arena bed, shop phase into the round");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_Ambience_MultiplayerWind, "audio.ambience.multiplayerwind", "Looping wind on the multiplayer page");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_UI_Banner, "audio.ui.banner", "Sting timed to the banner open animation");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_UI_MatchStats, "audio.ui.matchstats", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_World_FlagSmash, "audio.world.flagsmash", "Flags slamming at round start and on capture");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Audio_Ambience_LoadingScreen, "audio.ambience.loadingscreen", "Loops while the loading screen is up");
}