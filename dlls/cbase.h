/**
 * @file cbase.h
 * @brief Core entity declarations for the Half-Life game DLL.
 *
 * This header defines the fundamental entity hierarchy used by the Half-Life
 * server-side game code. Most gameplay entities ultimately derive from
 * `CBaseEntity`, which provides common functionality such as spawning,
 * thinking, touch and use callbacks, save/restore support, and networking.
 *
 * The classes declared here form the foundation for players, monsters,
 * weapons, items, triggers, and many other gameplay systems.
 *
 * @section hierarchy Entity Hierarchy
 *
 * @code
 * CBaseEntity
 * ├── CBaseDelay
 * │   └── CBaseToggle
 * │       ├── CBaseItem
 * │       └── CBaseMonster
 * │           ├── CBaseCycler
 * │           ├── CBasePlayer
 * │           └── CBaseGroup
 * @endcode
 *
 * @note
 * This hierarchy is not exhaustive. Additional entity classes are declared
 * throughout the game DLL and derive from these base classes.
 *
 * @copyright
 * Copyright (c) 1996–2001 Valve LLC.
 *
 * This product contains software technology licensed from Id Software, Inc.
 * ("Id Technology"). Id Technology (c) 1996 Id Software, Inc.
 *
 * The original Half-Life SDK license restricts use, distribution, and
 * modification to non-commercial enhancements of Valve products.
 */

#define		MAX_PATH_SIZE	10 // max number of nodes available for a path.

// These are caps bits to indicate what an object's capabilities (currently used for save/restore and level transitions)
#define		FCAP_CUSTOMSAVE				0x00000001
#define		FCAP_ACROSS_TRANSITION		0x00000002		// should transfer between transitions
#define		FCAP_MUST_SPAWN				0x00000004		// Spawn after restore
#define		FCAP_DONT_SAVE				0x80000000		// Don't save this
#define		FCAP_IMPULSE_USE			0x00000008		// can be used by the player
#define		FCAP_CONTINUOUS_USE			0x00000010		// can be used by the player
#define		FCAP_ONOFF_USE				0x00000020		// can be used by the player
#define		FCAP_DIRECTIONAL_USE		0x00000040		// Player sends +/- 1 when using (currently only tracktrains)
#define		FCAP_MASTER					0x00000080		// Can be used to "master" other entities (like multisource)

// UNDONE: This will ignore transition volumes (trigger_transition), but not the PVS!!!
#define		FCAP_FORCE_TRANSITION		0x00000080		// ALWAYS goes across transitions

#include "archtypes.h"     // DAL
#include "saverestore.h"
#include "schedule.h"

#ifndef MONSTEREVENT_H
#include "monsterevent.h"
#endif

// C functions for external declarations that call the appropriate C++ methods

#ifndef CBASE_DLLEXPORT
#ifdef _WIN32
#define CBASE_DLLEXPORT _declspec( dllexport )
#else
#define CBASE_DLLEXPORT __attribute__ ((visibility("default")))
#endif
#endif

#if defined EXPORT
#undef EXPORT
#endif

#define EXPORT CBASE_DLLEXPORT

extern "C" CBASE_DLLEXPORT int GetEntityAPI(DLL_FUNCTIONS* pFunctionTable, int interfaceVersion);
extern "C" CBASE_DLLEXPORT int GetEntityAPI2(DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion);

extern int DispatchSpawn(edict_t* pent);
extern void DispatchKeyValue(edict_t* pentKeyvalue, KeyValueData* pkvd);
extern void DispatchTouch(edict_t* pentTouched, edict_t* pentOther);
extern void DispatchUse(edict_t* pentUsed, edict_t* pentOther);
extern void DispatchThink(edict_t* pent);
extern void DispatchBlocked(edict_t* pentBlocked, edict_t* pentOther);
extern void DispatchSave(edict_t* pent, SAVERESTOREDATA* pSaveData);
extern int  DispatchRestore(edict_t* pent, SAVERESTOREDATA* pSaveData, int globalEntity);
extern void	DispatchObjectCollsionBox(edict_t* pent);
extern void SaveWriteFields(SAVERESTOREDATA* pSaveData, const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields, int fieldCount);
extern void SaveReadFields(SAVERESTOREDATA* pSaveData, const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields, int fieldCount);
extern void SaveGlobalState(SAVERESTOREDATA* pSaveData);
extern void RestoreGlobalState(SAVERESTOREDATA* pSaveData);
extern void ResetGlobalState(void);

typedef enum { USE_OFF = 0, USE_ON = 1, USE_SET = 2, USE_TOGGLE = 3 } USE_TYPE;

extern void FireTargets(const char* targetName, CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

typedef void (CBaseEntity::* BASEPTR)(void);
typedef void (CBaseEntity::* ENTITYFUNCPTR)(CBaseEntity* pOther);
typedef void (CBaseEntity::* USEPTR)(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

// For CLASSIFY
#define	CLASS_NONE				0
#define CLASS_MACHINE			1
#define CLASS_PLAYER			2
#define	CLASS_HUMAN_PASSIVE		3
#define CLASS_HUMAN_MILITARY	4
#define CLASS_ALIEN_MILITARY	5
#define CLASS_ALIEN_PASSIVE		6
#define CLASS_ALIEN_MONSTER		7
#define CLASS_ALIEN_PREY		8
#define CLASS_ALIEN_PREDATOR	9
#define CLASS_INSECT			10
#define CLASS_PLAYER_ALLY		11
#define CLASS_PLAYER_BIOWEAPON	12 // hornets and snarks.launched by players
#define CLASS_ALIEN_BIOWEAPON	13 // hornets and snarks.launched by the alien menace
#define	CLASS_BARNACLE			99 // special because no one pays attention to it, and it eats a wide cross-section of creatures.

#define CLASS_VEHICLE			14

class CBaseEntity;
class CBaseToggle;
class CBaseMonster;
class CBasePlayerItem;
class CSquadMonster;


#define	SF_NORESPAWN	( 1 << 30 )// !!!set this bit on guns and stuff that should never respawn.

//
// EHANDLE. Safe way to point to CBaseEntities who may die between frames
//
class EHANDLE
{
private:
	edict_t* m_pent;
	int		m_serialnumber;
public:
	edict_t* Get(void);
	edict_t* Set(edict_t* pent);

	operator int();

	operator CBaseEntity* ();

	CBaseEntity* operator = (CBaseEntity* pEntity);
	CBaseEntity* operator ->();
};


/**
 * @brief Base class for every game entity.
 *
 * CBaseEntity is the root of the gameplay object hierarchy. Every entity
 * created by the Half-Life game DLL ultimately derives from this class,
 * including players, NPCs, weapons, triggers, doors, and projectiles.
 *
 * The engine associates each CBaseEntity instance with an `entvars_t`
 * structure, exposed through the `pev` member.
 *
 * @note Entity instances are allocated by the GoldSrc engine and should
 * not be created using the standard C++ `new` operator.
 */
class CBaseEntity
{
public:
	/**
	 * @brief Engine-managed entity state.
	 *
	 * Points to this entity's `entvars_t` structure, which stores the entity's
	 * state as maintained by the GoldSrc engine (position, velocity, health,
	 * model, flags, and more).
	 *
	 * @note This pointer is owned and managed by the engine. It should not be
	 * saved or restored, as the engine reinitializes it automatically during
	 * entity restoration.
	 */
	entvars_t* pev;

	/**
	 * @brief Current navigation goal entity.
	 *
	 * Points to the next path corner or navigation target that this entity is
	 * moving toward. Primarily used by path-following entities such as monsters
	 * and scripted movement.
	 */
	CBaseEntity* m_pGoalEnt;

	/**
	 * @brief Temporary link pointer.
	 *
	 * Used internally to build temporary linked lists during engine or game
	 * logic operations. This pointer does not represent a persistent ownership
	 * or gameplay relationship between entities.
	 */
	CBaseEntity* m_pLink;

	/**
	 * @brief Initializes the entity.
	 *
	 * Called after the entity has been created and its keyvalues have been
	 * assigned. Override this to perform initialization such as setting the
	 * model, collision bounds, movement type, and think functions.
	 */
	virtual void Spawn() {}

	/**
	 * @brief Loads resources required by the entity.
	 *
	 * Override this to precache models, sounds, sprites, and other assets
	 * before the entity becomes active.
	 *
	 * @note All resources used by the entity should be precached here.
	 */
	virtual void Precache() {}

	/**
	 * @brief Processes a key-value pair from the map.
	 *
	 * Called once for each key-value pair parsed from the map entity
	 * definition. Override this to handle custom entity properties.
	 *
	 * @param pkvd The parsed key-value pair.
	 */
	virtual void KeyValue(KeyValueData* pkvd) { pkvd->fHandled = FALSE; }

	/**
	 * @brief Saves the entity's persistent state.
	 *
	 * Serializes the entity into a save game.
	 *
	 * @param save Save-game serializer.
	 * @return Non-zero on success.
	 */
	virtual int Save(CSave& save);

	/**
	 * @brief Restores the entity's persistent state.
	 *
	 * Deserializes the entity from a save game.
	 *
	 * @param restore Save-game deserializer.
	 * @return Non-zero on success.
	 */
	virtual int Restore(CRestore& restore);

	/**
	 * @brief Returns the entity's capability flags.
	 *
	 * Capability flags determine how the engine interacts with the entity,
	 * such as whether it may transition between map levels.
	 *
	 * @return A bitmask of FCAP_* flags.
	 */
	virtual int ObjectCaps() { return FCAP_ACROSS_TRANSITION; }

	/**
	 * @brief Activates the entity after all entities have spawned.
	 *
	 * Called once the map has finished spawning, allowing entities to safely
	 * resolve references to other entities.
	 */
	virtual void Activate() {}

	/**
	 * @brief Computes the entity's collision bounds.
	 *
	 * Updates the object-to-object collision box used for entity collision
	 * tests. This differs from `pev->mins` and `pev->maxs`, which define the
	 * entity's object-to-world collision bounds.
	 */
	virtual void SetObjectCollisionBox();

	/**
	 * @brief Returns the entity's classification.
	 *
	 * Classification identifies the entity's faction or relationship group,
	 * allowing AI to determine allies, enemies, and neutral entities. Multiple
	 * entity classes may share the same classification (for example, different
	 * human soldiers all belong to the same military faction).
	 *
	 * @return One of the `CLASS_*` classification constants.
	 */
	virtual int Classify() { return CLASS_NONE; }

	/**
	 * @brief Notifies the entity that one of its children has died.
	 *
	 * Called by child entities created by this entity, such as monsters spawned
	 * by a monster maker. Override this to update internal state or spawn
	 * additional entities in response.
	 *
	 * @param pevChild The engine state of the child entity that died.
	 */
	virtual void DeathNotice(entvars_t* pevChild) {}


	/// @brief Save/restore field description table used by the engine.
	static TYPEDESCRIPTION m_SaveData[];

	/**
 * @brief Applies damage from an attack trace.
 *
 * Called when an attack intersects the entity before damage is fully
 * processed. Allows entities to modify or accumulate damage, spawn
 * effects, or determine hitgroup-specific behavior.
 *
 * @param pevAttacker The entity responsible for the attack.
 * @param flDamage Amount of incoming damage.
 * @param vecDir Direction the attack traveled.
 * @param ptr Trace result describing the impact.
 * @param bitsDamageType Bitmask of `DMG_*` damage types.
 */
	virtual void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);

	/**
	 * @brief Applies damage to the entity.
	 *
	 * Called after damage has been calculated to reduce health and perform
	 * any gameplay logic associated with taking damage.
	 *
	 * @param pevInflictor Entity directly causing the damage (for example, a grenade).
	 * @param pevAttacker Entity ultimately responsible for the attack.
	 * @param flDamage Amount of damage to apply.
	 * @param bitsDamageType Bitmask of `DMG_*` damage types.
	 * @return Non-zero if the damage was accepted.
	 */
	virtual int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);

	/**
	 * @brief Restores health to the entity.
	 *
	 * @param flHealth Amount of health to restore.
	 * @param bitsDamageType Damage type being healed, if applicable.
	 * @return Non-zero if health was restored.
	 */
	virtual int TakeHealth(float flHealth, int bitsDamageType);

	/**
	 * @brief Handles the entity's death.
	 *
	 * Called when the entity has been killed after taking fatal damage.
	 *
	 * @param pevAttacker Entity responsible for the kill.
	 * @param iGib Gibbing behavior (`GIB_*`).
	 */
	virtual void Killed(entvars_t* pevAttacker, int iGib);

	/**
	 * @brief Returns the entity's blood color.
	 *
	 * Used to determine which blood effects are produced when the entity
	 * is damaged.
	 *
	 * @return One of the blood color constants, or `DONT_BLEED`.
	 */
	virtual int BloodColor() { return DONT_BLEED; }

	/**
	 * @brief Spawns blood effects from an attack.
	 *
	 * Called after damage has been applied to emit blood decals or particles.
	 *
	 * @param flDamage Damage dealt by the attack.
	 * @param vecDir Direction of the attack.
	 * @param ptr Trace result describing the impact.
	 * @param bitsDamageType Bitmask of `DMG_*` damage types.
	 */
	virtual void TraceBleed(float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);

	/**
	 * @brief Determines whether the entity is currently triggered.
	 *
	 * Used by trigger and logic entities to determine whether they may
	 * activate.
	 *
	 * @param pActivator Entity attempting the activation.
	 * @return `true` if the entity is triggered.
	 */
	virtual BOOL IsTriggered(CBaseEntity* pActivator) { return TRUE; }

	/**
	 * @brief Returns this entity as a toggle entity.
	 *
	 * Allows safe runtime access without requiring a C++ cast.
	 *
	 * @return Pointer to this entity as `CBaseToggle`, or `nullptr`.
	 */
	virtual CBaseToggle* MyTogglePointer() { return nullptr; }

	/**
	 * @brief Returns this entity as a monster.
	 *
	 * @return Pointer to this entity as `CBaseMonster`, or `nullptr`.
	 */
	virtual CBaseMonster* MyMonsterPointer() { return nullptr; }

	/**
	 * @brief Returns this entity as a squad monster.
	 *
	 * @return Pointer to this entity as `CSquadMonster`, or `nullptr`.
	 */
	virtual CSquadMonster* MySquadMonsterPointer() { return nullptr; }

	/**
	 * @brief Returns the current toggle state.
	 *
	 * @return One of the `TOGGLE_STATE` values.
	 */
	virtual int GetToggleState() { return TS_AT_TOP; }

	/**
	 * @brief Awards score to this entity.
	 *
	 * Primarily overridden by player entities.
	 *
	 * @param score Number of points to award.
	 * @param bAllowNegativeScore Whether negative scores are permitted.
	 */
	virtual void AddPoints(int score, bool bAllowNegativeScore) {}

	/**
	 * @brief Awards score to this entity's team.
	 *
	 * @param score Number of points to award.
	 * @param bAllowNegativeScore Whether negative scores are permitted.
	 */
	virtual void AddPointsToTeam(int score, bool bAllowNegativeScore) {}

	/**
	 * @brief Adds an item to the entity's inventory.
	 *
	 * @param pItem Item to add.
	 * @return `true` if the item was accepted.
	 */
	virtual BOOL AddPlayerItem(CBasePlayerItem* pItem) { return FALSE; }

	/**
	 * @brief Removes an item from the entity's inventory.
	 *
	 * @param pItem Item to remove.
	 * @return `true` if the item was removed.
	 */
	virtual BOOL RemovePlayerItem(CBasePlayerItem* pItem) { return FALSE; }

	/**
	 * @brief Gives ammunition to the entity.
	 *
	 * @param iAmount Amount of ammunition.
	 * @param szName Name of the ammunition type.
	 * @param iMax Maximum ammunition capacity.
	 * @return Amount accepted, or `-1` if unsupported.
	 */
	virtual int GiveAmmo(int iAmount, char* szName, int iMax) { return -1; }

	/**
	 * @brief Returns the entity's activation delay.
	 *
	 * @return Delay in seconds.
	 */
	virtual float GetDelay() { return 0.0f; }

	/**
	 * @brief Determines whether the entity is moving.
	 *
	 * @return `true` if the entity currently has non-zero velocity.
	 */
	virtual int IsMoving() { return pev->velocity != g_vecZero; }

	/**
	 * @brief Restores entity-specific state after a reset.
	 *
	 * Override to restore custom runtime data.
	 */
	virtual void OverrideReset() {}

	/**
	 * @brief Returns the decal produced when damaged.
	 *
	 * @param bitsDamageType Bitmask of `DMG_*` damage types.
	 * @return Decal index.
	 */
	virtual int DamageDecal(int bitsDamageType);

	/**
	 * @brief Sets the current toggle state.
	 *
	 * Used only by the node graph to test movement through doors.
	 *
	 * @param state New toggle state.
	 */
	virtual void SetToggleState(int state) {}

	/**
	 * @brief Places the entity into a sneaking state.
	 */
	virtual void StartSneaking() {}

	/**
	 * @brief Ends the entity's sneaking state.
	 */
	virtual void StopSneaking() {}

	/**
	 * @brief Determines whether the entity is currently using controls.
	 *
	 * @param pev Engine state of the controlling entity.
	 * @return `true` if the controls are active.
	 */
	virtual BOOL OnControls(entvars_t* pev) { return FALSE; }

	/**
	 * @brief Determines whether the entity is sneaking.
	 *
	 * @return `true` if sneaking.
	 */
	virtual BOOL IsSneaking() { return FALSE; }

	/**
	 * @brief Determines whether the entity is alive.
	 *
	 * @return `true` if the entity is alive.
	 */
	virtual BOOL IsAlive() { return (pev->deadflag == DEAD_NO) && pev->health > 0; }

	/**
	 * @brief Determines whether this entity uses a BSP model.
	 *
	 * @return `true` if the entity is represented by a BSP model.
	 */
	virtual BOOL IsBSPModel() { return pev->solid == SOLID_BSP || pev->movetype == MOVETYPE_PUSHSTEP; }

	/**
	 * @brief Determines whether Gauss beams should reflect from this entity.
	 *
	 * @return `true` if Gauss shots should reflect.
	 */
	virtual BOOL ReflectGauss() { return IsBSPModel() && !pev->takedamage; }

	/**
	 * @brief Determines whether the entity has the specified target name.
	 *
	 * @param targetname Target name to compare.
	 * @return `true` if the names match.
	 */
	virtual BOOL HasTarget(string_t targetname)
	{
		return FStrEq(STRING(targetname), STRING(pev->targetname));
	}

	/**
	 * @brief Determines whether the entity is inside valid world bounds.
	 *
	 * @return `true` if the entity is within the world.
	 */
	virtual BOOL IsInWorld();

	/**
	 * @brief Determines whether this entity is a player.
	 *
	 * @return `true` if this entity is a player.
	 */
	virtual BOOL IsPlayer() { return FALSE; }

	/**
	 * @brief Determines whether this entity represents a network client.
	 *
	 * @return `true` if this entity is controlled by a connected client.
	 */
	virtual BOOL IsNetClient() { return FALSE; }

	/**
	 * @brief Returns the entity's team identifier.
	 *
	 * @return Team identifier string, or an empty string if none exists.
	 */
	virtual const char* TeamID() { return ""; }


	/**
	 * @brief Returns the entity targeted by this entity.
	 *
	 * Resolves and returns the next entity referenced by this entity's target.
	 * Commonly used by scripted sequences, triggers, and path-following entities.
	 *
	 * @return Pointer to the targeted entity, or `nullptr` if no valid target exists.
	 */
	virtual CBaseEntity* GetNextTarget();

	/**
	 * @brief Function invoked when the entity thinks.
	 *
	 * Assigned through `SetThink()` and executed by the engine during the
	 * entity's scheduled think cycle.
	 */
	void (CBaseEntity::* m_pfnThink)();

	/**
	 * @brief Function invoked when another entity touches this entity.
	 *
	 * Assigned through `SetTouch()` and executed by the engine whenever
	 * a collision or touch event occurs.
	 */
	void (CBaseEntity::* m_pfnTouch)(CBaseEntity* pOther);

	/**
	 * @brief Function invoked when the entity is used.
	 *
	 * Assigned through `SetUse()` and executed when another entity activates
	 * this entity.
	 */
	void (CBaseEntity::* m_pfnUse)(
		CBaseEntity* pActivator,
		CBaseEntity* pCaller,
		USE_TYPE useType,
		float value
		);

	/**
	 * @brief Function invoked when the entity becomes blocked.
	 *
	 * Assigned through `SetBlocked()` and executed when movement is obstructed
	 * by another entity.
	 */
	void (CBaseEntity::* m_pfnBlocked)(CBaseEntity* pOther);

	/**
	 * @brief Executes the entity's think callback.
	 *
	 * Calls the function previously assigned with `SetThink()`, if one exists.
	 */
	virtual void Think()
	{
		if (m_pfnThink)
			(this->*m_pfnThink)();
	}

	/**
	 * @brief Executes the entity's touch callback.
	 *
	 * Calls the function previously assigned with `SetTouch()`.
	 *
	 * @param pOther The entity that touched this entity.
	 */
	virtual void Touch(CBaseEntity* pOther)
	{
		if (m_pfnTouch)
			(this->*m_pfnTouch)(pOther);
	}

	/**
	 * @brief Executes the entity's use callback.
	 *
	 * Calls the function previously assigned with `SetUse()`.
	 *
	 * @param pActivator Entity responsible for activating this entity.
	 * @param pCaller Entity that directly invoked the use action.
	 * @param useType Type of use operation.
	 * @param value Additional value associated with the use action.
	 */
	virtual void Use(
		CBaseEntity* pActivator,
		CBaseEntity* pCaller,
		USE_TYPE useType,
		float value)
	{
		if (m_pfnUse)
			(this->*m_pfnUse)(pActivator, pCaller, useType, value);
	}

	/**
	 * @brief Executes the entity's blocked callback.
	 *
	 * Calls the function previously assigned with `SetBlocked()`.
	 *
	 * @param pOther The entity blocking this entity.
	 */
	virtual void Blocked(CBaseEntity* pOther)
	{
		if (m_pfnBlocked)
			(this->*m_pfnBlocked)(pOther);
	}

	/**
	 * @brief Allocates entity memory using the GoldSrc engine.
	 *
	 * Entities are allocated from engine-managed private data rather than the
	 * standard C++ heap. This overload is used internally by `GetClassPtr()`
	 * and other engine allocation routines.
	 *
	 * @param stAllocateBlock Size of the allocation in bytes.
	 * @param pev Engine entity variables associated with the allocation.
	 * @return Pointer to the newly allocated entity memory.
	 */
	void* operator new(size_t stAllocateBlock, entvars_t* pev)
	{
		return static_cast<void*>(ALLOC_PRIVATE(ENT(pev), stAllocateBlock));
	}

	// This overload exists only to satisfy placement new semantics.
	// Entity destruction is handled by the engine.
#if _MSC_VER >= 1200
	/**
	 * @brief Placement delete corresponding to the engine allocation overload.
	 *
	 * This function should never be called directly. If construction fails,
	 * the entity is marked for removal by the engine.
	 *
	 * @param pMem Unused pointer to the allocated memory.
	 * @param pev Engine entity variables associated with the allocation.
	 */
	void operator delete(void* pMem, entvars_t* pev)
	{
		(void)pMem;
		pev->flags |= FL_KILLME;
	}
#endif

	/**
	 * @brief Performs cleanup before the entity is removed.
	 *
	 * Called immediately before the entity is deleted from the world. Override
	 * this to release references, notify other entities, or perform any final
	 * cleanup required by derived classes.
	 */
	void UpdateOnRemove();

	/**
	 * @brief Removes the entity from the world.
	 *
	 * Standard think callback used to safely delete an entity on the next
	 * engine update.
	 */
	void EXPORT SUB_Remove();

	/**
	 * @brief Empty think callback.
	 *
	 * Performs no action and is commonly used to clear an entity's think
	 * function.
	 */
	void EXPORT SUB_DoNothing();

	/**
	 * @brief Begins fading the entity out.
	 *
	 * Initializes the fade-out process before repeatedly calling
	 * `SUB_FadeOut()`.
	 */
	void EXPORT SUB_StartFadeOut();

	/**
	 * @brief Continues fading the entity out.
	 *
	 * Gradually decreases the entity's render opacity until it is removed.
	 */
	void EXPORT SUB_FadeOut();

	/**
	 * @brief Invokes this entity's use callback with `USE_TOGGLE`.
	 *
	 * Acts as though the entity activated itself using the toggle use type.
	 */
	void EXPORT SUB_CallUseToggle()
	{
		this->Use(this, this, USE_TOGGLE, 0);
	}

	/**
	 * @brief Determines whether a toggle operation should occur.
	 *
	 * Compares the requested use type against the entity's current state to
	 * determine whether a state change is appropriate.
	 *
	 * @param useType Requested use operation.
	 * @param currentState Current toggle state.
	 * @return Non-zero if the toggle should occur.
	 */
	int ShouldToggle(USE_TYPE useType, BOOL currentState);

	/**
	 * @brief Fires one or more bullets from this entity.
	 *
	 * Performs hit detection, applies damage, and optionally generates tracer
	 * effects.
	 *
	 * @param cShots Number of bullets to fire.
	 * @param vecSrc Bullet origin.
	 * @param vecDirShooting Forward shooting direction.
	 * @param vecSpread Random spread applied to each shot.
	 * @param flDistance Maximum trace distance.
	 * @param iBulletType Bullet type identifier.
	 * @param iTracerFreq Frequency at which tracers are generated.
	 * @param iDamage Override damage value. A value of `0` uses the default.
	 * @param pevAttacker Entity credited with the attack.
	 */
	void FireBullets(
		ULONG cShots,
		Vector vecSrc,
		Vector vecDirShooting,
		Vector vecSpread,
		float flDistance,
		int iBulletType,
		int iTracerFreq = 4,
		int iDamage = 0,
		entvars_t* pevAttacker = nullptr
	);

	/**
	 * @brief Fires bullets using player-specific spread calculations.
	 *
	 * Similar to `FireBullets()`, but incorporates deterministic randomization
	 * for multiplayer prediction.
	 *
	 * @param cShots Number of bullets to fire.
	 * @param vecSrc Bullet origin.
	 * @param vecDirShooting Forward shooting direction.
	 * @param vecSpread Random spread applied to each shot.
	 * @param flDistance Maximum trace distance.
	 * @param iBulletType Bullet type identifier.
	 * @param iTracerFreq Frequency at which tracers are generated.
	 * @param iDamage Override damage value. A value of `0` uses the default.
	 * @param pevAttacker Entity credited with the attack.
	 * @param shared_rand Shared random seed used for prediction.
	 * @return Final shooting direction after spread has been applied.
	 */
	Vector FireBulletsPlayer(
		ULONG cShots,
		Vector vecSrc,
		Vector vecDirShooting,
		Vector vecSpread,
		float flDistance,
		int iBulletType,
		int iTracerFreq = 4,
		int iDamage = 0,
		entvars_t* pevAttacker = nullptr,
		int shared_rand = 0
	);

	/**
	 * @brief Creates a replacement instance after this entity respawns.
	 *
	 * Override to implement custom respawn behavior for items, weapons, or
	 * other respawnable entities.
	 *
	 * @return Pointer to the respawned entity, or `nullptr` if respawning is
	 * not supported.
	 */
	virtual CBaseEntity* Respawn() { return nullptr; }

	/**
	 * @brief Fires this entity's targets.
	 *
	 * Activates all entities referenced by this entity's target field using the
	 * specified use type and value.
	 *
	 * @param pActivator Entity responsible for the activation.
	 * @param useType Type of use operation to perform.
	 * @param value Additional value associated with the activation.
	 */
	void SUB_UseTargets(CBaseEntity* pActivator, USE_TYPE useType, float value);

	/**
	 * @brief Determines whether this entity intersects another.
	 *
	 * Performs an axis-aligned bounding box (AABB) intersection test using the
	 * entities' collision bounds.
	 *
	 * @param pOther Entity to test against.
	 * @return `true` if the entities' bounding boxes overlap.
	 */
	BOOL Intersects(CBaseEntity* pOther);

	/**
	 * @brief Places the entity into a dormant state.
	 *
	 * Dormant entities remain allocated but are temporarily inactive and ignored
	 * by most gameplay logic until reactivated.
	 */
	void MakeDormant();

	/**
	 * @brief Determines whether the entity is dormant.
	 *
	 * @return `true` if the entity is currently dormant.
	 */
	BOOL IsDormant();

	/**
	 * @brief Determines whether this entity is locked by a master entity.
	 *
	 * Entities such as doors and buttons override this to prevent activation
	 * until their associated multisource or master entity has been triggered.
	 *
	 * @return `true` if the entity is currently locked.
	 */
	BOOL IsLockedByMaster() { return FALSE; }

	/**
	 * @brief Retrieves the C++ object associated with an engine entity.
	 *
	 * Converts an engine `edict_t` into its corresponding `CBaseEntity`
	 * instance.
	 *
	 * @param pent Engine entity to resolve. If `nullptr`, the world entity is
	 * used.
	 * @return Pointer to the associated entity, or `nullptr` if none exists.
	 */
	static CBaseEntity* Instance(edict_t* pent)
	{
		if (!pent)
			pent = ENT(0);

		CBaseEntity* pEnt = static_cast<CBaseEntity*>(GET_PRIVATE(pent));
		return pEnt;
	}

	/**
	 * @brief Retrieves the C++ object associated with an engine entity.
	 *
	 * Convenience overload accepting an `entvars_t` pointer.
	 *
	 * @param pev Engine entity variables.
	 * @return Pointer to the associated entity, or `nullptr` if none exists.
	 */
	static CBaseEntity* Instance(entvars_t* pev)
	{
		return Instance(ENT(pev));
	}

	/**
	 * @brief Retrieves the C++ object associated with an entity offset.
	 *
	 * Convenience overload accepting an engine entity offset.
	 *
	 * @param eoffset Engine entity offset.
	 * @return Pointer to the associated entity, or `nullptr` if none exists.
	 */
	static CBaseEntity* Instance(int eoffset)
	{
		return Instance(ENT(eoffset));
	}

	/**
	 * @brief Retrieves a monster object from an engine entity.
	 *
	 * Resolves the specified engine entity and returns its monster interface if
	 * it represents a `CBaseMonster`.
	 *
	 * @param pevMonster Engine variables of the monster entity.
	 * @return Pointer to the monster, or `nullptr` if the entity does not exist
	 * or is not a monster.
	 */
	CBaseMonster* GetMonsterPointer(entvars_t* pevMonster)
	{
		CBaseEntity* pEntity = Instance(pevMonster);

		if (pEntity)
			return pEntity->MyMonsterPointer();

		return nullptr;
	}

	/**
	 * @brief Retrieves a monster object from an engine entity.
	 *
	 * Resolves the specified engine entity and returns its monster interface if
	 * it represents a `CBaseMonster`.
	 *
	 * @param pentMonster Engine entity to resolve.
	 * @return Pointer to the monster, or `nullptr` if the entity does not exist
	 * or is not a monster.
	 */
	CBaseMonster* GetMonsterPointer(edict_t* pentMonster)
	{
		CBaseEntity* pEntity = Instance(pentMonster);

		if (pEntity)
			return pEntity->MyMonsterPointer();

		return nullptr;
	}


	// Ugly code to lookup all functions to make sure they are exported when set.
#ifdef _DEBUG

	/**
	 * @brief Verifies that a callback function is exported.
	 *
	 * Debug helper used by the callback assignment functions to ensure the
	 * specified function is exported and can be resolved by the GoldSrc engine.
	 *
	 * @param pFunction Function pointer to validate.
	 * @param name Name of the callback function.
	 */
	void FunctionCheck(void* pFunction, char* name)
	{
		if (pFunction && !NAME_FOR_FUNCTION((uint32)pFunction))
			ALERT(
				AlertType::Error,
				"No EXPORT: %s:%s (%08lx)\n",
				STRING(pev->classname),
				name,
				(uint32)pFunction
			);
	}

	/**
	 * @brief Assigns the entity's think callback.
	 *
	 * Stores the callback and verifies that it is exported in debug builds.
	 *
	 * @param func Callback function to assign.
	 * @param name Name of the callback function.
	 * @return The assigned callback.
	 */
	BASEPTR ThinkSet(BASEPTR func, char* name)
	{
		m_pfnThink = func;

		FunctionCheck(
			(void*)*((int*)((char*)this + offsetof(CBaseEntity, m_pfnThink))),
			name
		);

		return func;
	}

	/**
	 * @brief Assigns the entity's touch callback.
	 *
	 * Stores the callback and verifies that it is exported in debug builds.
	 *
	 * @param func Callback function to assign.
	 * @param name Name of the callback function.
	 * @return The assigned callback.
	 */
	ENTITYFUNCPTR TouchSet(ENTITYFUNCPTR func, char* name)
	{
		m_pfnTouch = func;

		FunctionCheck(
			(void*)*((int*)((char*)this + offsetof(CBaseEntity, m_pfnTouch))),
			name
		);

		return func;
	}

	/**
	 * @brief Assigns the entity's use callback.
	 *
	 * Stores the callback and verifies that it is exported in debug builds.
	 *
	 * @param func Callback function to assign.
	 * @param name Name of the callback function.
	 * @return The assigned callback.
	 */
	USEPTR UseSet(USEPTR func, char* name)
	{
		m_pfnUse = func;

		FunctionCheck(
			(void*)*((int*)((char*)this + offsetof(CBaseEntity, m_pfnUse))),
			name
		);

		return func;
	}

	/**
	 * @brief Assigns the entity's blocked callback.
	 *
	 * Stores the callback and verifies that it is exported in debug builds.
	 *
	 * @param func Callback function to assign.
	 * @param name Name of the callback function.
	 * @return The assigned callback.
	 */
	ENTITYFUNCPTR BlockedSet(ENTITYFUNCPTR func, char* name)
	{
		m_pfnBlocked = func;

		FunctionCheck(
			(void*)*((int*)((char*)this + offsetof(CBaseEntity, m_pfnBlocked))),
			name
		);

		return func;
	}

#endif


	// virtual functions used by a few classes

	/**
	 * @brief Updates the entity's owner.
	 *
	 * Used by monsters created by a MonsterMaker to update their owning entity.
	 * The default implementation performs no action.
	 */
	virtual void UpdateOwner() {}

	//

	/**
	 * @brief Creates a new game entity.
	 *
	 * Creates an entity of the specified classname, places it at the given
	 * position and orientation, and optionally assigns an owner.
	 *
	 * @param szName Classname of the entity to create.
	 * @param vecOrigin World position of the new entity.
	 * @param vecAngles Initial orientation of the new entity.
	 * @param pentOwner Optional owning engine entity.
	 * @return Pointer to the newly created entity, or `nullptr` if creation
	 * failed.
	 *
	 * @note This is the preferred way to create game entities. Do not allocate
	 * entities using the standard C++ `new` operator.
	 */
	static CBaseEntity* Create(
		char* szName,
		const Vector& vecOrigin,
		const Vector& vecAngles,
		edict_t* pentOwner = nullptr
	);

	/**
	 * @brief Attempts to place the entity into a prone state.
	 *
	 * Overridden by entities that support becoming prone.
	 *
	 * @return `true` if the entity successfully became prone.
	 */
	virtual BOOL FBecomeProne() { return FALSE; }

	/**
	 * @brief Returns the engine entity associated with this object.
	 *
	 * @return Pointer to this entity's `edict_t`.
	 */
	edict_t* edict() { return ENT(pev); }

	/**
	 * @brief Returns this entity's engine offset.
	 *
	 * @return Engine offset (`EOFFSET`) identifying this entity.
	 */
	EOFFSET eoffset() { return OFFSET(pev); }

	/**
	 * @brief Returns this entity's index.
	 *
	 * The entity index uniquely identifies this entity within the current map.
	 *
	 * @return Engine entity index.
	 */
	int entindex() { return ENTINDEX(edict()); }

	/**
	 * @brief Returns the center point of the entity.
	 *
	 * Computes the geometric center of the entity using its absolute bounding
	 * box.
	 *
	 * @return World-space center of the entity.
	 */
	virtual Vector Center() { return (pev->absmax + pev->absmin) * 0.5; }

	/**
	 * @brief Returns the entity's eye position.
	 *
	 * This position is typically used as the origin for line-of-sight checks
	 * and ranged attacks.
	 *
	 * @return World-space position of the entity's eyes.
	 */
	virtual Vector EyePosition() { return pev->origin + pev->view_ofs; }

	/**
	 * @brief Returns the entity's ear position.
	 *
	 * Used by the AI sound system when determining whether the entity can hear
	 * nearby sounds. By default, this is the same as the eye position.
	 *
	 * @return World-space position of the entity's ears.
	 */
	virtual Vector EarPosition() { return pev->origin + pev->view_ofs; }

	/**
	 * @brief Returns the preferred point to aim at when attacking this entity.
	 *
	 * Override this to provide a more suitable target location for entities
	 * with unusual shapes or animations.
	 *
	 * @param posSrc Position from which the attack originates.
	 * @return World-space position that attackers should aim toward.
	 */
	virtual Vector BodyTarget(const Vector& posSrc) { return Center(); }

	/**
	 * @brief Returns the amount of light affecting this entity.
	 *
	 * Queries the engine for the current illumination level at the entity's
	 * position. This is commonly used by AI to determine visibility.
	 *
	 * @return Illumination value reported by the engine.
	 */
	virtual int Illumination() { return GETENTITYILLUM(ENT(pev)); }

	/**
	 * @brief Determines whether another entity is visible.
	 *
	 * Performs a line-of-sight check between this entity and the specified
	 * target entity.
	 *
	 * @param pEntity Entity to test visibility against.
	 * @return `true` if the target entity is visible.
	 */
	virtual BOOL FVisible(CBaseEntity* pEntity);

	/**
	 * @brief Determines whether a world position is visible.
	 *
	 * Performs a line-of-sight check between this entity and the specified
	 * position in world space.
	 *
	 * @param vecOrigin World-space position to test.
	 * @return `true` if the position is visible.
	 */
	virtual BOOL FVisible(const Vector& vecOrigin);

	// We use these variables to store each ammo count.

	/// @brief Amount of 9mm ammunition currently held.
	int ammo_9mm;

	/// @brief Amount of .357 Magnum ammunition currently held.
	int ammo_357;

	/// @brief Amount of crossbow bolts currently held.
	int ammo_bolts;

	/// @brief Amount of buckshot ammunition currently held.
	int ammo_buckshot;

	/// @brief Amount of RPG rockets currently held.
	int ammo_rockets;

	/// @brief Amount of uranium ammunition used by the Gauss Gun and Egon.
	int ammo_uranium;

	/// @brief Amount of Hornet ammunition currently held.
	int ammo_hornets;

	/// @brief Amount of M203 grenade ammunition currently held.
	int ammo_argrens;

	// Special stuff for grenades and satchels.

	/**
	 * @brief Time at which a grenade throw was initiated.
	 *
	 * Used while preparing grenade throw animations.
	 */
	float m_flStartThrow;

	/**
	 * @brief Time at which a grenade should be released.
	 *
	 * Determines when the thrown grenade leaves the player's hand.
	 */
	float m_flReleaseThrow;

	/**
	 * @brief Indicates whether a charged explosive is ready.
	 *
	 * Used by weapons such as satchel charges.
	 */
	int m_chargeReady;

	/**
	 * @brief Indicates whether the primary attack is currently active.
	 *
	 * Used internally by several weapon implementations.
	 */
	int m_fInAttack;

	/**
	 * @brief Firing states for the Egon weapon.
	 */
	enum class EgonFirestate
	{
		/// @brief The weapon is not firing.
		Off,

		/// @brief The weapon is charging or actively firing.
		Charge
	};

	/**
	 * @brief Current firing state of the Egon weapon.
	 *
	 * Stores one of the `EgonFirestate` values.
	 *
	 * @note this is an `int` for compatibility with the original codebase, but it should be an `EgonFirestate` enum for type safety and clarity. This will be addressed in a future refactor.
	 * @todo TODO-001: Change `m_fireState` to be of type `EgonFirestate` instead of `int` for better type safety and clarity.
	 */
	int m_fireState{ static_cast<int>(EgonFirestate::Off) };
	// TODO-001: EgonFirestate m_fireState{ EgonFirestate::Off };
};

/**
 * @brief Entity callback registration macros.
 *
 * These macros assign callback member functions for an entity's Think, Touch,
 * Use, and Blocked handlers.
 *
 * @details
 * Callback member functions are explicitly cast to the corresponding
 * `CBaseEntity` member function pointer type. This is a legacy workaround
 * because pointer-to-member function types of derived classes are not
 * implicitly convertible to those of their base class.
 *
 * In debug builds, callbacks are assigned through helper functions that also
 * record the callback name for debugging and export validation. In release
 * builds, the callback pointers are assigned directly for minimal overhead.
 */
#ifdef _DEBUG

 /**
  * @brief Registers the Think callback.
  *
  * @param a Pointer to the Think member function.
  */
#define SetThink( a ) \
    ThinkSet( static_cast<void (CBaseEntity::*)(void)>(a), #a )

  /**
   * @brief Registers the Touch callback.
   *
   * @param a Pointer to the Touch member function.
   */
#define SetTouch( a ) \
    TouchSet( static_cast<void (CBaseEntity::*)(CBaseEntity*)>(a), #a )

   /**
	* @brief Registers the Use callback.
	*
	* @param a Pointer to the Use member function.
	*/
#define SetUse( a ) \
    UseSet( static_cast<void (CBaseEntity::*)(CBaseEntity*, CBaseEntity*, USE_TYPE, float)>(a), #a )

	/**
	 * @brief Registers the Blocked callback.
	 *
	 * @param a Pointer to the Blocked member function.
	 */
#define SetBlocked( a ) \
    BlockedSet( static_cast<void (CBaseEntity::*)(CBaseEntity*)>(a), #a )

#else

 /**
  * @brief Assigns the Think callback.
  *
  * @param a Pointer to the Think member function.
  */
#define SetThink( a ) \
    m_pfnThink = static_cast<void (CBaseEntity::*)(void)>(a)

  /**
   * @brief Assigns the Touch callback.
   *
   * @param a Pointer to the Touch member function.
   */
#define SetTouch( a ) \
    m_pfnTouch = static_cast<void (CBaseEntity::*)(CBaseEntity*)>(a)

   /**
	* @brief Assigns the Use callback.
	*
	* @param a Pointer to the Use member function.
	*/
#define SetUse( a ) \
    m_pfnUse = static_cast<void (CBaseEntity::*)(CBaseEntity*, CBaseEntity*, USE_TYPE, float)>(a)

	/**
	 * @brief Assigns the Blocked callback.
	 *
	 * @param a Pointer to the Blocked member function.
	 */
#define SetBlocked( a ) \
    m_pfnBlocked = static_cast<void (CBaseEntity::*)(CBaseEntity*)>(a)

#endif

	 /**
	  * @brief Base class for non-solid point entities.
	  *
	  * A point entity has an origin in the world but typically has no model,
	  * collision bounds, or physical presence. It serves as the base class for
	  * many utility and logic entities, such as targets, triggers, and spawn
	  * points.
	  */
class CPointEntity : public CBaseEntity
{
public:

	/**
	 * @brief Spawns the point entity.
	 *
	 * Performs any initialization required when the entity is created.
	 */
	void Spawn();

	/**
	 * @brief Returns the entity's capability flags.
	 *
	 * @details
	 * Point entities do not persist across level transitions. This override
	 * clears the `FCAP_ACROSS_TRANSITION` capability inherited from
	 * `CBaseEntity`.
	 *
	 * @return The entity capability flags with `FCAP_ACROSS_TRANSITION`
	 * removed.
	 */
	int ObjectCaps() override
	{
		return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION;
	}
};


/**
 * @brief Stores the sounds and sentence groups used by locked entities.
 *
 * Used by entities such as doors and buttons to manage the audio played when
 * a player attempts to interact with them while they are locked or unlocked.
 *
 * @details
 * This structure also tracks sentence playback state and cooldown timers to
 * prevent sounds and spoken lines from repeating too frequently.
 *
 * @todo Replace this legacy C-style typedef with a modern C++ struct.
 */
typedef struct locksounds
{
	/**
	 * @brief Sound played when the entity is locked.
	 */
	string_t sLockedSound;

	/**
	 * @brief Sentence group played when the entity is locked.
	 */
	string_t sLockedSentence;

	/**
	 * @brief Sound played when the entity is unlocked.
	 */
	string_t sUnlockedSound;

	/**
	 * @brief Sentence group played when the entity is unlocked.
	 */
	string_t sUnlockedSentence;

	/**
	 * @brief Index of the next locked sentence to play.
	 */
	int iLockedSentence;

	/**
	 * @brief Index of the next unlocked sentence to play.
	 */
	int iUnlockedSentence;

	/**
	 * @brief Minimum delay between consecutive locked or unlocked sounds.
	 */
	float flwaitSound;

	/**
	 * @brief Minimum delay between consecutive sentence playback.
	 */
	float flwaitSentence;

	/**
	 * @brief Indicates that all locked sentences have been played.
	 */
	BYTE bEOFLocked;

	/**
	 * @brief Indicates that all unlocked sentences have been played.
	 */
	BYTE bEOFUnlocked;

} locksound_t;

/**
 * @brief Plays the appropriate lock or unlock sound for an entity.
 *
 * Determines which sound effect or sentence group should be played based on
 * the entity's lock state and updates the associated playback timers and
 * sentence indices.
 *
 * @param pev Pointer to the entity's variables.
 * @param pls Pointer to the lock sound configuration and playback state.
 * @param flocked Non-zero if the entity is currently locked; otherwise zero.
 * @param fbutton Non-zero if the entity is a button; otherwise it is treated
 * as a door.
 */
void PlayLockSounds(entvars_t* pev, locksound_t* pls, int flocked, int fbutton);

/**
 * @brief Maximum number of target entities supported by a single multi_manager.
 *
 * A `multi_manager` may trigger up to this many target entities in sequence.
 *
 * @note
 * This is the original limit used by the Half-Life SDK.
 */
#define MAX_MULTI_TARGETS 16

 /**
  * @brief Maximum number of targets supported by a multi_source.
  *
  * A `multi_source` can monitor up to this many input entities before it
  * activates its target.
 */
#define MS_MAX_TARGETS 32

 /**
  * @brief Logical AND gate for multiple entity inputs.
  *
  * A `multi_source` monitors a collection of entities and only becomes
  * triggered once all registered inputs have been activated. It is commonly
  * used to implement puzzles or mechanisms that require multiple switches,
  * buttons, or other entities to be activated before an action occurs.
  */
class CMultiSource : public CPointEntity
{
public:

	/**
	 * @brief Spawns the multi-source entity.
	 */
	void Spawn();

	/**
	 * @brief Processes a key-value pair from the map.
	 *
	 * @param pkvd Pointer to the key-value data.
	 */
	void KeyValue(KeyValueData* pkvd);

	/**
	 * @brief Handles activation of one of the multi-source's inputs.
	 *
	 * Updates the activation state of the calling entity and triggers the
	 * multi-source if all registered inputs have been activated.
	 *
	 * @param pActivator Entity responsible for the activation.
	 * @param pCaller Entity invoking this callback.
	 * @param useType Type of use interaction.
	 * @param value Additional use value.
	 */
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	/**
	 * @brief Returns the entity's capability flags.
	 *
	 * @return The capabilities inherited from `CPointEntity` with
	 * `FCAP_MASTER` added.
	 */
	int ObjectCaps(void)
	{
		return CPointEntity::ObjectCaps() | FCAP_MASTER;
	}

	/**
	 * @brief Determines whether the multi-source is currently triggered.
	 *
	 * @param pActivator Entity requesting the trigger state.
	 * @return `TRUE` if all required inputs have been activated; otherwise `FALSE`.
	 */
	BOOL IsTriggered(CBaseEntity* pActivator);

	/**
	 * @brief Registers the entity with its input targets.
	 *
	 * Called after all entities have spawned to resolve entity references.
	 */
	void EXPORT Register(void);

	/**
	 * @brief Saves the entity's persistent state.
	 *
	 * @param save Save-game writer.
	 * @return Non-zero on success.
	 */
	virtual int Save(CSave& save);

	/**
	 * @brief Restores the entity's persistent state.
	 *
	 * @param restore Save-game reader.
	 * @return Non-zero on success.
	 */
	virtual int Restore(CRestore& restore);

	/**
	 * @brief Save/restore field descriptions.
	 */
	static TYPEDESCRIPTION m_SaveData[];

	/**
	 * @brief Registered input entities.
	 */
	EHANDLE m_rgEntities[MS_MAX_TARGETS];

	/**
	 * @brief Activation state for each registered input.
	 */
	int m_rgTriggered[MS_MAX_TARGETS];

	/**
	 * @brief Number of registered input entities.
	 */
	int m_iTotal;

	/**
	 * @brief Name of the associated global state.
	 *
	 * If specified, the multi-source is only considered active when the
	 * referenced global state allows it.
	 */
	string_t m_globalstate;
};


/**
 * @brief Base class for entities that support delayed actions.
 *
 * Extends `CBaseEntity` with support for delayed target activation and
 * optional kill targets. Many trigger and logic entities inherit from this
 * class to gain common delay functionality.
 */
class CBaseDelay : public CBaseEntity
{
public:

	/**
	 * @brief Delay, in seconds, before targets are activated.
	 */
	float m_flDelay;

	/**
	 * @brief Target name of entities to remove when activated.
	 */
	int m_iszKillTarget;

	/**
	 * @brief Processes key-value pairs from the map.
	 *
	 * @param pkvd Pointer to the key-value data.
	 */
	virtual void KeyValue(KeyValueData* pkvd);

	/**
	 * @brief Saves the entity's persistent state.
	 *
	 * @param save Save-game writer.
	 * @return Non-zero on success.
	 */
	virtual int Save(CSave& save);

	/**
	 * @brief Restores the entity's persistent state.
	 *
	 * @param restore Save-game reader.
	 * @return Non-zero on success.
	 */
	virtual int Restore(CRestore& restore);

	/**
	 * @brief Save/restore field descriptions.
	 */
	static TYPEDESCRIPTION m_SaveData[];

	/**
	 * @brief Activates this entity's targets.
	 *
	 * If a delay has been specified, activation is deferred until
	 * `DelayThink()` is executed. Any configured kill target is processed
	 * before the target entities are fired.
	 *
	 * @param pActivator Entity responsible for the activation.
	 * @param useType Type of use interaction.
	 * @param value Additional use value.
	 */
	void SUB_UseTargets(CBaseEntity* pActivator, USE_TYPE useType, float value);

	/**
	 * @brief Executes delayed target activation.
	 *
	 * Called after `m_flDelay` has elapsed to activate the entity's targets.
	 */
	void EXPORT DelayThink(void);
};


class CBaseAnimating : public CBaseDelay
{
public:
	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);

	static	TYPEDESCRIPTION m_SaveData[];

	// Basic Monster Animation functions
	float StudioFrameAdvance(float flInterval = 0.0); // accumulate animation frame time from last time called until now
	int	 GetSequenceFlags(void);
	int  LookupActivity(int activity);
	int  LookupActivityHeaviest(int activity);
	int  LookupSequence(const char* label);
	void ResetSequenceInfo();
	void DispatchAnimEvents(float flFutureInterval = 0.1); // Handle events that have happend since last time called up until X seconds into the future
	virtual void HandleAnimEvent(MonsterEvent_t* pEvent) {};
	float SetBoneController(int iController, float flValue);
	void InitBoneControllers(void);
	float SetBlending(int iBlender, float flValue);
	void GetBonePosition(int iBone, Vector& origin, Vector& angles);
	void GetAutomovement(Vector& origin, Vector& angles, float flInterval = 0.1);
	int  FindTransition(int iEndingSequence, int iGoalSequence, int* piDir);
	void GetAttachment(int iAttachment, Vector& origin, Vector& angles);
	void SetBodygroup(int iGroup, int iValue);
	int GetBodygroup(int iGroup);
	int ExtractBbox(int sequence, float* mins, float* maxs);
	void SetSequenceBox(void);

	// animation needs
	float				m_flFrameRate;		// computed FPS for current sequence
	float				m_flGroundSpeed;	// computed linear movement rate for current sequence
	float				m_flLastEventCheck;	// last time the event list was checked
	BOOL				m_fSequenceFinished;// flag set when StudioAdvanceFrame moves across a frame boundry
	BOOL				m_fSequenceLoops;	// true if the sequence loops
};


//
// generic Toggle entity.
//
#define	SF_ITEM_USE_ONLY	256 //  ITEM_USE_ONLY = BUTTON_USE_ONLY = DOOR_USE_ONLY!!! 

class CBaseToggle : public CBaseAnimating
{
public:
	void				KeyValue(KeyValueData* pkvd);

	TOGGLE_STATE		m_toggle_state;
	float				m_flActivateFinished;//like attack_finished, but for doors
	float				m_flMoveDistance;// how far a door should slide or rotate
	float				m_flWait;
	float				m_flLip;
	float				m_flTWidth;// for plats
	float				m_flTLength;// for plats

	Vector				m_vecPosition1;
	Vector				m_vecPosition2;
	Vector				m_vecAngle1;
	Vector				m_vecAngle2;

	int					m_cTriggersLeft;		// trigger_counter only, # of activations remaining
	float				m_flHeight;
	EHANDLE				m_hActivator;
	void (CBaseToggle::* m_pfnCallWhenMoveDone)(void);
	Vector				m_vecFinalDest;
	Vector				m_vecFinalAngle;

	int					m_bitsDamageInflict;	// DMG_ damage type that the door or tigger does

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);

	static	TYPEDESCRIPTION m_SaveData[];

	virtual int		GetToggleState(void) { return m_toggle_state; }
	virtual float	GetDelay(void) { return m_flWait; }

	// common member functions
	void LinearMove(Vector	vecDest, float flSpeed);
	void EXPORT LinearMoveDone(void);
	void AngularMove(Vector vecDestAngle, float flSpeed);
	void EXPORT AngularMoveDone(void);
	BOOL IsLockedByMaster(void);

	virtual CBaseToggle* MyTogglePointer(void) { return this; }

	// monsters use this, but so could buttons for instance
	virtual void PlaySentence(const char* pszSentence, float duration, float volume, float attenuation);
	virtual void PlayScriptedSentence(const char* pszSentence, float duration, float volume, float attenuation, BOOL bConcurrent, CBaseEntity* pListener);
	virtual void SentenceStop(void);
	virtual BOOL IsAllowedToSpeak() { return FALSE; }

	static float		AxisValue(int flags, const Vector& angles);
	static void			AxisDir(entvars_t* pev);
	static float		AxisDelta(int flags, const Vector& angle1, const Vector& angle2);

	string_t m_sMaster;		// If this button has a master switch, this is the targetname.
	// A master switch must be of the multisource type. If all 
	// of the switches in the multisource have been triggered, then
	// the button will be allowed to operate. Otherwise, it will be
	// deactivated.
};
#define SetMoveDone( a ) m_pfnCallWhenMoveDone = static_cast <void (CBaseToggle::*)(void)> (a)


// people gib if their health is <= this at the time of death
#define	GIB_HEALTH_VALUE	-30

#define	ROUTE_SIZE			8 // how many waypoints a monster can store at one time
#define MAX_OLD_ENEMIES		4 // how many old enemies to remember

#define	bits_CAP_DUCK			( 1 << 0 )// crouch
#define	bits_CAP_JUMP			( 1 << 1 )// jump/leap
#define bits_CAP_STRAFE			( 1 << 2 )// strafe ( walk/run sideways)
#define bits_CAP_SQUAD			( 1 << 3 )// can form squads
#define	bits_CAP_SWIM			( 1 << 4 )// proficiently navigate in water
#define bits_CAP_CLIMB			( 1 << 5 )// climb ladders/ropes
#define bits_CAP_USE			( 1 << 6 )// open doors/push buttons/pull levers
#define bits_CAP_HEAR			( 1 << 7 )// can hear forced sounds
#define bits_CAP_AUTO_DOORS		( 1 << 8 )// can trigger auto doors
#define bits_CAP_OPEN_DOORS		( 1 << 9 )// can open manual doors
#define bits_CAP_TURN_HEAD		( 1 << 10)// can turn head, always bone controller 0

#define bits_CAP_RANGE_ATTACK1	( 1 << 11)// can do a range attack 1
#define bits_CAP_RANGE_ATTACK2	( 1 << 12)// can do a range attack 2
#define bits_CAP_MELEE_ATTACK1	( 1 << 13)// can do a melee attack 1
#define bits_CAP_MELEE_ATTACK2	( 1 << 14)// can do a melee attack 2

#define bits_CAP_FLY			( 1 << 15)// can fly, move all around

#define bits_CAP_DOORS_GROUP    (bits_CAP_USE | bits_CAP_AUTO_DOORS | bits_CAP_OPEN_DOORS)

// used by suit voice to indicate damage sustained and repaired type to player

// instant damage

#define DMG_GENERIC			0			// generic damage was done
#define DMG_CRUSH			(1 << 0)	// crushed by falling or moving object
#define DMG_BULLET			(1 << 1)	// shot
#define DMG_SLASH			(1 << 2)	// cut, clawed, stabbed
#define DMG_BURN			(1 << 3)	// heat burned
#define DMG_FREEZE			(1 << 4)	// frozen
#define DMG_FALL			(1 << 5)	// fell too far
#define DMG_BLAST			(1 << 6)	// explosive blast damage
#define DMG_CLUB			(1 << 7)	// crowbar, punch, headbutt
#define DMG_SHOCK			(1 << 8)	// electric shock
#define DMG_SONIC			(1 << 9)	// sound pulse shockwave
#define DMG_ENERGYBEAM		(1 << 10)	// laser or other high energy beam 
#define DMG_NEVERGIB		(1 << 12)	// with this bit OR'd in, no damage type will be able to gib victims upon death
#define DMG_ALWAYSGIB		(1 << 13)	// with this bit OR'd in, any damage type can be made to gib victims upon death.
#define DMG_DROWN			(1 << 14)	// Drowning
// time-based damage
#define DMG_TIMEBASED		(~(0x3fff))	// mask for time-based damage

#define DMG_PARALYZE		(1 << 15)	// slows affected creature down
#define DMG_NERVEGAS		(1 << 16)	// nerve toxins, very bad
#define DMG_POISON			(1 << 17)	// blood poisioning
#define DMG_RADIATION		(1 << 18)	// radiation exposure
#define DMG_DROWNRECOVER	(1 << 19)	// drowning recovery
#define DMG_ACID			(1 << 20)	// toxic chemicals or acid burns
#define DMG_SLOWBURN		(1 << 21)	// in an oven
#define DMG_SLOWFREEZE		(1 << 22)	// in a subzero freezer
#define DMG_MORTAR			(1 << 23)	// Hit by air raid (done to distinguish grenade from mortar)

// these are the damage types that are allowed to gib corpses
#define DMG_GIB_CORPSE		( DMG_CRUSH | DMG_FALL | DMG_BLAST | DMG_SONIC | DMG_CLUB )

// these are the damage types that have client hud art
#define DMG_SHOWNHUD		(DMG_POISON | DMG_ACID | DMG_FREEZE | DMG_SLOWFREEZE | DMG_DROWN | DMG_BURN | DMG_SLOWBURN | DMG_NERVEGAS | DMG_RADIATION | DMG_SHOCK)

// NOTE: tweak these values based on gameplay feedback:

#define PARALYZE_DURATION	2		// number of 2 second intervals to take damage
#define PARALYZE_DAMAGE		1.0		// damage to take each 2 second interval

#define NERVEGAS_DURATION	2
#define NERVEGAS_DAMAGE		5.0

#define POISON_DURATION		5
#define POISON_DAMAGE		2.0

#define RADIATION_DURATION	2
#define RADIATION_DAMAGE	1.0

#define ACID_DURATION		2
#define ACID_DAMAGE			5.0

#define SLOWBURN_DURATION	2
#define SLOWBURN_DAMAGE		1.0

#define SLOWFREEZE_DURATION	2
#define SLOWFREEZE_DAMAGE	1.0


#define	itbd_Paralyze		0		
#define	itbd_NerveGas		1
#define	itbd_Poison			2
#define	itbd_Radiation		3
#define	itbd_DrownRecover	4
#define	itbd_Acid			5
#define	itbd_SlowBurn		6
#define	itbd_SlowFreeze		7
#define CDMG_TIMEBASED		8

// when calling KILLED(), a value that governs gib behavior is expected to be 
// one of these three values
#define GIB_NORMAL			0// gib if entity was overkilled
#define GIB_NEVER			1// never gib, no matter how much death damage is done ( freezing, etc )
#define GIB_ALWAYS			2// always gib ( Houndeye Shock, Barnacle Bite )

class CBaseMonster;
class CCineMonster;
class CSound;

#include "basemonster.h"


char* ButtonSound(int sound);				// get string of button sound number


//
// Generic Button
//
class CBaseButton : public CBaseToggle
{
public:
	void Spawn(void);
	virtual void Precache(void);
	void RotSpawn(void);
	virtual void KeyValue(KeyValueData* pkvd);

	void ButtonActivate();
	void SparkSoundCache(void);

	void EXPORT ButtonShot(void);
	void EXPORT ButtonTouch(CBaseEntity* pOther);
	void EXPORT ButtonSpark(void);
	void EXPORT TriggerAndWait(void);
	void EXPORT ButtonReturn(void);
	void EXPORT ButtonBackHome(void);
	void EXPORT ButtonUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	virtual int		TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);

	enum BUTTON_CODE { BUTTON_NOTHING, BUTTON_ACTIVATE, BUTTON_RETURN };
	BUTTON_CODE	ButtonResponseToTouch(void);

	static	TYPEDESCRIPTION m_SaveData[];
	// Buttons that don't take damage can be IMPULSE used
	virtual int	ObjectCaps(void) { return (CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | (pev->takedamage ? 0 : FCAP_IMPULSE_USE); }
	virtual BOOL IsAllowedToSpeak() { return TRUE; }

	BOOL	m_fStayPushed;	// button stays pushed in until touched again?
	BOOL	m_fRotating;		// a rotating button?  default is a sliding button.

	string_t m_strChangeTarget;	// if this field is not nullptr, this is an index into the engine string array.
	// when this button is touched, it's target entity's TARGET field will be set
	// to the button's ChangeTarget. This allows you to make a func_train switch paths, etc.

	locksound_t m_ls;			// door lock sounds

	BYTE	m_bLockedSound;		// ordinals from entity selection
	BYTE	m_bLockedSentence;
	BYTE	m_bUnlockedSound;
	BYTE	m_bUnlockedSentence;
	int		m_sounds;
};

//
// Weapons 
//

#define	BAD_WEAPON 0x00007FFF

//
// Converts a entvars_t * to a class pointer
// It will allocate the class and entity if necessary
//
template <class T> T* GetClassPtr(T* a)
{
	entvars_t* pev = (entvars_t*)a;

	// allocate entity if necessary
	if (pev == nullptr)
		pev = VARS(CREATE_ENTITY());

	// get the private data
	a = (T*)GET_PRIVATE(ENT(pev));

	if (a == nullptr)
	{
		// allocate private data 
		a = new(pev) T;
		a->pev = pev;
	}
	return a;
}


/*
bit_PUSHBRUSH_DATA | bit_TOGGLE_DATA
bit_MONSTER_DATA
bit_DELAY_DATA
bit_TOGGLE_DATA | bit_DELAY_DATA | bit_MONSTER_DATA
bit_PLAYER_DATA | bit_MONSTER_DATA
bit_MONSTER_DATA | CYCLER_DATA
bit_LIGHT_DATA
path_corner_data
bit_MONSTER_DATA | wildcard_data
bit_MONSTER_DATA | bit_GROUP_DATA
boid_flock_data
boid_data
CYCLER_DATA
bit_ITEM_DATA
bit_ITEM_DATA | func_hud_data
bit_TOGGLE_DATA | bit_ITEM_DATA
EOFFSET
env_sound_data
env_sound_data
push_trigger_data
*/

#define TRACER_FREQ		4			// Tracers fire every 4 bullets

typedef struct _SelAmmo
{
	BYTE	Ammo1Type;
	BYTE	Ammo1;
	BYTE	Ammo2Type;
	BYTE	Ammo2;
} SelAmmo;


// this moved here from world.cpp, to allow classes to be derived from it
//=======================
// CWorld
//
// This spawns first when each level begins.
//=======================
class CWorld : public CBaseEntity
{
public:
	void Spawn(void);
	void Precache(void);
	void KeyValue(KeyValueData* pkvd);
};
