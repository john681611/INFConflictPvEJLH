enum JLH_DCF_QRFForcePreset
{
	CUSTOM_MANUAL = 0,
	LIGHT_PROBE = 1,
	STANDARD_ASSAULT = 2,
	HEAVY_ASSAULT = 3,
	TRANSPORT_ASSAULT = 4,
	RANDOM = 5
}

enum JLH_DCF_QRFVehicleSupportState
{
	ACTIVE_SUPPORT = 0,
	SUPPORT_COMPLETE = 1,
	SAFE_TO_REMOVE = 2
}

class JLH_DCF_QRFVehicleSupportRuntime
{
	IEntity VehicleEntity;
	SCR_AIGroup CrewGroup;
	IEntity DriverEntity;
	IEntity GunnerEntity;
	ResourceName VehiclePrefab;
	ResourceName CrewPrefab;
	vector DropPosition;
	vector TransportCapCenter;
	bool DropPending;
	bool MountPending;
	int MountReadyTick;
	int MountRetryCount;
	int WaveIndex;
	int VehicleIndex;
	int ActiveSupportStartedTick;
	int SupportCompletedTick;
	int NextCleanupAttemptTick;
	int IntendedCrewCount;
	int LastDamageTick;
	int LastMovementTick;
	int LastCleanupDeferredLogTick;
	float LastHealthScaled;
	float LastCrewHealthScaled;
	vector LastPosition;
	string LastCleanupDeferredReason;
	string MountFailureReason;
	string TransportBaseKey;
	string TransportLockToken;
	bool LastPositionValid;
	bool TransportMoveAssigned;
	bool TransportDismountScheduled;
	bool TransportDismountRequested;
	bool TransportPassengerHandoffComplete;
	bool TransportDismountScheduleLogged;
	bool TransportDropoffSpeedApplied;
	bool TransportPassengerSearchDestroyScheduled;
	bool TransportPassengerSearchDestroyDefendFallbackScheduled;
	bool ArmedMountedDefence;
	int TransportDismountVerifyCount;
	JLH_DCF_QRFVehicleSupportState SupportState;
	SCR_AIGroup TransportPassengerGroup;
	ref array<IEntity> VehicleCrewUnits = {};
	ref array<IEntity> TransportPassengerUnits = {};
	ref array<IEntity> RouteWaypoints = {};

	void Init(IEntity vehicle, SCR_AIGroup crewGroup, IEntity driver, IEntity gunner, ResourceName vehiclePrefab, ResourceName crewPrefab, array<IEntity> routeWaypoints, bool dropPending, vector dropPosition)
	{
		int now = System.GetTickCount();
		VehicleEntity = vehicle;
		CrewGroup = crewGroup;
		DriverEntity = driver;
		GunnerEntity = gunner;
		VehiclePrefab = vehiclePrefab;
		CrewPrefab = crewPrefab;
		DropPending = dropPending;
		MountPending = false;
		MountReadyTick = 0;
		MountRetryCount = 0;
		WaveIndex = 0;
		VehicleIndex = 0;
		DropPosition = dropPosition;
		TransportCapCenter = dropPosition;
		ActiveSupportStartedTick = now;
		SupportCompletedTick = 0;
		NextCleanupAttemptTick = 0;
		IntendedCrewCount = 0;
		LastDamageTick = 0;
		LastMovementTick = 0;
		LastCleanupDeferredLogTick = 0;
		LastHealthScaled = -1.0;
		LastCrewHealthScaled = -1.0;
		LastPosition = "0 0 0";
		LastCleanupDeferredReason = "";
		MountFailureReason = "";
		TransportBaseKey = "";
		TransportLockToken = "";
		LastPositionValid = false;
		TransportMoveAssigned = false;
		TransportDismountScheduled = false;
		TransportDismountRequested = false;
		TransportPassengerHandoffComplete = false;
		TransportDismountScheduleLogged = false;
		TransportDropoffSpeedApplied = false;
		TransportPassengerSearchDestroyScheduled = false;
		TransportPassengerSearchDestroyDefendFallbackScheduled = false;
		ArmedMountedDefence = false;
		TransportDismountVerifyCount = 0;
		SupportState = JLH_DCF_QRFVehicleSupportState.ACTIVE_SUPPORT;
		TransportPassengerGroup = null;
		VehicleCrewUnits.Clear();
		TransportPassengerUnits.Clear();
		RouteWaypoints.Clear();
		if (!routeWaypoints)
			return;

		foreach (IEntity waypoint : routeWaypoints)
		{
			if (waypoint)
				RouteWaypoints.Insert(waypoint);
		}
	}
}

[ComponentEditorProps(category: "JLH Dynamic Conflict Framework/Nodes", description: "World-placeable QRF response node built on SCR_AmbientPatrolSpawnPointComponent.")]
class JLH_QRFNodeComponentClass : SCR_AmbientPatrolSpawnPointComponentClass
{
}

class JLH_QRFNodeComponent : SCR_AmbientPatrolSpawnPointComponent
{
	protected static const string SYSTEM_NAME = "QRF";
	protected static const string TRIGGERING_PLAYER_FACTION_KEY = "US";
	protected static const int WAVE_UNIT_DISCOVERY_GRACE_MS = 2000;
	protected static const int SPAWN_SAFETY_LOG_THROTTLE_MS = 30000;
	protected static const int TRIGGER_REJECT_LOG_THROTTLE_MS = 30000;
	protected static const int PENDING_VEHICLE_MOUNT_DELAY_MS = 1500;
	protected static const int VEHICLE_MOUNT_RETRY_DELAY_MS = 500;
	protected static const int VEHICLE_MOUNT_MAX_RETRIES = 20;
	protected static const int VEHICLE_TRANSPORT_MOUNT_MAX_RETRIES = 3;
	protected static const int VEHICLE_SUPPORT_ACTIVE_WINDOW_MS = 300000;
	protected static const int VEHICLE_SUPPORT_CLEANUP_RETRY_MS = 30000;
	protected static const int VEHICLE_SUPPORT_RECENT_DAMAGE_GRACE_MS = 60000;
	protected static const int VEHICLE_SUPPORT_MOVEMENT_GRACE_MS = 45000;
	protected static const float VEHICLE_SUPPORT_CLEANUP_PLAYER_RADIUS = 900.0;
	protected static const float QRF_SUCCESS_PLAYER_CLEAR_DISTANCE = 1000.0;
	protected static const float QRF_VIRTUAL_WAVE_ACTIVATION_DISTANCE = 800.0;
	protected static const float QRF_VIRTUAL_WAVE_DEACTIVATION_DISTANCE = 1000.0;
	protected static const float QRF_VIRTUAL_WAVE_INFANTRY_RADIUS_METERS = 85.0;
	protected static const float QRF_VIRTUAL_WAVE_INFANTRY_RING_STEP_METERS = 35.0;
	protected static const float QRF_VIRTUAL_WAVE_INFANTRY_SPACING_METERS = 18.0;
	protected static const float QRF_VIRTUAL_WAVE_SAFE_SPAWN_DISTANCE = 500.0;
	protected static const int QRF_VIRTUAL_WAVE_ACTIVATION_RETRY_MS = 5000;
	protected static const int QRF_VIRTUAL_WAVE_DISCOVERY_GRACE_MS = 10000;
	protected static const float VEHICLE_SUPPORT_MOVEMENT_EPSILON_METERS = 3.0;
	protected static const int ARMED_SUPPORT_MAX_MOUNTED_CREW = 3;
	protected static const int TRANSPORT_TOTAL_CAPACITY = 15;
	protected static const int TRANSPORT_PASSENGER_GROUP_SIZE = 14;
	protected static const ResourceName DEFAULT_USSR_ARMED_SUPPORT_GROUP_PREFAB = "{D4CF0B2B840CF001}Prefabs/Groups/JLH_DCF/JLH_USSR_VehicleGroup.et";
	protected static const ResourceName DEFAULT_US_ARMED_SUPPORT_GROUP_PREFAB = "{101C1DACCD535E45}Prefabs/Groups/JLH_DCF/JLH_US_VehicleGroup.et";
	protected static const ResourceName DEFAULT_FIA_ARMED_SUPPORT_GROUP_PREFAB = "{B0F1A7E33C6B4205}Prefabs/Groups/JLH_DCF/USSR_GroupLibrary/JLH_USSR_LargeMixed_01.et";
	protected static const ResourceName DEFAULT_USSR_VEHICLE_PASSENGER_GROUP_PREFAB = "{AAE710F15D0C4B01}Prefabs/Groups/JLH_DCF/JLH_USSR_TransportAssaultGroup_15Man.et";
	protected static const ResourceName DEFAULT_US_VEHICLE_PASSENGER_GROUP_PREFAB = "{AAE710F15D0C4B02}Prefabs/Groups/JLH_DCF/JLH_US_TransportAssaultGroup_15Man.et";
	protected static const ResourceName DEFAULT_FIA_VEHICLE_PASSENGER_GROUP_PREFAB = "{AAE710F15D0C4B03}Prefabs/Groups/JLH_DCF/JLH_FIA_TransportAssaultGroup_15Man.et";
	protected static const ResourceName DEFAULT_USSR_TRANSPORT_VEHICLE_PREFAB = "{16C1F16C9B053801}Prefabs/Vehicles/Wheeled/Ural4320/Ural4320_transport.et";
	protected static const ResourceName ASSAULT_MOVE_WAYPOINT_PREFAB = "{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et";
	protected static const ResourceName CYCLE_WAYPOINT_PREFAB = "{35BD6541CBB8AC08}Prefabs/AI/Waypoints/AIWaypoint_Cycle.et";
	protected static const ResourceName DEFEND_CP_WAYPOINT_PREFAB = "{2A81753527971941}Prefabs/AI/Waypoints/AIWaypoint_Defend_CP.et";
	protected static const ResourceName DEFEND_WAYPOINT_PREFAB = "{93291E72AC23930F}Prefabs/AI/Waypoints/AIWaypoint_Defend.et";
	protected static const ResourceName SEARCH_AND_DESTROY_WAYPOINT_PREFAB = "Prefabs/AI/Waypoints/AIWaypoint_SearchAndDestroy.et";
	protected static const float VEHICLE_SUPPORT_SPAWN_SPACING_METERS = 22.0;
	protected static const float VEHICLE_SUPPORT_HOLD_SPACING_METERS = 28.0;
	protected static const float VEHICLE_SUPPORT_HOLD_RADIUS_METERS = 55.0;
	protected static const float VEHICLE_SUPPORT_HOLD_RING_STEP_METERS = 20.0;
	protected static const float VEHICLE_SUPPORT_HOLD_MIN_CAP_DISTANCE_METERS = 30.0;
	protected static const float TRANSPORT_ARRIVAL_RADIUS = 70.0;
	protected static const float TRANSPORT_DISMOUNT_DELAY = 7.0;
	protected static const float TRANSPORT_HANDOFF_VERIFY_DELAY = 4.5;
	protected static const float TRANSPORT_SEARCH_DESTROY_ASSIGN_DELAY = 5.0;
	protected static const float TRANSPORT_SEARCH_DESTROY_DEFEND_FALLBACK_DELAY = 30.0;
	protected static const float TRANSPORT_DROPOFF_CRUISE_SPEED_KMH = 25.0;
	protected static const int TRANSPORT_EXIT_STAGGER_MS = 250;
	protected static const int TRANSPORT_HANDOFF_MAX_VERIFY_ATTEMPTS = 8;
	protected static const int TRANSPORT_EXIT_DOOR_SCAN_COUNT = 8;
	protected static const int MAX_QRF_VEHICLES_PER_WAVE = 8;

	[Attribute("0", UIWidgets.ComboBox, "Main QRF force selector. CUSTOM_MANUAL uses the manual wave and vehicle fields; named presets control those values automatically.", "", ParamEnumArray.FromEnum(JLH_DCF_QRFForcePreset), category: "JLH DCF QRF - Basic")]
	protected JLH_DCF_QRFForcePreset ForcePreset;

	[Attribute("0", UIWidgets.EditBox, "Seconds to wait after an accepted trigger before spawning the first wave. Applies to all presets.", "0 3600 1", category: "JLH DCF QRF - Basic")]
	protected float ActivationDelaySeconds;

	[Attribute("", UIWidgets.EditBox, "Optional exact infantry group prefab. Leave empty for per-slot random or inherited vanilla group selection.", category: "JLH DCF QRF - Optional Overrides")]
	protected ResourceName GroupPrefabOverride;

	[Attribute("1200", UIWidgets.EditBox, "Seconds before this node can accept another trigger after finishing its waves. Applies to all presets.", "0 7200 1", category: "JLH DCF QRF - Basic")]
	protected float CooldownSeconds;

	[Attribute("3", UIWidgets.EditBox, "CUSTOM_MANUAL only. Named presets ignore this and choose their own wave count.", "1 20 1", category: "JLH DCF QRF - Wave Setup")]
	protected int MaxWaves;

	[Attribute("1", UIWidgets.EditBox, "CUSTOM_MANUAL only. Infantry groups spawned per wave; TRANSPORT_ASSAULT presets force this to 0.", "1 20 1", category: "JLH DCF QRF - Wave Setup")]
	protected int GroupsPerWave;

	[Attribute("1", UIWidgets.EditBox, "CUSTOM_MANUAL only. Wave number that receives vehicle support. Set 0 to disable manual vehicle support. Named presets control this automatically.", "0 20 1", category: "JLH DCF QRF - Wave Setup")]
	protected int AddVehicleToWave;

	[Attribute("1", UIWidgets.EditBox, "CUSTOM_MANUAL only. Number of vehicles spawned with the configured wave.", "1 8 1", category: "JLH DCF QRF - Wave Setup")]
	protected int VehicleCount;

	[Attribute("0", UIWidgets.EditBox, "CUSTOM_MANUAL only. Vehicles spawned on wave 1. Leave every wave vehicle count at 0 to use AddVehicleToWave and VehicleCount.", "0 8 1", category: "JLH DCF QRF - Vehicle Counts Per Wave")]
	protected int VehicleCountWave01;

	[Attribute("0", UIWidgets.EditBox, "CUSTOM_MANUAL only. Vehicles spawned on wave 2.", "0 8 1", category: "JLH DCF QRF - Vehicle Counts Per Wave")]
	protected int VehicleCountWave02;

	[Attribute("0", UIWidgets.EditBox, "CUSTOM_MANUAL only. Vehicles spawned on wave 3.", "0 8 1", category: "JLH DCF QRF - Vehicle Counts Per Wave")]
	protected int VehicleCountWave03;

	[Attribute("0", UIWidgets.EditBox, "CUSTOM_MANUAL only. Vehicles spawned on wave 4.", "0 8 1", category: "JLH DCF QRF - Vehicle Counts Per Wave")]
	protected int VehicleCountWave04;

	[Attribute("0", UIWidgets.EditBox, "CUSTOM_MANUAL only. Vehicles spawned on wave 5.", "0 8 1", category: "JLH DCF QRF - Vehicle Counts Per Wave")]
	protected int VehicleCountWave05;

	[Attribute("0", UIWidgets.EditBox, "CUSTOM_MANUAL only. Vehicles spawned on wave 6.", "0 8 1", category: "JLH DCF QRF - Vehicle Counts Per Wave")]
	protected int VehicleCountWave06;

	[Attribute("0", UIWidgets.EditBox, "CUSTOM_MANUAL only. Vehicles spawned on wave 7.", "0 8 1", category: "JLH DCF QRF - Vehicle Counts Per Wave")]
	protected int VehicleCountWave07;

	[Attribute("0", UIWidgets.EditBox, "CUSTOM_MANUAL only. Vehicles spawned on wave 8.", "0 8 1", category: "JLH DCF QRF - Vehicle Counts Per Wave")]
	protected int VehicleCountWave08;

	[Attribute("0", UIWidgets.EditBox, "CUSTOM_MANUAL only. Vehicles spawned on wave 9.", "0 8 1", category: "JLH DCF QRF - Vehicle Counts Per Wave")]
	protected int VehicleCountWave09;

	[Attribute("0", UIWidgets.EditBox, "CUSTOM_MANUAL only. Vehicles spawned on wave 10.", "0 8 1", category: "JLH DCF QRF - Vehicle Counts Per Wave")]
	protected int VehicleCountWave10;

	[Attribute("0", UIWidgets.EditBox, "CUSTOM_MANUAL only. Vehicles spawned on wave 11.", "0 8 1", category: "JLH DCF QRF - Vehicle Counts Per Wave")]
	protected int VehicleCountWave11;

	[Attribute("0", UIWidgets.EditBox, "CUSTOM_MANUAL only. Vehicles spawned on wave 12.", "0 8 1", category: "JLH DCF QRF - Vehicle Counts Per Wave")]
	protected int VehicleCountWave12;

	[Attribute("0", UIWidgets.EditBox, "CUSTOM_MANUAL only. Vehicles spawned on wave 13.", "0 8 1", category: "JLH DCF QRF - Vehicle Counts Per Wave")]
	protected int VehicleCountWave13;

	[Attribute("0", UIWidgets.EditBox, "CUSTOM_MANUAL only. Vehicles spawned on wave 14.", "0 8 1", category: "JLH DCF QRF - Vehicle Counts Per Wave")]
	protected int VehicleCountWave14;

	[Attribute("0", UIWidgets.EditBox, "CUSTOM_MANUAL only. Vehicles spawned on wave 15.", "0 8 1", category: "JLH DCF QRF - Vehicle Counts Per Wave")]
	protected int VehicleCountWave15;

	[Attribute("0", UIWidgets.EditBox, "CUSTOM_MANUAL only. Vehicles spawned on wave 16.", "0 8 1", category: "JLH DCF QRF - Vehicle Counts Per Wave")]
	protected int VehicleCountWave16;

	[Attribute("0", UIWidgets.EditBox, "CUSTOM_MANUAL only. Vehicles spawned on wave 17.", "0 8 1", category: "JLH DCF QRF - Vehicle Counts Per Wave")]
	protected int VehicleCountWave17;

	[Attribute("0", UIWidgets.EditBox, "CUSTOM_MANUAL only. Vehicles spawned on wave 18.", "0 8 1", category: "JLH DCF QRF - Vehicle Counts Per Wave")]
	protected int VehicleCountWave18;

	[Attribute("0", UIWidgets.EditBox, "CUSTOM_MANUAL only. Vehicles spawned on wave 19.", "0 8 1", category: "JLH DCF QRF - Vehicle Counts Per Wave")]
	protected int VehicleCountWave19;

	[Attribute("0", UIWidgets.EditBox, "CUSTOM_MANUAL only. Vehicles spawned on wave 20.", "0 8 1", category: "JLH DCF QRF - Vehicle Counts Per Wave")]
	protected int VehicleCountWave20;

	[Attribute("", UIWidgets.EditBox, "Optional exact vehicle prefab. For TRANSPORT_ASSAULT this must be a valid transport truck; otherwise it is ignored.", category: "JLH DCF QRF - Optional Overrides")]
	protected ResourceName SpecificVehiclePrefab;

	[Attribute("", UIWidgets.Hidden, "Optional armed support vehicle prefab pool. Ignored by TRANSPORT_ASSAULT. Leave empty to use trait-filtered random armed support.", category: "JLH DCF QRF - Internal")]
	protected ref array<ResourceName> ArmedSupportVehiclePrefabs;

	[Attribute("", UIWidgets.Hidden, "Optional TRANSPORT_ASSAULT truck pool. Max one active transport is allowed per base. Leave empty to use default faction transport trucks.", category: "JLH DCF QRF - Internal")]
	protected ref array<ResourceName> TransportVehiclePrefabs;

	[Attribute("0", UIWidgets.Hidden, "Armed support trait filter. Defaults at runtime to TRAIT_ARMED and TRAIT_ARMORED when empty; ignored by TRANSPORT_ASSAULT.", "", ParamEnumArray.FromEnum(EEditableEntityLabel), category: "JLH DCF QRF - Internal")]
	protected ref array<EEditableEntityLabel> IncludedVehicleTraits;

	[Attribute("0", UIWidgets.Hidden, "Armed support trait exclusions. Defaults at runtime to VEHICLE_HELICOPTER and VEHICLE_AIRPLANE when empty.", "", ParamEnumArray.FromEnum(EEditableEntityLabel), category: "JLH DCF QRF - Internal")]
	protected ref array<EEditableEntityLabel> ExcludedVehicleTraits;

	[Attribute("0", UIWidgets.Hidden, "Armed support trait matching. Enabled means every included trait must match; disabled means any included trait may match.", category: "JLH DCF QRF - Internal")]
	protected bool RequireAllIncludedVehicleTraits;

	[Attribute("1", UIWidgets.Hidden, "Keep armed/turret vehicle support mounted as vehicle defence. TRANSPORT_ASSAULT still unloads cargo passengers.", category: "JLH DCF QRF - Internal")]
	protected bool QRFKeepArmedVehiclesMounted;

	[Attribute("15", UIWidgets.Hidden, "Seconds to delay vehicle support after infantry spawns and starts moving. Applies to armed and transport support.", "0 300 1", category: "JLH DCF QRF - Internal")]
	protected float VehicleSpawnDelaySeconds;

	[Attribute("5", UIWidgets.Hidden, "Additional random seconds added to vehicle support delay.", "0 120 1", category: "JLH DCF QRF - Internal")]
	protected float VehicleSpawnDelayJitter;

	[Attribute("1", UIWidgets.Hidden, "Prevent QRF infantry and vehicle support from spawning visibly in front of nearby players. Defers and retries instead of permanently failing activation.", category: "JLH DCF QRF - Internal")]
	protected bool EnableSpawnVisibilitySafety;

	[Attribute("175", UIWidgets.Hidden, "Infantry waves inside this distance require visibility checks before spawning.", "0 2000 1", category: "JLH DCF QRF - Internal")]
	protected float MinimumInfantrySpawnSafetyDistance;

	[Attribute("250", UIWidgets.Hidden, "Vehicle support waves inside this distance require visibility checks before spawning.", "0 3000 1", category: "JLH DCF QRF - Internal")]
	protected float MinimumVehicleSpawnSafetyDistance;

	[Attribute("5", UIWidgets.Hidden, "Seconds between spawn visibility retry checks while a wave is deferred.", "1 120 1", category: "JLH DCF QRF - Internal")]
	protected float SpawnSafetyRetryDelay;

	[Attribute("30", UIWidgets.Hidden, "Maximum seconds to defer a wave for spawn visibility before optional unsafe fallback.", "0 600 1", category: "JLH DCF QRF - Internal")]
	protected float MaximumSpawnSafetyDeferral;

	[Attribute("1", UIWidgets.Hidden, "Allow the least-bad same-base QRF node to spawn after maximum visibility deferral expires.", category: "JLH DCF QRF - Internal")]
	protected bool AllowUnsafeFallbackAfterTimeout;

	[Attribute("1", UIWidgets.Hidden, "If enabled, close spawns are blocked only when a player has direct line of sight. If disabled, close distance alone defers spawning.", category: "JLH DCF QRF - Internal")]
	protected bool RequireLOSToBlockCloseSpawn;

	[Attribute("1", UIWidgets.Hidden, "Advanced. Clean up this node's previously owned runtime entities before a new activation.", category: "JLH DCF QRF - Internal")]
	protected bool CleanupBeforeActivation;

	[Attribute("0", UIWidgets.CheckBox, "Enable detailed registration, trigger, rejection, spawn, and cleanup logs for this node.", category: "JLH DCF QRF - Debug")]
	protected bool DebugLogging;

	protected SCR_CampaignMilitaryBaseComponent m_BaseComponent;
	protected IEntity m_BaseEntity;
	protected string m_sBaseName;
	protected string m_sFactionKey;
	protected bool m_bRegistered;
	protected bool m_bDisabled;
	protected int m_iNextAllowedTriggerTick;
	protected int m_iPendingActivationTick;
	protected bool m_bActivationPending;
	protected bool m_bWaveActive;
	protected int m_iCurrentWave;
	protected int m_iCurrentWaveSpawnTick;
	protected int m_iEffectiveMaxWaves;
	protected int m_iEffectiveGroupsPerWave;
	protected int m_iEffectiveAddVehicleToWave;
	protected int m_iEffectiveVehicleCount;
	protected JLH_DCF_QRFForcePreset m_eResolvedForcePreset;
	protected bool m_bEffectivePassengerDelivery;
	protected bool m_bEffectiveVehicleCountsPerWave;
	protected int m_iNextSpawnSafetyRetryTick;
	protected int m_iSpawnSafetyDeferralStartedTick;
	protected int m_iLastSpawnSafetyLogTick;
	protected int m_iLastSpawnSafetyRetryLogTick;
	protected int m_iLastActiveRejectLogTick;
	protected int m_iLastCooldownRejectLogTick;
	protected int m_iLastWaveAliveLogCount = -1;
	protected bool m_bCurrentWaveHadTrackedUnits;
	protected bool m_bCurrentWaveHadVehicleSupport;
	protected bool m_bCurrentWaveVehicleSupportClearedLogged;
	protected bool m_bVehicleSupportWaveWarningLogged;
	protected bool m_bVehicleSupportSpawnPending;
	protected bool m_bCurrentWaveVirtualized;
	protected int m_iPendingVehicleSupportWave;
	protected int m_iPendingVehicleSupportCount;
	protected int m_iPendingVehicleSupportIntentionalSkips;
	protected int m_iPendingVehicleSupportReadyTick;
	protected int m_iVirtualWaveStoredCount;
	protected int m_iVirtualWaveInfantryCount;
	protected int m_iVirtualWaveNextActivationAttemptTick;
	protected int m_iVirtualWaveDiscoveryGraceUntilTick;
	protected vector m_vPendingVehicleSupportDefendTarget;
	protected ResourceName m_rVirtualWavePreferredGroupPrefab;
	protected string m_sPendingReason;
	protected string m_sLastSpawnSafetyLogKey;
	protected string m_sLastDefendTargetSource;
	protected bool m_bQRFControlledSpawnAllowed;
	protected bool m_bVanillaSuppressionLogged;
	protected bool m_bRegistrationAttempted;
	protected ref JLH_DCF_NodeRuntimeTracker m_RuntimeTracker = new JLH_DCF_NodeRuntimeTracker();
	protected ref array<IEntity> m_aRuntimeRouteWaypoints = {};
	protected ref array<SCR_AIGroup> m_aCurrentWaveGroups = {};
	protected ref array<IEntity> m_aCurrentWaveUnits = {};
	protected ref array<IEntity> m_aCurrentWaveVehicles = {};
	protected ref array<vector> m_aCurrentWaveInfantrySpawnPositions = {};
	protected ref array<vector> m_aCurrentWaveVehicleSpawnPositions = {};
	protected ref array<vector> m_aCurrentWaveVehicleHoldPositions = {};
	protected ref array<int> m_aEffectiveVehicleCountsPerWave = {};
	protected ref array<ref JLH_DCF_QRFVehicleSupportRuntime> m_aCurrentWaveVehicleSupports = {};
	protected ref array<ref JLH_DCF_QRFVehicleSupportRuntime> m_aVehicleSupportsAwaitingCleanup = {};
	protected ref array<ref JLH_DCF_QRFVehicleSupportRuntime> m_aRetainedTransportSupports = {};
	protected ref array<ref JLH_DCF_GhostInfantryDefencePackage> m_aVirtualWaveInfantryPackages = {};
	protected ref array<ref JLH_DCF_GhostVehicleDefencePackage> m_aVirtualWaveVehiclePackages = {};
	protected ref array<ref JLH_DCF_GhostVehicleDefencePackage> m_aPendingVirtualWaveVehiclePackages = {};
	protected JLH_QRFNodeComponent m_SelectedSpawnNode;

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		EnsureDefaultVehicleTraitFilters();
		SetEventMask(owner, EntityEvent.INIT);
		LogDebug(string.Format("Init node=%1 phase=OnPostInit", JLH_DCF_NodeDebug.EntityLabel(owner)));
		SuppressInheritedAmbientState(false);
	}

	override void EOnInit(IEntity owner)
	{
		LogDebug(string.Format("Init node=%1 phase=EOnInit", JLH_DCF_NodeDebug.EntityLabel(owner)));
		JLH_QRFManager.RegisterNode(this);
	}

	override void SpawnPatrol()
	{
		if (!m_bQRFControlledSpawnAllowed)
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

		if (!m_bRegistrationAttempted)
		{
			m_bRegistrationAttempted = true;
			LogDebug(string.Format("Register attempt node=%1", JLH_DCF_NodeDebug.EntityLabel(GetOwner())));
		}

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
			m_RuntimeTracker.SetRuntimeOwner("QRF");

		string baseRegistrationSource;
		string failureReason;
		if (!JLH_DCF_NodeBaseResolver.ResolveBase(owner, "", true, m_BaseComponent, m_BaseEntity, m_sBaseName, baseRegistrationSource, failureReason))
		{
			JLH_DCF_NodeDebug.Warn(SYSTEM_NAME, string.Format("Registration failed reason=no_parent_base node=%1", JLH_DCF_NodeDebug.EntityLabel(owner)));
			DisableNode(failureReason);
			return;
		}

		//SetMembersAlive(0);
		SetIsSpawned(false);
		SetIsPaused(true);

		m_bRegistered = true;
		JLH_DCF_GhostDefenceManager.RegisterQRFAnchor(this);

		LogDebug(string.Format("Registered node=%1 base=%2 faction=%3", JLH_DCF_NodeDebug.EntityLabel(owner), m_sBaseName, m_sFactionKey));
		LogDebug(string.Format("Node registered base=%1", m_sBaseName));
		if (!m_bVanillaSuppressionLogged)
		{
			LogDebug(string.Format("Vanilla ambient spawn suppressed node=%1", JLH_DCF_NodeDebug.EntityLabel(owner)));
			m_bVanillaSuppressionLogged = true;
		}

		LogDebug(string.Format("Waiting for seizing trigger node=%1", JLH_DCF_NodeDebug.EntityLabel(owner)));
	}

	void Tick()
	{
		if (!m_bRegistered || m_bDisabled)
			return;

		if (m_RuntimeTracker)
			m_RuntimeTracker.PruneMissing();

		if (m_RuntimeTracker && m_RuntimeTracker.CountTrackedGroups() <= 0 && !m_aRuntimeRouteWaypoints.IsEmpty())
			CleanupRuntimeRouteWaypoints();

		int now = System.GetTickCount();
		if (!m_bActivationPending)
			TickCooldown(now);

		if (m_bActivationPending)
			TickActivation(now);

		TickVehicleSupportCleanup(now);
		TickRetainedTransportSupports(now);
	}

	void EvaluateBaseState()
	{
		if (!m_bRegistered || m_bDisabled || !m_BaseComponent)
			return;

		if (!RefreshNodeFaction(GetOwner()))
			return;

		EvaluateSeizingTrigger();
	}

	protected void EvaluateSeizingTrigger()
	{
		string currentOwner = JLH_DCF_NodeBaseResolver.GetBaseFactionKey(m_BaseComponent);
		if (currentOwner == TRIGGERING_PLAYER_FACTION_KEY)
		{
			JLH_DCF_GhostDefenceManager.ClearForOwnershipChange(m_sBaseName);
			return;
		}

		SCR_CampaignSeizingComponent seizingComponent = FindSeizingComponent();
		if (!seizingComponent)
		{
			LogDebug(string.Format("Seizing trigger rejected reason=no_seizing_component base=%1", m_sBaseName));
			return;
		}

		string prevailingFaction = seizingComponent.JLH_DCF_GetPrevailingFactionKey();
		LogDebug(string.Format("Seizing event seen base=%1 owner=%2 seizingFaction=%3", m_sBaseName, currentOwner, prevailingFaction));
		if (prevailingFaction != TRIGGERING_PLAYER_FACTION_KEY)
		{
			LogDebug(string.Format("Seizing trigger rejected reason=triggering_faction_not_us base=%1 playerFaction=%2", m_sBaseName, prevailingFaction));
			return;
		}

		RequestActivation("base_seizing_by_us");
	}

	protected void RequestActivation(string reason)
	{
		int now = System.GetTickCount();

		if (m_bActivationPending)
		{
			LogActiveTriggerRejected(now);
			return;
		}

		if (m_iNextAllowedTriggerTick > now)
		{
			LogCooldownTriggerRejected(now);
			return;
		}

		if (JLH_DCF_GhostDefenceManager.HasGhostOrActiveDefence(m_sBaseName))
		{
			JLH_DCF_GhostDefenceManager.LogSpawnBlocked(m_sBaseName);
			return;
		}

		if (CleanupBeforeActivation)
		{
			if (m_RuntimeTracker)
				m_RuntimeTracker.CleanupOwnedRuntime(SYSTEM_NAME, "qrf_reactivation", DebugLogging);

			CleanupRuntimeRouteWaypoints();
			m_aVehicleSupportsAwaitingCleanup.Clear();
		}

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Seizing trigger accepted base=%1 playerFaction=%2", m_sBaseName, TRIGGERING_PLAYER_FACTION_KEY), true);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "Trigger accepted reason=us_seizing_registered_base", true);
		ResolveQRFPresetForActivation();

		m_iCurrentWave = 0;
		m_iCurrentWaveSpawnTick = 0;
		m_iNextSpawnSafetyRetryTick = 0;
		m_iSpawnSafetyDeferralStartedTick = 0;
		m_iLastSpawnSafetyLogTick = 0;
		m_iLastSpawnSafetyRetryLogTick = 0;
		m_iLastActiveRejectLogTick = 0;
		m_iLastCooldownRejectLogTick = 0;
		m_bWaveActive = false;
		m_iLastWaveAliveLogCount = -1;
		m_bCurrentWaveHadTrackedUnits = false;
		m_bCurrentWaveHadVehicleSupport = false;
		m_bCurrentWaveVehicleSupportClearedLogged = false;
		m_bVehicleSupportWaveWarningLogged = false;
		ClearVirtualWaveState();
		ClearPendingVirtualWaveVehiclePackages("new_activation");
		m_sPendingReason = reason;
		m_sLastSpawnSafetyLogKey = "";
		m_SelectedSpawnNode = null;
		m_iPendingActivationTick = now + SecondsToMs(ActivationDelaySeconds);
		m_bActivationPending = true;
		ClearCurrentWaveTracking();

		LogDebug(string.Format("Activation started waves=%1 groupsPerWave=%2 delaySeconds=%3 node=%4 base=%5", m_iEffectiveMaxWaves, m_iEffectiveGroupsPerWave, ActivationDelaySeconds, JLH_DCF_NodeDebug.EntityLabel(GetOwner()), m_sBaseName));
	}

	protected void TickActivation(int now)
	{
		if (!m_bActivationPending)
			return;

		if (m_bCurrentWaveVirtualized)
		{
			TickVirtualizedCurrentWave(now);
			return;
		}

		if (!m_bWaveActive)
		{
			if (m_iCurrentWave == 0 && now < m_iPendingActivationTick)
				return;

			SpawnNextWave();
		}

		if (m_bWaveActive)
		{
			TickPendingVehicleSupport(now);
			TickCurrentWaveVehicleSupport(now);
			if (TryVirtualizeCurrentWaveIfEligible())
				return;

			EvaluateCurrentWave();
		}
	}

	protected void ResolveQRFPresetForActivation()
	{
		m_eResolvedForcePreset = ForcePreset;
		if (m_eResolvedForcePreset == JLH_DCF_QRFForcePreset.RANDOM)
		{
			int presetRoll = Math.RandomInt(0, 4);
			if (presetRoll == 0)
				m_eResolvedForcePreset = JLH_DCF_QRFForcePreset.LIGHT_PROBE;
			else if (presetRoll == 1)
				m_eResolvedForcePreset = JLH_DCF_QRFForcePreset.STANDARD_ASSAULT;
			else if (presetRoll == 2)
				m_eResolvedForcePreset = JLH_DCF_QRFForcePreset.HEAVY_ASSAULT;
			else
				m_eResolvedForcePreset = JLH_DCF_QRFForcePreset.TRANSPORT_ASSAULT;

			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("RANDOM resolved preset=%1", GetQRFPresetLabel(m_eResolvedForcePreset)), true);
		}

		m_iEffectiveMaxWaves = MaxWaves;
		m_iEffectiveGroupsPerWave = GroupsPerWave;
		m_iEffectiveAddVehicleToWave = AddVehicleToWave;
		m_iEffectiveVehicleCount = VehicleCount;
		m_bEffectivePassengerDelivery = false;
		m_bEffectiveVehicleCountsPerWave = false;
		m_aEffectiveVehicleCountsPerWave.Clear();

		if (m_eResolvedForcePreset == JLH_DCF_QRFForcePreset.LIGHT_PROBE)
		{
			m_iEffectiveMaxWaves = 1;
			m_iEffectiveGroupsPerWave = 1;
			m_iEffectiveAddVehicleToWave = 0;
			m_iEffectiveVehicleCount = 0;
		}
		else if (m_eResolvedForcePreset == JLH_DCF_QRFForcePreset.STANDARD_ASSAULT)
		{
			m_iEffectiveMaxWaves = 2;
			m_iEffectiveGroupsPerWave = 1;
			m_iEffectiveAddVehicleToWave = 1;
			m_iEffectiveVehicleCount = 1;
		}
		else if (m_eResolvedForcePreset == JLH_DCF_QRFForcePreset.HEAVY_ASSAULT)
		{
			m_iEffectiveMaxWaves = 4;
			m_iEffectiveGroupsPerWave = 2;
			m_iEffectiveAddVehicleToWave = 1;
			m_iEffectiveVehicleCount = 1;
		}
		else if (m_eResolvedForcePreset == JLH_DCF_QRFForcePreset.TRANSPORT_ASSAULT)
		{
			m_iEffectiveMaxWaves = 1;
			m_iEffectiveGroupsPerWave = 0;
			m_iEffectiveAddVehicleToWave = 1;
			m_iEffectiveVehicleCount = 1;
			m_bEffectivePassengerDelivery = true;
		}

		if (m_iEffectiveMaxWaves < 1)
			m_iEffectiveMaxWaves = 1;
		if (m_iEffectiveGroupsPerWave < 0)
			m_iEffectiveGroupsPerWave = 0;
		if (!m_bEffectivePassengerDelivery && m_iEffectiveGroupsPerWave < 1)
			m_iEffectiveGroupsPerWave = 1;
		if (m_iEffectiveVehicleCount < 0)
			m_iEffectiveVehicleCount = 0;
		if (m_iEffectiveVehicleCount > MAX_QRF_VEHICLES_PER_WAVE)
			m_iEffectiveVehicleCount = MAX_QRF_VEHICLES_PER_WAVE;

		BuildEffectiveVehicleCountsPerWave();

		int vehicleTotal = GetEffectiveVehicleSupportTotal();
		int armedVehicles = vehicleTotal;
		int transportVehicles = 0;
		if (m_bEffectivePassengerDelivery)
		{
			armedVehicles = 0;
			transportVehicles = vehicleTotal;
		}

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("preset selected=%1", GetQRFPresetLabel(ForcePreset)), true);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("preset resolved infantryGroups=%1 armedVehicles=%2 transportVehicles=%3 passengerDelivery=%4", m_iEffectiveGroupsPerWave, armedVehicles, transportVehicles, m_bEffectivePassengerDelivery), true);
		if (m_bEffectiveVehicleCountsPerWave)
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("manual vehicle counts per wave=%1", BuildVehicleCountsPerWaveLabel()), true);

		if (m_eResolvedForcePreset == JLH_DCF_QRFForcePreset.TRANSPORT_ASSAULT)
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("TRANSPORT_ASSAULT resolved infantryGroups=%1 transportVehicles=%2 passengerDelivery=1", m_iEffectiveGroupsPerWave, transportVehicles), true);
	}

	protected void BuildEffectiveVehicleCountsPerWave()
	{
		m_aEffectiveVehicleCountsPerWave.Clear();
		m_bEffectiveVehicleCountsPerWave = false;

		if (m_eResolvedForcePreset != JLH_DCF_QRFForcePreset.CUSTOM_MANUAL)
			return;

		for (int waveIndex = 1; waveIndex <= m_iEffectiveMaxWaves; waveIndex++)
		{
			if (GetConfiguredVehicleCountForWave(waveIndex) > 0)
			{
				m_bEffectiveVehicleCountsPerWave = true;
				break;
			}
		}

		if (!m_bEffectiveVehicleCountsPerWave)
			return;

		for (int waveIndex = 1; waveIndex <= m_iEffectiveMaxWaves; waveIndex++)
		{
			m_aEffectiveVehicleCountsPerWave.Insert(ClampVehicleCount(GetConfiguredVehicleCountForWave(waveIndex)));
		}
	}

	protected int GetConfiguredVehicleCountForWave(int waveIndex)
	{
		if (waveIndex == 1)
			return VehicleCountWave01;
		if (waveIndex == 2)
			return VehicleCountWave02;
		if (waveIndex == 3)
			return VehicleCountWave03;
		if (waveIndex == 4)
			return VehicleCountWave04;
		if (waveIndex == 5)
			return VehicleCountWave05;
		if (waveIndex == 6)
			return VehicleCountWave06;
		if (waveIndex == 7)
			return VehicleCountWave07;
		if (waveIndex == 8)
			return VehicleCountWave08;
		if (waveIndex == 9)
			return VehicleCountWave09;
		if (waveIndex == 10)
			return VehicleCountWave10;
		if (waveIndex == 11)
			return VehicleCountWave11;
		if (waveIndex == 12)
			return VehicleCountWave12;
		if (waveIndex == 13)
			return VehicleCountWave13;
		if (waveIndex == 14)
			return VehicleCountWave14;
		if (waveIndex == 15)
			return VehicleCountWave15;
		if (waveIndex == 16)
			return VehicleCountWave16;
		if (waveIndex == 17)
			return VehicleCountWave17;
		if (waveIndex == 18)
			return VehicleCountWave18;
		if (waveIndex == 19)
			return VehicleCountWave19;
		if (waveIndex == 20)
			return VehicleCountWave20;

		return 0;
	}

	protected int GetEffectiveVehicleSupportTotal()
	{
		int total = 0;
		for (int waveIndex = 1; waveIndex <= m_iEffectiveMaxWaves; waveIndex++)
		{
			total += GetEffectiveVehicleCountForWave(waveIndex);
		}

		return total;
	}

	protected int GetEffectiveVehicleCountForWave(int waveIndex)
	{
		if (waveIndex <= 0 || waveIndex > m_iEffectiveMaxWaves)
			return 0;

		if (m_bEffectiveVehicleCountsPerWave)
		{
			int arrayIndex = waveIndex - 1;
			if (arrayIndex < 0 || arrayIndex >= m_aEffectiveVehicleCountsPerWave.Count())
				return 0;

			return m_aEffectiveVehicleCountsPerWave[arrayIndex];
		}

		if (m_iEffectiveAddVehicleToWave <= 0 || m_iEffectiveAddVehicleToWave > m_iEffectiveMaxWaves)
			return 0;

		if (waveIndex != m_iEffectiveAddVehicleToWave)
			return 0;

		return ClampVehicleCount(m_iEffectiveVehicleCount);
	}

	protected int ClampVehicleCount(int vehicleCount)
	{
		if (vehicleCount < 0)
			return 0;

		if (vehicleCount > MAX_QRF_VEHICLES_PER_WAVE)
			return MAX_QRF_VEHICLES_PER_WAVE;

		return vehicleCount;
	}

	protected string BuildVehicleCountsPerWaveLabel()
	{
		string label = "";
		for (int i = 0; i < m_aEffectiveVehicleCountsPerWave.Count(); i++)
		{
			if (label != "")
				label += ",";

			label += string.Format("%1", m_aEffectiveVehicleCountsPerWave[i]);
		}

		return label;
	}

	protected void SpawnNextWave()
	{
		if (!m_bActivationPending)
			return;

		if (m_iCurrentWave >= m_iEffectiveMaxWaves)
		{
			CompleteActivation();
			return;
		}

		int now = System.GetTickCount();
		if (m_iNextSpawnSafetyRetryTick > now)
			return;

		if (!CanSpawnWaveSafely())
		{
			m_iNextSpawnSafetyRetryTick = now + SecondsToMs(SpawnSafetyRetryDelay);
			LogSpawnSafetyRetryScheduled();
			return;
		}

		m_iNextSpawnSafetyRetryTick = 0;
		m_iSpawnSafetyDeferralStartedTick = 0;
		int nextWaveIndex = m_iCurrentWave + 1;
		if (m_iCurrentWave > 0)
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Next wave spawning index=%1", nextWaveIndex), true);

		m_iCurrentWave++;
		m_iCurrentWaveSpawnTick = now;
		m_bWaveActive = true;
		m_iLastWaveAliveLogCount = -1;
		m_bCurrentWaveHadTrackedUnits = false;
		m_bCurrentWaveHadVehicleSupport = false;
		m_bCurrentWaveVehicleSupportClearedLogged = false;
		ClearCurrentWaveTracking();

		int spawned = SpawnGroups(m_iEffectiveGroupsPerWave, m_sPendingReason);
		SpawnWaveVehicleSupport(m_iCurrentWave);
		int infantryGroups = spawned;
		int supportVehicles = CountCurrentWaveVehicles();
		int supportCrew = CountCurrentWaveSupportCrew();
		int trackedAlive = CountLivingCurrentWaveUnits();

		string waveSummary = string.Format("Wave spawned index=%1/%2 requestedGroups=%3 spawnedGroups=%4 infantryGroups=%5 supportVehicles=%6 supportCrew=%7 trackedAlive=%8", m_iCurrentWave, m_iEffectiveMaxWaves, m_iEffectiveGroupsPerWave, spawned, infantryGroups, supportVehicles, supportCrew, trackedAlive);
		waveSummary = waveSummary + string.Format(" node=%1 base=%2", JLH_DCF_NodeDebug.EntityLabel(GetOwner()), m_sBaseName);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, waveSummary, true);
	}

	protected void EvaluateCurrentWave()
	{
		if (m_bCurrentWaveVirtualized)
			return;

		if (m_bVehicleSupportSpawnPending)
			return;

		int now = System.GetTickCount();
		if (m_iVirtualWaveDiscoveryGraceUntilTick > now && CountLivingCurrentWaveUnits() <= 0)
			return;

		int trackedUnits = CountCurrentWaveUnits();
		if (trackedUnits > 0)
			m_bCurrentWaveHadTrackedUnits = true;

		if (!m_bCurrentWaveHadTrackedUnits && m_iVirtualWaveDiscoveryGraceUntilTick > now)
			return;

		if (!m_bCurrentWaveHadTrackedUnits && System.GetTickCount() - m_iCurrentWaveSpawnTick < WAVE_UNIT_DISCOVERY_GRACE_MS)
			return;

		int alive = CountLivingCurrentWaveUnits();
		if (alive > 0)
		{
			m_iVirtualWaveDiscoveryGraceUntilTick = 0;
			LogWaveWaiting(alive);
			return;
		}

		LogVehicleSupportCleared();
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Wave cleared index=%1", m_iCurrentWave), true);
		ClearPendingVirtualWaveVehiclePackages("wave_cleared");
		ClearCurrentWaveTracking();
		m_bWaveActive = false;

		if (m_iCurrentWave >= m_iEffectiveMaxWaves)
		{
			CompleteActivation();
			return;
		}

		SpawnNextWave();
	}

	protected void CompleteActivation()
	{
		CompleteActivationWithReason("waves_complete");
	}

	protected void CompleteActivationWithReason(string reason)
	{
		bool successfulOutcome = IsQRFSuccessOutcomeConfirmed();
		int survivorsRetained = 0;
		if (successfulOutcome)
		{
			survivorsRetained = StoreQRFSurvivorsAsGhostDefence();
		}
		else
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Ghost handoff skipped base=%1 reason=outcome_not_successful owner=%2 defender=%3 completeReason=%4", m_sBaseName, JLH_DCF_NodeBaseResolver.GetBaseFactionKey(m_BaseComponent), m_sFactionKey, reason), true);
			if (m_RuntimeTracker)
				m_RuntimeTracker.CleanupOwnedRuntime(SYSTEM_NAME, "qrf_unsuccessful", DebugLogging);

			ReleaseRetainedTransportLocks("qrf_unsuccessful");
			CleanupRuntimeRouteWaypoints();
			m_aVehicleSupportsAwaitingCleanup.Clear();
		}

		m_bActivationPending = false;
		m_bWaveActive = false;
		m_iPendingActivationTick = 0;
		m_iCurrentWaveSpawnTick = 0;
		m_iNextSpawnSafetyRetryTick = 0;
		m_iSpawnSafetyDeferralStartedTick = 0;
		m_iLastSpawnSafetyLogTick = 0;
		m_iLastSpawnSafetyRetryLogTick = 0;
		m_iLastActiveRejectLogTick = 0;
		m_iLastCooldownRejectLogTick = 0;
		m_bCurrentWaveHadTrackedUnits = false;
		m_bCurrentWaveHadVehicleSupport = false;
		m_bCurrentWaveVehicleSupportClearedLogged = false;
		m_sLastSpawnSafetyLogKey = "";
		m_SelectedSpawnNode = null;
		ClearVirtualWaveState();
		ClearPendingVirtualWaveVehiclePackages(reason);
		ClearCurrentWaveTracking();
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Activation complete reason=%1 waves=%2 survivorsRetained=%3 node=%4 base=%5", reason, m_iCurrentWave, survivorsRetained, JLH_DCF_NodeDebug.EntityLabel(GetOwner()), m_sBaseName), true);
		StartCooldown();
	}

	protected int StoreQRFSurvivorsAsGhostDefence()
	{
		if (!m_BaseComponent || m_sBaseName == "" || m_sFactionKey == "")
			return 0;

		ref array<SCR_AIGroup> groups = {};
		ref array<IEntity> units = {};
		ref array<IEntity> vehicles = {};
		ref array<IEntity> waypoints = {};
		ref array<SCR_AIGroup> trackedGroups = {};
		ref array<IEntity> trackedUnits = {};
		ref array<IEntity> trackedVehicles = {};
		ref array<ref JLH_DCF_GhostVehicleDefencePackage> vehiclePackages = {};
		ResourceName preferredPrefab = GroupPrefabOverride;
		CollectQRFSurvivorState(groups, units, vehicles, waypoints, trackedGroups, trackedUnits, trackedVehicles, vehiclePackages, "ghost_stored");
		MergePendingVirtualWaveVehiclePackages(vehiclePackages, "ghost_handoff");
		ref array<IEntity> noLooseUnits = {};
		string handoffLog = string.Format("Ghost handoff candidates base=%1 groups=%2 groupUnits=%3 units=%4 vehicles=%5 trackerGroups=%6 trackerUnits=%7 trackerVehicles=%8 vehiclePackages=%9", m_sBaseName, groups.Count(), CountLivingUnitsForVirtualWave(groups, noLooseUnits), units.Count(), vehicles.Count(), trackedGroups.Count(), trackedUnits.Count(), trackedVehicles.Count(), vehiclePackages.Count());
		handoffLog += string.Format(" vehiclePackageCrew=%1", CountVehiclePackageCrewForVirtualWave(vehiclePackages));
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, handoffLog, true);
		int stored = JLH_DCF_GhostDefenceManager.StoreFromQRF(m_sBaseName, m_BaseComponent, m_BaseEntity, m_sFactionKey, preferredPrefab, groups, units, vehicles, waypoints, vehiclePackages);
		m_aRetainedTransportSupports.Clear();
		m_aVehicleSupportsAwaitingCleanup.Clear();
		m_aRuntimeRouteWaypoints.Clear();
		if (m_RuntimeTracker)
			m_RuntimeTracker.ForgetAll();

		return stored;
	}

	protected bool TryVirtualizeCurrentWaveIfEligible()
	{
		if (!m_bWaveActive || m_bCurrentWaveVirtualized || m_bVehicleSupportSpawnPending)
			return false;

		if (!IsQRFSuccessOutcomeConfirmed())
			return false;

		if (HasLiveHumanNearDefendTarget(QRF_VIRTUAL_WAVE_DEACTIVATION_DISTANCE))
			return false;

		if (CountLivingCurrentWaveUnits() <= 0)
			return false;

		return VirtualizeCurrentWave("no_players_near");
	}

	protected bool CanCleanupWavePhysicalStateSafely(string label, array<SCR_AIGroup> groups, array<IEntity> units, array<IEntity> vehicles, array<IEntity> waypoints, array<ref JLH_DCF_GhostVehicleDefencePackage> vehiclePackages, out string blockedReason)
	{
		JLH_CleanupSafetyQuery query = new JLH_CleanupSafetyQuery();
		query.Init(label, ResolveWaveCleanupAnchor(groups, units, vehicles));
		query.CleanupDistance = JLH_CleanupSafetyService.DEFAULT_CLEANUP_DISTANCE_METERS;
		query.VisibilityDistance = JLH_CleanupSafetyService.DEFAULT_VISIBILITY_DISTANCE_METERS;
		query.PlayerFactionFilter = "";
		query.DebugEnabled = DebugLogging;

		foreach (SCR_AIGroup group : groups)
		{
			if (group && !query.Groups.Contains(group))
				query.Groups.Insert(group);
		}
		foreach (IEntity unit : units)
		{
			if (unit && !query.Entities.Contains(unit))
				query.Entities.Insert(unit);
		}
		foreach (IEntity vehicle : vehicles)
		{
			if (vehicle && !query.Vehicles.Contains(vehicle))
				query.Vehicles.Insert(vehicle);
		}
		foreach (IEntity waypoint : waypoints)
		{
			if (waypoint && !query.Waypoints.Contains(waypoint))
				query.Waypoints.Insert(waypoint);
		}
		foreach (JLH_DCF_GhostVehicleDefencePackage vehiclePackage : vehiclePackages)
		{
			if (!vehiclePackage)
				continue;
			if (vehiclePackage.SourceGroup && !query.Groups.Contains(vehiclePackage.SourceGroup))
				query.Groups.Insert(vehiclePackage.SourceGroup);
			if (vehiclePackage.SourceVehicle && !query.Vehicles.Contains(vehiclePackage.SourceVehicle))
				query.Vehicles.Insert(vehiclePackage.SourceVehicle);
			foreach (IEntity sourceUnit : vehiclePackage.SourceUnits)
			{
				if (sourceUnit && !query.Entities.Contains(sourceUnit))
					query.Entities.Insert(sourceUnit);
			}
			foreach (IEntity sourceWaypoint : vehiclePackage.SourceWaypoints)
			{
				if (sourceWaypoint && !query.Waypoints.Contains(sourceWaypoint))
					query.Waypoints.Insert(sourceWaypoint);
			}
		}

		return JLH_CleanupSafetyService.CanCleanupPackage(query, blockedReason);
	}

	protected vector ResolveWaveCleanupAnchor(array<SCR_AIGroup> groups, array<IEntity> units, array<IEntity> vehicles)
	{
		foreach (IEntity vehicle : vehicles)
		{
			if (vehicle)
				return vehicle.GetOrigin();
		}
		foreach (IEntity unit : units)
		{
			if (unit)
				return unit.GetOrigin();
		}
		foreach (SCR_AIGroup group : groups)
		{
			if (group)
				return group.GetOrigin();
		}
		if (GetOwner())
			return GetOwner().GetOrigin();

		return "0 0 0";
	}

	protected bool VirtualizeCurrentWave(string reason)
	{
		ref array<SCR_AIGroup> groups = {};
		ref array<IEntity> units = {};
		ref array<IEntity> vehicles = {};
		ref array<IEntity> waypoints = {};
		ref array<SCR_AIGroup> trackedGroups = {};
		ref array<IEntity> trackedUnits = {};
		ref array<IEntity> trackedVehicles = {};
		ref array<ref JLH_DCF_GhostInfantryDefencePackage> infantryPackages = {};
		ref array<ref JLH_DCF_GhostVehicleDefencePackage> vehiclePackages = {};

		CollectQRFSurvivorState(groups, units, vehicles, waypoints, trackedGroups, trackedUnits, trackedVehicles, vehiclePackages, "qrf_virtualized");
		int collectedVehiclePackages = vehiclePackages.Count();
		int pendingVehiclePackages = m_aPendingVirtualWaveVehiclePackages.Count();
		MergePendingVirtualWaveVehiclePackages(vehiclePackages, "virtualize");
		BuildInfantryPackagesForVirtualWave(GroupPrefabOverride, groups, units, infantryPackages);

		int vehiclePackageCrew = CountVehiclePackageCrewForVirtualWave(vehiclePackages);
		int infantryCount = CountInfantryPackageUnitsForVirtualWave(infantryPackages);
		if (infantryCount <= 0)
			infantryCount = CountLivingUnitsForVirtualWave(groups, units);
		int storedCount = infantryCount + vehiclePackageCrew;
		if (storedCount <= 0)
			return false;

		m_iVirtualWaveStoredCount = storedCount;
		m_iVirtualWaveInfantryCount = infantryCount;
		m_iVirtualWaveNextActivationAttemptTick = 0;
		m_iVirtualWaveDiscoveryGraceUntilTick = 0;
		m_rVirtualWavePreferredGroupPrefab = GroupPrefabOverride;
		m_aVirtualWaveInfantryPackages.Clear();
		m_aVirtualWaveVehiclePackages.Clear();
		CopyInfantryPackagesForVirtualWave(infantryPackages, m_aVirtualWaveInfantryPackages);
		CopyVehiclePackagesForVirtualWave(vehiclePackages, m_aVirtualWaveVehiclePackages);

		string blockedReason;
		if (!CanCleanupWavePhysicalStateSafely("QRFVirtualWave base=" + m_sBaseName + " wave=" + m_iCurrentWave.ToString(), groups, units, vehicles, waypoints, vehiclePackages, blockedReason))
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Virtual wave deferred base=%1 wave=%2 reason=%3", m_sBaseName, m_iCurrentWave, blockedReason), true);
			return false;
		}

		CleanupVirtualizedWavePhysicalState(groups, units, vehicles, waypoints, vehiclePackages);
		m_aRetainedTransportSupports.Clear();
		m_aVehicleSupportsAwaitingCleanup.Clear();
		m_aRuntimeRouteWaypoints.Clear();
		if (m_RuntimeTracker)
			m_RuntimeTracker.ForgetAll();

		ClearCurrentWaveTracking();
		m_bCurrentWaveVirtualized = true;
		m_bWaveActive = true;
		m_iLastWaveAliveLogCount = -1;
		m_bCurrentWaveHadTrackedUnits = true;
		m_bCurrentWaveHadVehicleSupport = !m_aVirtualWaveVehiclePackages.IsEmpty();
		m_bCurrentWaveVehicleSupportClearedLogged = false;

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("vehicle_package_count before_virtualization base=%1 wave=%2 collected=%3 pending=%4 afterMerge=%5", m_sBaseName, m_iCurrentWave, collectedVehiclePackages, pendingVehiclePackages, vehiclePackages.Count()), true);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Virtual wave stored base=%1 wave=%2 count=%3 infantryPackages=%4 vehiclePackages=%5 reason=%6", m_sBaseName, m_iCurrentWave, storedCount, m_aVirtualWaveInfantryPackages.Count(), m_aVirtualWaveVehiclePackages.Count(), reason), true);
		return true;
	}

	protected void TickVirtualizedCurrentWave(int now)
	{
		if (!m_bCurrentWaveVirtualized)
			return;

		if (m_iVirtualWaveStoredCount <= 0)
		{
			ClearVirtualWaveState();
			m_bWaveActive = false;
			EvaluateCurrentWave();
			return;
		}

		if (!HasLiveHumanNearDefendTarget(QRF_VIRTUAL_WAVE_ACTIVATION_DISTANCE))
			return;

		ActivateVirtualizedCurrentWave(now);
	}

	protected void ActivateVirtualizedCurrentWave(int now)
	{
		if (!m_bCurrentWaveVirtualized || m_iVirtualWaveStoredCount <= 0)
			return;

		if (m_iVirtualWaveNextActivationAttemptTick > now)
			return;

		vector defendTarget;
		string defendTargetFailureReason;
		if (!ResolveDefendTarget(defendTarget, defendTargetFailureReason))
		{
			m_iVirtualWaveNextActivationAttemptTick = now + QRF_VIRTUAL_WAVE_ACTIVATION_RETRY_MS;
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Virtual wave activation deferred base=%1 wave=%2 reason=%3", m_sBaseName, m_iCurrentWave, defendTargetFailureReason), true);
			return;
		}

		ref array<SCR_AIGroup> spawnedGroups = {};
		ref array<IEntity> spawnedUnits = {};
		ref array<IEntity> spawnedVehicles = {};
		ref array<IEntity> spawnedWaypoints = {};
		ref array<ref JLH_DCF_GhostVehicleDefencePackage> failedVehiclePackages = {};

		ClearCurrentWaveTracking();
		int requestedInfantryPackages = m_aVirtualWaveInfantryPackages.Count();
		int requestedVehiclePackages = m_aVirtualWaveVehiclePackages.Count();
		int vehiclePackagesSpawned = SpawnVirtualWaveVehiclePackages(defendTarget, spawnedGroups, spawnedUnits, spawnedVehicles, spawnedWaypoints, failedVehiclePackages);

		int targetAfterVehiclePackages = CountLivingUnitsInArrayForVirtualWave(spawnedUnits) + m_iVirtualWaveInfantryCount;
		int infantryPackagesSpawned = 0;
		if (!m_aVirtualWaveInfantryPackages.IsEmpty())
		{
			for (int packageIndex = 0; packageIndex < m_aVirtualWaveInfantryPackages.Count(); packageIndex++)
			{
				if (SpawnVirtualWaveInfantryPackage(m_aVirtualWaveInfantryPackages[packageIndex], defendTarget, packageIndex + 1, spawnedGroups, spawnedUnits, spawnedWaypoints))
					infantryPackagesSpawned++;
			}
		}
		else
		{
			int attempts = 0;
			while (CountLivingUnitsInArrayForVirtualWave(spawnedUnits) < targetAfterVehiclePackages && attempts < m_iVirtualWaveInfantryCount)
			{
				int livingBeforeSpawn = CountLivingUnitsInArrayForVirtualWave(spawnedUnits);
				attempts++;
				bool spawned = SpawnVirtualWaveGroup(m_rVirtualWavePreferredGroupPrefab, defendTarget, attempts, spawnedGroups, spawnedUnits, spawnedWaypoints);
				if (!spawned)
					break;

				int livingAfterSpawn = CountLivingUnitsInArrayForVirtualWave(spawnedUnits);
				if (livingAfterSpawn <= livingBeforeSpawn && spawnedGroups.Count() > 0)
					break;
			}
		}

		TrimUnitsArrayToCountForVirtualWave(spawnedUnits, targetAfterVehiclePackages);

		int activeCount = CountLivingUnitsInArrayForVirtualWave(spawnedUnits);
		if (activeCount <= 0 && spawnedGroups.IsEmpty() && spawnedVehicles.IsEmpty())
		{
			CleanupVirtualizedWavePhysicalState(spawnedGroups, spawnedUnits, spawnedVehicles, spawnedWaypoints, m_aVirtualWaveVehiclePackages);
			m_iVirtualWaveNextActivationAttemptTick = now + QRF_VIRTUAL_WAVE_ACTIVATION_RETRY_MS;
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Virtual wave activation deferred base=%1 wave=%2 reason=no_spawned_entities", m_sBaseName, m_iCurrentWave), true);
			return;
		}

		foreach (SCR_AIGroup group : spawnedGroups)
		{
			if (!group)
				continue;

			TrackCurrentWaveGroup(group);
			if (m_RuntimeTracker)
				m_RuntimeTracker.TrackGroup(group);
		}

		foreach (IEntity unit : spawnedUnits)
		{
			AddUniqueEntity(unit, m_aCurrentWaveUnits);
			if (m_RuntimeTracker)
				m_RuntimeTracker.TrackUnit(unit);
		}

		foreach (IEntity vehicle : spawnedVehicles)
		{
			AddUniqueEntity(vehicle, m_aCurrentWaveVehicles);
			if (m_RuntimeTracker)
				m_RuntimeTracker.TrackVehicle(vehicle);
		}

		foreach (IEntity waypoint : spawnedWaypoints)
		{
			AddUniqueEntity(waypoint, m_aRuntimeRouteWaypoints);
		}

		if (activeCount <= 0)
			activeCount = m_iVirtualWaveStoredCount;

		RefreshPendingVirtualWaveVehiclePackages(failedVehiclePackages, "restore_failed");
		ClearVirtualWaveState();
		m_bCurrentWaveVirtualized = false;
		m_bWaveActive = true;
		m_iCurrentWaveSpawnTick = now;
		m_iLastWaveAliveLogCount = -1;
		m_bCurrentWaveHadTrackedUnits = false;
		m_bCurrentWaveHadVehicleSupport = !spawnedVehicles.IsEmpty() || !m_aPendingVirtualWaveVehiclePackages.IsEmpty();
		m_bCurrentWaveVehicleSupportClearedLogged = false;
		m_iVirtualWaveDiscoveryGraceUntilTick = now + QRF_VIRTUAL_WAVE_DISCOVERY_GRACE_MS;

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Virtual wave activated base=%1 wave=%2 spawned=%3 infantryPackages=%4 spawnedInfantryPackages=%5 vehiclePackages=%6 spawnedVehiclePackages=%7 pendingVehiclePackages=%8 reason=player_near", m_sBaseName, m_iCurrentWave, activeCount, requestedInfantryPackages, infantryPackagesSpawned, requestedVehiclePackages, vehiclePackagesSpawned, m_aPendingVirtualWaveVehiclePackages.Count()), true);
	}

	protected bool SpawnVirtualWaveInfantryPackage(JLH_DCF_GhostInfantryDefencePackage infantryPackage, vector defendTarget, int packageIndex, notnull array<SCR_AIGroup> groups, notnull array<IEntity> units, notnull array<IEntity> waypoints)
	{
		if (!infantryPackage)
			return false;

		if (infantryPackage.UnitCount <= 0)
		{
			LogVirtualInfantryRejected(infantryPackage.UnitCount, packageIndex);
			return false;
		}

		ResourceName prefab = infantryPackage.GroupPrefab;
		if (IsEmptyResource(prefab))
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Virtual wave infantry skipped base=%1 wave=%2 package=%3 originalPrefab=%4 preferredPosition=%5 placementSource=skipped skippedReason=resource_missing", m_sBaseName, m_iCurrentWave, packageIndex, infantryPackage.GroupPrefab, infantryPackage.LastKnownPosition.ToString()), true);
			return false;
		}

		vector spawnPosition;
		string placementSource;
		string skippedReason;
		if (!JLH_DCF_GhostPlacement.ResolveInfantryPosition(infantryPackage.LastKnownPosition, infantryPackage.HasLastKnownPosition, defendTarget, packageIndex, m_aCurrentWaveInfantrySpawnPositions, QRF_VIRTUAL_WAVE_INFANTRY_SPACING_METERS, QRF_VIRTUAL_WAVE_SAFE_SPAWN_DISTANCE, spawnPosition, placementSource, skippedReason))
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Virtual wave infantry skipped base=%1 wave=%2 package=%3 originalPrefab=%4 preferredPosition=%5 placementSource=skipped skippedReason=%6", m_sBaseName, m_iCurrentWave, packageIndex, prefab, infantryPackage.LastKnownPosition.ToString(), skippedReason), true);
			return false;
		}

		LogGroupPrefabSelected("virtual_wave_package", m_iCurrentWave, packageIndex, prefab);
		SCR_AIGroup group = SpawnGroupPrefabAt(prefab, spawnPosition, infantryPackage.UnitCount);
		if (!group)
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Virtual wave infantry skipped base=%1 wave=%2 package=%3 originalPrefab=%4 preferredPosition=%5 resolvedPosition=%6 placementSource=%7 skippedReason=spawn_failed", m_sBaseName, m_iCurrentWave, packageIndex, prefab, infantryPackage.LastKnownPosition.ToString(), spawnPosition.ToString(), placementSource), true);
			return false;
		}

		array<IEntity> retainedUnits = {};
		PruneSpawnedGroupToRequestedUnits(group, infantryPackage.UnitCount, packageIndex, retainedUnits);

		array<IEntity> routeWaypoints = {};
		if (!AssignDefendWaypointToGroup(group, defendTarget, routeWaypoints))
		{
			CleanupRouteWaypoints(group, routeWaypoints);
			DeleteGroupAndMembers(group);
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Virtual wave infantry skipped base=%1 wave=%2 package=%3 originalPrefab=%4 preferredPosition=%5 resolvedPosition=%6 placementSource=%7 skippedReason=waypoint_failed", m_sBaseName, m_iCurrentWave, packageIndex, prefab, infantryPackage.LastKnownPosition.ToString(), spawnPosition.ToString(), placementSource), true);
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

		RecordCurrentWaveInfantrySpawnPosition(spawnPosition);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Virtual wave infantry spawned base=%1 wave=%2 package=%3 originalPrefab=%4 preferredPosition=%5 resolvedPosition=%6 placementSource=%7 requestedUnits=%8", m_sBaseName, m_iCurrentWave, packageIndex, prefab, infantryPackage.LastKnownPosition.ToString(), spawnPosition.ToString(), placementSource, infantryPackage.UnitCount), true);
		return true;
	}

	protected bool SpawnVirtualWaveGroup(ResourceName preferredPrefab, vector defendTarget, int slotIndex, notnull array<SCR_AIGroup> groups, notnull array<IEntity> units, notnull array<IEntity> waypoints)
	{
		ResourceName prefab = preferredPrefab;
		if (IsEmptyResource(prefab))
			return false;

		vector spawnPosition = ResolveVirtualWaveInfantrySpawnPosition(defendTarget, slotIndex);
		LogGroupPrefabSelected("virtual_wave", m_iCurrentWave, slotIndex, prefab);
		SCR_AIGroup group = SpawnGroupPrefabAt(prefab, spawnPosition);
		if (!group)
			return false;

		array<IEntity> routeWaypoints = {};
		if (!AssignDefendWaypointToGroup(group, defendTarget, routeWaypoints))
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

		RecordCurrentWaveInfantrySpawnPosition(spawnPosition);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Virtual wave infantry spawned base=%1 wave=%2 slot=%3 pos=%4", m_sBaseName, m_iCurrentWave, slotIndex, spawnPosition.ToString()), true);
		return true;
	}

	protected SCR_AIGroup SpawnGroupPrefabAt(ResourceName groupPrefab, vector position, int requestedMembers = -1)
	{
		Resource resource = Resource.Load(groupPrefab);
		if (!resource || !resource.IsValid())
		{
			LogDebug(string.Format("Spawn failed: group prefab resource load failed prefab=%1", groupPrefab));
			return null;
		}

		IEntity owner = GetSelectedSpawnOwner();
		if (!owner || !GetGame() || !GetGame().GetWorld())
			return null;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		owner.GetTransform(spawnParams.Transform);
		spawnParams.Transform[3] = GroundPosition(position);

		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams));
		if (!group)
			return null;

		if (requestedMembers > 0)
			group.SetNumberOfMembersToSpawn(requestedMembers);

		if (!group.GetSpawnImmediately())
			group.SpawnUnits();

		JLH_DCF_PersistenceExclusion.StopTrackingTree(group, "qrf_virtual_group_spawn");
		//SetMembersAlive(0);
		SetIsSpawned(false);
		SetIsPaused(true);
		return group;
	}

	protected vector ResolveVirtualWaveInfantrySpawnPosition(vector defendTarget, int slotIndex)
	{
		int directionOffset = Math.RandomInt(0, 8);
		for (int ring = 0; ring < 4; ring++)
		{
			float radius = QRF_VIRTUAL_WAVE_INFANTRY_RADIUS_METERS + (ring * QRF_VIRTUAL_WAVE_INFANTRY_RING_STEP_METERS);
			for (int step = 0; step < 8; step++)
			{
				int directionIndex = directionOffset + slotIndex + step + (ring * 3);
				vector candidate = GroundPosition(defendTarget + (GetVehicleHoldDirection(directionIndex) * radius));
				if (IsVirtualWaveInfantrySpawnPositionClear(candidate))
					return candidate;
			}
		}

		return GroundPosition(defendTarget + (GetVehicleHoldDirection(directionOffset + slotIndex) * QRF_VIRTUAL_WAVE_INFANTRY_RADIUS_METERS));
	}

	protected bool IsVirtualWaveInfantrySpawnPositionClear(vector candidate)
	{
		string placementReason;
		if (!JLH_DCF_GhostPlacement.ValidateInfantryPosition(candidate, m_aCurrentWaveInfantrySpawnPositions, QRF_VIRTUAL_WAVE_INFANTRY_SPACING_METERS, QRF_VIRTUAL_WAVE_SAFE_SPAWN_DISTANCE, placementReason))
			return false;

		foreach (vector plannedPosition : m_aCurrentWaveInfantrySpawnPositions)
		{
			if (vector.Distance(candidate, plannedPosition) < QRF_VIRTUAL_WAVE_INFANTRY_SPACING_METERS)
				return false;
		}

		ref array<IEntity> players = {};
		GatherProximityPlayers(players);
		if (IsAnyValidPlayerWithinPositionWithPlayers(candidate, QRF_VIRTUAL_WAVE_SAFE_SPAWN_DISTANCE, players) && HasAnyValidPlayerLineOfSightWithPlayers(candidate, QRF_VIRTUAL_WAVE_SAFE_SPAWN_DISTANCE, players))
			return false;

		return true;
	}

	protected int SpawnVirtualWaveVehiclePackages(vector defendTarget, notnull array<SCR_AIGroup> spawnedGroups, notnull array<IEntity> spawnedUnits, notnull array<IEntity> spawnedVehicles, notnull array<IEntity> spawnedWaypoints, notnull array<ref JLH_DCF_GhostVehicleDefencePackage> failedVehiclePackages)
	{
		int spawnedCount = 0;
		foreach (JLH_DCF_GhostVehicleDefencePackage vehiclePackage : m_aVirtualWaveVehiclePackages)
		{
			if (!vehiclePackage || vehiclePackage.CrewCount <= 0)
			{
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("vehicle_package_removed base=%1 wave=%2 reason=invalid_package", m_sBaseName, m_iCurrentWave), true);
				continue;
			}

			if (IsEmptyResource(vehiclePackage.VehiclePrefab) || IsEmptyResource(vehiclePackage.CrewPrefab))
			{
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("vehicle_package_removed base=%1 wave=%2 vehicle=%3 reason=resource_missing", m_sBaseName, m_iCurrentWave, vehiclePackage.VehiclePrefab), true);
				continue;
			}

			if (SpawnVirtualWaveVehiclePackage(vehiclePackage, defendTarget, spawnedGroups, spawnedUnits, spawnedVehicles, spawnedWaypoints))
			{
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Virtual wave vehicle package activated base=%1 wave=%2 vehicle=%3 crew=%4", m_sBaseName, m_iCurrentWave, vehiclePackage.VehiclePrefab, vehiclePackage.CrewCount), true);
				spawnedCount++;
			}
			else
			{
				failedVehiclePackages.Insert(vehiclePackage);
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("vehicle_package_skipped base=%1 wave=%2 vehicle=%3 crew=%4 reason=restore_failed retained=true", m_sBaseName, m_iCurrentWave, vehiclePackage.VehiclePrefab, vehiclePackage.CrewCount), true);
			}
		}

		return spawnedCount;
	}

	protected bool SpawnVirtualWaveVehiclePackage(JLH_DCF_GhostVehicleDefencePackage vehiclePackage, vector defendTarget, notnull array<SCR_AIGroup> groups, notnull array<IEntity> units, notnull array<IEntity> vehicles, notnull array<IEntity> waypoints)
	{
		if (!vehiclePackage || IsEmptyResource(vehiclePackage.VehiclePrefab) || IsEmptyResource(vehiclePackage.CrewPrefab))
			return false;

		vector preferredPosition = vehiclePackage.DefencePosition;
		if (vehiclePackage.HasLastSafePosition)
			preferredPosition = vehiclePackage.LastSafePosition;

		vector spawnPosition;
		string placementSource;
		string skippedReason;
		string placementContext = string.Format("base=%1 wave=%2 originalPrefab=%3 preferredPosition=%4", m_sBaseName, m_iCurrentWave, vehiclePackage.VehiclePrefab, preferredPosition.ToString());
		if (!ResolveGhostVehiclePackagePosition(preferredPosition, vehiclePackage.HasLastSafePosition || vector.Distance(vehiclePackage.DefencePosition, "0 0 0") >= 0.1, defendTarget, vehicles.Count() + 1, spawnPosition, placementSource, skippedReason, placementContext))
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "vehicle_package_skipped " + placementContext + " reason=no_safe_position", true);
			return false;
		}

		IEntity vehicle = SpawnVehicleAtGhostPackage(vehiclePackage.VehiclePrefab, spawnPosition, vehiclePackage);
		if (!vehicle)
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Virtual wave vehicle package skipped base=%1 wave=%2 originalPrefab=%3 preferredPosition=%4 resolvedPosition=%5 placementSource=%6 skippedReason=spawn_failed", m_sBaseName, m_iCurrentWave, vehiclePackage.VehiclePrefab, preferredPosition.ToString(), spawnPosition.ToString(), placementSource), true);
			return false;
		}

		SCR_AIGroup crewGroup = SpawnVehiclePassengerGroupAt(vehiclePackage.CrewPrefab, JLH_DCF_GhostPlacement.GroundPosition(spawnPosition + Vector(2, 0, 0)));
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

		InsertRuntimeRouteWaypoints(routeWaypoints);
		m_aCurrentWaveVehicleSpawnPositions.Insert(spawnPosition);
		m_aCurrentWaveVehicleHoldPositions.Insert(spawnPosition);
		JLH_DCF_AssaultVehicleHoldRegistry.Reserve(spawnPosition);

		AddUniqueGroup(crewGroup, groups);
		AddUniqueEntity(vehicle, vehicles);
		CollectGroupUnitsToArray(crewGroup, units);
		foreach (IEntity waypoint : routeWaypoints)
		{
			AddUniqueEntity(waypoint, waypoints);
		}

		if (m_RuntimeTracker)
		{
			m_RuntimeTracker.TrackGroup(crewGroup);
			m_RuntimeTracker.TrackVehicle(vehicle);
		}

		JLH_DCF_QRFVehicleSupportRuntime support = new JLH_DCF_QRFVehicleSupportRuntime();
		support.Init(vehicle, crewGroup, null, null, vehiclePackage.VehiclePrefab, vehiclePackage.CrewPrefab, routeWaypoints, false, spawnPosition);
		support.IntendedCrewCount = vehiclePackage.CrewCount;
		support.TransportCapCenter = defendTarget;
		support.MountPending = true;
		support.MountReadyTick = System.GetTickCount() + VEHICLE_MOUNT_RETRY_DELAY_MS;
		support.WaveIndex = m_iCurrentWave;
		support.LastHealthScaled = GetEntityHealthScaled(vehicle);
		support.LastPosition = vehicle.GetOrigin();
		support.LastPositionValid = true;
		JLH_DCF_VehicleSustainment.RegisterVehicle(SYSTEM_NAME, vehicle, crewGroup, m_sFactionKey, m_sBaseName);
		support.ArmedMountedDefence = true;

		TrackCurrentWaveVehicleSupport(support);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Virtual wave vehicle package spawned base=%1 wave=%2 originalPrefab=%3 preferredPosition=%4 resolvedPosition=%5 placementSource=%6 crew=%7", m_sBaseName, m_iCurrentWave, vehiclePackage.VehiclePrefab, preferredPosition.ToString(), spawnPosition.ToString(), placementSource, vehiclePackage.CrewCount), true);
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
		if (!JLH_DCF_GhostPlacement.ValidateVehiclePosition(candidate, m_aCurrentWaveVehicleSpawnPositions, VEHICLE_SUPPORT_HOLD_SPACING_METERS, QRF_VIRTUAL_WAVE_SAFE_SPAWN_DISTANCE, reason))
			return false;

		if (trustLastKnown)
			return true;

		return IsVehicleSupportHoldPositionClear(candidate, defendTarget, reason);
	}

	protected void CollectQRFSurvivorState(notnull array<SCR_AIGroup> groups, notnull array<IEntity> units, notnull array<IEntity> vehicles, notnull array<IEntity> waypoints, notnull array<SCR_AIGroup> trackedGroups, notnull array<IEntity> trackedUnits, notnull array<IEntity> trackedVehicles, notnull array<ref JLH_DCF_GhostVehicleDefencePackage> vehiclePackages, string lockReleaseReason)
	{
		RefreshCurrentWaveUnits();
		RefreshCurrentWaveVehicles();

		ref array<SCR_AIGroup> packagedGroups = {};
		ref array<IEntity> packagedUnits = {};
		ref array<IEntity> packagedVehicles = {};
		ref array<IEntity> packagedWaypoints = {};

		foreach (JLH_DCF_QRFVehicleSupportRuntime support : m_aCurrentWaveVehicleSupports)
		{
			TryAddArmedVehicleGhostPackage(support, vehiclePackages, packagedGroups, packagedUnits, packagedVehicles, packagedWaypoints);
		}

		foreach (JLH_DCF_QRFVehicleSupportRuntime completedSupport : m_aVehicleSupportsAwaitingCleanup)
		{
			TryAddArmedVehicleGhostPackage(completedSupport, vehiclePackages, packagedGroups, packagedUnits, packagedVehicles, packagedWaypoints);
		}

		foreach (SCR_AIGroup group : m_aCurrentWaveGroups)
		{
			if (packagedGroups.Contains(group))
				continue;

			AddUniqueGroup(group, groups);
		}

		foreach (IEntity unit : m_aCurrentWaveUnits)
		{
			if (packagedUnits.Contains(unit))
				continue;

			AddUniqueEntity(unit, units);
		}

		foreach (IEntity vehicle : m_aCurrentWaveVehicles)
		{
			if (packagedVehicles.Contains(vehicle))
				continue;

			AddUniqueEntity(vehicle, vehicles);
		}

		if (m_RuntimeTracker)
		{
			m_RuntimeTracker.CollectTrackedState(trackedGroups, trackedUnits, trackedVehicles);
			AddTrackedSurvivorsForGhostStore(trackedGroups, trackedUnits, trackedVehicles, packagedGroups, packagedUnits, packagedVehicles, groups, units, vehicles);
		}

		foreach (JLH_DCF_QRFVehicleSupportRuntime support : m_aRetainedTransportSupports)
		{
			if (!support)
				continue;

			if (TryAddArmedVehicleGhostPackage(support, vehiclePackages, packagedGroups, packagedUnits, packagedVehicles, packagedWaypoints))
				continue;

			CollectVehicleSupportCrewUnits(support);
			ReleaseTransportLockForSupport(support, lockReleaseReason);
			AddUniqueGroup(support.CrewGroup, groups);
			AddUniqueGroup(support.TransportPassengerGroup, groups);
			AddUniqueEntity(support.DriverEntity, units);
			AddUniqueEntity(support.GunnerEntity, units);
			AddUniqueEntity(support.VehicleEntity, vehicles);

			foreach (IEntity crewUnit : support.VehicleCrewUnits)
			{
				AddUniqueEntity(crewUnit, units);
			}

			foreach (IEntity passengerUnit : support.TransportPassengerUnits)
			{
				AddUniqueEntity(passengerUnit, units);
			}

			foreach (IEntity waypoint : support.RouteWaypoints)
			{
				AddUniqueEntity(waypoint, waypoints);
			}
		}

		foreach (IEntity runtimeWaypoint : m_aRuntimeRouteWaypoints)
		{
			if (packagedWaypoints.Contains(runtimeWaypoint))
				continue;

			AddUniqueEntity(runtimeWaypoint, waypoints);
		}
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

	protected bool TryAddArmedVehicleGhostPackage(JLH_DCF_QRFVehicleSupportRuntime support, notnull array<ref JLH_DCF_GhostVehicleDefencePackage> vehiclePackages, notnull array<SCR_AIGroup> packagedGroups, notnull array<IEntity> packagedUnits, notnull array<IEntity> packagedVehicles, notnull array<IEntity> packagedWaypoints)
	{
		if (!QRFKeepArmedVehiclesMounted || !support || support.DropPending || !support.CrewGroup || !support.VehicleEntity)
			return false;

		if (support.MountPending && (!support.ArmedMountedDefence || support.IntendedCrewCount <= 0))
			return false;

		if (packagedVehicles.Contains(support.VehicleEntity))
			return false;

		if (IsEmptyResource(support.VehiclePrefab) || IsEmptyResource(support.CrewPrefab))
			return false;

		if (!support.ArmedMountedDefence && !IsArmedVehicleSupportPrefab(support.VehiclePrefab))
			return false;

		if (IsEntityDestroyed(support.VehicleEntity))
			return false;

		CollectVehicleSupportCrewUnits(support);
		int crewCount = CountLivingVehicleSupportCrewUnits(support);
		if (support.IntendedCrewCount > 0)
		{
			if ((support.MountPending && crewCount <= 0) || crewCount > support.IntendedCrewCount)
				crewCount = support.IntendedCrewCount;
		}

		if (crewCount <= 0)
			return false;

		vector defencePosition = support.VehicleEntity.GetOrigin();
		if (support.LastPositionValid && vector.Distance(support.LastPosition, "0 0 0") >= 0.1)
			defencePosition = support.LastPosition;
		else if (vector.Distance(defencePosition, "0 0 0") < 0.1)
			defencePosition = support.DropPosition;

		JLH_DCF_GhostVehicleDefencePackage vehiclePackage = new JLH_DCF_GhostVehicleDefencePackage();
		vehiclePackage.Init(support.VehiclePrefab, support.CrewPrefab, defencePosition, crewCount);
		vehiclePackage.SetContext(ResolveGhostDefenceAnchorPosition(), "QRF");
		vehiclePackage.SetTransformFromEntity(support.VehicleEntity);
		vehiclePackage.SourceGroup = support.CrewGroup;
		vehiclePackage.SourceVehicle = support.VehicleEntity;

		AddUniqueGroup(support.CrewGroup, packagedGroups);
		AddUniqueEntity(support.VehicleEntity, packagedVehicles);
		int packagedCrewUnits = 0;
		foreach (IEntity crewUnit : support.VehicleCrewUnits)
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

		foreach (IEntity waypoint : support.RouteWaypoints)
		{
			AddUniqueEntity(waypoint, packagedWaypoints);
			AddUniqueEntity(waypoint, vehiclePackage.SourceWaypoints);
		}

		vehiclePackages.Insert(vehiclePackage);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("vehicle_package_stored base=%1 wave=%2 vehicle=%3 crew=%4 reason=survivor_refresh mountPending=%5", m_sBaseName, m_iCurrentWave, support.VehiclePrefab, crewCount, JLH_DCF_NodeDebug.BoolLabel(support.MountPending)), true);
		return true;
	}

	protected bool ShouldRetainVehicleSupportAsBaseDefence(JLH_DCF_QRFVehicleSupportRuntime support, string reason)
	{
		if (reason == "vehicle_destroyed" || reason == "crew_dead" || reason == "mount_failed")
			return false;

		return IsArmedVehicleSupportReadyForGhostDefence(support);
	}

	protected bool ShouldRetainVehicleSupportForGhostDefence(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!IsQRFSuccessOutcomeConfirmed())
			return false;

		return IsArmedVehicleSupportReadyForGhostDefence(support);
	}

	protected bool IsArmedVehicleSupportReadyForGhostDefence(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!QRFKeepArmedVehiclesMounted || !support || support.DropPending || !support.CrewGroup || !support.VehicleEntity)
			return false;

		if (support.MountPending && (!support.ArmedMountedDefence || support.IntendedCrewCount <= 0))
			return false;

		if (IsEmptyResource(support.VehiclePrefab) || IsEmptyResource(support.CrewPrefab))
			return false;

		if (!support.ArmedMountedDefence && !IsArmedVehicleSupportPrefab(support.VehiclePrefab))
			return false;

		if (IsEntityDestroyed(support.VehicleEntity))
			return false;

		CollectVehicleSupportCrewUnits(support);
		if (support.MountPending && support.IntendedCrewCount > 0)
			return true;

		return CountLivingVehicleSupportCrewUnits(support) > 0;
	}

	protected bool HasLiveHumanNearDefendTarget(float distanceLimit)
	{
		vector defendTarget;
		string failureReason;
		if (!ResolveDefendTarget(defendTarget, failureReason))
			return JLH_DCF_PlayerPresence.HasLiveHumanInWorld();

		return JLH_DCF_PlayerPresence.HasLiveHumanWithin(defendTarget, distanceLimit);
	}

	protected int CountLivingUnitsForVirtualWave(array<SCR_AIGroup> groups, array<IEntity> units)
	{
		ref array<IEntity> liveUnits = {};
		if (groups)
		{
			foreach (SCR_AIGroup group : groups)
			{
				if (!group)
					continue;

				array<AIAgent> agents = {};
				group.GetAgents(agents);
				foreach (AIAgent agent : agents)
				{
					if (!agent)
						continue;

					AddLivingUnitForVirtualWave(agent.GetControlledEntity(), liveUnits);
				}
			}
		}

		if (units)
		{
			foreach (IEntity unit : units)
			{
				AddLivingUnitForVirtualWave(unit, liveUnits);
			}
		}

		return liveUnits.Count();
	}

	protected void BuildInfantryPackagesForVirtualWave(ResourceName preferredGroupPrefab, array<SCR_AIGroup> groups, array<IEntity> units, notnull array<ref JLH_DCF_GhostInfantryDefencePackage> infantryPackages)
	{
		ref array<IEntity> packagedUnits = {};
		if (groups)
		{
			foreach (SCR_AIGroup group : groups)
			{
				if (!group)
					continue;

				ref array<IEntity> groupUnits = {};
				CollectLivingGroupUnitsForVirtualWave(group, groupUnits);
				if (groupUnits.Count() <= 0)
					continue;

				JLH_DCF_GhostInfantryDefencePackage infantryPackage = new JLH_DCF_GhostInfantryDefencePackage();
				infantryPackage.Init(ResolveGroupPrefabForVirtualPackage(group, preferredGroupPrefab), groupUnits.Count());
				infantryPackage.SetContext(ResolveGhostDefenceAnchorPosition(), "QRF");
				infantryPackage.SetLastKnownPosition(ResolveAverageEntityPositionForVirtualWave(groupUnits));
				infantryPackage.SourceGroup = group;
				foreach (IEntity groupUnit : groupUnits)
				{
					AddUniqueEntity(groupUnit, packagedUnits);
					AddUniqueEntity(groupUnit, infantryPackage.SourceUnits);
				}

				infantryPackages.Insert(infantryPackage);
			}
		}

		if (units)
		{
			foreach (IEntity unit : units)
			{
				if (!unit || packagedUnits.Contains(unit) || !IsUnitAlive(unit))
					continue;

				JLH_DCF_GhostInfantryDefencePackage loosePackage = new JLH_DCF_GhostInfantryDefencePackage();
				loosePackage.Init(preferredGroupPrefab, 1);
				loosePackage.SetContext(ResolveGhostDefenceAnchorPosition(), "QRF");
				loosePackage.SetLastKnownPosition(unit.GetOrigin());
				loosePackage.SourceUnits.Insert(unit);
				infantryPackages.Insert(loosePackage);
				packagedUnits.Insert(unit);
			}
		}
	}

	protected void CollectLivingGroupUnitsForVirtualWave(SCR_AIGroup group, notnull array<IEntity> liveUnits)
	{
		if (!group)
			return;

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			AddLivingUnitForVirtualWave(agent.GetControlledEntity(), liveUnits);
		}
	}

	protected ResourceName ResolveGroupPrefabForVirtualPackage(SCR_AIGroup group, ResourceName fallbackPrefab)
	{
		if (group)
		{
			EntityPrefabData prefabData = group.GetPrefabData();
			if (prefabData)
			{
				ResourceName prefabName = prefabData.GetPrefabName();
				if (!IsEmptyResource(prefabName))
					return prefabName;
			}
		}

		return fallbackPrefab;
	}

	protected vector ResolveAverageEntityPositionForVirtualWave(array<IEntity> entities)
	{
		if (!entities || entities.IsEmpty())
			return "0 0 0";

		vector sum = "0 0 0";
		int count = 0;
		foreach (IEntity entity : entities)
		{
			if (!entity)
				continue;

			sum = sum + entity.GetOrigin();
			count++;
		}

		if (count <= 0)
			return "0 0 0";

		return GroundPosition(sum * (1.0 / count));
	}

	protected vector ResolveGhostDefenceAnchorPosition()
	{
		IEntity owner = GetOwner();
		if (owner)
			return GroundPosition(owner.GetOrigin());

		if (m_BaseEntity)
			return GroundPosition(m_BaseEntity.GetOrigin());

		return "0 0 0";
	}

	protected void AddLivingUnitForVirtualWave(IEntity unit, notnull array<IEntity> liveUnits)
	{
		if (!unit || liveUnits.Contains(unit) || !IsUnitAlive(unit))
			return;

		liveUnits.Insert(unit);
	}

	protected int CountLivingUnitsInArrayForVirtualWave(array<IEntity> units)
	{
		if (!units)
			return 0;

		int count = 0;
		for (int i = units.Count() - 1; i >= 0; i--)
		{
			IEntity unit = units[i];
			if (!unit)
			{
				units.Remove(i);
				continue;
			}

			if (IsUnitAlive(unit))
				count++;
		}

		return count;
	}

	protected int CountVehiclePackageCrewForVirtualWave(array<ref JLH_DCF_GhostVehicleDefencePackage> vehiclePackages)
	{
		if (!QRFKeepArmedVehiclesMounted || !vehiclePackages)
			return 0;

		int count = 0;
		foreach (JLH_DCF_GhostVehicleDefencePackage vehiclePackage : vehiclePackages)
		{
			if (!vehiclePackage || vehiclePackage.CrewCount <= 0)
				continue;

			if (IsEmptyResource(vehiclePackage.VehiclePrefab) || IsEmptyResource(vehiclePackage.CrewPrefab))
				continue;

			count += vehiclePackage.CrewCount;
		}

		return count;
	}

	protected int CountInfantryPackageUnitsForVirtualWave(array<ref JLH_DCF_GhostInfantryDefencePackage> infantryPackages)
	{
		if (!infantryPackages)
			return 0;

		int count = 0;
		foreach (JLH_DCF_GhostInfantryDefencePackage infantryPackage : infantryPackages)
		{
			if (infantryPackage && infantryPackage.UnitCount > 0)
				count += infantryPackage.UnitCount;
		}

		return count;
	}

	protected void CopyInfantryPackagesForVirtualWave(array<ref JLH_DCF_GhostInfantryDefencePackage> source, notnull array<ref JLH_DCF_GhostInfantryDefencePackage> target)
	{
		if (!source)
			return;

		foreach (JLH_DCF_GhostInfantryDefencePackage infantryPackage : source)
		{
			if (!infantryPackage || infantryPackage.UnitCount <= 0)
				continue;

			JLH_DCF_GhostInfantryDefencePackage copy = new JLH_DCF_GhostInfantryDefencePackage();
			copy.Init(infantryPackage.GroupPrefab, infantryPackage.UnitCount);
			copy.SetContext(infantryPackage.DefenceAnchor, infantryPackage.SourceSystem);
			if (infantryPackage.HasLastKnownPosition)
				copy.SetLastKnownPosition(infantryPackage.LastKnownPosition);
			target.Insert(copy);
		}
	}

	protected void CopyVehiclePackagesForVirtualWave(array<ref JLH_DCF_GhostVehicleDefencePackage> source, notnull array<ref JLH_DCF_GhostVehicleDefencePackage> target)
	{
		if (!QRFKeepArmedVehiclesMounted || !source)
			return;

		foreach (JLH_DCF_GhostVehicleDefencePackage vehiclePackage : source)
		{
			if (!vehiclePackage || vehiclePackage.CrewCount <= 0)
				continue;

			JLH_DCF_GhostVehicleDefencePackage copy = CopyVehiclePackageForVirtualWave(vehiclePackage);
			if (copy)
				target.Insert(copy);
		}
	}

	protected JLH_DCF_GhostVehicleDefencePackage CopyVehiclePackageForVirtualWave(JLH_DCF_GhostVehicleDefencePackage vehiclePackage)
	{
		if (!vehiclePackage || vehiclePackage.CrewCount <= 0)
			return null;

		JLH_DCF_GhostVehicleDefencePackage copy = new JLH_DCF_GhostVehicleDefencePackage();
		copy.Init(vehiclePackage.VehiclePrefab, vehiclePackage.CrewPrefab, vehiclePackage.DefencePosition, vehiclePackage.CrewCount);
		copy.SetContext(vehiclePackage.DefenceAnchor, vehiclePackage.SourceSystem);
		if (vehiclePackage.HasLastSafePosition)
			copy.SetLastSafePosition(vehiclePackage.LastSafePosition);
		copy.HasOrientation = vehiclePackage.HasOrientation;
		copy.Transform0 = vehiclePackage.Transform0;
		copy.Transform1 = vehiclePackage.Transform1;
		copy.Transform2 = vehiclePackage.Transform2;
		return copy;
	}

	protected void MergePendingVirtualWaveVehiclePackages(notnull array<ref JLH_DCF_GhostVehicleDefencePackage> vehiclePackages, string reason)
	{
		if (m_aPendingVirtualWaveVehiclePackages.IsEmpty())
			return;

		int pendingBefore = m_aPendingVirtualWaveVehiclePackages.Count();
		int added = 0;
		foreach (JLH_DCF_GhostVehicleDefencePackage pendingPackage : m_aPendingVirtualWaveVehiclePackages)
		{
			if (!pendingPackage || pendingPackage.CrewCount <= 0)
				continue;

			if (HasSimilarVehiclePackage(vehiclePackages, pendingPackage))
				continue;

			JLH_DCF_GhostVehicleDefencePackage copy = CopyVehiclePackageForVirtualWave(pendingPackage);
			if (!copy)
				continue;

			vehiclePackages.Insert(copy);
			added++;
		}

		m_aPendingVirtualWaveVehiclePackages.Clear();
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("vehicle_package_refreshed base=%1 wave=%2 reason=%3 pendingBefore=%4 merged=%5 after=%6", m_sBaseName, m_iCurrentWave, reason, pendingBefore, added, vehiclePackages.Count()), true);
	}

	protected void RefreshPendingVirtualWaveVehiclePackages(array<ref JLH_DCF_GhostVehicleDefencePackage> failedVehiclePackages, string reason)
	{
		if (!failedVehiclePackages || failedVehiclePackages.IsEmpty())
			return;

		int before = m_aPendingVirtualWaveVehiclePackages.Count();
		int added = 0;
		foreach (JLH_DCF_GhostVehicleDefencePackage failedPackage : failedVehiclePackages)
		{
			if (!failedPackage || failedPackage.CrewCount <= 0)
				continue;

			if (IsEmptyResource(failedPackage.VehiclePrefab) || IsEmptyResource(failedPackage.CrewPrefab))
				continue;

			if (HasSimilarVehiclePackage(m_aPendingVirtualWaveVehiclePackages, failedPackage))
				continue;

			JLH_DCF_GhostVehicleDefencePackage copy = CopyVehiclePackageForVirtualWave(failedPackage);
			if (!copy)
				continue;

			m_aPendingVirtualWaveVehiclePackages.Insert(copy);
			added++;
		}

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("vehicle_package_refreshed base=%1 wave=%2 reason=%3 failed=%4 added=%5 pendingBefore=%6 pendingAfter=%7", m_sBaseName, m_iCurrentWave, reason, failedVehiclePackages.Count(), added, before, m_aPendingVirtualWaveVehiclePackages.Count()), true);
	}

	protected void ClearPendingVirtualWaveVehiclePackages(string reason)
	{
		if (m_aPendingVirtualWaveVehiclePackages.IsEmpty())
			return;

		int removed = m_aPendingVirtualWaveVehiclePackages.Count();
		m_aPendingVirtualWaveVehiclePackages.Clear();
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("vehicle_package_removed base=%1 wave=%2 reason=%3 pendingCleared=%4", m_sBaseName, m_iCurrentWave, reason, removed), true);
	}

	protected bool HasSimilarVehiclePackage(array<ref JLH_DCF_GhostVehicleDefencePackage> vehiclePackages, JLH_DCF_GhostVehicleDefencePackage candidate)
	{
		if (!vehiclePackages || !candidate)
			return false;

		vector candidatePosition = candidate.DefencePosition;
		if (candidate.HasLastSafePosition)
			candidatePosition = candidate.LastSafePosition;

		foreach (JLH_DCF_GhostVehicleDefencePackage vehiclePackage : vehiclePackages)
		{
			if (!vehiclePackage)
				continue;

			if (vehiclePackage.VehiclePrefab != candidate.VehiclePrefab || vehiclePackage.CrewPrefab != candidate.CrewPrefab)
				continue;

			vector packagePosition = vehiclePackage.DefencePosition;
			if (vehiclePackage.HasLastSafePosition)
				packagePosition = vehiclePackage.LastSafePosition;

			if (vector.Distance(packagePosition, candidatePosition) <= 10.0)
				return true;
		}

		return false;
	}

	protected void TrimUnitsArrayToCountForVirtualWave(notnull array<IEntity> units, int maxLivingUnits)
	{
		if (maxLivingUnits < 0)
			maxLivingUnits = 0;

		int livingSeen = 0;
		for (int i = units.Count() - 1; i >= 0; i--)
		{
			IEntity unit = units[i];
			if (!unit)
			{
				units.Remove(i);
				continue;
			}

			if (!IsUnitAlive(unit))
				continue;

			livingSeen++;
			if (livingSeen <= maxLivingUnits)
				continue;

			if (!JLH_DCF_PlayerPresence.IsPlayerControlledEntity(unit))
				SCR_EntityHelper.DeleteEntityAndChildren(unit);

			units.Remove(i);
		}
	}

	protected void CleanupVirtualizedWavePhysicalState(array<SCR_AIGroup> groups, array<IEntity> units, array<IEntity> vehicles, array<IEntity> waypoints, array<ref JLH_DCF_GhostVehicleDefencePackage> vehiclePackages)
	{
		CleanupVirtualizedWaveVehiclePackageSources(vehiclePackages);

		if (waypoints)
		{
			foreach (IEntity waypoint : waypoints)
			{
				if (waypoint)
					SCR_EntityHelper.DeleteEntityAndChildren(waypoint);
			}
		}

		if (units)
		{
			foreach (IEntity unit : units)
			{
				if (unit && !JLH_DCF_PlayerPresence.IsPlayerControlledEntity(unit))
					SCR_EntityHelper.DeleteEntityAndChildren(unit);
			}
		}

		if (groups)
		{
			foreach (SCR_AIGroup group : groups)
			{
				if (group)
					SCR_EntityHelper.DeleteEntityAndChildren(group);
			}
		}

		if (vehicles)
		{
			foreach (IEntity vehicle : vehicles)
			{
				if (vehicle)
					JLH_DCF_VehicleSustainment.UnregisterVehicle(vehicle, "virtualized");

				if (vehicle)
					SCR_EntityHelper.DeleteEntityAndChildren(vehicle);
			}
		}
	}

	protected void CleanupVirtualizedWaveVehiclePackageSources(array<ref JLH_DCF_GhostVehicleDefencePackage> vehiclePackages)
	{
		if (!vehiclePackages)
			return;

		foreach (JLH_DCF_GhostVehicleDefencePackage vehiclePackage : vehiclePackages)
		{
			if (!vehiclePackage)
				continue;

			foreach (IEntity waypoint : vehiclePackage.SourceWaypoints)
			{
				if (waypoint)
					SCR_EntityHelper.DeleteEntityAndChildren(waypoint);
			}

			foreach (IEntity unit : vehiclePackage.SourceUnits)
			{
				if (unit && !JLH_DCF_PlayerPresence.IsPlayerControlledEntity(unit))
					SCR_EntityHelper.DeleteEntityAndChildren(unit);
			}

			if (vehiclePackage.SourceGroup)
				SCR_EntityHelper.DeleteEntityAndChildren(vehiclePackage.SourceGroup);

			if (vehiclePackage.SourceVehicle)
			{
				JLH_DCF_VehicleSustainment.UnregisterVehicle(vehiclePackage.SourceVehicle, "virtualized");
				SCR_EntityHelper.DeleteEntityAndChildren(vehiclePackage.SourceVehicle);
			}

			vehiclePackage.SourceUnits.Clear();
			vehiclePackage.SourceWaypoints.Clear();
			vehiclePackage.SourceGroup = null;
			vehiclePackage.SourceVehicle = null;
		}
	}

	protected void ClearVirtualWaveState()
	{
		m_bCurrentWaveVirtualized = false;
		m_iVirtualWaveStoredCount = 0;
		m_iVirtualWaveInfantryCount = 0;
		m_iVirtualWaveNextActivationAttemptTick = 0;
		m_iVirtualWaveDiscoveryGraceUntilTick = 0;
		m_rVirtualWavePreferredGroupPrefab = ResourceName.Empty;
		m_aVirtualWaveInfantryPackages.Clear();
		m_aVirtualWaveVehiclePackages.Clear();
	}

	protected void StartCooldown()
	{
		int cooldownMs = SecondsToMs(CooldownSeconds);
		m_iLastCooldownRejectLogTick = 0;
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Cooldown started seconds=%1", CooldownSeconds), true);

		if (cooldownMs <= 0)
		{
			m_iNextAllowedTriggerTick = 0;
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "Cooldown complete", true);
			return;
		}

		m_iNextAllowedTriggerTick = System.GetTickCount() + cooldownMs;
	}

	protected void TickCooldown(int now)
	{
		if (m_iNextAllowedTriggerTick <= 0)
			return;

		if (now < m_iNextAllowedTriggerTick)
			return;

		m_iNextAllowedTriggerTick = 0;
		m_iLastCooldownRejectLogTick = 0;
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "Cooldown complete", true);
	}

	protected void LogActiveTriggerRejected(int now)
	{
		if (m_iLastActiveRejectLogTick > 0 && now - m_iLastActiveRejectLogTick < TRIGGER_REJECT_LOG_THROTTLE_MS)
			return;

		m_iLastActiveRejectLogTick = now;
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "Trigger rejected reason=active", true);
	}

	protected void LogCooldownTriggerRejected(int now)
	{
		if (m_iLastCooldownRejectLogTick > 0 && now - m_iLastCooldownRejectLogTick < TRIGGER_REJECT_LOG_THROTTLE_MS)
			return;

		m_iLastCooldownRejectLogTick = now;
		int remainingMs = m_iNextAllowedTriggerTick - now;
		if (remainingMs < 0)
			remainingMs = 0;

		float remainingSeconds = remainingMs * 0.001;
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Trigger rejected reason=cooldown remaining=%1", remainingSeconds), true);
	}

	protected int SpawnGroups(int groupCount, string reason)
	{
		IEntity owner = GetSelectedSpawnOwner();
		if (!RefreshNodeFaction(owner))
			return 0;

		vector defendTarget;
		string defendTargetFailureReason;
		if (!ResolveDefendTarget(defendTarget, defendTargetFailureReason))
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Defend assignment failed reason=%1", defendTargetFailureReason), true);
			return 0;
		}

		LogDebug(string.Format("Defend target resolved base=%1 source=%2 position=%3", m_sBaseName, m_sLastDefendTargetSource, defendTarget.ToString()));

		int waveIndex = m_iCurrentWave;
		if (!IsEmptyResource(GroupPrefabOverride))
			return SpawnOverrideGroups(groupCount, reason, owner, defendTarget, waveIndex);

		return SpawnRandomGroups(groupCount, reason, owner, defendTarget, waveIndex);
	}

	protected int SpawnRandomGroups(int groupCount, string reason, IEntity owner, vector defendTarget, int waveIndex)
	{
		int spawned = 0;
		for (int i = 0; i < groupCount; i++)
		{
			int slotIndex = i + 1;
			ResourceName groupPrefab = ResolveRandomGroupPrefab();
			if (IsEmptyResource(groupPrefab))
			{
				LogDebug(string.Format("Spawn failed: random group prefab selection returned empty node=%1 reason=%2 wave=%3 slot=%4", JLH_DCF_NodeDebug.EntityLabel(owner), reason, waveIndex, slotIndex));
				continue;
			}

			if (SpawnSelectedGroupSlot(groupPrefab, "random", waveIndex, slotIndex, reason, owner, defendTarget))
				spawned++;
		}

		return spawned;
	}

	protected int SpawnOverrideGroups(int groupCount, string reason, IEntity owner, vector defendTarget, int waveIndex)
	{
		int spawned = 0;
		for (int i = 0; i < groupCount; i++)
		{
			if (SpawnSelectedGroupSlot(GroupPrefabOverride, "override", waveIndex, i + 1, reason, owner, defendTarget))
				spawned++;
		}

		return spawned;
	}

	protected bool SpawnSelectedGroupSlot(ResourceName groupPrefab, string mode, int waveIndex, int slotIndex, string reason, IEntity owner, vector defendTarget)
	{
		LogGroupPrefabSelected(mode, waveIndex, slotIndex, groupPrefab);

		SCR_AIGroup spawnedGroup = SpawnGroupPrefab(groupPrefab, owner);
		if (!spawnedGroup)
		{
			LogDebug(string.Format("Spawn failed: group prefab spawn returned no group node=%1 reason=%2 mode=%3 wave=%4 slot=%5 prefab=%6", JLH_DCF_NodeDebug.EntityLabel(owner), reason, mode, waveIndex, slotIndex, groupPrefab));
			return false;
		}

		array<IEntity> routeWaypoints = {};
		if (!AssignDefendWaypointToGroup(spawnedGroup, defendTarget, routeWaypoints))
		{
			CleanupRouteWaypoints(spawnedGroup, routeWaypoints);
			DeleteGroupAndMembers(spawnedGroup);
			LogDebug(string.Format("Spawn failed: group prefab defend assignment failed node=%1 reason=%2 mode=%3 wave=%4 slot=%5 prefab=%6", JLH_DCF_NodeDebug.EntityLabel(owner), reason, mode, waveIndex, slotIndex, groupPrefab));
			return false;
		}

		foreach (IEntity waypointEntity : routeWaypoints)
		{
			if (waypointEntity)
				m_aRuntimeRouteWaypoints.Insert(waypointEntity);
		}

		ActivateGroupAI(spawnedGroup);

		if (m_RuntimeTracker)
			m_RuntimeTracker.TrackGroup(spawnedGroup);

		RecordCurrentWaveInfantrySpawnPosition(owner.GetOrigin());
		TrackCurrentWaveGroup(spawnedGroup);
		return true;
	}

	protected ResourceName ResolveRandomGroupPrefab()
	{
		ResourceName resolverPrefab;
		if (JLH_DCF_DEVEnemyAssetResolver.TryResolveEnemyGroupPrefabByRole(JLH_EnemyAssetRole.QRF_ASSAULT, resolverPrefab))
			return resolverPrefab;

		SCR_EntityCatalog catalog = ResolveFactionCatalog(EEntityCatalogType.GROUP);
		if (!catalog)
			return ResourceName.Empty;

		array<SCR_EntityCatalogEntry> entries = {};
		catalog.GetEntityList(entries);
		if (entries.IsEmpty())
			return ResourceName.Empty;

		array<ResourceName> candidates = {};
		foreach (SCR_EntityCatalogEntry entry : entries)
		{
			ResourceName prefab;
			if (!IsRandomGroupCatalogEntryAllowed(entry, prefab))
				continue;

			candidates.Insert(prefab);
		}

		if (candidates.IsEmpty())
			return ResourceName.Empty;

		int selectedIndex = Math.RandomInt(0, candidates.Count());
		return candidates[selectedIndex];
	}

	protected bool IsRandomGroupCatalogEntryAllowed(SCR_EntityCatalogEntry entry, out ResourceName prefab)
	{
		prefab = ResourceName.Empty;
		if (!entry || !entry.IsEnabled())
			return false;

		prefab = entry.GetPrefab();
		if (IsEmptyResource(prefab))
			return false;

		if (IsUnsupportedRandomGroupPrefab(prefab))
			return false;

		if (!JLH_AddonSpawnUtility.CanLoadPrefab(prefab))
			return false;

		return true;
	}

	protected bool IsUnsupportedRandomGroupPrefab(ResourceName prefab)
	{
		if (IsEmptyResource(prefab))
			return false;

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
		if (token.Contains("ambulance")) return true;
		if (token.Contains("supply")) return true;
		if (token.Contains("repair")) return true;
		if (token.Contains("fuel")) return true;
		if (token.Contains("ammo")) return true;
		if (token.Contains("arsenal")) return true;
		if (token.Contains("crew")) return true;
		if (token.Contains("pilot")) return true;

		return false;
	}

	protected void LogGroupPrefabSelected(string mode, int waveIndex, int slotIndex, ResourceName prefab)
	{
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Group prefab selected mode=%1 wave=%2 slot=%3 prefab=%4", mode, waveIndex, slotIndex, prefab), true);
	}

	protected SCR_AIGroup SpawnGroupPrefab(ResourceName groupPrefab, IEntity owner)
	{
		Resource resource = Resource.Load(groupPrefab);
		if (!resource || !resource.IsValid())
		{
			LogDebug(string.Format("Spawn failed: group prefab resource load failed prefab=%1", groupPrefab));
			return null;
		}

		if (!owner || !GetGame() || !GetGame().GetWorld())
			return null;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		owner.GetTransform(spawnParams.Transform);

		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams));
		if (!group)
			return null;

		if (!group.GetSpawnImmediately())
			group.SpawnUnits();

		JLH_DCF_PersistenceExclusion.StopTrackingTree(group, "qrf_group_spawn");
		//SetMembersAlive(0);
		SetIsSpawned(false);
		SetIsPaused(true);
		return group;
	}

	protected int SpawnWaveVehicleSupport(int waveIndex)
	{
		if (!m_bEffectiveVehicleCountsPerWave && m_iEffectiveAddVehicleToWave > m_iEffectiveMaxWaves)
		{
			if (!m_bVehicleSupportWaveWarningLogged)
			{
				m_bVehicleSupportWaveWarningLogged = true;
				JLH_DCF_NodeDebug.Warn(SYSTEM_NAME, string.Format("Vehicle support ignored reason=wave_exceeds_max addVehicleToWave=%1 maxWaves=%2 node=%3", m_iEffectiveAddVehicleToWave, m_iEffectiveMaxWaves, JLH_DCF_NodeDebug.EntityLabel(GetOwner())));
			}

			return 0;
		}

		int requestedCount = GetEffectiveVehicleCountForWave(waveIndex);
		if (requestedCount <= 0)
			return 0;

		vector defendTarget;
		string defendTargetFailureReason;
		if (!ResolveDefendTarget(defendTarget, defendTargetFailureReason))
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Defend assignment failed reason=%1", defendTargetFailureReason), true);
			return 0;
		}

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle support wave=%1 mode=%2 count=%3", waveIndex, GetVehicleSupportModeLabel(), requestedCount), true);

		ScheduleVehicleSupportSpawn(waveIndex, requestedCount, defendTarget);
		return 0;
	}

	protected void ScheduleVehicleSupportSpawn(int waveIndex, int requestedCount, vector defendTarget)
	{
		if (requestedCount <= 0)
			return;

		int delayMs = GetVehicleSpawnDelayMs();
		m_bVehicleSupportSpawnPending = true;
		m_iPendingVehicleSupportWave = waveIndex;
		m_iPendingVehicleSupportCount = requestedCount;
		m_iPendingVehicleSupportReadyTick = System.GetTickCount() + delayMs;
		m_vPendingVehicleSupportDefendTarget = defendTarget;
		m_bCurrentWaveHadVehicleSupport = true;

		float delaySeconds = delayMs * 0.001;
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("vehicle spawn delayed base=%1 wave=%2 delay=%3", m_sBaseName, waveIndex, delaySeconds), true);
	}

	protected void TickPendingVehicleSupport(int now)
	{
		if (!m_bVehicleSupportSpawnPending)
			return;

		if (now < m_iPendingVehicleSupportReadyTick)
			return;

		int waveIndex = m_iPendingVehicleSupportWave;
		int requestedCount = m_iPendingVehicleSupportCount;
		vector defendTarget = m_vPendingVehicleSupportDefendTarget;
		m_bVehicleSupportSpawnPending = false;
		m_iPendingVehicleSupportWave = 0;
		m_iPendingVehicleSupportCount = 0;
		m_iPendingVehicleSupportReadyTick = 0;
		m_vPendingVehicleSupportDefendTarget = "0 0 0";

		RefreshCurrentWaveUnits();
		int liveInfantry = CountLivingCurrentWaveUnits();
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("vehicle delay elapsed liveInfantry=%1 wave=%2 base=%3", liveInfantry, waveIndex, m_sBaseName), true);

		int spawned = 0;
		m_iPendingVehicleSupportIntentionalSkips = 0;
		for (int i = 0; i < requestedCount; i++)
		{
			if (SpawnQRFVehicleSupport(waveIndex, i, defendTarget))
				spawned++;
		}

		if (spawned > 0)
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("vehicle spawned after delay wave=%1 count=%2 base=%3", waveIndex, spawned, m_sBaseName), true);
		else if (m_iPendingVehicleSupportIntentionalSkips > 0)
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "transport skip handled as intentional", true);
		else
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("vehicle spawn skipped reason=no_support_spawned wave=%1 base=%2", waveIndex, m_sBaseName), true);
	}

	protected bool SpawnQRFVehicleSupport(int waveIndex, int vehicleIndex, vector defendTarget)
	{
		bool dropPending = m_bEffectivePassengerDelivery;
		ResourceName vehiclePrefab = ResolveQRFVehiclePrefab(dropPending);
		if (IsEmptyResource(vehiclePrefab))
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle support skipped reason=vehicle_prefab_missing wave=%1", waveIndex), true);
			return false;
		}

		bool directSeatMode = !dropPending && (QRFKeepArmedVehiclesMounted || IsArmedVehicleSupportPrefab(vehiclePrefab));
		if (directSeatMode)
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "vehicle mode=ARMED_SUPPORT", true);
		else
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "vehicle mode=TRANSPORT_DROP", true);

		ResourceName armedSupportGroupPrefab = ResourceName.Empty;
		ResourceName passengerGroupPrefab = ResourceName.Empty;
		if (directSeatMode)
		{
			armedSupportGroupPrefab = ResolveArmedVehicleCrewGroupPrefab();
			if (IsEmptyResource(armedSupportGroupPrefab))
			{
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle support skipped reason=vehicle_group_prefab_missing wave=%1 faction=%2", waveIndex, m_sFactionKey), true);
				return false;
			}
		}
		else
		{
			passengerGroupPrefab = ResolveVehiclePassengerGroupPrefab();
			if (IsEmptyResource(passengerGroupPrefab))
			{
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle support skipped reason=passenger_group_prefab_missing wave=%1 faction=%2", waveIndex, m_sFactionKey), true);
				return false;
			}
		}

		vector spawnPosition = ResolveVehicleSpawnPosition(vehicleIndex);
		string positionReason;
		if (!ValidateDelayedVehicleSpawnPosition(spawnPosition, positionReason))
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("vehicle spawn skipped reason=%1 wave=%2", positionReason, waveIndex), true);
			return false;
		}

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("spawn position validated=true pos=%1", spawnPosition.ToString()), true);

		string spawnSafetyReason;
		if (!IsVehicleSpawnVisibilitySafe(spawnPosition, spawnSafetyReason))
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle support skipped reason=spawn_visibility_failed safety=%1 wave=%2", spawnSafetyReason, waveIndex), true);
			return false;
		}

		vector vehicleTarget = "0 0 0";
		vector dropPosition = "0 0 0";
		bool vehicleTargetResolved = false;
		string transportLockToken = "";
		if (dropPending)
		{
			transportLockToken = BuildTransportLockToken(waveIndex, vehicleIndex);
			if (!JLH_DCF_BaseTransportLock.TryAcquire(m_sBaseName, transportLockToken))
			{
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport skipped reason=base_transport_active base=%1", m_sBaseName), true);
				m_iPendingVehicleSupportIntentionalSkips++;
				return false;
			}

			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport accepted base=%1 activeTransport=1", m_sBaseName), true);

			vehicleTarget = ResolveTransportVehicleSupportDropPosition(defendTarget, spawnPosition, vehicleIndex);
			dropPosition = vehicleTarget;
			vehicleTargetResolved = true;
		}

		IEntity vehicle = SpawnVehicleAt(vehiclePrefab, spawnPosition);
		if (!vehicle)
		{
			if (dropPending)
				ReleaseTransportLock(m_sBaseName, transportLockToken, "vehicle_spawn_failed");

			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle support skipped reason=vehicle_spawn_failed wave=%1 prefab=%2", waveIndex, vehiclePrefab), true);
			return false;
		}

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle spawned base=%1 node=%2 wave=%3 slot=%4 vehicle=%5 prefab=%6 pos=%7", m_sBaseName, JLH_DCF_NodeDebug.EntityLabel(GetOwner()), waveIndex, vehicleIndex + 1, JLH_DCF_NodeDebug.EntityLabel(vehicle), vehiclePrefab, vehicle.GetOrigin().ToString()), true);

		SCR_AIGroup crewGroup;
		JLH_DCF_VehicleMountStats mountStats = null;
		if (directSeatMode)
			crewGroup = SpawnVehiclePassengerGroupAt(armedSupportGroupPrefab, spawnPosition + Vector(2, 0, 0));
		else
			crewGroup = SpawnVehiclePassengerGroupAt(passengerGroupPrefab, spawnPosition + Vector(2, 0, 0));

		if (!crewGroup)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(vehicle);
			if (dropPending)
				ReleaseTransportLock(m_sBaseName, transportLockToken, "crew_group_spawn_failed");

			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle support skipped reason=crew_group_spawn_failed wave=%1 vehiclePrefab=%2", waveIndex, vehiclePrefab), true);
			return false;
		}

		if (directSeatMode)
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("crew method=vehiclepatrol_dedicated_vehicle_group base=%1 node=%2 wave=%3 slot=%4 vehicle=%5 crewGroup=%6", m_sBaseName, JLH_DCF_NodeDebug.EntityLabel(GetOwner()), waveIndex, vehicleIndex + 1, JLH_DCF_NodeDebug.EntityLabel(vehicle), JLH_DCF_NodeDebug.EntityLabel(crewGroup)), true);
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("crew group prefab=%1 base=%2 wave=%3 slot=%4", armedSupportGroupPrefab, m_sBaseName, waveIndex, vehicleIndex + 1), true);
			LogMountCandidateState(crewGroup, vehicle, "spawned");
		}
		else
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle passenger support preserved mode=%1 prefab=%2", GetVehicleSupportModeLabel(), passengerGroupPrefab), true);
		}

		if (!RegisterUsableVehicle(crewGroup, vehicle))
		{
			DeleteGroupAndMembers(crewGroup);
			SCR_EntityHelper.DeleteEntityAndChildren(vehicle);
			if (dropPending)
				ReleaseTransportLock(m_sBaseName, transportLockToken, "vehicle_usage_register_failed");

			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle support skipped reason=vehicle_usage_register_failed wave=%1 prefab=%2", waveIndex, vehiclePrefab), true);
			return false;
		}

		array<IEntity> routeWaypoints = {};
		if (!vehicleTargetResolved)
			vehicleTarget = ResolveVehicleSupportHoldPosition(defendTarget, vehicleIndex);

		if (!dropPending)
			dropPosition = vehicleTarget;

		bool assigned = true;
		if (!dropPending)
		{
			assigned = AssignMountedVehicleHoldWaypointToGroup(crewGroup, vehicleTarget, routeWaypoints);
		}

		if (!assigned)
		{
			CleanupRouteWaypoints(crewGroup, routeWaypoints);
			DeleteGroupAndMembers(crewGroup);
			SCR_EntityHelper.DeleteEntityAndChildren(vehicle);
			if (dropPending)
				ReleaseTransportLock(m_sBaseName, transportLockToken, "waypoint_assignment_failed");

			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle support skipped reason=waypoint_assignment_failed wave=%1 prefab=%2", waveIndex, vehiclePrefab), true);
			return false;
		}

		InsertRuntimeRouteWaypoints(routeWaypoints);
		m_aCurrentWaveVehicleSpawnPositions.Insert(spawnPosition);

		if (m_RuntimeTracker)
		{
			m_RuntimeTracker.TrackGroup(crewGroup);
			m_RuntimeTracker.TrackVehicle(vehicle);
		}

		JLH_DCF_QRFVehicleSupportRuntime support = new JLH_DCF_QRFVehicleSupportRuntime();
		ResourceName supportCrewPrefab = passengerGroupPrefab;
		IEntity driverEntity = null;
		IEntity gunnerEntity = null;
		if (directSeatMode)
			supportCrewPrefab = armedSupportGroupPrefab;

		if (directSeatMode && mountStats)
		{
			driverEntity = mountStats.DriverEntity;
			gunnerEntity = mountStats.GunnerEntity;
		}

		support.Init(vehicle, crewGroup, driverEntity, gunnerEntity, vehiclePrefab, supportCrewPrefab, routeWaypoints, dropPending, vehicleTarget);
		support.TransportCapCenter = defendTarget;
		support.MountPending = true;
		if (support.MountPending)
		{
			if (directSeatMode)
				support.MountReadyTick = System.GetTickCount() + VEHICLE_MOUNT_RETRY_DELAY_MS;
			else
				support.MountReadyTick = System.GetTickCount() + PENDING_VEHICLE_MOUNT_DELAY_MS;
		}

		support.WaveIndex = waveIndex;
		support.VehicleIndex = vehicleIndex + 1;
		if (dropPending)
		{
			support.TransportBaseKey = m_sBaseName;
			support.TransportLockToken = transportLockToken;
		}

		support.LastHealthScaled = GetEntityHealthScaled(vehicle);
		support.LastPosition = vehicle.GetOrigin();
		support.LastPositionValid = true;
		JLH_DCF_VehicleSustainment.RegisterVehicle(SYSTEM_NAME, vehicle, crewGroup, m_sFactionKey, m_sBaseName);
		if (directSeatMode && !support.MountPending)
		{
			CollectVehicleSupportCrewUnits(support);
			support.LastCrewHealthScaled = GetVehicleSupportCrewHealthScaled(support);
			ActivateGroupAI(crewGroup);
		}

		TrackCurrentWaveVehicleSupport(support);
		return true;
	}

	protected ResourceName ResolveQRFVehiclePrefab(bool transportMode)
	{
		if (transportMode)
			return ResolveQRFTransportVehiclePrefab();

		return ResolveQRFArmedSupportVehiclePrefab();
	}

	protected ResourceName ResolveQRFArmedSupportVehiclePrefab()
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

		array<SCR_EntityCatalogEntry> candidates = {};
		array<SCR_EntityCatalogEntry> entries = {};
		catalog.GetEntityList(entries);
		foreach (SCR_EntityCatalogEntry entry : entries)
		{
			if (!entry || !entry.IsEnabled())
				continue;

			if (!MatchesVehicleTraitFilters(entry))
				continue;

			if (entry.GetPrefab() == "")
				continue;

			candidates.Insert(entry);
		}

		if (candidates.IsEmpty())
			return ResourceName.Empty;

		Math.Randomize(-1);
		SCR_EntityCatalogEntry selected = candidates.GetRandomElement();
		if (!selected)
			return ResourceName.Empty;

		return selected.GetPrefab();
	}

	protected ResourceName ResolveQRFTransportVehiclePrefab()
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

	protected SCR_AIGroup SpawnEmptyVehicleCrewCommandGroup(ResourceName groupPrefab, vector position)
	{
		Resource resource = Resource.Load(groupPrefab);
		if (!resource || !resource.IsValid())
			return null;

		IEntity owner = GetSelectedSpawnOwner();
		if (!owner || !GetGame() || !GetGame().GetWorld())
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
		JLH_DCF_PersistenceExclusion.StopTrackingTree(group, "qrf_empty_vehicle_crew_group_spawn");
		return group;
	}

	protected SCR_AIGroup SpawnVehiclePassengerGroupAt(ResourceName groupPrefab, vector position)
	{
		Resource resource = Resource.Load(groupPrefab);
		if (!resource || !resource.IsValid())
			return null;

		IEntity owner = GetSelectedSpawnOwner();
		if (!owner || !GetGame() || !GetGame().GetWorld())
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
		JLH_DCF_PersistenceExclusion.StopTrackingTree(group, "qrf_vehicle_passenger_group_spawn");
		return group;
	}

	protected IEntity SpawnVehicleAt(ResourceName vehiclePrefab, vector position)
	{
		Resource resource = Resource.Load(vehiclePrefab);
		if (!resource || !resource.IsValid())
			return null;

		IEntity owner = GetSelectedSpawnOwner();
		if (!owner || !GetGame() || !GetGame().GetWorld())
			return null;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		owner.GetTransform(spawnParams.Transform);
		spawnParams.Transform[3] = position;

		IEntity vehicle = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams);
		JLH_DCF_PersistenceExclusion.StopTrackingTree(vehicle, "qrf_vehicle_spawn");
		return vehicle;
	}

	protected IEntity SpawnVehicleAtGhostPackage(ResourceName vehiclePrefab, vector position, JLH_DCF_GhostVehicleDefencePackage vehiclePackage)
	{
		Resource resource = Resource.Load(vehiclePrefab);
		if (!resource || !resource.IsValid())
			return null;

		IEntity owner = GetSelectedSpawnOwner();
		if (!owner || !GetGame() || !GetGame().GetWorld())
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
		JLH_DCF_PersistenceExclusion.StopTrackingTree(vehicle, "qrf_ghost_vehicle_package_spawn");
		return vehicle;
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
		int maxCargoSeats = GetMaxSupportCargoSeats(likelyArmed, passengerDeliveryMode);
		if (likelyArmed)
		{
			LogArmedSupportMountPolicy();
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
				else if (CanMountSupportVehicleSeat(likelyArmed, passengerDeliveryMode, stats) && access.MoveInVehicle(vehicle, ECompartmentType.TURRET))
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
						maxCargoSeats = GetMaxSupportCargoSeats(true, passengerDeliveryMode);
						if (!armedPolicyLogged)
						{
							LogArmedSupportMountPolicy();
							armedPolicyLogged = true;
						}
					}
				}
				else if (CanMountSupportCargoSeat(likelyArmed, passengerDeliveryMode, stats, maxCargoSeats) && access.MoveInVehicle(vehicle, ECompartmentType.CARGO))
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
			stats.RemovedUnassigned += PruneMountedSupportCrewOverLimit(group, vehicle, ARMED_SUPPORT_MAX_MOUNTED_CREW);

		int driverMounted = 0;
		if (stats.DriverMounted)
			driverMounted = 1;

		int gunnerMounted = 0;
		if (stats.GunnerMounted)
			gunnerMounted = 1;

		if (likelyArmed || stats.GunnerMounted || passengerDeliveryMode)
		{
			int commanderMounted = stats.CargoSeatsFilled;
			int passengersMounted = 0;
			if (passengerDeliveryMode)
			{
				commanderMounted = 0;
				passengersMounted = stats.CargoSeatsFilled;
			}

			string mountLog = string.Format("Mount complete base=%1 node=%2 vehicle=%3 vehiclePos=%4 driver=%5 gunner=%6 commander=%7", m_sBaseName, JLH_DCF_NodeDebug.EntityLabel(GetOwner()), JLH_DCF_NodeDebug.EntityLabel(vehicle), GetEntityPositionLabel(vehicle), driverMounted, gunnerMounted, commanderMounted);
			mountLog += string.Format(" passengers=%1 unused=%2", passengersMounted, stats.RemovedUnassigned);
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, mountLog, true);
			if (passengerDeliveryMode)
			{
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport group base=%1 node=%2 vehicle=%3 passengers=%4 driver=%5", m_sBaseName, JLH_DCF_NodeDebug.EntityLabel(GetOwner()), JLH_DCF_NodeDebug.EntityLabel(vehicle), TRANSPORT_PASSENGER_GROUP_SIZE, driverMounted), true);
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("passengerCapacity base=%1 node=%2 vehicle=%3 capacity=%4 passengerGroupSize=%5 passengersMounted=%6 overflowOnFoot=%7", m_sBaseName, JLH_DCF_NodeDebug.EntityLabel(GetOwner()), JLH_DCF_NodeDebug.EntityLabel(vehicle), TRANSPORT_TOTAL_CAPACITY, TRANSPORT_PASSENGER_GROUP_SIZE, passengersMounted, stats.RemovedUnassigned), true);
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport mounted base=%1 node=%2 vehicle=%3 driver=%4 passengers=%5 overflowOnFoot=%6", m_sBaseName, JLH_DCF_NodeDebug.EntityLabel(GetOwner()), JLH_DCF_NodeDebug.EntityLabel(vehicle), driverMounted, passengersMounted, stats.RemovedUnassigned), true);
			}
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

		LogDebug(string.Format("mount audit base=%1 node=%2 phase=%3 groupEntity=%4 aiGroup=%5 vehicle=%6 vehiclePos=%7 agentCount=%8 candidateCount=%9", m_sBaseName, JLH_DCF_NodeDebug.EntityLabel(GetOwner()), phase, JLH_DCF_NodeDebug.EntityLabel(group), JLH_DCF_NodeDebug.BoolLabel(group != null), JLH_DCF_NodeDebug.EntityLabel(vehicle), GetEntityPositionLabel(vehicle), agentCount, candidateCount));
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
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("mounted base=%1 node=%2 vehicle=%3 vehiclePos=%4 driver=%5 gunner=%6 commander=%7 passengers=0 deletedUnused=%8", m_sBaseName, JLH_DCF_NodeDebug.EntityLabel(GetOwner()), JLH_DCF_NodeDebug.EntityLabel(vehicle), GetEntityPositionLabel(vehicle), driverMounted, gunnerMounted, commanderMounted, removed), true);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("turret usable base=%1 node=%2 vehicle=%3 usable=%4", m_sBaseName, JLH_DCF_NodeDebug.EntityLabel(GetOwner()), JLH_DCF_NodeDebug.EntityLabel(vehicle), JLH_DCF_NodeDebug.BoolLabel(turretUsable)), true);
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

	protected int GetMaxSupportCargoSeats(bool likelyArmed, bool passengerDeliveryMode)
	{
		if (passengerDeliveryMode || !likelyArmed)
			return TRANSPORT_PASSENGER_GROUP_SIZE;

		int maxCargoSeats = ARMED_SUPPORT_MAX_MOUNTED_CREW - 2;
		if (maxCargoSeats < 0)
			return 0;

		return maxCargoSeats;
	}

	protected bool CanMountSupportCargoSeat(bool likelyArmed, bool passengerDeliveryMode, JLH_DCF_VehicleMountStats stats, int maxCargoSeats)
	{
		if (!stats || stats.CargoSeatsFilled >= maxCargoSeats)
			return false;

		return CanMountSupportVehicleSeat(likelyArmed, passengerDeliveryMode, stats);
	}

	protected bool CanMountSupportVehicleSeat(bool likelyArmed, bool passengerDeliveryMode, JLH_DCF_VehicleMountStats stats)
	{
		if (!stats)
			return false;

		if (passengerDeliveryMode || (!likelyArmed && !stats.GunnerMounted))
			return true;

		return stats.SeatsFilled < ARMED_SUPPORT_MAX_MOUNTED_CREW;
	}

	protected int PruneMountedSupportCrewOverLimit(SCR_AIGroup group, IEntity vehicle, int maxMountedCrew)
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

	protected void LogArmedSupportMountPolicy()
	{
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Armed support mount policy maxCrew=%1", ARMED_SUPPORT_MAX_MOUNTED_CREW), true);
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

	protected void TrackCurrentWaveVehicleSupport(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support)
			return;

		m_aCurrentWaveVehicleSupports.Insert(support);
		if (support.VehicleEntity && !m_aCurrentWaveVehicles.Contains(support.VehicleEntity))
			m_aCurrentWaveVehicles.Insert(support.VehicleEntity);

		TrackCurrentWaveGroup(support.CrewGroup);
		m_bCurrentWaveHadVehicleSupport = true;
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle support active wave=%1", support.WaveIndex), true);
	}

	protected void TickCurrentWaveVehicleSupport(int now)
	{
		for (int i = m_aCurrentWaveVehicleSupports.Count() - 1; i >= 0; i--)
		{
			JLH_DCF_QRFVehicleSupportRuntime support = m_aCurrentWaveVehicleSupports[i];
			if (!support)
			{
				m_aCurrentWaveVehicleSupports.Remove(i);
				continue;
			}

			if (support.MountPending)
			{
				if (now < support.MountReadyTick)
					continue;

				if (!FinalizeVehicleSupportMount(support))
				{
					if (ShouldRetryVehicleMount(support))
					{
						support.MountRetryCount++;
						support.MountReadyTick = now + VEHICLE_MOUNT_RETRY_DELAY_MS;
						if (support.DropPending)
							LogDebug(string.Format("Vehicle support transport mount retry scheduled wave=%1 retry=%2", support.WaveIndex, support.MountRetryCount));
						else
							JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle support mount retry scheduled %1 retry=%2", BuildVehicleSupportContext(support), support.MountRetryCount), true);
						continue;
					}

					JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle support mount final failure %1 reason=%2 retries=%3", BuildVehicleSupportContext(support), support.MountFailureReason, support.MountRetryCount), true);
					CollectVehicleSupportCrewUnits(support);
					MarkVehicleSupportComplete(support, "mount_failed", now);
					continue;
				}
			}

			if (support.DropPending && support.TransportMoveAssigned)
				continue;

			UpdateVehicleSupportBlockingState(support, now);
		}
	}

	protected bool FinalizeVehicleSupportMount(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support || !support.CrewGroup || !support.VehicleEntity)
			return false;

		JLH_DCF_PersistenceExclusion.StopTrackingTree(support.CrewGroup, "qrf_vehicle_support_mount_group");
		JLH_DCF_PersistenceExclusion.StopTrackingTree(support.VehicleEntity, "qrf_vehicle_support_mount_vehicle");

		JLH_DCF_VehicleMountStats stats = new JLH_DCF_VehicleMountStats();
		LogDebug(string.Format("Mount ready %1 crewSeen=%2", BuildVehicleSupportContext(support), CountGroupUnits(support.CrewGroup)));
		LogMountCandidateState(support.CrewGroup, support.VehicleEntity, "mount_attempt");
		bool passengerDeliveryMode = support.DropPending;
		if (!QRFKeepArmedVehiclesMounted && !support.DropPending)
			passengerDeliveryMode = !IsArmedVehicleSupportPrefab(support.VehiclePrefab);

		if (!MountVehicleCrew(support.CrewGroup, support.VehicleEntity, support.VehiclePrefab, stats, passengerDeliveryMode))
		{
			support.MountFailureReason = stats.FailureReason;
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle support mount failed %1 reason=%2 crewSeen=%3 seatsFilled=%4", BuildVehicleSupportContext(support), stats.FailureReason, stats.CrewSeen, stats.SeatsFilled), true);
			return false;
		}

		support.DriverEntity = stats.DriverEntity;
		support.GunnerEntity = stats.GunnerEntity;
		support.ArmedMountedDefence = !passengerDeliveryMode && IsArmedVehicleMount(support.VehiclePrefab, stats);
		support.MountPending = false;
		support.MountRetryCount = 0;
		CacheMountedTransportPassengers(support, stats);
		CollectVehicleSupportCrewUnits(support);
		support.LastCrewHealthScaled = GetVehicleSupportCrewHealthScaled(support);
		TrackCurrentWaveGroupUnits(support.CrewGroup);
		ActivateGroupAI(support.CrewGroup);
		if (passengerDeliveryMode)
		{
			if (!AssignTransportMoveAfterMount(support))
			{
				support.MountFailureReason = "transport_move_assignment_failed";
				return false;
			}

			RetainTransportSupportAsBaseDefence(support);
		}
		else
		{
			LogArmedSupportVehicleGroupMount(support.VehicleEntity, stats);
		}

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle support mount finalized %1 driver=%2 gunner=%3 seatsFilled=%4 crewSeen=%5", BuildVehicleSupportContext(support), JLH_DCF_NodeDebug.EntityLabel(support.DriverEntity), JLH_DCF_NodeDebug.EntityLabel(support.GunnerEntity), stats.SeatsFilled, stats.CrewSeen), true);

		JLH_DCF_PersistenceExclusion.StopTrackingTree(support.CrewGroup, "qrf_vehicle_support_mount_finalized_group");
		JLH_DCF_PersistenceExclusion.StopTrackingTree(support.VehicleEntity, "qrf_vehicle_support_mount_finalized_vehicle");
		return true;
	}

	protected bool AssignTransportMoveAfterMount(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support || !support.CrewGroup)
			return false;

		if (support.TransportMoveAssigned)
			return true;

		if (!RegisterUsableVehicle(support.CrewGroup, support.VehicleEntity))
			return false;

		ActivateGroupAI(support.CrewGroup);
		CleanupVehicleSupportRoute(support);
		array<IEntity> routeWaypoints = {};
		if (!AssignVehicleMoveWaypointToGroup(support.CrewGroup, support.DropPosition, routeWaypoints))
			return false;

		InsertRuntimeRouteWaypoints(routeWaypoints);
		support.RouteWaypoints.Clear();
		foreach (IEntity waypoint : routeWaypoints)
		{
			if (waypoint)
				support.RouteWaypoints.Insert(waypoint);
		}

		if (!IsTransportDriverValid(support))
			LogDebug(string.Format("transport driver validation deferred target=%1", support.DropPosition.ToString()));

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport move assigned target=%1", support.DropPosition.ToString()), true);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "transport move handoff=vanilla_ai", true);
		support.TransportMoveAssigned = true;
		ScheduleTransportDismount(support);
		return true;
	}

	protected bool IsTransportDriverValid(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support || !support.VehicleEntity || !support.DriverEntity)
			return false;

		if (!IsUnitAlive(support.DriverEntity))
			return false;

		if (IsUnitInCompartmentType(support.DriverEntity, support.VehicleEntity, ECompartmentType.PILOT))
			return true;

		return IsUnitInVehicle(support.DriverEntity, support.VehicleEntity);
	}

	protected void CacheMountedTransportPassengers(JLH_DCF_QRFVehicleSupportRuntime support, JLH_DCF_VehicleMountStats stats)
	{
		if (!support)
			return;

		support.TransportPassengerUnits.Clear();
		if (!stats || !stats.PassengerEntities)
			return;

		foreach (IEntity passenger : stats.PassengerEntities)
		{
			if (!passenger || passenger == support.DriverEntity || passenger == support.GunnerEntity || support.TransportPassengerUnits.Contains(passenger))
				continue;

			support.TransportPassengerUnits.Insert(passenger);
		}
	}

	protected void ScheduleTransportDismount(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support || !support.DropPending || support.TransportDismountScheduled || support.TransportDismountRequested || support.TransportPassengerHandoffComplete)
			return;

		if (!GetGame() || !GetGame().GetCallqueue())
			return;

		int delayMs = SecondsToMs(TRANSPORT_DISMOUNT_DELAY);
		if (delayMs <= 0)
			delayMs = 1;

		support.TransportDismountScheduled = true;
		if (!support.TransportDismountScheduleLogged)
		{
			support.TransportDismountScheduleLogged = true;
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport dismount scheduled delay=%1 target=%2", TRANSPORT_DISMOUNT_DELAY, support.DropPosition.ToString()), true);
		}
		GetGame().GetCallqueue().CallLater(JLH_DCF_QRFNodeComponent_HandleTransportDismountCheck, delayMs, false, this, support);
	}

	void HandleTransportDismountCheck(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support)
			return;

		support.TransportDismountScheduled = false;
		if (!support.DropPending || support.TransportDismountRequested || support.TransportPassengerHandoffComplete)
			return;

		if (!support.VehicleEntity || IsEntityDestroyed(support.VehicleEntity) || !support.CrewGroup)
			return;

		vector capTarget = ResolveTransportPassengerCapTarget(support.TransportCapCenter, support.DropPosition);
		vector vehiclePosition = support.VehicleEntity.GetOrigin();
		float distanceToDrop = GetTransportHorizontalDistance(vehiclePosition, support.DropPosition);
		float distanceToCap = GetTransportHorizontalDistance(vehiclePosition, capTarget);

		if (distanceToDrop > TRANSPORT_ARRIVAL_RADIUS && distanceToCap > TRANSPORT_ARRIVAL_RADIUS)
		{
			ScheduleTransportDismount(support);
			return;
		}

		ApplyTransportDropoffSpeed(support);

		float arrivalDistance = distanceToDrop;
		if (distanceToCap < arrivalDistance)
			arrivalDistance = distanceToCap;

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport dismount watch radiusTarget=%1 distance=%2 distanceDrop=%3 distanceCap=%4", TRANSPORT_ARRIVAL_RADIUS, arrivalDistance, distanceToDrop, distanceToCap), true);
		ExecuteTransportDismount(support, capTarget);
	}

	protected bool ApplyTransportDropoffSpeed(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support || support.TransportDropoffSpeedApplied)
			return false;

		support.TransportDropoffSpeedApplied = true;
		bool applied = ApplyTransportCruiseSpeed(support.CrewGroup, TRANSPORT_DROPOFF_CRUISE_SPEED_KMH);
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

	protected void ExecuteTransportDismount(JLH_DCF_QRFVehicleSupportRuntime support, vector capTarget)
	{
		if (!support || support.TransportPassengerHandoffComplete)
			return;

		array<IEntity> cargoUnits = {};
		CollectTransportCargoPassengers(support, cargoUnits);
		if (cargoUnits.IsEmpty())
		{
			support.TransportDismountRequested = true;
			support.TransportDismountVerifyCount = TRANSPORT_HANDOFF_MAX_VERIFY_ATTEMPTS;
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "transport dismount requested once=true count=0", true);
			ScheduleTransportPassengerHandoff(support);
			return;
		}

		int requested = RequestTransportPassengerAnimatedExit(support, cargoUnits);
		support.TransportDismountRequested = true;
		support.TransportDismountVerifyCount = 0;
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport dismount requested once=true count=%1", requested), true);
		ScheduleTransportPassengerHandoff(support);
	}

	protected void ScheduleTransportPassengerHandoff(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support || support.TransportPassengerHandoffComplete)
			return;

		if (!GetGame() || !GetGame().GetCallqueue())
			return;

		int delayMs = SecondsToMs(TRANSPORT_HANDOFF_VERIFY_DELAY);
		if (delayMs <= 0)
			delayMs = 1;

		GetGame().GetCallqueue().CallLater(JLH_DCF_QRFNodeComponent_HandleTransportPassengerHandoff, delayMs, false, this, support);
	}

	void HandleTransportPassengerHandoff(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support || support.TransportPassengerHandoffComplete)
			return;

		if (!support.VehicleEntity || IsEntityDestroyed(support.VehicleEntity) || !support.CrewGroup)
			return;

		vector capTarget = ResolveTransportPassengerCapTarget(support.TransportCapCenter, support.DropPosition);
		array<IEntity> onFootUnits = {};
		array<IEntity> mountedUnits = {};
		CollectTransportPassengerHandoffState(support, onFootUnits, mountedUnits);

		if (!mountedUnits.IsEmpty() && support.TransportDismountVerifyCount < TRANSPORT_HANDOFF_MAX_VERIFY_ATTEMPTS)
		{
			support.TransportDismountVerifyCount++;
			int requested = RequestTransportPassengerAnimatedExit(support, mountedUnits);
			LogDebug(string.Format("transport dismount verify pending onFoot=%1 mounted=%2 retry=%3 requested=%4", onFootUnits.Count(), mountedUnits.Count(), support.TransportDismountVerifyCount, requested));
			ScheduleTransportPassengerHandoff(support);
			return;
		}

		if (onFootUnits.IsEmpty())
		{
			support.TransportPassengerHandoffComplete = true;
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport infantry dismounted count=0 stillMounted=%1", mountedUnits.Count()), true);
			if (mountedUnits.IsEmpty())
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "transport infantry handoff complete=false reason=no_passengers", true);
			else
				JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "transport infantry handoff complete=false reason=still_mounted", true);
			return;
		}

		SCR_AIGroup passengerGroup = support.TransportPassengerGroup;
		if (!passengerGroup)
		{
			passengerGroup = SpawnEmptyVehicleCrewCommandGroup(support.CrewPrefab, capTarget);
			if (!passengerGroup)
			{
				ScheduleTransportPassengerHandoff(support);
				return;
			}

			support.TransportPassengerGroup = passengerGroup;
		}

		int moved = MoveTransportPassengerUnitsToGroup(support.CrewGroup, passengerGroup, onFootUnits);
		if (moved <= 0 && CountLivingTransportGroupUnits(passengerGroup) <= 0)
		{
			ScheduleTransportPassengerHandoff(support);
			return;
		}

		ActivateGroupAI(passengerGroup);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport infantry dismounted count=%1 stillMounted=%2", moved, mountedUnits.Count()), true);
		ScheduleTransportPassengerSearchDestroyAssignment(support);
	}

	protected void ScheduleTransportPassengerSearchDestroyAssignment(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support || support.TransportPassengerHandoffComplete || support.TransportPassengerSearchDestroyScheduled)
			return;

		if (!GetGame() || !GetGame().GetCallqueue())
			return;

		int delayMs = SecondsToMs(TRANSPORT_SEARCH_DESTROY_ASSIGN_DELAY);
		if (delayMs <= 0)
			delayMs = 1;

		support.TransportPassengerSearchDestroyScheduled = true;
		vector capTarget = ResolveTransportPassengerCapTarget(support.TransportCapCenter, support.DropPosition);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport infantry search destroy scheduled delay=%1 target=%2", TRANSPORT_SEARCH_DESTROY_ASSIGN_DELAY, capTarget.ToString()), true);
		GetGame().GetCallqueue().CallLater(JLH_DCF_QRFNodeComponent_HandleTransportPassengerSearchDestroyAssignment, delayMs, false, this, support);
	}

	void HandleTransportPassengerSearchDestroyAssignment(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support || support.TransportPassengerHandoffComplete)
			return;

		vector capTarget = ResolveTransportPassengerCapTarget(support.TransportCapCenter, support.DropPosition);
		array<IEntity> routeWaypoints = {};
		if (!AssignTransportPassengerSearchDestroyWaypoint(support.TransportPassengerGroup, capTarget, routeWaypoints))
		{
			support.TransportPassengerHandoffComplete = true;
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "transport infantry handoff complete=false reason=search_destroy_waypoint_failed", true);
			return;
		}

		ActivateGroupAI(support.TransportPassengerGroup);
		support.TransportPassengerHandoffComplete = true;
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport infantry search destroy target=%1", capTarget.ToString()), true);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "transport infantry handoff complete=true", true);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("truck hold at drop point target=%1", support.DropPosition.ToString()), true);
		ScheduleTransportPassengerSearchDestroyDefendFallback(support);
	}

	protected bool AssignTransportPassengerSearchDestroyWaypoint(SCR_AIGroup group, vector capTarget, notnull array<IEntity> routeWaypoints)
	{
		routeWaypoints.Clear();
		if (!group)
			return false;

		JLH_DCF_NodeWaypointResolver.ClearGroupWaypoints(group);
		AIWaypoint searchWaypoint = SpawnWaypoint(SEARCH_AND_DESTROY_WAYPOINT_PREFAB, capTarget);
		if (!searchWaypoint)
			return false;

		group.AddWaypoint(searchWaypoint);
		routeWaypoints.Insert(searchWaypoint);
		return true;
	}

	protected void ScheduleTransportPassengerSearchDestroyDefendFallback(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support || support.TransportPassengerSearchDestroyDefendFallbackScheduled)
			return;

		if (!GetGame() || !GetGame().GetCallqueue())
			return;

		int delayMs = SecondsToMs(TRANSPORT_SEARCH_DESTROY_DEFEND_FALLBACK_DELAY);
		if (delayMs <= 0)
			delayMs = 1;

		support.TransportPassengerSearchDestroyDefendFallbackScheduled = true;
		GetGame().GetCallqueue().CallLater(JLH_DCF_QRFNodeComponent_HandleTransportPassengerSearchDestroyDefendFallback, delayMs, false, this, support);
	}

	void HandleTransportPassengerSearchDestroyDefendFallback(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support || !support.TransportPassengerGroup)
			return;

		if (CountLivingTransportGroupUnits(support.TransportPassengerGroup) <= 0)
			return;

		vector capTarget = ResolveTransportPassengerCapTarget(support.TransportCapCenter, support.DropPosition);
		if (IsTransportPassengerCombatActive(support.TransportPassengerGroup))
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport infantry defend fallback skipped reason=active_combat target=%1", capTarget.ToString()), true);
			return;
		}

		array<IEntity> routeWaypoints = {};
		if (!AssignTransportPassengerDefendWaypoint(support.TransportPassengerGroup, capTarget, routeWaypoints))
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "transport infantry defend fallback failed reason=waypoint_spawn_failed", true);
			return;
		}

		ActivateGroupAI(support.TransportPassengerGroup);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("transport infantry defend fallback target=%1", capTarget.ToString()), true);
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

	protected bool IsTransportPassengerCombatActive(SCR_AIGroup group)
	{
		if (!group)
			return false;

		return HasTransportPassengerActiveCombatTarget(group);
	}

	protected bool HasTransportPassengerActiveCombatTarget(SCR_AIGroup group)
	{
		if (!group)
			return false;

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!IsUnitAlive(unit))
				continue;

			SCR_AICombatComponent combat = SCR_AICombatComponent.Cast(unit.FindComponent(SCR_AICombatComponent));
			if (!combat)
				continue;

			if (combat.GetCurrentTarget())
				return true;

			if (combat.GetRetreatTarget())
				return true;

			if (combat.GetEndangeringEnemy())
				return true;

			BaseTarget lastSeenEnemy = combat.GetLastSeenEnemy();
			if (lastSeenEnemy && lastSeenEnemy.GetTimeSinceSeen() <= 3.0)
				return true;
		}

		return false;
	}

	protected int CollectTransportPassengerHandoffState(JLH_DCF_QRFVehicleSupportRuntime support, notnull array<IEntity> onFootUnits, notnull array<IEntity> mountedUnits)
	{
		onFootUnits.Clear();
		mountedUnits.Clear();
		if (!support || !support.VehicleEntity)
			return 0;

		foreach (IEntity cachedPassenger : support.TransportPassengerUnits)
		{
			AddTransportPassengerHandoffState(support, cachedPassenger, onFootUnits, mountedUnits);
		}

		if (!onFootUnits.IsEmpty() || !mountedUnits.IsEmpty())
			return onFootUnits.Count() + mountedUnits.Count();

		AddTransportPassengerGroupState(support, support.CrewGroup, onFootUnits, mountedUnits);
		if (support.TransportPassengerGroup && support.TransportPassengerGroup != support.CrewGroup)
			AddTransportPassengerGroupState(support, support.TransportPassengerGroup, onFootUnits, mountedUnits);

		return onFootUnits.Count() + mountedUnits.Count();
	}

	protected void AddTransportPassengerGroupState(JLH_DCF_QRFVehicleSupportRuntime support, SCR_AIGroup group, notnull array<IEntity> onFootUnits, notnull array<IEntity> mountedUnits)
	{
		if (!group)
			return;

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			AddTransportPassengerHandoffState(support, agent.GetControlledEntity(), onFootUnits, mountedUnits);
		}
	}

	protected void AddTransportPassengerHandoffState(JLH_DCF_QRFVehicleSupportRuntime support, IEntity unit, notnull array<IEntity> onFootUnits, notnull array<IEntity> mountedUnits)
	{
		if (!support || !IsTransportPassengerCandidate(unit, support.VehicleEntity, support.DriverEntity, support.GunnerEntity))
			return;

		if (IsUnitInVehicle(unit, support.VehicleEntity))
		{
			if (!mountedUnits.Contains(unit))
				mountedUnits.Insert(unit);
			return;
		}

		if (!onFootUnits.Contains(unit))
			onFootUnits.Insert(unit);
	}

	protected int CollectTransportCargoPassengers(JLH_DCF_QRFVehicleSupportRuntime support, notnull array<IEntity> cargoUnits)
	{
		cargoUnits.Clear();
		if (!support || !support.CrewGroup || !support.VehicleEntity)
			return 0;

		foreach (IEntity cachedPassenger : support.TransportPassengerUnits)
		{
			if (!IsTransportCargoPassenger(cachedPassenger, support.VehicleEntity, support.DriverEntity, support.GunnerEntity))
				continue;

			if (!cargoUnits.Contains(cachedPassenger))
				cargoUnits.Insert(cachedPassenger);
		}

		if (!cargoUnits.IsEmpty())
			return cargoUnits.Count();

		array<AIAgent> agents = {};
		support.CrewGroup.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!IsTransportCargoPassenger(unit, support.VehicleEntity, support.DriverEntity, support.GunnerEntity))
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

	protected int RequestTransportPassengerAnimatedExit(JLH_DCF_QRFVehicleSupportRuntime support, notnull array<IEntity> cargoUnits)
	{
		if (!support || !support.VehicleEntity)
			return 0;

		int requested = 0;
		foreach (IEntity unit : cargoUnits)
		{
			if (!unit || !IsUnitAlive(unit))
				continue;

			if (!IsUnitInVehicle(unit, support.VehicleEntity))
				continue;

			int delayMs = requested * TRANSPORT_EXIT_STAGGER_MS;
			if (delayMs <= 0 || !GetGame() || !GetGame().GetCallqueue())
			{
				if (!RequestTransportPassengerExit(unit, support.VehicleEntity))
					continue;

				requested++;
				continue;
			}

			GetGame().GetCallqueue().CallLater(JLH_DCF_QRFNodeComponent_RequestTransportPassengerExit, delayMs, false, this, unit, support.VehicleEntity);
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

	protected bool ShouldRetryVehicleMount(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support)
			return false;

		int maxRetries = VEHICLE_MOUNT_MAX_RETRIES;
		if (support.DropPending)
			maxRetries = VEHICLE_TRANSPORT_MOUNT_MAX_RETRIES;

		if (support.MountRetryCount >= maxRetries)
			return false;

		return true;
	}

	protected void CollectVehicleSupportCrewUnits(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support || !support.CrewGroup)
			return;

		support.VehicleCrewUnits.Clear();
		array<AIAgent> agents = {};
		support.CrewGroup.GetAgents(agents);

		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (unit && !support.VehicleCrewUnits.Contains(unit))
				support.VehicleCrewUnits.Insert(unit);
		}
	}

	protected void UpdateVehicleSupportBlockingState(JLH_DCF_QRFVehicleSupportRuntime support, int now)
	{
		if (!support || support.SupportState != JLH_DCF_QRFVehicleSupportState.ACTIVE_SUPPORT)
			return;

		string ineffectiveReason;
		if (!IsVehicleSupportCombatEffective(support, ineffectiveReason))
		{
			MarkVehicleSupportComplete(support, ineffectiveReason, now);
			return;
		}

		if (support.DropPending)
			return;

		if (now - support.ActiveSupportStartedTick >= VEHICLE_SUPPORT_ACTIVE_WINDOW_MS)
			MarkVehicleSupportComplete(support, "timeout", now);
	}

	protected bool IsVehicleSupportCombatEffective(JLH_DCF_QRFVehicleSupportRuntime support, out string reason)
	{
		reason = "";
		if (!support || !support.VehicleEntity || IsEntityDestroyed(support.VehicleEntity))
		{
			reason = "vehicle_destroyed";
			return false;
		}

		if (support.DropPending)
		{
			if (CountLivingVehicleSupportCrewUnits(support) <= 0)
			{
				reason = "crew_dead";
				return false;
			}

			return true;
		}

		bool driverAlive = IsRequiredVehicleCrewAlive(support.DriverEntity);
		bool gunnerAlive = IsRequiredVehicleCrewAlive(support.GunnerEntity);

		if (!driverAlive && !gunnerAlive)
		{
			reason = "crew_dead";
			return false;
		}

		return true;
	}

	protected bool IsRequiredVehicleCrewAlive(IEntity crew)
	{
		if (!crew)
			return false;

		return IsUnitAlive(crew);
	}

	protected int CountLivingVehicleSupportCrewUnits(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support)
			return 0;

		int living = 0;
		foreach (IEntity unit : support.VehicleCrewUnits)
		{
			if (unit && IsUnitAlive(unit))
				living++;
		}

		return living;
	}

	protected void MarkVehicleSupportComplete(JLH_DCF_QRFVehicleSupportRuntime support, string reason, int now)
	{
		if (!support || support.SupportState != JLH_DCF_QRFVehicleSupportState.ACTIVE_SUPPORT)
			return;

		if (ShouldRetainVehicleSupportAsBaseDefence(support, reason))
		{
			RetainTransportSupportAsBaseDefence(support);
			return;
		}

		support.SupportState = JLH_DCF_QRFVehicleSupportState.SUPPORT_COMPLETE;
		support.SupportCompletedTick = now;
		support.NextCleanupAttemptTick = now + VEHICLE_SUPPORT_CLEANUP_RETRY_MS;
		RemoveCurrentWaveVehicleSupportBlockers(support);

		if (!m_aVehicleSupportsAwaitingCleanup.Contains(support))
			m_aVehicleSupportsAwaitingCleanup.Insert(support);

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle support no longer blocking wave=%1 reason=%2", support.WaveIndex, reason), true);
		ReleaseTransportLockForSupport(support, reason);
	}

	protected void RetainTransportSupportAsBaseDefence(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support)
			return;

		RemoveCurrentWaveVehicleSupportBlockers(support);
		RemoveCurrentWaveGroupAndUnits(support.CrewGroup);
		DetachRouteWaypointsFromRuntimeCleanup(support.RouteWaypoints);
		if (m_RuntimeTracker)
		{
			m_RuntimeTracker.ForgetGroup(support.CrewGroup);
			m_RuntimeTracker.ForgetVehicle(support.VehicleEntity);
		}

		if (!m_aRetainedTransportSupports.Contains(support))
			m_aRetainedTransportSupports.Insert(support);

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, "transport retained as base defence=true", true);
	}

	protected void TickRetainedTransportSupports(int now)
	{
		for (int i = m_aRetainedTransportSupports.Count() - 1; i >= 0; i--)
		{
			JLH_DCF_QRFVehicleSupportRuntime support = m_aRetainedTransportSupports[i];
			if (!support)
			{
				m_aRetainedTransportSupports.Remove(i);
				continue;
			}

			if (!support.VehicleEntity || IsEntityDestroyed(support.VehicleEntity))
			{
				ReleaseTransportLockForSupport(support, "vehicle_destroyed");
				m_aRetainedTransportSupports.Remove(i);
				continue;
			}

			if (CountLivingRetainedTransportUnits(support) <= 0)
			{
				ReleaseTransportLockForSupport(support, "crew_dead");
				m_aRetainedTransportSupports.Remove(i);
			}
		}
	}

	protected int CountLivingRetainedTransportUnits(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support)
			return 0;

		int living = CountLivingTransportGroupUnits(support.CrewGroup);
		if (support.TransportPassengerGroup && support.TransportPassengerGroup != support.CrewGroup)
			living += CountLivingTransportGroupUnits(support.TransportPassengerGroup);

		return living;
	}

	protected int CountLivingTransportGroupUnits(SCR_AIGroup group)
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

	protected void RemoveCurrentWaveVehicleSupportBlockers(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support)
			return;

		for (int i = m_aCurrentWaveVehicleSupports.Count() - 1; i >= 0; i--)
		{
			if (m_aCurrentWaveVehicleSupports[i] == support)
				m_aCurrentWaveVehicleSupports.Remove(i);
		}

		for (int j = m_aCurrentWaveVehicles.Count() - 1; j >= 0; j--)
		{
			if (m_aCurrentWaveVehicles[j] == support.VehicleEntity)
				m_aCurrentWaveVehicles.Remove(j);
		}

		RemoveCurrentWaveVehicleSupportCrewUnits(support);
	}

	protected void TickVehicleSupportCleanup(int now)
	{
		for (int i = m_aVehicleSupportsAwaitingCleanup.Count() - 1; i >= 0; i--)
		{
			JLH_DCF_QRFVehicleSupportRuntime support = m_aVehicleSupportsAwaitingCleanup[i];
			if (!support)
			{
				m_aVehicleSupportsAwaitingCleanup.Remove(i);
				continue;
			}

			if (support.SupportState != JLH_DCF_QRFVehicleSupportState.SUPPORT_COMPLETE)
				continue;

			if (support.NextCleanupAttemptTick > now)
				continue;

			if (ShouldRetainVehicleSupportAsBaseDefence(support, "cleanup"))
			{
				m_aVehicleSupportsAwaitingCleanup.Remove(i);
				RetainTransportSupportAsBaseDefence(support);
				continue;
			}

			if (ShouldRetainVehicleSupportForGhostDefence(support))
			{
				support.NextCleanupAttemptTick = now + VEHICLE_SUPPORT_CLEANUP_RETRY_MS;
				LogVehicleCleanupDeferred(support, "ghost_handoff", now);
				continue;
			}

			UpdateVehicleSupportActivity(support, now);

			string deferredReason;
			if (!CanCleanupVehicleSupportSafely(support, now, deferredReason))
			{
				support.NextCleanupAttemptTick = now + VEHICLE_SUPPORT_CLEANUP_RETRY_MS;
				LogVehicleCleanupDeferred(support, deferredReason, now);
				continue;
			}

			int removedCrew = CleanupVehicleSupportCrew(support);
			int removedVehicles = CleanupVehicleSupportVehicle(support);
			support.SupportState = JLH_DCF_QRFVehicleSupportState.SAFE_TO_REMOVE;
			m_aVehicleSupportsAwaitingCleanup.Remove(i);
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle cleanup safe removed wave=%1 vehicles=%2 crew=%3", support.WaveIndex, removedVehicles, removedCrew), true);
		}
	}

	protected void UpdateVehicleSupportActivity(JLH_DCF_QRFVehicleSupportRuntime support, int now)
	{
		if (!support)
			return;

		if (support.VehicleEntity)
		{
			float health = GetEntityHealthScaled(support.VehicleEntity);
			if (support.LastHealthScaled >= 0.0 && health < support.LastHealthScaled - 0.01)
				support.LastDamageTick = now;

			support.LastHealthScaled = health;

			vector position = support.VehicleEntity.GetOrigin();
			if (support.LastPositionValid && vector.Distance(position, support.LastPosition) > VEHICLE_SUPPORT_MOVEMENT_EPSILON_METERS)
				support.LastMovementTick = now;

			support.LastPosition = position;
			support.LastPositionValid = true;
		}

		float crewHealth = GetVehicleSupportCrewHealthScaled(support);
		if (support.LastCrewHealthScaled >= 0.0 && crewHealth < support.LastCrewHealthScaled - 0.01)
			support.LastDamageTick = now;

		support.LastCrewHealthScaled = crewHealth;
	}

	protected bool CanCleanupVehicleSupportSafely(JLH_DCF_QRFVehicleSupportRuntime support, int now, out string reason)
	{
		reason = "";
		if (!support)
			return true;

		if (support.LastDamageTick > 0 && now - support.LastDamageTick < VEHICLE_SUPPORT_RECENT_DAMAGE_GRACE_MS)
		{
			reason = "recent_damage";
			return false;
		}

		if (support.LastMovementTick > 0 && now - support.LastMovementTick < VEHICLE_SUPPORT_MOVEMENT_GRACE_MS)
		{
			reason = "combat";
			return false;
		}

		JLH_CleanupSafetyQuery query = new JLH_CleanupSafetyQuery();
		vector anchor = support.DropPosition;
		if (support.VehicleEntity)
			anchor = support.VehicleEntity.GetOrigin();
		else if (support.CrewGroup)
			anchor = support.CrewGroup.GetOrigin();
		query.Init("QRFVehicleSupport wave=" + support.WaveIndex.ToString(), anchor);
		query.CleanupDistance = VEHICLE_SUPPORT_CLEANUP_PLAYER_RADIUS;
		query.VisibilityDistance = Math.Max(VEHICLE_SUPPORT_CLEANUP_PLAYER_RADIUS, JLH_CleanupSafetyService.DEFAULT_VISIBILITY_DISTANCE_METERS);
		query.PlayerFactionFilter = "";
		query.DebugEnabled = DebugLogging;
		query.LastInteractionTick = support.LastDamageTick;
		if (support.LastMovementTick > query.LastInteractionTick)
			query.LastInteractionTick = support.LastMovementTick;
		if (support.CrewGroup)
			query.Groups.Insert(support.CrewGroup);
		if (support.VehicleEntity)
			query.Vehicles.Insert(support.VehicleEntity);
		foreach (IEntity crewUnit : support.VehicleCrewUnits)
		{
			if (crewUnit)
				query.Entities.Insert(crewUnit);
		}

		string blockedReason;
		if (!JLH_CleanupSafetyService.CanCleanupPackage(query, blockedReason))
		{
			reason = blockedReason;
			return false;
		}

		return true;
	}

	protected void LogVehicleCleanupDeferred(JLH_DCF_QRFVehicleSupportRuntime support, string reason, int now)
	{
		if (!support)
			return;

		if (support.LastCleanupDeferredReason == reason && now - support.LastCleanupDeferredLogTick < VEHICLE_SUPPORT_CLEANUP_RETRY_MS)
			return;

		support.LastCleanupDeferredReason = reason;
		support.LastCleanupDeferredLogTick = now;
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle cleanup deferred reason=%1", reason), true);
	}

	protected bool IsAnyValidPlayerNearVehicleSupport(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support)
			return false;

		if (support.VehicleEntity && IsAnyValidPlayerWithinPosition(support.VehicleEntity.GetOrigin(), VEHICLE_SUPPORT_CLEANUP_PLAYER_RADIUS))
			return true;

		foreach (IEntity unit : support.VehicleCrewUnits)
		{
			if (unit && IsAnyValidPlayerWithinPosition(unit.GetOrigin(), VEHICLE_SUPPORT_CLEANUP_PLAYER_RADIUS))
				return true;
		}

		return false;
	}

	protected bool HasAnyValidPlayerLineOfSightToVehicleSupport(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support)
			return false;

		if (support.VehicleEntity && HasAnyValidPlayerLineOfSight(support.VehicleEntity.GetOrigin(), VEHICLE_SUPPORT_CLEANUP_PLAYER_RADIUS))
			return true;

		foreach (IEntity unit : support.VehicleCrewUnits)
		{
			if (unit && HasAnyValidPlayerLineOfSight(unit.GetOrigin(), VEHICLE_SUPPORT_CLEANUP_PLAYER_RADIUS))
				return true;
		}

		return false;
	}

	protected int CleanupVehicleSupportCrew(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support)
			return 0;

		RemoveCurrentWaveGroupAndUnits(support.CrewGroup);
		int deletedGroupUnits = DeleteGroupAndMembers(support.CrewGroup);
		support.CrewGroup = null;
		support.VehicleCrewUnits.Clear();
		return deletedGroupUnits;
	}

	protected int CleanupVehicleSupportVehicle(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support)
			return 0;

		CleanupVehicleSupportRoute(support);

		if (!support.VehicleEntity)
			return 0;

		JLH_DCF_VehicleSustainment.UnregisterVehicle(support.VehicleEntity, "cleanup");
		DeleteRuntimeEntitySafe(support.VehicleEntity, "qrf_vehicle_support_cleanup");
		support.VehicleEntity = null;
		return 1;
	}

	protected void RemoveCurrentWaveVehicleSupportCrewUnits(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support || !support.VehicleCrewUnits)
			return;

		foreach (IEntity unit : support.VehicleCrewUnits)
		{
			for (int i = m_aCurrentWaveUnits.Count() - 1; i >= 0; i--)
			{
				if (m_aCurrentWaveUnits[i] == unit)
					m_aCurrentWaveUnits.Remove(i);
			}
		}
	}

	protected void RemoveCurrentWaveGroupAndUnits(SCR_AIGroup group)
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

			for (int i = m_aCurrentWaveUnits.Count() - 1; i >= 0; i--)
			{
				if (m_aCurrentWaveUnits[i] == unit)
					m_aCurrentWaveUnits.Remove(i);
			}
		}

		for (int j = m_aCurrentWaveGroups.Count() - 1; j >= 0; j--)
		{
			if (m_aCurrentWaveGroups[j] == group)
				m_aCurrentWaveGroups.Remove(j);
		}
	}

	protected bool AssignVehicleMoveWaypointToGroup(SCR_AIGroup group, vector moveTarget, notnull array<IEntity> routeWaypoints)
	{
		routeWaypoints.Clear();
		if (!group)
			return false;

		JLH_DCF_NodeWaypointResolver.ClearGroupWaypoints(group);

		AIWaypoint moveWaypoint = SpawnWaypoint(ASSAULT_MOVE_WAYPOINT_PREFAB, moveTarget);
		if (!moveWaypoint)
			return false;

		group.AddWaypoint(moveWaypoint);
		routeWaypoints.Insert(moveWaypoint);

		bool aggressiveResult = SetGroupAggressiveBehavior(group);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Group behavior set aggressive result=%1 group=%2", JLH_DCF_NodeDebug.BoolLabel(aggressiveResult), JLH_DCF_NodeDebug.EntityLabel(group)), true);
		return true;
	}

	protected bool AssignMountedVehicleHoldWaypointToGroup(SCR_AIGroup group, vector moveTarget, notnull array<IEntity> routeWaypoints)
	{
		routeWaypoints.Clear();
		if (!group)
			return false;

		JLH_DCF_NodeWaypointResolver.ClearGroupWaypoints(group);

		AIWaypoint moveWaypoint = SpawnWaypoint(ASSAULT_MOVE_WAYPOINT_PREFAB, moveTarget);
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
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("mounted vehicle hold assigned target=%1 cycle=true aggressive=%2 group=%3", moveTarget.ToString(), JLH_DCF_NodeDebug.BoolLabel(aggressiveResult), JLH_DCF_NodeDebug.EntityLabel(group)), true);
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

		JLH_DCF_PersistenceExclusion.StopTrackingTree(cycleEntity, "qrf_mounted_vehicle_cycle_waypoint");

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

	protected void CleanupVehicleSupportRoute(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support || !support.RouteWaypoints)
			return;

		foreach (IEntity waypointEntity : support.RouteWaypoints)
		{
			for (int i = m_aRuntimeRouteWaypoints.Count() - 1; i >= 0; i--)
			{
				if (m_aRuntimeRouteWaypoints[i] == waypointEntity)
					m_aRuntimeRouteWaypoints.Remove(i);
			}
		}

		CleanupRouteWaypoints(support.CrewGroup, support.RouteWaypoints);
	}

	protected vector ResolveVehicleSpawnPosition(int vehicleIndex)
	{
		IEntity owner = GetSelectedSpawnOwner();
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

	protected vector ResolveVehicleSupportHoldPosition(vector target, int vehicleIndex)
	{
		int directionOffset = Math.RandomInt(0, 8);
		for (int ring = 0; ring < 4; ring++)
		{
			float radius = VEHICLE_SUPPORT_HOLD_RADIUS_METERS + (ring * VEHICLE_SUPPORT_HOLD_RING_STEP_METERS);
			for (int step = 0; step < 8; step++)
			{
				int directionIndex = directionOffset + vehicleIndex + (step * 3) + ring;
				vector candidate = GroundPosition(target + (GetVehicleHoldDirection(directionIndex) * radius));
				string rejectionReason;
				if (!ValidateDelayedVehicleSpawnPosition(candidate, rejectionReason))
				{
					LogDebug("Vehicle hold rejected reason=" + rejectionReason + " pos=" + candidate.ToString());
					continue;
				}

				if (IsVehicleSupportHoldPositionClear(candidate, target, rejectionReason))
				{
					m_aCurrentWaveVehicleHoldPositions.Insert(candidate);
					JLH_DCF_AssaultVehicleHoldRegistry.Reserve(candidate);
					LogVehicleHoldSelected(vehicleIndex, candidate, string.Format("ok randomOffset=%1", directionOffset));
					return candidate;
				}

				LogDebug("Vehicle hold rejected reason=" + rejectionReason + " pos=" + candidate.ToString());
			}
		}

		vector fallback = GroundPosition(target + (GetVehicleHoldDirection(directionOffset + vehicleIndex) * (VEHICLE_SUPPORT_HOLD_RADIUS_METERS + VEHICLE_SUPPORT_HOLD_RING_STEP_METERS)));
		m_aCurrentWaveVehicleHoldPositions.Insert(fallback);
		JLH_DCF_AssaultVehicleHoldRegistry.Reserve(fallback);
		LogVehicleHoldSelected(vehicleIndex, fallback, string.Format("fallback_short randomOffset=%1", directionOffset));
		return fallback;
	}

	protected vector ResolveTransportVehicleSupportDropPosition(vector target, vector spawnPosition, int vehicleIndex)
	{
		vector direction = BuildTransportTargetDirection(target, spawnPosition, vehicleIndex);
		vector dropPosition = GroundPosition(target + (direction * TRANSPORT_ARRIVAL_RADIUS));
		m_aCurrentWaveVehicleHoldPositions.Insert(dropPosition);
		return dropPosition;
	}

	protected vector BuildTransportTargetDirection(vector target, vector spawnPosition, int vehicleIndex)
	{
		vector flatTarget = target;
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

	protected bool IsVehicleSupportHoldPositionClear(vector candidate, vector target, out string reason)
	{
		reason = "";
		if (vector.Distance(candidate, target) < VEHICLE_SUPPORT_HOLD_MIN_CAP_DISTANCE_METERS)
		{
			reason = "cap_center";
			return false;
		}

		foreach (vector plannedHold : m_aCurrentWaveVehicleHoldPositions)
		{
			if (vector.Distance(candidate, plannedHold) < VEHICLE_SUPPORT_HOLD_SPACING_METERS)
			{
				reason = "planned_vehicle_hold";
				return false;
			}
		}

		if (JLH_DCF_AssaultVehicleHoldRegistry.IsReservedNear(candidate, VEHICLE_SUPPORT_HOLD_SPACING_METERS))
		{
			reason = "reserved_vehicle_hold";
			return false;
		}

		foreach (JLH_DCF_QRFVehicleSupportRuntime support : m_aCurrentWaveVehicleSupports)
		{
			if (!support)
				continue;

			if (support.VehicleEntity && vector.Distance(candidate, support.VehicleEntity.GetOrigin()) < VEHICLE_SUPPORT_HOLD_SPACING_METERS)
			{
				reason = "active_vehicle";
				return false;
			}

			if (vector.Distance(candidate, support.DropPosition) < VEHICLE_SUPPORT_HOLD_SPACING_METERS)
			{
				reason = "active_vehicle_hold";
				return false;
			}
		}

		foreach (JLH_DCF_QRFVehicleSupportRuntime cleanupSupport : m_aVehicleSupportsAwaitingCleanup)
		{
			if (cleanupSupport && cleanupSupport.VehicleEntity && vector.Distance(candidate, cleanupSupport.VehicleEntity.GetOrigin()) < VEHICLE_SUPPORT_HOLD_SPACING_METERS)
			{
				reason = "cleanup_vehicle";
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

	protected void RecordCurrentWaveInfantrySpawnPosition(vector position)
	{
		m_aCurrentWaveInfantrySpawnPositions.Insert(position);
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

		if (!GetGame())
			return true;

		ref array<IEntity> players = {};
		GatherSpawnSafetyPlayers(players);
		bool closePlayerFound = false;

		foreach (IEntity controlled : players)
		{
			if (!IsValidSpawnSafetyPlayer(controlled))
				continue;

			if (vector.Distance(position, controlled.GetOrigin()) >= safetyDistance)
				continue;

			closePlayerFound = true;
			if (!RequireLOSToBlockCloseSpawn || HasDirectLineOfSightToSpawn(controlled, position))
			{
				reason = "visible_to_player";
				return false;
			}
		}

		if (closePlayerFound)
			reason = "close_not_visible";

		return true;
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

	protected float GetEntityHealthScaled(IEntity entity)
	{
		if (!entity)
			return 0.0;

		DamageManagerComponent damageManager = DamageManagerComponent.Cast(entity.FindComponent(DamageManagerComponent));
		if (!damageManager)
			return 1.0;

		return damageManager.GetHealthScaled();
	}

	protected float GetVehicleSupportCrewHealthScaled(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support || !support.VehicleCrewUnits)
			return 0.0;

		float health = 0.0;
		foreach (IEntity unit : support.VehicleCrewUnits)
		{
			if (unit)
				health += GetEntityHealthScaled(unit);
		}

		return health;
	}

	protected void EnsureDefaultVehicleTraitFilters()
	{
		NormalizeVehicleTraitFilters();
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
		if (!entry)
			return false;

		if (MatchesExcludedVehicleTrait(entry))
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
			if (labelName == "" || labelName == "NONE")
				continue;

			if (HasCatalogLabel(entry, labelName))
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
		SCR_EntityCatalogEntry entry = FindVehicleCatalogEntryByPrefab(vehiclePrefab);
		if (!entry)
			return true;

		if (!MatchesExcludedVehicleTrait(entry))
			return true;

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle support skipped reason=specific_prefab_excluded prefab=%1", vehiclePrefab), true);
		return false;
	}

	protected SCR_EntityCatalogEntry FindVehicleCatalogEntryByPrefab(ResourceName vehiclePrefab)
	{
		if (IsEmptyResource(vehiclePrefab))
			return null;

		SCR_EntityCatalog catalog = ResolveFactionCatalog(EEntityCatalogType.VEHICLE);
		if (!catalog)
			return null;

		array<SCR_EntityCatalogEntry> entries = {};
		catalog.GetEntityList(entries);
		foreach (SCR_EntityCatalogEntry entry : entries)
		{
			if (!entry)
				continue;

			if (entry.GetPrefab() == vehiclePrefab)
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

	protected string GetVehicleSupportModeLabel()
	{
		if (m_bEffectivePassengerDelivery)
			return "TRANSPORT_DROP";

		return "MOUNTED_ASSAULT";
	}

	protected string GetQRFPresetLabel(JLH_DCF_QRFForcePreset preset)
	{
		if (preset == JLH_DCF_QRFForcePreset.LIGHT_PROBE)
			return "LIGHT_PROBE";
		if (preset == JLH_DCF_QRFForcePreset.STANDARD_ASSAULT)
			return "STANDARD_ASSAULT";
		if (preset == JLH_DCF_QRFForcePreset.HEAVY_ASSAULT)
			return "HEAVY_ASSAULT";
		if (preset == JLH_DCF_QRFForcePreset.TRANSPORT_ASSAULT)
			return "TRANSPORT_ASSAULT";
		if (preset == JLH_DCF_QRFForcePreset.RANDOM)
			return "RANDOM";

		return "CUSTOM_MANUAL";
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

	protected void SuppressInheritedAmbientState(bool logSuppression)
	{
		m_bQRFControlledSpawnAllowed = false;
		//SetMembersAlive(0);
		SetIsSpawned(false);
		SetIsPaused(true);

		if (!logSuppression || m_bVanillaSuppressionLogged)
			return;

		m_bVanillaSuppressionLogged = true;
		LogDebug(string.Format("Vanilla ambient spawn suppressed node=%1", JLH_DCF_NodeDebug.EntityLabel(GetOwner())));
	}

	protected bool CanSpawnWaveSafely()
	{
		m_SelectedSpawnNode = null;
		if (!EnableSpawnVisibilitySafety)
		{
			m_SelectedSpawnNode = this;
			ResetSpawnSafetyDeferral();
			return true;
		}

		float safetyDistance = GetCurrentSpawnSafetyDistance();
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

		JLH_QRFNodeComponent alternateNode;
		string alternateReason;
		if (TrySelectAlternateSpawnNodeWithPlayers(safetyDistance, players, alternateNode, alternateReason))
		{
			m_SelectedSpawnNode = alternateNode;
			LogSpawnSafetyAlternateSelected(this, alternateNode);
			LogSpawnSafetyPassed(alternateNode, alternateReason);
			ResetSpawnSafetyDeferral();
			return true;
		}

		int now = System.GetTickCount();
		if (m_iSpawnSafetyDeferralStartedTick <= 0)
			m_iSpawnSafetyDeferralStartedTick = now;

		if (AllowUnsafeFallbackAfterTimeout && HasSpawnSafetyDeferralTimedOut(now))
		{
			JLH_QRFNodeComponent fallbackNode = SelectLeastBadSameBaseSpawnNodeWithPlayers(safetyDistance, players);
			if (!fallbackNode)
				fallbackNode = this;

			m_SelectedSpawnNode = fallbackNode;
			LogSpawnSafetyFallbackAllowed(fallbackNode);
			ResetSpawnSafetyDeferral();
			return true;
		}

		LogSpawnSafetyDeferred(this, preferredReason);
		return false;
	}

	protected float GetCurrentSpawnSafetyDistance()
	{
		float distance = MinimumInfantrySpawnSafetyDistance;
		if (distance < 0.0)
			distance = 0.0;

		int nextWaveIndex = m_iCurrentWave + 1;
		if (GetEffectiveVehicleCountForWave(nextWaveIndex) > 0)
		{
			float vehicleDistance = MinimumVehicleSpawnSafetyDistance;
			if (vehicleDistance < 0.0)
				vehicleDistance = 0.0;

			if (vehicleDistance > distance)
				distance = vehicleDistance;
		}

		return distance;
	}

	protected bool TrySelectAlternateSpawnNode(float safetyDistance, out JLH_QRFNodeComponent selectedNode, out string selectedReason)
	{
		ref array<IEntity> players = {};
		GatherSpawnSafetyPlayers(players);
		return TrySelectAlternateSpawnNodeWithPlayers(safetyDistance, players, selectedNode, selectedReason);
	}

	protected bool TrySelectAlternateSpawnNodeWithPlayers(float safetyDistance, notnull array<IEntity> players, out JLH_QRFNodeComponent selectedNode, out string selectedReason)
	{
		selectedNode = null;
		selectedReason = "";

		ref array<JLH_QRFNodeComponent> nodes = {};
		JLH_QRFManager.CollectRegisteredNodes(nodes);

		float bestScore = -1.0;
		foreach (JLH_QRFNodeComponent node : nodes)
		{
			if (!IsValidSameBaseSpawnNode(node) || node == this)
				continue;

			string reason;
			float closestDistance;
			bool visibleToPlayer;
			if (!node.EvaluateSpawnVisibilitySafetyWithPlayers(safetyDistance, players, reason, closestDistance, visibleToPlayer))
				continue;

			float score = GetSpawnNodeScore(reason, closestDistance, visibleToPlayer);
			if (!selectedNode || score > bestScore)
			{
				selectedNode = node;
				selectedReason = reason;
				bestScore = score;
			}
		}

		return selectedNode != null;
	}

	protected JLH_QRFNodeComponent SelectLeastBadSameBaseSpawnNode(float safetyDistance)
	{
		ref array<IEntity> players = {};
		GatherSpawnSafetyPlayers(players);
		return SelectLeastBadSameBaseSpawnNodeWithPlayers(safetyDistance, players);
	}

	protected JLH_QRFNodeComponent SelectLeastBadSameBaseSpawnNodeWithPlayers(float safetyDistance, notnull array<IEntity> players)
	{
		JLH_QRFNodeComponent selectedNode = null;
		float bestScore = -1.0;

		ref array<JLH_QRFNodeComponent> nodes = {};
		JLH_QRFManager.CollectRegisteredNodes(nodes);

		foreach (JLH_QRFNodeComponent node : nodes)
		{
			if (!IsValidSameBaseSpawnNode(node))
				continue;

			string reason;
			float closestDistance;
			bool visibleToPlayer;
			node.EvaluateSpawnVisibilitySafetyWithPlayers(safetyDistance, players, reason, closestDistance, visibleToPlayer);

			float score = GetSpawnNodeScore(reason, closestDistance, visibleToPlayer);
			if (!selectedNode || score > bestScore)
			{
				selectedNode = node;
				bestScore = score;
			}
		}

		return selectedNode;
	}

	protected float GetSpawnNodeScore(string reason, float closestDistance, bool visibleToPlayer)
	{
		float score = closestDistance;
		if (score < 0.0)
			score = 999999.0;

		if (!visibleToPlayer)
			score = score + 50000.0;

		if (reason == "distance_clear")
			score = score + 100000.0;
		else if (reason == "close_not_visible")
			score = score + 50000.0;

		return score;
	}

	protected bool IsValidSameBaseSpawnNode(JLH_QRFNodeComponent node)
	{
		if (!node || !node.IsRegistered() || !node.GetOwner())
			return false;

		if (node.GetRegisteredBaseName() == "" || m_sBaseName == "")
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
		ResetSpawnSafetyLogThrottle();
	}

	protected JLH_QRFNodeComponent ResolveSelectedSpawnNode()
	{
		if (m_SelectedSpawnNode && m_SelectedSpawnNode.GetOwner())
			return m_SelectedSpawnNode;

		return this;
	}

	protected IEntity GetSelectedSpawnOwner()
	{
		JLH_QRFNodeComponent node = ResolveSelectedSpawnNode();
		if (node)
		{
			IEntity selectedOwner = node.GetOwner();
			if (selectedOwner)
				return selectedOwner;
		}

		return GetOwner();
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

		if (!GetGame() || !GetGame().GetWorld())
		{
			reason = "world_missing";
			return false;
		}

		vector spawnPosition = owner.GetOrigin();
		bool closePlayerFound = false;

		foreach (IEntity controlled : players)
		{
			if (!IsValidSpawnSafetyPlayer(controlled))
				continue;

			float distance = vector.Distance(spawnPosition, controlled.GetOrigin());
			if (closestDistance < 0.0 || distance < closestDistance)
				closestDistance = distance;

			if (distance >= safetyDistance)
				continue;

			closePlayerFound = true;
			if (!RequireLOSToBlockCloseSpawn)
			{
				visibleToPlayer = true;
				continue;
			}

			if (HasDirectLineOfSightToSpawn(controlled, spawnPosition))
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

	protected void LogSpawnSafetyRetryScheduled()
	{
		int now = System.GetTickCount();
		if (m_iLastSpawnSafetyRetryLogTick > 0 && now - m_iLastSpawnSafetyRetryLogTick < SPAWN_SAFETY_LOG_THROTTLE_MS)
			return;

		m_iLastSpawnSafetyRetryLogTick = now;
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Spawn safety retry scheduled seconds=%1", SpawnSafetyRetryDelay), true);
	}

	protected void LogSpawnSafetyPassed(JLH_QRFNodeComponent node, string reason)
	{
		if (!node || !ShouldLogSpawnSafetyKey(string.Format("passed_%1_%2", reason, JLH_DCF_NodeDebug.EntityLabel(node.GetOwner()))))
			return;

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Spawn safety passed node=%1 reason=%2", JLH_DCF_NodeDebug.EntityLabel(node.GetOwner()), reason), true);
	}

	protected void LogSpawnSafetyDeferred(JLH_QRFNodeComponent node, string reason)
	{
		if (!node || !ShouldLogSpawnSafetyKey(string.Format("deferred_%1_%2", reason, JLH_DCF_NodeDebug.EntityLabel(node.GetOwner()))))
			return;

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Spawn safety deferred node=%1 reason=%2 retry=%3", JLH_DCF_NodeDebug.EntityLabel(node.GetOwner()), reason, SpawnSafetyRetryDelay), true);
	}

	protected void LogSpawnSafetyAlternateSelected(JLH_QRFNodeComponent fromNode, JLH_QRFNodeComponent toNode)
	{
		if (!fromNode || !toNode || !ShouldLogSpawnSafetyKey(string.Format("alternate_%1_%2", JLH_DCF_NodeDebug.EntityLabel(fromNode.GetOwner()), JLH_DCF_NodeDebug.EntityLabel(toNode.GetOwner()))))
			return;

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Spawn safety alternate node selected from=%1 to=%2 reason=preferred_visible", JLH_DCF_NodeDebug.EntityLabel(fromNode.GetOwner()), JLH_DCF_NodeDebug.EntityLabel(toNode.GetOwner())), true);
	}

	protected void LogSpawnSafetyFallbackAllowed(JLH_QRFNodeComponent node)
	{
		if (!node || !ShouldLogSpawnSafetyKey(string.Format("fallback_%1", JLH_DCF_NodeDebug.EntityLabel(node.GetOwner()))))
			return;

		JLH_DCF_NodeDebug.Warn(SYSTEM_NAME, string.Format("Spawn safety fallback allowed node=%1 reason=timeout leastBad=true", JLH_DCF_NodeDebug.EntityLabel(node.GetOwner())));
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

	protected void ResetSpawnSafetyLogThrottle()
	{
		m_sLastSpawnSafetyLogKey = "";
		m_iLastSpawnSafetyLogTick = 0;
		m_iLastSpawnSafetyRetryLogTick = 0;
	}

	protected bool IsValidSpawnSafetyPlayer(IEntity entity)
	{
		return JLH_DCF_PlayerPresence.IsLiveHumanControlledCharacterForFaction(entity, TRIGGERING_PLAYER_FACTION_KEY);
	}

	protected void GatherSpawnSafetyPlayers(notnull array<IEntity> players)
	{
		JLH_AddonWorldQuery.GatherLiveDeployedPlayers(players, TRIGGERING_PLAYER_FACTION_KEY);
	}

	protected void GatherProximityPlayers(notnull array<IEntity> players)
	{
		JLH_AddonWorldQuery.GatherLiveDeployedPlayers(players, "");
	}

	protected bool HasDirectLineOfSightToSpawn(IEntity playerEntity, vector spawnPosition)
	{
		if (!playerEntity || !GetGame() || !GetGame().GetWorld())
			return false;

		TraceParam trace = new TraceParam();
		trace.Start = playerEntity.GetOrigin() + Vector(0, 1.6, 0);
		trace.End = spawnPosition + Vector(0, 1.2, 0);
		trace.Exclude = playerEntity;
		trace.Flags = TraceFlags.DEFAULT | TraceFlags.ANY_CONTACT;
		trace.LayerMask = EPhysicsLayerDefs.Projectile;

		float traceDistance = GetGame().GetWorld().TraceMove(trace, null);
		return traceDistance >= 0.98;
	}

	protected bool IsAnyValidPlayerWithinPosition(vector position, float distanceLimit)
	{
		ref array<IEntity> players = {};
		GatherProximityPlayers(players);
		return IsAnyValidPlayerWithinPositionWithPlayers(position, distanceLimit, players);
	}

	protected bool IsAnyValidPlayerWithinPositionWithPlayers(vector position, float distanceLimit, notnull array<IEntity> players)
	{
		foreach (IEntity controlled : players)
		{
			if (!IsValidPlayerForProximity(controlled))
				continue;

			if (vector.Distance(position, controlled.GetOrigin()) <= distanceLimit)
				return true;
		}

		return false;
	}

	protected bool AreAllValidPlayersBeyondPosition(vector position, float distanceLimit)
	{
		ref array<IEntity> players = {};
		GatherProximityPlayers(players);
		return AreAllValidPlayersBeyondPositionWithPlayers(position, distanceLimit, players);
	}

	protected bool AreAllValidPlayersBeyondPositionWithPlayers(vector position, float distanceLimit, notnull array<IEntity> players)
	{
		foreach (IEntity controlled : players)
		{
			if (!IsValidPlayerForProximity(controlled))
				continue;

			if (vector.Distance(position, controlled.GetOrigin()) <= distanceLimit)
				return false;
		}

		return true;
	}

	protected bool HasAnyValidPlayerLineOfSight(vector position, float distanceLimit)
	{
		ref array<IEntity> players = {};
		GatherProximityPlayers(players);
		return HasAnyValidPlayerLineOfSightWithPlayers(position, distanceLimit, players);
	}

	protected bool HasAnyValidPlayerLineOfSightWithPlayers(vector position, float distanceLimit, notnull array<IEntity> players)
	{
		foreach (IEntity controlled : players)
		{
			if (!IsValidPlayerForProximity(controlled))
				continue;

			if (vector.Distance(position, controlled.GetOrigin()) > distanceLimit)
				continue;

			if (HasDirectLineOfSightToSpawn(controlled, position))
				return true;
		}

		return false;
	}

	protected PlayerManager GetPlayerManagerSafe()
	{
		if (!GetGame())
			return null;

		return GetGame().GetPlayerManager();
	}

	protected bool IsValidPlayerForProximity(IEntity entity)
	{
		return JLH_DCF_PlayerPresence.IsLiveHumanControlledCharacter(entity);
	}

	protected bool IsQRFSuccessOutcomeConfirmed()
	{
		if (!m_BaseComponent || m_sFactionKey == "")
			return false;

		string currentOwner = JLH_DCF_NodeBaseResolver.GetBaseFactionKey(m_BaseComponent);
		if (currentOwner != m_sFactionKey)
			return false;

		SCR_CampaignSeizingComponent seizingComponent = FindSeizingComponent();
		if (!seizingComponent)
			return true;

		string prevailingFaction = seizingComponent.JLH_DCF_GetPrevailingFactionKey();
		float progress = seizingComponent.JLH_DCF_GetSeizingProgressNormalized();
		if (prevailingFaction == TRIGGERING_PLAYER_FACTION_KEY && progress > 0.0)
			return !HasLiveTriggeringPlayerNearDefendTarget();

		return true;
	}

	protected bool HasLiveTriggeringPlayerNearDefendTarget()
	{
		vector defendTarget;
		string failureReason;
		if (!ResolveDefendTarget(defendTarget, failureReason))
			return JLH_DCF_PlayerPresence.HasLiveHumanInWorld();

		return JLH_DCF_PlayerPresence.HasLiveHumanForFactionWithin(defendTarget, QRF_SUCCESS_PLAYER_CLEAR_DISTANCE, TRIGGERING_PLAYER_FACTION_KEY);
	}

	protected bool ResolveDefendTarget(out vector defendTarget, out string failureReason)
	{
		IEntity owner = GetOwner();
		if (owner)
			defendTarget = owner.GetOrigin();
		else
			defendTarget = "0 0 0";

		failureReason = "";
		m_sLastDefendTargetSource = "NONE";

		IEntity holdCenter = FindBaseHoldCenterEntity();
		if (holdCenter)
		{
			defendTarget = holdCenter.GetOrigin();
			m_sLastDefendTargetSource = "campaign_spawn_point_group";
			return true;
		}

		if (m_BaseEntity)
		{
			defendTarget = m_BaseEntity.GetOrigin();
			m_sLastDefendTargetSource = "base_entity";
			return true;
		}

		if (m_BaseComponent)
		{
			IEntity baseOwner = m_BaseComponent.GetOwner();
			if (baseOwner)
			{
				defendTarget = baseOwner.GetOrigin();
				m_sLastDefendTargetSource = "base_component_owner";
				return true;
			}
		}

		failureReason = "parent_base_missing";
		return false;
	}

	protected IEntity FindBaseHoldCenterEntity()
	{
		if (!m_BaseEntity)
			return null;

		IEntity child = m_BaseEntity.GetChildren();
		while (child)
		{
			SCR_CampaignSpawnPointGroup spawnPointGroup = SCR_CampaignSpawnPointGroup.Cast(child);
			if (spawnPointGroup)
				return child;

			child = child.GetSibling();
		}

		return null;
	}

	protected bool AssignDefendWaypointToGroup(SCR_AIGroup group, vector defendTarget, notnull array<IEntity> routeWaypoints)
	{
		routeWaypoints.Clear();
		if (!group)
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Defend assignment failed reason=group_missing wave=%1", m_iCurrentWave), true);
			return false;
		}

		JLH_DCF_NodeWaypointResolver.ClearGroupWaypoints(group);

		bool waypointAssigned = false;
		AIWaypoint assaultWaypoint = SpawnWaypoint(ASSAULT_MOVE_WAYPOINT_PREFAB, defendTarget);
		if (assaultWaypoint)
		{
			group.AddWaypoint(assaultWaypoint);
			routeWaypoints.Insert(assaultWaypoint);
			waypointAssigned = true;
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Assault move assigned target=%1 group=%2", defendTarget.ToString(), JLH_DCF_NodeDebug.EntityLabel(group)), true);
		}

		AIWaypoint defendWaypoint = SpawnDefendWaypoint(defendTarget);
		if (defendWaypoint)
		{
			group.AddWaypoint(defendWaypoint);
			routeWaypoints.Insert(defendWaypoint);
			waypointAssigned = true;
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Defend fallback assigned target=%1 group=%2", defendTarget.ToString(), JLH_DCF_NodeDebug.EntityLabel(group)), true);
		}

		if (!waypointAssigned)
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Defend assignment failed reason=waypoint_spawn_failed wave=%1 position=%2", m_iCurrentWave, defendTarget.ToString()), true);
			return false;
		}

		bool aggressiveResult = SetGroupAggressiveBehavior(group);
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Group behavior set aggressive result=%1 group=%2", JLH_DCF_NodeDebug.BoolLabel(aggressiveResult), JLH_DCF_NodeDebug.EntityLabel(group)), true);
		return true;
	}

	protected bool SetGroupAggressiveBehavior(SCR_AIGroup group)
	{
		if (!group)
			return false;

		bool result = false;
		group.ActivateAI();
		//group.ActivateAllMembers();
		result = true;

		AIControlComponent control = group.GetControlComponent();
		if (control)
		{
			control.ActivateAI();
			result = true;
		}

		return result;
	}

	protected AIWaypoint SpawnDefendWaypoint(vector defendTarget)
	{
		AIWaypoint waypoint = SpawnWaypoint(DEFEND_CP_WAYPOINT_PREFAB, defendTarget);
		if (waypoint)
			return waypoint;

		return SpawnWaypoint(DEFEND_WAYPOINT_PREFAB, defendTarget);
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

		JLH_DCF_PersistenceExclusion.StopTrackingTree(waypointEntity, "qrf_waypoint_spawn");
		return waypoint;
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

			DeleteRuntimeEntitySafe(waypointEntity, "qrf_route_waypoint");
		}

		routeWaypoints.Clear();
	}

	protected void CleanupRuntimeRouteWaypoints()
	{
		CleanupRouteWaypoints(null, m_aRuntimeRouteWaypoints);
	}

	protected int DeleteGroupAndMembers(SCR_AIGroup group)
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

			DeleteRuntimeEntitySafe(unit, "qrf_group_member");
			deleted++;
		}

		DeleteRuntimeEntitySafe(group, "qrf_group");
		return deleted;
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

	protected bool IsEmptyResource(ResourceName prefab)
	{
		return prefab == "" || prefab == ResourceName.Empty;
	}

	protected void TrackCurrentWaveGroup(SCR_AIGroup group)
	{
		if (!group)
			return;

		if (!m_aCurrentWaveGroups.Contains(group))
			m_aCurrentWaveGroups.Insert(group);

		TrackCurrentWaveGroupUnits(group);
	}

	protected void TrackCurrentWaveGroupUnits(SCR_AIGroup group)
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
			if (IsNonBlockingVehicleSupportCrew(unit))
				continue;

			if (unit && !m_aCurrentWaveUnits.Contains(unit))
				m_aCurrentWaveUnits.Insert(unit);
		}
	}

	protected bool IsNonBlockingVehicleSupportCrew(IEntity unit)
	{
		if (!unit)
			return false;

		foreach (JLH_DCF_QRFVehicleSupportRuntime support : m_aVehicleSupportsAwaitingCleanup)
		{
			if (!support || support.SupportState == JLH_DCF_QRFVehicleSupportState.ACTIVE_SUPPORT)
				continue;

			if (support.VehicleCrewUnits && support.VehicleCrewUnits.Contains(unit))
				return true;
		}

		return false;
	}

	protected void RefreshCurrentWaveUnits()
	{
		for (int i = m_aCurrentWaveGroups.Count() - 1; i >= 0; i--)
		{
			SCR_AIGroup group = m_aCurrentWaveGroups[i];
			if (!group)
			{
				m_aCurrentWaveGroups.Remove(i);
				continue;
			}

			TrackCurrentWaveGroupUnits(group);
		}

		for (int j = m_aCurrentWaveUnits.Count() - 1; j >= 0; j--)
		{
			if (!m_aCurrentWaveUnits[j])
				m_aCurrentWaveUnits.Remove(j);
		}
	}

	protected void RefreshCurrentWaveVehicles()
	{
		for (int i = m_aCurrentWaveVehicleSupports.Count() - 1; i >= 0; i--)
		{
			JLH_DCF_QRFVehicleSupportRuntime support = m_aCurrentWaveVehicleSupports[i];
			if (!support)
			{
				m_aCurrentWaveVehicleSupports.Remove(i);
				continue;
			}

			if (support.SupportState != JLH_DCF_QRFVehicleSupportState.ACTIVE_SUPPORT)
			{
				for (int k = m_aCurrentWaveVehicles.Count() - 1; k >= 0; k--)
				{
					if (m_aCurrentWaveVehicles[k] == support.VehicleEntity)
						m_aCurrentWaveVehicles.Remove(k);
				}

				continue;
			}

			if (support.VehicleEntity && !m_aCurrentWaveVehicles.Contains(support.VehicleEntity))
				m_aCurrentWaveVehicles.Insert(support.VehicleEntity);
		}

		for (int j = m_aCurrentWaveVehicles.Count() - 1; j >= 0; j--)
		{
			if (!m_aCurrentWaveVehicles[j])
				m_aCurrentWaveVehicles.Remove(j);
		}
	}

	protected int CountCurrentWaveUnits()
	{
		RefreshCurrentWaveUnits();
		RefreshCurrentWaveVehicles();
		return m_aCurrentWaveUnits.Count() + m_aCurrentWaveVehicles.Count();
	}

	protected int CountCurrentWaveVehicles()
	{
		RefreshCurrentWaveVehicles();
		return m_aCurrentWaveVehicles.Count();
	}

	protected int CountCurrentWaveSupportCrew()
	{
		int count = 0;
		foreach (JLH_DCF_QRFVehicleSupportRuntime support : m_aCurrentWaveVehicleSupports)
		{
			if (!support || !support.CrewGroup)
				continue;

			array<AIAgent> agents = {};
			support.CrewGroup.GetAgents(agents);
			foreach (AIAgent agent : agents)
			{
				if (!agent)
					continue;

				if (agent.GetControlledEntity())
					count++;
			}
		}

		return count;
	}

	protected int CountLivingCurrentWaveUnits()
	{
		RefreshCurrentWaveUnits();
		RefreshCurrentWaveVehicles();

		int alive = 0;
		foreach (IEntity unit : m_aCurrentWaveUnits)
		{
			if (IsUnitAlive(unit))
				alive++;
		}

		alive += CountLivingCurrentWaveVehicles();

		return alive;
	}

	protected int CountLivingCurrentWaveVehicles()
	{
		RefreshCurrentWaveVehicles();

		int alive = 0;
		foreach (JLH_DCF_QRFVehicleSupportRuntime support : m_aCurrentWaveVehicleSupports)
		{
			if (!support || support.SupportState != JLH_DCF_QRFVehicleSupportState.ACTIVE_SUPPORT || !support.VehicleEntity || IsEntityDestroyed(support.VehicleEntity))
				continue;

			alive++;
		}

		return alive;
	}

	protected int TrimCurrentWaveUnitsToCount(int maxLivingUnits)
	{
		RefreshCurrentWaveUnits();

		int livingSeen = 0;
		int deleted = 0;
		foreach (IEntity unit : m_aCurrentWaveUnits)
		{
			if (!IsUnitAlive(unit))
				continue;

			livingSeen++;
			if (livingSeen <= maxLivingUnits)
				continue;

			SCR_EntityHelper.DeleteEntityAndChildren(unit);
			deleted++;
		}

		RefreshCurrentWaveUnits();
		return deleted;
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

	protected void LogWaveWaiting(int alive)
	{
		if (m_iLastWaveAliveLogCount == alive)
			return;

		m_iLastWaveAliveLogCount = alive;
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Waiting for wave cleared index=%1 alive=%2", m_iCurrentWave, alive), true);
	}

	protected void LogVehicleSupportCleared()
	{
		if (!m_bCurrentWaveHadVehicleSupport || m_bCurrentWaveVehicleSupportClearedLogged)
			return;

		m_bCurrentWaveVehicleSupportClearedLogged = true;
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Vehicle support cleared wave=%1", m_iCurrentWave), true);
	}

	protected void ClearCurrentWaveTracking()
	{
		foreach (JLH_DCF_QRFVehicleSupportRuntime support : m_aCurrentWaveVehicleSupports)
		{
			ReleaseTransportLockForSupport(support, "wave_clear");
		}

		m_aCurrentWaveGroups.Clear();
		m_aCurrentWaveUnits.Clear();
		m_aCurrentWaveVehicles.Clear();
		m_aCurrentWaveInfantrySpawnPositions.Clear();
		m_aCurrentWaveVehicleSpawnPositions.Clear();
		m_aCurrentWaveVehicleHoldPositions.Clear();
		m_aCurrentWaveVehicleSupports.Clear();
		m_bVehicleSupportSpawnPending = false;
		m_iPendingVehicleSupportWave = 0;
		m_iPendingVehicleSupportCount = 0;
		m_iPendingVehicleSupportReadyTick = 0;
		m_vPendingVehicleSupportDefendTarget = "0 0 0";
	}

	protected string BuildTransportLockToken(int waveIndex, int vehicleIndex)
	{
		return "QRF:" + m_sBaseName + ":" + JLH_DCF_NodeDebug.EntityLabel(GetOwner()) + ":" + string.Format("%1", waveIndex) + ":" + string.Format("%1", vehicleIndex + 1) + ":" + string.Format("%1", System.GetTickCount());
	}

	protected void ReleaseTransportLockForSupport(JLH_DCF_QRFVehicleSupportRuntime support, string reason)
	{
		if (!support || support.TransportBaseKey == "" || support.TransportLockToken == "")
			return;

		ReleaseTransportLock(support.TransportBaseKey, support.TransportLockToken, reason);
		support.TransportBaseKey = "";
		support.TransportLockToken = "";
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

		if (reason == "wave_clear" || reason == "removed" || reason == "timeout")
			return "assault_end";

		return reason;
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

	void CleanupOwnedRuntime(string reason)
	{
		ReleaseRetainedTransportLocks(reason);

		if (m_RuntimeTracker)
			m_RuntimeTracker.CleanupOwnedRuntime(SYSTEM_NAME, reason, DebugLogging);

		CleanupRuntimeRouteWaypoints();
		ClearCurrentWaveTracking();
		m_aVehicleSupportsAwaitingCleanup.Clear();

		m_bActivationPending = false;
		m_bWaveActive = false;
		m_iPendingActivationTick = 0;
		m_iCurrentWaveSpawnTick = 0;
		m_iNextSpawnSafetyRetryTick = 0;
		m_iSpawnSafetyDeferralStartedTick = 0;
		m_iLastSpawnSafetyLogTick = 0;
		m_iLastSpawnSafetyRetryLogTick = 0;
		m_iLastWaveAliveLogCount = -1;
		m_bCurrentWaveHadTrackedUnits = false;
		m_bCurrentWaveHadVehicleSupport = false;
		m_bCurrentWaveVehicleSupportClearedLogged = false;
		m_sLastSpawnSafetyLogKey = "";
		m_SelectedSpawnNode = null;
		ClearVirtualWaveState();
		ClearPendingVirtualWaveVehiclePackages(reason);
		//SetMembersAlive(0);
		SetIsSpawned(false);
		SetIsPaused(true);
	}

	bool IsRuntimeEntityCleanupProtected(IEntity entity, JLH_DCF_RuntimeCleanupEntityKind kind, out string reason)
	{
		reason = "";
		if (!entity)
			return false;

		if (m_bActivationPending)
		{
			reason = "qrf_activation_pending";
			return true;
		}

		if (m_bWaveActive)
		{
			if (m_aCurrentWaveUnits.Contains(entity) || m_aCurrentWaveVehicles.Contains(entity))
			{
				reason = "qrf_active_wave";
				return true;
			}

			foreach (SCR_AIGroup group : m_aCurrentWaveGroups)
			{
				if (group == entity || IsGroupMember(group, entity))
				{
					reason = "qrf_active_wave";
					return true;
				}
			}
		}

		foreach (JLH_DCF_QRFVehicleSupportRuntime support : m_aCurrentWaveVehicleSupports)
		{
			if (IsVehicleSupportEntity(support, entity))
			{
				reason = "qrf_active_vehicle_support";
				return true;
			}
		}

		foreach (JLH_DCF_QRFVehicleSupportRuntime cleanupSupport : m_aVehicleSupportsAwaitingCleanup)
		{
			if (IsVehicleSupportEntity(cleanupSupport, entity))
			{
				reason = "qrf_local_vehicle_support_cleanup";
				return true;
			}
		}

		foreach (JLH_DCF_QRFVehicleSupportRuntime retainedSupport : m_aRetainedTransportSupports)
		{
			if (IsVehicleSupportEntity(retainedSupport, entity))
			{
				reason = "qrf_retained_transport_defence";
				return true;
			}
		}

		if (m_aRuntimeRouteWaypoints.Contains(entity))
		{
			reason = "qrf_route_waypoint";
			return true;
		}

		return false;
	}

	protected bool IsVehicleSupportEntity(JLH_DCF_QRFVehicleSupportRuntime support, IEntity entity)
	{
		if (!support || !entity)
			return false;

		if (support.VehicleEntity == entity || support.CrewGroup == entity)
			return true;

		if (support.VehicleCrewUnits && support.VehicleCrewUnits.Contains(entity))
			return true;

		if (support.RouteWaypoints && support.RouteWaypoints.Contains(entity))
			return true;

		return IsGroupMember(support.CrewGroup, entity);
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
		foreach (JLH_DCF_QRFVehicleSupportRuntime support : m_aRetainedTransportSupports)
		{
			ReleaseTransportLockForSupport(support, reason);
		}

		m_aRetainedTransportSupports.Clear();
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
		if (!JLH_DCF_GhostPlacement.ResolveInfantryPosition("0 0 0", false, defendTarget, groups.Count() + 1, m_aCurrentWaveInfantrySpawnPositions, QRF_VIRTUAL_WAVE_INFANTRY_SPACING_METERS, QRF_VIRTUAL_WAVE_SAFE_SPAWN_DISTANCE, spawnPosition, placementSource, skippedReason))
			return false;

		SCR_AIGroup group = SpawnGroupPrefabAt(prefab, spawnPosition);
		if (!group)
			return false;

		array<IEntity> routeWaypoints = {};
		if (!AssignDefendWaypointToGroup(group, defendTarget, routeWaypoints))
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

		RecordCurrentWaveInfantrySpawnPosition(spawnPosition);
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
		if (!JLH_DCF_GhostPlacement.ResolveInfantryPosition(infantryPackage.LastKnownPosition, infantryPackage.HasLastKnownPosition, defendTarget, packageIndex, m_aCurrentWaveInfantrySpawnPositions, QRF_VIRTUAL_WAVE_INFANTRY_SPACING_METERS, QRF_VIRTUAL_WAVE_SAFE_SPAWN_DISTANCE, spawnPosition, placementSource, skippedReason))
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Ghost defence infantry skipped base=%1 package=%2 originalPrefab=%3 preferredPosition=%4 placementSource=skipped skippedReason=%5", m_sBaseName, packageIndex, prefab, infantryPackage.LastKnownPosition.ToString(), skippedReason), true);
			return false;
		}

		SCR_AIGroup group = SpawnGroupPrefabAt(prefab, spawnPosition, infantryPackage.UnitCount);
		if (!group)
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Ghost defence infantry skipped base=%1 package=%2 originalPrefab=%3 preferredPosition=%4 resolvedPosition=%5 placementSource=%6 skippedReason=spawn_failed", m_sBaseName, packageIndex, prefab, infantryPackage.LastKnownPosition.ToString(), spawnPosition.ToString(), placementSource), true);
			return false;
		}

		array<IEntity> retainedUnits = {};
		PruneSpawnedGroupToRequestedUnits(group, infantryPackage.UnitCount, packageIndex, retainedUnits);

		array<IEntity> routeWaypoints = {};
		if (!AssignDefendWaypointToGroup(group, defendTarget, routeWaypoints))
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

		RecordCurrentWaveInfantrySpawnPosition(spawnPosition);
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

		IEntity vehicle = SpawnVehicleAtGhostPackage(vehiclePackage.VehiclePrefab, spawnPosition, vehiclePackage);
		if (!vehicle)
		{
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("Ghost defence vehicle skipped base=%1 originalPrefab=%2 preferredPosition=%3 resolvedPosition=%4 placementSource=%5 skippedReason=spawn_failed", m_sBaseName, vehiclePackage.VehiclePrefab, preferredPosition.ToString(), spawnPosition.ToString(), placementSource), true);
			return false;
		}

		SCR_AIGroup crewGroup = SpawnVehiclePassengerGroupAt(vehiclePackage.CrewPrefab, JLH_DCF_GhostPlacement.GroundPosition(spawnPosition + Vector(2, 0, 0)));
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

		JLH_DCF_VehicleMountStats stats = new JLH_DCF_VehicleMountStats();
		if (!MountVehicleCrew(crewGroup, vehicle, vehiclePackage.VehiclePrefab, stats, false))
		{
			DeleteGroupAndMembers(crewGroup);
			SCR_EntityHelper.DeleteEntityAndChildren(vehicle);
			return false;
		}

		JLH_DCF_PersistenceExclusion.StopTrackingTree(crewGroup, "qrf_ghost_vehicle_package_mounted_group");
		JLH_DCF_PersistenceExclusion.StopTrackingTree(vehicle, "qrf_ghost_vehicle_package_mounted_vehicle");

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
		m_aCurrentWaveVehicleSpawnPositions.Insert(spawnPosition);
		m_aCurrentWaveVehicleHoldPositions.Insert(spawnPosition);
		JLH_DCF_AssaultVehicleHoldRegistry.Reserve(spawnPosition);
		CollectGroupUnitsToArray(crewGroup, units);
		foreach (IEntity waypoint : routeWaypoints)
		{
			AddUniqueEntity(waypoint, waypoints);
		}

		JLH_DCF_VehicleSustainment.RegisterVehicle(SYSTEM_NAME, vehicle, crewGroup, m_sFactionKey, m_sBaseName);
		JLH_DCF_PersistenceExclusion.StopTrackingTree(crewGroup, "qrf_ghost_vehicle_package_ready_group");
		JLH_DCF_PersistenceExclusion.StopTrackingTree(vehicle, "qrf_ghost_vehicle_package_ready_vehicle");
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("ghost mounted vehicle defence activated originalPrefab=%1 preferredPosition=%2 resolvedPosition=%3 placementSource=%4 crew=%5", vehiclePackage.VehiclePrefab, preferredPosition.ToString(), spawnPosition.ToString(), placementSource, vehiclePackage.CrewCount), true);
		return true;
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
			JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("virtual_infantry_prune base=%1 wave=%2 package=%3 spawned=0 requested=%4 deleted=0 final=0", m_sBaseName, m_iCurrentWave, packageIndex, requestedUnits), true);
			return 0;
		}

		array<IEntity> liveUnits = {};
		CollectLivingGroupUnitsForVirtualWave(group, liveUnits);
		int spawned = liveUnits.Count();
		int deleted = 0;
		if (spawned > requestedUnits)
		{
			for (int i = liveUnits.Count() - 1; i >= requestedUnits; i--)
			{
				IEntity excessUnit = liveUnits[i];
				if (excessUnit)
				{
					DeleteRuntimeEntitySafe(excessUnit, "qrf_virtual_infantry_prune");
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

		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("virtual_infantry_prune base=%1 wave=%2 package=%3 spawned=%4 requested=%5 deleted=%6 final=%7", m_sBaseName, m_iCurrentWave, packageIndex, spawned, requestedUnits, deleted, finalCount), true);
		return finalCount;
	}

	protected void LogVirtualInfantryRejected(int requestedUnits, int packageIndex)
	{
		JLH_DCF_NodeDebug.Log(SYSTEM_NAME, string.Format("virtual_infantry_rejected base=%1 wave=%2 package=%3 reason=no_requested_units requested=%4", m_sBaseName, m_iCurrentWave, packageIndex, requestedUnits), true);
	}

	bool IsRegistered()
	{
		return m_bRegistered && !m_bDisabled;
	}

	string GetRegisteredBaseName()
	{
		return m_sBaseName;
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

	protected string BuildVehicleSupportContext(JLH_DCF_QRFVehicleSupportRuntime support)
	{
		if (!support)
			return string.Format("base=%1 node=%2 wave=0 slot=0 vehicle=NONE", m_sBaseName, JLH_DCF_NodeDebug.EntityLabel(GetOwner()));

		string context = string.Format("base=%1 node=%2 wave=%3 slot=%4 vehicle=%5", m_sBaseName, JLH_DCF_NodeDebug.EntityLabel(GetOwner()), support.WaveIndex, support.VehicleIndex, JLH_DCF_NodeDebug.EntityLabel(support.VehicleEntity));
		context += string.Format(" vehiclePos=%1 crewGroup=%2 prefab=%3 crewPrefab=%4", GetEntityPositionLabel(support.VehicleEntity), JLH_DCF_NodeDebug.EntityLabel(support.CrewGroup), support.VehiclePrefab, support.CrewPrefab);
		context += string.Format(" dropPending=%1 mountPending=%2", JLH_DCF_NodeDebug.BoolLabel(support.DropPending), JLH_DCF_NodeDebug.BoolLabel(support.MountPending));
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

	protected int SecondsToMs(float seconds)
	{
		if (seconds <= 0)
			return 0;

		return (int)(seconds * 1000.0);
	}
}

void JLH_DCF_QRFNodeComponent_HandleTransportDismountCheck(JLH_QRFNodeComponent node, JLH_DCF_QRFVehicleSupportRuntime support)
{
	if (node)
		node.HandleTransportDismountCheck(support);
}

void JLH_DCF_QRFNodeComponent_HandleTransportPassengerHandoff(JLH_QRFNodeComponent node, JLH_DCF_QRFVehicleSupportRuntime support)
{
	if (node)
		node.HandleTransportPassengerHandoff(support);
}

void JLH_DCF_QRFNodeComponent_HandleTransportPassengerSearchDestroyAssignment(JLH_QRFNodeComponent node, JLH_DCF_QRFVehicleSupportRuntime support)
{
	if (node)
		node.HandleTransportPassengerSearchDestroyAssignment(support);
}

void JLH_DCF_QRFNodeComponent_HandleTransportPassengerSearchDestroyDefendFallback(JLH_QRFNodeComponent node, JLH_DCF_QRFVehicleSupportRuntime support)
{
	if (node)
		node.HandleTransportPassengerSearchDestroyDefendFallback(support);
}

void JLH_DCF_QRFNodeComponent_RequestTransportPassengerExit(JLH_QRFNodeComponent node, IEntity unit, IEntity vehicle)
{
	if (node)
		node.RequestTransportPassengerExit(unit, vehicle);
}
