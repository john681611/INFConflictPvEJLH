enum JLH_DCF_VehiclePatrolState
{
	PATROLLING = 0,
	COMBAT_PAUSED = 1,
	DISMOUNTED_FALLBACK = 2
}

[ComponentEditorProps(category: "JLH Dynamic Conflict Framework/Nodes", description: "Standalone JLH vehicle patrol node with owned vehicle selection, crew mounting, child waypoint routing, cleanup, and lifecycle controls.")]
class JLH_VehiclePatrolNodeComponentClass : ScriptComponentClass
{
}

class JLH_DCF_VehicleMountStats
{
	int CrewSeen;
	int SeatsFilled;
	int CargoSeatsFilled;
	int TurretSeatsFilled;
	int RemovedUnassigned;
	bool DriverMounted;
	bool GunnerMounted;
	bool HasCompartmentAccess;
	IEntity DriverEntity;
	IEntity GunnerEntity;
	string FailureReason;
	ref array<IEntity> PassengerEntities = {};

	void Reset()
	{
		CrewSeen = 0;
		SeatsFilled = 0;
		CargoSeatsFilled = 0;
		TurretSeatsFilled = 0;
		RemovedUnassigned = 0;
		DriverMounted = false;
		GunnerMounted = false;
		HasCompartmentAccess = false;
		DriverEntity = null;
		GunnerEntity = null;
		FailureReason = "";
		PassengerEntities.Clear();
	}
}

class JLH_DCF_VehiclePatrolRuntime
{
	int PatrolId;
	IEntity VehicleEntity;
	SCR_AIGroup CrewGroup;
	IEntity DriverEntity;
	IEntity GunnerEntity;
	ResourceName VehiclePrefab;
	ResourceName CrewPrefab;
	int SpawnTick;
	int LastCombatTick;
	int DriverLostSinceTick;
	int RouteResumeReadyTick;
	int LastMountedRecoveryAttemptTick;
	int MountedRecoveryAttempts;
	int LastMountedRecoveryLogTick;
	bool m_bCleared;
	int m_iClearedTick;
	bool AttackCapable;
	bool RoutePausedForRecovery;
	JLH_DCF_VehiclePatrolState PatrolState;
	ref array<IEntity> PassengerEntities = {};
	ref array<IEntity> RouteWaypoints = {};
	ref array<IEntity> QueuedDeadBodies = {};

	void Init(int patrolId, IEntity vehicle, SCR_AIGroup crewGroup, IEntity driver, IEntity gunner, ResourceName vehiclePrefab, ResourceName crewPrefab, array<IEntity> passengerEntities, array<IEntity> routeWaypoints)
	{
		PatrolId = patrolId;
		VehicleEntity = vehicle;
		CrewGroup = crewGroup;
		DriverEntity = driver;
		GunnerEntity = gunner;
		VehiclePrefab = vehiclePrefab;
		CrewPrefab = crewPrefab;
		SpawnTick = System.GetTickCount();
		LastCombatTick = 0;
		DriverLostSinceTick = 0;
		RouteResumeReadyTick = 0;
		LastMountedRecoveryAttemptTick = 0;
		MountedRecoveryAttempts = 0;
		LastMountedRecoveryLogTick = 0;
		m_bCleared = false;
		m_iClearedTick = 0;
		AttackCapable = gunner != null;
		RoutePausedForRecovery = false;
		PatrolState = JLH_DCF_VehiclePatrolState.PATROLLING;
		QueuedDeadBodies.Clear();
		PassengerEntities.Clear();
		if (passengerEntities)
		{
			foreach (IEntity passenger : passengerEntities)
			{
				if (passenger && !PassengerEntities.Contains(passenger))
					PassengerEntities.Insert(passenger);
			}
		}

		RouteWaypoints.Clear();
		if (routeWaypoints)
		{
			foreach (IEntity waypoint : routeWaypoints)
			{
				if (waypoint)
					RouteWaypoints.Insert(waypoint);
			}
		}
	}
}

class JLH_DCF_VehiclePatrolPendingMount
{
	IEntity VehicleEntity;
	SCR_AIGroup CrewGroup;
	ResourceName VehiclePrefab;
	ResourceName CrewPrefab;
	int WaypointCount;
	int ReadyTick;

	void Init(IEntity vehicle, SCR_AIGroup crewGroup, ResourceName vehiclePrefab, ResourceName crewPrefab, int waypointCount, int delayMs)
	{
		VehicleEntity = vehicle;
		CrewGroup = crewGroup;
		VehiclePrefab = vehiclePrefab;
		CrewPrefab = crewPrefab;
		WaypointCount = waypointCount;
		ReadyTick = System.GetTickCount() + delayMs;
	}
}

class JLH_DCF_VehicleWreckCleanupState
{
	IEntity VehicleEntity;
	vector LastPosition;
	int DestroyedTick;
	int NextAttemptTick;

	void Init(IEntity vehicle, int now, int graceMs)
	{
		VehicleEntity = vehicle;
		DestroyedTick = now;
		NextAttemptTick = now + graceMs;
		LastPosition = "0 0 0";
		if (vehicle)
			LastPosition = vehicle.GetOrigin();
	}
}

class JLH_DCF_VehicleCorpseCleanupState
{
	int PatrolId;
	IEntity CorpseEntity;
	vector LastPosition;
	int DeathTick;
	int RecentCombatTick;
	int NextAttemptTick;
	bool VehiclePatrolOwned;

	void Init(IEntity corpse, int now, int graceMs, int patrolId, int recentCombatTick)
	{
		PatrolId = patrolId;
		CorpseEntity = corpse;
		DeathTick = now;
		RecentCombatTick = recentCombatTick;
		NextAttemptTick = now + graceMs;
		VehiclePatrolOwned = true;
		LastPosition = "0 0 0";
		if (corpse)
			LastPosition = corpse.GetOrigin();
	}
}

class JLH_VehiclePatrolNodeComponent : ScriptComponent
{
	protected static const string SYSTEM_NAME = "VEHICLE";
	protected static const int PENDING_MOUNT_DELAY_MS = 1500;
	protected static const int POST_MOUNT_VALIDATION_DELAY_MS = 2000;
	protected static const int SPAWN_RANGE_LOG_THROTTLE_MS = 30000;
	protected static const int DRIVER_LOST_TIMEOUT_MS = 45000;
	protected static const int POST_COMBAT_QUIET_PERIOD_MS = 15000;
	protected static const int POST_COMBAT_ROUTE_RESUME_GRACE_MS = 3000;
	protected static const int MOUNTED_RECOVERY_RETRY_MS = 5000;
	protected static const int MOUNTED_RECOVERY_LOG_THROTTLE_MS = 15000;
	protected static const int WRECK_CLEANUP_GRACE_MS = 120000;
	protected static const int WRECK_CLEANUP_RETRY_MS = 30000;
	protected static const float WRECK_CLEANUP_PLAYER_NEAR_DISTANCE = 350.0;
	protected static const float WRECK_CLEANUP_LOS_DISTANCE = 900.0;
	protected static const int CORPSE_CLEANUP_RETRY_MS = 20000;
	protected static const float CORPSE_CLEANUP_LOS_DISTANCE = 500.0;
	protected static const ResourceName DEFAULT_USSR_VEHICLE_CREW_GROUP_PREFAB = "{D4CF0B2B840CF001}Prefabs/Groups/JLH_DCF/JLH_USSR_VehicleGroup.et";
	protected static const ResourceName DEFAULT_US_VEHICLE_CREW_GROUP_PREFAB = "{101C1DACCD535E45}Prefabs/Groups/JLH_DCF/JLH_US_VehicleGroup.et";

	[Attribute("0", UIWidgets.CheckBox, "Enable detailed registration, lifecycle, selection, spawn, mount, route, despawn, and cleanup logs for this node.", category: "Basic")]
	protected bool m_bDebugLogging;

	[Attribute("900", UIWidgets.Hidden, "Internal spawn distance fallback. Vehicle Patrol spawn safety is handled automatically by the framework.", "0 10000 1", category: "Internal")]
	protected float m_fSpawnDistance;

	[Attribute("1300", UIWidgets.Hidden, "Internal despawn distance fallback. Vehicle Patrol cleanup safety is handled automatically by the framework.", "0 15000 1", category: "Internal")]
	protected float m_fDespawnDistance;

	[Attribute("500", UIWidgets.Hidden, "Minimum distance from any live deployed player required before a Vehicle Patrol node may spawn.", "0 5000 1", category: "Internal")]
	protected float m_fSpawnMinimumPlayerDistance;

	[Attribute("900", UIWidgets.Hidden, "Vehicle Patrol spawn is blocked when any live deployed player has line of sight to the spawn point inside this distance.", "0 5000 1", category: "Internal")]
	protected float m_fSpawnLineOfSightRejectDistance;

	[Attribute("500", UIWidgets.Hidden, "Live Vehicle Patrol cleanup is blocked while any live deployed player is this close to the current runtime patrol position.", "0 5000 1", category: "Internal")]
	protected float m_fLiveCleanupPlayerSafeDistance;

	[Attribute("900", UIWidgets.Hidden, "Live Vehicle Patrol cleanup is blocked when any live deployed player has line of sight to the current runtime patrol position inside this distance.", "0 5000 1", category: "Internal")]
	protected float m_fLiveCleanupLineOfSightRejectDistance;

	[Attribute("300", UIWidgets.EditBox, "Seconds before this node may spawn a replacement patrol after destruction, cleanup, failure, or despawn.", "0 7200 1", category: "Basic")]
	protected int m_iRespawnPeriod;

	[Attribute("1", UIWidgets.CheckBox, "Allow this node to spawn as soon as it initializes when normal spawn conditions are valid.", category: "Basic")]
	protected bool m_bSpawnOnStartup;

	[Attribute("", UIWidgets.EditBox, "Exact vehicle prefab resource path. When set, this exact prefab is used and editable entity label filters are ignored.", category: "Vehicle Selection")]
	protected ResourceName m_sVehiclePrefabOverride;

	[Attribute("0", UIWidgets.ComboBox, "Select editable entity labels to include when Vehicle Prefab Override is empty. Leave empty to allow any faction vehicle not rejected by excluded labels.", "", ParamEnumArray.FromEnum(EEditableEntityLabel), category: "Vehicle Selection")]
	protected ref array<EEditableEntityLabel> m_aIncludedEditableEntityLabels;

	[Attribute("0", UIWidgets.ComboBox, "Select editable entity labels to exclude when Vehicle Prefab Override is empty.", "", ParamEnumArray.FromEnum(EEditableEntityLabel), category: "Vehicle Selection")]
	protected ref array<EEditableEntityLabel> m_aExcludedEditableEntityLabels;

	[Attribute("0", UIWidgets.CheckBox, "If enabled, selected vehicles must contain every included label. If disabled, any included label may match.", category: "Vehicle Selection")]
	protected bool m_bRequireAllIncludedLabels;

	// Internal standard Vehicle Patrol safeguards; hidden so cleanup and crew protection stay automatic.
	[Attribute("", UIWidgets.Hidden, "Crew group prefab resource path used to spawn the patrol crew.", category: "Internal")]
	protected ResourceName m_sCrewGroupPrefab;

	[Attribute("2", UIWidgets.ComboBox, "Crew behavior mode. Routes to modular crew spawn handlers for future transport, logistics, and assault expansions.", "", ParamEnumArray.FromEnum(JLH_DCF_VehicleCrewSpawnMode), category: "Crew Setup")]
	protected JLH_DCF_VehicleCrewSpawnMode m_eCrewSpawnMode;

	[Attribute("3", UIWidgets.Hidden, "Maximum mounted crew for armed/turreted patrol vehicles. Driver and gunner are always prioritized; one extra commander/front passenger is allowed by default.", "2 3 1", category: "Internal")]
	protected int m_iMaxMountedCrewForArmedVehicles;

	[Attribute("1", UIWidgets.Hidden, "Queue dead AI bodies owned by this VehiclePatrol node for delayed, player-safe cleanup.", category: "Internal")]
	protected bool m_bDeadBodyCleanupEnabled;

	[Attribute("300", UIWidgets.Hidden, "Seconds to wait before a dead VehiclePatrol AI body may be cleaned up.", "0 7200 1", category: "Internal")]
	protected float m_fDeadBodyCleanupDelaySeconds;

	[Attribute("250", UIWidgets.Hidden, "Dead VehiclePatrol AI bodies are not removed while a valid player is within this distance.", "0 3000 1", category: "Internal")]
	protected float m_fDeadBodyCleanupPlayerSafeDistance;

	[Attribute("120", UIWidgets.Hidden, "Seconds after nearby VehiclePatrol combat before dead body cleanup may proceed.", "0 3600 1", category: "Internal")]
	protected float m_fDeadBodyCleanupRecentCombatDelaySeconds;

	protected string m_sFactionKey;
	protected bool m_bRegistered;
	protected bool m_bDisabled;
	protected int m_iNextAllowedSpawnTick;
	protected int m_iNextSpawnRangeDebugLogTick;
	protected bool m_bSpawnRangeLogStateInitialized;
	protected bool m_bLastSpawnRangeInRange;
	protected bool m_bRespawnCooldownActive;
	protected bool m_bRespawnStartPendingLog;
	protected bool m_bAwaitingReactivationAfterOutOfRange;
	protected bool m_bDeadBodyCleanupDisabledLogged;
	protected int m_iNextPatrolId = 1;
	protected ref JLH_DCF_VehiclePatrolPendingMount m_PendingMount;
	protected ref array<ref JLH_DCF_VehiclePatrolRuntime> m_aPatrols = {};
	protected ref array<ref JLH_DCF_VehicleWreckCleanupState> m_aDeferredWrecks = {};
	protected ref array<ref JLH_DCF_VehicleCorpseCleanupState> m_aDeferredCorpses = {};
	protected ref JLH_DCF_NodeRuntimeTracker m_RuntimeTracker = new JLH_DCF_NodeRuntimeTracker();

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		LogDebug(string.Format("Init node=%1 phase=OnPostInit", JLH_DCF_NodeDebug.EntityLabel(owner)));
		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		LogDebug(string.Format("Init node=%1 phase=EOnInit", JLH_DCF_NodeDebug.EntityLabel(owner)));
		JLH_VehiclePatrolManager.RegisterNode(this);
	}

	override void EOnActivate(IEntity owner)
	{
		super.EOnActivate(owner);
		LogDebug(string.Format("Activation entry node=%1", JLH_DCF_NodeDebug.EntityLabel(owner)));
	}

	void RegisterNode()
	{
		if (m_bRegistered || m_bDisabled)
			return;

		IEntity owner = GetOwner();
		if (!owner)
		{
			DisableNode("owner_missing");
			return;
		}

		if (!RefreshNodeFaction(owner))
		{
			DisableNode("node faction could not be resolved from SCR_FactionAffiliationComponent");
			return;
		}

		if (m_RuntimeTracker)
			m_RuntimeTracker.SetRuntimeOwner("VehiclePatrol");

		int waypointCount = JLH_DCF_NodeWaypointResolver.CountChildWaypoints(owner);
		int directChildCount = JLH_DCF_NodeWaypointResolver.CountDirectChildren(owner);
		LogDebug(string.Format("Child waypoint scan directChildren=%1 waypoints=%2", directChildCount, waypointCount));
		LogDebug(string.Format("Registered node name=%1", JLH_DCF_NodeDebug.EntityLabel(owner)));
		LogDebug(string.Format("Faction resolved faction=%1", m_sFactionKey));
		LogDebug(string.Format("Child waypoints resolved count=%1", waypointCount));

		int now = System.GetTickCount();
		m_iNextAllowedSpawnTick = now;
		m_bRespawnCooldownActive = false;
		m_bRespawnStartPendingLog = false;
		m_bAwaitingReactivationAfterOutOfRange = false;
		if (!m_bSpawnOnStartup)
			m_iNextAllowedSpawnTick = now + SecondsToMs(m_iRespawnPeriod);

		m_bRegistered = true;

		if (m_bSpawnOnStartup)
		{
			LogSmoke("Startup spawn requested");
			float startupDistance;
			if (IsPlayerWithinSpawnDistance(startupDistance))
			{
				if (!TryReserveVehicleSpawnBudget())
					return;

				LogSmoke("Spawn check passed");
				CleanupStaleTrackedRuntime();
				SpawnVehiclePatrol();
			}
		}
	}

	void Tick()
	{
		if (!m_bRegistered || m_bDisabled)
			return;

		ProcessDeferredWreckCleanup();
		ProcessDeferredCorpseCleanup();
		RefreshPatrols();

		UpdateDespawnLifecycle();

		if (m_PendingMount)
		{
			if (System.GetTickCount() >= m_PendingMount.ReadyTick)
				FinalizePendingMount();

			return;
		}

		if (m_aPatrols.Count() > 0)
			return;

		int now = System.GetTickCount();
		UpdateRespawnCooldown(now);
		if (m_iNextAllowedSpawnTick > now)
			return;

		float closestDistance;
		if (!IsPlayerWithinSpawnDistance(closestDistance))
			return;

		if (m_bAwaitingReactivationAfterOutOfRange)
		{
			if (!TryReserveVehicleSpawnBudget())
				return;

			LogSmoke("Reactivation spawn check passed");
		}
		else
		{
			if (!TryReserveVehicleSpawnBudget())
				return;

			LogSmoke("Spawn check passed");
		}

		if (m_bRespawnStartPendingLog)
		{
			LogSmoke("Respawn starting");
			m_bRespawnStartPendingLog = false;
		}

		CleanupStaleTrackedRuntime();
		SpawnVehiclePatrol();
	}

	protected void UpdateDespawnLifecycle()
	{
		if (!m_PendingMount && m_aPatrols.Count() <= 0)
			return;

		vector cleanupPosition;
		string cleanupPositionSource;
		if (!TryResolveLiveCleanupPosition(cleanupPosition, cleanupPositionSource))
			return;

		LogLiveCleanupPositionSource(cleanupPositionSource, cleanupPosition);

		ref array<IEntity> players = {};
		JLH_AddonWorldQuery.GatherLiveDeployedPlayers(players, "");

		float closestDistance;
		if (!AreAllLivePlayersBeyondPositionWithPlayers(cleanupPosition, GetEffectiveDespawnDistance(), players, closestDistance))
			return;

		string blockReason;
		if (!CanCleanupNodeRuntimePackageWithPlayers(false, players, blockReason))
			return;

		CleanupAllRuntime("players_out_of_range", false);
		LogDebug("Despawned reason=players_out_of_range");
		MarkOutOfRangeDespawnEligible();
	}

	protected void CleanupStaleTrackedRuntime()
	{
		if (!m_RuntimeTracker)
			return;

		if (m_aPatrols.Count() > 0 || m_PendingMount)
			return;

		int trackedGroups = m_RuntimeTracker.CountTrackedGroups();
		int trackedVehicles = m_RuntimeTracker.CountTrackedVehicles();
		if (trackedGroups <= 0 && trackedVehicles <= 0)
			return;

		m_RuntimeTracker.CleanupOwnedRuntime(SYSTEM_NAME, "before_respawn_activation", m_bDebugLogging);
		m_RuntimeTracker.ForgetAll();
	}

	protected bool HasActivePatrolOrPending()
	{
		if (m_PendingMount)
			return true;

		return m_aPatrols.Count() > 0;
	}

	protected int GetNextPatrolId()
	{
		int patrolId = m_iNextPatrolId;
		m_iNextPatrolId++;
		if (m_iNextPatrolId <= 0)
			m_iNextPatrolId = 1;

		return patrolId;
	}

	protected void UpdateRespawnCooldown(int now)
	{
		if (!m_bRespawnCooldownActive)
			return;

		if (m_iNextAllowedSpawnTick > now)
			return;

		m_bRespawnCooldownActive = false;
		m_bRespawnStartPendingLog = true;
		LogSmoke("Respawn cooldown complete");
		LogSmoke("Respawn allowed");
	}

	protected void ScheduleNextSpawn(string reason)
	{
		if (reason == "players_out_of_range")
		{
			MarkOutOfRangeDespawnEligible();
			return;
		}

		if (HasActiveAlivePatrol())
		{
			LogSmoke("Respawn cooldown skipped reason=active_patrol_alive");
			LogDebug(string.Format("Respawn cooldown skipped reason=active_patrol_alive source=%1", reason));
			return;
		}

		m_iNextAllowedSpawnTick = System.GetTickCount() + SecondsToMs(m_iRespawnPeriod);
		m_bRespawnCooldownActive = true;
		m_bRespawnStartPendingLog = false;
		LogSmoke(string.Format("Respawn cooldown started reason=%1 seconds=%2", GetRespawnCooldownLogReason(reason), m_iRespawnPeriod));
		LogDebug(string.Format("Respawn scheduled reason=%1 seconds=%2", reason, m_iRespawnPeriod));
	}

	protected void MarkOutOfRangeDespawnEligible()
	{
		m_iNextAllowedSpawnTick = System.GetTickCount();
		m_bRespawnCooldownActive = false;
		m_bRespawnStartPendingLog = false;
		m_bAwaitingReactivationAfterOutOfRange = true;
		LogSmoke("Despawned out of range, eligible for reactivation");
		LogDebug("Out-of-range despawn bypassed respawn cooldown");
	}

	protected bool TryReserveVehicleSpawnBudget()
	{
		int budgetDelayMs;
		if (JLH_DCF_RuntimeWorkBudget.TryReserveHeavySpawn("VehiclePatrol", budgetDelayMs))
			return true;

		if (budgetDelayMs < 250)
			budgetDelayMs = 250;

		m_iNextAllowedSpawnTick = System.GetTickCount() + budgetDelayMs;
		LogDebug(string.Format("Spawn deferred reason=runtime_budget delayMs=%1", budgetDelayMs));
		return false;
	}

	protected bool HasActiveAlivePatrol()
	{
		foreach (JLH_DCF_VehiclePatrolRuntime patrol : m_aPatrols)
		{
			if (!patrol || !patrol.VehicleEntity)
				continue;

			if (!IsEntityDestroyed(patrol.VehicleEntity))
				return true;
		}

		return false;
	}

	protected string GetRespawnCooldownLogReason(string reason)
	{
		if (reason == "vehicle_destroyed")
			return "destroyed";

		if (reason == "players_out_of_range")
			return "despawned";

		if (reason == "no_driver_timeout")
			return "no_driver_timeout";

		return reason;
	}

	protected void SpawnVehiclePatrol()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;

		if (HasActivePatrolOrPending())
		{
			LogSmoke("Spawn rejected reason=active_patrol_exists");
			return;
		}

		if (!RefreshNodeFaction(owner))
		{
			FailSpawn("faction_unresolved");
			return;
		}

		int waypointCount = JLH_DCF_NodeWaypointResolver.CountChildWaypoints(owner);
		int directChildCount = JLH_DCF_NodeWaypointResolver.CountDirectChildren(owner);
		LogSmoke(string.Format("Child waypoint scan directChildren=%1 waypoints=%2", directChildCount, waypointCount));
		LogDebug(string.Format("Child waypoints resolved count=%1", waypointCount));

		ResourceName vehiclePrefab = ResolveVehiclePrefab();
		LogSmoke(string.Format("Vehicle prefab resolved=%1", vehiclePrefab));
		if (IsEmptyResource(vehiclePrefab))
		{
			FailSpawn("vehicle_prefab_missing");
			return;
		}

		LogDebug(string.Format("Vehicle selected source=%1 prefab=%2", VehicleSelectionSourceLabel(), vehiclePrefab));

		IEntity vehicle = SpawnVehicle(vehiclePrefab);
		if (!vehicle)
		{
			FailSpawn("vehicle_spawn_failed");
			return;
		}

		LogDebug(string.Format("Vehicle spawned node=%1 vehicle=%2 vehiclePos=%3 prefab=%4", JLH_DCF_NodeDebug.EntityLabel(GetOwner()), JLH_DCF_NodeDebug.EntityLabel(vehicle), GetEntityPositionLabel(vehicle), vehiclePrefab));
		m_RuntimeTracker.TrackVehicle(vehicle);

		ResourceName crewPrefab = ResolveCrewGroupPrefab();
		LogSmoke(string.Format("Crew prefab resolved=%1", crewPrefab));
		if (IsEmptyResource(crewPrefab))
		{
			CleanupGroupVehicle(null, vehicle, "crew_group_prefab_missing");
			FailSpawn("crew_group_prefab_missing");
			return;
		}

		SCR_AIGroup crewGroup = SpawnCrewGroup(crewPrefab);
		if (!crewGroup)
		{
			CleanupGroupVehicle(null, vehicle, "crew_spawn_failed");
			FailSpawn("crew_spawn_failed");
			return;
		}

		m_RuntimeTracker.TrackGroup(crewGroup);
		int initialLiveCrewVisible = CountGroupUnits(crewGroup);
		LogSmoke(string.Format("Vehicle crew spawn returned initialLiveCrewVisible=%1 phase=spawn_return", initialLiveCrewVisible));
		LogDebug(string.Format("Vehicle crew spawn returned initialLiveCrewVisible=%1 phase=spawn_return", initialLiveCrewVisible));
		RegisterUsableVehicle(crewGroup, vehicle);

		QueuePendingMount(vehicle, crewGroup, vehiclePrefab, crewPrefab, waypointCount);
	}

	protected void QueuePendingMount(IEntity vehicle, SCR_AIGroup crewGroup, ResourceName vehiclePrefab, ResourceName crewPrefab, int waypointCount)
	{
		m_PendingMount = new JLH_DCF_VehiclePatrolPendingMount();
		m_PendingMount.Init(vehicle, crewGroup, vehiclePrefab, crewPrefab, waypointCount, PENDING_MOUNT_DELAY_MS);

		if (!GetGame() || !GetGame().GetCallqueue())
		{
			CleanupGroupVehicle(crewGroup, vehicle, "mount_queue_missing");
			m_PendingMount = null;
			FailSpawn("mount_queue_missing");
			return;
		}

		GetGame().GetCallqueue().CallLater(JLH_DCF_VehiclePatrolNodeComponent_FinalizePendingMount, PENDING_MOUNT_DELAY_MS, false, this);
	}

	void FinalizePendingMount()
	{
		if (!m_PendingMount)
			return;

		JLH_DCF_VehiclePatrolPendingMount pending = m_PendingMount;
		m_PendingMount = null;

		IEntity owner = GetOwner();
		if (!owner || !pending.VehicleEntity || !pending.CrewGroup)
		{
			CleanupGroupVehicle(pending.CrewGroup, pending.VehicleEntity, "pending_mount_missing_runtime");
			FailSpawn("pending_mount_missing_runtime");
			return;
		}

		RegisterUsableVehicle(pending.CrewGroup, pending.VehicleEntity);
		JLH_DCF_PersistenceExclusion.StopTrackingTree(pending.CrewGroup, "vehicle_patrol_pending_mount_group");
		JLH_DCF_PersistenceExclusion.StopTrackingTree(pending.VehicleEntity, "vehicle_patrol_pending_mount_vehicle");

		LogSmoke(string.Format("Mount ready node=%1 vehicle=%2 vehiclePos=%3 crewGroup=%4 crewSeen=%5 phase=pre_mount delayedMs=%6", JLH_DCF_NodeDebug.EntityLabel(GetOwner()), JLH_DCF_NodeDebug.EntityLabel(pending.VehicleEntity), GetEntityPositionLabel(pending.VehicleEntity), JLH_DCF_NodeDebug.EntityLabel(pending.CrewGroup), CountGroupUnits(pending.CrewGroup), PENDING_MOUNT_DELAY_MS));
		JLH_DCF_VehicleMountStats stats = new JLH_DCF_VehicleMountStats();
		bool mounted = MountCrew(pending.CrewGroup, pending.VehicleEntity, pending.VehiclePrefab, stats);
		IEntity driver = stats.DriverEntity;
		IEntity gunner = stats.GunnerEntity;

		if (!mounted)
		{
			string failReason = stats.FailureReason;
			if (failReason == "")
				failReason = "no_driver";

			CleanupGroupVehicle(pending.CrewGroup, pending.VehicleEntity, failReason);
			FailSpawn(failReason);
			return;
		}

		int finalWaypointCount = JLH_DCF_NodeWaypointResolver.CountChildWaypoints(owner);
		int finalDirectChildCount = JLH_DCF_NodeWaypointResolver.CountDirectChildren(owner);
		LogSmoke(string.Format("Child waypoint scan directChildren=%1 waypoints=%2 phase=movement_assignment", finalDirectChildCount, finalWaypointCount));

		array<IEntity> runtimeRouteWaypoints = {};
		if (finalWaypointCount > 0)
		{
			if (!JLH_DCF_NodeWaypointResolver.AssignRuntimeCopiesOfChildWaypointsToGroup(owner, pending.CrewGroup, SYSTEM_NAME, runtimeRouteWaypoints))
			{
				CleanupGroupVehicle(pending.CrewGroup, pending.VehicleEntity, "waypoint_assignment_failed");
				FailSpawn("waypoint_assignment_failed");
				return;
			}
		}
		else
		{
			LogSmoke("No child waypoints found; patrol will hold position");
			LogDebug("No child waypoints found; patrol will hold position");
		}

		JLH_DCF_VehiclePatrolRuntime patrol = new JLH_DCF_VehiclePatrolRuntime();
		patrol.Init(GetNextPatrolId(), pending.VehicleEntity, pending.CrewGroup, driver, gunner, pending.VehiclePrefab, pending.CrewPrefab, stats.PassengerEntities, runtimeRouteWaypoints);
		m_aPatrols.Insert(patrol);
		JLH_DCF_VehicleSustainment.RegisterVehicle(SYSTEM_NAME, pending.VehicleEntity, pending.CrewGroup, m_sFactionKey, JLH_DCF_NodeDebug.EntityLabel(GetOwner()));
		ActivatePatrolGroupAI(patrol);
		JLH_DCF_PersistenceExclusion.StopTrackingTree(pending.CrewGroup, "vehicle_patrol_mount_finalized_group");
		JLH_DCF_PersistenceExclusion.StopTrackingTree(pending.VehicleEntity, "vehicle_patrol_mount_finalized_vehicle");

		LogSmoke(string.Format("Patrol started %1 waypoints=%2 runtimeWaypoints=%3", BuildPatrolContext(patrol), finalWaypointCount, runtimeRouteWaypoints.Count()));
		LogDebug(string.Format("Patrol started %1 waypoints=%2 runtimeWaypoints=%3", BuildPatrolContext(patrol), finalWaypointCount, runtimeRouteWaypoints.Count()));
		if (m_bAwaitingReactivationAfterOutOfRange)
		{
			LogSmoke("Patrol restarted after out-of-range despawn");
			m_bAwaitingReactivationAfterOutOfRange = false;
		}

		if (GetGame() && GetGame().GetCallqueue())
			GetGame().GetCallqueue().CallLater(JLH_DCF_VehiclePatrolNodeComponent_ValidateMountedPatrols, POST_MOUNT_VALIDATION_DELAY_MS, false, this);
	}

	protected void FailSpawn(string reason)
	{
		m_bAwaitingReactivationAfterOutOfRange = false;
		LogSmoke(string.Format("Spawn failed reason=%1", reason));
		LogDebug(string.Format("Spawn failed reason=%1", reason));
		ScheduleNextSpawn(reason);
	}

	protected ResourceName ResolveVehiclePrefab()
	{
		if (!IsEmptyResource(m_sVehiclePrefabOverride))
			return m_sVehiclePrefabOverride;

		ResourceName resolverPrefab;
		if (JLH_DCF_DEVEnemyAssetResolver.TryResolveEnemyVehiclePrefabByRole(GetEnemyVehicleResolverRoleForCurrentFilters(), resolverPrefab) && IsEnemyProfileVehicleAllowedByCurrentFilters(resolverPrefab))
			return resolverPrefab;

		SCR_EntityCatalog catalog = ResolveFactionCatalog(EEntityCatalogType.VEHICLE);
		if (!catalog)
			return ResourceName.Empty;

		array<SCR_EntityCatalogEntry> candidates = {};
		catalog.GetFullFilteredEntityListWithLabels(candidates, m_aIncludedEditableEntityLabels, m_aExcludedEditableEntityLabels, m_bRequireAllIncludedLabels);

		if (candidates.IsEmpty())
			return ResourceName.Empty;

		SCR_EntityCatalogEntry selected = candidates.GetRandomElement();
		if (!selected)
			return ResourceName.Empty;

		return selected.GetPrefab();
	}

	protected ResourceName ResolveCrewGroupPrefab()
	{
		if (!IsEmptyResource(m_sCrewGroupPrefab))
		{
			if (IsOfficialCrewGroupPrefabForFaction(m_sCrewGroupPrefab))
				return m_sCrewGroupPrefab;

			LogSmoke(string.Format("Crew prefab override rejected reason=faction_mismatch_or_not_official faction=%1 prefab=%2", m_sFactionKey, m_sCrewGroupPrefab));
		}

		ResourceName resolverCrewPrefab;
		if (JLH_DCF_DEVEnemyAssetResolver.TryResolveEnemyCrewGroupPrefabByRole(JLH_EnemyAssetRole.VEHICLE_CREW, resolverCrewPrefab))
			return resolverCrewPrefab;

		return ResolveDedicatedCrewGroupPrefabForFaction();
	}

	protected string GetEnemyVehicleResolverRoleForCurrentFilters()
	{
		if (HasEditableEntityLabel(m_aIncludedEditableEntityLabels, "TRAIT_ARMORED"))
			return JLH_EnemyAssetRole.VEHICLE_PATROL_ARMORED;

		if (HasEditableEntityLabel(m_aIncludedEditableEntityLabels, "TRAIT_ARMED"))
			return JLH_EnemyAssetRole.VEHICLE_PATROL_ARMED;

		return JLH_EnemyAssetRole.VEHICLE_PATROL_LIGHT;
	}

	protected bool HasEditableEntityLabel(array<EEditableEntityLabel> labels, string expectedLabel)
	{
		if (!labels || expectedLabel == "")
			return false;

		foreach (EEditableEntityLabel label : labels)
		{
			string labelName = SCR_Enum.GetEnumName(EEditableEntityLabel, label);
			if (labelName == expectedLabel)
				return true;
		}

		return false;
	}

	protected bool IsEnemyProfileVehicleAllowedByCurrentFilters(ResourceName prefab)
	{
		if (IsEmptyResource(prefab))
			return false;

		SCR_EntityCatalog catalog = ResolveFactionCatalog(EEntityCatalogType.VEHICLE);
		if (!catalog)
			return false;

		array<SCR_EntityCatalogEntry> candidates = {};
		catalog.GetFullFilteredEntityListWithLabels(candidates, m_aIncludedEditableEntityLabels, m_aExcludedEditableEntityLabels, m_bRequireAllIncludedLabels);

		foreach (SCR_EntityCatalogEntry entry : candidates)
		{
			if (entry && entry.GetPrefab() == prefab)
				return true;
		}

		LogDebug(string.Format("Enemy profile vehicle rejected reason=label_filter_mismatch prefab=%1", prefab));
		return false;
	}

	protected ResourceName ResolveDedicatedCrewGroupPrefabForFaction()
	{
		ResourceName crewGroupPrefab = GetDedicatedCrewGroupPrefabForFaction();
		if (IsEmptyResource(crewGroupPrefab))
		{
			LogSmoke(string.Format("Dedicated vehicle group result=false reason=unsupported_faction faction=%1", m_sFactionKey));
			return ResourceName.Empty;
		}

		Resource resource = Resource.Load(crewGroupPrefab);
		bool valid = resource && resource.IsValid();
		LogSmoke(string.Format("Dedicated vehicle group result=%1 faction=%2 prefab=%3", JLH_DCF_NodeDebug.BoolLabel(valid), m_sFactionKey, crewGroupPrefab));
		if (!valid)
			return ResourceName.Empty;

		return crewGroupPrefab;
	}

	protected ResourceName GetDedicatedCrewGroupPrefabForFaction()
	{
		string factionKey = m_sFactionKey;
		factionKey.ToUpper();

		if (factionKey == "USSR")
			return DEFAULT_USSR_VEHICLE_CREW_GROUP_PREFAB;

		if (factionKey == "US")
			return DEFAULT_US_VEHICLE_CREW_GROUP_PREFAB;

		return ResourceName.Empty;
	}

	protected bool IsOfficialCrewGroupPrefabForFaction(ResourceName prefab)
	{
		if (IsEmptyResource(prefab))
			return false;

		string prefabPath = prefab;
		prefabPath.ToUpper();

		string factionKey = m_sFactionKey;
		factionKey.ToUpper();

		if (factionKey == "USSR")
		{
			if (prefabPath.Contains("PREFABS/GROUPS/JLH_DCF/JLH_USSR_VEHICLEGROUP.ET"))
				return true;

			return prefabPath.Contains("PREFABS/GROUPS/OPFOR/") && prefabPath.Contains("USSR");
		}

		if (factionKey == "US")
		{
			if (prefabPath.Contains("PREFABS/GROUPS/JLH_DCF/JLH_US_VEHICLEGROUP.ET"))
				return true;

			return prefabPath.Contains("PREFABS/GROUPS/BLUFOR/") && prefabPath.Contains("GROUP_US_");
		}

		return false;
	}

	protected SCR_EntityCatalog ResolveFactionCatalog(EEntityCatalogType catalogType)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return null;

		SCR_EntityCatalogManagerComponent catalogManager = null;
		if (GetGame() && GetGame().GetGameMode())
			catalogManager = SCR_EntityCatalogManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_EntityCatalogManagerComponent));

		if (catalogManager && m_sFactionKey != "")
		{
			SCR_EntityCatalog factionCatalog = catalogManager.GetFactionEntityCatalogOfType(catalogType, m_sFactionKey, false);
			if (factionCatalog)
				return factionCatalog;
		}

		SCR_FactionAffiliationComponent affiliation = SCR_FactionAffiliationComponent.Cast(owner.FindComponent(SCR_FactionAffiliationComponent));
		if (!affiliation)
			return null;

		SCR_Faction faction = SCR_Faction.Cast(affiliation.GetAffiliatedFaction());
		if (!faction)
			faction = SCR_Faction.Cast(affiliation.GetDefaultAffiliatedFaction());

		if (faction)
			return faction.GetFactionEntityCatalogOfType(catalogType, false);

		if (!catalogManager)
			catalogManager = SCR_EntityCatalogManagerComponent.GetInstance();

		if (!catalogManager)
			return null;

		return catalogManager.GetEntityCatalogOfType(catalogType, false);
	}

	protected IEntity SpawnVehicle(ResourceName vehiclePrefab)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return null;

		return SpawnVehicleAt(vehiclePrefab, owner.GetOrigin());
	}

	protected IEntity SpawnVehicleAt(ResourceName vehiclePrefab, vector position)
	{
		Resource resource = Resource.Load(vehiclePrefab);
		bool resourceLoaded = resource && resource.IsValid();
		LogSmoke(string.Format("Resource load result=%1 prefab=%2", JLH_DCF_NodeDebug.BoolLabel(resourceLoaded), vehiclePrefab));
		if (!resource || !resource.IsValid())
			return null;

		IEntity owner = GetOwner();
		if (!owner || !GetGame() || !GetGame().GetWorld())
			return null;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		owner.GetTransform(spawnParams.Transform);
		spawnParams.Transform[3] = position;

		IEntity spawnedVehicle = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams);
		JLH_DCF_PersistenceExclusion.StopTrackingTree(spawnedVehicle, "vehicle_patrol_vehicle_spawn");
		LogSmoke(string.Format("SpawnEntity result=%1 entity=%2", JLH_DCF_NodeDebug.BoolLabel(spawnedVehicle != null), JLH_DCF_NodeDebug.EntityLabel(spawnedVehicle)));
		return spawnedVehicle;
	}

	protected SCR_AIGroup SpawnCrewGroup(ResourceName crewPrefab)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return null;

		return SpawnCrewGroupAt(crewPrefab, owner.GetOrigin() + Vector(2, 0, 0));
	}

	protected SCR_AIGroup SpawnCrewGroupAt(ResourceName crewPrefab, vector position)
	{
		Resource resource = Resource.Load(crewPrefab);
		bool resourceLoaded = resource && resource.IsValid();
		LogSmoke(string.Format("Crew resource load result=%1 prefab=%2", JLH_DCF_NodeDebug.BoolLabel(resourceLoaded), crewPrefab));
		if (!resource || !resource.IsValid())
			return null;

		IEntity owner = GetOwner();
		if (!owner || !GetGame() || !GetGame().GetWorld())
			return null;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		owner.GetTransform(spawnParams.Transform);
		spawnParams.Transform[3] = position;

		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams));
		LogSmoke(string.Format("Crew SpawnEntity result=%1 entity=%2", JLH_DCF_NodeDebug.BoolLabel(group != null), JLH_DCF_NodeDebug.EntityLabel(group)));
		if (!group)
			return null;

		if (!group.GetSpawnImmediately())
			group.SpawnUnits();

		JLH_DCF_PersistenceExclusion.StopTrackingTree(group, "vehicle_patrol_crew_group_spawn");
		//group.DeactivateAllMembers();
		return group;
	}

	protected int CountGroupUnits(SCR_AIGroup group)
	{
		if (!group)
			return 0;

		array<AIAgent> agents = {};
		group.GetAgents(agents);

		int count = 0;
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			if (agent.GetControlledEntity())
				count++;
		}

		return count;
	}

	protected int CountLivingGroupUnits(SCR_AIGroup group)
	{
		if (!group)
			return 0;

		array<AIAgent> agents = {};
		group.GetAgents(agents);

		int count = 0;
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (unit && !IsEntityDestroyed(unit))
				count++;
		}

		return count;
	}

	protected JLH_DCF_VehicleCrewSpawnModeHandler ResolveCrewSpawnModeHandler()
	{
		return JLH_DCF_VehicleCrewSpawnModeRouter.Create(m_eCrewSpawnMode);
	}

	protected bool MountCrew(SCR_AIGroup group, IEntity vehicle, ResourceName vehiclePrefab, JLH_DCF_VehicleMountStats stats)
	{
		if (stats)
			stats.Reset();

		if (!group || !vehicle || !stats)
		{
			if (stats)
				stats.FailureReason = "crew_entity_invalid";

			return false;
		}

		if (!IsEntitySafeForMount(vehicle))
		{
			stats.FailureReason = "replication_not_ready";
			return false;
		}

		//group.DeactivateAllMembers();
		array<AIAgent> agents = {};
		group.GetAgents(agents);
		bool replicationNotReady = false;
		bool likelyArmed = VehiclePrefabLikelyRequiresGunner(vehiclePrefab);
		bool armedCrewLimitLogged = false;
		int maxMountedCrewForArmedVehicle = GetMaxMountedCrewForArmedVehicle();
		JLH_DCF_VehicleCrewSpawnModeHandler crewSpawnHandler = ResolveCrewSpawnModeHandler();
		int maxCargoSeats = crewSpawnHandler.GetMaxCargoSeats(likelyArmed, maxMountedCrewForArmedVehicle);
		if (likelyArmed)
		{
			LogArmedCrewLimitApplied();
			armedCrewLimitLogged = true;
		}

		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!unit)
				continue;

			stats.CrewSeen++;
			if (!IsEntitySafeForMount(unit))
			{
				replicationNotReady = true;
				continue;
			}

			if (IsUnitInVehicle(unit, vehicle))
			{
				RecordExistingMountedUnit(unit, vehicle, stats, crewSpawnHandler);
				continue;
			}

			bool moved = false;
			SCR_CompartmentAccessComponent access = SCR_CompartmentAccessComponent.Cast(unit.FindComponent(SCR_CompartmentAccessComponent));
			if (access)
			{
				stats.HasCompartmentAccess = true;
				unit.SetOrigin(vehicle.GetOrigin());

				if (!stats.DriverMounted && access.MoveInVehicle(vehicle, ECompartmentType.PILOT))
				{
					stats.DriverMounted = true;
					stats.DriverEntity = unit;
					stats.SeatsFilled++;
					moved = true;
				}
				else if (crewSpawnHandler.CanMountGunnerSeat(likelyArmed, stats, maxMountedCrewForArmedVehicle) && access.MoveInVehicle(vehicle, ECompartmentType.TURRET))
				{
					if (!stats.GunnerMounted)
					{
						stats.GunnerMounted = true;
						stats.GunnerEntity = unit;
					}

					stats.TurretSeatsFilled++;
					stats.SeatsFilled++;
					moved = true;
					if (!likelyArmed)
					{
						likelyArmed = true;
						maxCargoSeats = crewSpawnHandler.GetMaxCargoSeats(true, maxMountedCrewForArmedVehicle);
						if (!armedCrewLimitLogged)
						{
							LogArmedCrewLimitApplied();
							armedCrewLimitLogged = true;
						}
					}
				}
				else if (crewSpawnHandler.CanMountCargoSeat(likelyArmed, stats, maxCargoSeats, maxMountedCrewForArmedVehicle) && access.MoveInVehicle(vehicle, ECompartmentType.CARGO))
				{
					stats.CargoSeatsFilled++;
					stats.SeatsFilled++;
					if (unit != stats.DriverEntity && unit != stats.GunnerEntity && !stats.PassengerEntities.Contains(unit))
						stats.PassengerEntities.Insert(unit);

					moved = true;
				}
			}

			if (moved)
				continue;

			DeleteEntitySafe(unit);
			stats.RemovedUnassigned++;
		}

		if (likelyArmed || stats.GunnerMounted)
			stats.RemovedUnassigned += PruneMountedCrewOverLimit(group, vehicle, maxMountedCrewForArmedVehicle);

		SanitizeMountStatsPassengerRoles(stats);

		int driverMounted = 0;
		if (stats.DriverMounted)
			driverMounted = 1;

		int gunnerMounted = 0;
		if (stats.GunnerMounted)
			gunnerMounted = 1;

		if (likelyArmed || stats.GunnerMounted)
		{
			string mountLog = string.Format("Mount complete node=%1 vehicle=%2 vehiclePos=%3 driver=%4 gunner=%5 commander=%6 passengers=0", JLH_DCF_NodeDebug.EntityLabel(GetOwner()), JLH_DCF_NodeDebug.EntityLabel(vehicle), GetEntityPositionLabel(vehicle), driverMounted, gunnerMounted, stats.CargoSeatsFilled);
			mountLog += string.Format(" deletedUnused=%1", stats.RemovedUnassigned);
			LogSmoke(mountLog);
			LogDebug(mountLog);
		}
		else
		{
			string mountLog = string.Format("Mount complete node=%1 vehicle=%2 vehiclePos=%3 driver=%4 gunner=%5 passengers=%6 deletedUnused=%7", JLH_DCF_NodeDebug.EntityLabel(GetOwner()), JLH_DCF_NodeDebug.EntityLabel(vehicle), GetEntityPositionLabel(vehicle), driverMounted, gunnerMounted, stats.CargoSeatsFilled, stats.RemovedUnassigned);
			LogSmoke(mountLog);
			LogDebug(mountLog);
		}

		bool gunnerRequired = crewSpawnHandler.RequiresGunner(likelyArmed);

		if (stats.CrewSeen <= 0)
			stats.FailureReason = "crew_entity_invalid";
		else if (replicationNotReady && stats.SeatsFilled <= 0)
			stats.FailureReason = "replication_not_ready";
		else if (!stats.HasCompartmentAccess)
			stats.FailureReason = "vehicle_compartment_api_failed";
		else if (!stats.DriverMounted)
			stats.FailureReason = "no_driver";
		else if (gunnerRequired && !stats.GunnerMounted)
			stats.FailureReason = "no_gunner";
		else
			stats.FailureReason = "";

		if (gunnerRequired && !stats.GunnerMounted)
			return false;

		return stats.DriverMounted && stats.SeatsFilled > 0;
	}

	protected int PruneMountedCrewOverLimit(SCR_AIGroup group, IEntity vehicle, int maxMountedCrew)
	{
		if (!group || !vehicle || maxMountedCrew <= 0)
			return 0;

		array<AIAgent> agents = {};
		group.GetAgents(agents);

		int mountedSeen = 0;
		int removed = 0;
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!unit || !IsUnitInVehicle(unit, vehicle))
				continue;

			mountedSeen++;
			if (mountedSeen <= maxMountedCrew)
				continue;

			DeleteEntitySafe(unit);
			removed++;
		}

		if (removed > 0)
			LogSmoke(string.Format("Armed patrol mount policy pruned extraMountedCrew=%1 maxCrew=%2", removed, maxMountedCrew));

		return removed;
	}

	protected void LogArmedCrewLimitApplied()
	{
		LogSmoke(string.Format("Armed patrol mount policy maxCrew=%1", GetMaxMountedCrewForArmedVehicle()));
	}

	protected void RecordExistingMountedUnit(IEntity unit, IEntity vehicle, JLH_DCF_VehicleMountStats stats, JLH_DCF_VehicleCrewSpawnModeHandler crewSpawnHandler)
	{
		if (!unit || !vehicle || !stats || !crewSpawnHandler)
			return;

		stats.HasCompartmentAccess = true;
		stats.SeatsFilled++;

		if (!stats.DriverMounted && IsUnitInCompartmentType(unit, vehicle, ECompartmentType.PILOT))
		{
			stats.DriverMounted = true;
			stats.DriverEntity = unit;
			return;
		}

		if (crewSpawnHandler.AllowsGunnerSeat() && IsUnitInGunnerRole(unit, vehicle))
		{
			stats.TurretSeatsFilled++;
			if (!stats.GunnerMounted)
			{
				stats.GunnerMounted = true;
				stats.GunnerEntity = unit;
			}
			return;
		}

		if (crewSpawnHandler.AllowsCargoSeats() && IsUnitInCompartmentType(unit, vehicle, ECompartmentType.CARGO))
		{
			stats.CargoSeatsFilled++;
			if (unit != stats.DriverEntity && unit != stats.GunnerEntity && !stats.PassengerEntities.Contains(unit))
				stats.PassengerEntities.Insert(unit);
		}
	}

	protected int GetMaxMountedCrewForArmedVehicle()
	{
		if (m_iMaxMountedCrewForArmedVehicles < 2)
			return 2;

		if (m_iMaxMountedCrewForArmedVehicles > 3)
			return 3;

		return m_iMaxMountedCrewForArmedVehicles;
	}

	protected bool VehiclePrefabLikelyRequiresGunner(ResourceName vehiclePrefab)
	{
		if (IsEmptyResource(vehiclePrefab))
			return false;

		string token = vehiclePrefab;
		token.ToLower();

		if (token.Contains("unarmed"))
			return false;

		if (token.Contains("armed"))
			return true;

		if (token.Contains("turret"))
			return true;

		if (token.Contains("weapon"))
			return true;

		if (token.Contains("pkm"))
			return true;

		if (token.Contains("pkp"))
			return true;

		if (token.Contains("pkt"))
			return true;

		if (token.Contains("dshk"))
			return true;

		if (token.Contains("m2hb"))
			return true;

		if (token.Contains("hmg"))
			return true;

		if (token.Contains("ags17"))
			return true;

		if (token.Contains("ags_17"))
			return true;

		if (token.Contains("ags-17"))
			return true;

		if (token.Contains("ags"))
			return true;

		if (token.Contains("spg9"))
			return true;

		if (token.Contains("spg-9"))
			return true;

		if (token.Contains("kpvt"))
			return true;

		if (token.Contains("cannon"))
			return true;

		if (token.Contains("autocannon"))
			return true;

		if (token.Contains("btr"))
			return true;

		if (token.Contains("brdm"))
			return true;

		if (token.Contains("bmp"))
			return true;

		if (token.Contains("apc"))
			return true;

		return false;
	}

	protected void SanitizeMountStatsPassengerRoles(JLH_DCF_VehicleMountStats stats)
	{
		if (!stats || !stats.PassengerEntities)
			return;

		for (int i = stats.PassengerEntities.Count() - 1; i >= 0; i--)
		{
			IEntity passenger = stats.PassengerEntities[i];
			if (!passenger || passenger == stats.DriverEntity || passenger == stats.GunnerEntity)
				stats.PassengerEntities.Remove(i);
		}
	}

	protected int ResumePatrolRoute(JLH_DCF_VehiclePatrolRuntime patrol)
	{
		if (!patrol || !patrol.CrewGroup || !patrol.RouteWaypoints)
			return 0;

		array<AIWaypoint> currentWaypoints = {};
		patrol.CrewGroup.GetWaypoints(currentWaypoints);

		int assigned = 0;
		foreach (IEntity waypointEntity : patrol.RouteWaypoints)
		{
			AIWaypoint waypoint = AIWaypoint.Cast(waypointEntity);
			if (!waypoint)
				continue;

			if (!currentWaypoints.Contains(waypoint))
			{
				patrol.CrewGroup.AddWaypoint(waypoint);
				currentWaypoints.Insert(waypoint);
			}

			assigned++;
		}

		ActivatePatrolGroupAI(patrol);
		patrol.RoutePausedForRecovery = false;
		return assigned;
	}

	protected void PausePatrolRoute(JLH_DCF_VehiclePatrolRuntime patrol)
	{
		if (!patrol || !patrol.CrewGroup || patrol.RoutePausedForRecovery)
			return;

		JLH_DCF_NodeWaypointResolver.ClearGroupWaypoints(patrol.CrewGroup);
		patrol.RoutePausedForRecovery = true;
	}

	protected void RefreshMountedCrewAssignments(JLH_DCF_VehiclePatrolRuntime patrol)
	{
		if (!patrol || !patrol.CrewGroup || !patrol.VehicleEntity)
			return;

		bool driverFound = false;
		bool gunnerFound = false;
		if (patrol.DriverEntity && !IsEntityDestroyed(patrol.DriverEntity) && IsUnitInCompartmentType(patrol.DriverEntity, patrol.VehicleEntity, ECompartmentType.PILOT))
			driverFound = true;

		if (patrol.GunnerEntity && !IsEntityDestroyed(patrol.GunnerEntity) && IsUnitInGunnerRole(patrol.GunnerEntity, patrol.VehicleEntity))
			gunnerFound = true;

		array<AIAgent> agents = {};
		patrol.CrewGroup.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!unit || IsEntityDestroyed(unit))
				continue;

			if (!driverFound && IsUnitInCompartmentType(unit, patrol.VehicleEntity, ECompartmentType.PILOT))
			{
				patrol.DriverEntity = unit;
				driverFound = true;
			}

			if (!gunnerFound && IsUnitInGunnerRole(unit, patrol.VehicleEntity))
			{
				patrol.GunnerEntity = unit;
				gunnerFound = true;
			}

			if (driverFound && gunnerFound)
				break;
		}
	}

	protected void ResetMountedRecoveryState(JLH_DCF_VehiclePatrolRuntime patrol)
	{
		if (!patrol)
			return;

		patrol.DriverLostSinceTick = 0;
		patrol.RouteResumeReadyTick = 0;
		patrol.LastMountedRecoveryAttemptTick = 0;
		patrol.MountedRecoveryAttempts = 0;
		patrol.LastMountedRecoveryLogTick = 0;
	}

	protected int CountLivingPatrolCrew(JLH_DCF_VehiclePatrolRuntime patrol)
	{
		if (!patrol || !patrol.CrewGroup)
			return 0;

		int count = 0;
		array<AIAgent> agents = {};
		patrol.CrewGroup.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (unit && !IsEntityDestroyed(unit))
				count++;
		}

		return count;
	}

	protected bool TryMoveCrewUnitIntoSeat(IEntity unit, IEntity vehicle, ECompartmentType compartmentType)
	{
		if (!unit || !vehicle || IsEntityDestroyed(unit) || IsEntityDestroyed(vehicle))
			return false;

		if (!IsEntitySafeForMount(unit) || !IsEntitySafeForMount(vehicle))
			return false;

		if (compartmentType == ECompartmentType.TURRET && IsUnitInGunnerRole(unit, vehicle))
			return true;

		if (IsUnitInCompartmentType(unit, vehicle, compartmentType))
			return true;

		SCR_CompartmentAccessComponent access = SCR_CompartmentAccessComponent.Cast(unit.FindComponent(SCR_CompartmentAccessComponent));
		if (!access)
			return false;

		unit.SetOrigin(vehicle.GetOrigin());
		return access.MoveInVehicle(vehicle, compartmentType);
	}

	protected IEntity FindCrewForSeat(JLH_DCF_VehiclePatrolRuntime patrol, ECompartmentType compartmentType, IEntity preferred, IEntity reserved)
	{
		if (!patrol || !patrol.CrewGroup || !patrol.VehicleEntity)
			return null;

		if (preferred && preferred != reserved && TryMoveCrewUnitIntoSeat(preferred, patrol.VehicleEntity, compartmentType))
			return preferred;

		array<AIAgent> agents = {};
		patrol.CrewGroup.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!unit || unit == reserved)
				continue;

			if (TryMoveCrewUnitIntoSeat(unit, patrol.VehicleEntity, compartmentType))
				return unit;
		}

		return null;
	}

	protected bool TryRecoverMountedCrew(JLH_DCF_VehiclePatrolRuntime patrol, int now, string reason)
	{
		if (!patrol || !patrol.VehicleEntity || !patrol.CrewGroup)
			return false;

		if (IsEntityDestroyed(patrol.VehicleEntity))
			return false;

		int livingCrew = CountLivingPatrolCrew(patrol);
		if (livingCrew <= 0)
			return false;

		if (patrol.LastMountedRecoveryAttemptTick > 0 && now - patrol.LastMountedRecoveryAttemptTick < MOUNTED_RECOVERY_RETRY_MS)
			return false;

		patrol.LastMountedRecoveryAttemptTick = now;
		patrol.MountedRecoveryAttempts++;
		PausePatrolRoute(patrol);
		RegisterUsableVehicle(patrol.CrewGroup, patrol.VehicleEntity);
		//patrol.CrewGroup.DeactivateAllMembers();

		IEntity driver = FindCrewForSeat(patrol, ECompartmentType.PILOT, patrol.DriverEntity, null);
		if (driver)
			patrol.DriverEntity = driver;

		bool gunnerRequired = patrol.AttackCapable || VehiclePrefabLikelyRequiresGunner(patrol.VehiclePrefab);
		if (gunnerRequired)
		{
			IEntity gunner = FindCrewForSeat(patrol, ECompartmentType.TURRET, patrol.GunnerEntity, patrol.DriverEntity);
			if (gunner)
			{
				patrol.GunnerEntity = gunner;
				patrol.AttackCapable = true;
			}
		}

		RefreshMountedCrewAssignments(patrol);
		bool driverValid = IsDriverValid(patrol);
		bool gunnerValid = !gunnerRequired || IsGunnerValid(patrol);
		if (driverValid && gunnerValid)
		{
			patrol.PatrolState = JLH_DCF_VehiclePatrolState.PATROLLING;
			ResetMountedRecoveryState(patrol);
			int resumedWaypoints = ResumePatrolRoute(patrol);
			LogSmoke(string.Format("Mounted recovery succeeded %1 reason=%2 livingCrew=%3 waypoints=%4", BuildPatrolContext(patrol), reason, livingCrew, resumedWaypoints));
			return true;
		}

		ActivateGroupAI(patrol.CrewGroup);
		if (patrol.LastMountedRecoveryLogTick <= 0 || now - patrol.LastMountedRecoveryLogTick >= MOUNTED_RECOVERY_LOG_THROTTLE_MS)
		{
			patrol.LastMountedRecoveryLogTick = now;
			LogSmoke(string.Format("Mounted recovery deferred %1 reason=%2 livingCrew=%3 driver=%4 gunner=%5 attempt=%6", BuildPatrolContext(patrol), reason, livingCrew, JLH_DCF_NodeDebug.BoolLabel(driverValid), JLH_DCF_NodeDebug.BoolLabel(gunnerValid), patrol.MountedRecoveryAttempts));
		}

		return false;
	}

	protected bool IsDriverValid(JLH_DCF_VehiclePatrolRuntime patrol)
	{
		if (!patrol || !patrol.VehicleEntity || !patrol.DriverEntity)
			return false;

		if (IsEntityDestroyed(patrol.DriverEntity))
			return false;

		return IsUnitInCompartmentType(patrol.DriverEntity, patrol.VehicleEntity, ECompartmentType.PILOT) || IsUnitInVehicle(patrol.DriverEntity, patrol.VehicleEntity);
	}

	protected bool IsGunnerValid(JLH_DCF_VehiclePatrolRuntime patrol)
	{
		if (!patrol || !patrol.VehicleEntity || !patrol.GunnerEntity)
			return false;

		if (IsEntityDestroyed(patrol.GunnerEntity))
			return false;

		return IsUnitInGunnerRole(patrol.GunnerEntity, patrol.VehicleEntity);
	}

	protected bool NeedsMountedRecovery(JLH_DCF_VehiclePatrolRuntime patrol)
	{
		if (!patrol || !patrol.AttackCapable)
			return false;

		if (!IsDriverValid(patrol))
			return true;

		if (!IsGunnerValid(patrol))
			return true;

		return false;
	}

	protected bool UpdatePatrolDriverState(JLH_DCF_VehiclePatrolRuntime patrol, int now)
	{
		if (!patrol)
			return false;

		if (!patrol.VehicleEntity || IsEntityDestroyed(patrol.VehicleEntity))
		{
			LogSmoke(string.Format("Armed vehicle fallback %1 reason=vehicle_destroyed", BuildPatrolContext(patrol)));
			return false;
		}

		RefreshMountedCrewAssignments(patrol);
		bool driverValid = IsDriverValid(patrol);
		bool gunnerValid = !patrol.AttackCapable || IsGunnerValid(patrol);
		bool mountedRecoveryNeeded = NeedsMountedRecovery(patrol);

		if (driverValid && gunnerValid && !mountedRecoveryNeeded)
		{
			if (patrol.PatrolState == JLH_DCF_VehiclePatrolState.COMBAT_PAUSED || patrol.RoutePausedForRecovery)
			{
				if (patrol.RouteResumeReadyTick <= 0)
					patrol.RouteResumeReadyTick = now + POST_COMBAT_ROUTE_RESUME_GRACE_MS;

				if (now < patrol.RouteResumeReadyTick)
					return true;

				int resumedWaypoints = ResumePatrolRoute(patrol);
				ResetMountedRecoveryState(patrol);
				patrol.PatrolState = JLH_DCF_VehiclePatrolState.PATROLLING;
				LogSmoke(string.Format("Post combat route resumed %1 waypoints=%2", BuildPatrolContext(patrol), resumedWaypoints));
				return true;
			}

			ResetMountedRecoveryState(patrol);
			return true;
		}

		if (patrol.DriverLostSinceTick <= 0)
		{
			patrol.DriverLostSinceTick = now;
			patrol.LastCombatTick = now;
			patrol.PatrolState = JLH_DCF_VehiclePatrolState.COMBAT_PAUSED;
			PausePatrolRoute(patrol);
			LogSmoke(string.Format("Combat engaged %1 mountedGunner=%2", BuildPatrolContext(patrol), JLH_DCF_NodeDebug.BoolLabel(gunnerValid)));
			if (!driverValid)
				LogSmoke(string.Format("Armed vehicle fallback %1 reason=no_driver status=temporary_recovery_pending", BuildPatrolContext(patrol)));
			else if (!gunnerValid)
				LogSmoke(string.Format("Armed vehicle fallback %1 reason=no_gunner status=temporary_recovery_pending", BuildPatrolContext(patrol)));

			TryRecoverMountedCrew(patrol, now, "initial_driver_state_lost");
			return true;
		}

		patrol.LastCombatTick = now;
		if (TryRecoverMountedCrew(patrol, now, "driver_state_lost"))
			return true;

		if (now - patrol.DriverLostSinceTick < DRIVER_LOST_TIMEOUT_MS)
			return true;

		if (CountLivingPatrolCrew(patrol) > 0)
		{
			patrol.PatrolState = JLH_DCF_VehiclePatrolState.DISMOUNTED_FALLBACK;
			TryRecoverMountedCrew(patrol, now, "no_driver_timeout_recovery");
			if (patrol.LastMountedRecoveryLogTick <= 0 || now - patrol.LastMountedRecoveryLogTick >= MOUNTED_RECOVERY_LOG_THROTTLE_MS)
			{
				patrol.LastMountedRecoveryLogTick = now;
				LogSmoke(string.Format("Patrol recovery held %1 reason=alive_crew_vehicle_valid", BuildPatrolContext(patrol)));
			}
			return true;
		}

		patrol.PatrolState = JLH_DCF_VehiclePatrolState.DISMOUNTED_FALLBACK;
		return false;
	}

	void ValidateMountedPatrols()
	{
		int now = System.GetTickCount();
		for (int i = m_aPatrols.Count() - 1; i >= 0; i--)
		{
			JLH_DCF_VehiclePatrolRuntime patrol = m_aPatrols[i];
			if (!patrol || !patrol.VehicleEntity || !patrol.CrewGroup)
			{
				m_aPatrols.Remove(i);
				continue;
			}

			if (!UpdatePatrolDriverState(patrol, now))
			{
				LogSmoke(string.Format("Patrol invalid %1 reason=no_driver_timeout status=confirmed_timeout", BuildPatrolContext(patrol)));
				LogSmoke(string.Format("Patrol cleanup started %1", BuildPatrolContext(patrol)));
				MarkPatrolCleared(patrol, "no_driver_timeout");
				string noDriverBlockReason;
				if (!CanCleanupVehiclePatrol(patrol, true, noDriverBlockReason))
					continue;

				CleanupPatrolRuntime(patrol, "no_driver_timeout");
				m_aPatrols.Remove(i);
				ScheduleNextSpawn("no_driver_timeout");
				continue;
			}

			if (!IsDriverValid(patrol))
				continue;

			int prunedMounted = 0;
			if (patrol.AttackCapable || VehiclePrefabLikelyRequiresGunner(patrol.VehiclePrefab))
				prunedMounted = PruneMountedCrewOverLimit(patrol.CrewGroup, patrol.VehicleEntity, GetMaxMountedCrewForArmedVehicle());

			int mountedCrew = 0;
			int deletedUnseated = 0;
			array<AIAgent> agents = {};
			patrol.CrewGroup.GetAgents(agents);

			foreach (AIAgent agent : agents)
			{
				if (!agent)
					continue;

				IEntity unit = agent.GetControlledEntity();
				if (!unit)
					continue;

				if (IsUnitInVehicle(unit, patrol.VehicleEntity))
				{
					mountedCrew++;
					continue;
				}

				if (IsCorpseEntity(unit))
				{
					QueueCorpseCleanup(unit, patrol.PatrolId, patrol.LastCombatTick);
					deletedUnseated++;
					continue;
				}

				DeleteEntitySafe(unit);
				deletedUnseated++;
			}

			LogSmoke(string.Format("Post mount validation %1 driver=1 mountedCrew=%2 deletedUnseated=%3 prunedMounted=%4", BuildPatrolContext(patrol), mountedCrew, deletedUnseated, prunedMounted));
			LogDebug(string.Format("Post mount validation %1 driver=1 mountedCrew=%2 deletedUnseated=%3 prunedMounted=%4", BuildPatrolContext(patrol), mountedCrew, deletedUnseated, prunedMounted));
		}
	}

	protected bool IsUnitInVehicle(IEntity unit, IEntity vehicle)
	{
		if (!unit || !vehicle)
			return false;

		if (!IsEntitySafeForMount(unit) || !IsEntitySafeForMount(vehicle))
			return false;

		SCR_CompartmentAccessComponent access = SCR_CompartmentAccessComponent.Cast(unit.FindComponent(SCR_CompartmentAccessComponent));
		if (!access)
			return false;

		IEntity currentVehicle = access.GetVehicle();
		if (!currentVehicle)
			return false;

		IEntity currentRoot = currentVehicle.GetRootParent();
		IEntity vehicleRoot = vehicle.GetRootParent();
		return currentVehicle == vehicle || currentRoot == vehicle || currentVehicle == vehicleRoot || currentRoot == vehicleRoot;
	}

	protected bool IsEntitySafeForMount(IEntity entity)
	{
		if (!entity)
			return false;

		if (RplComponent.Cast(entity.FindComponent(RplComponent)))
			return true;

		IEntity root = entity.GetRootParent();
		if (root && root != entity)
			return RplComponent.Cast(root.FindComponent(RplComponent)) != null;

		return false;
	}

	protected bool IsUnitInCompartmentType(IEntity unit, IEntity vehicle, ECompartmentType compartmentType)
	{
		if (!IsUnitInVehicle(unit, vehicle))
			return false;

		SCR_CompartmentAccessComponent access = SCR_CompartmentAccessComponent.Cast(unit.FindComponent(SCR_CompartmentAccessComponent));
		if (!access)
			return false;

		BaseCompartmentSlot compartment = access.GetCompartment();
		if (!compartment)
			return false;

		return compartment.GetType() == compartmentType;
	}

	protected bool IsUnitInGunnerRole(IEntity unit, IEntity vehicle)
	{
		if (!IsUnitInVehicle(unit, vehicle))
			return false;

		SCR_CompartmentAccessComponent access = SCR_CompartmentAccessComponent.Cast(unit.FindComponent(SCR_CompartmentAccessComponent));
		if (!access)
			return false;

		BaseCompartmentSlot compartment = access.GetCompartment();
		if (compartment)
		{
			ECompartmentType compartmentType = compartment.GetType();
			if (compartmentType == ECompartmentType.TURRET)
				return true;

			if (compartmentType == ECompartmentType.PILOT || compartmentType == ECompartmentType.CARGO)
				return false;
		}

		return access.GetControlledFireModeManager() != null;
	}

	protected bool RegisterUsableVehicle(SCR_AIGroup group, IEntity vehicle)
	{
		if (!group || !vehicle)
		{
			LogSmoke("Vehicle usage register result=false reason=missing_group_or_vehicle");
			return false;
		}

		SCR_AIGroupUtilityComponent utility = group.GetGroupUtilityComponent();
		if (!utility)
		{
			LogSmoke("Vehicle usage register result=false reason=missing_group_utility");
			return false;
		}

		IEntity usageOwner = null;
		SCR_AIVehicleUsageComponent vehicleUsage = SCR_AIVehicleUsageComponent.FindOnNearestParent(vehicle, usageOwner);
		if (!vehicleUsage || !vehicleUsage.CanBePiloted())
		{
			LogSmoke("Vehicle usage register result=false reason=vehicle_not_pilotable");
			return false;
		}

		utility.SetMaxAutonomousDistance(4500.0);
		utility.AddUsableVehicle(vehicleUsage);
		LogDebug("Vehicle usage register result=true");
		return true;
	}

	protected void ActivateGroupAI(SCR_AIGroup group)
	{
		if (!group)
			return;

		group.ActivateAI();
		//group.ActivateAllMembers();

		AIControlComponent control = group.GetControlComponent();
		if (control)
			control.ActivateAI();
	}

	protected void ActivatePatrolGroupAI(JLH_DCF_VehiclePatrolRuntime patrol)
	{
		if (!patrol || !patrol.CrewGroup)
			return;

		ActivateGroupAI(patrol.CrewGroup);
	}

	protected void RefreshPatrols()
	{
		int now = System.GetTickCount();
		for (int i = m_aPatrols.Count() - 1; i >= 0; i--)
		{
			JLH_DCF_VehiclePatrolRuntime patrol = m_aPatrols[i];
			if (!patrol || !patrol.VehicleEntity || IsEntityDestroyed(patrol.VehicleEntity))
			{
				if (patrol)
				{
					LogSmoke(string.Format("Armed vehicle fallback %1 reason=vehicle_destroyed", BuildPatrolContext(patrol)));
					LogSmoke(string.Format("Active patrol destroyed %1", BuildPatrolContext(patrol)));
					MarkPatrolCleared(patrol, "vehicle_destroyed");
					string vehicleDestroyedBlockReason;
					if (!CanCleanupVehiclePatrol(patrol, true, vehicleDestroyedBlockReason))
						continue;

					CleanupDestroyedPatrolRuntime(patrol);
				}

				m_aPatrols.Remove(i);
				ScheduleNextSpawn("vehicle_destroyed");
				continue;
			}

			if (!UpdatePatrolDriverState(patrol, now))
			{
				LogSmoke(string.Format("Patrol invalid %1 reason=no_driver_timeout status=confirmed_timeout", BuildPatrolContext(patrol)));
				LogSmoke(string.Format("Patrol cleanup started %1", BuildPatrolContext(patrol)));
				MarkPatrolCleared(patrol, "no_driver_timeout");
				string noDriverBlockReason;
				if (!CanCleanupVehiclePatrol(patrol, true, noDriverBlockReason))
					continue;

				CleanupPatrolRuntime(patrol, "no_driver_timeout");
				m_aPatrols.Remove(i);
				ScheduleNextSpawn("no_driver_timeout");
				continue;
			}

			ScanPatrolDeadBodies(patrol, now);
		}

		if (m_RuntimeTracker)
			m_RuntimeTracker.PruneMissing();
	}

	protected void ScanPatrolDeadBodies(JLH_DCF_VehiclePatrolRuntime patrol, int now)
	{
		if (!patrol || !patrol.CrewGroup)
			return;

		array<AIAgent> agents = {};
		patrol.CrewGroup.GetAgents(agents);

		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!unit || !IsCorpseEntity(unit))
				continue;

			if (patrol.QueuedDeadBodies.Contains(unit))
				continue;

			patrol.QueuedDeadBodies.Insert(unit);
			QueueCorpseCleanup(unit, patrol.PatrolId, patrol.LastCombatTick);
		}
	}

	protected bool IsEntityDestroyed(IEntity entity)
	{
		if (!entity)
			return true;

		DamageManagerComponent damageManager = DamageManagerComponent.Cast(entity.FindComponent(DamageManagerComponent));
		if (!damageManager)
			return false;

		if (damageManager.IsDestroyed())
			return true;

		if (damageManager.GetState() == EDamageState.DESTROYED)
			return true;

		if (damageManager.GetHealthScaled() <= 0.0)
			return true;

		return false;
	}

	protected void MarkPatrolCleared(notnull JLH_DCF_VehiclePatrolRuntime patrol, string reason)
	{
		if (patrol.m_bCleared)
			return;

		patrol.m_bCleared = true;
		patrol.m_iClearedTick = System.GetTickCount();
		JLH_CleanupSafetyService.LogPackageEnteredClearedState(BuildVehiclePatrolCleanupLabel(patrol, reason), m_bDebugLogging);
	}

	protected bool CanCleanupVehiclePatrol(notnull JLH_DCF_VehiclePatrolRuntime patrol, bool requireLinger, out string blockedReason)
	{
		vector anchor = "0 0 0";
		if (patrol.VehicleEntity)
			anchor = patrol.VehicleEntity.GetOrigin();
		else if (patrol.CrewGroup)
			anchor = patrol.CrewGroup.GetOrigin();
		else if (GetOwner())
			anchor = GetOwner().GetOrigin();

		JLH_CleanupSafetyQuery query = new JLH_CleanupSafetyQuery();
		query.Init(BuildVehiclePatrolCleanupLabel(patrol, ""), anchor);
		query.RequireLinger = requireLinger;
		query.ClearedTick = patrol.m_iClearedTick;
		query.LingerSeconds = JLH_CleanupSafetyService.DEFAULT_PACKAGE_LINGER_SECONDS;
		query.CleanupDistance = Math.Max(m_fLiveCleanupPlayerSafeDistance, JLH_CleanupSafetyService.MIN_CLEANUP_DISTANCE_METERS);
		query.VisibilityDistance = Math.Max(m_fLiveCleanupLineOfSightRejectDistance, JLH_CleanupSafetyService.DEFAULT_VISIBILITY_DISTANCE_METERS);
		query.LastInteractionTick = patrol.LastCombatTick;
		query.PlayerFactionFilter = "";
		query.DebugEnabled = m_bDebugLogging;
		if (patrol.CrewGroup)
			query.Groups.Insert(patrol.CrewGroup);
		if (patrol.VehicleEntity)
			query.Vehicles.Insert(patrol.VehicleEntity);
		foreach (IEntity waypoint : patrol.RouteWaypoints)
		{
			if (waypoint)
				query.Waypoints.Insert(waypoint);
		}

		return JLH_CleanupSafetyService.CanCleanupPackage(query, blockedReason);
	}

	protected bool CanCleanupNodeRuntimePackage(bool requireLinger, out string blockedReason)
	{
		ref array<IEntity> players = {};
		JLH_AddonWorldQuery.GatherLiveDeployedPlayers(players, "");
		return CanCleanupNodeRuntimePackageWithPlayers(requireLinger, players, blockedReason);
	}

	protected bool CanCleanupNodeRuntimePackageWithPlayers(bool requireLinger, notnull array<IEntity> players, out string blockedReason)
	{
		vector anchor;
		string anchorSource;
		if (!TryResolveLiveCleanupPosition(anchor, anchorSource))
		{
			if (GetOwner())
				anchor = GetOwner().GetOrigin();
			else
				anchor = "0 0 0";
		}

		JLH_CleanupSafetyQuery query = new JLH_CleanupSafetyQuery();
		query.Init("VehiclePatrolNode node=" + JLH_DCF_NodeDebug.EntityLabel(GetOwner()), anchor);
		query.RequireLinger = requireLinger;
		query.LingerSeconds = JLH_CleanupSafetyService.DEFAULT_PACKAGE_LINGER_SECONDS;
		query.CleanupDistance = Math.Max(m_fLiveCleanupPlayerSafeDistance, JLH_CleanupSafetyService.MIN_CLEANUP_DISTANCE_METERS);
		query.VisibilityDistance = Math.Max(m_fLiveCleanupLineOfSightRejectDistance, JLH_CleanupSafetyService.DEFAULT_VISIBILITY_DISTANCE_METERS);
		query.PlayerFactionFilter = "";
		query.DebugEnabled = m_bDebugLogging;
		if (m_RuntimeTracker)
			m_RuntimeTracker.CollectTrackedState(query.Groups, query.Entities, query.Vehicles);
		if (m_PendingMount)
		{
			if (m_PendingMount.CrewGroup && !query.Groups.Contains(m_PendingMount.CrewGroup))
				query.Groups.Insert(m_PendingMount.CrewGroup);
			if (m_PendingMount.VehicleEntity && !query.Vehicles.Contains(m_PendingMount.VehicleEntity))
				query.Vehicles.Insert(m_PendingMount.VehicleEntity);
		}

		return JLH_CleanupSafetyService.CanCleanupPackageWithPlayers(query, players, blockedReason);
	}

	protected string BuildVehiclePatrolCleanupLabel(notnull JLH_DCF_VehiclePatrolRuntime patrol, string reason)
	{
		string label = "VehiclePatrol patrol=" + patrol.PatrolId.ToString();
		if (reason != "")
			label += " reason=" + reason;

		return label;
	}

	protected void CleanupAllRuntime(string reason, bool scheduleRespawn)
	{
		if (m_PendingMount)
		{
			CleanupGroupVehicle(m_PendingMount.CrewGroup, m_PendingMount.VehicleEntity, reason);
			m_PendingMount = null;
		}

		for (int i = m_aPatrols.Count() - 1; i >= 0; i--)
		{
			JLH_DCF_VehiclePatrolRuntime patrol = m_aPatrols[i];
			if (patrol)
				CleanupPatrolRuntime(patrol, reason);

			m_aPatrols.Remove(i);
		}

		if (m_RuntimeTracker)
			m_RuntimeTracker.ForgetAll();

		LogDebug("Cleanup complete");

		if (scheduleRespawn)
			ScheduleNextSpawn(reason);
	}

	protected void CleanupPatrolRuntime(JLH_DCF_VehiclePatrolRuntime patrol, string reason)
	{
		if (!patrol)
			return;

		CleanupRouteWaypoints(patrol.CrewGroup, patrol.RouteWaypoints);
		CleanupGroupVehicleOwned(patrol.CrewGroup, patrol.VehicleEntity, reason, patrol.PatrolId, patrol.LastCombatTick);

		if (m_RuntimeTracker)
			m_RuntimeTracker.ForgetAll();
	}

	protected void CleanupDestroyedPatrolRuntime(JLH_DCF_VehiclePatrolRuntime patrol)
	{
		if (!patrol)
			return;

		if (patrol.LastCombatTick <= 0)
			patrol.LastCombatTick = System.GetTickCount();

		IEntity wreck = patrol.VehicleEntity;
		CleanupRouteWaypoints(patrol.CrewGroup, patrol.RouteWaypoints);
		CleanupGroupVehicleOwned(patrol.CrewGroup, null, "vehicle_destroyed", patrol.PatrolId, patrol.LastCombatTick);
		QueueWreckCleanup(wreck);

		if (m_RuntimeTracker)
			m_RuntimeTracker.ForgetAll();
	}

	protected void QueueWreckCleanup(IEntity wreck)
	{
		if (!wreck)
			return;

		foreach (JLH_DCF_VehicleWreckCleanupState existing : m_aDeferredWrecks)
		{
			if (existing && existing.VehicleEntity == wreck)
				return;
		}

		int now = System.GetTickCount();
		JLH_DCF_VehicleWreckCleanupState cleanupState = new JLH_DCF_VehicleWreckCleanupState();
		cleanupState.Init(wreck, now, WRECK_CLEANUP_GRACE_MS);
		m_aDeferredWrecks.Insert(cleanupState);
		LogSmoke("Wreck cleanup deferred reason=recent_destroyed");
	}

	protected void ProcessDeferredWreckCleanup()
	{
		int now = System.GetTickCount();
		for (int i = m_aDeferredWrecks.Count() - 1; i >= 0; i--)
		{
			JLH_DCF_VehicleWreckCleanupState cleanupState = m_aDeferredWrecks[i];
			if (!cleanupState || !cleanupState.VehicleEntity)
			{
				m_aDeferredWrecks.Remove(i);
				continue;
			}

			if (cleanupState.NextAttemptTick > now)
				continue;

			string deferReason;
			if (!IsWreckCleanupSafe(cleanupState, now, deferReason))
			{
				LogSmoke(string.Format("Wreck cleanup deferred reason=%1", deferReason));
				cleanupState.NextAttemptTick = now + WRECK_CLEANUP_RETRY_MS;
				continue;
			}

			DeleteEntitySafe(cleanupState.VehicleEntity);
			LogSmoke("Wreck cleanup safe removed");
			m_aDeferredWrecks.Remove(i);
		}
	}

	protected bool IsWreckCleanupSafe(JLH_DCF_VehicleWreckCleanupState cleanupState, int now, out string deferReason)
	{
		deferReason = "";
		if (!cleanupState || !cleanupState.VehicleEntity)
			return true;

		if (now - cleanupState.DestroyedTick < WRECK_CLEANUP_GRACE_MS)
		{
			deferReason = "recent_destroyed";
			return false;
		}

		vector wreckPosition = cleanupState.VehicleEntity.GetOrigin();
		cleanupState.LastPosition = wreckPosition;

		JLH_CleanupSafetyQuery query = new JLH_CleanupSafetyQuery();
		query.Init("VehiclePatrolWreck", wreckPosition);
		query.CleanupDistance = WRECK_CLEANUP_PLAYER_NEAR_DISTANCE;
		query.VisibilityDistance = Math.Max(WRECK_CLEANUP_LOS_DISTANCE, JLH_CleanupSafetyService.DEFAULT_VISIBILITY_DISTANCE_METERS);
		query.PlayerFactionFilter = "";
		query.DebugEnabled = m_bDebugLogging;
		query.Vehicles.Insert(cleanupState.VehicleEntity);
		string blockedReason;
		if (!JLH_CleanupSafetyService.CanCleanupPackage(query, blockedReason))
		{
			deferReason = blockedReason;
			return false;
		}

		if (HasDeferredCorpseCleanupNearPosition(wreckPosition, 12.0))
		{
			deferReason = "dead_body_pending";
			return false;
		}

		return true;
	}

	protected void QueueCorpseCleanup(IEntity corpse, int patrolId, int recentCombatTick)
	{
		if (!corpse)
			return;

		if (!m_bDeadBodyCleanupEnabled)
		{
			if (!m_bDeadBodyCleanupDisabledLogged)
			{
				m_bDeadBodyCleanupDisabledLogged = true;
				LogSmoke("Dead body cleanup skipped reason=disabled");
			}

			return;
		}

		foreach (JLH_DCF_VehicleCorpseCleanupState existing : m_aDeferredCorpses)
		{
			if (existing && existing.CorpseEntity == corpse)
			{
				if (recentCombatTick > existing.RecentCombatTick)
					existing.RecentCombatTick = recentCombatTick;

				JLH_DCF_RuntimeCleanupManager.UnregisterTree(corpse);
				return;
			}
		}

		int now = System.GetTickCount();
		JLH_DCF_VehicleCorpseCleanupState cleanupState = new JLH_DCF_VehicleCorpseCleanupState();
		cleanupState.Init(corpse, now, GetDeadBodyCleanupDelayMs(), patrolId, recentCombatTick);
		m_aDeferredCorpses.Insert(cleanupState);
		JLH_DCF_RuntimeCleanupManager.UnregisterTree(corpse);
		LogSmoke(string.Format("Dead body registered patrol=%1 entity=%2 delay=%3", patrolId, JLH_DCF_NodeDebug.EntityLabel(corpse), m_fDeadBodyCleanupDelaySeconds));
	}

	protected void ProcessDeferredCorpseCleanup()
	{
		if (!m_bDeadBodyCleanupEnabled)
		{
			if (m_aDeferredCorpses.Count() > 0 && !m_bDeadBodyCleanupDisabledLogged)
			{
				m_bDeadBodyCleanupDisabledLogged = true;
				LogSmoke("Dead body cleanup skipped reason=disabled");
			}

			return;
		}

		int now = System.GetTickCount();
		for (int i = m_aDeferredCorpses.Count() - 1; i >= 0; i--)
		{
			JLH_DCF_VehicleCorpseCleanupState cleanupState = m_aDeferredCorpses[i];
			if (!cleanupState || !cleanupState.CorpseEntity)
			{
				m_aDeferredCorpses.Remove(i);
				continue;
			}

			if (cleanupState.NextAttemptTick > now)
				continue;

			string deferReason;
			if (!IsCorpseCleanupSafe(cleanupState, now, deferReason))
			{
				LogSmoke(string.Format("Dead body cleanup deferred reason=%1", deferReason));
				cleanupState.NextAttemptTick = now + CORPSE_CLEANUP_RETRY_MS;
				continue;
			}

			DeleteEntitySafe(cleanupState.CorpseEntity);
			LogSmoke(string.Format("Dead body cleanup removed patrol=%1 count=1", cleanupState.PatrolId));
			m_aDeferredCorpses.Remove(i);
		}
	}

	protected bool IsCorpseCleanupSafe(JLH_DCF_VehicleCorpseCleanupState cleanupState, int now, out string deferReason)
	{
		deferReason = "";
		if (!cleanupState || !cleanupState.CorpseEntity)
			return true;

		if (!cleanupState.VehiclePatrolOwned)
		{
			deferReason = "invalid_owner";
			return false;
		}

		if (!IsCorpseEntity(cleanupState.CorpseEntity))
		{
			deferReason = "not_dead";
			return false;
		}

		if (now - cleanupState.DeathTick < GetDeadBodyCleanupDelayMs())
		{
			deferReason = "recent_death";
			return false;
		}

		vector corpsePosition = cleanupState.CorpseEntity.GetOrigin();
		cleanupState.LastPosition = corpsePosition;

		if (cleanupState.RecentCombatTick > 0 && now - cleanupState.RecentCombatTick < GetDeadBodyRecentCombatDelayMs())
		{
			deferReason = "recent_combat";
			return false;
		}

		if (IsRecentVehiclePatrolCombatNearPosition(corpsePosition, now))
		{
			deferReason = "recent_combat";
			return false;
		}

		if (IsCorpseInsideActiveVehicle(cleanupState.CorpseEntity))
		{
			deferReason = "in_vehicle";
			return false;
		}

		JLH_CleanupSafetyQuery query = new JLH_CleanupSafetyQuery();
		query.Init("VehiclePatrolCorpse patrol=" + cleanupState.PatrolId.ToString(), corpsePosition);
		query.CleanupDistance = m_fDeadBodyCleanupPlayerSafeDistance;
		query.VisibilityDistance = Math.Max(CORPSE_CLEANUP_LOS_DISTANCE, JLH_CleanupSafetyService.DEFAULT_VISIBILITY_DISTANCE_METERS);
		query.LastInteractionTick = cleanupState.RecentCombatTick;
		query.PlayerFactionFilter = "";
		query.DebugEnabled = m_bDebugLogging;
		query.Entities.Insert(cleanupState.CorpseEntity);
		string blockedReason;
		if (!JLH_CleanupSafetyService.CanCleanupPackage(query, blockedReason))
		{
			deferReason = blockedReason;
			return false;
		}

		return true;
	}

	protected int GetDeadBodyCleanupDelayMs()
	{
		return SecondsToMs(m_fDeadBodyCleanupDelaySeconds);
	}

	protected int GetDeadBodyRecentCombatDelayMs()
	{
		return SecondsToMs(m_fDeadBodyCleanupRecentCombatDelaySeconds);
	}

	protected bool IsRecentVehiclePatrolCombatNearPosition(vector position, int now)
	{
		int recentCombatDelayMs = GetDeadBodyRecentCombatDelayMs();
		if (recentCombatDelayMs <= 0)
			return false;

		foreach (JLH_DCF_VehiclePatrolRuntime patrol : m_aPatrols)
		{
			if (!patrol || patrol.LastCombatTick <= 0)
				continue;

			if (now - patrol.LastCombatTick >= recentCombatDelayMs)
				continue;

			if (patrol.VehicleEntity && vector.Distance(position, patrol.VehicleEntity.GetOrigin()) <= m_fDeadBodyCleanupPlayerSafeDistance)
				return true;
		}

		return false;
	}

	protected bool IsCorpseInsideActiveVehicle(IEntity corpse)
	{
		if (!corpse)
			return false;

		foreach (JLH_DCF_VehiclePatrolRuntime patrol : m_aPatrols)
		{
			if (!patrol || !patrol.VehicleEntity || IsEntityDestroyed(patrol.VehicleEntity))
				continue;

			if (IsUnitInVehicle(corpse, patrol.VehicleEntity))
				return true;
		}

		return false;
	}

	protected bool HasDeferredCorpseCleanupNearPosition(vector position, float distanceLimit)
	{
		foreach (JLH_DCF_VehicleCorpseCleanupState cleanupState : m_aDeferredCorpses)
		{
			if (!cleanupState || !cleanupState.CorpseEntity)
				continue;

			if (vector.Distance(position, cleanupState.CorpseEntity.GetOrigin()) <= distanceLimit)
				return true;
		}

		return false;
	}

	protected void CleanupRouteWaypoints(SCR_AIGroup group, array<IEntity> routeWaypoints)
	{
		if (!routeWaypoints)
			return;

		foreach (IEntity waypointEntity : routeWaypoints)
		{
			if (!waypointEntity)
				continue;

			AIWaypoint waypoint = AIWaypoint.Cast(waypointEntity);
			if (waypoint && group)
				group.RemoveWaypoint(waypoint);

			SCR_EntityHelper.DeleteEntityAndChildren(waypointEntity);
		}

		routeWaypoints.Clear();
	}

	protected void CleanupGroupVehicle(SCR_AIGroup group, IEntity vehicle, string reason)
	{
		CleanupGroupVehicleOwned(group, vehicle, reason, 0, 0);
	}

	protected void CleanupGroupVehicleOwned(SCR_AIGroup group, IEntity vehicle, string reason, int patrolId, int recentCombatTick)
	{
		int deletedCrewUnits = DeleteGroupUnitsOwned(group, patrolId, recentCombatTick);

		if (group)
			DeleteEntitySafe(group);

		if (vehicle)
		{
			JLH_DCF_VehicleSustainment.UnregisterVehicle(vehicle, reason);
			DeleteEntitySafe(vehicle);
		}

		LogDebug(string.Format("Cleanup complete reason=%1 group=%2 vehicle=%3 crewUnits=%4", reason, JLH_DCF_NodeDebug.EntityLabel(group), JLH_DCF_NodeDebug.EntityLabel(vehicle), deletedCrewUnits));
	}

	protected int DeleteGroupUnitsOwned(SCR_AIGroup group, int patrolId, int recentCombatTick)
	{
		if (!group)
			return 0;

		array<AIAgent> agents = {};
		group.GetAgents(agents);

		int deleted = 0;
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!unit)
				continue;

			if (IsCorpseEntity(unit))
			{
				QueueCorpseCleanup(unit, patrolId, recentCombatTick);
				continue;
			}

			DeleteEntitySafe(unit);
			deleted++;
		}

		return deleted;
	}

	protected bool IsCorpseEntity(IEntity entity)
	{
		if (!entity)
			return false;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(entity);
		if (character && character.GetCharacterController())
			return character.GetCharacterController().IsDead();

		return IsEntityDestroyed(entity);
	}

	protected void DeleteEntitySafe(IEntity entity)
	{
		if (!entity)
			return;

		JLH_DCF_RuntimeCleanupManager.UnregisterTree(entity);
		JLH_AddonRuntimeEntityMetadata.ForgetTree(entity);
		JLH_DCF_PersistenceExclusion.StopTrackingTreeNow(entity, "vehicle_patrol_local_delete");
		SCR_EntityHelper.DeleteEntityAndChildren(entity);
	}

	void CleanupOwnedRuntime(string reason)
	{
		CleanupAllRuntime(reason, true);
	}

	bool IsRuntimeEntityCleanupProtected(IEntity entity, JLH_DCF_RuntimeCleanupEntityKind kind, out string reason)
	{
		reason = "";
		if (!entity)
			return false;

		if (m_PendingMount)
		{
			if (m_PendingMount.VehicleEntity == entity || m_PendingMount.CrewGroup == entity || IsGroupMember(m_PendingMount.CrewGroup, entity))
			{
				reason = "vehicle_patrol_pending_mount";
				return true;
			}
		}

		foreach (JLH_DCF_VehicleWreckCleanupState wreckState : m_aDeferredWrecks)
		{
			if (wreckState && wreckState.VehicleEntity == entity)
			{
				reason = "vehicle_patrol_local_wreck_cleanup";
				return true;
			}
		}

		foreach (JLH_DCF_VehicleCorpseCleanupState corpseState : m_aDeferredCorpses)
		{
			if (corpseState && corpseState.CorpseEntity == entity)
			{
				reason = "vehicle_patrol_local_corpse_cleanup";
				return true;
			}
		}

		foreach (JLH_DCF_VehiclePatrolRuntime patrol : m_aPatrols)
		{
			if (!patrol)
				continue;

			if (patrol.VehicleEntity == entity)
			{
				if (!IsEntityDestroyed(entity))
				{
					reason = "vehicle_patrol_active_vehicle";
					return true;
				}

				reason = "vehicle_patrol_destroyed_vehicle_local_cleanup";
				return true;
			}

			if (patrol.CrewGroup == entity)
			{
				reason = "vehicle_patrol_active_group";
				return true;
			}

			if (IsGroupMember(patrol.CrewGroup, entity))
			{
				if (IsCorpseEntity(entity))
				{
					if (m_bDeadBodyCleanupEnabled)
					{
						reason = "vehicle_patrol_dead_body_local_cleanup";
						return true;
					}

					return false;
				}

				reason = "vehicle_patrol_active_crew";
				return true;
			}

			if (patrol.RouteWaypoints && patrol.RouteWaypoints.Contains(entity))
			{
				reason = "vehicle_patrol_route_waypoint";
				return true;
			}
		}

		return false;
	}

	bool IsRegistered()
	{
		return m_bRegistered && !m_bDisabled;
	}

	protected bool IsGroupMember(SCR_AIGroup group, IEntity entity)
	{
		if (!group || !entity)
			return false;

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (agent && agent.GetControlledEntity() == entity)
				return true;
		}

		return false;
	}

	protected bool IsPlayerWithinSpawnDistance(out float closestDistance)
	{
		closestDistance = GetClosestValidPlayerDistance();
		bool inRange = closestDistance >= 0.0 && closestDistance <= m_fSpawnDistance;
		LogSpawnDistanceCheck(inRange, closestDistance);
		if (!inRange)
			return false;

		IEntity owner = GetOwner();
		if (!owner)
			return false;

		return !IsSpawnBlockedByPlayerSafety(owner.GetOrigin());
	}

	protected bool IsSpawnBlockedByPlayerSafety(vector spawnPosition)
	{
		ref array<IEntity> players = {};
		JLH_AddonWorldQuery.GatherLiveDeployedPlayers(players, "");

		if (m_fSpawnMinimumPlayerDistance > 0.0 && JLH_AddonWorldQuery.HasLiveDeployedPlayerWithinPlayers(spawnPosition, m_fSpawnMinimumPlayerDistance, players))
		{
			float nearestDistance = JLH_AddonWorldQuery.GetNearestLivePlayerDistanceFromPlayers(spawnPosition, players);
			LogSmoke(string.Format("spawn_blocked_player_too_close distance=%1 limit=%2 position=%3", nearestDistance, m_fSpawnMinimumPlayerDistance, spawnPosition));
			return true;
		}

		if (m_fSpawnLineOfSightRejectDistance > 0.0 && JLH_AddonWorldQuery.HasLineOfSightFromAnyPlayer(spawnPosition, m_fSpawnLineOfSightRejectDistance, players))
		{
			LogSmoke(string.Format("spawn_blocked_player_los range=%1 position=%2", m_fSpawnLineOfSightRejectDistance, spawnPosition));
			return true;
		}

		return false;
	}

	protected void LogSpawnDistanceCheck(bool inRange, float closestDistance)
	{
		int now = System.GetTickCount();
		string message = string.Format("Spawn check playerInRange=%1 distance=%2", JLH_DCF_NodeDebug.BoolLabel(inRange), closestDistance);

		if (!m_bSpawnRangeLogStateInitialized)
		{
			m_bSpawnRangeLogStateInitialized = true;
			m_bLastSpawnRangeInRange = inRange;
			m_iNextSpawnRangeDebugLogTick = now + SPAWN_RANGE_LOG_THROTTLE_MS;
			if (inRange)
				LogSmoke(message);
			else
				LogDebug(message);
			return;
		}

		if (inRange != m_bLastSpawnRangeInRange)
		{
			m_bLastSpawnRangeInRange = inRange;
			m_iNextSpawnRangeDebugLogTick = now + SPAWN_RANGE_LOG_THROTTLE_MS;
			if (inRange)
				LogSmoke(message);
			else
				LogDebug(message);
			return;
		}

		if (!inRange && m_bDebugLogging && now >= m_iNextSpawnRangeDebugLogTick)
		{
			m_iNextSpawnRangeDebugLogTick = now + SPAWN_RANGE_LOG_THROTTLE_MS;
			LogDebug(message);
		}
	}

	protected bool AreAllPlayersBeyondDespawnDistance(out float closestDistance)
	{
		closestDistance = GetClosestValidPlayerDistance();
		if (closestDistance < 0.0)
			return true;

		return closestDistance > GetEffectiveDespawnDistance();
	}

	protected bool AreAllLivePlayersBeyondPosition(vector position, float distanceLimit, out float closestDistance)
	{
		ref array<IEntity> players = {};
		JLH_AddonWorldQuery.GatherLiveDeployedPlayers(players, "");
		return AreAllLivePlayersBeyondPositionWithPlayers(position, distanceLimit, players, closestDistance);
	}

	protected bool AreAllLivePlayersBeyondPositionWithPlayers(vector position, float distanceLimit, notnull array<IEntity> players, out float closestDistance)
	{
		closestDistance = -1.0;
		foreach (IEntity player : players)
		{
			if (!player)
				continue;

			float distance = vector.Distance(position, player.GetOrigin());
			if (closestDistance < 0.0 || distance < closestDistance)
				closestDistance = distance;
		}

		if (closestDistance < 0.0)
			return true;

		return closestDistance > distanceLimit;
	}

	protected bool IsLiveCleanupBlockedByPlayerSafety(vector cleanupPosition)
	{
		ref array<IEntity> players = {};
		JLH_AddonWorldQuery.GatherLiveDeployedPlayers(players, "");

		if (m_fLiveCleanupPlayerSafeDistance > 0.0 && JLH_AddonWorldQuery.HasLiveDeployedPlayerWithinPlayers(cleanupPosition, m_fLiveCleanupPlayerSafeDistance, players))
		{
			float nearestDistance = JLH_AddonWorldQuery.GetNearestLivePlayerDistanceFromPlayers(cleanupPosition, players);
			LogSmoke(string.Format("live_cleanup_blocked_player_near distance=%1 limit=%2 position=%3", nearestDistance, m_fLiveCleanupPlayerSafeDistance, cleanupPosition));
			return true;
		}

		if (m_fLiveCleanupLineOfSightRejectDistance > 0.0 && JLH_AddonWorldQuery.HasLineOfSightFromAnyPlayer(cleanupPosition, m_fLiveCleanupLineOfSightRejectDistance, players))
		{
			LogSmoke(string.Format("live_cleanup_blocked_player_los range=%1 position=%2", m_fLiveCleanupLineOfSightRejectDistance, cleanupPosition));
			return true;
		}

		return false;
	}

	protected bool TryResolveLiveCleanupPosition(out vector cleanupPosition, out string source)
	{
		cleanupPosition = "0 0 0";
		source = "";

		if (m_PendingMount)
		{
			if (m_PendingMount.VehicleEntity && !IsEntityDestroyed(m_PendingMount.VehicleEntity))
			{
				cleanupPosition = m_PendingMount.VehicleEntity.GetOrigin();
				source = "vehicle";
				return true;
			}

			if (TryGetGroupRuntimePosition(m_PendingMount.CrewGroup, cleanupPosition))
			{
				source = "group";
				return true;
			}
		}

		foreach (JLH_DCF_VehiclePatrolRuntime patrol : m_aPatrols)
		{
			if (!patrol)
				continue;

			if (patrol.VehicleEntity && !IsEntityDestroyed(patrol.VehicleEntity))
			{
				cleanupPosition = patrol.VehicleEntity.GetOrigin();
				source = "vehicle";
				return true;
			}
		}

		foreach (JLH_DCF_VehiclePatrolRuntime groupPatrol : m_aPatrols)
		{
			if (!groupPatrol)
				continue;

			if (TryGetGroupRuntimePosition(groupPatrol.CrewGroup, cleanupPosition))
			{
				source = "group";
				return true;
			}
		}

		IEntity owner = GetOwner();
		if (owner)
		{
			cleanupPosition = owner.GetOrigin();
			source = "node";
			return true;
		}

		return false;
	}

	protected bool TryGetGroupRuntimePosition(SCR_AIGroup group, out vector position)
	{
		position = "0 0 0";
		if (!group)
			return false;

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!unit || IsCorpseEntity(unit) || IsEntityDestroyed(unit))
				continue;

			position = unit.GetOrigin();
			return true;
		}

		position = group.GetOrigin();
		return true;
	}

	protected void LogLiveCleanupPositionSource(string source, vector cleanupPosition)
	{
		if (source == "vehicle")
		{
			LogDebug(string.Format("live_cleanup_using_vehicle_position position=%1", cleanupPosition));
			return;
		}

		if (source == "node")
			LogDebug(string.Format("live_cleanup_using_node_fallback position=%1", cleanupPosition));
	}

	protected bool AreAllValidPlayersBeyondPosition(vector position, float distanceLimit)
	{
		PlayerManager playerManager = GetPlayerManagerSafe();
		if (!playerManager)
			return true;

		ref array<int> playerIds = {};
		playerManager.GetPlayers(playerIds);

		foreach (int playerId : playerIds)
		{
			IEntity controlled = playerManager.GetPlayerControlledEntity(playerId);
			if (!IsValidPlayerEntity(controlled))
				continue;

			if (vector.Distance(position, controlled.GetOrigin()) <= distanceLimit)
				return false;
		}

		return true;
	}

	protected bool HasAnyValidPlayerLineOfSight(vector position, float distanceLimit)
	{
		PlayerManager playerManager = GetPlayerManagerSafe();
		if (!playerManager)
			return false;

		ref array<int> playerIds = {};
		playerManager.GetPlayers(playerIds);

		foreach (int playerId : playerIds)
		{
			IEntity controlled = playerManager.GetPlayerControlledEntity(playerId);
			if (!IsValidPlayerEntity(controlled))
				continue;

			if (vector.Distance(position, controlled.GetOrigin()) > distanceLimit)
				continue;

			if (HasDirectLineOfSight(controlled, position))
				return true;
		}

		return false;
	}

	protected bool HasDirectLineOfSight(IEntity playerEntity, vector position)
	{
		if (!playerEntity || !GetGame() || !GetGame().GetWorld())
			return false;

		TraceParam trace = new TraceParam();
		trace.Start = playerEntity.GetOrigin() + Vector(0, 1.6, 0);
		trace.End = position + Vector(0, 1.2, 0);
		trace.Exclude = playerEntity;
		trace.Flags = TraceFlags.DEFAULT | TraceFlags.ANY_CONTACT;
		trace.LayerMask = EPhysicsLayerDefs.Projectile;

		float traceDistance = GetGame().GetWorld().TraceMove(trace, null);
		return traceDistance >= 0.98;
	}

	protected PlayerManager GetPlayerManagerSafe()
	{
		if (!GetGame())
			return null;

		return GetGame().GetPlayerManager();
	}

	protected float GetClosestValidPlayerDistance()
	{
		IEntity owner = GetOwner();
		if (!owner || !GetGame())
			return -1.0;

		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return -1.0;

		ref array<int> playerIds = {};
		playerManager.GetPlayers(playerIds);

		float closest = -1.0;
		vector origin = owner.GetOrigin();
		foreach (int playerId : playerIds)
		{
			IEntity controlled = playerManager.GetPlayerControlledEntity(playerId);
			if (!IsValidPlayerEntity(controlled))
				continue;

			float distance = vector.Distance(origin, controlled.GetOrigin());
			if (closest < 0.0 || distance < closest)
				closest = distance;
		}

		return closest;
	}

	protected bool IsValidPlayerEntity(IEntity entity)
	{
		if (!entity)
			return false;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(entity);
		if (!character)
			return true;

		if (!character.GetCharacterController())
			return true;

		return !character.GetCharacterController().IsDead();
	}

	protected float GetEffectiveDespawnDistance()
	{
		if (m_fDespawnDistance < m_fSpawnDistance)
			return m_fSpawnDistance;

		return m_fDespawnDistance;
	}

	protected void DisableNode(string reason)
	{
		m_bDisabled = true;
		LogDebug(string.Format("Disabled node name=%1 reason=%2", JLH_DCF_NodeDebug.EntityLabel(GetOwner()), reason));
	}

	protected string BuildPatrolContext(JLH_DCF_VehiclePatrolRuntime patrol)
	{
		if (!patrol)
			return string.Format("node=%1 patrol=NONE vehicle=NONE", JLH_DCF_NodeDebug.EntityLabel(GetOwner()));

		string context = string.Format("node=%1 patrol=%2 vehicle=%3", JLH_DCF_NodeDebug.EntityLabel(GetOwner()), patrol.PatrolId, JLH_DCF_NodeDebug.EntityLabel(patrol.VehicleEntity));
		context += string.Format(" vehiclePos=%1 crewGroup=%2 driver=%3 gunner=%4", GetEntityPositionLabel(patrol.VehicleEntity), JLH_DCF_NodeDebug.EntityLabel(patrol.CrewGroup), JLH_DCF_NodeDebug.EntityLabel(patrol.DriverEntity), JLH_DCF_NodeDebug.EntityLabel(patrol.GunnerEntity));
		context += string.Format(" state=%1 attackCapable=%2 recoveryAttempts=%3 driverLostMs=%4", GetPatrolStateLabel(patrol.PatrolState), JLH_DCF_NodeDebug.BoolLabel(patrol.AttackCapable), patrol.MountedRecoveryAttempts, GetDriverLostAgeMs(patrol));
		return context;
	}

	protected string GetPatrolStateLabel(JLH_DCF_VehiclePatrolState state)
	{
		if (state == JLH_DCF_VehiclePatrolState.PATROLLING)
			return "PATROLLING";

		if (state == JLH_DCF_VehiclePatrolState.COMBAT_PAUSED)
			return "COMBAT_PAUSED";

		if (state == JLH_DCF_VehiclePatrolState.DISMOUNTED_FALLBACK)
			return "DISMOUNTED_FALLBACK";

		return "UNKNOWN";
	}

	protected int GetDriverLostAgeMs(JLH_DCF_VehiclePatrolRuntime patrol)
	{
		if (!patrol || patrol.DriverLostSinceTick <= 0)
			return 0;

		int age = System.GetTickCount() - patrol.DriverLostSinceTick;
		if (age < 0)
			return 0;

		return age;
	}

	protected string GetEntityPositionLabel(IEntity entity)
	{
		if (!entity)
			return "NONE";

		return entity.GetOrigin().ToString();
	}

	protected void LogDebug(string message)
	{
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, message, m_bDebugLogging);
	}

	protected void LogSmoke(string message)
	{
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, message, true);
	}

	protected bool RefreshNodeFaction(IEntity owner)
	{
		m_sFactionKey = JLH_DCF_NodeFactionResolver.GetFactionKey(owner);
		if (m_sFactionKey == "" || m_sFactionKey == "NONE" || m_sFactionKey == "UNKNOWN")
		{
			LogDebug(string.Format("Rejected: faction unresolved from SCR_FactionAffiliationComponent node=%1 faction=%2", JLH_DCF_NodeDebug.EntityLabel(owner), m_sFactionKey));
			return false;
		}

		return true;
	}

	protected bool IsEmptyResource(ResourceName prefab)
	{
		return prefab == "" || prefab == ResourceName.Empty;
	}

	protected int SecondsToMs(float seconds)
	{
		if (seconds <= 0)
			return 0;

		return (int)(seconds * 1000.0);
	}

	protected string VehicleSelectionSourceLabel()
	{
		if (!IsEmptyResource(m_sVehiclePrefabOverride))
			return "PREFAB_OVERRIDE";

		return "LABEL_FILTERS";
	}
}

void JLH_DCF_VehiclePatrolNodeComponent_FinalizePendingMount(JLH_VehiclePatrolNodeComponent node)
{
	if (node)
		node.FinalizePendingMount();
}

void JLH_DCF_VehiclePatrolNodeComponent_ValidateMountedPatrols(JLH_VehiclePatrolNodeComponent node)
{
	if (node)
		node.ValidateMountedPatrols();
}
