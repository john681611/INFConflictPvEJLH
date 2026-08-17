enum JLH_DCF_RecapForcePreset
{
	LIGHT_PROBE = 0,
	STANDARD_ASSAULT = 1,
	HEAVY_ASSAULT = 2,
	RANDOM = 3,
	CUSTOM_MANUAL = 4,
	TRANSPORT_ASSAULT = 5
}

enum JLH_DCF_RecapVehicleState
{
	ACTIVE_SUPPORT = 0,
	SUPPORT_COMPLETE = 1,
	SAFE_TO_REMOVE = 2
}

class JLH_DCF_RecapVehicleRuntime
{
	IEntity VehicleEntity;
	SCR_AIGroup CrewGroup;
	IEntity DriverEntity;
	IEntity GunnerEntity;
	ResourceName VehiclePrefab;
	ResourceName CrewPrefab;
	vector SpawnPosition;
	vector TargetPosition;
	vector AssignedWaypointPosition;
	vector DropPosition;
	vector TransportCapCenter;
	float LastDistanceToTarget;
	float ProgressBaselineDistance;
	int SpawnTick;
	int VehicleIndex;
	int MountReadyTick;
	int MountRetryCount;
	int SupportCompletedTick;
	int NextCleanupAttemptTick;
	int IntendedCrewCount;
	int LastProgressLogTick;
	int LastRepathTick;
	int LastOrderAssignedTick;
	int NoProgressStartedTick;
	int RepathAttempts;
	string TransportBaseKey;
	string TransportLockToken;
	string MountFailureReason;
	bool MountPending;
	bool PassengerDeliveryMode;
	bool DropPending;
	bool TransportMoveAssigned;
	bool TransportDismountScheduled;
	bool TransportDismountRequested;
	bool TransportPassengerHandoffComplete;
	bool TransportDismountScheduleLogged;
	bool TransportDropoffSpeedApplied;
	bool TransportPassengerDefendScheduled;
	bool ArmedMountedDefence;
	int TransportDismountVerifyCount;
	JLH_DCF_RecapVehicleState SupportState;
	SCR_AIGroup TransportPassengerGroup;
	ref array<IEntity> CrewUnits = {};
	ref array<IEntity> TransportPassengerUnits = {};
	ref array<IEntity> RouteWaypoints = {};

	void Init(IEntity vehicle, SCR_AIGroup crewGroup, ResourceName vehiclePrefab, ResourceName crewPrefab, vector spawnPosition, vector targetPosition, vector assignedWaypointPosition, array<IEntity> routeWaypoints)
	{
		int now = System.GetTickCount();
		VehicleEntity = vehicle;
		CrewGroup = crewGroup;
		DriverEntity = null;
		GunnerEntity = null;
		VehiclePrefab = vehiclePrefab;
		CrewPrefab = crewPrefab;
		SpawnPosition = spawnPosition;
		TargetPosition = targetPosition;
		AssignedWaypointPosition = assignedWaypointPosition;
		DropPosition = assignedWaypointPosition;
		TransportCapCenter = targetPosition;
		LastDistanceToTarget = -1.0;
		ProgressBaselineDistance = -1.0;
		SpawnTick = now;
		VehicleIndex = 0;
		MountReadyTick = now + 1500;
		MountRetryCount = 0;
		SupportCompletedTick = 0;
		NextCleanupAttemptTick = 0;
		IntendedCrewCount = 0;
		LastProgressLogTick = 0;
		LastRepathTick = 0;
		LastOrderAssignedTick = now;
		NoProgressStartedTick = 0;
		RepathAttempts = 0;
		TransportBaseKey = "";
		TransportLockToken = "";
		MountFailureReason = "";
		MountPending = true;
		PassengerDeliveryMode = false;
		DropPending = false;
		TransportMoveAssigned = false;
		TransportDismountScheduled = false;
		TransportDismountRequested = false;
		TransportPassengerHandoffComplete = false;
		TransportDismountScheduleLogged = false;
		TransportDropoffSpeedApplied = false;
		TransportPassengerDefendScheduled = false;
		ArmedMountedDefence = false;
		TransportDismountVerifyCount = 0;
		SupportState = JLH_DCF_RecapVehicleState.ACTIVE_SUPPORT;
		TransportPassengerGroup = null;
		CrewUnits.Clear();
		TransportPassengerUnits.Clear();
		RouteWaypoints.Clear();

		if (!routeWaypoints)
			return;

		foreach (IEntity waypointEntity : routeWaypoints)
		{
			if (waypointEntity)
				RouteWaypoints.Insert(waypointEntity);
		}
	}
}

[ComponentEditorProps(category: "JLH Dynamic Conflict Framework/Nodes", description: "World-placeable long-timer base recapture node.")]
class JLH_RecapNodeComponentClass : SCR_AmbientPatrolSpawnPointComponentClass
{
}

class JLH_RecapNodeComponent : SCR_AmbientPatrolSpawnPointComponent
{
	protected static const string SYSTEM_NAME = "RECAP";
	protected static const string PLAYER_FACTION_KEY = "US";
	protected static bool s_bNoLiveHumanPlayerLaunchSkipLogged;
	protected static const ResourceName DEFAULT_USSR_VEHICLE_PASSENGER_GROUP_PREFAB = "{AAE710F15D0C4B01}Prefabs/Groups/JLH_DCF/JLH_USSR_TransportAssaultGroup_15Man.et";
	protected static const ResourceName DEFAULT_US_VEHICLE_PASSENGER_GROUP_PREFAB = "{AAE710F15D0C4B02}Prefabs/Groups/JLH_DCF/JLH_US_TransportAssaultGroup_15Man.et";
	protected static const ResourceName DEFAULT_FIA_VEHICLE_PASSENGER_GROUP_PREFAB = "{AAE710F15D0C4B03}Prefabs/Groups/JLH_DCF/JLH_FIA_TransportAssaultGroup_15Man.et";
	protected static const ResourceName ASSAULT_MOVE_WAYPOINT_PREFAB = "{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et";
	protected static const ResourceName CYCLE_WAYPOINT_PREFAB = "{35BD6541CBB8AC08}Prefabs/AI/Waypoints/AIWaypoint_Cycle.et";
	protected static const ResourceName DEFEND_CP_WAYPOINT_PREFAB = "{2A81753527971941}Prefabs/AI/Waypoints/AIWaypoint_Defend_CP.et";
	protected static const ResourceName DEFEND_WAYPOINT_PREFAB = "{93291E72AC23930F}Prefabs/AI/Waypoints/AIWaypoint_Defend.et";
	protected static const int ATTACK_SETTLE_MIN_MS = 120000;
	protected static const int ATTACK_TIMEOUT_MS = 1800000;
	protected static const int ASSAULT_UNIT_DISCOVERY_GRACE_MS = 5000;
	protected static const float ATTACK_SETTLE_RADIUS = 130.0;
	protected static const int VEHICLE_CLEANUP_RETRY_MS = 30000;
	protected static const int VEHICLE_CLEANUP_GRACE_MS = 60000;
	protected static const float VEHICLE_CLEANUP_PLAYER_RADIUS = 900.0;
	protected static const int SPAWN_SAFETY_LOG_THROTTLE_MS = 30000;
	protected static const int ATTACK_STATUS_LOG_INTERVAL_MS = 30000;
	protected static const int VEHICLE_PROGRESS_LOG_INTERVAL_MS = 30000;
	protected static const int VEHICLE_REPATH_NO_PROGRESS_MS = 45000;
	protected static const int VEHICLE_REPATH_COOLDOWN_MS = 60000;
	protected static const int VEHICLE_REPATH_ORDER_GRACE_MS = 15000;
	protected static const int VEHICLE_MOUNT_RETRY_DELAY_MS = 500;
	protected static const int VEHICLE_MOUNT_MAX_RETRIES = 20;
	protected static const int VEHICLE_TRANSPORT_MOUNT_MAX_RETRIES = 3;
	protected static const float VEHICLE_PROGRESS_EPSILON_METERS = 8.0;
	protected static const float VEHICLE_WRONG_WAY_EPSILON_METERS = 25.0;
	protected static const int ATTACK_VEHICLE_MAX_MOUNTED_CREW = 3;
	protected static const int TRANSPORT_TOTAL_CAPACITY = 15;
	protected static const int TRANSPORT_PASSENGER_GROUP_SIZE = 14;
	protected static const float VEHICLE_SPAWN_SPACING_METERS = 22.0;
	protected static const float VEHICLE_HOLD_SPACING_METERS = 28.0;
	protected static const float VEHICLE_HOLD_RADIUS_METERS = 55.0;
	protected static const float VEHICLE_HOLD_RING_STEP_METERS = 20.0;
	protected static const float VEHICLE_HOLD_MIN_CAP_DISTANCE_METERS = 30.0;
	protected static const float GHOST_DEFENCE_INFANTRY_RADIUS_METERS = 85.0;
	protected static const float GHOST_DEFENCE_INFANTRY_RING_STEP_METERS = 35.0;
	protected static const float GHOST_DEFENCE_INFANTRY_SPACING_METERS = 18.0;
	protected static const float GHOST_DEFENCE_SAFE_SPAWN_DISTANCE = 500.0;
	protected static const float TRANSPORT_ARRIVAL_RADIUS = 70.0;
	protected static const float TRANSPORT_DISMOUNT_DELAY = 7.0;
	protected static const float TRANSPORT_HANDOFF_VERIFY_DELAY = 4.5;
	protected static const float TRANSPORT_DEFEND_ASSIGN_DELAY = 5.0;
	protected static const float TRANSPORT_DROPOFF_CRUISE_SPEED_KMH = 25.0;
	protected static const int TRANSPORT_EXIT_STAGGER_MS = 250;
	protected static const int TRANSPORT_HANDOFF_MAX_VERIFY_ATTEMPTS = 8;
	protected static const int TRANSPORT_EXIT_DOOR_SCAN_COUNT = 8;
	protected static const ResourceName DEFAULT_USSR_ARMED_SUPPORT_GROUP_PREFAB = "{D4CF0B2B840CF001}Prefabs/Groups/JLH_DCF/JLH_USSR_VehicleGroup.et";
	protected static const ResourceName DEFAULT_US_ARMED_SUPPORT_GROUP_PREFAB = "{101C1DACCD535E45}Prefabs/Groups/JLH_DCF/JLH_US_VehicleGroup.et";
	protected static const ResourceName DEFAULT_FIA_ARMED_SUPPORT_GROUP_PREFAB = "{B0F1A7E33C6B4205}Prefabs/Groups/JLH_DCF/USSR_GroupLibrary/JLH_USSR_LargeMixed_01.et";
	protected static const ResourceName DEFAULT_USSR_TRANSPORT_VEHICLE_PREFAB = "{16C1F16C9B053801}Prefabs/Vehicles/Wheeled/Ural4320/Ural4320_transport.et";

	[Attribute("1", UIWidgets.CheckBox, "Enable this Recap node.", category: "JLH DCF Recap - Basic")]
	protected bool RecapEnabled;

	[Attribute("1", UIWidgets.CheckBox, "Allow the global Recap Pressure Director to use this node.", category: "JLH DCF Recap - Director")]
	protected bool RecapEnabledByDirector;

	[Attribute("1", UIWidgets.CheckBox, "Treat this node as a major location candidate for director eligibility.", category: "JLH DCF Recap - Director")]
	protected bool RecapMajorLocation;

	[Attribute("5", UIWidgets.ComboBox, "Explicit Recap pressure region. UNKNOWN lets the director infer by map position.", "", ParamEnumArray.FromEnum(JLH_DCF_RecapRegion), category: "JLH DCF Recap - Director")]
	protected JLH_DCF_RecapRegion RecapDirectorRegion;

	[Attribute("3600", UIWidgets.EditBox, "Minimum seconds before a US-owned parent base receives a Recap assault.", "0 86400 1", category: "JLH DCF Recap - Attack Timing")]
	protected float MinAttackDelaySeconds;

	[Attribute("18000", UIWidgets.EditBox, "Maximum seconds before a US-owned parent base receives a Recap assault.", "0 86400 1", category: "JLH DCF Recap - Attack Timing")]
	protected float MaxAttackDelaySeconds;

	[Attribute("4", UIWidgets.ComboBox, "Main Recap force selector. CUSTOM_MANUAL uses the manual count fields; named presets control infantry and vehicle counts automatically.", "", ParamEnumArray.FromEnum(JLH_DCF_RecapForcePreset), category: "JLH DCF Recap - Basic")]
	protected JLH_DCF_RecapForcePreset ForcePreset;

	[Attribute("3", UIWidgets.EditBox, "CUSTOM_MANUAL only. Minimum infantry groups. Named presets ignore this field.", "0 20 1", category: "JLH DCF Recap - Force Size")]
	protected int InfantryGroupCountMin;

	[Attribute("3", UIWidgets.EditBox, "CUSTOM_MANUAL only. Maximum infantry groups. Named presets ignore this field.", "0 20 1", category: "JLH DCF Recap - Force Size")]
	protected int InfantryGroupCountMax;

	[Attribute("1", UIWidgets.CheckBox, "When Group Prefab Override is empty, select a fresh random group prefab for every infantry group slot.", category: "JLH DCF Recap - Force Size")]
	protected bool RecapPickRandomGroupType;

	[Attribute("", UIWidgets.EditBox, "Optional exact infantry group prefab for every Recap group slot. Leave empty for per-slot random group selection.", category: "JLH DCF Recap - Optional Overrides")]
	protected ResourceName GroupPrefabOverride;

	[Attribute("1", UIWidgets.CheckBox, "Allow Recap vehicle support. TRANSPORT_ASSAULT still uses the per-base max-one transport lock.", category: "JLH DCF Recap - Vehicle Support")]
	protected bool VehicleSupportEnabled;

	[Attribute("1", UIWidgets.EditBox, "CUSTOM_MANUAL only. Minimum vehicles. Named presets ignore this field.", "0 8 1", category: "JLH DCF Recap - Vehicle Support")]
	protected int VehicleCountMin;

	[Attribute("1", UIWidgets.EditBox, "CUSTOM_MANUAL only. Maximum vehicles. Named presets ignore this field.", "0 8 1", category: "JLH DCF Recap - Vehicle Support")]
	protected int VehicleCountMax;

	[Attribute("", UIWidgets.EditBox, "Optional exact Recap vehicle prefab. For TRANSPORT_ASSAULT this must be a valid transport truck; otherwise it is ignored.", category: "JLH DCF Recap - Optional Overrides")]
	protected ResourceName SpecificVehiclePrefab;

	// Inherited SCR ambient internals stay parent-owned for prefab compatibility.
	// Hidden editor fields below are internal standard RECAP safeguards; keep them serialized for existing prefabs.
	[Attribute("", UIWidgets.Hidden, "Optional armed support vehicle prefab pool. Ignored by TRANSPORT_ASSAULT. Leave empty to use trait-filtered random armed support.", category: "JLH DCF Recap - Internal")]
	protected ref array<ResourceName> ArmedSupportVehiclePrefabs;

	[Attribute("", UIWidgets.Hidden, "Optional TRANSPORT_ASSAULT truck pool. Max one active transport is allowed per base. Leave empty to use default faction transport trucks.", category: "JLH DCF Recap - Internal")]
	protected ref array<ResourceName> TransportVehiclePrefabs;

	[Attribute("0", UIWidgets.Hidden, "Armed support trait filter. Defaults to TRAIT_ARMED and TRAIT_ARMORED when empty; ignored by TRANSPORT_ASSAULT.", "", ParamEnumArray.FromEnum(EEditableEntityLabel), category: "JLH DCF Recap - Internal")]
	protected ref array<EEditableEntityLabel> IncludedVehicleTraits;

	[Attribute("0", UIWidgets.Hidden, "Armed support trait exclusions. Helicopters and airplanes are always excluded by default.", "", ParamEnumArray.FromEnum(EEditableEntityLabel), category: "JLH DCF Recap - Internal")]
	protected ref array<EEditableEntityLabel> ExcludedVehicleTraits;

	[Attribute("0", UIWidgets.Hidden, "Armed support trait matching. Enabled means every included trait must match; disabled means any included trait may match.", category: "JLH DCF Recap - Internal")]
	protected bool RequireAllIncludedVehicleTraits;

	[Attribute("1", UIWidgets.Hidden, "Keep armed/turret vehicle support mounted as vehicle defence. TRANSPORT_ASSAULT still unloads cargo passengers.", category: "JLH DCF Recap - Internal")]
	protected bool RecapKeepArmedVehiclesMounted;

	[Attribute("15", UIWidgets.Hidden, "Seconds to delay vehicle support after Recap infantry spawns and starts moving. Applies to armed and transport support.", "0 300 1", category: "JLH DCF Recap - Internal")]
	protected float VehicleSpawnDelaySeconds;

	[Attribute("5", UIWidgets.Hidden, "Additional random seconds added to Recap vehicle support delay.", "0 120 1", category: "JLH DCF Recap - Internal")]
	protected float VehicleSpawnDelayJitter;

	[Attribute("1", UIWidgets.Hidden, "Prevent Recap forces from spawning visibly in front of nearby players. Defers and retries instead of cancelling the attack.", category: "JLH DCF Recap - Internal")]
	protected bool EnableSpawnVisibilitySafety;

	[Attribute("175", UIWidgets.Hidden, "Infantry assault packages inside this distance require visibility checks before spawning.", "0 2000 1", category: "JLH DCF Recap - Internal")]
	protected float MinimumInfantrySpawnSafetyDistance;

	[Attribute("250", UIWidgets.Hidden, "Vehicle assault packages inside this distance require visibility checks before spawning.", "0 3000 1", category: "JLH DCF Recap - Internal")]
	protected float MinimumVehicleSpawnSafetyDistance;

	[Attribute("5", UIWidgets.Hidden, "Seconds between spawn visibility retry checks while a Recap launch is deferred.", "1 120 1", category: "JLH DCF Recap - Internal")]
	protected float SpawnSafetyRetryDelay;

	[Attribute("30", UIWidgets.Hidden, "Maximum seconds to defer a Recap launch for spawn visibility before optional unsafe fallback.", "0 600 1", category: "JLH DCF Recap - Internal")]
	protected float MaximumSpawnSafetyDeferral;

	[Attribute("1", UIWidgets.Hidden, "Allow the least-bad same-base Recap node to spawn after maximum visibility deferral expires.", category: "JLH DCF Recap - Internal")]
	protected bool AllowUnsafeFallbackAfterTimeout;

	[Attribute("1", UIWidgets.Hidden, "If enabled, close spawns are blocked only when a player has direct line of sight. If disabled, close distance alone defers spawning.", category: "JLH DCF Recap - Internal")]
	protected bool RequireLOSToBlockCloseSpawn;

	[Attribute("0", UIWidgets.CheckBox, "Enable detailed Recap registration, timer, spawn, and cleanup logs for this node.", category: "JLH DCF Recap - Debug")]
	protected bool DebugLogging;

	protected SCR_CampaignMilitaryBaseComponent m_BaseComponent;
	protected IEntity m_BaseEntity;
	protected string m_sBaseName;
	protected string m_sBaseRegistrationSource;
	protected string m_sFactionKey;
	protected string m_sLastBaseOwnerFaction;
	protected bool m_bRegistered;
	protected bool m_bDisabled;
	protected bool m_bRecapTimerActive;
	protected float m_fRecapRemainingSeconds;
	protected float m_fLastRecapTickTime;
	protected bool m_bRecapTimerPausedNoPlayers;
	protected bool m_bAttackActive;
	protected bool m_bRecapControlledSpawnAllowed;
	protected bool m_bCurrentAssaultPassengerDelivery;
	protected int m_iLaunchTick;
	protected int m_iAttackLaunchTick;
	protected int m_iActiveAssaultId;
	protected int m_iNextSpawnSafetyRetryTick;
	protected int m_iSpawnSafetyDeferralStartedTick;
	protected int m_iLastSpawnSafetyLogTick;
	protected int m_iLastCleanupDeferredLogTick;
	protected int m_iLastAttackStatusLogTick;
	protected string m_sLastSpawnSafetyLogKey;
	protected JLH_RecapNodeComponent m_SelectedSpawnNode;
	protected bool m_bVehicleSpawnPending;
	protected int m_iPendingVehicleCount;
	protected int m_iPendingVehicleIntentionalSkips;
	protected int m_iPendingVehicleReadyTick;
	protected vector m_vPendingVehicleTarget;
	protected IEntity m_PendingVehicleOwner;
	protected ref JLH_DCF_NodeRuntimeTracker m_RuntimeTracker = new JLH_DCF_NodeRuntimeTracker();
	protected ref array<SCR_AIGroup> m_aAttackGroups = {};
	protected ref array<IEntity> m_aAttackUnits = {};
	protected ref array<vector> m_aAttackInfantrySpawnPositions = {};
	protected ref array<vector> m_aAttackVehicleSpawnPositions = {};
	protected ref array<vector> m_aAttackVehicleHoldPositions = {};
	protected ref array<IEntity> m_aRuntimeRouteWaypoints = {};
	protected ref array<ref JLH_DCF_RecapVehicleRuntime> m_aActiveVehicles = {};
	protected ref array<ref JLH_DCF_RecapVehicleRuntime> m_aVehiclesAwaitingCleanup = {};
	protected ref array<ref JLH_DCF_RecapVehicleRuntime> m_aRetainedTransportRuntimes = {};
	protected ref array<SCR_AIGroup> m_aRetainedDefenderGroups = {};
	protected ref array<IEntity> m_aRetainedDefenderUnits = {};
	protected ref array<IEntity> m_aRetainedDefenderVehicles = {};

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		NormalizeVehicleTraitFilters();
		SetEventMask(owner, EntityEvent.INIT);
		SuppressInheritedAmbientState(false);
	}

	override void EOnInit(IEntity owner)
	{
		JLH_RecapManager.RegisterNode(this);
	}

	override void SpawnPatrol()
	{
		if (!m_bRecapControlledSpawnAllowed)
		{
			SuppressInheritedAmbientState(true);
			return;
		}

		super.SpawnPatrol();
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

		if (!RecapEnabled)
		{
			DisableNode("disabled");
			return;
		}

		if (!RefreshNodeFaction(owner))
		{
			DisableNode("node faction could not be resolved from SCR_FactionAffiliationComponent");
			return;
		}

		if (m_RuntimeTracker)
			m_RuntimeTracker.SetRuntimeOwner("Recap");

		string failureReason;
		if (!JLH_DCF_NodeBaseResolver.ResolveBase(owner, "", true, m_BaseComponent, m_BaseEntity, m_sBaseName, m_sBaseRegistrationSource, failureReason))
		{
			DisableNode(failureReason);
			return;
		}

		m_sLastBaseOwnerFaction = JLH_DCF_NodeBaseResolver.GetBaseFactionKey(m_BaseComponent);
		m_bRegistered = true;
		JLH_DCF_GhostDefenceManager.RegisterRecapAnchor(this);
		SuppressInheritedAmbientState(false);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Registered node=%1 base=%2", JLH_DCF_NodeDebug.EntityLabel(owner), m_sBaseName), true);
	}

	void EvaluateBaseState()
	{
		if (!m_bRegistered || m_bDisabled || !m_BaseComponent)
			return;

		if (!RefreshNodeFaction(GetOwner()))
			return;

		string currentOwner = JLH_DCF_NodeBaseResolver.GetBaseFactionKey(m_BaseComponent);
		if (currentOwner == PLAYER_FACTION_KEY)
			JLH_DCF_GhostDefenceManager.ClearForOwnershipChange(m_sBaseName);

		if (currentOwner != m_sLastBaseOwnerFaction)
		{
			JLH_DCF_RecapPressureDirector.GetInstance().NotifyBaseOwnershipChanged(m_sBaseName, m_sLastBaseOwnerFaction, currentOwner);
			m_sLastBaseOwnerFaction = currentOwner;
			if (currentOwner != PLAYER_FACTION_KEY)
			{
				ClearRecapTimer();
				ResetSpawnSafetyDeferral();
			}
		}

		if (m_bAttackActive)
			return;

		if (currentOwner == m_sFactionKey)
			return;

		if (currentOwner != PLAYER_FACTION_KEY)
			return;

		ScheduleTimerIfNeeded();
	}

	void Tick()
	{
		if (!m_bRegistered || m_bDisabled)
			return;

		if (m_RuntimeTracker)
			m_RuntimeTracker.PruneMissing();

		int now = System.GetTickCount();
		if (m_bAttackActive)
			TickAttack(now);

		TickVehicleCleanup(now);
		TickRetainedTransportRuntimes(now);

		if (!m_bAttackActive && m_bRecapTimerActive)
			TickRecapTimer(now);
	}

	protected void ScheduleTimerIfNeeded()
	{
		if (m_bRecapTimerActive)
			return;

		float minSeconds = GetRecapMinAttackDelaySeconds();
		float maxSeconds = GetRecapMaxAttackDelaySeconds(minSeconds);
		float delaySeconds = RollAttackDelaySeconds(minSeconds, maxSeconds);

		m_fRecapRemainingSeconds = delaySeconds;
		m_fLastRecapTickTime = GetRecapTimerTimeSeconds(System.GetTickCount());
		m_bRecapTimerActive = true;
		m_bRecapTimerPausedNoPlayers = false;
		m_iLaunchTick = 0;
		ResetSpawnSafetyDeferral();
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Timer scheduled base=%1 min=%2 max=%3 delay=%4", m_sBaseName, minSeconds, maxSeconds, delaySeconds), true);
	}

	protected void TickRecapTimer(int now)
	{
		if (!m_bRecapTimerActive)
			return;

		float tickTime = GetRecapTimerTimeSeconds(now);
		if (!HasValidPlayersOnlineOrDeployed())
		{
			PauseRecapTimerNoPlayers(now);
			return;
		}

		if (m_bRecapTimerPausedNoPlayers)
		{
			m_bRecapTimerPausedNoPlayers = false;
			m_fLastRecapTickTime = tickTime;
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Timer resumed players_present base=%1 remaining=%2", m_sBaseName, m_fRecapRemainingSeconds), true);
			return;
		}

		if (m_fLastRecapTickTime <= 0.0)
		{
			m_fLastRecapTickTime = tickTime;
			return;
		}

		if (m_fRecapRemainingSeconds > 0.0)
		{
			float deltaSeconds = tickTime - m_fLastRecapTickTime;
			if (deltaSeconds < 0.0)
				deltaSeconds = 0.0;

			m_fLastRecapTickTime = tickTime;
			m_fRecapRemainingSeconds = m_fRecapRemainingSeconds - deltaSeconds;
			if (m_fRecapRemainingSeconds > 0.0)
				return;

			m_fRecapRemainingSeconds = 0.0;
		}

		if (m_iNextSpawnSafetyRetryTick > 0 && now < m_iLaunchTick)
			return;

		TryLaunch(now);
	}

	protected float RollAttackDelaySeconds(float minSeconds, float maxSeconds)
	{
		if (maxSeconds <= minSeconds)
			return minSeconds;

		return Math.RandomFloat(minSeconds, maxSeconds);
	}

	protected float GetRecapMinAttackDelaySeconds()
	{
		JLH_DCF_RecapPressureDirector director = JLH_DCF_RecapPressureDirector.GetInstance();
		if (director && director.IsEnabled())
			return director.GetConfiguredMinDelaySeconds();

		float minSeconds = MinAttackDelaySeconds;
		if (minSeconds < 0.0)
			minSeconds = 0.0;

		return minSeconds;
	}

	protected float GetRecapMaxAttackDelaySeconds(float minSeconds)
	{
		JLH_DCF_RecapPressureDirector director = JLH_DCF_RecapPressureDirector.GetInstance();
		if (director && director.IsEnabled())
			return Math.Max(minSeconds, director.GetConfiguredMaxDelaySeconds());

		float maxSeconds = MaxAttackDelaySeconds;
		if (maxSeconds < minSeconds)
			maxSeconds = minSeconds;

		return maxSeconds;
	}

	protected float GetRecapTimerTimeSeconds(int now)
	{
		return now * 0.001;
	}

	protected void ClearRecapTimer()
	{
		m_bRecapTimerActive = false;
		m_fRecapRemainingSeconds = 0.0;
		m_fLastRecapTickTime = 0.0;
		m_bRecapTimerPausedNoPlayers = false;
		m_iLaunchTick = 0;
	}

	protected void PauseRecapTimerNoPlayers(int now)
	{
		m_fLastRecapTickTime = GetRecapTimerTimeSeconds(now);
		if (m_bRecapTimerPausedNoPlayers)
			return;

		m_bRecapTimerPausedNoPlayers = true;
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Timer paused no_players base=%1 remaining=%2", m_sBaseName, m_fRecapRemainingSeconds), true);
	}

	protected void TryLaunch(int now)
	{
		if (!RecapEnabled)
		{
			LogLaunchRejected("disabled");
			return;
		}

		if (!RequireLiveHumanPlayerForLaunch())
		{
			JLH_DCF_RecapPressureDirector noPlayerDirector = JLH_DCF_RecapPressureDirector.GetInstance();
			string noPlayerDenyReason = "";
			int noPlayerRetrySeconds = 0;
			if (noPlayerDirector && noPlayerDirector.IsEnabled())
				noPlayerDirector.CanLaunchRecap(this, noPlayerDenyReason, noPlayerRetrySeconds);

			PauseRecapTimerNoPlayers(now);
			return;
		}

		if (!m_BaseComponent)
		{
			LogLaunchRejected("no_base");
			return;
		}

		int activeAssaults = JLH_RecapManager.CountActiveAttacksForBase(m_sBaseName, this);
		if (m_bAttackActive)
			activeAssaults++;

		if (activeAssaults > 0)
		{
			LogLaunchRejected("active_attack");
			LogDebug(string.Format("Launch rejected reason=active_attack base=%1 activeAssaults=%2", m_sBaseName, activeAssaults));
			return;
		}

		string currentOwner = JLH_DCF_NodeBaseResolver.GetBaseFactionKey(m_BaseComponent);
		if (currentOwner != PLAYER_FACTION_KEY)
		{
			LogLaunchRejected("base_not_enemy_owned");
			ClearRecapTimer();
			return;
		}

		JLH_DCF_GhostDefenceManager.ClearForOwnershipChange(m_sBaseName);
		if (JLH_DCF_GhostDefenceManager.HasGhostOrActiveDefence(m_sBaseName))
		{
			JLH_DCF_GhostDefenceManager.LogSpawnBlocked(m_sBaseName);
			ClearRecapTimer();
			return;
		}

		if (IsBaseContested())
		{
			LogLaunchRejected("base_contested");
			DeferLaunchRetry(now);
			return;
		}

		JLH_DCF_RecapPressureDirector director = JLH_DCF_RecapPressureDirector.GetInstance();
		string directorDenyReason = "";
		int directorRetrySeconds = 0;
		if (director && !director.CanLaunchRecap(this, directorDenyReason, directorRetrySeconds))
		{
			RescheduleWithDirectorBackoff(now, directorDenyReason, directorRetrySeconds);
			return;
		}

		JLH_DCF_RecapForcePreset selectedPreset = ForcePreset;
		int infantryCount = 0;
		int vehicleCount = 0;
		bool passengerDelivery = false;
		ResolveAssaultPlan(selectedPreset, infantryCount, vehicleCount, passengerDelivery);

		if (!CanSpawnAssaultSafely(vehicleCount > 0))
		{
			DeferLaunchRetry(now);
			return;
		}

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Timer expired launching base=%1", m_sBaseName), true);
		LaunchAssault(selectedPreset, infantryCount, vehicleCount, passengerDelivery);
	}

	protected void DeferLaunchRetry(int now)
	{
		m_iNextSpawnSafetyRetryTick = now + SecondsToMs(SpawnSafetyRetryDelay);
		m_iLaunchTick = m_iNextSpawnSafetyRetryTick;
	}

	protected void RescheduleWithDirectorBackoff(int now, string reason, int retrySeconds)
	{
		ResetSpawnSafetyDeferral();
		if (retrySeconds <= 0)
		{
			ClearRecapTimer();
			LogDebug(string.Format("Director disabled launch retries base=%1 reason=%2", m_sBaseName, reason));
			return;
		}

		m_fRecapRemainingSeconds = retrySeconds;
		m_fLastRecapTickTime = GetRecapTimerTimeSeconds(now);
		m_bRecapTimerActive = true;
		m_bRecapTimerPausedNoPlayers = false;
		m_iLaunchTick = 0;
		LogDebug(string.Format("Director backoff base=%1 reason=%2 retrySeconds=%3", m_sBaseName, reason, retrySeconds));
	}

	protected void ResolveAssaultPlan(out JLH_DCF_RecapForcePreset selectedPreset, out int infantryCount, out int vehicleCount, out bool passengerDelivery)
	{
		selectedPreset = ForcePreset;
		passengerDelivery = false;
		if (selectedPreset == JLH_DCF_RecapForcePreset.RANDOM)
		{
			int presetRoll = Math.RandomInt(0, 4);
			if (presetRoll == 0)
				selectedPreset = JLH_DCF_RecapForcePreset.LIGHT_PROBE;
			else if (presetRoll == 2)
				selectedPreset = JLH_DCF_RecapForcePreset.HEAVY_ASSAULT;
			else if (presetRoll == 3)
				selectedPreset = JLH_DCF_RecapForcePreset.TRANSPORT_ASSAULT;
			else
				selectedPreset = JLH_DCF_RecapForcePreset.STANDARD_ASSAULT;

			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("RANDOM resolved preset=%1", GetPresetLabel(selectedPreset)), true);
		}

		int defaultInfantryMin = 3;
		int defaultInfantryMax = 3;
		int defaultVehicleMin = 1;
		int defaultVehicleMax = 1;

		if (selectedPreset == JLH_DCF_RecapForcePreset.LIGHT_PROBE)
		{
			defaultInfantryMin = 2;
			defaultInfantryMax = 2;
			defaultVehicleMin = 0;
			defaultVehicleMax = 1;
		}
		else if (selectedPreset == JLH_DCF_RecapForcePreset.HEAVY_ASSAULT)
		{
			defaultInfantryMin = 4;
			defaultInfantryMax = 5;
			defaultVehicleMin = 1;
			defaultVehicleMax = 2;
		}
		else if (selectedPreset == JLH_DCF_RecapForcePreset.TRANSPORT_ASSAULT)
		{
			defaultInfantryMin = 0;
			defaultInfantryMax = 0;
			defaultVehicleMin = 1;
			defaultVehicleMax = 1;
			passengerDelivery = true;
		}

		bool manualCountsAllowed = selectedPreset == JLH_DCF_RecapForcePreset.CUSTOM_MANUAL;

		int infantryMin = defaultInfantryMin;
		int infantryMax = defaultInfantryMax;
		if (manualCountsAllowed && (InfantryGroupCountMin > 0 || InfantryGroupCountMax > 0))
		{
			infantryMin = InfantryGroupCountMin;
			infantryMax = InfantryGroupCountMax;
		}

		int vehicleMin = defaultVehicleMin;
		int vehicleMax = defaultVehicleMax;
		if (manualCountsAllowed && (VehicleCountMin > 0 || VehicleCountMax > 0))
		{
			vehicleMin = VehicleCountMin;
			vehicleMax = VehicleCountMax;
		}

		infantryCount = RollCount(infantryMin, infantryMax);
		if (passengerDelivery)
			infantryCount = 0;

		vehicleCount = 0;
		if (VehicleSupportEnabled)
			vehicleCount = RollCount(vehicleMin, vehicleMax);

		int armedVehicles = vehicleCount;
		int transportVehicles = 0;
		if (passengerDelivery)
		{
			armedVehicles = 0;
			transportVehicles = vehicleCount;
		}

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("preset selected=%1", GetPresetLabel(ForcePreset)), true);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("preset resolved infantryGroups=%1 armedVehicles=%2 transportVehicles=%3 passengerDelivery=%4", infantryCount, armedVehicles, transportVehicles, passengerDelivery), true);
		if (selectedPreset == JLH_DCF_RecapForcePreset.TRANSPORT_ASSAULT)
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("TRANSPORT_ASSAULT resolved infantryGroups=%1 transportVehicles=%2 passengerDelivery=1", infantryCount, transportVehicles), true);
	}

	protected int RollCount(int minCount, int maxCount)
	{
		if (minCount < 0)
			minCount = 0;
		if (maxCount < minCount)
			maxCount = minCount;
		if (maxCount <= minCount)
			return minCount;

		return Math.RandomInt(minCount, maxCount + 1);
	}

	protected void LaunchAssault(JLH_DCF_RecapForcePreset selectedPreset, int infantryCount, int vehicleCount, bool passengerDelivery)
	{
		if (!RequireLiveHumanPlayerForLaunch())
		{
			PauseRecapTimerNoPlayers(System.GetTickCount());
			return;
		}

		IEntity owner = GetSelectedSpawnOwner();
		if (!owner || !RefreshNodeFaction(owner))
			return;

		vector target = ResolveBaseTargetPosition();
		ClearRecapTimer();
		m_bAttackActive = true;
		m_iAttackLaunchTick = System.GetTickCount();
		m_iActiveAssaultId = m_iAttackLaunchTick;
		m_iLastAttackStatusLogTick = 0;
		ClearActiveAttackTracking(false);
		m_bCurrentAssaultPassengerDelivery = passengerDelivery;
		JLH_DCF_RecapPressureDirector.GetInstance().NotifyRecapLaunched(this);

		int activeAssaults = JLH_RecapManager.CountActiveAttacksForBase(m_sBaseName, this) + 1;
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Launch accepted base=%1 preset=%2 assaultId=%3 activeAssaults=%4", m_sBaseName, GetPresetLabel(selectedPreset), m_iActiveAssaultId, activeAssaults), true);

		int spawnedGroups = SpawnInfantryGroups(infantryCount, owner, target);
		int spawnedVehicles = 0;
		if (vehicleCount > 0)
			ScheduleVehicleSpawn(vehicleCount, owner, target);

		LogDebug(string.Format("Launch force spawned base=%1 infantryRequested=%2 infantrySpawned=%3 vehiclesRequested=%4 vehiclesSpawned=%5", m_sBaseName, infantryCount, spawnedGroups, vehicleCount, spawnedVehicles));

		if (spawnedGroups <= 0 && vehicleCount <= 0)
			CompleteAttack("empty_force");
	}

	protected int SpawnInfantryGroups(int groupCount, IEntity owner, vector target)
	{
		if (!RequireLiveHumanPlayerForLaunch())
			return 0;

		int spawned = 0;
		for (int i = 0; i < groupCount; i++)
		{
			int slot = i + 1;
			ResourceName prefab = GroupPrefabOverride;
			string mode = "override";
			if (IsEmptyResource(prefab))
			{
				mode = "random";
				prefab = ResolveGroupPrefab();
			}

			LogGroupPrefabSelected(mode, slot, prefab);
			if (IsEmptyResource(prefab))
				continue;

			SCR_AIGroup group = SpawnGroupPrefab(prefab, owner);
			if (!group)
				continue;

			array<IEntity> routeWaypoints = {};
			if (!AssignAssaultAndDefendWaypoints(group, target, routeWaypoints))
			{
				CleanupRouteWaypoints(group, routeWaypoints);
				DeleteGroupAndMembers(group);
				continue;
			}

			InsertRuntimeRouteWaypoints(routeWaypoints);
			TrackAttackGroup(group);
			RecordAttackInfantrySpawnPosition(owner.GetOrigin());
			spawned++;
		}

		return spawned;
	}

	protected void ScheduleVehicleSpawn(int vehicleCount, IEntity owner, vector target)
	{
		if (vehicleCount <= 0 || !owner)
			return;

		if (!RequireLiveHumanPlayerForLaunch())
			return;

		int delayMs = GetVehicleSpawnDelayMs();
		m_bVehicleSpawnPending = true;
		m_iPendingVehicleCount = vehicleCount;
		m_iPendingVehicleReadyTick = System.GetTickCount() + delayMs;
		m_vPendingVehicleTarget = target;
		m_PendingVehicleOwner = owner;

		float delaySeconds = delayMs * 0.001;
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("vehicle spawn delayed assaultId=%1 base=%2 delay=%3", m_iActiveAssaultId, m_sBaseName, delaySeconds), true);
	}

	protected void TickPendingVehicleSpawn(int now)
	{
		if (!m_bVehicleSpawnPending)
			return;

		if (now < m_iPendingVehicleReadyTick)
			return;

		if (!RequireLiveHumanPlayerForLaunch())
		{
			m_bVehicleSpawnPending = false;
			m_iPendingVehicleCount = 0;
			m_iPendingVehicleReadyTick = 0;
			m_vPendingVehicleTarget = "0 0 0";
			m_PendingVehicleOwner = null;
			return;
		}

		int vehicleCount = m_iPendingVehicleCount;
		IEntity owner = m_PendingVehicleOwner;
		vector target = m_vPendingVehicleTarget;
		m_bVehicleSpawnPending = false;
		m_iPendingVehicleCount = 0;
		m_iPendingVehicleReadyTick = 0;
		m_vPendingVehicleTarget = "0 0 0";
		m_PendingVehicleOwner = null;

		RefreshAttackUnitsFromGroups();
		int liveInfantry = CountLivingAttackUnits();
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("vehicle delay elapsed liveInfantry=%1 assaultId=%2 base=%3", liveInfantry, m_iActiveAssaultId, m_sBaseName), true);

		m_iPendingVehicleIntentionalSkips = 0;
		int spawned = SpawnVehicles(vehicleCount, owner, target);
		if (spawned > 0)
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("vehicle spawned after delay assaultId=%1 base=%2 count=%3", m_iActiveAssaultId, m_sBaseName, spawned), true);
		else if (m_iPendingVehicleIntentionalSkips > 0)
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "transport skip handled as intentional", true);
		else
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("vehicle spawn skipped reason=no_vehicle_spawned assaultId=%1 base=%2", m_iActiveAssaultId, m_sBaseName), true);
	}

	protected int SpawnVehicles(int vehicleCount, IEntity owner, vector target)
	{
		if (!owner)
			return 0;

		if (!RequireLiveHumanPlayerForLaunch())
			return 0;

		RefreshAttackUnitsFromGroups();
		int spawned = 0;
		for (int i = 0; i < vehicleCount; i++)
		{
			ResourceName vehiclePrefab = ResolveVehiclePrefab(m_bCurrentAssaultPassengerDelivery);
			if (IsEmptyResource(vehiclePrefab))
				continue;

			bool directSeatMode = !m_bCurrentAssaultPassengerDelivery && (RecapKeepArmedVehiclesMounted || IsArmedVehicleSupportPrefab(vehiclePrefab));
			ResourceName armedSupportGroupPrefab = ResourceName.Empty;
			ResourceName passengerGroupPrefab = ResourceName.Empty;
			if (directSeatMode)
			{
				armedSupportGroupPrefab = ResolveArmedVehicleCrewGroupPrefab();
				if (IsEmptyResource(armedSupportGroupPrefab))
					continue;
			}
			else
			{
				passengerGroupPrefab = ResolveVehiclePassengerGroupPrefab();
				if (IsEmptyResource(passengerGroupPrefab))
					continue;
			}

			vector position = ResolveVehicleSpawnPosition(owner, i);
			string positionReason;
			if (!ValidateDelayedVehicleSpawnPosition(position, positionReason))
			{
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("vehicle spawn skipped reason=%1 assaultId=%2 base=%3", positionReason, m_iActiveAssaultId, m_sBaseName), true);
				continue;
			}

			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("spawn position validated=true pos=%1", position.ToString()), true);

			string spawnSafetyReason;
			if (!IsVehicleSpawnVisibilitySafe(position, spawnSafetyReason))
			{
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle spawn skipped reason=spawn_visibility_failed safety=%1", spawnSafetyReason), true);
				continue;
			}

			string holdSpacingResult = "ok";
			vector assignedWaypointPosition = "0 0 0";
			bool assignedWaypointResolved = false;
			string transportLockToken = "";
			if (!directSeatMode)
			{
				transportLockToken = BuildTransportLockToken(i);
				if (!JLH_DCF_BaseTransportLock.TryAcquire(m_sBaseName, transportLockToken))
				{
					JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport skipped reason=base_transport_active base=%1", m_sBaseName), true);
					m_iPendingVehicleIntentionalSkips++;
					continue;
				}

				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport accepted base=%1 activeTransport=1", m_sBaseName), true);

				assignedWaypointPosition = ResolveTransportVehicleHoldPosition(target, position, i);
				holdSpacingResult = "transport_ok";
				assignedWaypointResolved = true;
			}

			IEntity vehicle = SpawnVehicleAt(vehiclePrefab, owner, position);
			if (!vehicle)
			{
				if (!directSeatMode)
					ReleaseTransportLock(m_sBaseName, transportLockToken, "vehicle_spawn_failed");

				continue;
			}

			if (directSeatMode)
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "vehicle mode=ARMED_SUPPORT", true);
			else
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "vehicle mode=TRANSPORT_DROP", true);

			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle spawned base=%1 assaultId=%2 slot=%3 vehicle=%4 prefab=%5 pos=%6", m_sBaseName, m_iActiveAssaultId, i + 1, JLH_DCF_NodeDebug.EntityLabel(vehicle), vehiclePrefab, vehicle.GetOrigin().ToString()), true);
			LogDebug(string.Format("Vehicle spawn pos=%1 target=%2", position.ToString(), target.ToString()));
			SCR_AIGroup crewGroup;
			if (directSeatMode)
				crewGroup = SpawnVehiclePassengerGroupAt(armedSupportGroupPrefab, owner, position + Vector(2, 0, 0));
			else
				crewGroup = SpawnVehiclePassengerGroupAt(passengerGroupPrefab, owner, position + Vector(2, 0, 0));

			if (!crewGroup)
			{
				SCR_EntityHelper.DeleteEntityAndChildren(vehicle);
				if (!directSeatMode)
					ReleaseTransportLock(m_sBaseName, transportLockToken, "crew_group_spawn_failed");

				continue;
			}

			JLH_DCF_VehicleMountStats mountStats = null;
			if (directSeatMode)
			{
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("crew method=vehiclepatrol_dedicated_vehicle_group base=%1 assaultId=%2 slot=%3 vehicle=%4 crewGroup=%5", m_sBaseName, m_iActiveAssaultId, i + 1, JLH_DCF_NodeDebug.EntityLabel(vehicle), JLH_DCF_NodeDebug.EntityLabel(crewGroup)), true);
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("crew group prefab=%1 base=%2 assaultId=%3 slot=%4", armedSupportGroupPrefab, m_sBaseName, m_iActiveAssaultId, i + 1), true);
				LogMountCandidateState(crewGroup, vehicle, "spawned");
			}
			else
			{
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle passenger support preserved prefab=%1", passengerGroupPrefab), true);
			}

			if (!RegisterUsableVehicle(crewGroup, vehicle))
			{
				DeleteGroupAndMembers(crewGroup);
				SCR_EntityHelper.DeleteEntityAndChildren(vehicle);
				if (!directSeatMode)
					ReleaseTransportLock(m_sBaseName, transportLockToken, "vehicle_usage_register_failed");

				continue;
			}

			array<IEntity> routeWaypoints = {};
			if (!assignedWaypointResolved)
				assignedWaypointPosition = ResolveVehicleHoldPosition(target, i, holdSpacingResult);
			bool assigned = true;
			if (directSeatMode)
			{
				assigned = AssignMountedVehicleHoldWaypointToGroup(crewGroup, assignedWaypointPosition, routeWaypoints);
			}

			if (!assigned)
			{
				CleanupRouteWaypoints(crewGroup, routeWaypoints);
				DeleteGroupAndMembers(crewGroup);
				SCR_EntityHelper.DeleteEntityAndChildren(vehicle);
				if (!directSeatMode)
					ReleaseTransportLock(m_sBaseName, transportLockToken, "waypoint_assignment_failed");

				continue;
			}

			LogDebug(string.Format("Vehicle waypoint assigned spawn=%1 target=%2 waypoint=%3", position.ToString(), target.ToString(), assignedWaypointPosition.ToString()));
			LogVehicleHoldSelected(i, assignedWaypointPosition, holdSpacingResult);
			InsertRuntimeRouteWaypoints(routeWaypoints);
			JLH_DCF_RecapVehicleRuntime runtime = new JLH_DCF_RecapVehicleRuntime();
			ResourceName supportCrewPrefab = passengerGroupPrefab;
			if (directSeatMode)
				supportCrewPrefab = armedSupportGroupPrefab;

			runtime.Init(vehicle, crewGroup, vehiclePrefab, supportCrewPrefab, position, assignedWaypointPosition, assignedWaypointPosition, routeWaypoints);
			runtime.VehicleIndex = i + 1;
			runtime.TransportCapCenter = target;
			runtime.PassengerDeliveryMode = !directSeatMode;
			runtime.DropPending = false;
			if (runtime.PassengerDeliveryMode)
			{
				runtime.DropPending = true;
				runtime.TransportBaseKey = m_sBaseName;
				runtime.TransportLockToken = transportLockToken;
			}

			runtime.DropPosition = assignedWaypointPosition;
			if (directSeatMode)
			{
				runtime.MountPending = true;
				runtime.MountReadyTick = System.GetTickCount() + VEHICLE_MOUNT_RETRY_DELAY_MS;
			}

			JLH_DCF_VehicleSustainment.RegisterVehicle(SYSTEM_NAME, vehicle, crewGroup, m_sFactionKey, m_sBaseName);
			m_aActiveVehicles.Insert(runtime);
			TrackAttackGroup(crewGroup);
			if (directSeatMode && !runtime.MountPending)
				ActivateGroupAI(crewGroup);

			if (m_RuntimeTracker)
				m_RuntimeTracker.TrackVehicle(vehicle);

			m_aAttackVehicleSpawnPositions.Insert(position);
			spawned++;
		}

		return spawned;
	}

	protected void TickAttack(int now)
	{
		RefreshAttackUnitsFromGroups();

		if (JLH_DCF_NodeBaseResolver.GetBaseFactionKey(m_BaseComponent) == m_sFactionKey)
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Ownership flip handling base=%1 owner=%2 assaultId=%3", m_sBaseName, m_sFactionKey, m_iActiveAssaultId), true);
			CompleteAttack("base_retaken");
			return;
		}

		TickPendingVehicleSpawn(now);
		TickActiveVehicles(now);

		int alive = CountLivingAttackUnits();
		int activeVehicles = CountActiveVehicleAssaultBlockers();
		LogAttackStatus(now, alive, activeVehicles);

		if (m_bVehicleSpawnPending)
			return;

		if (alive <= 0 && activeVehicles <= 0 && now - m_iAttackLaunchTick >= ASSAULT_UNIT_DISCOVERY_GRACE_MS)
		{
			CompleteAttack("all_dead");
			return;
		}

		if (now - m_iAttackLaunchTick >= ATTACK_TIMEOUT_MS)
		{
			CompleteAttack("assault_timeout");
			return;
		}

		if (now - m_iAttackLaunchTick >= ATTACK_SETTLE_MIN_MS && (CountLivingAttackUnitsNearBase() > 0 || CountActiveVehiclesNearBase() > 0))
			LogDebug(string.Format("Assault still active base=%1 reason=forces_near_base owner=US alive=%2 vehicles=%3", m_sBaseName, alive, activeVehicles));
	}

	protected void CompleteAttack(string reason)
	{
		RefreshAttackUnitsFromGroups();
		int survivorsRetained = 0;
		int activeVehicles = m_aActiveVehicles.Count();
		if (reason == "base_retaken" && IsRecapSuccessOutcomeConfirmed())
		{
			survivorsRetained = StoreRecapSurvivorsAsGhostDefence();
		}
		else
		{
			if (reason == "base_retaken")
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Ghost handoff skipped base=%1 reason=outcome_not_successful owner=%2 defender=%3", m_sBaseName, JLH_DCF_NodeBaseResolver.GetBaseFactionKey(m_BaseComponent), m_sFactionKey), true);

			RetireActiveVehiclesForCleanup(System.GetTickCount());
		}

		JLH_DCF_RecapPressureDirector.GetInstance().NotifyRecapEnded(this, ResolveDirectorResult(reason));
		m_bAttackActive = false;
		m_iAttackLaunchTick = 0;
		m_iActiveAssaultId = 0;
		ClearActiveAttackTracking(false);
		if (m_RuntimeTracker)
			m_RuntimeTracker.ForgetAll();

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Attack complete reason=%1 survivorsRetained=%2 activeVehicles=%3 defenders=%4", reason, survivorsRetained, activeVehicles, JLH_DCF_GhostDefenceManager.GetDefenceCount(m_sBaseName)), true);

		string currentOwner = JLH_DCF_NodeBaseResolver.GetBaseFactionKey(m_BaseComponent);
		if (currentOwner == PLAYER_FACTION_KEY)
			ScheduleTimerIfNeeded();
	}

	protected JLH_DCF_RecapResult ResolveDirectorResult(string reason)
	{
		if (reason == "base_retaken" && IsRecapSuccessOutcomeConfirmed())
			return JLH_DCF_RecapResult.RECAPTURED_BY_USSR;

		if (reason == "all_dead")
			return JLH_DCF_RecapResult.DEFENDED_BY_US;

		if (reason == "assault_timeout" || reason == "empty_force")
			return JLH_DCF_RecapResult.FAILED_TIMEOUT;

		if (reason == "cleanup" || reason == "cancelled")
			return JLH_DCF_RecapResult.CANCELLED;

		return JLH_DCF_RecapResult.UNKNOWN;
	}

	protected void RetireActiveVehiclesForCleanup(int now)
	{
		for (int i = m_aActiveVehicles.Count() - 1; i >= 0; i--)
		{
			JLH_DCF_RecapVehicleRuntime runtime = m_aActiveVehicles[i];
			if (!runtime)
				continue;

			MarkVehicleSupportComplete(runtime, "attack_complete", now);
		}
	}

	protected int StoreRecapSurvivorsAsGhostDefence()
	{
		if (!m_BaseComponent || m_sBaseName == "" || m_sFactionKey == "")
			return 0;

		RefreshAttackUnitsFromGroups();

		ref array<SCR_AIGroup> groups = {};
		ref array<IEntity> units = {};
		ref array<IEntity> vehicles = {};
		ref array<IEntity> waypoints = {};
		ref array<SCR_AIGroup> trackedGroups = {};
		ref array<IEntity> trackedUnits = {};
		ref array<IEntity> trackedVehicles = {};
		ref array<ref JLH_DCF_GhostVehicleDefencePackage> vehiclePackages = {};
		ref array<SCR_AIGroup> packagedGroups = {};
		ref array<IEntity> packagedUnits = {};
		ref array<IEntity> packagedVehicles = {};
		ref array<IEntity> packagedWaypoints = {};

		foreach (JLH_DCF_RecapVehicleRuntime runtime : m_aActiveVehicles)
		{
			TryAddArmedVehicleGhostPackage(runtime, vehiclePackages, packagedGroups, packagedUnits, packagedVehicles, packagedWaypoints);
		}

		foreach (JLH_DCF_RecapVehicleRuntime completedRuntime : m_aVehiclesAwaitingCleanup)
		{
			TryAddArmedVehicleGhostPackage(completedRuntime, vehiclePackages, packagedGroups, packagedUnits, packagedVehicles, packagedWaypoints);
		}

		foreach (JLH_DCF_RecapVehicleRuntime retainedRuntime : m_aRetainedTransportRuntimes)
		{
			TryAddArmedVehicleGhostPackage(retainedRuntime, vehiclePackages, packagedGroups, packagedUnits, packagedVehicles, packagedWaypoints);
		}

		foreach (SCR_AIGroup group : m_aAttackGroups)
		{
			if (packagedGroups.Contains(group))
				continue;

			AddUniqueGroup(group, groups);
		}

		foreach (SCR_AIGroup retainedGroup : m_aRetainedDefenderGroups)
		{
			if (packagedGroups.Contains(retainedGroup))
				continue;

			AddUniqueGroup(retainedGroup, groups);
		}

		foreach (IEntity unit : m_aAttackUnits)
		{
			if (packagedUnits.Contains(unit))
				continue;

			AddUniqueEntity(unit, units);
		}

		foreach (IEntity retainedUnit : m_aRetainedDefenderUnits)
		{
			if (packagedUnits.Contains(retainedUnit))
				continue;

			AddUniqueEntity(retainedUnit, units);
		}

		foreach (JLH_DCF_RecapVehicleRuntime runtime : m_aActiveVehicles)
		{
			if (runtime && packagedVehicles.Contains(runtime.VehicleEntity))
				continue;

			AddRecapVehicleRuntimeToGhostStore(runtime, groups, units, vehicles, waypoints);
		}

		foreach (JLH_DCF_RecapVehicleRuntime completedRuntime : m_aVehiclesAwaitingCleanup)
		{
			if (completedRuntime && packagedVehicles.Contains(completedRuntime.VehicleEntity))
				continue;

			AddRecapVehicleRuntimeToGhostStore(completedRuntime, groups, units, vehicles, waypoints);
		}

		if (m_RuntimeTracker)
		{
			m_RuntimeTracker.CollectTrackedState(trackedGroups, trackedUnits, trackedVehicles);
			AddTrackedSurvivorsForGhostStore(trackedGroups, trackedUnits, trackedVehicles, packagedGroups, packagedUnits, packagedVehicles, groups, units, vehicles);
		}

		foreach (JLH_DCF_RecapVehicleRuntime retainedRuntime : m_aRetainedTransportRuntimes)
		{
			if (retainedRuntime && packagedVehicles.Contains(retainedRuntime.VehicleEntity))
				continue;

			AddRecapVehicleRuntimeToGhostStore(retainedRuntime, groups, units, vehicles, waypoints);
		}

		foreach (IEntity retainedVehicle : m_aRetainedDefenderVehicles)
		{
			if (packagedVehicles.Contains(retainedVehicle))
				continue;

			AddUniqueEntity(retainedVehicle, vehicles);
		}

		foreach (IEntity runtimeWaypoint : m_aRuntimeRouteWaypoints)
		{
			if (packagedWaypoints.Contains(runtimeWaypoint))
				continue;

			AddUniqueEntity(runtimeWaypoint, waypoints);
		}

		ResourceName preferredPrefab = GroupPrefabOverride;
		string handoffLog = string.Format("Ghost handoff candidates base=%1 groups=%2 groupUnits=%3 units=%4 vehicles=%5 trackerGroups=%6 trackerUnits=%7 trackerVehicles=%8 vehiclePackages=%9", m_sBaseName, groups.Count(), CountLivingUnitsInGroupsForGhostHandoff(groups), units.Count(), vehicles.Count(), trackedGroups.Count(), trackedUnits.Count(), trackedVehicles.Count(), vehiclePackages.Count());
		handoffLog += string.Format(" vehiclePackageCrew=%1", CountVehiclePackageCrewForGhostHandoff(vehiclePackages));
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, handoffLog, true);
		int stored = JLH_DCF_GhostDefenceManager.StoreFromRecap(m_sBaseName, m_BaseComponent, m_BaseEntity, m_sFactionKey, preferredPrefab, groups, units, vehicles, waypoints, vehiclePackages);
		ClearRetainedDefenderTracking();
		m_aRetainedTransportRuntimes.Clear();
		m_aVehiclesAwaitingCleanup.Clear();
		return stored;
	}

	protected void AddTrackedSurvivorsForGhostStore(array<SCR_AIGroup> trackedGroups, array<IEntity> trackedUnits, array<IEntity> trackedVehicles, array<SCR_AIGroup> packagedGroups, array<IEntity> packagedUnits, array<IEntity> packagedVehicles, notnull array<SCR_AIGroup> groups, notnull array<IEntity> units, notnull array<IEntity> vehicles)
	{
		if (trackedGroups)
		{
			foreach (SCR_AIGroup trackedGroup : trackedGroups)
			{
				if (!trackedGroup || packagedGroups.Contains(trackedGroup))
					continue;

				AddUniqueGroup(trackedGroup, groups);
			}
		}

		if (trackedUnits)
		{
			foreach (IEntity trackedUnit : trackedUnits)
			{
				if (!trackedUnit || packagedUnits.Contains(trackedUnit))
					continue;

				AddUniqueEntity(trackedUnit, units);
			}
		}

		if (trackedVehicles)
		{
			foreach (IEntity trackedVehicle : trackedVehicles)
			{
				if (!trackedVehicle || packagedVehicles.Contains(trackedVehicle))
					continue;

				AddUniqueEntity(trackedVehicle, vehicles);
			}
		}
	}

	protected int CountLivingUnitsInGroupsForGhostHandoff(array<SCR_AIGroup> groups)
	{
		if (!groups)
			return 0;

		int living = 0;
		foreach (SCR_AIGroup group : groups)
		{
			living += CountLivingGroupUnits(group);
		}

		return living;
	}

	protected int CountVehiclePackageCrewForGhostHandoff(array<ref JLH_DCF_GhostVehicleDefencePackage> vehiclePackages)
	{
		if (!vehiclePackages)
			return 0;

		int crew = 0;
		foreach (JLH_DCF_GhostVehicleDefencePackage vehiclePackage : vehiclePackages)
		{
			if (vehiclePackage && vehiclePackage.CrewCount > 0)
				crew += vehiclePackage.CrewCount;
		}

		return crew;
	}

	protected bool TryAddArmedVehicleGhostPackage(JLH_DCF_RecapVehicleRuntime runtime, notnull array<ref JLH_DCF_GhostVehicleDefencePackage> vehiclePackages, notnull array<SCR_AIGroup> packagedGroups, notnull array<IEntity> packagedUnits, notnull array<IEntity> packagedVehicles, notnull array<IEntity> packagedWaypoints)
	{
		if (!RecapKeepArmedVehiclesMounted || !runtime || runtime.PassengerDeliveryMode || runtime.DropPending || !runtime.CrewGroup || !runtime.VehicleEntity)
			return false;

		if (runtime.MountPending && (!runtime.ArmedMountedDefence || runtime.IntendedCrewCount <= 0))
			return false;

		if (packagedVehicles.Contains(runtime.VehicleEntity))
			return false;

		if (IsEmptyResource(runtime.VehiclePrefab) || IsEmptyResource(runtime.CrewPrefab))
			return false;

		if (!runtime.ArmedMountedDefence && !IsArmedVehicleSupportPrefab(runtime.VehiclePrefab))
			return false;

		if (IsEntityDestroyed(runtime.VehicleEntity))
			return false;

		CollectVehicleCrewUnits(runtime);
		int crewCount = CountLivingVehicleCrew(runtime);
		if (runtime.IntendedCrewCount > 0)
		{
			if ((runtime.MountPending && crewCount <= 0) || crewCount > runtime.IntendedCrewCount)
				crewCount = runtime.IntendedCrewCount;
		}

		if (crewCount <= 0)
			return false;

		vector defencePosition = runtime.VehicleEntity.GetOrigin();
		if (vector.Distance(defencePosition, "0 0 0") < 0.1)
			defencePosition = runtime.AssignedWaypointPosition;

		JLH_DCF_GhostVehicleDefencePackage vehiclePackage = new JLH_DCF_GhostVehicleDefencePackage();
		vehiclePackage.Init(runtime.VehiclePrefab, runtime.CrewPrefab, defencePosition, crewCount);
		vehiclePackage.SetContext(JLH_DCF_GetGhostDefenceAnchorPosition(), "RECAP");
		vehiclePackage.SetTransformFromEntity(runtime.VehicleEntity);
		vehiclePackage.SourceGroup = runtime.CrewGroup;
		vehiclePackage.SourceVehicle = runtime.VehicleEntity;

		AddUniqueGroup(runtime.CrewGroup, packagedGroups);
		AddUniqueEntity(runtime.VehicleEntity, packagedVehicles);
		int packagedCrewUnits = 0;
		foreach (IEntity crewUnit : runtime.CrewUnits)
		{
			if (!crewUnit)
				continue;

			AddUniqueEntity(crewUnit, packagedUnits);
			if (!IsUnitAlive(crewUnit))
				continue;

			if (packagedCrewUnits >= crewCount)
				continue;

			AddUniqueEntity(crewUnit, vehiclePackage.SourceUnits);
			packagedCrewUnits++;
		}

		foreach (IEntity waypoint : runtime.RouteWaypoints)
		{
			AddUniqueEntity(waypoint, packagedWaypoints);
			AddUniqueEntity(waypoint, vehiclePackage.SourceWaypoints);
		}

		vehiclePackages.Insert(vehiclePackage);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("vehicle_package_stored base=%1 vehicle=%2 crew=%3 reason=survivor_refresh mountPending=%4", m_sBaseName, runtime.VehiclePrefab, crewCount, JLH_DCF_NodeDebug.BoolLabel(runtime.MountPending)), true);
		return true;
	}

	protected bool ShouldRetainVehicleRuntimeAsBaseDefence(JLH_DCF_RecapVehicleRuntime runtime, string reason)
	{
		if (reason == "vehicle_destroyed" || reason == "crew_dead" || reason == "mount_failed")
			return false;

		return IsArmedVehicleRuntimeReadyForGhostDefence(runtime);
	}

	protected bool ShouldRetainVehicleRuntimeForGhostDefence(JLH_DCF_RecapVehicleRuntime runtime)
	{
		if (!IsRecapSuccessOutcomeConfirmed())
			return false;

		return IsArmedVehicleRuntimeReadyForGhostDefence(runtime);
	}

	protected bool IsArmedVehicleRuntimeReadyForGhostDefence(JLH_DCF_RecapVehicleRuntime runtime)
	{
		if (!RecapKeepArmedVehiclesMounted || !runtime || runtime.PassengerDeliveryMode || runtime.DropPending || !runtime.CrewGroup || !runtime.VehicleEntity)
			return false;

		if (runtime.MountPending && (!runtime.ArmedMountedDefence || runtime.IntendedCrewCount <= 0))
			return false;

		if (IsEmptyResource(runtime.VehiclePrefab) || IsEmptyResource(runtime.CrewPrefab))
			return false;

		if (!runtime.ArmedMountedDefence && !IsArmedVehicleSupportPrefab(runtime.VehiclePrefab))
			return false;

		if (IsEntityDestroyed(runtime.VehicleEntity))
			return false;

		CollectVehicleCrewUnits(runtime);
		if (runtime.MountPending && runtime.IntendedCrewCount > 0)
			return true;

		return CountLivingVehicleCrew(runtime) > 0;
	}

	protected void AddRecapVehicleRuntimeToGhostStore(JLH_DCF_RecapVehicleRuntime runtime, notnull array<SCR_AIGroup> groups, notnull array<IEntity> units, notnull array<IEntity> vehicles, notnull array<IEntity> waypoints)
	{
		if (!runtime)
			return;

		CollectVehicleCrewUnits(runtime);
		TrackVehicleCrewUnits(runtime);
		ReleaseTransportLockForRuntime(runtime, "ghost_stored");
		AddUniqueGroup(runtime.CrewGroup, groups);
		AddUniqueGroup(runtime.TransportPassengerGroup, groups);
		AddUniqueEntity(runtime.DriverEntity, units);
		AddUniqueEntity(runtime.GunnerEntity, units);
		AddUniqueEntity(runtime.VehicleEntity, vehicles);

		foreach (IEntity crewUnit : runtime.CrewUnits)
		{
			AddUniqueEntity(crewUnit, units);
		}

		foreach (IEntity passengerUnit : runtime.TransportPassengerUnits)
		{
			AddUniqueEntity(passengerUnit, units);
		}

		foreach (IEntity waypoint : runtime.RouteWaypoints)
		{
			AddUniqueEntity(waypoint, waypoints);
		}
	}

	protected bool IsActiveTransportCrewGroup(SCR_AIGroup group)
	{
		if (!group)
			return false;

		foreach (JLH_DCF_RecapVehicleRuntime runtime : m_aActiveVehicles)
		{
			if (runtime && runtime.PassengerDeliveryMode && runtime.CrewGroup == group)
				return true;
		}

		return false;
	}

	protected void TickActiveVehicles(int now)
	{
		for (int i = m_aActiveVehicles.Count() - 1; i >= 0; i--)
		{
			JLH_DCF_RecapVehicleRuntime runtime = m_aActiveVehicles[i];
			if (!runtime)
			{
				m_aActiveVehicles.Remove(i);
				continue;
			}

			if (runtime.MountPending && now >= runtime.MountReadyTick)
			{
				if (!FinalizeVehicleMount(runtime))
				{
					if (ShouldRetryVehicleMount(runtime))
					{
						runtime.MountRetryCount++;
						runtime.MountReadyTick = now + VEHICLE_MOUNT_RETRY_DELAY_MS;
						if (runtime.PassengerDeliveryMode)
							LogDebug(string.Format("Vehicle transport mount retry scheduled assaultId=%1 retry=%2", m_iActiveAssaultId, runtime.MountRetryCount));
						else
							JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle mount retry scheduled %1 retry=%2", BuildVehicleRuntimeContext(runtime), runtime.MountRetryCount), true);
						continue;
					}

					JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle mount final failure %1 reason=%2 retries=%3", BuildVehicleRuntimeContext(runtime), runtime.MountFailureReason, runtime.MountRetryCount), true);
					CollectVehicleCrewUnits(runtime);
					MarkVehicleSupportComplete(runtime, "mount_failed", now);
					continue;
				}
			}
			else if (runtime.MountPending)
			{
				continue;
			}

			string ineffectiveReason;
			if (!IsVehicleCombatEffective(runtime, ineffectiveReason))
				MarkVehicleSupportComplete(runtime, ineffectiveReason, now);
			else if (runtime.PassengerDeliveryMode && runtime.TransportMoveAssigned)
				continue;
			else
			{
				if (!runtime.PassengerDeliveryMode)
					TickActiveVehicleProgress(runtime, now);
			}
		}
	}

	protected void TickActiveVehicleProgress(JLH_DCF_RecapVehicleRuntime runtime, int now)
	{
		if (!runtime || !runtime.VehicleEntity)
			return;

		vector position = runtime.VehicleEntity.GetOrigin();
		float distance = vector.Distance(position, runtime.TargetPosition);
		if (runtime.LastDistanceToTarget < 0.0)
		{
			runtime.LastDistanceToTarget = distance;
			runtime.ProgressBaselineDistance = distance;
			runtime.LastProgressLogTick = now;
			LogDebug(string.Format("Vehicle distance-to-target assaultId=%1 distance=%2", m_iActiveAssaultId, distance));
			return;
		}

		if (now - runtime.LastProgressLogTick >= VEHICLE_PROGRESS_LOG_INTERVAL_MS)
		{
			runtime.LastProgressLogTick = now;
			LogDebug(string.Format("Vehicle distance-to-target assaultId=%1 distance=%2", m_iActiveAssaultId, distance));
		}

		if (runtime.ProgressBaselineDistance < 0.0)
			runtime.ProgressBaselineDistance = runtime.LastDistanceToTarget;

		if (now - runtime.LastOrderAssignedTick < VEHICLE_REPATH_ORDER_GRACE_MS)
		{
			runtime.LastDistanceToTarget = distance;
			return;
		}

		if (distance + VEHICLE_PROGRESS_EPSILON_METERS < runtime.ProgressBaselineDistance)
		{
			runtime.NoProgressStartedTick = 0;
			runtime.ProgressBaselineDistance = distance;
			runtime.LastDistanceToTarget = distance;
			return;
		}

		bool wrongWay = distance > runtime.ProgressBaselineDistance + VEHICLE_WRONG_WAY_EPSILON_METERS;
		bool noProgress = distance >= runtime.ProgressBaselineDistance - VEHICLE_PROGRESS_EPSILON_METERS;
		if (wrongWay || noProgress)
		{
			if (runtime.NoProgressStartedTick <= 0)
				runtime.NoProgressStartedTick = now;

			if (now - runtime.NoProgressStartedTick >= VEHICLE_REPATH_NO_PROGRESS_MS && (runtime.LastRepathTick <= 0 || now - runtime.LastRepathTick >= VEHICLE_REPATH_COOLDOWN_MS))
				RepathActiveVehicle(runtime, wrongWay, now);
		}

		runtime.LastDistanceToTarget = distance;
	}

	protected void RepathActiveVehicle(JLH_DCF_RecapVehicleRuntime runtime, bool wrongWay, int now)
	{
		if (!runtime || !runtime.CrewGroup)
			return;

		if (runtime.PassengerDeliveryMode)
			return;

		RemoveRuntimeRouteWaypoints(runtime);
		array<IEntity> routeWaypoints = {};
		if (!AssignMountedVehicleHoldWaypointToGroup(runtime.CrewGroup, runtime.TargetPosition, routeWaypoints))
		{
			LogDebug(string.Format("Vehicle repath failed assaultId=%1 reason=waypoint_assign_failed", m_iActiveAssaultId));
			runtime.LastRepathTick = now;
			runtime.NoProgressStartedTick = 0;
			return;
		}

		InsertRuntimeRouteWaypoints(routeWaypoints);
		runtime.RouteWaypoints.Clear();
		foreach (IEntity waypointEntity : routeWaypoints)
		{
			if (waypointEntity)
				runtime.RouteWaypoints.Insert(waypointEntity);
		}

		runtime.AssignedWaypointPosition = runtime.TargetPosition;
		int elapsedSinceOrder = now - runtime.LastOrderAssignedTick;
		runtime.LastRepathTick = now;
		runtime.LastOrderAssignedTick = now;
		runtime.NoProgressStartedTick = 0;
		runtime.RepathAttempts++;
		runtime.ProgressBaselineDistance = vector.Distance(runtime.VehicleEntity.GetOrigin(), runtime.TargetPosition);
		string reason = "no_progress";
		if (wrongWay)
			reason = "wrong_direction";

		string repathLog = string.Format("Vehicle repath assigned reason=%1 target=%2", reason, runtime.TargetPosition.ToString());
		repathLog += string.Format(" distance=%1 elapsedSinceOrderMs=%2", runtime.ProgressBaselineDistance, elapsedSinceOrder);
		repathLog += string.Format(" retryCount=%1", runtime.RepathAttempts);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, repathLog, true);
	}

	protected bool FinalizeVehicleMount(JLH_DCF_RecapVehicleRuntime runtime)
	{
		if (!runtime || !runtime.CrewGroup || !runtime.VehicleEntity)
			return false;

		JLH_DCF_PersistenceExclusion.StopTrackingTree(runtime.CrewGroup, "recap_vehicle_mount_group");
		JLH_DCF_PersistenceExclusion.StopTrackingTree(runtime.VehicleEntity, "recap_vehicle_mount_vehicle");

		JLH_DCF_VehicleMountStats stats = new JLH_DCF_VehicleMountStats();
		LogDebug(string.Format("Mount ready %1 crewSeen=%2", BuildVehicleRuntimeContext(runtime), CountGroupUnits(runtime.CrewGroup)));
		LogMountCandidateState(runtime.CrewGroup, runtime.VehicleEntity, "mount_attempt");
		if (!MountVehicleCrew(runtime.CrewGroup, runtime.VehicleEntity, runtime.VehiclePrefab, stats, runtime.PassengerDeliveryMode))
		{
			runtime.MountFailureReason = stats.FailureReason;
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle mount failed %1 reason=%2 crewSeen=%3 seatsFilled=%4", BuildVehicleRuntimeContext(runtime), stats.FailureReason, stats.CrewSeen, stats.SeatsFilled), true);
			return false;
		}

		runtime.DriverEntity = stats.DriverEntity;
		runtime.GunnerEntity = stats.GunnerEntity;
		runtime.ArmedMountedDefence = !runtime.PassengerDeliveryMode && IsArmedVehicleMount(runtime.VehiclePrefab, stats);
		runtime.MountPending = false;
		runtime.MountRetryCount = 0;
		CacheMountedTransportPassengers(runtime, stats);
		CollectVehicleCrewUnits(runtime);
		TrackVehicleCrewUnits(runtime);
		ActivateGroupAI(runtime.CrewGroup);
		if (runtime.PassengerDeliveryMode)
		{
			if (!AssignTransportMoveAfterMount(runtime))
			{
				runtime.MountFailureReason = "transport_move_assignment_failed";
				return false;
			}

			RetainRecapTransportAsBaseDefence(runtime);
		}
		else
		{
			LogArmedSupportVehicleGroupMount(runtime.VehicleEntity, stats);
		}

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle mount finalized %1 driver=%2 gunner=%3 seatsFilled=%4 crewSeen=%5", BuildVehicleRuntimeContext(runtime), JLH_DCF_NodeDebug.EntityLabel(runtime.DriverEntity), JLH_DCF_NodeDebug.EntityLabel(runtime.GunnerEntity), stats.SeatsFilled, stats.CrewSeen), true);

		JLH_DCF_PersistenceExclusion.StopTrackingTree(runtime.CrewGroup, "recap_vehicle_mount_finalized_group");
		JLH_DCF_PersistenceExclusion.StopTrackingTree(runtime.VehicleEntity, "recap_vehicle_mount_finalized_vehicle");
		return true;
	}

	protected bool AssignTransportMoveAfterMount(JLH_DCF_RecapVehicleRuntime runtime)
	{
		if (!runtime || !runtime.CrewGroup)
			return false;

		if (runtime.TransportMoveAssigned)
			return true;

		if (!RegisterUsableVehicle(runtime.CrewGroup, runtime.VehicleEntity))
			return false;

		ActivateGroupAI(runtime.CrewGroup);
		RemoveRuntimeRouteWaypoints(runtime);
		array<IEntity> routeWaypoints = {};
		if (!AssignVehicleMoveWaypointToGroup(runtime.CrewGroup, runtime.DropPosition, routeWaypoints))
			return false;

		InsertRuntimeRouteWaypoints(routeWaypoints);
		runtime.RouteWaypoints.Clear();
		foreach (IEntity waypoint : routeWaypoints)
		{
			if (waypoint)
				runtime.RouteWaypoints.Insert(waypoint);
		}

		if (!IsTransportDriverValid(runtime))
			LogDebug(string.Format("transport driver validation deferred target=%1", runtime.DropPosition.ToString()));

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport move assigned target=%1", runtime.DropPosition.ToString()), true);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "transport move handoff=vanilla_ai", true);
		runtime.TransportMoveAssigned = true;
		ScheduleTransportDismount(runtime);
		return true;
	}

	protected bool IsTransportDriverValid(JLH_DCF_RecapVehicleRuntime runtime)
	{
		if (!runtime || !runtime.VehicleEntity || !runtime.DriverEntity)
			return false;

		if (!IsUnitAlive(runtime.DriverEntity))
			return false;

		if (IsUnitInCompartmentType(runtime.DriverEntity, runtime.VehicleEntity, ECompartmentType.PILOT))
			return true;

		return IsUnitInVehicle(runtime.DriverEntity, runtime.VehicleEntity);
	}

	protected void CacheMountedTransportPassengers(JLH_DCF_RecapVehicleRuntime runtime, JLH_DCF_VehicleMountStats stats)
	{
		if (!runtime)
			return;

		runtime.TransportPassengerUnits.Clear();
		if (!stats || !stats.PassengerEntities)
			return;

		foreach (IEntity passenger : stats.PassengerEntities)
		{
			if (!passenger || passenger == runtime.DriverEntity || passenger == runtime.GunnerEntity || runtime.TransportPassengerUnits.Contains(passenger))
				continue;

			runtime.TransportPassengerUnits.Insert(passenger);
		}
	}

	protected void ScheduleTransportDismount(JLH_DCF_RecapVehicleRuntime runtime)
	{
		if (!runtime || !runtime.DropPending || runtime.TransportDismountScheduled || runtime.TransportDismountRequested || runtime.TransportPassengerHandoffComplete)
			return;

		if (!GetGame() || !GetGame().GetCallqueue())
			return;

		int delayMs = SecondsToMs(TRANSPORT_DISMOUNT_DELAY);
		if (delayMs <= 0)
			delayMs = 1;

		runtime.TransportDismountScheduled = true;
		if (!runtime.TransportDismountScheduleLogged)
		{
			runtime.TransportDismountScheduleLogged = true;
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport dismount scheduled delay=%1 target=%2", TRANSPORT_DISMOUNT_DELAY, runtime.DropPosition.ToString()), true);
		}
		GetGame().GetCallqueue().CallLater(JLH_DCF_RecapNodeComponent_HandleTransportDismountCheck, delayMs, false, this, runtime);
	}

	void HandleTransportDismountCheck(JLH_DCF_RecapVehicleRuntime runtime)
	{
		if (!runtime)
			return;

		runtime.TransportDismountScheduled = false;
		if (!runtime.DropPending || runtime.TransportDismountRequested || runtime.TransportPassengerHandoffComplete)
			return;

		if (!runtime.VehicleEntity || IsEntityDestroyed(runtime.VehicleEntity) || !runtime.CrewGroup)
			return;

		vector capTarget = ResolveTransportPassengerCapTarget(runtime.TransportCapCenter, runtime.DropPosition);
		vector vehiclePosition = runtime.VehicleEntity.GetOrigin();
		float distanceToDrop = GetTransportHorizontalDistance(vehiclePosition, runtime.DropPosition);
		float distanceToCap = GetTransportHorizontalDistance(vehiclePosition, capTarget);

		if (distanceToDrop > TRANSPORT_ARRIVAL_RADIUS && distanceToCap > TRANSPORT_ARRIVAL_RADIUS)
		{
			ScheduleTransportDismount(runtime);
			return;
		}

		ApplyTransportDropoffSpeed(runtime);

		float arrivalDistance = distanceToDrop;
		if (distanceToCap < arrivalDistance)
			arrivalDistance = distanceToCap;

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport dismount watch radiusTarget=%1 distance=%2 distanceDrop=%3 distanceCap=%4", TRANSPORT_ARRIVAL_RADIUS, arrivalDistance, distanceToDrop, distanceToCap), true);
		ExecuteTransportDismount(runtime, capTarget);
	}

	protected bool ApplyTransportDropoffSpeed(JLH_DCF_RecapVehicleRuntime runtime)
	{
		if (!runtime || runtime.TransportDropoffSpeedApplied)
			return false;

		runtime.TransportDropoffSpeedApplied = true;
		bool applied = ApplyTransportCruiseSpeed(runtime.CrewGroup, TRANSPORT_DROPOFF_CRUISE_SPEED_KMH);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport dropoff speed limited kmh=%1 applied=%2", TRANSPORT_DROPOFF_CRUISE_SPEED_KMH, JLH_DCF_NodeDebug.BoolLabel(applied)), true);
		return true;
	}

	protected bool ApplyTransportCruiseSpeed(SCR_AIGroup group, float cruiseSpeedKmh)
	{
		if (!group)
			return false;

		bool applied = false;
		AICarMovementComponent groupCarMovement = AICarMovementComponent.Cast(group.GetMovementComponent());
		if (groupCarMovement)
		{
			groupCarMovement.SetCruiseSpeed(cruiseSpeedKmh);
			applied = true;
		}

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			AICarMovementComponent carMovement = AICarMovementComponent.Cast(agent.GetMovementComponent());
			if (!carMovement)
				continue;

			carMovement.SetCruiseSpeed(cruiseSpeedKmh);
			applied = true;
		}

		return applied;
	}

	protected void ExecuteTransportDismount(JLH_DCF_RecapVehicleRuntime runtime, vector capTarget)
	{
		if (!runtime || runtime.TransportPassengerHandoffComplete)
			return;

		array<IEntity> cargoUnits = {};
		CollectTransportCargoPassengers(runtime, cargoUnits);
		if (cargoUnits.IsEmpty())
		{
			runtime.TransportDismountRequested = true;
			runtime.TransportDismountVerifyCount = TRANSPORT_HANDOFF_MAX_VERIFY_ATTEMPTS;
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "transport dismount requested once=true count=0", true);
			ScheduleTransportPassengerHandoff(runtime);
			return;
		}

		int requested = RequestTransportPassengerAnimatedExit(runtime, cargoUnits);
		runtime.TransportDismountRequested = true;
		runtime.TransportDismountVerifyCount = 0;
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport dismount requested once=true count=%1", requested), true);
		ScheduleTransportPassengerHandoff(runtime);
	}

	protected void ScheduleTransportPassengerHandoff(JLH_DCF_RecapVehicleRuntime runtime)
	{
		if (!runtime || runtime.TransportPassengerHandoffComplete)
			return;

		if (!GetGame() || !GetGame().GetCallqueue())
			return;

		int delayMs = SecondsToMs(TRANSPORT_HANDOFF_VERIFY_DELAY);
		if (delayMs <= 0)
			delayMs = 1;

		GetGame().GetCallqueue().CallLater(JLH_DCF_RecapNodeComponent_HandleTransportPassengerHandoff, delayMs, false, this, runtime);
	}

	void HandleTransportPassengerHandoff(JLH_DCF_RecapVehicleRuntime runtime)
	{
		if (!runtime || runtime.TransportPassengerHandoffComplete)
			return;

		if (!runtime.VehicleEntity || IsEntityDestroyed(runtime.VehicleEntity) || !runtime.CrewGroup)
			return;

		vector capTarget = ResolveTransportPassengerCapTarget(runtime.TransportCapCenter, runtime.DropPosition);
		array<IEntity> onFootUnits = {};
		array<IEntity> mountedUnits = {};
		CollectTransportPassengerHandoffState(runtime, onFootUnits, mountedUnits);

		if (!mountedUnits.IsEmpty() && runtime.TransportDismountVerifyCount < TRANSPORT_HANDOFF_MAX_VERIFY_ATTEMPTS)
		{
			runtime.TransportDismountVerifyCount++;
			int requested = RequestTransportPassengerAnimatedExit(runtime, mountedUnits);
			LogDebug(string.Format("transport dismount verify pending onFoot=%1 mounted=%2 retry=%3 requested=%4", onFootUnits.Count(), mountedUnits.Count(), runtime.TransportDismountVerifyCount, requested));
			ScheduleTransportPassengerHandoff(runtime);
			return;
		}

		if (onFootUnits.IsEmpty())
		{
			runtime.TransportPassengerHandoffComplete = true;
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport infantry dismounted count=0 stillMounted=%1", mountedUnits.Count()), true);
			if (mountedUnits.IsEmpty())
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "transport infantry handoff complete=false reason=no_passengers", true);
			else
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "transport infantry handoff complete=false reason=still_mounted", true);
			return;
		}

		SCR_AIGroup passengerGroup = runtime.TransportPassengerGroup;
		if (!passengerGroup)
		{
			IEntity owner = GetSelectedSpawnOwner();
			if (!owner)
				owner = runtime.VehicleEntity;

			passengerGroup = SpawnEmptyVehicleCrewCommandGroup(runtime.CrewPrefab, owner, capTarget);
			if (!passengerGroup)
			{
				ScheduleTransportPassengerHandoff(runtime);
				return;
			}

			runtime.TransportPassengerGroup = passengerGroup;
		}

		int moved = MoveTransportPassengerUnitsToGroup(runtime.CrewGroup, passengerGroup, onFootUnits);
		if (moved <= 0 && CountLivingGroupUnits(passengerGroup) <= 0)
		{
			ScheduleTransportPassengerHandoff(runtime);
			return;
		}

		if (!m_aRetainedDefenderGroups.Contains(passengerGroup))
			m_aRetainedDefenderGroups.Insert(passengerGroup);

		foreach (IEntity unit : onFootUnits)
		{
			if (unit && IsUnitAlive(unit) && !m_aRetainedDefenderUnits.Contains(unit))
				m_aRetainedDefenderUnits.Insert(unit);
		}

		ActivateGroupAI(passengerGroup);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport infantry dismounted count=%1 stillMounted=%2", moved, mountedUnits.Count()), true);
		ScheduleTransportPassengerDefendAssignment(runtime);
	}

	protected void ScheduleTransportPassengerDefendAssignment(JLH_DCF_RecapVehicleRuntime runtime)
	{
		if (!runtime || runtime.TransportPassengerHandoffComplete || runtime.TransportPassengerDefendScheduled)
			return;

		if (!GetGame() || !GetGame().GetCallqueue())
			return;

		int delayMs = SecondsToMs(TRANSPORT_DEFEND_ASSIGN_DELAY);
		if (delayMs <= 0)
			delayMs = 1;

		runtime.TransportPassengerDefendScheduled = true;
		vector capTarget = ResolveTransportPassengerCapTarget(runtime.TransportCapCenter, runtime.DropPosition);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport infantry defend scheduled delay=%1 target=%2", TRANSPORT_DEFEND_ASSIGN_DELAY, capTarget.ToString()), true);
		GetGame().GetCallqueue().CallLater(JLH_DCF_RecapNodeComponent_HandleTransportPassengerDefendAssignment, delayMs, false, this, runtime);
	}

	void HandleTransportPassengerDefendAssignment(JLH_DCF_RecapVehicleRuntime runtime)
	{
		if (!runtime || runtime.TransportPassengerHandoffComplete)
			return;

		vector capTarget = ResolveTransportPassengerCapTarget(runtime.TransportCapCenter, runtime.DropPosition);
		array<IEntity> routeWaypoints = {};
		if (!AssignTransportPassengerDefendWaypoint(runtime.TransportPassengerGroup, capTarget, routeWaypoints))
		{
			runtime.TransportPassengerHandoffComplete = true;
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "transport infantry handoff complete=false reason=defend_waypoint_failed", true);
			return;
		}

		ActivateGroupAI(runtime.TransportPassengerGroup);
		runtime.TransportPassengerHandoffComplete = true;
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport infantry defend target=%1", capTarget.ToString()), true);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "transport infantry handoff complete=true", true);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("truck hold at drop point target=%1", runtime.DropPosition.ToString()), true);
	}

	protected bool AssignTransportPassengerDefendWaypoint(SCR_AIGroup group, vector capTarget, notnull array<IEntity> routeWaypoints)
	{
		routeWaypoints.Clear();
		if (!group)
			return false;

		JLH_DCF_NodeWaypointResolver.ClearGroupWaypoints(group);
		AIWaypoint defendWaypoint = SpawnDefendWaypoint(capTarget);
		if (!defendWaypoint)
			return false;

		group.AddWaypoint(defendWaypoint);
		routeWaypoints.Insert(defendWaypoint);
		return true;
	}

	protected int CollectTransportPassengerHandoffState(JLH_DCF_RecapVehicleRuntime runtime, notnull array<IEntity> onFootUnits, notnull array<IEntity> mountedUnits)
	{
		onFootUnits.Clear();
		mountedUnits.Clear();
		if (!runtime || !runtime.VehicleEntity)
			return 0;

		foreach (IEntity cachedPassenger : runtime.TransportPassengerUnits)
		{
			AddTransportPassengerHandoffState(runtime, cachedPassenger, onFootUnits, mountedUnits);
		}

		if (!onFootUnits.IsEmpty() || !mountedUnits.IsEmpty())
			return onFootUnits.Count() + mountedUnits.Count();

		AddTransportPassengerGroupState(runtime, runtime.CrewGroup, onFootUnits, mountedUnits);
		if (runtime.TransportPassengerGroup && runtime.TransportPassengerGroup != runtime.CrewGroup)
			AddTransportPassengerGroupState(runtime, runtime.TransportPassengerGroup, onFootUnits, mountedUnits);

		return onFootUnits.Count() + mountedUnits.Count();
	}

	protected void AddTransportPassengerGroupState(JLH_DCF_RecapVehicleRuntime runtime, SCR_AIGroup group, notnull array<IEntity> onFootUnits, notnull array<IEntity> mountedUnits)
	{
		if (!group)
			return;

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			AddTransportPassengerHandoffState(runtime, agent.GetControlledEntity(), onFootUnits, mountedUnits);
		}
	}

	protected void AddTransportPassengerHandoffState(JLH_DCF_RecapVehicleRuntime runtime, IEntity unit, notnull array<IEntity> onFootUnits, notnull array<IEntity> mountedUnits)
	{
		if (!runtime || !IsTransportPassengerCandidate(unit, runtime.VehicleEntity, runtime.DriverEntity, runtime.GunnerEntity))
			return;

		if (IsUnitInVehicle(unit, runtime.VehicleEntity))
		{
			if (!mountedUnits.Contains(unit))
				mountedUnits.Insert(unit);
			return;
		}

		if (!onFootUnits.Contains(unit))
			onFootUnits.Insert(unit);
	}

	protected int CollectTransportCargoPassengers(JLH_DCF_RecapVehicleRuntime runtime, notnull array<IEntity> cargoUnits)
	{
		cargoUnits.Clear();
		if (!runtime || !runtime.CrewGroup || !runtime.VehicleEntity)
			return 0;

		foreach (IEntity cachedPassenger : runtime.TransportPassengerUnits)
		{
			if (!IsTransportCargoPassenger(cachedPassenger, runtime.VehicleEntity, runtime.DriverEntity, runtime.GunnerEntity))
				continue;

			if (!cargoUnits.Contains(cachedPassenger))
				cargoUnits.Insert(cachedPassenger);
		}

		if (!cargoUnits.IsEmpty())
			return cargoUnits.Count();

		array<AIAgent> agents = {};
		runtime.CrewGroup.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!IsTransportCargoPassenger(unit, runtime.VehicleEntity, runtime.DriverEntity, runtime.GunnerEntity))
				continue;

			if (!cargoUnits.Contains(unit))
				cargoUnits.Insert(unit);
		}

		return cargoUnits.Count();
	}

	protected bool IsTransportCargoPassenger(IEntity unit, IEntity vehicle, IEntity driver, IEntity gunner)
	{
		if (!IsTransportPassengerCandidate(unit, vehicle, driver, gunner))
			return false;

		return IsUnitInVehicle(unit, vehicle);
	}

	protected bool IsTransportPassengerCandidate(IEntity unit, IEntity vehicle, IEntity driver, IEntity gunner)
	{
		if (!unit || !vehicle || !IsUnitAlive(unit))
			return false;

		if (driver && unit == driver)
			return false;

		if (gunner && unit == gunner)
			return false;

		if (IsUnitInCompartmentType(unit, vehicle, ECompartmentType.PILOT))
			return false;

		return !IsUnitInGunnerRole(unit, vehicle);
	}

	protected int RequestTransportPassengerAnimatedExit(JLH_DCF_RecapVehicleRuntime runtime, notnull array<IEntity> cargoUnits)
	{
		if (!runtime || !runtime.VehicleEntity)
			return 0;

		int requested = 0;
		foreach (IEntity unit : cargoUnits)
		{
			if (!unit || !IsUnitAlive(unit))
				continue;

			if (!IsUnitInVehicle(unit, runtime.VehicleEntity))
				continue;

			int delayMs = requested * TRANSPORT_EXIT_STAGGER_MS;
			if (delayMs <= 0 || !GetGame() || !GetGame().GetCallqueue())
			{
				if (!RequestTransportPassengerExit(unit, runtime.VehicleEntity))
					continue;

				requested++;
				continue;
			}

			GetGame().GetCallqueue().CallLater(JLH_DCF_RecapNodeComponent_RequestTransportPassengerExit, delayMs, false, this, unit, runtime.VehicleEntity);
			requested++;
		}

		return requested;
	}

	bool RequestTransportPassengerExit(IEntity unit, IEntity vehicle)
	{
		if (!unit || !vehicle || !IsUnitAlive(unit))
			return false;

		if (!IsUnitInVehicle(unit, vehicle))
			return false;

		SCR_CompartmentAccessComponent access = SCR_CompartmentAccessComponent.Cast(unit.FindComponent(SCR_CompartmentAccessComponent));
		if (!access)
			return false;

		return RequestAnimatedTransportExit(access);
	}

	protected bool RequestAnimatedTransportExit(SCR_CompartmentAccessComponent access)
	{
		if (!access)
			return false;

		if (access.IsGettingOut())
			return true;

		for (int doorIndex = 0; doorIndex < TRANSPORT_EXIT_DOOR_SCAN_COUNT; doorIndex++)
		{
			if (!access.CanGetOutVehicleViaDoor(doorIndex))
				continue;

			if (access.GetOutVehicle(EGetOutType.ANIMATED, doorIndex, ECloseDoorAfterActions.LEAVE_OPEN, false))
				return true;

			access.AskOwnerToGetOutFromVehicle(EGetOutType.ANIMATED, doorIndex, ECloseDoorAfterActions.LEAVE_OPEN, false, false);
			return true;
		}

		if (access.GetOutVehicle(EGetOutType.ANIMATED, 0, ECloseDoorAfterActions.LEAVE_OPEN, false))
			return true;

		access.AskOwnerToGetOutFromVehicle(EGetOutType.ANIMATED, 0, ECloseDoorAfterActions.LEAVE_OPEN, false, false);
		return true;
	}

	protected int MoveTransportPassengerUnitsToGroup(SCR_AIGroup sourceGroup, SCR_AIGroup targetGroup, notnull array<IEntity> units)
	{
		if (!targetGroup)
			return 0;

		int moved = 0;
		foreach (IEntity unit : units)
		{
			if (!unit || !IsUnitAlive(unit))
				continue;

			bool removed = true;
			if (sourceGroup && sourceGroup != targetGroup)
				removed = sourceGroup.RemoveAIEntityFromGroup(unit);

			if (targetGroup.AddAIEntityToGroup(unit))
			{
				moved++;
				continue;
			}

			if (removed && sourceGroup && sourceGroup != targetGroup)
				sourceGroup.AddAIEntityToGroup(unit);
		}

		return moved;
	}

	protected vector ResolveTransportPassengerCapTarget(vector capTarget, vector fallbackTarget)
	{
		if (vector.Distance(capTarget, "0 0 0") <= 0.01)
			return fallbackTarget;

		return capTarget;
	}

	protected float GetTransportHorizontalDistance(vector first, vector second)
	{
		first[1] = 0;
		second[1] = 0;
		return vector.Distance(first, second);
	}

	protected bool ShouldRetryVehicleMount(JLH_DCF_RecapVehicleRuntime runtime)
	{
		if (!runtime)
			return false;

		int maxRetries = VEHICLE_MOUNT_MAX_RETRIES;
		if (runtime.PassengerDeliveryMode)
			maxRetries = VEHICLE_TRANSPORT_MOUNT_MAX_RETRIES;

		if (runtime.MountRetryCount >= maxRetries)
			return false;

		return true;
	}

	protected bool MountVehicleCrew(SCR_AIGroup group, IEntity vehicle, ResourceName vehiclePrefab, JLH_DCF_VehicleMountStats stats, bool passengerDeliveryMode)
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
		bool likelyArmed = IsArmedVehicleSupportPrefab(vehiclePrefab);
		bool armedPolicyLogged = false;
		int replicationFailureCount = 0;
		int compartmentFailureCount = 0;
		int seatFailureCount = 0;
		int maxCargoSeats = GetMaxVehicleCrewCargoSeats(likelyArmed, passengerDeliveryMode);
		if (likelyArmed)
		{
			LogArmedVehicleMountPolicy();
			armedPolicyLogged = true;
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
				replicationFailureCount++;
				replicationNotReady = true;
				continue;
			}

			if (IsUnitInVehicle(unit, vehicle))
			{
				RecordExistingMountedUnit(unit, vehicle, stats);
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
				else if (CanMountVehicleCrewSeat(likelyArmed, passengerDeliveryMode, stats) && access.MoveInVehicle(vehicle, ECompartmentType.TURRET))
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
						maxCargoSeats = GetMaxVehicleCrewCargoSeats(true, passengerDeliveryMode);
						if (!armedPolicyLogged)
						{
							LogArmedVehicleMountPolicy();
							armedPolicyLogged = true;
						}
					}
				}
				else if (CanMountVehicleCrewCargoSeat(likelyArmed, passengerDeliveryMode, stats, maxCargoSeats) && access.MoveInVehicle(vehicle, ECompartmentType.CARGO))
				{
					stats.CargoSeatsFilled++;
					stats.SeatsFilled++;
					if (unit != stats.DriverEntity && unit != stats.GunnerEntity && !stats.PassengerEntities.Contains(unit))
						stats.PassengerEntities.Insert(unit);

					moved = true;
				}
			}
			else
			{
				compartmentFailureCount++;
				if (passengerDeliveryMode)
				{
					stats.RemovedUnassigned++;
					continue;
				}
			}

			if (moved)
				continue;

			seatFailureCount++;
			if (!passengerDeliveryMode)
			{
				SCR_EntityHelper.DeleteEntityAndChildren(unit);
				stats.RemovedUnassigned++;
			}
			else
			{
				stats.RemovedUnassigned++;
			}
		}

		if (!passengerDeliveryMode && (likelyArmed || stats.GunnerMounted))
			stats.RemovedUnassigned += PruneMountedVehicleCrewOverLimit(group, vehicle, ATTACK_VEHICLE_MAX_MOUNTED_CREW);

		int driverMounted = 0;
		if (stats.DriverMounted)
			driverMounted = 1;

		int gunnerMounted = 0;
		if (stats.GunnerMounted)
			gunnerMounted = 1;

		int commanderMounted = stats.CargoSeatsFilled;
		int passengersMounted = 0;
		if (passengerDeliveryMode)
		{
			commanderMounted = 0;
			passengersMounted = stats.CargoSeatsFilled;
		}

		string mountLog = string.Format("Mount complete base=%1 assaultId=%2 vehicle=%3 vehiclePos=%4 driver=%5 gunner=%6 commander=%7", m_sBaseName, m_iActiveAssaultId, JLH_DCF_NodeDebug.EntityLabel(vehicle), GetEntityPositionLabel(vehicle), driverMounted, gunnerMounted, commanderMounted);
		mountLog += string.Format(" passengers=%1 unused=%2", passengersMounted, stats.RemovedUnassigned);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, mountLog, true);
		if (passengerDeliveryMode)
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport group base=%1 assaultId=%2 vehicle=%3 passengers=%4 driver=%5", m_sBaseName, m_iActiveAssaultId, JLH_DCF_NodeDebug.EntityLabel(vehicle), TRANSPORT_PASSENGER_GROUP_SIZE, driverMounted), true);
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("passengerCapacity base=%1 assaultId=%2 vehicle=%3 capacity=%4 passengerGroupSize=%5 passengersMounted=%6 overflowOnFoot=%7", m_sBaseName, m_iActiveAssaultId, JLH_DCF_NodeDebug.EntityLabel(vehicle), TRANSPORT_TOTAL_CAPACITY, TRANSPORT_PASSENGER_GROUP_SIZE, passengersMounted, stats.RemovedUnassigned), true);
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport mounted base=%1 assaultId=%2 vehicle=%3 driver=%4 passengers=%5 overflowOnFoot=%6", m_sBaseName, m_iActiveAssaultId, JLH_DCF_NodeDebug.EntityLabel(vehicle), driverMounted, passengersMounted, stats.RemovedUnassigned), true);
		}

		if (replicationFailureCount > 0 || compartmentFailureCount > 0 || seatFailureCount > 0)
		{
			LogDebug(string.Format("seat assignment summary replicationNotReady=%1 noCompartment=%2 noSeat=%3", replicationFailureCount, compartmentFailureCount, seatFailureCount));
		}

		if (passengerDeliveryMode && stats.DriverMounted && stats.CargoSeatsFilled == TRANSPORT_PASSENGER_GROUP_SIZE && stats.RemovedUnassigned == 0)
			stats.FailureReason = "";
		else if (passengerDeliveryMode)
			stats.FailureReason = "transport_partial_mount";
		else if (stats.CrewSeen <= 0)
			stats.FailureReason = "crew_entity_invalid";
		else if (replicationNotReady && stats.SeatsFilled <= 0)
			stats.FailureReason = "replication_not_ready";
		else if (!stats.HasCompartmentAccess)
			stats.FailureReason = "vehicle_compartment_api_failed";
		else if (!stats.DriverMounted)
			stats.FailureReason = "no_driver";
		else if (likelyArmed && !stats.GunnerMounted)
			stats.FailureReason = "no_gunner";
		else
			stats.FailureReason = "";

		if (passengerDeliveryMode && stats.FailureReason != "")
			return false;

		if (likelyArmed && !stats.GunnerMounted)
			return false;

		return stats.DriverMounted && stats.SeatsFilled > 0;
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

	protected void LogMountCandidateState(SCR_AIGroup group, IEntity vehicle, string phase)
	{
		int agentCount = 0;
		int candidateCount = 0;
		if (group)
		{
			array<AIAgent> agents = {};
			group.GetAgents(agents);
			agentCount = agents.Count();
			foreach (AIAgent agent : agents)
			{
				if (agent && agent.GetControlledEntity())
					candidateCount++;
			}
		}

		LogDebug(string.Format("mount audit base=%1 assaultId=%2 phase=%3 groupEntity=%4 aiGroup=%5 vehicle=%6 vehiclePos=%7 agentCount=%8 candidateCount=%9", m_sBaseName, m_iActiveAssaultId, phase, JLH_DCF_NodeDebug.EntityLabel(group), JLH_DCF_NodeDebug.BoolLabel(group != null), JLH_DCF_NodeDebug.EntityLabel(vehicle), GetEntityPositionLabel(vehicle), agentCount, candidateCount));
	}

	protected void LogArmedSupportVehicleGroupMount(IEntity vehicle, JLH_DCF_VehicleMountStats stats)
	{
		int driverMounted = 0;
		if (stats && stats.DriverMounted)
			driverMounted = 1;

		int gunnerMounted = 0;
		if (stats && stats.GunnerMounted)
			gunnerMounted = 1;

		int commanderMounted = 0;
		if (stats)
			commanderMounted = stats.CargoSeatsFilled;

		int removed = 0;
		if (stats)
			removed = stats.RemovedUnassigned;

		bool turretUsable = stats && stats.GunnerMounted;
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("mounted base=%1 assaultId=%2 vehicle=%3 vehiclePos=%4 driver=%5 gunner=%6 commander=%7 passengers=0 deletedUnused=%8", m_sBaseName, m_iActiveAssaultId, JLH_DCF_NodeDebug.EntityLabel(vehicle), GetEntityPositionLabel(vehicle), driverMounted, gunnerMounted, commanderMounted, removed), true);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("turret usable base=%1 assaultId=%2 vehicle=%3 usable=%4", m_sBaseName, m_iActiveAssaultId, JLH_DCF_NodeDebug.EntityLabel(vehicle), JLH_DCF_NodeDebug.BoolLabel(turretUsable)), true);
	}

	protected int GetMaxVehicleCrewCargoSeats(bool likelyArmed, bool passengerDeliveryMode)
	{
		if (passengerDeliveryMode || !likelyArmed)
			return TRANSPORT_PASSENGER_GROUP_SIZE;

		int maxCargoSeats = ATTACK_VEHICLE_MAX_MOUNTED_CREW - 2;
		if (maxCargoSeats < 0)
			return 0;

		return maxCargoSeats;
	}

	protected bool CanMountVehicleCrewCargoSeat(bool likelyArmed, bool passengerDeliveryMode, JLH_DCF_VehicleMountStats stats, int maxCargoSeats)
	{
		if (!stats || stats.CargoSeatsFilled >= maxCargoSeats)
			return false;

		return CanMountVehicleCrewSeat(likelyArmed, passengerDeliveryMode, stats);
	}

	protected bool CanMountVehicleCrewSeat(bool likelyArmed, bool passengerDeliveryMode, JLH_DCF_VehicleMountStats stats)
	{
		if (!stats)
			return false;

		if (passengerDeliveryMode || (!likelyArmed && !stats.GunnerMounted))
			return true;

		return stats.SeatsFilled < ATTACK_VEHICLE_MAX_MOUNTED_CREW;
	}

	protected int PruneMountedVehicleCrewOverLimit(SCR_AIGroup group, IEntity vehicle, int maxMountedCrew)
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

			SCR_EntityHelper.DeleteEntityAndChildren(unit);
			removed++;
		}

		if (removed > 0)
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Armed support mount policy pruned extraMountedCrew=%1 maxCrew=%2", removed, maxMountedCrew), true);

		return removed;
	}

	protected void LogArmedVehicleMountPolicy()
	{
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Armed support mount policy maxCrew=%1", ATTACK_VEHICLE_MAX_MOUNTED_CREW), true);
	}

	protected bool IsUnitInVehicle(IEntity unit, IEntity vehicle)
	{
		if (!unit || !vehicle)
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

	protected void RecordExistingMountedUnit(IEntity unit, IEntity vehicle, JLH_DCF_VehicleMountStats stats)
	{
		if (!unit || !vehicle || !stats)
			return;

		stats.HasCompartmentAccess = true;
		stats.SeatsFilled++;

		if (!stats.DriverMounted && IsUnitInCompartmentType(unit, vehicle, ECompartmentType.PILOT))
		{
			stats.DriverMounted = true;
			stats.DriverEntity = unit;
			return;
		}

		if (IsUnitInGunnerRole(unit, vehicle))
		{
			stats.TurretSeatsFilled++;
			if (!stats.GunnerMounted)
			{
				stats.GunnerMounted = true;
				stats.GunnerEntity = unit;
			}
			return;
		}

		if (IsUnitInCompartmentType(unit, vehicle, ECompartmentType.CARGO))
		{
			stats.CargoSeatsFilled++;
			if (unit != stats.DriverEntity && unit != stats.GunnerEntity && !stats.PassengerEntities.Contains(unit))
				stats.PassengerEntities.Insert(unit);
		}
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

		return SCR_CompartmentAccessComponent.GetCompartmentType(compartment) == compartmentType;
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
			ECompartmentType compartmentType = SCR_CompartmentAccessComponent.GetCompartmentType(compartment);
			if (compartmentType == ECompartmentType.TURRET)
				return true;

			if (compartmentType == ECompartmentType.PILOT || compartmentType == ECompartmentType.CARGO)
				return false;
		}

		return access.GetControlledFireModeManager() != null;
	}

	protected void MarkVehicleSupportComplete(JLH_DCF_RecapVehicleRuntime runtime, string reason, int now)
	{
		if (!runtime || runtime.SupportState != JLH_DCF_RecapVehicleState.ACTIVE_SUPPORT)
			return;

		if (ShouldRetainVehicleRuntimeAsBaseDefence(runtime, reason))
		{
			RetainRecapTransportAsBaseDefence(runtime);
			return;
		}

		runtime.SupportState = JLH_DCF_RecapVehicleState.SUPPORT_COMPLETE;
		runtime.SupportCompletedTick = now;
		runtime.NextCleanupAttemptTick = now + VEHICLE_CLEANUP_RETRY_MS;
		ReleaseTransportLockForRuntime(runtime, reason);
		RemoveActiveVehicle(runtime);
		if (!m_aVehiclesAwaitingCleanup.Contains(runtime))
			m_aVehiclesAwaitingCleanup.Insert(runtime);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle support no longer active reason=%1", reason), true);
	}

	protected void RetainRecapTransportAsBaseDefence(JLH_DCF_RecapVehicleRuntime runtime)
	{
		if (!runtime)
			return;

		RemoveActiveVehicle(runtime);
		RemoveAttackGroupAndUnits(runtime.CrewGroup);
		DetachRouteWaypointsFromRuntimeCleanup(runtime.RouteWaypoints);
		if (m_RuntimeTracker)
		{
			m_RuntimeTracker.ForgetGroup(runtime.CrewGroup);
			m_RuntimeTracker.ForgetVehicle(runtime.VehicleEntity);
		}

		if (runtime.CrewGroup && !m_aRetainedDefenderGroups.Contains(runtime.CrewGroup))
			m_aRetainedDefenderGroups.Insert(runtime.CrewGroup);

		foreach (IEntity unit : runtime.CrewUnits)
		{
			if (unit && !m_aRetainedDefenderUnits.Contains(unit))
				m_aRetainedDefenderUnits.Insert(unit);
		}

		if (runtime.VehicleEntity && !m_aRetainedDefenderVehicles.Contains(runtime.VehicleEntity))
			m_aRetainedDefenderVehicles.Insert(runtime.VehicleEntity);

		if (!m_aRetainedTransportRuntimes.Contains(runtime))
			m_aRetainedTransportRuntimes.Insert(runtime);

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "transport retained as base defence=true", true);
	}

	protected void TickRetainedTransportRuntimes(int now)
	{
		for (int i = m_aRetainedTransportRuntimes.Count() - 1; i >= 0; i--)
		{
			JLH_DCF_RecapVehicleRuntime runtime = m_aRetainedTransportRuntimes[i];
			if (!runtime)
			{
				m_aRetainedTransportRuntimes.Remove(i);
				continue;
			}

			if (!runtime.VehicleEntity || IsEntityDestroyed(runtime.VehicleEntity))
			{
				ReleaseTransportLockForRuntime(runtime, "vehicle_destroyed");
				m_aRetainedTransportRuntimes.Remove(i);
				continue;
			}

			if (CountLivingRetainedTransportUnits(runtime) <= 0)
			{
				ReleaseTransportLockForRuntime(runtime, "crew_dead");
				m_aRetainedTransportRuntimes.Remove(i);
			}
		}
	}

	protected int CountLivingRetainedTransportUnits(JLH_DCF_RecapVehicleRuntime runtime)
	{
		if (!runtime)
			return 0;

		int living = CountLivingGroupUnits(runtime.CrewGroup);
		if (runtime.TransportPassengerGroup && runtime.TransportPassengerGroup != runtime.CrewGroup)
			living += CountLivingGroupUnits(runtime.TransportPassengerGroup);

		return living;
	}

	protected void RemoveAttackGroupAndUnits(SCR_AIGroup group)
	{
		if (!group)
			return;

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!unit)
				continue;

			for (int i = m_aAttackUnits.Count() - 1; i >= 0; i--)
			{
				if (m_aAttackUnits[i] == unit)
					m_aAttackUnits.Remove(i);
			}
		}

		for (int j = m_aAttackGroups.Count() - 1; j >= 0; j--)
		{
			if (m_aAttackGroups[j] == group)
				m_aAttackGroups.Remove(j);
		}
	}

	protected void DetachRouteWaypointsFromRuntimeCleanup(array<IEntity> routeWaypoints)
	{
		if (!routeWaypoints)
			return;

		foreach (IEntity waypoint : routeWaypoints)
		{
			for (int i = m_aRuntimeRouteWaypoints.Count() - 1; i >= 0; i--)
			{
				if (m_aRuntimeRouteWaypoints[i] == waypoint)
					m_aRuntimeRouteWaypoints.Remove(i);
			}
		}

		routeWaypoints.Clear();
	}

	protected void TickVehicleCleanup(int now)
	{
		for (int i = m_aVehiclesAwaitingCleanup.Count() - 1; i >= 0; i--)
		{
			JLH_DCF_RecapVehicleRuntime runtime = m_aVehiclesAwaitingCleanup[i];
			if (!runtime)
			{
				m_aVehiclesAwaitingCleanup.Remove(i);
				continue;
			}

			if (runtime.NextCleanupAttemptTick > now)
				continue;

			if (ShouldRetainVehicleRuntimeAsBaseDefence(runtime, "cleanup"))
			{
				m_aVehiclesAwaitingCleanup.Remove(i);
				RetainRecapTransportAsBaseDefence(runtime);
				continue;
			}

			string deferredReason;
			if (!CanCleanupVehicleSafely(runtime, now, deferredReason))
			{
				LogCleanupDeferred(deferredReason, now);
				runtime.NextCleanupAttemptTick = now + VEHICLE_CLEANUP_RETRY_MS;
				continue;
			}

			RemoveRuntimeRouteWaypoints(runtime);
			int crewRemoved = CleanupVehicleCrew(runtime);
			int vehicleRemoved = CleanupVehicleEntity(runtime);
			runtime.SupportState = JLH_DCF_RecapVehicleState.SAFE_TO_REMOVE;
			m_aVehiclesAwaitingCleanup.Remove(i);
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Cleanup safe removed vehicles=%1 crew=%2", vehicleRemoved, crewRemoved), true);
		}
	}

	protected bool CanCleanupVehicleSafely(JLH_DCF_RecapVehicleRuntime runtime, int now, out string reason)
	{
		reason = "";
		if (!runtime)
			return true;

		if (m_bAttackActive && m_aActiveVehicles.Contains(runtime))
		{
			reason = "active_assault";
			return false;
		}

		if (ShouldRetainVehicleRuntimeForGhostDefence(runtime))
		{
			reason = "ghost_handoff";
			return false;
		}

		bool failedMount = runtime.MountFailureReason != "";
		if (!failedMount && runtime.VehicleEntity && !IsEntityDestroyed(runtime.VehicleEntity) && CountLivingVehicleCrew(runtime) > 0)
		{
			reason = "living_crew";
			return false;
		}

		if (now - runtime.SupportCompletedTick < VEHICLE_CLEANUP_GRACE_MS)
		{
			reason = "recent_combat";
			return false;
		}

		JLH_CleanupSafetyQuery query = new JLH_CleanupSafetyQuery();
		vector anchor = runtime.AssignedWaypointPosition;
		if (runtime.VehicleEntity)
			anchor = runtime.VehicleEntity.GetOrigin();
		else if (runtime.CrewGroup)
			anchor = runtime.CrewGroup.GetOrigin();
		query.Init("RecapVehicleSupport vehicle=" + runtime.VehicleIndex.ToString(), anchor);
		query.CleanupDistance = VEHICLE_CLEANUP_PLAYER_RADIUS;
		query.VisibilityDistance = Math.Max(VEHICLE_CLEANUP_PLAYER_RADIUS, JLH_CleanupSafetyService.DEFAULT_VISIBILITY_DISTANCE_METERS);
		query.PlayerFactionFilter = "";
		query.DebugEnabled = DebugLogging;
		query.LastInteractionTick = runtime.SupportCompletedTick;
		if (runtime.CrewGroup)
			query.Groups.Insert(runtime.CrewGroup);
		if (runtime.VehicleEntity)
			query.Vehicles.Insert(runtime.VehicleEntity);
		foreach (IEntity unit : runtime.CrewUnits)
		{
			if (!unit)
				continue;

			query.Entities.Insert(unit);
		}

		string blockedReason;
		if (!JLH_CleanupSafetyService.CanCleanupPackage(query, blockedReason))
		{
			reason = blockedReason;
			return false;
		}

		return true;
	}

	protected void LogCleanupDeferred(string reason, int now)
	{
		if (m_iLastCleanupDeferredLogTick > 0 && now - m_iLastCleanupDeferredLogTick < SPAWN_SAFETY_LOG_THROTTLE_MS)
			return;

		m_iLastCleanupDeferredLogTick = now;
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Cleanup deferred reason=%1", reason), true);
	}

	protected int CleanupVehicleCrew(JLH_DCF_RecapVehicleRuntime runtime)
	{
		if (!runtime)
			return 0;

		int removed = 0;
		foreach (IEntity unit : runtime.CrewUnits)
		{
			if (!unit)
				continue;

			DeleteRuntimeEntitySafe(unit, "recap_vehicle_crew_cleanup");
			removed++;
		}

		if (runtime.CrewGroup)
			DeleteRuntimeEntitySafe(runtime.CrewGroup, "recap_vehicle_crew_group_cleanup");

		return removed;
	}

	protected int CleanupVehicleEntity(JLH_DCF_RecapVehicleRuntime runtime)
	{
		if (!runtime || !runtime.VehicleEntity)
			return 0;

		JLH_DCF_VehicleSustainment.UnregisterVehicle(runtime.VehicleEntity, "cleanup");
		DeleteRuntimeEntitySafe(runtime.VehicleEntity, "recap_vehicle_cleanup");
		return 1;
	}

	protected void RemoveActiveVehicle(JLH_DCF_RecapVehicleRuntime runtime)
	{
		for (int i = m_aActiveVehicles.Count() - 1; i >= 0; i--)
		{
			if (m_aActiveVehicles[i] == runtime)
				m_aActiveVehicles.Remove(i);
		}
	}

	protected bool IsVehicleCombatEffective(JLH_DCF_RecapVehicleRuntime runtime, out string reason)
	{
		reason = "";
		if (!runtime || !runtime.VehicleEntity || IsEntityDestroyed(runtime.VehicleEntity))
		{
			reason = "vehicle_destroyed";
			return false;
		}

		if (runtime.PassengerDeliveryMode)
		{
			if (CountLivingVehicleCrew(runtime) <= 0)
			{
				reason = "crew_dead";
				return false;
			}

			return true;
		}

		bool driverAlive = IsUnitAlive(runtime.DriverEntity);
		bool gunnerAlive = IsUnitAlive(runtime.GunnerEntity);
		if (!driverAlive && !gunnerAlive)
		{
			reason = "crew_dead";
			return false;
		}

		return true;
	}

	protected void CollectVehicleCrewUnits(JLH_DCF_RecapVehicleRuntime runtime)
	{
		if (!runtime || !runtime.CrewGroup)
			return;

		runtime.CrewUnits.Clear();
		array<AIAgent> agents = {};
		runtime.CrewGroup.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (unit && !runtime.CrewUnits.Contains(unit))
				runtime.CrewUnits.Insert(unit);
		}
	}

	protected void TrackVehicleCrewUnits(JLH_DCF_RecapVehicleRuntime runtime)
	{
		if (!runtime)
			return;

		foreach (IEntity unit : runtime.CrewUnits)
		{
			if (unit && !m_aAttackUnits.Contains(unit))
				m_aAttackUnits.Insert(unit);
		}
	}

	protected bool CanSpawnAssaultSafely(bool includesVehicle)
	{
		m_SelectedSpawnNode = null;
		if (!EnableSpawnVisibilitySafety)
		{
			m_SelectedSpawnNode = this;
			ResetSpawnSafetyDeferral();
			return true;
		}

		float safetyDistance = MinimumInfantrySpawnSafetyDistance;
		if (includesVehicle && MinimumVehicleSpawnSafetyDistance > safetyDistance)
			safetyDistance = MinimumVehicleSpawnSafetyDistance;

		ref array<IEntity> players = {};
		GatherSpawnSafetyPlayers(players);
		string preferredReason;
		float preferredDistance;
		bool preferredVisible;
		if (EvaluateSpawnVisibilitySafetyWithPlayers(safetyDistance, players, preferredReason, preferredDistance, preferredVisible))
		{
			m_SelectedSpawnNode = this;
			LogSpawnSafetyPassed(this, preferredReason);
			ResetSpawnSafetyDeferral();
			return true;
		}

		JLH_RecapNodeComponent alternateNode;
		string alternateReason;
		if (TrySelectAlternateSpawnNodeWithPlayers(safetyDistance, players, alternateNode, alternateReason))
		{
			m_SelectedSpawnNode = alternateNode;
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Spawn safety alternate node selected from=%1 to=%2 reason=preferred_visible", JLH_DCF_NodeDebug.EntityLabel(GetOwner()), JLH_DCF_NodeDebug.EntityLabel(alternateNode.GetOwner())), true);
			LogSpawnSafetyPassed(alternateNode, alternateReason);
			ResetSpawnSafetyDeferral();
			return true;
		}

		int now = System.GetTickCount();
		if (m_iSpawnSafetyDeferralStartedTick <= 0)
			m_iSpawnSafetyDeferralStartedTick = now;

		if (AllowUnsafeFallbackAfterTimeout && HasSpawnSafetyDeferralTimedOut(now))
		{
			JLH_RecapNodeComponent fallbackNode = SelectLeastBadSameBaseSpawnNodeWithPlayers(safetyDistance, players);
			if (!fallbackNode)
				fallbackNode = this;

			m_SelectedSpawnNode = fallbackNode;
			JLH_DCF_NodeDebug.Warn(SYSTEM_NAME, string.Format("Spawn safety fallback allowed node=%1 reason=timeout leastBad=true", JLH_DCF_NodeDebug.EntityLabel(fallbackNode.GetOwner())));
			ResetSpawnSafetyDeferral();
			return true;
		}

		LogSpawnSafetyDeferred(preferredReason);
		return false;
	}

	bool EvaluateSpawnVisibilitySafety(float safetyDistance, out string reason, out float closestDistance, out bool visibleToPlayer)
	{
		ref array<IEntity> players = {};
		GatherSpawnSafetyPlayers(players);
		return EvaluateSpawnVisibilitySafetyWithPlayers(safetyDistance, players, reason, closestDistance, visibleToPlayer);
	}

	bool EvaluateSpawnVisibilitySafetyWithPlayers(float safetyDistance, notnull array<IEntity> players, out string reason, out float closestDistance, out bool visibleToPlayer)
	{
		reason = "distance_clear";
		closestDistance = -1.0;
		visibleToPlayer = false;

		IEntity owner = GetOwner();
		if (!owner)
		{
			reason = "node_missing";
			return false;
		}

		vector spawnPosition = owner.GetOrigin();
		bool closePlayerFound = false;

		foreach (IEntity controlled : players)
		{
			if (!IsValidPlayer(controlled))
				continue;

			float distance = vector.Distance(spawnPosition, controlled.GetOrigin());
			if (closestDistance < 0.0 || distance < closestDistance)
				closestDistance = distance;

			if (distance >= safetyDistance)
				continue;

			closePlayerFound = true;
			if (!RequireLOSToBlockCloseSpawn || HasDirectLineOfSight(controlled, spawnPosition))
				visibleToPlayer = true;
		}

		if (!closePlayerFound)
		{
			reason = "distance_clear";
			return true;
		}

		if (!visibleToPlayer)
		{
			reason = "close_not_visible";
			return true;
		}

		reason = "visible_to_player";
		return false;
	}

	protected bool TrySelectAlternateSpawnNode(float safetyDistance, out JLH_RecapNodeComponent selectedNode, out string selectedReason)
	{
		ref array<IEntity> players = {};
		GatherSpawnSafetyPlayers(players);
		return TrySelectAlternateSpawnNodeWithPlayers(safetyDistance, players, selectedNode, selectedReason);
	}

	protected bool TrySelectAlternateSpawnNodeWithPlayers(float safetyDistance, notnull array<IEntity> players, out JLH_RecapNodeComponent selectedNode, out string selectedReason)
	{
		selectedNode = null;
		selectedReason = "";

		ref array<JLH_RecapNodeComponent> nodes = {};
		JLH_RecapManager.CollectRegisteredNodes(nodes);

		float bestScore = -1.0;
		foreach (JLH_RecapNodeComponent node : nodes)
		{
			if (!IsValidSameBaseSpawnNode(node) || node == this)
				continue;

			string reason;
			float closestDistance;
			bool visible;
			if (!node.EvaluateSpawnVisibilitySafetyWithPlayers(safetyDistance, players, reason, closestDistance, visible))
				continue;

			float score = GetSpawnNodeScore(reason, closestDistance, visible);
			if (!selectedNode || score > bestScore)
			{
				selectedNode = node;
				selectedReason = reason;
				bestScore = score;
			}
		}

		return selectedNode != null;
	}

	protected JLH_RecapNodeComponent SelectLeastBadSameBaseSpawnNode(float safetyDistance)
	{
		ref array<IEntity> players = {};
		GatherSpawnSafetyPlayers(players);
		return SelectLeastBadSameBaseSpawnNodeWithPlayers(safetyDistance, players);
	}

	protected JLH_RecapNodeComponent SelectLeastBadSameBaseSpawnNodeWithPlayers(float safetyDistance, notnull array<IEntity> players)
	{
		JLH_RecapNodeComponent selectedNode = null;
		float bestScore = -1.0;
		ref array<JLH_RecapNodeComponent> nodes = {};
		JLH_RecapManager.CollectRegisteredNodes(nodes);

		foreach (JLH_RecapNodeComponent node : nodes)
		{
			if (!IsValidSameBaseSpawnNode(node))
				continue;

			string reason;
			float closestDistance;
			bool visible;
			node.EvaluateSpawnVisibilitySafetyWithPlayers(safetyDistance, players, reason, closestDistance, visible);
			float score = GetSpawnNodeScore(reason, closestDistance, visible);
			if (!selectedNode || score > bestScore)
			{
				selectedNode = node;
				bestScore = score;
			}
		}

		return selectedNode;
	}

	protected float GetSpawnNodeScore(string reason, float closestDistance, bool visible)
	{
		float score = closestDistance;
		if (score < 0.0)
			score = 999999.0;

		if (!visible)
			score = score + 50000.0;
		if (reason == "distance_clear")
			score = score + 100000.0;
		else if (reason == "close_not_visible")
			score = score + 50000.0;

		return score;
	}

	protected bool IsValidSameBaseSpawnNode(JLH_RecapNodeComponent node)
	{
		if (!node || !node.IsRegistered() || !node.GetOwner())
			return false;

		return node.GetRegisteredBaseName() == m_sBaseName;
	}

	protected bool HasSpawnSafetyDeferralTimedOut(int now)
	{
		if (m_iSpawnSafetyDeferralStartedTick <= 0)
			return false;

		int deferralMs = SecondsToMs(MaximumSpawnSafetyDeferral);
		if (deferralMs <= 0)
			return true;

		return now - m_iSpawnSafetyDeferralStartedTick >= deferralMs;
	}

	protected void ResetSpawnSafetyDeferral()
	{
		m_iNextSpawnSafetyRetryTick = 0;
		m_iSpawnSafetyDeferralStartedTick = 0;
		m_sLastSpawnSafetyLogKey = "";
		m_iLastSpawnSafetyLogTick = 0;
	}

	protected void LogSpawnSafetyPassed(JLH_RecapNodeComponent node, string reason)
	{
		if (!node || !ShouldLogSpawnSafetyKey(string.Format("pass_%1_%2", reason, JLH_DCF_NodeDebug.EntityLabel(node.GetOwner()))))
			return;

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Spawn safety passed node=%1 reason=%2", JLH_DCF_NodeDebug.EntityLabel(node.GetOwner()), reason), true);
	}

	protected void LogSpawnSafetyDeferred(string reason)
	{
		if (!ShouldLogSpawnSafetyKey(string.Format("defer_%1", reason)))
			return;

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Spawn safety deferred node=%1 reason=%2 retry=%3", JLH_DCF_NodeDebug.EntityLabel(GetOwner()), reason, SpawnSafetyRetryDelay), true);
	}

	protected bool ShouldLogSpawnSafetyKey(string key)
	{
		int now = System.GetTickCount();
		if (m_sLastSpawnSafetyLogKey == key && now - m_iLastSpawnSafetyLogTick < SPAWN_SAFETY_LOG_THROTTLE_MS)
			return false;

		m_sLastSpawnSafetyLogKey = key;
		m_iLastSpawnSafetyLogTick = now;
		return true;
	}

	protected ResourceName ResolveGroupPrefab()
	{
		ResourceName resolverPrefab;
		if (JLH_DCF_DEVEnemyAssetResolver.TryResolveEnemyGroupPrefabByRole(JLH_EnemyAssetRole.RECAP_ASSAULT, resolverPrefab))
			return resolverPrefab;

		SCR_EntityCatalog catalog = ResolveFactionCatalog(EEntityCatalogType.GROUP);
		if (!catalog)
			return ResourceName.Empty;

		array<SCR_EntityCatalogEntry> entries = {};
		catalog.GetEntityList(entries);
		array<ResourceName> candidates = {};
		foreach (SCR_EntityCatalogEntry entry : entries)
		{
			ResourceName prefab;
			if (!IsGroupCatalogEntryAllowed(entry, prefab))
				continue;

			candidates.Insert(prefab);
		}

		if (candidates.IsEmpty())
			return ResourceName.Empty;

		if (!RecapPickRandomGroupType)
			return candidates[0];

		return candidates[Math.RandomInt(0, candidates.Count())];
	}

	protected bool IsGroupCatalogEntryAllowed(SCR_EntityCatalogEntry entry, out ResourceName prefab)
	{
		prefab = ResourceName.Empty;
		if (!entry || !entry.IsEnabled())
			return false;

		prefab = entry.GetPrefab();
		if (IsEmptyResource(prefab) || IsUnsupportedGroupPrefab(prefab))
			return false;

		if (!JLH_AddonSpawnUtility.CanLoadPrefab(prefab))
			return false;

		return true;
	}

	protected bool IsUnsupportedGroupPrefab(ResourceName prefab)
	{
		if (IsEmptyResource(prefab))
			return true;

		string token = prefab;
		token.ToLower();

		if (token.Contains("/vehicles/")) return true;
		if (token.Contains("/characters/")) return true;
		if (token.Contains("/weapons/")) return true;
		if (token.Contains("civilian")) return true;
		if (token.Contains("/civ/")) return true;
		if (token.Contains("support")) return true;
		if (token.Contains("logistic")) return true;
		if (token.Contains("transport")) return true;
		if (token.Contains("medical")) return true;
		if (token.Contains("medic")) return true;
		if (token.Contains("supply")) return true;
		if (token.Contains("repair")) return true;
		if (token.Contains("fuel")) return true;
		if (token.Contains("ammo")) return true;
		if (token.Contains("crew")) return true;
		if (token.Contains("pilot")) return true;

		return false;
	}

	protected ResourceName ResolveVehiclePrefab(bool transportMode)
	{
		if (transportMode)
			return ResolveTransportVehiclePrefab();

		return ResolveArmedSupportVehiclePrefab();
	}

	protected ResourceName ResolveArmedSupportVehiclePrefab()
	{
		if (!IsEmptyResource(SpecificVehiclePrefab))
		{
			if (!IsSpecificVehiclePrefabAllowed(SpecificVehiclePrefab))
				return ResourceName.Empty;

			return SpecificVehiclePrefab;
		}

		ResourceName pooledPrefab;
		if (TrySelectConfiguredVehiclePrefab(ArmedSupportVehiclePrefabs, false, pooledPrefab))
			return pooledPrefab;

		NormalizeVehicleTraitFilters();
		SCR_EntityCatalog catalog = ResolveFactionCatalog(EEntityCatalogType.VEHICLE);
		if (!catalog)
			return ResourceName.Empty;

		array<SCR_EntityCatalogEntry> entries = {};
		catalog.GetEntityList(entries);
		array<SCR_EntityCatalogEntry> candidates = {};
		foreach (SCR_EntityCatalogEntry entry : entries)
		{
			if (!entry || !entry.IsEnabled())
				continue;

			if (!MatchesVehicleTraitFilters(entry))
				continue;

			if (IsEmptyResource(entry.GetPrefab()))
				continue;

			candidates.Insert(entry);
		}

		if (candidates.IsEmpty())
			return ResourceName.Empty;

		SCR_EntityCatalogEntry selected = candidates.GetRandomElement();
		if (!selected)
			return ResourceName.Empty;

		return selected.GetPrefab();
	}

	protected ResourceName ResolveTransportVehiclePrefab()
	{
		ResourceName pooledPrefab;
		if (TrySelectConfiguredVehiclePrefab(TransportVehiclePrefabs, true, pooledPrefab))
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport vehicle selected=%1", pooledPrefab), true);
			return pooledPrefab;
		}

		if (!IsEmptyResource(SpecificVehiclePrefab))
		{
			string rejectReason;
			if (IsTransportVehiclePrefabAllowed(SpecificVehiclePrefab, rejectReason))
			{
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport vehicle selected=%1", SpecificVehiclePrefab), true);
				return SpecificVehiclePrefab;
			}

			LogDebug(string.Format("rejected vehicle reason=%1 prefab=%2", rejectReason, SpecificVehiclePrefab));
		}

		if (TrySelectDefaultTransportVehiclePrefab(pooledPrefab))
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport vehicle selected=%1", pooledPrefab), true);
			return pooledPrefab;
		}

		return ResolveTransportVehicleFromCatalog();
	}

	protected bool TrySelectConfiguredVehiclePrefab(array<ResourceName> prefabPool, bool transportOnly, out ResourceName selectedPrefab)
	{
		selectedPrefab = ResourceName.Empty;
		if (!prefabPool || prefabPool.IsEmpty())
			return false;

		array<ResourceName> candidates = {};
		foreach (ResourceName prefab : prefabPool)
		{
			if (IsEmptyResource(prefab))
				continue;

			if (!JLH_AddonSpawnUtility.CanLoadPrefab(prefab))
				continue;

			if (transportOnly)
			{
				string rejectReason;
				if (!IsTransportVehiclePrefabAllowed(prefab, rejectReason))
				{
					LogDebug(string.Format("rejected vehicle reason=%1 prefab=%2", rejectReason, prefab));
					continue;
				}
			}
			else if (!IsSpecificVehiclePrefabAllowed(prefab))
			{
				continue;
			}

			candidates.Insert(prefab);
		}

		if (candidates.IsEmpty())
			return false;

		selectedPrefab = candidates.GetRandomElement();
		return !IsEmptyResource(selectedPrefab);
	}

	protected bool TrySelectDefaultTransportVehiclePrefab(out ResourceName selectedPrefab)
	{
		selectedPrefab = ResourceName.Empty;
		array<ResourceName> candidates = {};
		AddValidTransportVehicleCandidate(DEFAULT_USSR_TRANSPORT_VEHICLE_PREFAB, candidates);

		if (candidates.IsEmpty())
			return false;

		selectedPrefab = candidates.GetRandomElement();
		return !IsEmptyResource(selectedPrefab);
	}

	protected void AddValidTransportVehicleCandidate(ResourceName prefab, notnull array<ResourceName> candidates)
	{
		if (IsEmptyResource(prefab))
			return;

		if (!JLH_AddonSpawnUtility.CanLoadPrefab(prefab))
			return;

		string rejectReason;
		if (!IsTransportVehiclePrefabAllowed(prefab, rejectReason))
			return;

		candidates.Insert(prefab);
	}

	protected ResourceName ResolveTransportVehicleFromCatalog()
	{
		SCR_EntityCatalog catalog = ResolveFactionCatalog(EEntityCatalogType.VEHICLE);
		if (!catalog)
			return ResourceName.Empty;

		array<SCR_EntityCatalogEntry> entries = {};
		catalog.GetEntityList(entries);
		array<ResourceName> candidates = {};
		foreach (SCR_EntityCatalogEntry entry : entries)
		{
			if (!entry || !entry.IsEnabled())
				continue;

			ResourceName prefab = entry.GetPrefab();
			if (IsEmptyResource(prefab))
				continue;

			string rejectReason;
			if (!IsTransportVehiclePrefabAllowed(prefab, rejectReason))
			{
				if (rejectReason == "armed_support_vehicle")
					LogDebug(string.Format("rejected vehicle reason=%1 prefab=%2", rejectReason, prefab));

				continue;
			}

			candidates.Insert(prefab);
		}

		if (candidates.IsEmpty())
			return ResourceName.Empty;

		ResourceName selectedPrefab = candidates.GetRandomElement();
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport vehicle selected=%1", selectedPrefab), true);
		return selectedPrefab;
	}

	protected bool IsTransportVehiclePrefabAllowed(ResourceName vehiclePrefab, out string rejectReason)
	{
		rejectReason = "";
		if (IsEmptyResource(vehiclePrefab))
		{
			rejectReason = "missing_prefab";
			return false;
		}

		string token = vehiclePrefab;
		token.ToLower();
		if (token.Contains("helicopter") || token.Contains("airplane") || token.Contains("/air/"))
		{
			rejectReason = "armed_support_vehicle";
			return false;
		}

		if (IsArmedVehicleSupportPrefab(vehiclePrefab))
		{
			rejectReason = "armed_support_vehicle";
			return false;
		}

		if (token.Contains("uaz") || token.Contains("brdm"))
		{
			rejectReason = "insufficient_capacity";
			return false;
		}

		if (token.Contains("transport") || token.Contains("truck") || token.Contains("ural") || token.Contains("m923") || token.Contains("m113"))
			return true;

		rejectReason = "insufficient_capacity";
		return false;
	}

	protected void NormalizeVehicleTraitFilters()
	{
		if (!IncludedVehicleTraits)
			IncludedVehicleTraits = {};

		if (!ExcludedVehicleTraits)
			ExcludedVehicleTraits = {};

		PruneNoneVehicleTraits(IncludedVehicleTraits);
		PruneNoneVehicleTraits(ExcludedVehicleTraits);
	}

	protected void PruneNoneVehicleTraits(notnull array<EEditableEntityLabel> traits)
	{
		for (int i = traits.Count() - 1; i >= 0; i--)
		{
			if (traits[i] == EEditableEntityLabel.NONE)
				traits.Remove(i);
		}
	}

	protected bool MatchesVehicleTraitFilters(SCR_EntityCatalogEntry entry)
	{
		if (!entry || MatchesExcludedVehicleTrait(entry))
			return false;

		if (IncludedVehicleTraits && !IncludedVehicleTraits.IsEmpty())
		{
			if (RequireAllIncludedVehicleTraits)
				return MatchesAllConfiguredVehicleTraits(entry, IncludedVehicleTraits);

			return MatchesAnyConfiguredVehicleTrait(entry, IncludedVehicleTraits);
		}

		return HasCatalogLabel(entry, "TRAIT_ARMED") || HasCatalogLabel(entry, "TRAIT_ARMORED");
	}

	protected bool MatchesExcludedVehicleTrait(SCR_EntityCatalogEntry entry)
	{
		if (!entry)
			return true;

		if (HasCatalogLabel(entry, "VEHICLE_HELICOPTER") || HasCatalogLabel(entry, "VEHICLE_AIRPLANE"))
			return true;

		if (ExcludedVehicleTraits && !ExcludedVehicleTraits.IsEmpty())
			return MatchesAnyConfiguredVehicleTrait(entry, ExcludedVehicleTraits);

		return false;
	}

	protected bool MatchesAnyConfiguredVehicleTrait(SCR_EntityCatalogEntry entry, notnull array<EEditableEntityLabel> traits)
	{
		foreach (EEditableEntityLabel label : traits)
		{
			string labelName = SCR_Enum.GetEnumName(EEditableEntityLabel, label);
			if (labelName != "" && labelName != "NONE" && HasCatalogLabel(entry, labelName))
				return true;
		}

		return false;
	}

	protected bool MatchesAllConfiguredVehicleTraits(SCR_EntityCatalogEntry entry, notnull array<EEditableEntityLabel> traits)
	{
		bool hasRequiredTrait = false;
		foreach (EEditableEntityLabel label : traits)
		{
			string labelName = SCR_Enum.GetEnumName(EEditableEntityLabel, label);
			if (labelName == "" || labelName == "NONE")
				continue;

			hasRequiredTrait = true;
			if (!HasCatalogLabel(entry, labelName))
				return false;
		}

		return hasRequiredTrait;
	}

	protected bool IsSpecificVehiclePrefabAllowed(ResourceName vehiclePrefab)
	{
		string token = vehiclePrefab;
		token.ToLower();
		if (token.Contains("helicopter") || token.Contains("airplane") || token.Contains("/air/"))
			return false;

		SCR_EntityCatalogEntry entry = FindVehicleCatalogEntryByPrefab(vehiclePrefab);
		if (!entry)
			return true;

		if (!MatchesExcludedVehicleTrait(entry))
			return true;

		return false;
	}

	protected SCR_EntityCatalogEntry FindVehicleCatalogEntryByPrefab(ResourceName vehiclePrefab)
	{
		SCR_EntityCatalog catalog = ResolveFactionCatalog(EEntityCatalogType.VEHICLE);
		if (!catalog)
			return null;

		array<SCR_EntityCatalogEntry> entries = {};
		catalog.GetEntityList(entries);
		foreach (SCR_EntityCatalogEntry entry : entries)
		{
			if (entry && entry.GetPrefab() == vehiclePrefab)
				return entry;
		}

		return null;
	}

	protected bool HasCatalogLabel(SCR_EntityCatalogEntry entry, string labelName)
	{
		if (!entry || labelName == "")
			return false;

		ref array<EEditableEntityLabel> labels = {};
		entry.GetEditableEntityLabels(labels);
		foreach (EEditableEntityLabel label : labels)
		{
			string candidateName = SCR_Enum.GetEnumName(EEditableEntityLabel, label);
			if (candidateName == labelName)
				return true;
		}

		return false;
	}

	protected SCR_EntityCatalog ResolveFactionCatalog(EEntityCatalogType catalogType)
	{
		SCR_EntityCatalogManagerComponent catalogManager = null;
		if (GetGame() && GetGame().GetGameMode())
			catalogManager = SCR_EntityCatalogManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_EntityCatalogManagerComponent));

		if (catalogManager && m_sFactionKey != "")
		{
			SCR_EntityCatalog factionCatalog = catalogManager.GetFactionEntityCatalogOfType(catalogType, m_sFactionKey, false);
			if (factionCatalog)
				return factionCatalog;
		}

		return null;
	}

	protected ResourceName ResolveArmedVehicleCrewGroupPrefab()
	{
		string factionKey = m_sFactionKey;
		factionKey.ToUpper();

		ResourceName prefab = DEFAULT_USSR_ARMED_SUPPORT_GROUP_PREFAB;
		if (factionKey == "FIA")
			prefab = DEFAULT_FIA_ARMED_SUPPORT_GROUP_PREFAB;
		else if (factionKey == "US")
			prefab = DEFAULT_US_ARMED_SUPPORT_GROUP_PREFAB;

		if (IsEmptyResource(prefab))
			return ResourceName.Empty;

		if (!JLH_AddonSpawnUtility.CanLoadPrefab(prefab))
			return ResourceName.Empty;

		return prefab;
	}

	protected ResourceName ResolveVehiclePassengerGroupPrefab()
	{
		string factionKey = m_sFactionKey;
		factionKey.ToUpper();

		ResourceName prefab = ResourceName.Empty;
		if (factionKey == "USSR")
			prefab = DEFAULT_USSR_VEHICLE_PASSENGER_GROUP_PREFAB;
		else if (factionKey == "FIA")
			prefab = DEFAULT_FIA_VEHICLE_PASSENGER_GROUP_PREFAB;
		else if (factionKey == "US")
			prefab = DEFAULT_US_VEHICLE_PASSENGER_GROUP_PREFAB;

		if (IsEmptyResource(prefab))
			return ResourceName.Empty;

		if (!JLH_AddonSpawnUtility.CanLoadPrefab(prefab))
			return ResourceName.Empty;

		return prefab;
	}

	protected SCR_AIGroup SpawnEmptyVehicleCrewCommandGroup(ResourceName groupPrefab, IEntity owner, vector position)
	{
		Resource resource = Resource.Load(groupPrefab);
		if (!resource || !resource.IsValid() || !owner || !GetGame() || !GetGame().GetWorld())
			return null;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		owner.GetTransform(spawnParams.Transform);
		spawnParams.Transform[3] = position;

		SCR_AIGroup.IgnoreSpawning(true);
		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams));
		SCR_AIGroup.IgnoreSpawning(false);
		if (!group)
			return null;

		group.InitFactionKey(m_sFactionKey);
		//group.DeactivateAllMembers();
		JLH_DCF_PersistenceExclusion.StopTrackingTree(group, "recap_empty_vehicle_crew_group_spawn");
		return group;
	}

	protected SCR_AIGroup SpawnVehiclePassengerGroupAt(ResourceName groupPrefab, IEntity owner, vector position)
	{
		Resource resource = Resource.Load(groupPrefab);
		if (!resource || !resource.IsValid() || !owner || !GetGame() || !GetGame().GetWorld())
			return null;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		owner.GetTransform(spawnParams.Transform);
		spawnParams.Transform[3] = position;

		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams));
		if (!group)
			return null;

		if (!group.GetSpawnImmediately())
			group.SpawnUnits();

		//group.DeactivateAllMembers();
		JLH_DCF_PersistenceExclusion.StopTrackingTree(group, "recap_vehicle_passenger_group_spawn");
		return group;
	}

	protected SCR_AIGroup SpawnGroupPrefab(ResourceName groupPrefab, IEntity owner)
	{
		Resource resource = Resource.Load(groupPrefab);
		if (!resource || !resource.IsValid() || !owner || !GetGame() || !GetGame().GetWorld())
			return null;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		owner.GetTransform(spawnParams.Transform);

		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams));
		if (!group)
			return null;

		if (!group.GetSpawnImmediately())
			group.SpawnUnits();

		ActivateGroupAI(group);
		JLH_DCF_PersistenceExclusion.StopTrackingTree(group, "recap_group_spawn");
		return group;
	}

	protected IEntity SpawnVehicleAt(ResourceName vehiclePrefab, IEntity owner, vector position)
	{
		Resource resource = Resource.Load(vehiclePrefab);
		if (!resource || !resource.IsValid() || !owner || !GetGame() || !GetGame().GetWorld())
			return null;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		owner.GetTransform(spawnParams.Transform);
		spawnParams.Transform[3] = position;
		IEntity vehicle = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams);
		JLH_DCF_PersistenceExclusion.StopTrackingTree(vehicle, "recap_vehicle_spawn");
		return vehicle;
	}

	protected IEntity SpawnVehicleAtGhostPackage(ResourceName vehiclePrefab, IEntity owner, vector position, JLH_DCF_GhostVehicleDefencePackage vehiclePackage)
	{
		Resource resource = Resource.Load(vehiclePrefab);
		if (!resource || !resource.IsValid() || !owner || !GetGame() || !GetGame().GetWorld())
			return null;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		owner.GetTransform(spawnParams.Transform);
		if (vehiclePackage && vehiclePackage.HasOrientation)
		{
			spawnParams.Transform[0] = vehiclePackage.Transform0;
			spawnParams.Transform[1] = vehiclePackage.Transform1;
			spawnParams.Transform[2] = vehiclePackage.Transform2;
		}

		spawnParams.Transform[3] = position;
		IEntity vehicle = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams);
		JLH_DCF_PersistenceExclusion.StopTrackingTree(vehicle, "recap_ghost_vehicle_package_spawn");
		return vehicle;
	}

	protected vector ResolveVehicleSpawnPosition(IEntity owner, int vehicleIndex)
	{
		if (!owner)
			return "0 0 0";

		return GroundPosition(owner.GetOrigin());
	}

	protected void AppendVectorArray(notnull array<vector> targetArray, array<vector> sourceArray)
	{
		if (!sourceArray)
			return;

		foreach (vector position : sourceArray)
		{
			targetArray.Insert(position);
		}
	}

	protected vector ResolveVehicleHoldPosition(vector capTarget, int vehicleIndex, out string spacingResult)
	{
		int directionOffset = Math.RandomInt(0, 8);
		spacingResult = "ok";
		for (int ring = 0; ring < 4; ring++)
		{
			float radius = VEHICLE_HOLD_RADIUS_METERS + (ring * VEHICLE_HOLD_RING_STEP_METERS);
			for (int step = 0; step < 8; step++)
			{
				int directionIndex = directionOffset + vehicleIndex + (step * 3) + ring;
				vector candidate = GroundPosition(capTarget + (GetVehicleHoldDirection(directionIndex) * radius));
				string rejectionReason;
				if (!ValidateDelayedVehicleSpawnPosition(candidate, rejectionReason))
				{
					LogDebug("Vehicle hold rejected reason=" + rejectionReason + " pos=" + candidate.ToString());
					continue;
				}

				if (IsVehicleHoldPositionClear(candidate, capTarget, rejectionReason))
				{
					m_aAttackVehicleHoldPositions.Insert(candidate);
					JLH_DCF_AssaultVehicleHoldRegistry.Reserve(candidate);
					spacingResult = string.Format("ok randomOffset=%1", directionOffset);
					return candidate;
				}

				LogDebug("Vehicle hold rejected reason=" + rejectionReason + " pos=" + candidate.ToString());
			}
		}

		vector fallback = GroundPosition(capTarget + (GetVehicleHoldDirection(directionOffset + vehicleIndex) * (VEHICLE_HOLD_RADIUS_METERS + VEHICLE_HOLD_RING_STEP_METERS)));
		m_aAttackVehicleHoldPositions.Insert(fallback);
		JLH_DCF_AssaultVehicleHoldRegistry.Reserve(fallback);
		spacingResult = string.Format("fallback_short randomOffset=%1", directionOffset);
		return fallback;
	}

	protected vector ResolveTransportVehicleHoldPosition(vector capTarget, vector spawnPosition, int vehicleIndex)
	{
		vector direction = BuildTransportTargetDirection(capTarget, spawnPosition, vehicleIndex);
		vector dropPosition = GroundPosition(capTarget + (direction * TRANSPORT_ARRIVAL_RADIUS));
		m_aAttackVehicleHoldPositions.Insert(dropPosition);
		return dropPosition;
	}

	protected vector BuildTransportTargetDirection(vector capTarget, vector spawnPosition, int vehicleIndex)
	{
		vector flatTarget = capTarget;
		vector flatSpawn = spawnPosition;
		flatTarget[1] = 0;
		flatSpawn[1] = 0;

		float distance = vector.Distance(flatSpawn, flatTarget);
		if (distance <= 1.0)
			return GetVehicleHoldDirection(vehicleIndex);

		vector direction = (flatSpawn - flatTarget) * (1.0 / distance);
		direction[1] = 0;
		return direction;
	}

	protected vector GroundPosition(vector position)
	{
		if (GetGame() && GetGame().GetWorld())
			position[1] = GetGame().GetWorld().GetSurfaceY(position[0], position[2]);

		return position;
	}

	protected vector GetVehicleHoldDirection(int index)
	{
		int direction = index % 8;
		if (direction < 0)
			direction = 0;

		if (direction == 0) return "1 0 0";
		if (direction == 1) return "0.707 0 0.707";
		if (direction == 2) return "0 0 1";
		if (direction == 3) return "-0.707 0 0.707";
		if (direction == 4) return "-1 0 0";
		if (direction == 5) return "-0.707 0 -0.707";
		if (direction == 6) return "0 0 -1";
		return "0.707 0 -0.707";
	}

	protected bool IsVehicleHoldPositionClear(vector candidate, vector capTarget, out string reason)
	{
		reason = "";
		if (vector.Distance(candidate, capTarget) < VEHICLE_HOLD_MIN_CAP_DISTANCE_METERS)
		{
			reason = "cap_center";
			return false;
		}

		foreach (vector plannedHold : m_aAttackVehicleHoldPositions)
		{
			if (vector.Distance(candidate, plannedHold) < VEHICLE_HOLD_SPACING_METERS)
			{
				reason = "planned_vehicle_hold";
				return false;
			}
		}

		if (JLH_DCF_AssaultVehicleHoldRegistry.IsReservedNear(candidate, VEHICLE_HOLD_SPACING_METERS))
		{
			reason = "reserved_vehicle_hold";
			return false;
		}

		foreach (JLH_DCF_RecapVehicleRuntime runtime : m_aActiveVehicles)
		{
			if (!runtime)
				continue;

			if (vector.Distance(candidate, runtime.TargetPosition) < VEHICLE_HOLD_SPACING_METERS)
			{
				reason = "active_vehicle_hold";
				return false;
			}

			if (runtime.VehicleEntity && vector.Distance(candidate, runtime.VehicleEntity.GetOrigin()) < VEHICLE_HOLD_SPACING_METERS)
			{
				reason = "active_vehicle";
				return false;
			}
		}

		foreach (IEntity retainedVehicle : m_aRetainedDefenderVehicles)
		{
			if (retainedVehicle && vector.Distance(candidate, retainedVehicle.GetOrigin()) < VEHICLE_HOLD_SPACING_METERS)
			{
				reason = "retained_vehicle";
				return false;
			}
		}

		reason = "ok";
		return true;
	}

	protected void LogVehicleHoldSelected(int vehicleIndex, vector position, string spacingResult)
	{
		string logLine = "Vehicle hold selected index=" + string.Format("%1", vehicleIndex + 1);
		string displaySpacing = spacingResult;
		logLine += " pos=" + position.ToString();
		logLine += " spacing=" + displaySpacing;
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, logLine, true);
	}

	protected int GetVehicleSpawnDelayMs()
	{
		int delayMs = SecondsToMs(VehicleSpawnDelaySeconds);
		int jitterMs = SecondsToMs(VehicleSpawnDelayJitter);
		if (jitterMs > 0)
			delayMs = delayMs + Math.RandomInt(0, jitterMs + 1);

		if (delayMs < 0)
			delayMs = 0;

		return delayMs;
	}

	protected void RecordAttackInfantrySpawnPosition(vector position)
	{
		m_aAttackInfantrySpawnPositions.Insert(position);
	}

	protected bool ValidateDelayedVehicleSpawnPosition(vector position, out string reason)
	{
		reason = "";
		if (!GetGame() || !GetGame().GetWorld())
			return true;

		BaseWorld world = GetGame().GetWorld();
		float center = world.GetSurfaceY(position[0], position[2]);
		if (center <= 0.25)
		{
			reason = "water";
			return false;
		}

		float sample = 4.0;
		float maxDelta = 2.5;
		float east = world.GetSurfaceY(position[0] + sample, position[2]);
		float west = world.GetSurfaceY(position[0] - sample, position[2]);
		float north = world.GetSurfaceY(position[0], position[2] + sample);
		float south = world.GetSurfaceY(position[0], position[2] - sample);
		if (AbsFloat(east - center) > maxDelta || AbsFloat(west - center) > maxDelta || AbsFloat(north - center) > maxDelta || AbsFloat(south - center) > maxDelta)
		{
			reason = "invalid_ground";
			return false;
		}

		reason = "ok";
		return true;
	}

	protected float AbsFloat(float value)
	{
		if (value < 0.0)
			return -value;

		return value;
	}

	protected bool IsVehicleSpawnVisibilitySafe(vector position, out string reason)
	{
		reason = "distance_clear";
		if (!EnableSpawnVisibilitySafety)
			return true;

		float safetyDistance = MinimumVehicleSpawnSafetyDistance;
		if (safetyDistance <= 0.0)
			return true;

		ref array<IEntity> players = {};
		GatherSpawnSafetyPlayers(players);
		bool closePlayerFound = false;

		foreach (IEntity controlled : players)
		{
			if (!IsValidPlayer(controlled))
				continue;

			if (vector.Distance(position, controlled.GetOrigin()) >= safetyDistance)
				continue;

			closePlayerFound = true;
			if (!RequireLOSToBlockCloseSpawn || HasDirectLineOfSight(controlled, position))
			{
				reason = "visible_to_player";
				return false;
			}
		}

		if (closePlayerFound)
			reason = "close_not_visible";

		return true;
	}

	protected bool AssignAssaultAndDefendWaypoints(SCR_AIGroup group, vector target, notnull array<IEntity> routeWaypoints)
	{
		routeWaypoints.Clear();
		if (!group)
			return false;

		JLH_DCF_NodeWaypointResolver.ClearGroupWaypoints(group);
		bool assigned = false;
		AIWaypoint moveWaypoint = SpawnWaypoint(ASSAULT_MOVE_WAYPOINT_PREFAB, target);
		if (moveWaypoint)
		{
			group.AddWaypoint(moveWaypoint);
			routeWaypoints.Insert(moveWaypoint);
			assigned = true;
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Assault move assigned target=%1", target.ToString()), true);
		}

		AIWaypoint defendWaypoint = SpawnDefendWaypoint(target);
		if (defendWaypoint)
		{
			group.AddWaypoint(defendWaypoint);
			routeWaypoints.Insert(defendWaypoint);
			assigned = true;
		}

		if (!assigned)
			return false;

		SetGroupAggressiveBehavior(group);
		return true;
	}

	protected bool AssignVehicleMoveWaypointToGroup(SCR_AIGroup group, vector target, notnull array<IEntity> routeWaypoints)
	{
		routeWaypoints.Clear();
		if (!group)
			return false;

		JLH_DCF_NodeWaypointResolver.ClearGroupWaypoints(group);
		AIWaypoint moveWaypoint = SpawnWaypoint(ASSAULT_MOVE_WAYPOINT_PREFAB, target);
		if (!moveWaypoint)
			return false;

		group.AddWaypoint(moveWaypoint);
		routeWaypoints.Insert(moveWaypoint);
		bool aggressiveResult = SetGroupAggressiveBehavior(group);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Group behavior set aggressive result=%1 group=%2", JLH_DCF_NodeDebug.BoolLabel(aggressiveResult), JLH_DCF_NodeDebug.EntityLabel(group)), true);
		return true;
	}

	protected bool AssignMountedVehicleHoldWaypointToGroup(SCR_AIGroup group, vector target, notnull array<IEntity> routeWaypoints)
	{
		routeWaypoints.Clear();
		if (!group)
			return false;

		JLH_DCF_NodeWaypointResolver.ClearGroupWaypoints(group);
		AIWaypoint moveWaypoint = SpawnWaypoint(ASSAULT_MOVE_WAYPOINT_PREFAB, target);
		if (!moveWaypoint)
			return false;

		group.AddWaypoint(moveWaypoint);
		routeWaypoints.Insert(moveWaypoint);
		if (!AddMountedVehicleCycleWaypoint(group, moveWaypoint, routeWaypoints))
		{
			CleanupRouteWaypoints(group, routeWaypoints);
			return false;
		}

		bool aggressiveResult = SetGroupAggressiveBehavior(group);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("mounted vehicle hold assigned target=%1 cycle=true aggressive=%2 group=%3", target.ToString(), JLH_DCF_NodeDebug.BoolLabel(aggressiveResult), JLH_DCF_NodeDebug.EntityLabel(group)), true);
		return true;
	}

	protected bool AddMountedVehicleCycleWaypoint(SCR_AIGroup group, AIWaypoint moveWaypoint, notnull array<IEntity> routeWaypoints)
	{
		if (!group || !moveWaypoint || !GetGame() || !GetGame().GetWorld())
			return false;

		Resource cycleResource = Resource.Load(CYCLE_WAYPOINT_PREFAB);
		if (!cycleResource || !cycleResource.IsValid())
			return false;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		moveWaypoint.GetTransform(spawnParams.Transform);

		IEntity cycleEntity = GetGame().SpawnEntityPrefab(cycleResource, GetGame().GetWorld(), spawnParams);
		if (!cycleEntity)
			return false;

		JLH_DCF_PersistenceExclusion.StopTrackingTree(cycleEntity, "recap_mounted_vehicle_cycle_waypoint");

		AIWaypointCycle cycleWaypoint = AIWaypointCycle.Cast(cycleEntity);
		if (!cycleWaypoint)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(cycleEntity);
			return false;
		}

		array<AIWaypoint> loopWaypoints = {};
		loopWaypoints.Insert(moveWaypoint);
		cycleWaypoint.SetRerunCounter(-1);
		cycleWaypoint.SetWaypoints(loopWaypoints);
		group.AddWaypoint(cycleWaypoint);
		routeWaypoints.Insert(cycleEntity);
		return true;
	}

	protected bool AssignDefendOnlyWaypoint(SCR_AIGroup group, vector target, notnull array<IEntity> routeWaypoints)
	{
		routeWaypoints.Clear();
		if (!group)
			return false;

		JLH_DCF_NodeWaypointResolver.ClearGroupWaypoints(group);
		AIWaypoint defendWaypoint = SpawnDefendWaypoint(target);
		if (!defendWaypoint)
			return false;

		group.AddWaypoint(defendWaypoint);
		routeWaypoints.Insert(defendWaypoint);
		SetGroupAggressiveBehavior(group);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Defend handoff assigned target=%1", target.ToString()), true);
		return true;
	}

	protected AIWaypoint SpawnDefendWaypoint(vector target)
	{
		AIWaypoint waypoint = SpawnWaypoint(DEFEND_CP_WAYPOINT_PREFAB, target);
		if (waypoint)
			return waypoint;

		return SpawnWaypoint(DEFEND_WAYPOINT_PREFAB, target);
	}

	protected AIWaypoint SpawnWaypoint(ResourceName waypointPrefab, vector position)
	{
		if (waypointPrefab == "" || !GetGame() || !GetGame().GetWorld())
			return null;

		Resource waypointResource = Resource.Load(waypointPrefab);
		if (!waypointResource || !waypointResource.IsValid())
			return null;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[3] = position;

		IEntity waypointEntity = GetGame().SpawnEntityPrefab(waypointResource, GetGame().GetWorld(), spawnParams);
		AIWaypoint waypoint = AIWaypoint.Cast(waypointEntity);
		if (!waypoint)
		{
			if (waypointEntity)
				SCR_EntityHelper.DeleteEntityAndChildren(waypointEntity);

			return null;
		}

		JLH_DCF_PersistenceExclusion.StopTrackingTree(waypointEntity, "recap_waypoint_spawn");
		return waypoint;
	}

	protected void TrackAttackGroup(SCR_AIGroup group)
	{
		if (!group)
			return;

		if (!m_aAttackGroups.Contains(group))
			m_aAttackGroups.Insert(group);

		if (m_RuntimeTracker)
			m_RuntimeTracker.TrackGroup(group);

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (unit && !m_aAttackUnits.Contains(unit))
				m_aAttackUnits.Insert(unit);
		}
	}

	protected int CountLivingAttackUnits()
	{
		int living = 0;
		for (int i = m_aAttackUnits.Count() - 1; i >= 0; i--)
		{
			IEntity unit = m_aAttackUnits[i];
			if (!unit)
			{
				m_aAttackUnits.Remove(i);
				continue;
			}

			if (IsUnitAlive(unit))
				living++;
		}

		return living;
	}

	protected void RefreshAttackUnitsFromGroups()
	{
		for (int i = m_aAttackGroups.Count() - 1; i >= 0; i--)
		{
			SCR_AIGroup group = m_aAttackGroups[i];
			if (!group)
			{
				m_aAttackGroups.Remove(i);
				continue;
			}

			array<AIAgent> agents = {};
			group.GetAgents(agents);
			foreach (AIAgent agent : agents)
			{
				if (!agent)
					continue;

				IEntity unit = agent.GetControlledEntity();
				if (unit && !m_aAttackUnits.Contains(unit))
					m_aAttackUnits.Insert(unit);
			}
		}

		foreach (JLH_DCF_RecapVehicleRuntime runtime : m_aActiveVehicles)
		{
			CollectVehicleCrewUnits(runtime);
			TrackVehicleCrewUnits(runtime);
		}
	}

	protected int CountActiveVehicleAssaultBlockers()
	{
		int blockers = 0;
		foreach (JLH_DCF_RecapVehicleRuntime runtime : m_aActiveVehicles)
		{
			if (!runtime)
				continue;

			if (runtime.MountPending)
			{
				blockers++;
				continue;
			}

			string reason;
			if (IsVehicleCombatEffective(runtime, reason))
				blockers++;
		}

		return blockers;
	}

	protected int CountLivingVehicleCrew(JLH_DCF_RecapVehicleRuntime runtime)
	{
		if (!runtime)
			return 0;

		int living = 0;
		foreach (IEntity unit : runtime.CrewUnits)
		{
			if (unit && IsUnitAlive(unit))
				living++;
		}

		return living;
	}

	protected int CountLivingGroupUnits(SCR_AIGroup group)
	{
		if (!group)
			return 0;

		int living = 0;
		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (unit && IsUnitAlive(unit))
				living++;
		}

		return living;
	}

	protected int CountLivingAttackUnitsNearBase()
	{
		vector target = ResolveBaseTargetPosition();
		int count = 0;
		foreach (IEntity unit : m_aAttackUnits)
		{
			if (unit && IsUnitAlive(unit) && vector.Distance(unit.GetOrigin(), target) <= ATTACK_SETTLE_RADIUS)
				count++;
		}

		return count;
	}

	protected int CountActiveVehiclesNearBase()
	{
		vector target = ResolveBaseTargetPosition();
		int count = 0;
		foreach (JLH_DCF_RecapVehicleRuntime runtime : m_aActiveVehicles)
		{
			if (runtime && runtime.VehicleEntity && !IsEntityDestroyed(runtime.VehicleEntity) && vector.Distance(runtime.VehicleEntity.GetOrigin(), target) <= ATTACK_SETTLE_RADIUS)
				count++;
		}

		return count;
	}

	protected void LogAttackStatus(int now, int aliveUnits, int activeVehicles)
	{
		if (m_iLastAttackStatusLogTick > 0 && now - m_iLastAttackStatusLogTick < ATTACK_STATUS_LOG_INTERVAL_MS)
			return;

		m_iLastAttackStatusLogTick = now;
		LogDebug(string.Format("Attack status assaultId=%1 base=%2 aliveUnits=%3 activeVehicles=%4", m_iActiveAssaultId, m_sBaseName, aliveUnits, activeVehicles));
	}

	protected void InsertRuntimeRouteWaypoints(array<IEntity> routeWaypoints)
	{
		if (!routeWaypoints)
			return;

		foreach (IEntity waypointEntity : routeWaypoints)
		{
			if (waypointEntity)
				m_aRuntimeRouteWaypoints.Insert(waypointEntity);
		}
	}

	protected void RemoveRuntimeRouteWaypoints(JLH_DCF_RecapVehicleRuntime runtime)
	{
		if (!runtime || !runtime.RouteWaypoints)
			return;

		foreach (IEntity waypointEntity : runtime.RouteWaypoints)
		{
			for (int i = m_aRuntimeRouteWaypoints.Count() - 1; i >= 0; i--)
			{
				if (m_aRuntimeRouteWaypoints[i] == waypointEntity)
					m_aRuntimeRouteWaypoints.Remove(i);
			}
		}

		CleanupRouteWaypoints(runtime.CrewGroup, runtime.RouteWaypoints);
	}

	protected void CleanupRouteWaypoints(SCR_AIGroup group, array<IEntity> routeWaypoints)
	{
		if (!routeWaypoints)
			return;

		foreach (IEntity waypointEntity : routeWaypoints)
		{
			if (waypointEntity)
				DeleteRuntimeEntitySafe(waypointEntity, "recap_route_waypoint");
		}

		routeWaypoints.Clear();
	}

	protected void ClearActiveAttackTracking(bool deleteRuntime)
	{
		foreach (JLH_DCF_RecapVehicleRuntime runtime : m_aActiveVehicles)
		{
			ReleaseTransportLockForRuntime(runtime, "assault_end");
		}

		if (deleteRuntime)
		{
			foreach (SCR_AIGroup group : m_aAttackGroups)
			{
				if (group)
					DeleteGroupAndMembers(group);
			}

			foreach (IEntity waypointEntity : m_aRuntimeRouteWaypoints)
			{
				if (waypointEntity)
					DeleteRuntimeEntitySafe(waypointEntity, "recap_runtime_route_waypoint");
			}
		}

		m_aAttackGroups.Clear();
		m_aAttackUnits.Clear();
		m_aAttackInfantrySpawnPositions.Clear();
		m_aAttackVehicleSpawnPositions.Clear();
		m_aAttackVehicleHoldPositions.Clear();
		m_aRuntimeRouteWaypoints.Clear();
		m_aActiveVehicles.Clear();
		m_bVehicleSpawnPending = false;
		m_bCurrentAssaultPassengerDelivery = false;
		m_iPendingVehicleCount = 0;
		m_iPendingVehicleReadyTick = 0;
		m_vPendingVehicleTarget = "0 0 0";
		m_PendingVehicleOwner = null;
	}

	protected string BuildTransportLockToken(int vehicleIndex)
	{
		return "RECAP:" + m_sBaseName + ":" + JLH_DCF_NodeDebug.EntityLabel(GetOwner()) + ":" + string.Format("%1", m_iActiveAssaultId) + ":" + string.Format("%1", vehicleIndex + 1) + ":" + string.Format("%1", System.GetTickCount());
	}

	protected void ReleaseTransportLockForRuntime(JLH_DCF_RecapVehicleRuntime runtime, string reason)
	{
		if (!runtime || runtime.TransportBaseKey == "" || runtime.TransportLockToken == "")
			return;

		ReleaseTransportLock(runtime.TransportBaseKey, runtime.TransportLockToken, reason);
		runtime.TransportBaseKey = "";
		runtime.TransportLockToken = "";
	}

	protected void ReleaseTransportLock(string baseKey, string token, string reason)
	{
		if (JLH_DCF_BaseTransportLock.Release(baseKey, token))
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport lock released reason=%1 base=%2", NormalizeTransportLockReleaseReason(reason), baseKey), true);
	}

	protected string NormalizeTransportLockReleaseReason(string reason)
	{
		if (reason == "vehicle_destroyed")
			return "destroyed";

		if (reason == "crew_dead")
			return "group_dead";

		if (reason == "attack_complete" || reason == "assault_end" || reason == "timeout")
			return "assault_end";

		return reason;
	}

	protected void ClearRetainedDefenderTracking()
	{
		m_aRetainedDefenderGroups.Clear();
		m_aRetainedDefenderUnits.Clear();
		m_aRetainedDefenderVehicles.Clear();
	}

	protected bool IsBaseContested()
	{
		SCR_CampaignSeizingComponent seizingComponent = FindSeizingComponent();
		if (!seizingComponent)
			return false;

		string prevailingFaction = seizingComponent.JLH_DCF_GetPrevailingFactionKey();
		float progress = seizingComponent.JLH_DCF_GetSeizingProgressNormalized();
		return prevailingFaction != "NONE" && prevailingFaction != "" && progress > 0.0 && progress < 1.0;
	}

	SCR_CampaignSeizingComponent FindSeizingComponent()
	{
		if (m_BaseEntity)
		{
			SCR_CampaignSeizingComponent seizingComponent = SCR_CampaignSeizingComponent.Cast(m_BaseEntity.FindComponent(SCR_CampaignSeizingComponent));
			if (seizingComponent)
				return seizingComponent;
		}

		if (m_BaseComponent)
		{
			IEntity owner = m_BaseComponent.GetOwner();
			if (owner && owner != m_BaseEntity)
				return SCR_CampaignSeizingComponent.Cast(owner.FindComponent(SCR_CampaignSeizingComponent));
		}

		return null;
	}

	protected vector ResolveBaseTargetPosition()
	{
		if (m_BaseEntity)
			return m_BaseEntity.GetOrigin();

		if (m_BaseComponent && m_BaseComponent.GetOwner())
			return m_BaseComponent.GetOwner().GetOrigin();

		if (GetOwner())
			return GetOwner().GetOrigin();

		return "0 0 0";
	}

	protected JLH_RecapNodeComponent ResolveSelectedSpawnNode()
	{
		if (m_SelectedSpawnNode && m_SelectedSpawnNode.GetOwner())
			return m_SelectedSpawnNode;

		return this;
	}

	protected IEntity GetSelectedSpawnOwner()
	{
		JLH_RecapNodeComponent node = ResolveSelectedSpawnNode();
		if (node && node.GetOwner())
			return node.GetOwner();

		return GetOwner();
	}

	protected bool IsEntityDestroyed(IEntity entity)
	{
		if (!entity)
			return true;

		DamageManagerComponent damageManager = DamageManagerComponent.Cast(entity.FindComponent(DamageManagerComponent));
		if (!damageManager)
			return false;

		return damageManager.GetHealthScaled() <= 0.0;
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

	protected bool VehiclePrefabLikelyRequiresGunner(ResourceName vehiclePrefab)
	{
		string token = vehiclePrefab;
		token.ToLower();
		if (token.Contains("unarmed")) return false;
		if (token.Contains("armed")) return true;
		if (token.Contains("turret")) return true;
		if (token.Contains("pkm")) return true;
		if (token.Contains("dshk")) return true;
		if (token.Contains("m2hb")) return true;
		if (token.Contains("hmg")) return true;
		if (token.Contains("ags")) return true;
		if (token.Contains("spg")) return true;
		if (token.Contains("kpvt")) return true;
		if (token.Contains("btr")) return true;
		if (token.Contains("brdm")) return true;
		if (token.Contains("bmp")) return true;
		if (token.Contains("apc")) return true;
		return false;
	}

	protected bool VehiclePrefabHasArmedCatalogTrait(ResourceName vehiclePrefab)
	{
		SCR_EntityCatalogEntry entry = FindVehicleCatalogEntryByPrefab(vehiclePrefab);
		if (!entry)
			return false;

		return HasCatalogLabel(entry, "TRAIT_ARMED") || HasCatalogLabel(entry, "TRAIT_ARMORED");
	}

	protected bool IsArmedVehicleSupportPrefab(ResourceName vehiclePrefab)
	{
		if (VehiclePrefabHasArmedCatalogTrait(vehiclePrefab))
			return true;

		return VehiclePrefabLikelyRequiresGunner(vehiclePrefab);
	}

	protected bool IsArmedVehicleMount(ResourceName vehiclePrefab, JLH_DCF_VehicleMountStats stats)
	{
		if (stats && (stats.GunnerMounted || stats.TurretSeatsFilled > 0))
			return true;

		return IsArmedVehicleSupportPrefab(vehiclePrefab);
	}

	protected bool RegisterUsableVehicle(SCR_AIGroup group, IEntity vehicle)
	{
		if (!group || !vehicle)
			return false;

		SCR_AIGroupUtilityComponent utility = group.GetGroupUtilityComponent();
		if (!utility)
			return false;

		IEntity usageOwner = null;
		SCR_AIVehicleUsageComponent vehicleUsage = SCR_AIVehicleUsageComponent.FindOnNearestParent(vehicle, usageOwner);
		if (!vehicleUsage || !vehicleUsage.CanBePiloted())
			return false;

		utility.SetMaxAutonomousDistance(4500.0);
		utility.AddUsableVehicle(vehicleUsage);
		return true;
	}

	protected bool SetGroupAggressiveBehavior(SCR_AIGroup group)
	{
		if (!group)
			return false;

		group.ActivateAI();
		//group.ActivateAllMembers();
		AIControlComponent control = group.GetControlComponent();
		if (control)
			control.ActivateAI();

		return true;
	}

	protected void ActivateGroupAI(SCR_AIGroup group)
	{
		SetGroupAggressiveBehavior(group);
	}

	protected bool IsUnitAlive(IEntity unit)
	{
		if (!unit)
			return false;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(unit);
		if (!character)
			return true;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return true;

		return !controller.IsDead();
	}

	protected bool IsValidPlayer(IEntity entity)
	{
		return JLH_DCF_PlayerPresence.IsLiveHumanControlledCharacterForFaction(entity, PLAYER_FACTION_KEY);
	}

	protected void GatherSpawnSafetyPlayers(notnull array<IEntity> players)
	{
		JLH_AddonWorldQuery.GatherLiveDeployedPlayers(players, PLAYER_FACTION_KEY);
	}

	protected bool IsRecapSuccessOutcomeConfirmed()
	{
		if (!m_BaseComponent || m_sFactionKey == "")
			return false;

		return JLH_DCF_NodeBaseResolver.GetBaseFactionKey(m_BaseComponent) == m_sFactionKey;
	}

	protected bool RequireLiveHumanPlayerForLaunch()
	{
		if (HasValidPlayersOnlineOrDeployed())
			return true;

		if (!s_bNoLiveHumanPlayerLaunchSkipLogged)
		{
			s_bNoLiveHumanPlayerLaunchSkipLogged = true;
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "launch skipped reason=no_live_human_player", true);
		}

		return false;
	}

	protected bool HasLiveHumanPlayerInWorld()
	{
		return HasValidPlayersOnlineOrDeployed();
	}

	protected bool HasValidPlayersOnlineOrDeployed()
	{
		if (JLH_DCF_PlayerPresence.HasLiveHumanInWorld())
		{
			s_bNoLiveHumanPlayerLaunchSkipLogged = false;
			return true;
		}

		return false;
	}

	protected bool IsLiveHumanPlayerEntity(IEntity entity)
	{
		return JLH_DCF_PlayerPresence.IsLiveHumanControlledCharacter(entity);
	}

	protected PlayerManager GetPlayerManagerSafe()
	{
		if (!GetGame())
			return null;

		return GetGame().GetPlayerManager();
	}

	protected bool IsAnyValidPlayerWithinPosition(vector position, float distanceLimit)
	{
		ref array<IEntity> players = {};
		GatherSpawnSafetyPlayers(players);
		return IsAnyValidPlayerWithinPositionWithPlayers(position, distanceLimit, players);
	}

	protected bool IsAnyValidPlayerWithinPositionWithPlayers(vector position, float distanceLimit, notnull array<IEntity> players)
	{
		foreach (IEntity controlled : players)
		{
			if (IsValidPlayer(controlled) && vector.Distance(position, controlled.GetOrigin()) <= distanceLimit)
				return true;
		}

		return false;
	}

	protected bool HasAnyValidPlayerLineOfSight(vector position, float distanceLimit)
	{
		ref array<IEntity> players = {};
		GatherSpawnSafetyPlayers(players);
		return HasAnyValidPlayerLineOfSightWithPlayers(position, distanceLimit, players);
	}

	protected bool HasAnyValidPlayerLineOfSightWithPlayers(vector position, float distanceLimit, notnull array<IEntity> players)
	{
		foreach (IEntity controlled : players)
		{
			if (!IsValidPlayer(controlled))
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

	protected void LogGroupPrefabSelected(string mode, int slot, ResourceName prefab)
	{
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Group prefab selected mode=%1 slot=%2 prefab=%3", mode, slot, prefab), true);
	}

	protected void LogLaunchRejected(string reason)
	{
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Launch rejected reason=%1", reason), true);
	}

	protected string GetPresetLabel(JLH_DCF_RecapForcePreset preset)
	{
		if (preset == JLH_DCF_RecapForcePreset.CUSTOM_MANUAL)
			return "CUSTOM_MANUAL";
		if (preset == JLH_DCF_RecapForcePreset.LIGHT_PROBE)
			return "LIGHT_PROBE";
		if (preset == JLH_DCF_RecapForcePreset.STANDARD_ASSAULT)
			return "STANDARD_ASSAULT";
		if (preset == JLH_DCF_RecapForcePreset.HEAVY_ASSAULT)
			return "HEAVY_ASSAULT";
		if (preset == JLH_DCF_RecapForcePreset.TRANSPORT_ASSAULT)
			return "TRANSPORT_ASSAULT";
		if (preset == JLH_DCF_RecapForcePreset.RANDOM)
			return "RANDOM";

		return "STANDARD_ASSAULT";
	}

	void CleanupOwnedRuntime(string reason)
	{
		ReleaseRetainedTransportLocks(reason);

		if (m_RuntimeTracker)
			m_RuntimeTracker.CleanupOwnedRuntime(SYSTEM_NAME, reason, DebugLogging);

		ClearActiveAttackTracking(true);
		ClearRetainedDefenderTracking();
		m_aVehiclesAwaitingCleanup.Clear();
		if (m_bAttackActive)
			JLH_DCF_RecapPressureDirector.GetInstance().NotifyRecapEnded(this, JLH_DCF_RecapResult.CANCELLED);

		m_bAttackActive = false;
		ClearRecapTimer();
		SuppressInheritedAmbientState(false);
	}

	bool IsRuntimeEntityCleanupProtected(IEntity entity, JLH_DCF_RuntimeCleanupEntityKind kind, out string reason)
	{
		reason = "";
		if (!entity)
			return false;

		if (m_bAttackActive)
		{
			if (m_aAttackUnits.Contains(entity))
			{
				reason = "recap_active_attack";
				return true;
			}

			foreach (SCR_AIGroup group : m_aAttackGroups)
			{
				if (group == entity || IsGroupMember(group, entity))
				{
					reason = "recap_active_attack";
					return true;
				}
			}

			foreach (JLH_DCF_RecapVehicleRuntime activeRuntime : m_aActiveVehicles)
			{
				if (IsVehicleRuntimeEntity(activeRuntime, entity))
				{
					reason = "recap_active_vehicle";
					return true;
				}
			}
		}

		foreach (JLH_DCF_RecapVehicleRuntime cleanupRuntime : m_aVehiclesAwaitingCleanup)
		{
			if (IsVehicleRuntimeEntity(cleanupRuntime, entity))
			{
				reason = "recap_local_vehicle_cleanup";
				return true;
			}
		}

		foreach (JLH_DCF_RecapVehicleRuntime retainedRuntime : m_aRetainedTransportRuntimes)
		{
			if (IsVehicleRuntimeEntity(retainedRuntime, entity))
			{
				reason = "recap_retained_transport_defence";
				return true;
			}
		}

		if (m_aRuntimeRouteWaypoints.Contains(entity))
		{
			reason = "recap_route_waypoint";
			return true;
		}

		return false;
	}

	protected bool IsVehicleRuntimeEntity(JLH_DCF_RecapVehicleRuntime runtime, IEntity entity)
	{
		if (!runtime || !entity)
			return false;

		if (runtime.VehicleEntity == entity || runtime.CrewGroup == entity)
			return true;

		if (runtime.CrewUnits && runtime.CrewUnits.Contains(entity))
			return true;

		if (runtime.RouteWaypoints && runtime.RouteWaypoints.Contains(entity))
			return true;

		return IsGroupMember(runtime.CrewGroup, entity);
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

	protected void ReleaseRetainedTransportLocks(string reason)
	{
		foreach (JLH_DCF_RecapVehicleRuntime runtime : m_aRetainedTransportRuntimes)
		{
			ReleaseTransportLockForRuntime(runtime, reason);
		}

		m_aRetainedTransportRuntimes.Clear();
	}

	bool JLH_DCF_IsGhostDefenceAnchor(string baseName, string factionKey)
	{
		return IsRegistered() && GetOwner() != null && m_sBaseName == baseName && m_sFactionKey == factionKey;
	}

	vector JLH_DCF_GetGhostDefenceAnchorPosition()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return "0 0 0";

		return GroundPosition(owner.GetOrigin());
	}

	bool JLH_DCF_SpawnGhostDefenceGroup(ResourceName preferredPrefab, vector defendTarget, notnull array<SCR_AIGroup> groups, notnull array<IEntity> units, notnull array<IEntity> waypoints)
	{
		if (!IsRegistered() || !GetOwner())
			return false;

		ResourceName prefab = preferredPrefab;
		if (IsEmptyResource(prefab))
			return false;

		vector spawnPosition;
		string placementSource;
		string skippedReason;
		if (!JLH_DCF_GhostPlacement.ResolveInfantryPosition("0 0 0", false, defendTarget, groups.Count() + 1, m_aAttackInfantrySpawnPositions, GHOST_DEFENCE_INFANTRY_SPACING_METERS, GHOST_DEFENCE_SAFE_SPAWN_DISTANCE, spawnPosition, placementSource, skippedReason))
			return false;

		SCR_AIGroup group = SpawnGroupPrefabAt(prefab, GetOwner(), spawnPosition);
		if (!group)
			return false;

		array<IEntity> routeWaypoints = {};
		if (!AssignDefendOnlyWaypoint(group, defendTarget, routeWaypoints))
		{
			CleanupRouteWaypoints(group, routeWaypoints);
			DeleteGroupAndMembers(group);
			return false;
		}

		ActivateGroupAI(group);
		AddUniqueGroup(group, groups);
		CollectGroupUnitsToArray(group, units);
		foreach (IEntity waypoint : routeWaypoints)
		{
			AddUniqueEntity(waypoint, waypoints);
		}

		RecordAttackInfantrySpawnPosition(spawnPosition);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Ghost defence infantry spawned base=%1 slot=%2 pos=%3", m_sBaseName, groups.Count(), spawnPosition.ToString()), true);
		return true;
	}

	void JLH_DCF_LogVirtualInfantryRejected(int requestedUnits, int packageIndex)
	{
		LogVirtualInfantryRejected(requestedUnits, packageIndex);
	}

	bool JLH_DCF_SpawnGhostDefenceInfantryPackage(JLH_DCF_GhostInfantryDefencePackage infantryPackage, vector defendTarget, int packageIndex, notnull array<SCR_AIGroup> groups, notnull array<IEntity> units, notnull array<IEntity> waypoints)
	{
		if (!IsRegistered() || !GetOwner() || !infantryPackage)
			return false;

		if (infantryPackage.UnitCount <= 0)
		{
			LogVirtualInfantryRejected(infantryPackage.UnitCount, packageIndex);
			return false;
		}

		ResourceName prefab = infantryPackage.GroupPrefab;
		if (IsEmptyResource(prefab))
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Ghost defence infantry skipped base=%1 package=%2 originalPrefab=%3 preferredPosition=%4 placementSource=skipped skippedReason=resource_missing", m_sBaseName, packageIndex, infantryPackage.GroupPrefab, infantryPackage.LastKnownPosition.ToString()), true);
			return false;
		}

		vector spawnPosition;
		string placementSource;
		string skippedReason;
		if (!JLH_DCF_GhostPlacement.ResolveInfantryPosition(infantryPackage.LastKnownPosition, infantryPackage.HasLastKnownPosition, defendTarget, packageIndex, m_aAttackInfantrySpawnPositions, GHOST_DEFENCE_INFANTRY_SPACING_METERS, GHOST_DEFENCE_SAFE_SPAWN_DISTANCE, spawnPosition, placementSource, skippedReason))
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Ghost defence infantry skipped base=%1 package=%2 originalPrefab=%3 preferredPosition=%4 placementSource=skipped skippedReason=%5", m_sBaseName, packageIndex, prefab, infantryPackage.LastKnownPosition.ToString(), skippedReason), true);
			return false;
		}

		SCR_AIGroup group = SpawnGroupPrefabAt(prefab, GetOwner(), spawnPosition);
		if (!group)
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Ghost defence infantry skipped base=%1 package=%2 originalPrefab=%3 preferredPosition=%4 resolvedPosition=%5 placementSource=%6 skippedReason=spawn_failed", m_sBaseName, packageIndex, prefab, infantryPackage.LastKnownPosition.ToString(), spawnPosition.ToString(), placementSource), true);
			return false;
		}

		array<IEntity> retainedUnits = {};
		PruneSpawnedGroupToRequestedUnits(group, infantryPackage.UnitCount, packageIndex, retainedUnits);

		array<IEntity> routeWaypoints = {};
		if (!AssignDefendOnlyWaypoint(group, defendTarget, routeWaypoints))
		{
			CleanupRouteWaypoints(group, routeWaypoints);
			DeleteGroupAndMembers(group);
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Ghost defence infantry skipped base=%1 package=%2 originalPrefab=%3 preferredPosition=%4 resolvedPosition=%5 placementSource=%6 skippedReason=waypoint_failed", m_sBaseName, packageIndex, prefab, infantryPackage.LastKnownPosition.ToString(), spawnPosition.ToString(), placementSource), true);
			return false;
		}

		ActivateGroupAI(group);
		AddUniqueGroup(group, groups);
		foreach (IEntity unit : retainedUnits)
		{
			AddUniqueEntity(unit, units);
		}

		foreach (IEntity waypoint : routeWaypoints)
		{
			AddUniqueEntity(waypoint, waypoints);
		}

		RecordAttackInfantrySpawnPosition(spawnPosition);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Ghost defence infantry spawned base=%1 package=%2 originalPrefab=%3 preferredPosition=%4 resolvedPosition=%5 placementSource=%6 requestedUnits=%7", m_sBaseName, packageIndex, prefab, infantryPackage.LastKnownPosition.ToString(), spawnPosition.ToString(), placementSource, infantryPackage.UnitCount), true);
		return true;
	}

	bool JLH_DCF_SpawnGhostDefenceVehiclePackage(JLH_DCF_GhostVehicleDefencePackage vehiclePackage, vector defendTarget, notnull array<SCR_AIGroup> groups, notnull array<IEntity> units, notnull array<IEntity> vehicles, notnull array<IEntity> waypoints)
	{
		if (!IsRegistered() || !GetOwner() || !vehiclePackage)
			return false;

		if (IsEmptyResource(vehiclePackage.VehiclePrefab) || IsEmptyResource(vehiclePackage.CrewPrefab))
			return false;

		vector preferredPosition = vehiclePackage.DefencePosition;
		if (vehiclePackage.HasLastSafePosition)
			preferredPosition = vehiclePackage.LastSafePosition;

		vector spawnPosition;
		string placementSource;
		string skippedReason;
		string placementContext = string.Format("base=%1 originalPrefab=%2 preferredPosition=%3", m_sBaseName, vehiclePackage.VehiclePrefab, preferredPosition.ToString());
		if (!ResolveGhostVehiclePackagePosition(preferredPosition, vehiclePackage.HasLastSafePosition || vector.Distance(vehiclePackage.DefencePosition, "0 0 0") >= 0.1, defendTarget, vehicles.Count() + 1, spawnPosition, placementSource, skippedReason, placementContext))
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "vehicle_package_skipped " + placementContext + " reason=no_safe_position", true);
			return false;
		}

		IEntity vehicle = SpawnVehicleAtGhostPackage(vehiclePackage.VehiclePrefab, GetOwner(), spawnPosition, vehiclePackage);
		if (!vehicle)
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Ghost defence vehicle skipped base=%1 originalPrefab=%2 preferredPosition=%3 resolvedPosition=%4 placementSource=%5 skippedReason=spawn_failed", m_sBaseName, vehiclePackage.VehiclePrefab, preferredPosition.ToString(), spawnPosition.ToString(), placementSource), true);
			return false;
		}

		SCR_AIGroup crewGroup = SpawnVehiclePassengerGroupAt(vehiclePackage.CrewPrefab, GetOwner(), JLH_DCF_GhostPlacement.GroundPosition(spawnPosition + Vector(2, 0, 0)));
		if (!crewGroup)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(vehicle);
			return false;
		}

		if (!RegisterUsableVehicle(crewGroup, vehicle))
		{
			DeleteGroupAndMembers(crewGroup);
			SCR_EntityHelper.DeleteEntityAndChildren(vehicle);
			return false;
		}

		array<IEntity> routeWaypoints = {};
		if (!AssignMountedVehicleHoldWaypointToGroup(crewGroup, spawnPosition, routeWaypoints))
		{
			CleanupRouteWaypoints(crewGroup, routeWaypoints);
			DeleteGroupAndMembers(crewGroup);
			SCR_EntityHelper.DeleteEntityAndChildren(vehicle);
			return false;
		}

		ActivateGroupAI(crewGroup);
		AddUniqueGroup(crewGroup, groups);
		AddUniqueEntity(vehicle, vehicles);
		m_aAttackVehicleHoldPositions.Insert(spawnPosition);
		JLH_DCF_AssaultVehicleHoldRegistry.Reserve(spawnPosition);
		CollectGroupUnitsToArray(crewGroup, units);
		foreach (IEntity waypoint : routeWaypoints)
		{
			AddUniqueEntity(waypoint, waypoints);
		}

		ScheduleGhostDefenceVehicleMount(crewGroup, vehicle, vehiclePackage.VehiclePrefab, 0);
		JLH_DCF_VehicleSustainment.RegisterVehicle(SYSTEM_NAME, vehicle, crewGroup, m_sFactionKey, m_sBaseName);
		JLH_DCF_PersistenceExclusion.StopTrackingTree(crewGroup, "recap_ghost_vehicle_package_ready_group");
		JLH_DCF_PersistenceExclusion.StopTrackingTree(vehicle, "recap_ghost_vehicle_package_ready_vehicle");
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("ghost mounted vehicle defence spawned originalPrefab=%1 preferredPosition=%2 resolvedPosition=%3 placementSource=%4 crew=%5", vehiclePackage.VehiclePrefab, preferredPosition.ToString(), spawnPosition.ToString(), placementSource, vehiclePackage.CrewCount), true);
		return true;
	}

	protected bool ResolveGhostVehiclePackagePosition(vector preferredPosition, bool hasPreferredPosition, vector defendTarget, int packageIndex, out vector resolvedPosition, out string placementSource, out string skippedReason, string placementContext)
	{
		resolvedPosition = "0 0 0";
		placementSource = "skipped";
		skippedReason = "no_safe_position";

		if (hasPreferredPosition && vector.Distance(preferredPosition, "0 0 0") >= 0.1)
		{
			vector lastKnown = GroundPosition(preferredPosition);
			if (IsGhostVehiclePackagePositionClear(lastKnown, defendTarget, true, skippedReason))
			{
				resolvedPosition = lastKnown;
				placementSource = "last_known";
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "placement_resolved " + placementContext + " source=last_known resolvedPosition=" + resolvedPosition.ToString(), true);
				return true;
			}

			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "placement_rejected " + placementContext + " source=last_known reason=" + skippedReason + " trying=safe_nearby", true);
			if (TryResolveGhostVehiclePackageRing(lastKnown, defendTarget, packageIndex, 12.0, 12.0, 4, "safe_nearby", resolvedPosition, skippedReason, placementContext))
			{
				placementSource = "safe_nearby";
				return true;
			}

			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "placement_rejected " + placementContext + " source=safe_nearby reason=" + skippedReason + " trying=base_anchor", true);
		}

		vector anchor = GroundPosition(defendTarget);
		if (TryResolveGhostVehiclePackageRing(anchor, defendTarget, packageIndex, 55.0, 35.0, 5, "base_anchor", resolvedPosition, skippedReason, placementContext))
		{
			placementSource = "base_anchor";
			return true;
		}

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "placement_rejected " + placementContext + " source=base_anchor reason=" + skippedReason + " trying=road_fallback", true);
		if (TryResolveGhostVehiclePackageRing(anchor, defendTarget, packageIndex, 90.0, 45.0, 5, "road_fallback", resolvedPosition, skippedReason, placementContext))
		{
			placementSource = "road_fallback";
			return true;
		}

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "placement_rejected " + placementContext + " source=road_fallback reason=" + skippedReason + " trying=skipped", true);
		return false;
	}

	protected bool TryResolveGhostVehiclePackageRing(vector center, vector defendTarget, int packageIndex, float firstRadius, float radiusStep, int ringCount, string source, out vector resolvedPosition, out string skippedReason, string placementContext)
	{
		resolvedPosition = "0 0 0";
		int directionOffset = packageIndex % 8;
		if (directionOffset < 0)
			directionOffset = 0;

		for (int ring = 0; ring < ringCount; ring++)
		{
			float radius = firstRadius + (ring * radiusStep);
			for (int step = 0; step < 8; step++)
			{
				int directionIndex = directionOffset + step + (ring * 3);
				vector candidate = GroundPosition(center + (GetVehicleHoldDirection(directionIndex) * radius));
				if (!IsGhostVehiclePackagePositionClear(candidate, defendTarget, false, skippedReason))
					continue;

				resolvedPosition = candidate;
				skippedReason = "ok";
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "placement_resolved " + placementContext + " source=" + source + " resolvedPosition=" + resolvedPosition.ToString(), true);
				return true;
			}
		}

		if (skippedReason == "" || skippedReason == "ok")
			skippedReason = "no_safe_position";

		return false;
	}

	protected bool IsGhostVehiclePackagePositionClear(vector candidate, vector defendTarget, bool trustLastKnown, out string reason)
	{
		if (!JLH_DCF_GhostPlacement.ValidateVehiclePosition(candidate, m_aAttackVehicleHoldPositions, VEHICLE_HOLD_SPACING_METERS, GHOST_DEFENCE_SAFE_SPAWN_DISTANCE, reason))
			return false;

		if (trustLastKnown)
			return true;

		return IsVehicleHoldPositionClear(candidate, defendTarget, reason);
	}

	protected SCR_AIGroup SpawnGroupPrefabAt(ResourceName groupPrefab, IEntity owner, vector position)
	{
		Resource resource = Resource.Load(groupPrefab);
		if (!resource || !resource.IsValid())
			return null;

		if (!owner || !GetGame() || !GetGame().GetWorld())
			return null;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		owner.GetTransform(spawnParams.Transform);
		spawnParams.Transform[3] = GroundPosition(position);

		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams));
		if (!group)
			return null;

		if (!group.GetSpawnImmediately())
			group.SpawnUnits();

		JLH_DCF_PersistenceExclusion.StopTrackingTree(group, "recap_ghost_group_spawn");
		//SetMembersAlive(0);
		SetIsSpawned(false);
		SetIsPaused(true);
		return group;
	}

	protected vector ResolveGhostDefenceInfantrySpawnPosition(vector defendTarget, int slotIndex)
	{
		int directionOffset = Math.RandomInt(0, 8);
		for (int ring = 0; ring < 4; ring++)
		{
			float radius = GHOST_DEFENCE_INFANTRY_RADIUS_METERS + (ring * GHOST_DEFENCE_INFANTRY_RING_STEP_METERS);
			for (int step = 0; step < 8; step++)
			{
				int directionIndex = directionOffset + slotIndex + step + (ring * 3);
				vector candidate = GroundPosition(defendTarget + (GetVehicleHoldDirection(directionIndex) * radius));
				if (IsGhostDefenceInfantrySpawnPositionClear(candidate))
					return candidate;
			}
		}

		return GroundPosition(defendTarget + (GetVehicleHoldDirection(directionOffset + slotIndex) * GHOST_DEFENCE_INFANTRY_RADIUS_METERS));
	}

	protected bool IsGhostDefenceInfantrySpawnPositionClear(vector candidate)
	{
		string placementReason;
		if (!JLH_DCF_GhostPlacement.ValidateInfantryPosition(candidate, m_aAttackInfantrySpawnPositions, GHOST_DEFENCE_INFANTRY_SPACING_METERS, GHOST_DEFENCE_SAFE_SPAWN_DISTANCE, placementReason))
			return false;

		foreach (vector plannedPosition : m_aAttackInfantrySpawnPositions)
		{
			if (vector.Distance(candidate, plannedPosition) < GHOST_DEFENCE_INFANTRY_SPACING_METERS)
				return false;
		}

		ref array<IEntity> players = {};
		GatherSpawnSafetyPlayers(players);
		if (IsAnyValidPlayerWithinPositionWithPlayers(candidate, GHOST_DEFENCE_SAFE_SPAWN_DISTANCE, players) && HasAnyValidPlayerLineOfSightWithPlayers(candidate, GHOST_DEFENCE_SAFE_SPAWN_DISTANCE, players))
			return false;

		m_aAttackInfantrySpawnPositions.Insert(candidate);
		return true;
	}

	protected void ScheduleGhostDefenceVehicleMount(SCR_AIGroup crewGroup, IEntity vehicle, ResourceName vehiclePrefab, int retryCount)
	{
		if (!GetGame() || !GetGame().GetCallqueue())
			return;

		GetGame().GetCallqueue().CallLater(JLH_DCF_RecapNodeComponent_HandleGhostDefenceVehicleMount, VEHICLE_MOUNT_RETRY_DELAY_MS, false, this, crewGroup, vehicle, vehiclePrefab, retryCount);
	}

	void HandleGhostDefenceVehicleMount(SCR_AIGroup crewGroup, IEntity vehicle, ResourceName vehiclePrefab, int retryCount)
	{
		if (!crewGroup || !vehicle)
			return;

		JLH_DCF_VehicleMountStats stats = new JLH_DCF_VehicleMountStats();
		if (MountVehicleCrew(crewGroup, vehicle, vehiclePrefab, stats, false))
		{
			LogArmedSupportVehicleGroupMount(vehicle, stats);
			ActivateGroupAI(crewGroup);
			JLH_DCF_PersistenceExclusion.StopTrackingTree(crewGroup, "recap_ghost_vehicle_mount_finalized_group");
			JLH_DCF_PersistenceExclusion.StopTrackingTree(vehicle, "recap_ghost_vehicle_mount_finalized_vehicle");
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("ghost mounted vehicle defence activated vehicle=%1 crewSeen=%2", vehiclePrefab, stats.CrewSeen), true);
			return;
		}

		if (retryCount < VEHICLE_MOUNT_MAX_RETRIES)
		{
			ScheduleGhostDefenceVehicleMount(crewGroup, vehicle, vehiclePrefab, retryCount + 1);
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("ghost mounted vehicle defence mount retry vehicle=%1 retry=%2 reason=%3", vehiclePrefab, retryCount + 1, stats.FailureReason), true);
			return;
		}

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("ghost mounted vehicle defence mount failed vehicle=%1 reason=%2", vehiclePrefab, stats.FailureReason), true);
	}

	protected void AddUniqueGroup(SCR_AIGroup group, notnull array<SCR_AIGroup> groups)
	{
		if (group && !groups.Contains(group))
			groups.Insert(group);
	}

	protected void AddUniqueEntity(IEntity entity, notnull array<IEntity> entities)
	{
		if (entity && !entities.Contains(entity))
			entities.Insert(entity);
	}

	protected void CollectGroupUnitsToArray(SCR_AIGroup group, notnull array<IEntity> units)
	{
		if (!group)
			return;

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			AddUniqueEntity(agent.GetControlledEntity(), units);
		}
	}

	protected int PruneSpawnedGroupToRequestedUnits(SCR_AIGroup group, int requestedUnits, int packageIndex, notnull array<IEntity> retainedUnits)
	{
		retainedUnits.Clear();
		if (requestedUnits <= 0)
		{
			if (group)
				DeleteGroupAndMembers(group);

			LogVirtualInfantryRejected(requestedUnits, packageIndex);
			return 0;
		}

		if (!group)
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("virtual_infantry_prune base=%1 wave=%2 package=%3 spawned=0 requested=%4 deleted=0 final=0", m_sBaseName, m_iActiveAssaultId, packageIndex, requestedUnits), true);
			return 0;
		}

		array<IEntity> liveUnits = {};
		CollectLivingGroupUnitsForPrune(group, liveUnits);
		int spawned = liveUnits.Count();
		int deleted = 0;
		if (spawned > requestedUnits)
		{
			for (int i = liveUnits.Count() - 1; i >= requestedUnits; i--)
			{
				IEntity excessUnit = liveUnits[i];
				if (excessUnit)
				{
					DeleteRuntimeEntitySafe(excessUnit, "recap_virtual_infantry_prune");
					deleted++;
				}

				liveUnits.Remove(i);
			}
		}

		foreach (IEntity unit : liveUnits)
		{
			AddUniqueEntity(unit, retainedUnits);
		}

		int finalCount = spawned - deleted;
		if (finalCount < 0)
			finalCount = 0;

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("virtual_infantry_prune base=%1 wave=%2 package=%3 spawned=%4 requested=%5 deleted=%6 final=%7", m_sBaseName, m_iActiveAssaultId, packageIndex, spawned, requestedUnits, deleted, finalCount), true);
		return finalCount;
	}

	protected void CollectLivingGroupUnitsForPrune(SCR_AIGroup group, notnull array<IEntity> liveUnits)
	{
		if (!group)
			return;

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (unit && IsUnitAlive(unit) && !liveUnits.Contains(unit))
				liveUnits.Insert(unit);
		}
	}

	protected void LogVirtualInfantryRejected(int requestedUnits, int packageIndex)
	{
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("virtual_infantry_rejected base=%1 wave=%2 package=%3 reason=no_requested_units requested=%4", m_sBaseName, m_iActiveAssaultId, packageIndex, requestedUnits), true);
	}

	bool IsRegistered()
	{
		return m_bRegistered && !m_bDisabled;
	}

	bool IsAttackActive()
	{
		return m_bAttackActive;
	}

	string GetRegisteredBaseName()
	{
		return m_sBaseName;
	}

	SCR_CampaignMilitaryBaseComponent GetRegisteredBaseComponent()
	{
		return m_BaseComponent;
	}

	IEntity GetRegisteredBaseEntity()
	{
		return m_BaseEntity;
	}

	string GetRecapDefenderFactionKey()
	{
		return m_sFactionKey;
	}

	bool IsRecapDirectorEnabledOnNode()
	{
		return RecapEnabledByDirector;
	}

	bool IsRecapMajorLocation()
	{
		return RecapMajorLocation;
	}

	JLH_DCF_RecapRegion GetRecapDirectorRegion()
	{
		return RecapDirectorRegion;
	}

	vector GetRecapDirectorPosition()
	{
		if (m_BaseEntity)
			return m_BaseEntity.GetOrigin();

		IEntity owner = GetOwner();
		if (owner)
			return owner.GetOrigin();

		return "0 0 0";
	}

	string GetRecapNodeId()
	{
		string label = JLH_DCF_NodeDebug.EntityLabel(GetOwner());
		if (m_sBaseName != "" && label != "")
			return m_sBaseName + ":" + label;

		if (m_sBaseName != "")
			return m_sBaseName;

		return label;
	}

	protected void DisableNode(string reason)
	{
		m_bDisabled = true;
		JLH_DCF_NodeDebug.Warn(SYSTEM_NAME, string.Format("Disabled node name=%1 reason=%2", JLH_DCF_NodeDebug.EntityLabel(GetOwner()), reason));
	}

	protected void LogDebug(string message)
	{
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, message, DebugLogging);
	}

	protected string BuildVehicleRuntimeContext(JLH_DCF_RecapVehicleRuntime runtime)
	{
		if (!runtime)
			return string.Format("base=%1 assaultId=%2 slot=0 vehicle=NONE", m_sBaseName, m_iActiveAssaultId);

		string context = string.Format("base=%1 assaultId=%2 slot=%3 vehicle=%4", m_sBaseName, m_iActiveAssaultId, runtime.VehicleIndex, JLH_DCF_NodeDebug.EntityLabel(runtime.VehicleEntity));
		context += string.Format(" vehiclePos=%1 crewGroup=%2 prefab=%3 crewPrefab=%4", GetEntityPositionLabel(runtime.VehicleEntity), JLH_DCF_NodeDebug.EntityLabel(runtime.CrewGroup), runtime.VehiclePrefab, runtime.CrewPrefab);
		context += string.Format(" passengerDelivery=%1 mountPending=%2", JLH_DCF_NodeDebug.BoolLabel(runtime.PassengerDeliveryMode), JLH_DCF_NodeDebug.BoolLabel(runtime.MountPending));
		return context;
	}

	protected string GetEntityPositionLabel(IEntity entity)
	{
		if (!entity)
			return "NONE";

		return entity.GetOrigin().ToString();
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

	protected void DeleteGroupAndMembers(SCR_AIGroup group)
	{
		if (!group)
			return;

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (unit)
				DeleteRuntimeEntitySafe(unit, "recap_group_member");
		}

		DeleteRuntimeEntitySafe(group, "recap_group");
	}

	protected void DeleteRuntimeEntitySafe(IEntity entity, string reason)
	{
		if (!entity)
			return;

		JLH_DCF_RuntimeCleanupManager.UnregisterTree(entity);
		JLH_AddonRuntimeEntityMetadata.ForgetTree(entity);
		JLH_DCF_PersistenceExclusion.StopTrackingTreeNow(entity, reason);
		SCR_EntityHelper.DeleteEntityAndChildren(entity);
	}

	protected void SuppressInheritedAmbientState(bool logSuppression)
	{
		m_bRecapControlledSpawnAllowed = false;
		//SetMembersAlive(0);
		SetIsSpawned(false);
		SetIsPaused(true);
	}
}

void JLH_DCF_RecapNodeComponent_HandleTransportDismountCheck(JLH_RecapNodeComponent node, JLH_DCF_RecapVehicleRuntime runtime)
{
	if (node)
		node.HandleTransportDismountCheck(runtime);
}

void JLH_DCF_RecapNodeComponent_HandleTransportPassengerHandoff(JLH_RecapNodeComponent node, JLH_DCF_RecapVehicleRuntime runtime)
{
	if (node)
		node.HandleTransportPassengerHandoff(runtime);
}

void JLH_DCF_RecapNodeComponent_HandleTransportPassengerDefendAssignment(JLH_RecapNodeComponent node, JLH_DCF_RecapVehicleRuntime runtime)
{
	if (node)
		node.HandleTransportPassengerDefendAssignment(runtime);
}

void JLH_DCF_RecapNodeComponent_RequestTransportPassengerExit(JLH_RecapNodeComponent node, IEntity unit, IEntity vehicle)
{
	if (node)
		node.RequestTransportPassengerExit(unit, vehicle);
}

void JLH_DCF_RecapNodeComponent_HandleGhostDefenceVehicleMount(JLH_RecapNodeComponent node, SCR_AIGroup crewGroup, IEntity vehicle, ResourceName vehiclePrefab, int retryCount)
{
	if (node)
		node.HandleGhostDefenceVehicleMount(crewGroup, vehicle, vehiclePrefab, retryCount);
}
