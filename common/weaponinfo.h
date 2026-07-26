/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

/**
 * @file weaponinfo.h
 * @brief Defines the shared weapon state structure used for client/server synchronization.
 *
 * This file contains the weapon state exchanged between the server and client
 * prediction systems. Each weapon serializes its runtime state into a
 * ::weapon_data_t instance to ensure consistent behavior during prediction.
 */

#if !defined(WEAPONINFOH)
#define WEAPONINFOH

#ifdef _WIN32
#pragma once
#endif

 /**
  * @brief Stores the runtime state of a weapon.
  *
  * This structure contains the information required to synchronize weapon
  * behavior between the server and the client. Along with generic weapon
  * properties such as ammunition counts and attack timers, it also provides
  * several user-defined fields that individual weapons may use for custom
  * state.
  */
typedef struct weapon_data_s
{
	/// @brief Unique identifier of the weapon.
	int m_iId;

	/// @brief Number of rounds currently loaded in the weapon's clip.
	int m_iClip;

	/// @brief Time until the primary attack may be used again.
	float m_flNextPrimaryAttack;

	/// @brief Time until the secondary attack may be used again.
	float m_flNextSecondaryAttack;

	/// @brief Time until the weapon's idle animation should play.
	float m_flTimeWeaponIdle;

	/// @brief Non-zero if the weapon is currently reloading.
	int m_fInReload;

	/// @brief Non-zero if the weapon is performing a staged reload sequence.
	int m_fInSpecialReload;

	/// @brief Time until the next reload step occurs.
	float m_flNextReload;

	/// @brief Time until the next pump action occurs.
	float m_flPumpTime;

	/// @brief Remaining duration of the reload operation.
	float m_fReloadTime;

	/// @brief Current aimed damage value or multiplier used by applicable weapons.
	float m_fAimedDamage;

	/// @brief Time until the next aim bonus is applied.
	float m_fNextAimBonus;

	/// @brief Non-zero if the weapon is currently zoomed.
	int m_fInZoom;

	/// @brief Weapon-specific state flags.
	int m_iWeaponState;
	
	/**
	 * @brief General-purpose integer reserved for weapon-specific data.
	 *
	 * The engine does not assign any meaning to this field. Individual weapons
	 * may use it to store custom state that needs to be synchronized between
	 * the server and the client.
	 *
	 * @note For example, one weapon might use this to store its firing mode,
	 * while another could use it as a simple state machine or animation index.
	 */
	int iuser1;

	/**
	 * @brief General-purpose integer reserved for weapon-specific data.
	 *
	 * The engine does not assign any meaning to this field. Individual weapons
	 * may use it to store custom state that needs to be synchronized between
	 * the server and the client.
	 *
	 * @note For example, one weapon might use this to store its firing mode,
	 * while another could use it as a simple state machine or animation index.
	 */
	int iuser2;

	/**
	 * @brief General-purpose integer reserved for weapon-specific data.
	 *
	 * The engine does not assign any meaning to this field. Individual weapons
	 * may use it to store custom state that needs to be synchronized between
	 * the server and the client.
	 *
	 * @note For example, one weapon might use it to store its firing mode,
	 * while another could use it as a simple state machine or animation index.
	 */
	int iuser3;

	/**
	 * @brief General-purpose integer reserved for weapon-specific data.
	 *
	 * The engine does not assign any meaning to this field. Individual weapons
	 * may use it to store custom state that needs to be synchronized between
	 * the server and the client.
	 *
	 * @note For example, one weapon might use it to store its firing mode,
	 * while another could use it as a simple state machine or animation index.
	 */
	int iuser4;

	/**
	 * @brief General-purpose floating-point value reserved for weapon-specific data.
	 *
	 * Like @ref iuser1, the engine does not interpret this value. Weapons can
	 * use it to synchronize custom floating-point data, such as timers,
	 * cooldowns, charge levels, or other values that do not fit into the
	 * predefined fields.
	 */
	float fuser1;

	/**
	 * @brief General-purpose floating-point value reserved for weapon-specific data.
	 *
	 * Like @ref iuser1, the engine does not interpret this value. Weapons can
	 * use it to synchronize custom floating-point data, such as timers,
	 * cooldowns, charge levels, or other values that do not fit into the
	 * predefined fields.
	 */
	float fuser2;

	/**
	 * @brief General-purpose floating-point value reserved for weapon-specific data.
	 *
	 * Like @ref iuser1, the engine does not interpret this value. Weapons can
	 * use it to synchronize custom floating-point data, such as timers,
	 * cooldowns, charge levels, or other values that do not fit into the
	 * predefined fields.
	 */
	float fuser3;

	/**
	 * @brief General-purpose floating-point value reserved for weapon-specific data.
	 *
	 * Like @ref iuser1, the engine does not interpret this value. Weapons can
	 * use it to synchronize custom floating-point data, such as timers,
	 * cooldowns, charge levels, or other values that do not fit into the
	 * predefined fields.
	 */
	float fuser4;

	// Had to copy the doc comments around because the 3 GB RAM hog (Visual Studio)
	// still can't understand @copydoc. "Professional IDE" my ass.

} weapon_data_t;

#endif