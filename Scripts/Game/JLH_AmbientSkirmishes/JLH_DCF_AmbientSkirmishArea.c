enum JLH_DCF_AmbientSkirmishIntensity
{
	LIGHT = 0,
	MEDIUM = 1,
	HEAVY = 2,
	RANDOM = 3
}

enum JLH_AmbientSkirmishEventType
{
	RANDOM = 0,
	INFANTRY_CLASH = 1,
	SMALL_PATROL_CONTACT = 2,
	VEHICLE_SKIRMISH = 4,
	CONVOY_AMBUSH_USSR_ATTACKS_US = 6,
	CONVOY_AMBUSH_US_ATTACKS_USSR = 7,
	SUPPLY_TRUCK_RAID = 8,
	CAMP_ASSAULT = 9,
	ROADBLOCK_BREAKTHROUGH = 10,
	DOWNED_CREW_RESCUE = 11,
	RECON_TEAM_CONTACT = 14,
	DOWNED_US_HELI_RESCUE = 15,
	ROADBLOCK_ASSAULT = 17
}

enum JLH_AmbientSkirmishVehicleRole
{
	ANY_ARMED_GROUND = 0,
	ARMED_LIGHT = 1,
	APC = 2,
	TANK = 3
}

enum JLH_AmbientSkirmishDebugLevel
{
	BASIC = 0,
	VERBOSE = 1,
	DEEP = 2
}

enum JLH_AmbientSkirmishVehicleMountState
{
	WAITING_FOR_CREW = 0,
	MOUNTING = 1,
	VEHICLE_OCCUPANCY_STABILIZING = 2,
	WAITING_FOR_MOUNT_STABLE = 3,
	WAITING_FOR_OCCUPANCY_CONFIRM = 4,
	VALIDATING_MOUNT = 5,
	OCCUPANCY_STABLE = 6,
	VALIDATED = 7,
	ROUTE_READY = 8,
	ROUTE_ASSIGN_ALLOWED = 9,
	ROUTE_ASSIGNED = 10,
	FAILED_RECOVERY = 11,
	ABORTED = 12
}

[ComponentEditorProps(category: "JLH Dynamic Conflict Framework/Ambient Skirmishes", description: "Optional legacy child marker used by JLH Ambient Skirmish areas as an exact contact-center marker.")]
class JLH_DCF_AmbientSkirmishMarkerComponentClass : ScriptComponentClass
{
}

class JLH_DCF_AmbientSkirmishMarkerComponent : ScriptComponent
{
	[Attribute("1", UIWidgets.CheckBox, "Marker can be selected by parent Ambient Skirmish areas.", category: "JLH DCF Ambient Skirmish Marker")]
	protected bool m_bEnabled;

	bool IsEnabled()
	{
		return m_bEnabled;
	}
}

class JLH_AmbientSkirmishAreaEditorBaseEntityClass : GenericEntityClass
{
}

class JLH_AmbientSkirmishAreaEditorBaseEntity : GenericEntity
{
#ifdef WORKBENCH
	override void _WB_GetBoundBox(inout vector min, inout vector max, IEntitySource src)
	{
		float radius = GetWorkbenchActivationRadius(src);
		min = Vector(-radius, -radius, -radius);
		max = Vector(radius, radius, radius);
	}

	override bool _WB_ShouldShowBoundBox(IEntitySource src)
	{
		return true;
	}

	protected float GetWorkbenchActivationRadius(IEntitySource src)
	{
		float radius = 850.0;
		if (src)
		{
			int componentCount = src.GetComponentCount();
			for (int i = 0; i < componentCount; i++)
			{
				IEntityComponentSource componentSrc = src.GetComponent(i);
				if (!componentSrc)
					continue;

				if (componentSrc.Get("m_fActivationRadius", radius))
					return Math.Clamp(radius, 1.0, 5000.0);

				string defaultRadius;
				if (componentSrc.GetDefaultAsString("m_fActivationRadius", defaultRadius))
					return Math.Clamp(defaultRadius.ToFloat(), 1.0, 5000.0);
			}
		}

		JLH_DCF_AmbientSkirmishAreaComponent area = JLH_DCF_AmbientSkirmishAreaComponent.Cast(this.FindComponent(JLH_DCF_AmbientSkirmishAreaComponent));
		if (area)
			return area.GetEditorVisualizerRadius();

		return radius;
	}
#endif
}

[EntityEditorProps(category: "JLH DCF/Ambient Skirmishes", description: "Visible white Ambient Skirmish activation area. Place scenario marker children inside it to choose specific encounters.", color: "255 255 255 255", visible: true, style: "sphere", sizeMin: "-850 -850 -850", sizeMax: "850 850 850", color2: "255 255 255 35", dynamicBox: true)]
class JLH_AmbientSkirmishAreaEntityClass : JLH_AmbientSkirmishAreaEditorBaseEntityClass {}
class JLH_AmbientSkirmishAreaEntity : JLH_AmbientSkirmishAreaEditorBaseEntity {}

class JLH_AmbientSkirmishMarkerEditorBaseEntityClass : GenericEntityClass
{
}

class JLH_AmbientSkirmishMarkerEditorBaseEntity : GenericEntity
{
#ifdef WORKBENCH
	override void _WB_GetBoundBox(inout vector min, inout vector max, IEntitySource src)
	{
		min = Vector(-12.0, -1.0, -12.0);
		max = Vector(12.0, 16.0, 12.0);
	}

	override bool _WB_ShouldShowBoundBox(IEntitySource src)
	{
		return true;
	}
#endif
}

[EntityEditorProps(category: "JLH DCF/Ambient Skirmishes", description: "Visible white Ambient Skirmish marker. Place as a child of JLH_AmbientSkirmishArea.", color: "255 255 255 255", visible: true, style: "sphere", sizeMin: "-12 -1 -12", sizeMax: "12 16 12", color2: "255 255 255 90", dynamicBox: true)]
class JLH_AmbientSkirmishMarkerEntityClass : JLH_AmbientSkirmishMarkerEditorBaseEntityClass {}
class JLH_AmbientSkirmishMarkerEntity : JLH_AmbientSkirmishMarkerEditorBaseEntity {}

class JLH_AmbientSkirmishEventDefinition
{
	JLH_AmbientSkirmishEventType SkirmishEventType;
	string SkirmishEventName;
	JLH_DCF_AmbientGroupRole FriendlyRole;
	JLH_DCF_AmbientGroupRole EnemyRole;
	int FriendlyMinUnits;
	int FriendlyMaxUnits;
	int EnemyMinUnits;
	int EnemyMaxUnits;
	float SeparationMultiplier;
	string EncounterRole;
	string CompositionType;
	string SpacingProfile;
	string CombatProfile;
	string RouteProfile;
	string ConvoyProfile;
	string AmbushProfile;
	string RescueProfile;
	float LayoutSeparationScale;
	float LayoutLateralOffset;
	float FriendlyDepthOffset;
	float EnemyDepthOffset;
	int FriendlySearchPoints;
	int EnemySearchPoints;
	bool RequiresVehicles;
	bool RequiresTanks;
	bool RequiresSupplyTruck;
	bool RequiresStaticWeapons;
	bool UsesArtilleryFX;
	bool Implemented;
	string NotImplementedReason;
	ResourceName FriendlyVehiclePrefab;
	ResourceName EnemyVehiclePrefab;
	ResourceName FriendlySupportVehiclePrefab;
	ResourceName EnemySupportVehiclePrefab;
	string FriendlyVehicleSource;
	string EnemyVehicleSource;
	string FriendlySupportVehicleSource;
	string EnemySupportVehicleSource;
	ref array<string> RequiredVehicleLabels = {};
	ref array<string> PreferredVehicleLabels = {};
	ref array<string> ExcludedVehicleLabels = {};
	ref array<string> FallbackVehicleLabels = {};

	void Init(JLH_AmbientSkirmishEventType eventType, string eventName, JLH_DCF_AmbientGroupRole friendlyRole, JLH_DCF_AmbientGroupRole enemyRole, int friendlyMinUnits, int friendlyMaxUnits, int enemyMinUnits, int enemyMaxUnits, float separationMultiplier)
	{
		SkirmishEventType = eventType;
		SkirmishEventName = eventName;
		FriendlyRole = friendlyRole;
		EnemyRole = enemyRole;
		FriendlyMinUnits = friendlyMinUnits;
		FriendlyMaxUnits = friendlyMaxUnits;
		EnemyMinUnits = enemyMinUnits;
		EnemyMaxUnits = enemyMaxUnits;
		SeparationMultiplier = separationMultiplier;
		EncounterRole = "balanced_contact";
		CompositionType = "standard_infantry";
		SpacingProfile = "moderate_line";
		CombatProfile = "balanced_firefight";
		RouteProfile = "none";
		ConvoyProfile = "none";
		AmbushProfile = "none";
		RescueProfile = "none";
		LayoutSeparationScale = 1.0;
		LayoutLateralOffset = 0.0;
		FriendlyDepthOffset = 0.0;
		EnemyDepthOffset = 0.0;
		FriendlySearchPoints = 2;
		EnemySearchPoints = 3;
		Implemented = true;
		NotImplementedReason = "";
	}

	void SetProfiles(string encounterRole, string compositionType, string spacingProfile, string combatProfile, string routeProfile, string convoyProfile = "none", string ambushProfile = "none", string rescueProfile = "none")
	{
		EncounterRole = encounterRole;
		CompositionType = compositionType;
		SpacingProfile = spacingProfile;
		CombatProfile = combatProfile;
		RouteProfile = routeProfile;
		ConvoyProfile = convoyProfile;
		AmbushProfile = ambushProfile;
		RescueProfile = rescueProfile;
	}

	void SetLayoutTuning(float separationScale, float lateralOffset, float friendlyDepthOffset, float enemyDepthOffset)
	{
		LayoutSeparationScale = Math.Clamp(separationScale, 0.35, 2.4);
		LayoutLateralOffset = Math.Clamp(lateralOffset, -220.0, 220.0);
		FriendlyDepthOffset = Math.Clamp(friendlyDepthOffset, -260.0, 260.0);
		EnemyDepthOffset = Math.Clamp(enemyDepthOffset, -260.0, 260.0);
	}

	void SetCombatTuning(int friendlySearchPoints, int enemySearchPoints)
	{
		FriendlySearchPoints = Math.Clamp(friendlySearchPoints, 1, 5);
		EnemySearchPoints = Math.Clamp(enemySearchPoints, 1, 5);
	}

	void RequireVehicleExecutor(bool tank, bool supplyTruck)
	{
		RequiresVehicles = true;
		RequiresTanks = tank;
		RequiresSupplyTruck = supplyTruck;
		Implemented = true;
		NotImplementedReason = "";
		AddDefaultGroundVehicleExclusions();
	}

	void AddRequiredVehicleLabel(string labelName)
	{
		AddVehicleLabel(RequiredVehicleLabels, labelName);
	}

	void AddPreferredVehicleLabel(string labelName)
	{
		AddVehicleLabel(PreferredVehicleLabels, labelName);
	}

	void AddExcludedVehicleLabel(string labelName)
	{
		AddVehicleLabel(ExcludedVehicleLabels, labelName);
	}

	void AddFallbackVehicleLabel(string labelName)
	{
		AddVehicleLabel(FallbackVehicleLabels, labelName);
	}

	void AddDefaultGroundVehicleExclusions()
	{
		AddExcludedVehicleLabel("VEHICLE_HELICOPTER");
		AddExcludedVehicleLabel("VEHICLE_AIRPLANE");
		AddExcludedVehicleLabel("VEHICLE_AIRCRAFT");
		AddExcludedVehicleLabel("VEHICLE_PLANE");
		AddExcludedVehicleLabel("VEHICLE_BOAT");
		AddExcludedVehicleLabel("VEHICLE_SHIP");
		AddExcludedVehicleLabel("VEHICLE_WATERCRAFT");
		AddExcludedVehicleLabel("VEHICLE_SUBMARINE");
	}

	void ApplyEditableVehicleLabelOverrides(array<EEditableEntityLabel> requiredLabels, array<EEditableEntityLabel> preferredLabels, array<EEditableEntityLabel> excludedLabels, array<EEditableEntityLabel> fallbackLabels)
	{
		if (HasEditableLabels(requiredLabels))
			CopyEditableLabelsToNames(requiredLabels, RequiredVehicleLabels);

		if (HasEditableLabels(preferredLabels))
			CopyEditableLabelsToNames(preferredLabels, PreferredVehicleLabels);

		if (HasEditableLabels(excludedLabels))
			CopyEditableLabelsToNames(excludedLabels, ExcludedVehicleLabels);

		if (HasEditableLabels(fallbackLabels))
			CopyEditableLabelsToNames(fallbackLabels, FallbackVehicleLabels);
	}

	protected bool HasEditableLabels(array<EEditableEntityLabel> labels)
	{
		if (!labels || labels.IsEmpty())
			return false;

		foreach (EEditableEntityLabel label : labels)
		{
			string labelName = SCR_Enum.GetEnumName(EEditableEntityLabel, label);
			if (labelName != "" && labelName != "NONE")
				return true;
		}

		return false;
	}

	protected void CopyEditableLabelsToNames(array<EEditableEntityLabel> sourceLabels, notnull array<string> targetLabels)
	{
		targetLabels.Clear();
		if (!sourceLabels)
			return;

		foreach (EEditableEntityLabel label : sourceLabels)
		{
			string labelName = SCR_Enum.GetEnumName(EEditableEntityLabel, label);
			if (labelName == "" || labelName == "NONE")
				continue;

			AddVehicleLabel(targetLabels, labelName);
		}
	}

	protected void AddVehicleLabel(notnull array<string> labels, string labelName)
	{
		if (labelName == "" || labelName == "NONE")
			return;

		if (!labels.Contains(labelName))
			labels.Insert(labelName);
	}
}

class JLH_AmbientSkirmishVehicleMountLog
{
	string SideLabel;
	int VehicleIndex;
	ResourceName VehiclePrefab;
	IEntity VehicleEntity;
	IEntity MountTargetEntity;
	SCR_AIGroup CrewGroup;
	IEntity DriverEntity;
	IEntity GunnerEntity;
	IEntity CommanderEntity;
	int CrewSeen;
	int UnitsSeen;
	int TargetCrew;
	int ExpectedCrew;
	int ReadyCrew;
	int MountedCount;
	bool DriverMounted;
	bool GunnerMounted;
	bool CommanderMounted;
	bool ArmedRequired;
	bool TurretFound;
	bool GunnerCompatible;
	int DriverCompartmentCount;
	int TurretCompartmentCount;
	int CommanderCompartmentCount;
	string DriverCompartment;
	string GunnerCompartment;
	string CommanderCompartment;
	int Unmoved;
	int DeletedUnused;
	string FailureReason;
}

class JLH_AmbientSkirmishVehicleLifecycle
{
	int MountAttemptCount;
	int ValidationFailureCount;
	int LastMountAttemptTime;
	int LastValidationTime;
	int LastRecoveryTime;
	int LastOccupancyPendingTime;
	int PendingMountedCount;
	bool PendingDriverSeat;
	bool PendingGunnerSeat;
	bool PendingCommanderSeat;
	string PendingDriverCompartment;
	string PendingGunnerCompartment;
	string PendingCommanderCompartment;
	bool TemporaryHoldAssigned;
	bool RouteAssignedAfterStableMount;
	bool MountValidated;
	bool RoutePendingLogged;
	bool IdleStateLogged;
	bool DismountPreventedLogged;
	bool FullCrewPopulationConfirmed;
	string FailedReason;
	bool RecoveryLocked;
	JLH_AmbientSkirmishVehicleMountState State;
}

class JLH_AmbientSkirmishVehicleMonitorRecord
{
	bool FriendlySide;
	string SideLabel;
	int VehicleIndex;
	int VehicleCount;
	int CrewCount;
	SCR_AIGroup CrewGroup;
	IEntity VehicleEntity;
	ResourceName VehiclePrefab;
	IEntity Driver;
	IEntity Gunner;
	IEntity Passenger;
	JLH_AmbientSkirmishVehicleLifecycle Lifecycle;
	bool GunnerDismountLogged;
	bool PassengerDismountLogged;
}

class JLH_AmbientSkirmishRecord
{
	int Id;
	string NodeKey;
	string NodeName;
	JLH_AmbientSkirmishEventType SkirmishEventType;
	string SkirmishEventName;
	string FriendlyFactionKey;
	string EnemyFactionKey;
	string FriendlySource;
	string EnemySource;
	int FriendlyUnits;
	int EnemyUnits;
	vector Center;
	vector FriendlyPosition;
	vector EnemyPosition;
	int CreatedTick;
	bool m_bCleared;
	int m_iClearedTick;
	SCR_AIGroup FriendlyGroup;
	SCR_AIGroup EnemyGroup;
	ref array<SCR_AIGroup> FriendlyGroups = {};
	ref array<SCR_AIGroup> EnemyGroups = {};
	ref array<string> FriendlyGroupSources = {};
	ref array<string> EnemyGroupSources = {};
	ref array<IEntity> FriendlyVehicles = {};
	ref array<IEntity> EnemyVehicles = {};
	ref array<ResourceName> FriendlyVehiclePrefabs = {};
	ref array<ResourceName> EnemyVehiclePrefabs = {};
	ref array<IEntity> FriendlyVehicleDrivers = {};
	ref array<IEntity> FriendlyVehicleGunners = {};
	ref array<IEntity> EnemyVehicleDrivers = {};
	ref array<IEntity> EnemyVehicleGunners = {};
	ref array<IEntity> FriendlyVehiclePassengers = {};
	ref array<IEntity> EnemyVehiclePassengers = {};
	ref array<bool> FriendlyVehicleGunnerDismountLogged = {};
	ref array<bool> EnemyVehicleGunnerDismountLogged = {};
	ref array<bool> FriendlyVehiclePassengerDismountLogged = {};
	ref array<bool> EnemyVehiclePassengerDismountLogged = {};
	ref array<ref JLH_AmbientSkirmishVehicleLifecycle> FriendlyVehicleLifecycles = {};
	ref array<ref JLH_AmbientSkirmishVehicleLifecycle> EnemyVehicleLifecycles = {};
	ref JLH_AddonRuntimeEntityRegistry Runtime = new JLH_AddonRuntimeEntityRegistry();
	bool FriendlyVehicleMounted;
	bool EnemyVehicleMounted;
	bool VehicleBehaviorAssigned;
	bool VehiclePostRouteValidationScheduled;
	bool VehiclePostRouteValidationComplete;
	IEntity ConvoyAmbushTruck;
	SCR_AIGroup ConvoyAmbushUSGroup;
	ref array<SCR_AIGroup> ConvoyAmbushTruckGroups = {};
	SCR_AIGroup ConvoyAmbushFrontCasualtyGroup;
	SCR_AIGroup ConvoyAmbushRearCasualtyGroup;
	bool ConvoyAmbushDismountScheduled;
	bool ConvoyAmbushRearDestroyScheduled;

	void Init(int id, string nodeKey, string nodeName, JLH_AmbientSkirmishEventDefinition definition, notnull array<SCR_AIGroup> friendlyGroups, notnull array<SCR_AIGroup> enemyGroups, vector center, vector friendlyPosition, vector enemyPosition, JLH_AmbientSkirmishGroupPlan friendlyPlan, JLH_AmbientSkirmishGroupPlan enemyPlan)
	{
		Id = id;
		NodeKey = nodeKey;
		NodeName = nodeName;
		if (definition)
		{
			SkirmishEventType = definition.SkirmishEventType;
			SkirmishEventName = definition.SkirmishEventName;
		}
		FriendlyGroups.Clear();
		EnemyGroups.Clear();
		FriendlyGroupSources.Clear();
		EnemyGroupSources.Clear();
		for (int friendlyIndex = 0; friendlyIndex < friendlyGroups.Count(); friendlyIndex++)
		{
			SCR_AIGroup friendlyGroup = friendlyGroups[friendlyIndex];
			if (friendlyGroup)
			{
				FriendlyGroups.Insert(friendlyGroup);
				FriendlyGroupSources.Insert(GetPlanGroupSource(friendlyPlan, friendlyIndex));
			}
		}

		for (int enemyIndex = 0; enemyIndex < enemyGroups.Count(); enemyIndex++)
		{
			SCR_AIGroup enemyGroup = enemyGroups[enemyIndex];
			if (enemyGroup)
			{
				EnemyGroups.Insert(enemyGroup);
				EnemyGroupSources.Insert(GetPlanGroupSource(enemyPlan, enemyIndex));
			}
		}

		if (!FriendlyGroups.IsEmpty())
			FriendlyGroup = FriendlyGroups[0];
		if (!EnemyGroups.IsEmpty())
			EnemyGroup = EnemyGroups[0];
		Center = center;
		FriendlyPosition = friendlyPosition;
		EnemyPosition = enemyPosition;
		CreatedTick = System.GetTickCount();
		m_bCleared = false;
		m_iClearedTick = 0;
		Runtime.SetRuntimeOwner("AmbientSkirmish");

		if (friendlyPlan)
		{
			FriendlyFactionKey = friendlyPlan.PlanFactionKey;
			FriendlySource = friendlyPlan.Source;
			FriendlyUnits = friendlyPlan.TotalUnits;
		}

		if (enemyPlan)
		{
			EnemyFactionKey = enemyPlan.PlanFactionKey;
			EnemySource = enemyPlan.Source;
			EnemyUnits = enemyPlan.TotalUnits;
		}

		for (int i = 0; i < FriendlyGroups.Count(); i++)
			Runtime.TrackGroup(FriendlyGroups[i], "ambient_skirmish_friendly_group_" + i.ToString());

		for (int j = 0; j < EnemyGroups.Count(); j++)
			Runtime.TrackGroup(EnemyGroups[j], "ambient_skirmish_enemy_group_" + j.ToString());
	}

	void AttachVehicles(notnull array<IEntity> friendlyVehicles, notnull array<IEntity> enemyVehicles)
	{
		FriendlyVehicles.Clear();
		EnemyVehicles.Clear();
		FriendlyVehicleDrivers.Clear();
		FriendlyVehicleGunners.Clear();
		EnemyVehicleDrivers.Clear();
		EnemyVehicleGunners.Clear();
		FriendlyVehiclePassengers.Clear();
		EnemyVehiclePassengers.Clear();
		FriendlyVehicleGunnerDismountLogged.Clear();
		EnemyVehicleGunnerDismountLogged.Clear();
		FriendlyVehiclePassengerDismountLogged.Clear();
		EnemyVehiclePassengerDismountLogged.Clear();
		FriendlyVehicleLifecycles.Clear();
		EnemyVehicleLifecycles.Clear();

		foreach (IEntity friendlyVehicle : friendlyVehicles)
		{
			if (friendlyVehicle)
			{
				FriendlyVehicles.Insert(friendlyVehicle);
				FriendlyVehicleDrivers.Insert(null);
				FriendlyVehicleGunners.Insert(null);
				FriendlyVehiclePassengers.Insert(null);
				FriendlyVehicleGunnerDismountLogged.Insert(false);
				FriendlyVehiclePassengerDismountLogged.Insert(false);
				FriendlyVehicleLifecycles.Insert(new JLH_AmbientSkirmishVehicleLifecycle());
			}
		}

		foreach (IEntity enemyVehicle : enemyVehicles)
		{
			if (enemyVehicle)
			{
				EnemyVehicles.Insert(enemyVehicle);
				EnemyVehicleDrivers.Insert(null);
				EnemyVehicleGunners.Insert(null);
				EnemyVehiclePassengers.Insert(null);
				EnemyVehicleGunnerDismountLogged.Insert(false);
				EnemyVehiclePassengerDismountLogged.Insert(false);
				EnemyVehicleLifecycles.Insert(new JLH_AmbientSkirmishVehicleLifecycle());
			}
		}

		for (int i = 0; i < FriendlyVehicles.Count(); i++)
			Runtime.TrackVehicle(FriendlyVehicles[i], "ambient_skirmish_friendly_vehicle_" + i.ToString());

		for (int j = 0; j < EnemyVehicles.Count(); j++)
			Runtime.TrackVehicle(EnemyVehicles[j], "ambient_skirmish_enemy_vehicle_" + j.ToString());
	}

	void AttachVehiclePrefabs(array<ResourceName> friendlyVehiclePrefabs, array<ResourceName> enemyVehiclePrefabs)
	{
		FriendlyVehiclePrefabs.Clear();
		EnemyVehiclePrefabs.Clear();

		if (friendlyVehiclePrefabs)
		{
			foreach (ResourceName friendlyPrefab : friendlyVehiclePrefabs)
			{
				FriendlyVehiclePrefabs.Insert(friendlyPrefab);
			}
		}

		if (enemyVehiclePrefabs)
		{
			foreach (ResourceName enemyPrefab : enemyVehiclePrefabs)
			{
				EnemyVehiclePrefabs.Insert(enemyPrefab);
			}
		}
	}

	ResourceName GetFriendlyVehiclePrefab(int index, ResourceName fallbackPrefab)
	{
		if (index >= 0 && index < FriendlyVehiclePrefabs.Count() && FriendlyVehiclePrefabs[index] != "")
			return FriendlyVehiclePrefabs[index];

		return fallbackPrefab;
	}

	ResourceName GetEnemyVehiclePrefab(int index, ResourceName fallbackPrefab)
	{
		if (index >= 0 && index < EnemyVehiclePrefabs.Count() && EnemyVehiclePrefabs[index] != "")
			return EnemyVehiclePrefabs[index];

		return fallbackPrefab;
	}

	void SetVehicleCrew(bool friendlySide, int index, IEntity driver, IEntity gunner)
	{
		if (index < 0)
			return;

		if (friendlySide)
		{
			EnsureVehicleCrewSlots(FriendlyVehicleDrivers, FriendlyVehicleGunners, index);
			FriendlyVehicleDrivers[index] = driver;
			FriendlyVehicleGunners[index] = gunner;
			return;
		}

		EnsureVehicleCrewSlots(EnemyVehicleDrivers, EnemyVehicleGunners, index);
		EnemyVehicleDrivers[index] = driver;
		EnemyVehicleGunners[index] = gunner;
	}

	IEntity GetVehicleDriver(bool friendlySide, int index)
	{
		if (index < 0)
			return null;

		if (friendlySide)
		{
			if (index < FriendlyVehicleDrivers.Count())
				return FriendlyVehicleDrivers[index];

			return null;
		}

		if (index < EnemyVehicleDrivers.Count())
			return EnemyVehicleDrivers[index];

		return null;
	}

	IEntity GetVehicleGunner(bool friendlySide, int index)
	{
		if (index < 0)
			return null;

		if (friendlySide)
		{
			if (index < FriendlyVehicleGunners.Count())
				return FriendlyVehicleGunners[index];

			return null;
		}

		if (index < EnemyVehicleGunners.Count())
			return EnemyVehicleGunners[index];

		return null;
	}

	void SetVehiclePassenger(bool friendlySide, int index, IEntity passenger)
	{
		if (index < 0 || !passenger)
			return;

		if (friendlySide)
		{
			EnsureVehicleDiagnosticSlots(FriendlyVehiclePassengers, FriendlyVehicleGunnerDismountLogged, FriendlyVehiclePassengerDismountLogged, index);
			FriendlyVehiclePassengers[index] = passenger;
			return;
		}

		EnsureVehicleDiagnosticSlots(EnemyVehiclePassengers, EnemyVehicleGunnerDismountLogged, EnemyVehiclePassengerDismountLogged, index);
		EnemyVehiclePassengers[index] = passenger;
	}

	IEntity GetVehiclePassenger(bool friendlySide, int index)
	{
		if (index < 0)
			return null;

		if (friendlySide)
		{
			if (index < FriendlyVehiclePassengers.Count())
				return FriendlyVehiclePassengers[index];

			return null;
		}

		if (index < EnemyVehiclePassengers.Count())
			return EnemyVehiclePassengers[index];

		return null;
	}

	bool IsVehicleDismountLogged(bool friendlySide, int index, string role)
	{
		if (index < 0)
			return false;

		if (friendlySide)
		{
			EnsureVehicleDiagnosticSlots(FriendlyVehiclePassengers, FriendlyVehicleGunnerDismountLogged, FriendlyVehiclePassengerDismountLogged, index);
			if (role == "gunner")
				return FriendlyVehicleGunnerDismountLogged[index];

			return FriendlyVehiclePassengerDismountLogged[index];
		}

		EnsureVehicleDiagnosticSlots(EnemyVehiclePassengers, EnemyVehicleGunnerDismountLogged, EnemyVehiclePassengerDismountLogged, index);
		if (role == "gunner")
			return EnemyVehicleGunnerDismountLogged[index];

		return EnemyVehiclePassengerDismountLogged[index];
	}

	void SetVehicleDismountLogged(bool friendlySide, int index, string role, bool logged)
	{
		if (index < 0)
			return;

		if (friendlySide)
		{
			EnsureVehicleDiagnosticSlots(FriendlyVehiclePassengers, FriendlyVehicleGunnerDismountLogged, FriendlyVehiclePassengerDismountLogged, index);
			if (role == "gunner")
				FriendlyVehicleGunnerDismountLogged[index] = logged;
			else
				FriendlyVehiclePassengerDismountLogged[index] = logged;

			return;
		}

		EnsureVehicleDiagnosticSlots(EnemyVehiclePassengers, EnemyVehicleGunnerDismountLogged, EnemyVehiclePassengerDismountLogged, index);
		if (role == "gunner")
			EnemyVehicleGunnerDismountLogged[index] = logged;
		else
			EnemyVehiclePassengerDismountLogged[index] = logged;
	}

	JLH_AmbientSkirmishVehicleLifecycle GetVehicleLifecycle(bool friendlySide, int index)
	{
		if (index < 0)
			return null;

		if (friendlySide)
		{
			EnsureVehicleLifecycleSlots(FriendlyVehicleLifecycles, index);
			return FriendlyVehicleLifecycles[index];
		}

		EnsureVehicleLifecycleSlots(EnemyVehicleLifecycles, index);
		return EnemyVehicleLifecycles[index];
	}

	protected void EnsureVehicleCrewSlots(notnull array<IEntity> drivers, notnull array<IEntity> gunners, int index)
	{
		while (drivers.Count() <= index)
			drivers.Insert(null);

		while (gunners.Count() <= index)
			gunners.Insert(null);
	}

	protected void EnsureVehicleDiagnosticSlots(notnull array<IEntity> passengers, notnull array<bool> gunnerLogged, notnull array<bool> passengerLogged, int index)
	{
		while (passengers.Count() <= index)
			passengers.Insert(null);

		while (gunnerLogged.Count() <= index)
			gunnerLogged.Insert(false);

		while (passengerLogged.Count() <= index)
			passengerLogged.Insert(false);
	}

	protected void EnsureVehicleLifecycleSlots(notnull array<ref JLH_AmbientSkirmishVehicleLifecycle> lifecycles, int index)
	{
		while (lifecycles.Count() <= index)
			lifecycles.Insert(new JLH_AmbientSkirmishVehicleLifecycle());
	}

	void AttachFriendlyGroup(SCR_AIGroup group, string label, int estimatedUnits, string source)
	{
		if (!group)
			return;

		FriendlyGroups.Insert(group);
		FriendlyGroupSources.Insert(source);
		if (!FriendlyGroup)
			FriendlyGroup = group;

		if (estimatedUnits > 0)
			FriendlyUnits += estimatedUnits;

		if (source != "")
			FriendlySource = FriendlySource + "+delayed:" + source;

		Runtime.TrackGroup(group, label);
	}

	protected string GetPlanGroupSource(JLH_AmbientSkirmishGroupPlan plan, int index)
	{
		if (!plan)
			return "";

		if (index >= 0 && index < plan.Groups.Count() && plan.Groups[index])
			return plan.Groups[index].Source;

		return plan.Source;
	}

	string GetFriendlyGroupSource(int index)
	{
		if (index >= 0 && index < FriendlyGroupSources.Count())
			return FriendlyGroupSources[index];

		return FriendlySource;
	}

	string GetEnemyGroupSource(int index)
	{
		if (index >= 0 && index < EnemyGroupSources.Count())
			return EnemyGroupSources[index];

		return EnemySource;
	}

	bool IsActive()
	{
		return CountAliveGroups(FriendlyGroups) > 0 || CountAliveGroups(EnemyGroups) > 0;
	}

	bool BothSidesDead()
	{
		return CountAliveGroups(FriendlyGroups) <= 0 && CountAliveGroups(EnemyGroups) <= 0;
	}

	protected int CountAliveGroups(notnull array<SCR_AIGroup> groups)
	{
		int alive = 0;
		foreach (SCR_AIGroup group : groups)
		{
			if (group)
				alive += JLH_AddonSpawnUtility.CountAliveGroupUnits(group);
		}

		return alive;
	}
}

[ComponentEditorProps(category: "JLH Dynamic Conflict Framework/Ambient Skirmishes", description: "Editor-placeable ambient skirmish activation area. Child scenario markers define the specific encounter; the area can use random fallback when none are present.")]
class JLH_DCF_AmbientSkirmishAreaComponentClass : ScriptComponentClass
{
}

class JLH_DCF_AmbientSkirmishAreaComponent : ScriptComponent
{
	protected static const string LOG_PREFIX = "[AMBIENT_SKIRMISH]";
	protected static const string CONVOY_AMBUSH_LOG_PREFIX = "[JLH_DCF][Skirmish][ConvoyAmbush]";
	protected static const string SYSTEM_NAME = "AMBIENT_SKIRMISH";
	protected static const int SPAWN_CANDIDATE_ATTEMPTS = 30;
	protected static const int LOG_THROTTLE_MS = 30000;
	protected static const int NO_LIVE_DEPLOYED_HUMAN_DEBUG_REPEAT_MS = 240000;
	protected static const int CRASH_SURVIVOR_LAYOUT_RETRY_MS = 250;
	protected static const int CRASH_SURVIVOR_LAYOUT_MAX_ATTEMPTS = 8;
	protected static const int CRASH_SURVIVOR_REINFORCEMENT_DELAY_MS = 1000;
	protected static const int DOWNED_US_HELI_SURVIVOR_POST_BLAST_BUFFER_MS = 450;
	protected static const int VEHICLE_MOUNT_RETRY_DELAY_MS = 3500;
	protected static const int VEHICLE_MOUNT_MAX_ATTEMPTS = 3;
	protected static const int VEHICLE_CREW_READY_MAX_ATTEMPTS = 5;
	protected static const int VEHICLE_MOUNT_VALIDATION_DELAY_MS = 5500;
	protected static const int VEHICLE_MOUNT_VALIDATION_MAX_ATTEMPTS = 3;
	protected static const int VEHICLE_MOUNT_VALIDATION_COOLDOWN_MS = 2500;
	protected static const int VEHICLE_OCCUPANCY_CONFIRM_TIMEOUT_MS = 14000;
	protected static const int VEHICLE_MOUNT_RECOVERY_DELAY_MS = 3000;
	protected static const int VEHICLE_POST_ROUTE_VALIDATION_DELAY_MS = 2200;
	protected static const int VEHICLE_VALIDATION_MAX_FAILURES = 3;
	protected static const int VEHICLE_SKIRMISH_MAX_MOUNTED_CREW = 2;
	protected static const int VEHICLE_SKIRMISH_MAX_CARGO_CREW = 1;
	protected static const int CONVOY_AMBUSH_US_DISMOUNT_DELAY_MS = 1200;
	protected static const int CONVOY_AMBUSH_REAR_DESTROY_MIN_DELAY_MS = 3000;
	protected static const int CONVOY_AMBUSH_REAR_DESTROY_MAX_DELAY_MS = 8000;
	protected static const float CONVOY_AMBUSH_AREA_ROAD_SEARCH_MIN_RADIUS = 220.0;
	protected static const float CONVOY_AMBUSH_AREA_ROAD_SAMPLE_MIN_SNAP = 70.0;
	protected static const float CONVOY_AMBUSH_AREA_ROAD_SAMPLE_MAX_SNAP = 140.0;
	protected static const float CONVOY_AMBUSH_ROAD_WATER_CLEARANCE_RADIUS = 8.0;
	protected static const float CONVOY_AMBUSH_MAX_ROAD_SLOPE_DEGREES = 18.0;
	protected static const ResourceName CONVOY_AMBUSH_DEFAULT_TROOP_TRUCK = "{81FDAD5EB644CC3D}Prefabs/Vehicles/Wheeled/M923A1/M923A1_transport_covered.et";
	protected static const ResourceName CONVOY_AMBUSH_DEFAULT_USSR_TROOP_TRUCK = "{D9B91FAB817A6033}Prefabs/Vehicles/Wheeled/Ural4320/Ural4320_transport_covered.et";
	protected static const ResourceName DEFAULT_USSR_VEHICLE_CREW_GROUP_PREFAB = "{D4CF0B2B840CF001}Prefabs/Groups/JLH_DCF/JLH_USSR_VehicleGroup.et";
	protected static const ResourceName DEFAULT_US_VEHICLE_CREW_GROUP_PREFAB = "{101C1DACCD535E45}Prefabs/Groups/JLH_DCF/JLH_US_VehicleGroup.et";
	protected static const int FULL_VEHICLE_CREW_GROUP_EXPECTED_UNITS = 20;
	protected static const ResourceName SCENE_US_HELI_LIVE = "{70BAEEFC2D3FEE64}Prefabs/Vehicles/Helicopters/UH1H/UH1H.et";
	protected static const ResourceName SCENE_US_DOWNED_HUMVEE = "{3EA6F47D95867114}Prefabs/Vehicles/Wheeled/M998/M1025_armed_M2HB.et";
	protected static const ResourceName VEHICLE_SKIRMISH_DEFAULT_US_ARMED = "{3EA6F47D95867114}Prefabs/Vehicles/Wheeled/M998/M1025_armed_M2HB.et";
	protected static const ResourceName SCENE_US_DOWNED_TRUCK = "{A3786C5C8687F004}Prefabs/Vehicles/Wheeled/M923A1/M923A1_transport_MERDC_filled.et";
	protected static const ResourceName SUPPLY_TRUCK_RAID_LOADED_TRUCK = "{A3786C5C8687F004}Prefabs/Vehicles/Wheeled/M923A1/M923A1_transport_MERDC_filled.et";
	protected static const ResourceName SCENE_USSR_UAZ = "{259EE7B78C51B624}Prefabs/Vehicles/Wheeled/UAZ469/UAZ469.et";
	protected static const ResourceName SCENE_USSR_URAL = "{16C1F16C9B053801}Prefabs/Vehicles/Wheeled/Ural4320/Ural4320_transport.et";
	protected static const ResourceName SCENE_TIRE_UAZ = "{6095B175AA9804DC}Prefabs/Props/VehicleParts/Tires/Tire_UAZ469.et";
	protected static const ResourceName SCENE_CAMP_ASSAULT_US = "{91E31502A3FA3000}Prefabs/Compositions/JLH_DCF/JLH_US_Camp.et";
	protected static const ResourceName SCENE_ROADBLOCK_USSR = "{83A10E8547281E58}Prefabs/Compositions/Slotted/SlotRoadSmall/Barricade_S_USSR_01.et";
	protected static const ResourceName ROADBLOCK_ASSAULT_COMPOSITION_USSR = "{2A27606856B8A914}Prefabs/Compositions/Slotted/SlotRoadLarge/Barricade_L_USSR_01.et";
	protected static const ResourceName ROADBLOCK_BREAKTHROUGH_COMPOSITION_MAIN_USSR = "{9F9924B626C5FA2C}Prefabs/Compositions/Slotted/SlotRoadLarge/Checkpoint_L_USSR_01.et";
	protected static const ResourceName ROADBLOCK_BREAKTHROUGH_COMPOSITION_DIRT_USSR = "{7C85836D444E3797}Prefabs/Compositions/Slotted/SlotRoadMedium/Checkpoint_M_USSR_01.et";
	protected static const ResourceName ROADBLOCK_ASSAULT_GROUP_US = "{84E5BBAB25EA23E5}Prefabs/Groups/BLUFOR/Group_US_FireTeam.et";
	protected static const ResourceName ROADBLOCK_ASSAULT_GROUP_USSR_AT = "{96BAB56E6558788E}Prefabs/Groups/OPFOR/Group_USSR_Team_AT.et";
	protected static const ResourceName ROADBLOCK_ASSAULT_GROUP_USSR_FIRE = "{30ED11AA4F0D41E5}Prefabs/Groups/OPFOR/Group_USSR_FireGroup.et";
	protected static const int ROADBLOCK_ASSAULT_GROUP_US_REQUESTED_UNITS = 4;
	protected static const int ROADBLOCK_ASSAULT_GROUP_USSR_AT_REQUESTED_UNITS = 2;
	protected static const int ROADBLOCK_ASSAULT_GROUP_USSR_FIRE_REQUESTED_UNITS = 6;
	protected static const ResourceName ROADBLOCK_ASSAULT_WAYPOINT_SEARCH_AND_DESTROY = "Prefabs/AI/Waypoints/AIWaypoint_SearchAndDestroy.et";
	protected static const ResourceName CAMP_ASSAULT_GROUP_US_DEFENDERS_6 = "{B0F1A7E33C6C4304}Prefabs/Groups/JLH_DCF/US_GroupLibrary/JLH_US_CampDefenders_6Man.et";
	protected static const ResourceName CAMP_ASSAULT_GROUP_USSR_FIRETEAM_4 = "{657590C1EC9E27D3}Prefabs/Groups/OPFOR/Group_USSR_LightFireTeam.et";
	protected static const float ROADBLOCK_ASSAULT_MIN_ATTACKER_DISTANCE = 150.0;
	protected static const float ROADBLOCK_ASSAULT_MAX_ATTACKER_DISTANCE = 300.0;
	protected static const float ROADBLOCK_ASSAULT_MIN_ROAD_WIDTH = 2.5;
	protected static const float ROADBLOCK_BREAKTHROUGH_MAIN_ROAD_MIN_WIDTH = 4.5;
	protected static const float SUPPLY_TRUCK_RAID_ROAD_SEARCH_RADIUS = 110.0;
	protected static const float CAMP_ASSAULT_MAX_SLOPE_DEGREES = 18.0;
	protected static const float CAMP_ASSAULT_DEFENDER_SPACING = 13.0;
	protected static const float CAMP_ASSAULT_ATTACKER_DISTANCE_MIN = 115.0;
	protected static const float CAMP_ASSAULT_ATTACKER_DISTANCE_MAX = 165.0;
	protected static const float CAMP_ASSAULT_ATTACKER_LATERAL = 58.0;
	protected static const float DOWNED_CREW_MAX_SLOPE_DEGREES = 18.0;
	protected static const ResourceName SCENE_SANDBAG_US = "{917158017F425567}Prefabs/Compositions/Slotted/SlotFlatSmall/SandbagPosition_S_US_01.et";
	protected static const ResourceName SCENE_BARREL_CLUTTER = "{D93EF3E97E06D435}Prefabs/Compositions/Misc/SubCompositions/BarrelClutter_04.et";
	protected static const ResourceName SCENE_CRATE_USSR = "{EF08CE3813C6BB6E}Prefabs/Compositions/Misc/SubCompositions/CrateWooden_Cluttered_USSR_01.et";
	protected static vector s_vContextQueryPosition;
	protected static float s_fContextQueryRadius;
	protected static bool s_bContextQueryFound;

	[Attribute("1", UIWidgets.CheckBox, "Enable this Ambient Skirmish area.", category: "JLH DCF Ambient Skirmish - Area")]
	protected bool m_bEnabled = true;

	protected JLH_AmbientSkirmishEventType m_eEventType = JLH_AmbientSkirmishEventType.RANDOM;

	[Attribute("1", UIWidgets.CheckBox, "When this area has no child scenario marker, spawn a random skirmish from framework defaults. Turn off for marker-only areas.", category: "JLH DCF Ambient Skirmish - Fallback")]
	protected bool m_bRandomizeEventType = true;

	[Attribute("2", UIWidgets.ComboBox, "Fallback random skirmish intensity used when no child scenario marker overrides this area.", "", ParamEnumArray.FromEnum(JLH_DCF_AmbientSkirmishIntensity), category: "JLH DCF Ambient Skirmish - Fallback")]
	protected JLH_DCF_AmbientSkirmishIntensity m_eIntensity = JLH_DCF_AmbientSkirmishIntensity.HEAVY;

	[Attribute("US", UIWidgets.EditBox, "Friendly faction key used by fallback skirmishes.", category: "JLH DCF Ambient Skirmish - Factions")]
	protected string m_sFriendlyFactionKey = "US";

	[Attribute("USSR", UIWidgets.EditBox, "Enemy faction key used by fallback skirmishes.", category: "JLH DCF Ambient Skirmish - Factions")]
	protected string m_sEnemyFactionKey = "USSR";

	[Attribute("15", UIWidgets.EditBox, "Minimum infantry units requested per side for fallback skirmishes.", "2 20 1", category: "JLH DCF Ambient Skirmish - Fallback")]
	protected int m_iMinInfantryPerSide = 15;

	[Attribute("20", UIWidgets.EditBox, "Maximum infantry units requested per side for fallback skirmishes.", "2 20 1", category: "JLH DCF Ambient Skirmish - Fallback")]
	protected int m_iMaxInfantryPerSide = 20;

	[Attribute("1", UIWidgets.CheckBox, "Allow infantry clash fallback skirmishes.", category: "JLH DCF Ambient Skirmish - Fallback Events")]
	protected bool m_bAllowInfantryClash = true;

	[Attribute("1", UIWidgets.CheckBox, "Allow small patrol contact fallback skirmishes.", category: "JLH DCF Ambient Skirmish - Fallback Events")]
	protected bool m_bAllowSmallPatrolContact = true;

	[Attribute("1", UIWidgets.CheckBox, "Allow camp assault fallback skirmishes.", category: "JLH DCF Ambient Skirmish - Fallback Events")]
	protected bool m_bAllowCampAssault = true;

	[Attribute("1", UIWidgets.CheckBox, "Allow roadblock breakthrough fallback skirmishes.", category: "JLH DCF Ambient Skirmish - Fallback Events")]
	protected bool m_bAllowRoadblockBreakthrough = true;

	[Attribute("1", UIWidgets.CheckBox, "Allow downed crew rescue fallback skirmishes.", category: "JLH DCF Ambient Skirmish - Fallback Events")]
	protected bool m_bAllowDownedCrewRescue = true;

	[Attribute("1", UIWidgets.CheckBox, "Allow downed US helicopter rescue fallback skirmishes.", category: "JLH DCF Ambient Skirmish - Fallback Events")]
	protected bool m_bAllowDownedHeliRescue = true;

	[Attribute("1", UIWidgets.CheckBox, "Allow recon team contact fallback skirmishes.", category: "JLH DCF Ambient Skirmish - Fallback Events")]
	protected bool m_bAllowReconTeamContact = true;

	[Attribute("1", UIWidgets.CheckBox, "Allow vehicle fallback skirmishes.", category: "JLH DCF Ambient Skirmish - Fallback Events")]
	protected bool m_bAllowVehicles = true;

	[Attribute("1", UIWidgets.CheckBox, "Allow supply truck raid fallback skirmishes.", category: "JLH DCF Ambient Skirmish - Fallback Events")]
	protected bool m_bAllowSupplyTrucks = true;

	[Attribute("0", UIWidgets.CheckBox, "Allow static weapon support in fallback skirmishes.", category: "JLH DCF Ambient Skirmish - Fallback Events")]
	protected bool m_bAllowStaticWeapons = false;

	[Attribute("0", UIWidgets.CheckBox, "Spawn optional scenario dressing for fallback skirmishes.", category: "JLH DCF Ambient Skirmish - Dressing")]
	protected bool m_bSpawnScenarioDressing = false;
	protected ResourceName m_sDownedUSHeliLivePrefab = "{70BAEEFC2D3FEE64}Prefabs/Vehicles/Helicopters/UH1H/UH1H.et";
	protected int m_iDownedUSHeliDestroyDelayMs = 1500;
	protected int m_iDownedCrewVehicleDestroyDelayMs = 1200;
	protected float m_fConvoyAmbushRoadSearchRadius = 75.0;
	protected ResourceName m_sConvoyAmbushFrontVehiclePrefab = "";
	protected ResourceName m_sConvoyAmbushTroopTruckPrefab = "{81FDAD5EB644CC3D}Prefabs/Vehicles/Wheeled/M923A1/M923A1_transport_covered.et";
	protected ResourceName m_sConvoyAmbushRearVehiclePrefab = "";
	protected ResourceName m_sConvoyAmbushUSTroopGroupPrefab = "";
	protected ResourceName m_sConvoyAmbushUSSRFrontGroupPrefab = "";
	protected ResourceName m_sConvoyAmbushUSSRRearGroupPrefab = "";
	protected int m_iConvoyAmbushMinEnemyGroups = 4;
	protected int m_iConvoyAmbushMaxEnemyGroups = 5;
	protected bool m_bConvoyAmbushAllowAT = false;
	protected JLH_AmbientSkirmishVehicleRole m_eVehicleRole = JLH_AmbientSkirmishVehicleRole.ANY_ARMED_GROUND;
	protected ref array<EEditableEntityLabel> m_aRequiredVehicleLabels = {};
	protected ref array<EEditableEntityLabel> m_aPreferredVehicleLabels = {};
	protected ref array<EEditableEntityLabel> m_aExcludedVehicleLabels = {};
	protected ref array<EEditableEntityLabel> m_aFallbackVehicleLabels = {};
	protected ref array<EEditableEntityLabel> m_aFriendlyRequiredVehicleLabels = {};
	protected ref array<EEditableEntityLabel> m_aFriendlyPreferredVehicleLabels = {};
	protected ref array<EEditableEntityLabel> m_aFriendlyExcludedVehicleLabels = {};
	protected ref array<EEditableEntityLabel> m_aFriendlyFallbackVehicleLabels = {};
	protected ref array<EEditableEntityLabel> m_aEnemyRequiredVehicleLabels = {};
	protected ref array<EEditableEntityLabel> m_aEnemyPreferredVehicleLabels = {};
	protected ref array<EEditableEntityLabel> m_aEnemyExcludedVehicleLabels = {};
	protected ref array<EEditableEntityLabel> m_aEnemyFallbackVehicleLabels = {};

	[Attribute("850", UIWidgets.EditBox, "Area wakes and may spawn when a live deployed player is within this distance.", "0 5000 1", category: "JLH DCF Ambient Skirmish - Distances")]
	protected float m_fActivationRadius;

	[Attribute("260", UIWidgets.EditBox, "Fallback random contact-center radius around this area.", "0 5000 1", category: "JLH DCF Ambient Skirmish - Distances")]
	protected float m_fSpawnRadius = 260.0;

	[Attribute("220", UIWidgets.EditBox, "Reject fallback skirmish positions closer than this to any live deployed player.", "0 3000 1", category: "JLH DCF Ambient Skirmish - Distances")]
	protected float m_fMinPlayerDistance = 220.0;

	[Attribute("180", UIWidgets.EditBox, "Approximate distance between friendly and enemy groups in fallback skirmishes.", "20 500 1", category: "JLH DCF Ambient Skirmish - Distances")]
	protected float m_fGroupSeparation = 180.0;

	[Attribute("1", UIWidgets.CheckBox, "Allow this area to spawn near US-owned bases, while still respecting base core safety radius.", category: "JLH DCF Ambient Skirmish - Safety")]
	protected bool m_bAllowNearUSBases = true;

	[Attribute("0", UIWidgets.EditBox, "Optional hard safety radius around US HQ/MOB bases. Set to 0 to allow first-cap and nearby wilderness spawns.", "0 3000 1", category: "JLH DCF Ambient Skirmish - Safety")]
	protected float m_fUSHQHardSafetyRadius;

	[Attribute("150", UIWidgets.EditBox, "Reject spawns this close to any campaign base core.", "0 3000 1", category: "JLH DCF Ambient Skirmish - Safety")]
	protected float m_fUSBaseCoreSafetyRadius;

	[Attribute("1500", UIWidgets.EditBox, "Cleanup runtime groups when all live deployed players are farther than this distance from the skirmish.", "250 8000 1", category: "JLH DCF Ambient Skirmish - Lifecycle")]
	protected float m_fCleanupDistance;

	[Attribute("900", UIWidgets.EditBox, "Seconds before this area may spawn another skirmish after a successful spawn or chance miss.", "0 7200 1", category: "JLH DCF Ambient Skirmish - Lifecycle")]
	protected int m_iCooldownSeconds;

	[Attribute("1", UIWidgets.EditBox, "Maximum active runtime skirmishes owned by this area.", "0 6 1", category: "JLH DCF Ambient Skirmish - Lifecycle")]
	protected int m_iMaxActiveSkirmishes;

	[Attribute("1.0", UIWidgets.EditBox, "Chance from 0.0 to 1.0 that an eligible activation creates a skirmish.", "0 1 0.01", category: "JLH DCF Ambient Skirmish - Lifecycle")]
	protected float m_fSpawnChance;

	[Attribute("1", UIWidgets.CheckBox, "Allow child scenario markers to override fallback skirmish settings.", category: "JLH DCF Ambient Skirmish - Child Markers")]
	protected bool m_bUseChildMarkers = true;

	[Attribute("1", UIWidgets.CheckBox, "Use valid child marker transforms as exact skirmish centers when available.", category: "JLH DCF Ambient Skirmish - Child Markers")]
	protected bool m_bUseExactChildMarkerTransform = true;

	[Attribute("1", UIWidgets.CheckBox, "Area prefabs only. Multiplies Activation Radius and Spawn Radius by this entity's scale so the visible Workbench zone and runtime area stay aligned.", category: "JLH DCF Ambient Skirmish - Area Visual")]
	protected bool m_bScaleDistancesByEntityScale;

	[Attribute("0", UIWidgets.CheckBox, "Enable detailed skirmish activation, spawn, and cleanup logs.", category: "JLH DCF Ambient Skirmish - Debug")]
	protected bool m_bDebug;

	[Attribute("0", UIWidgets.ComboBox, "Vehicle diagnostic detail: basic lifecycle, verbose validation, or deep runtime audits.", "", ParamEnumArray.FromEnum(JLH_AmbientSkirmishDebugLevel), category: "JLH DCF Ambient Skirmish - Debug")]
	protected JLH_AmbientSkirmishDebugLevel m_eDebugLevel;

	protected bool m_bRegistered;
	protected bool m_bLastEnabledState;
	protected bool m_bEnabledStateInitialized;
	protected bool m_bBlockedNoLiveDeployedHuman;
	protected int m_iNextAllowedSpawnTick;
	protected int m_iLastNoLiveDeployedHumanLogTick;
	protected string m_sNodeKey;
	protected JLH_DCF_AmbientSkirmishScenarioMarkerComponent m_ActiveScenarioMarker;
	protected ref array<ref JLH_AmbientSkirmishRecord> m_aRuntimeRecords = {};
	protected ref array<string> m_aLogKeys = {};
	protected ref array<int> m_aLogTicks = {};

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		JLH_DCF_AmbientSkirmishManager.RegisterArea(this);
	}

	void RegisterArea()
	{
		if (m_bRegistered)
			return;

		IEntity owner = GetOwner();
		if (!owner)
			return;

		NormalizeEditorFields();
		m_sNodeKey = BuildNodeKey(owner);
		m_iNextAllowedSpawnTick = 0;
		m_bRegistered = true;
		m_bLastEnabledState = m_bEnabled;
		m_bEnabledStateInitialized = true;

		LogEvent("ambient_skirmish_area_registered", "area=" + JLH_DCF_NodeDebug.EntityLabel(owner) + " randomFallback=" + JLH_DCF_NodeDebug.BoolLabel(m_bRandomizeEventType) + " activationRadius=" + GetEffectiveActivationRadius().ToString() + " spawnRadius=" + GetEffectiveSpawnRadius().ToString(), false);
		if (m_bEnabled)
			LogEvent("ambient_skirmish_area_enabled", "area=" + JLH_DCF_NodeDebug.EntityLabel(owner), false);
		else
			LogEvent("ambient_skirmish_area_disabled", "area=" + JLH_DCF_NodeDebug.EntityLabel(owner), false);
	}

	void Tick()
	{
		RegisterArea();
		if (!m_bRegistered)
			return;

		NormalizeEditorFields();
		LogEnabledStateChange();

		if (!m_bEnabled)
		{
			CleanupAllRuntime("ambient_skirmish_cleanup_disabled", true);
			return;
		}

		ref array<IEntity> livePlayers = {};
		JLH_AddonWorldQuery.GatherLiveDeployedPlayers(livePlayers, "");
		LogEvent("ambient_skirmish_live_player_count", "count=" + livePlayers.Count().ToString(), false);
		if (livePlayers.IsEmpty())
		{
			CleanupAllRuntime("ambient_skirmish_cleanup_no_live_players", true);
			LogNoLiveDeployedHumanBlocked();
			return;
		}

		LogLiveDeployedHumanResumed(livePlayers.Count());
		CleanupRuntime();
		MonitorVehicleCrewDismountDiagnostics();

		IEntity nearestPlayer;
		float nearestDistance;
		if (!FindNearestLivePlayerWithinActivation(nearestPlayer, nearestDistance))
			return;

		LogEvent("ambient_skirmish_player_near", "distance=" + nearestDistance.ToString(), false);
		TryActivate(nearestPlayer);
	}

	protected void TryActivate(IEntity player)
	{
		if (!player)
			return;

		int localActive = CountActiveLocalRuntime();
		LogEvent("ambient_skirmish_activation_check", "localActive=" + localActive.ToString() + " localCap=" + m_iMaxActiveSkirmishes.ToString(), false);

		if (System.GetTickCount() < m_iNextAllowedSpawnTick)
		{
			LogEvent("ambient_skirmish_blocked_cooldown", "remainingMs=" + (m_iNextAllowedSpawnTick - System.GetTickCount()).ToString(), false);
			return;
		}

		int globalActive;
		int globalCap;
		if (!JLH_DCF_AmbientSkirmishManager.HasGlobalCapRoom(globalActive, globalCap))
		{
			LogEvent("ambient_skirmish_blocked_global_cap", "globalActive=" + globalActive.ToString() + " globalCap=" + globalCap.ToString(), false);
			return;
		}

		if (m_iMaxActiveSkirmishes <= 0 || localActive >= m_iMaxActiveSkirmishes)
		{
			LogEvent("ambient_skirmish_blocked_local_cap", "localActive=" + localActive.ToString() + " localCap=" + m_iMaxActiveSkirmishes.ToString(), false);
			return;
		}

		if (m_fSpawnChance < 1.0 && Math.RandomFloat(0.0, 1.0) > m_fSpawnChance)
		{
			LogEvent("ambient_skirmish_spawn_failed", "reason=spawn_chance", false);
			MarkCooldown();
			return;
		}

		int budgetDelayMs;
		if (!JLH_DCF_RuntimeWorkBudget.TryReserveHeavySpawn("AmbientSkirmish", budgetDelayMs))
		{
			DeferSpawnForRuntimeBudget(budgetDelayMs);
			return;
		}

		JLH_AmbientSkirmishRecord record;
		string reason;
		if (!SpawnSkirmish(player, record, reason))
		{
			LogEvent("ambient_skirmish_spawn_failed", "reason=" + reason, true);
			MarkCooldown();
			return;
		}

		m_aRuntimeRecords.Insert(record);
		JLH_SkirmishMapIntelManager.OnSkirmishStarted(record);
		LogEvent("ambient_skirmish_runtime_registered", "id=" + record.Id.ToString(), true);
		LogEvent("ambient_skirmish_spawn_success", "id=" + record.Id.ToString() + " event=" + record.SkirmishEventName + " friendly=" + record.FriendlyFactionKey + " friendlyUnits=" + record.FriendlyUnits.ToString() + " friendlySource=" + record.FriendlySource + " enemy=" + record.EnemyFactionKey + " enemyUnits=" + record.EnemyUnits.ToString() + " enemySource=" + record.EnemySource, true);
		MarkCooldown();
	}

	protected bool SpawnSkirmish(IEntity player, out JLH_AmbientSkirmishRecord record, out string reason)
	{
		record = null;
		reason = "";

		JLH_DCF_AmbientSkirmishScenarioMarkerComponent scenarioMarker;
		if (SelectScenarioMarker(player, scenarioMarker, reason))
			return SpawnSkirmishFromScenarioMarker(player, scenarioMarker, record, reason);

		if (reason != "")
			return false;

		if (!m_bRandomizeEventType)
		{
			reason = "no_scenario_marker_random_fallback_disabled";
			return false;
		}

		ApplyAreaRandomFallbackSettings();
		return SpawnSkirmishWithCurrentSettings(player, record, reason);
	}

	protected void ApplyAreaRandomFallbackSettings()
	{
		m_eEventType = JLH_AmbientSkirmishEventType.RANDOM;
		m_eIntensity = JLH_DCF_AmbientSkirmishIntensity.HEAVY;
		m_sFriendlyFactionKey = "US";
		m_sEnemyFactionKey = "USSR";
		m_iMinInfantryPerSide = 15;
		m_iMaxInfantryPerSide = 20;
		m_bAllowInfantryClash = true;
		m_bAllowSmallPatrolContact = true;
		m_bAllowCampAssault = true;
		m_bAllowRoadblockBreakthrough = true;
		m_bAllowDownedCrewRescue = true;
		m_bAllowDownedHeliRescue = true;
		m_bAllowReconTeamContact = true;
		m_bAllowVehicles = true;
		m_bAllowSupplyTrucks = true;
		m_bAllowStaticWeapons = false;
		m_bSpawnScenarioDressing = false;
		m_sDownedUSHeliLivePrefab = SCENE_US_HELI_LIVE;
		m_iDownedUSHeliDestroyDelayMs = 1500;
		m_iDownedCrewVehicleDestroyDelayMs = 1200;
		m_fConvoyAmbushRoadSearchRadius = 75.0;
		m_sConvoyAmbushFrontVehiclePrefab = "";
		m_sConvoyAmbushTroopTruckPrefab = CONVOY_AMBUSH_DEFAULT_TROOP_TRUCK;
		m_sConvoyAmbushRearVehiclePrefab = "";
		m_sConvoyAmbushUSTroopGroupPrefab = "";
		m_sConvoyAmbushUSSRFrontGroupPrefab = "";
		m_sConvoyAmbushUSSRRearGroupPrefab = "";
		m_iConvoyAmbushMinEnemyGroups = 4;
		m_iConvoyAmbushMaxEnemyGroups = 5;
		m_bConvoyAmbushAllowAT = false;
		m_eVehicleRole = JLH_AmbientSkirmishVehicleRole.ANY_ARMED_GROUND;
		m_aRequiredVehicleLabels = {};
		m_aPreferredVehicleLabels = {};
		m_aExcludedVehicleLabels = {};
		m_aFallbackVehicleLabels = {};
		m_aFriendlyRequiredVehicleLabels = {};
		m_aFriendlyPreferredVehicleLabels = {};
		m_aFriendlyExcludedVehicleLabels = {};
		m_aFriendlyFallbackVehicleLabels = {};
		m_aEnemyRequiredVehicleLabels = {};
		m_aEnemyPreferredVehicleLabels = {};
		m_aEnemyExcludedVehicleLabels = {};
		m_aEnemyFallbackVehicleLabels = {};
	}

	protected bool SpawnSkirmishWithCurrentSettings(IEntity player, out JLH_AmbientSkirmishRecord record, out string reason)
	{
		record = null;
		reason = "";

		JLH_DCF_AmbientSkirmishIntensity intensity = ResolveIntensity();
		JLH_AmbientSkirmishEventDefinition definition;
		if (!SelectEventDefinition(intensity, definition, reason))
			return false;

		return JLH_AmbientSkirmishEventExecutor.Execute(this, definition, player, record, reason);
	}

	protected bool SpawnSkirmishFromScenarioMarker(IEntity player, notnull JLH_DCF_AmbientSkirmishScenarioMarkerComponent marker, out JLH_AmbientSkirmishRecord record, out string reason)
	{
		record = null;
		reason = "";

		JLH_AmbientSkirmishEventType savedEventType = m_eEventType;
		bool savedRandomizeEventType = m_bRandomizeEventType;
		JLH_DCF_AmbientSkirmishIntensity savedIntensity = m_eIntensity;
		string savedFriendlyFactionKey = m_sFriendlyFactionKey;
		string savedEnemyFactionKey = m_sEnemyFactionKey;
		int savedMinInfantryPerSide = m_iMinInfantryPerSide;
		int savedMaxInfantryPerSide = m_iMaxInfantryPerSide;
		bool savedAllowInfantryClash = m_bAllowInfantryClash;
		bool savedAllowSmallPatrolContact = m_bAllowSmallPatrolContact;
		bool savedAllowCampAssault = m_bAllowCampAssault;
		bool savedAllowRoadblockBreakthrough = m_bAllowRoadblockBreakthrough;
		bool savedAllowDownedCrewRescue = m_bAllowDownedCrewRescue;
		bool savedAllowDownedHeliRescue = m_bAllowDownedHeliRescue;
		bool savedAllowReconTeamContact = m_bAllowReconTeamContact;
		bool savedAllowVehicles = m_bAllowVehicles;
		bool savedAllowSupplyTrucks = m_bAllowSupplyTrucks;
		bool savedAllowStaticWeapons = m_bAllowStaticWeapons;
		bool savedSpawnScenarioDressing = m_bSpawnScenarioDressing;
		ResourceName savedDownedUSHeliLivePrefab = m_sDownedUSHeliLivePrefab;
		int savedDownedUSHeliDestroyDelayMs = m_iDownedUSHeliDestroyDelayMs;
		int savedDownedCrewVehicleDestroyDelayMs = m_iDownedCrewVehicleDestroyDelayMs;
		float savedConvoyAmbushRoadSearchRadius = m_fConvoyAmbushRoadSearchRadius;
		ResourceName savedConvoyAmbushFrontVehiclePrefab = m_sConvoyAmbushFrontVehiclePrefab;
		ResourceName savedConvoyAmbushTroopTruckPrefab = m_sConvoyAmbushTroopTruckPrefab;
		ResourceName savedConvoyAmbushRearVehiclePrefab = m_sConvoyAmbushRearVehiclePrefab;
		ResourceName savedConvoyAmbushUSTroopGroupPrefab = m_sConvoyAmbushUSTroopGroupPrefab;
		ResourceName savedConvoyAmbushUSSRFrontGroupPrefab = m_sConvoyAmbushUSSRFrontGroupPrefab;
		ResourceName savedConvoyAmbushUSSRRearGroupPrefab = m_sConvoyAmbushUSSRRearGroupPrefab;
		int savedConvoyAmbushMinEnemyGroups = m_iConvoyAmbushMinEnemyGroups;
		int savedConvoyAmbushMaxEnemyGroups = m_iConvoyAmbushMaxEnemyGroups;
		bool savedConvoyAmbushAllowAT = m_bConvoyAmbushAllowAT;
		JLH_AmbientSkirmishVehicleRole savedVehicleRole = m_eVehicleRole;
		ref array<EEditableEntityLabel> savedRequiredVehicleLabels = m_aRequiredVehicleLabels;
		ref array<EEditableEntityLabel> savedPreferredVehicleLabels = m_aPreferredVehicleLabels;
		ref array<EEditableEntityLabel> savedExcludedVehicleLabels = m_aExcludedVehicleLabels;
		ref array<EEditableEntityLabel> savedFallbackVehicleLabels = m_aFallbackVehicleLabels;
		ref array<EEditableEntityLabel> savedFriendlyRequiredVehicleLabels = m_aFriendlyRequiredVehicleLabels;
		ref array<EEditableEntityLabel> savedFriendlyPreferredVehicleLabels = m_aFriendlyPreferredVehicleLabels;
		ref array<EEditableEntityLabel> savedFriendlyExcludedVehicleLabels = m_aFriendlyExcludedVehicleLabels;
		ref array<EEditableEntityLabel> savedFriendlyFallbackVehicleLabels = m_aFriendlyFallbackVehicleLabels;
		ref array<EEditableEntityLabel> savedEnemyRequiredVehicleLabels = m_aEnemyRequiredVehicleLabels;
		ref array<EEditableEntityLabel> savedEnemyPreferredVehicleLabels = m_aEnemyPreferredVehicleLabels;
		ref array<EEditableEntityLabel> savedEnemyExcludedVehicleLabels = m_aEnemyExcludedVehicleLabels;
		ref array<EEditableEntityLabel> savedEnemyFallbackVehicleLabels = m_aEnemyFallbackVehicleLabels;
		float savedSpawnRadius = m_fSpawnRadius;
		float savedMinPlayerDistance = m_fMinPlayerDistance;
		float savedGroupSeparation = m_fGroupSeparation;
		JLH_DCF_AmbientSkirmishScenarioMarkerComponent savedActiveScenarioMarker = m_ActiveScenarioMarker;

		ApplyScenarioMarkerSettings(marker);
		NormalizeEditorFields();
		m_ActiveScenarioMarker = marker;
		string markerLog = "marker=" + JLH_DCF_NodeDebug.EntityLabel(marker.GetOwner());
		markerLog = markerLog + " event=" + JLH_AmbientSkirmishEventLibrary.EventName(m_eEventType);
		markerLog = markerLog + " randomize=" + JLH_DCF_NodeDebug.BoolLabel(m_bRandomizeEventType);
		LogEvent("ambient_skirmish_scenario_marker_selected", markerLog, true);

		bool spawned = SpawnSkirmishWithCurrentSettings(player, record, reason);

		m_eEventType = savedEventType;
		m_bRandomizeEventType = savedRandomizeEventType;
		m_eIntensity = savedIntensity;
		m_sFriendlyFactionKey = savedFriendlyFactionKey;
		m_sEnemyFactionKey = savedEnemyFactionKey;
		m_iMinInfantryPerSide = savedMinInfantryPerSide;
		m_iMaxInfantryPerSide = savedMaxInfantryPerSide;
		m_bAllowInfantryClash = savedAllowInfantryClash;
		m_bAllowSmallPatrolContact = savedAllowSmallPatrolContact;
		m_bAllowCampAssault = savedAllowCampAssault;
		m_bAllowRoadblockBreakthrough = savedAllowRoadblockBreakthrough;
		m_bAllowDownedCrewRescue = savedAllowDownedCrewRescue;
		m_bAllowDownedHeliRescue = savedAllowDownedHeliRescue;
		m_bAllowReconTeamContact = savedAllowReconTeamContact;
		m_bAllowVehicles = savedAllowVehicles;
		m_bAllowSupplyTrucks = savedAllowSupplyTrucks;
		m_bAllowStaticWeapons = savedAllowStaticWeapons;
		m_bSpawnScenarioDressing = savedSpawnScenarioDressing;
		m_sDownedUSHeliLivePrefab = savedDownedUSHeliLivePrefab;
		m_iDownedUSHeliDestroyDelayMs = savedDownedUSHeliDestroyDelayMs;
		m_iDownedCrewVehicleDestroyDelayMs = savedDownedCrewVehicleDestroyDelayMs;
		m_fConvoyAmbushRoadSearchRadius = savedConvoyAmbushRoadSearchRadius;
		m_sConvoyAmbushFrontVehiclePrefab = savedConvoyAmbushFrontVehiclePrefab;
		m_sConvoyAmbushTroopTruckPrefab = savedConvoyAmbushTroopTruckPrefab;
		m_sConvoyAmbushRearVehiclePrefab = savedConvoyAmbushRearVehiclePrefab;
		m_sConvoyAmbushUSTroopGroupPrefab = savedConvoyAmbushUSTroopGroupPrefab;
		m_sConvoyAmbushUSSRFrontGroupPrefab = savedConvoyAmbushUSSRFrontGroupPrefab;
		m_sConvoyAmbushUSSRRearGroupPrefab = savedConvoyAmbushUSSRRearGroupPrefab;
		m_iConvoyAmbushMinEnemyGroups = savedConvoyAmbushMinEnemyGroups;
		m_iConvoyAmbushMaxEnemyGroups = savedConvoyAmbushMaxEnemyGroups;
		m_bConvoyAmbushAllowAT = savedConvoyAmbushAllowAT;
		m_eVehicleRole = savedVehicleRole;
		m_aRequiredVehicleLabels = savedRequiredVehicleLabels;
		m_aPreferredVehicleLabels = savedPreferredVehicleLabels;
		m_aExcludedVehicleLabels = savedExcludedVehicleLabels;
		m_aFallbackVehicleLabels = savedFallbackVehicleLabels;
		m_aFriendlyRequiredVehicleLabels = savedFriendlyRequiredVehicleLabels;
		m_aFriendlyPreferredVehicleLabels = savedFriendlyPreferredVehicleLabels;
		m_aFriendlyExcludedVehicleLabels = savedFriendlyExcludedVehicleLabels;
		m_aFriendlyFallbackVehicleLabels = savedFriendlyFallbackVehicleLabels;
		m_aEnemyRequiredVehicleLabels = savedEnemyRequiredVehicleLabels;
		m_aEnemyPreferredVehicleLabels = savedEnemyPreferredVehicleLabels;
		m_aEnemyExcludedVehicleLabels = savedEnemyExcludedVehicleLabels;
		m_aEnemyFallbackVehicleLabels = savedEnemyFallbackVehicleLabels;
		m_fSpawnRadius = savedSpawnRadius;
		m_fMinPlayerDistance = savedMinPlayerDistance;
		m_fGroupSeparation = savedGroupSeparation;
		m_ActiveScenarioMarker = savedActiveScenarioMarker;

		return spawned;
	}

	bool ExecuteInfantrySkirmishDefinition(notnull JLH_AmbientSkirmishEventDefinition definition, IEntity player, out JLH_AmbientSkirmishRecord record, out string reason)
	{
		record = null;
		reason = "";

		JLH_AmbientSkirmishGroupPlan friendlyPlan;
		if (!SelectFriendlyPlan(definition, friendlyPlan, reason))
			return false;

		JLH_AmbientSkirmishGroupPlan enemyPlan;
		if (!SelectEnemyPlan(definition, enemyPlan, reason))
			return false;

		vector center;
		if (!SelectContactCenter(player, definition, center, reason))
			return false;

		vector friendlyPosition;
		vector enemyPosition;
		float groupSeparation = Math.Clamp(m_fGroupSeparation * Math.Max(0.1, definition.SeparationMultiplier), 20.0, 600.0);
		if (!BuildScenarioGroupPositions(definition, player, center, groupSeparation, friendlyPosition, enemyPosition, reason))
			return false;

		bool useCrashSurvivorLayout = ShouldUseCrashSurvivorLayout(definition);
		bool useRadialEnemyApproach = ShouldUseRadialEnemyApproach(definition);
		if (useCrashSurvivorLayout)
		{
			friendlyPosition = center;
			enemyPosition = center;
		}

		array<IEntity> exactSkirmishMarkers = {};
		array<IEntity> friendlyExactMarkers = {};
		array<IEntity> enemyExactMarkers = {};
		if (!useCrashSurvivorLayout && m_bUseChildMarkers && m_bUseExactChildMarkerTransform && CollectValidExactSkirmishMarkers(player, exactSkirmishMarkers, reason))
			PartitionExactSkirmishMarkers(exactSkirmishMarkers, friendlyPosition, enemyPosition, friendlyExactMarkers, enemyExactMarkers);

		float groupOffsetSpacing = 0.0;
		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.INFANTRY_CLASH)
			groupOffsetSpacing = 50.0;

		array<SCR_AIGroup> friendlyGroups = {};
		if (!SpawnGroupPlanWithOffset(friendlyPlan, friendlyPosition, "friendly", friendlyExactMarkers, friendlyGroups, reason, groupOffsetSpacing))
			return false;

		array<SCR_AIGroup> enemyGroups = {};
		bool enemySpawned;
		if (useRadialEnemyApproach)
			enemySpawned = SpawnRadialEnemyGroupPlan(definition, enemyPlan, center, groupSeparation, enemyGroups, reason);
		else
			enemySpawned = SpawnGroupPlanWithOffset(enemyPlan, enemyPosition, "enemy", enemyExactMarkers, enemyGroups, reason, groupOffsetSpacing);

		if (!enemySpawned)
		{
			CleanupSpawnedGroups(friendlyGroups, "ambient_skirmish_friendly_orphan_cleanup");
			return false;
		}

		record = new JLH_AmbientSkirmishRecord();
		record.Init(JLH_DCF_AmbientSkirmishManager.NextSkirmishId(), m_sNodeKey, JLH_DCF_NodeDebug.EntityLabel(GetOwner()), definition, friendlyGroups, enemyGroups, center, friendlyPosition, enemyPosition, friendlyPlan, enemyPlan);
		if (useCrashSurvivorLayout)
			LayoutCrashSurvivors(record, definition, 0);

		BuildScenarioScene(record, definition);
		AssignCombatBehavior(record, definition);
		record.Runtime.RefreshExclusions("ambient_skirmish_spawn_refresh");
		return true;
	}

	bool ExecuteDownedCrewRescueDefinition(notnull JLH_AmbientSkirmishEventDefinition definition, IEntity player, out JLH_AmbientSkirmishRecord record, out string reason)
	{
		record = null;
		reason = "";

		JLH_AmbientSkirmishGroupPlan friendlyPlan;
		if (!SelectFriendlyPlan(definition, friendlyPlan, reason))
			return false;

		JLH_AmbientSkirmishGroupPlan enemyPlan;
		if (!SelectEnemyPlan(definition, enemyPlan, reason))
			return false;

		vector markerPos;
		if (!SelectContactCenter(player, definition, markerPos, reason))
			return false;

		vector center;
		if (!ResolveDownedCrewFlatCenter(markerPos, center, reason))
		{
			LogEvent("downed_crew_flat_ground_failed", "marker=" + markerPos.ToString() + " reason=" + reason, true);
			return false;
		}

		vector sceneDir = ResolveDownedCrewSceneDirection(player, center);
		LogEvent("downed_crew_flat_ground", "marker=" + markerPos.ToString() + " center=" + center.ToString() + " dir=" + sceneDir.ToString() + " slope=" + JLH_AddonWorldQuery.GetSlopeDegrees(center).ToString(), true);

		array<IEntity> noExactMarkers = {};
		array<SCR_AIGroup> friendlyGroups = {};
		if (!SpawnGroupPlanWithOffset(friendlyPlan, center, "friendly", noExactMarkers, friendlyGroups, reason, 0.0))
			return false;

		float groupSeparation = Math.Clamp(m_fGroupSeparation * Math.Max(0.1, definition.SeparationMultiplier), 20.0, 600.0);
		array<SCR_AIGroup> enemyGroups = {};
		if (!SpawnRadialEnemyGroupPlan(definition, enemyPlan, center, groupSeparation, enemyGroups, reason))
		{
			CleanupSpawnedGroups(friendlyGroups, "downed_crew_friendly_orphan_cleanup");
			return false;
		}

		record = new JLH_AmbientSkirmishRecord();
		record.Init(JLH_DCF_AmbientSkirmishManager.NextSkirmishId(), m_sNodeKey, JLH_DCF_NodeDebug.EntityLabel(GetOwner()), definition, friendlyGroups, enemyGroups, center, center, center, friendlyPlan, enemyPlan);
		LayoutCrashSurvivors(record, definition, 0);
		BuildScenarioScene(record, definition);
		AssignCombatBehavior(record, definition);
		record.Runtime.RefreshExclusions("downed_crew_spawn_refresh");
		return true;
	}

	protected bool ShouldPreserveDownedCrewAuthoredPosition()
	{
		if (m_ActiveScenarioMarker && m_ActiveScenarioMarker.GetOwner())
			return true;

		return m_bUseChildMarkers && HasChildSkirmishMarkers();
	}

	protected bool ResolveDownedCrewFlatCenter(vector markerPos, out vector center, out string reason)
	{
		center = JLH_AddonWorldQuery.GroundPosition(markerPos);
		reason = "";

		if (IsDownedCrewGroundValid(center, reason))
			return true;

		string firstReason = reason;
		vector baseGround = center;
		float baseAngle = Math.RandomFloat(0.0, 360.0);
		for (int attempt = 0; attempt < SPAWN_CANDIDATE_ATTEMPTS; attempt++)
		{
			float angle = baseAngle + (attempt * 41.0);
			float distance = Math.RandomFloat(8.0, Math.Min(70.0, 18.0 + (attempt * 3.0)));
			vector candidate = JLH_AddonWorldQuery.GroundPosition(baseGround + (JLH_AddonWorldQuery.DirectionFromAngle(angle) * distance));
			if (!IsDownedCrewGroundValid(candidate, reason))
				continue;

			center = candidate;
			LogEvent("downed_crew_flat_ground_adjusted", "marker=" + markerPos.ToString() + " center=" + center.ToString() + " firstReason=" + firstReason, true);
			return true;
		}

		if (firstReason != "")
			reason = firstReason;
		else if (reason == "")
			reason = "no_flat_ground";

		return false;
	}

	protected bool IsDownedCrewGroundValid(vector candidate, out string reason)
	{
		reason = "";
		if (JLH_AddonWorldQuery.IsWaterOrNearWater(candidate, 8.0))
		{
			reason = "water";
			return false;
		}

		float slope = JLH_AddonWorldQuery.GetSlopeDegrees(candidate);
		if (slope > DOWNED_CREW_MAX_SLOPE_DEGREES)
		{
			reason = "steep_terrain";
			return false;
		}

		if (!IsAreaSafetyCandidateValid(candidate, reason))
			return false;

		return true;
	}

	protected vector ResolveDownedCrewSceneDirection(IEntity player, vector center)
	{
		vector markerFacing;
		if (TryGetActiveScenarioMarkerForward(markerFacing))
			return markerFacing;

		return ResolveConvoyAmbushFallbackDirection(player, center);
	}

	bool ExecuteCampAssaultDefinition(notnull JLH_AmbientSkirmishEventDefinition definition, IEntity player, out JLH_AmbientSkirmishRecord record, out string reason)
	{
		record = null;
		reason = "";

		JLH_AmbientSkirmishGroupPlan friendlyPlan;
		JLH_AmbientSkirmishGroupPlan enemyPlan;
		if (!BuildCampAssaultExactPlans(friendlyPlan, enemyPlan, reason))
			return false;

		vector markerPos;
		if (!SelectContactCenter(player, definition, markerPos, reason))
			return false;

		vector campCenter = JLH_AddonWorldQuery.GroundPosition(markerPos);
		vector campFacing = ResolveCampAssaultFacing(player, campCenter);
		if (!ShouldPreserveCampAssaultAuthoredPosition())
			TryMoveCampAssaultCenterOffRoad(player, campCenter, campFacing);

		if (!IsCampAssaultCenterValid(campCenter, reason))
		{
			LogEvent("camp_assault_center_rejected", "center=" + campCenter.ToString() + " reason=" + reason, true);
			return false;
		}

		LogEvent("camp_assault_selected", "center=" + campCenter.ToString() + " facing=" + campFacing.ToString() + " slope=" + JLH_AddonWorldQuery.GetSlopeDegrees(campCenter).ToString(), true);

		IEntity campEntity = SpawnCampAssaultComposition(campCenter, campFacing, reason);
		if (!campEntity)
			return false;

		array<IEntity> noExactMarkers = {};
		array<SCR_AIGroup> friendlyGroups = {};
		if (!SpawnGroupPlanWithOffset(friendlyPlan, campCenter, "camp_assault_us_defenders", noExactMarkers, friendlyGroups, reason, CAMP_ASSAULT_DEFENDER_SPACING))
		{
			JLH_AddonSpawnUtility.DeleteRuntimeEntity(campEntity, "camp_assault_orphan_cleanup_composition");
			return false;
		}
		LogEvent("camp_assault_defenders_spawned", "groups=" + friendlyGroups.Count().ToString() + " units=" + friendlyPlan.TotalUnits.ToString() + " center=" + campCenter.ToString(), true);

		array<vector> enemyPositions = {};
		if (!BuildCampAssaultEnemyPositions(campCenter, campFacing, enemyPlan.GroupCount(), enemyPositions, reason))
		{
			CleanupSpawnedGroups(friendlyGroups, "camp_assault_friendly_orphan_cleanup");
			JLH_AddonSpawnUtility.DeleteRuntimeEntity(campEntity, "camp_assault_orphan_cleanup_composition");
			return false;
		}

		array<SCR_AIGroup> enemyGroups = {};
		if (!SpawnCampAssaultGroupPlanAtPositions(enemyPlan, enemyPositions, enemyGroups, reason))
		{
			CleanupSpawnedGroups(friendlyGroups, "camp_assault_friendly_orphan_cleanup");
			JLH_AddonSpawnUtility.DeleteRuntimeEntity(campEntity, "camp_assault_orphan_cleanup_composition");
			return false;
		}

		vector enemyAnchor = enemyPositions[0];
		record = new JLH_AmbientSkirmishRecord();
		record.Init(JLH_DCF_AmbientSkirmishManager.NextSkirmishId(), m_sNodeKey, JLH_DCF_NodeDebug.EntityLabel(GetOwner()), definition, friendlyGroups, enemyGroups, campCenter, campCenter, enemyAnchor, friendlyPlan, enemyPlan);
		record.Runtime.TrackEntity(campEntity, "camp_assault_us_camp");
		AssignCampAssaultBehavior(record, definition, campFacing);
		record.Runtime.RefreshExclusions("camp_assault_spawn_refresh");

		LogEvent("camp_assault_started", "id=" + record.Id.ToString() + " center=" + campCenter.ToString() + " defenders=" + friendlyPlan.TotalUnits.ToString() + " attackers=" + enemyPlan.TotalUnits.ToString() + " attackerGroups=" + enemyGroups.Count().ToString(), true);
		return true;
	}

	protected bool IsCampAssaultCenterValid(vector center, out string reason)
	{
		reason = "";

		if (JLH_AddonWorldQuery.IsWaterOrNearWater(center, 8.0))
		{
			reason = "camp_assault_center_water";
			return false;
		}

		float slope = JLH_AddonWorldQuery.GetSlopeDegrees(center);
		if (slope > CAMP_ASSAULT_MAX_SLOPE_DEGREES)
		{
			reason = "camp_assault_center_too_steep";
			return false;
		}

		return true;
	}

	protected vector ResolveCampAssaultFacing(IEntity player, vector campCenter)
	{
		vector markerFacing;
		if (TryGetActiveScenarioMarkerForward(markerFacing))
			return markerFacing;

		if (player)
			return DirectionFromTo(player.GetOrigin(), campCenter);

		return JLH_AddonWorldQuery.DirectionFromAngle(Math.RandomFloat(0.0, 360.0));
	}

	protected bool ShouldPreserveCampAssaultAuthoredPosition()
	{
		if (m_ActiveScenarioMarker && m_ActiveScenarioMarker.GetOwner())
			return true;

		return m_bUseChildMarkers && HasChildSkirmishMarkers();
	}

	protected bool TryMoveCampAssaultCenterOffRoad(IEntity player, inout vector campCenter, inout vector campFacing)
	{
		RoadNetworkManager roadManager = GetRoadNetworkManager();
		if (!roadManager)
			return false;

		vector roadPosition;
		vector roadDirection;
		float roadWidth;
		float snapDistance;
		string roadReason;
		if (!TryProjectToNearbyRoad(roadManager, campCenter, 28.0, roadPosition, roadDirection, roadWidth, snapDistance, roadReason))
			return false;

		vector roadRight = Vector(roadDirection[2], 0.0, -roadDirection[0]);
		float side = 1.0;
		if (player && DotVectors(DirectionFromTo(roadPosition, player.GetOrigin()), roadRight) > 0.0)
			side = -1.0;

		vector candidate = JLH_AddonWorldQuery.GroundPosition(roadPosition + (roadRight * side * Math.Max(26.0, roadWidth + 20.0)));
		string reason;
		if (!IsCampAssaultCenterValid(candidate, reason))
			return false;

		campCenter = candidate;
		campFacing = DirectionFromTo(campCenter, roadPosition);
		LogEvent("camp_assault_road_avoid", "road=" + roadPosition.ToString() + " center=" + campCenter.ToString() + " distance=" + snapDistance.ToString() + " dir=" + campFacing.ToString(), true);
		return true;
	}

	protected bool TryGetActiveScenarioMarkerForward(out vector facing)
	{
		facing = "0 0 0";
		if (!m_ActiveScenarioMarker || !m_ActiveScenarioMarker.GetOwner())
			return false;

		vector transform[4];
		m_ActiveScenarioMarker.GetOwner().GetTransform(transform);
		facing = NormalizeConvoyAmbushDirection(transform[2]);
		return vector.Distance(facing, "0 0 0") > 0.001;
	}

	protected IEntity SpawnCampAssaultComposition(vector position, vector facing, out string reason)
	{
		reason = "";
		if (SCENE_CAMP_ASSAULT_US == "" || !JLH_AddonSpawnUtility.CanLoadPrefab(SCENE_CAMP_ASSAULT_US))
		{
			reason = "camp_assault_prefab_missing";
			LogEvent("camp_assault_camp_spawn_failed", "reason=" + reason + " prefab=" + SCENE_CAMP_ASSAULT_US, true);
			return null;
		}

		Resource resource = Resource.Load(SCENE_CAMP_ASSAULT_US);
		if (!resource || !resource.IsValid() || !GetGame() || !GetGame().GetWorld())
		{
			reason = "camp_assault_prefab_load_failed";
			LogEvent("camp_assault_camp_spawn_failed", "reason=" + reason + " prefab=" + SCENE_CAMP_ASSAULT_US, true);
			return null;
		}

		vector spawnPosition = JLH_AddonWorldQuery.GroundPosition(position);
		float slope = JLH_AddonWorldQuery.GetSlopeDegrees(spawnPosition);
		if (slope > CAMP_ASSAULT_MAX_SLOPE_DEGREES)
		{
			reason = "camp_assault_composition_too_steep";
			LogEvent("camp_assault_camp_spawn_failed", "reason=" + reason + " pos=" + spawnPosition.ToString() + " slope=" + slope.ToString(), true);
			return null;
		}

		vector up = EstimateCampAssaultTerrainNormal(spawnPosition);
		vector forward = ProjectDirectionOntoPlane(NormalizeConvoyAmbushDirection(facing), up);
		vector right = NormalizeConvoyAmbushVector3D(CrossVectors(up, forward), Vector(forward[2], 0.0, -forward[0]));
		spawnPosition[1] = spawnPosition[1] + 0.03;

		vector transform[4];
		transform[0] = right;
		transform[1] = up;
		transform[2] = forward;
		transform[3] = spawnPosition;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[0] = transform[0];
		spawnParams.Transform[1] = transform[1];
		spawnParams.Transform[2] = transform[2];
		spawnParams.Transform[3] = transform[3];

		IEntity entity = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams);
		if (!entity)
		{
			reason = "camp_assault_composition_spawn_failed";
			LogEvent("camp_assault_camp_spawn_failed", "reason=" + reason + " pos=" + spawnPosition.ToString(), true);
			return null;
		}

		JLH_DCF_PersistenceExclusion.StopTrackingTree(entity, "camp_assault_composition");
		LogEvent("camp_assault_camp_spawned", "prefab=" + SCENE_CAMP_ASSAULT_US + " pos=" + spawnPosition.ToString() + " forward=" + forward.ToString() + " up=" + up.ToString() + " slope=" + slope.ToString(), true);
		return entity;
	}

	protected vector EstimateCampAssaultTerrainNormal(vector position)
	{
		float sample = 2.4;
		vector east = JLH_AddonWorldQuery.GroundPosition(position + Vector(sample, 0.0, 0.0));
		vector west = JLH_AddonWorldQuery.GroundPosition(position - Vector(sample, 0.0, 0.0));
		vector north = JLH_AddonWorldQuery.GroundPosition(position + Vector(0.0, 0.0, sample));
		vector south = JLH_AddonWorldQuery.GroundPosition(position - Vector(0.0, 0.0, sample));

		vector dx = east - west;
		vector dz = north - south;
		vector normal = NormalizeConvoyAmbushVector3D(CrossVectors(dz, dx), Vector(0.0, 1.0, 0.0));
		if (normal[1] < 0.0)
			normal = normal * -1.0;

		return normal;
	}

	protected vector ProjectDirectionOntoPlane(vector direction, vector up)
	{
		float dot = DotVectors(direction, up);
		vector projected = direction - (up * dot);
		return NormalizeConvoyAmbushVector3D(projected, NormalizeConvoyAmbushDirection(direction));
	}

	protected float DotVectors(vector a, vector b)
	{
		return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]);
	}

	protected vector CrossVectors(vector a, vector b)
	{
		return Vector((a[1] * b[2]) - (a[2] * b[1]), (a[2] * b[0]) - (a[0] * b[2]), (a[0] * b[1]) - (a[1] * b[0]));
	}

	protected bool BuildCampAssaultEnemyPositions(vector center, vector facing, int groupCount, notnull array<vector> positions, out string reason)
	{
		positions.Clear();
		reason = "";

		vector forward = NormalizeConvoyAmbushDirection(facing);
		vector right = Vector(forward[2], 0.0, -forward[0]);
		for (int i = 0; i < groupCount; i++)
		{
			float distance = Math.RandomFloat(CAMP_ASSAULT_ATTACKER_DISTANCE_MIN, CAMP_ASSAULT_ATTACKER_DISTANCE_MAX);
			vector offset;
			if (i == 0)
				offset = (forward * distance) + (right * CAMP_ASSAULT_ATTACKER_LATERAL);
			else if (i == 1)
				offset = (forward * -distance) - (right * CAMP_ASSAULT_ATTACKER_LATERAL);
			else if (i == 2)
				offset = (right * -distance) + (forward * Math.RandomFloat(-36.0, 36.0));
			else
				offset = JLH_AddonWorldQuery.DirectionFromAngle(Math.RandomFloat(0.0, 360.0)) * distance;

			vector position;
			if (!ResolveCampAssaultGroupPosition(center + offset, center, position, reason))
				return false;

			positions.Insert(position);
		}

		return !positions.IsEmpty();
	}

	protected bool ResolveCampAssaultGroupPosition(vector desired, vector center, out vector position, out string reason)
	{
		position = "0 0 0";
		reason = "";

		vector direction = DirectionFromTo(center, desired);
		for (int attempt = 0; attempt < SPAWN_CANDIDATE_ATTEMPTS; attempt++)
		{
			float angle = Math.RandomFloat(-24.0, 24.0) + (attempt * 37.0);
			float distance = Math.RandomFloat(0.0, 18.0 + attempt);
			vector candidate = JLH_AddonWorldQuery.GroundPosition(desired + (direction * Math.Min(attempt * 2.0, 26.0)) + (JLH_AddonWorldQuery.DirectionFromAngle(angle) * distance));
			if (!IsGroupPositionValid(candidate, reason))
				continue;

			position = candidate;
			return true;
		}

		if (reason == "")
			reason = "camp_assault_no_valid_enemy_position";

		return false;
	}

	protected bool SpawnCampAssaultGroupPlanAtPositions(notnull JLH_AmbientSkirmishGroupPlan plan, notnull array<vector> positions, notnull array<SCR_AIGroup> outGroups, out string reason)
	{
		outGroups.Clear();
		reason = "";

		if (!plan || plan.IsEmpty())
		{
			reason = "camp_assault_enemy_plan_empty";
			return false;
		}

		for (int i = 0; i < plan.Groups.Count(); i++)
		{
			JLH_DCF_EnemyGroupSelection selection = plan.Groups[i];
			if (!selection || selection.Prefab == "")
			{
				CleanupSpawnedGroups(outGroups, "camp_assault_enemy_partial_cleanup_invalid_selection");
				reason = "camp_assault_enemy_selection_invalid";
				return false;
			}

			vector spawnPosition = positions[Math.Min(i, positions.Count() - 1)];
			SCR_AIGroup group = JLH_AddonSpawnUtility.SpawnGroup(selection.Prefab, spawnPosition, "camp_assault_ussr_attacker_" + i.ToString(), selection.EstimatedUnits);
			if (!group)
			{
				CleanupSpawnedGroups(outGroups, "camp_assault_enemy_partial_cleanup_spawn_failed");
				reason = "camp_assault_enemy_spawn_failed";
				return false;
			}

			outGroups.Insert(group);
			LogEvent("camp_assault_attacker_group_spawned", "group=" + i.ToString() + " prefab=" + selection.Prefab + " pos=" + spawnPosition.ToString() + " estimatedUnits=" + selection.EstimatedUnits.ToString(), true);
		}

		return !outGroups.IsEmpty();
	}

	protected bool BuildCampAssaultExactPlans(out JLH_AmbientSkirmishGroupPlan friendlyPlan, out JLH_AmbientSkirmishGroupPlan enemyPlan, out string reason)
	{
		reason = "";
		friendlyPlan = null;
		enemyPlan = null;

		if (!JLH_AddonSpawnUtility.CanLoadPrefab(CAMP_ASSAULT_GROUP_US_DEFENDERS_6))
		{
			reason = "camp_assault_us_defender_group_missing";
			return false;
		}

		if (!JLH_AddonSpawnUtility.CanLoadPrefab(CAMP_ASSAULT_GROUP_USSR_FIRETEAM_4))
		{
			reason = "camp_assault_ussr_fireteam_group_missing";
			return false;
		}

		friendlyPlan = new JLH_AmbientSkirmishGroupPlan();
		friendlyPlan.Init("US", JLH_DCF_AmbientGroupRole.SQUAD_6, 6, "camp_assault_exact");
		AppendRoadblockAssaultPlanSelection(friendlyPlan, CAMP_ASSAULT_GROUP_US_DEFENDERS_6, "US", "camp_assault_exact:us_defenders_6", JLH_DCF_AmbientGroupRole.SQUAD_6, 6);

		enemyPlan = new JLH_AmbientSkirmishGroupPlan();
		enemyPlan.Init("USSR", JLH_DCF_AmbientGroupRole.FIRETEAM_4, 8, "camp_assault_exact");
		AppendRoadblockAssaultPlanSelection(enemyPlan, CAMP_ASSAULT_GROUP_USSR_FIRETEAM_4, "USSR", "camp_assault_exact:ussr_fireteam_0", JLH_DCF_AmbientGroupRole.FIRETEAM_4, 4);
		AppendRoadblockAssaultPlanSelection(enemyPlan, CAMP_ASSAULT_GROUP_USSR_FIRETEAM_4, "USSR", "camp_assault_exact:ussr_fireteam_1", JLH_DCF_AmbientGroupRole.FIRETEAM_4, 4);

		return true;
	}

	bool ExecuteRoadblockAssaultDefinition(notnull JLH_AmbientSkirmishEventDefinition definition, IEntity player, out JLH_AmbientSkirmishRecord record, out string reason)
	{
		record = null;
		reason = "";

		vector roadblockPosition;
		vector roadDirection;
		float roadWidth;
		float snapDistance;
		if (!SelectRoadblockAssaultRoad(player, roadblockPosition, roadDirection, roadWidth, snapDistance, reason))
			return false;

		LogEvent("roadblock_selected", "pos=" + roadblockPosition.ToString() + " direction=" + roadDirection.ToString() + " width=" + roadWidth.ToString() + " snapDistance=" + snapDistance.ToString(), true);

		vector attackerPosition;
		vector defenderATPosition;
		vector defenderFirePosition;
		if (!BuildRoadblockAssaultPositions(player, roadblockPosition, roadDirection, attackerPosition, defenderATPosition, defenderFirePosition, reason))
			return false;

		IEntity roadblockEntity = SpawnRoadblockAssaultComposition(roadblockPosition, roadDirection);
		if (!roadblockEntity)
		{
			reason = "roadblock_assault_composition_spawn_failed";
			return false;
		}

		LogEvent("roadblock_placed", "pos=" + roadblockPosition.ToString() + " direction=" + roadDirection.ToString() + " prefab=" + ROADBLOCK_ASSAULT_COMPOSITION_USSR, true);

		array<SCR_AIGroup> enemyGroups = {};
		SCR_AIGroup ussrATGroup = SpawnRoadblockAssaultGroup(ROADBLOCK_ASSAULT_GROUP_USSR_AT, defenderATPosition, "ussr_at_team", enemyGroups, reason, ROADBLOCK_ASSAULT_GROUP_USSR_AT_REQUESTED_UNITS);
		if (!ussrATGroup)
		{
			JLH_AddonSpawnUtility.DeleteRuntimeEntity(roadblockEntity, "roadblock_assault_orphan_cleanup_composition");
			return false;
		}

		SCR_AIGroup ussrFireGroup = SpawnRoadblockAssaultGroup(ROADBLOCK_ASSAULT_GROUP_USSR_FIRE, defenderFirePosition, "ussr_firegroup", enemyGroups, reason, ROADBLOCK_ASSAULT_GROUP_USSR_FIRE_REQUESTED_UNITS);
		if (!ussrFireGroup)
		{
			CleanupSpawnedGroups(enemyGroups, "roadblock_assault_enemy_partial_cleanup");
			JLH_AddonSpawnUtility.DeleteRuntimeEntity(roadblockEntity, "roadblock_assault_orphan_cleanup_composition");
			return false;
		}

		int enemyAlive = JLH_AddonSpawnUtility.CountAliveGroupUnits(ussrATGroup) + JLH_AddonSpawnUtility.CountAliveGroupUnits(ussrFireGroup);
		LogEvent("ussr_defenders_spawned", "groups=2 units=" + enemyAlive.ToString() + " atPos=" + defenderATPosition.ToString() + " firePos=" + defenderFirePosition.ToString(), true);

		array<SCR_AIGroup> friendlyGroups = {};
		SCR_AIGroup usAttackTeam = SpawnRoadblockAssaultGroup(ROADBLOCK_ASSAULT_GROUP_US, attackerPosition, "us_fireteam", friendlyGroups, reason, ROADBLOCK_ASSAULT_GROUP_US_REQUESTED_UNITS);
		if (!usAttackTeam)
		{
			CleanupSpawnedGroups(enemyGroups, "roadblock_assault_enemy_orphan_cleanup");
			JLH_AddonSpawnUtility.DeleteRuntimeEntity(roadblockEntity, "roadblock_assault_orphan_cleanup_composition");
			return false;
		}

		int friendlyAlive = JLH_AddonSpawnUtility.CountAliveGroupUnits(usAttackTeam);
		LogEvent("us_attack_team_spawned", "groups=1 units=" + friendlyAlive.ToString() + " pos=" + attackerPosition.ToString() + " target=" + roadblockPosition.ToString(), true);

		JLH_AmbientSkirmishGroupPlan friendlyPlan;
		JLH_AmbientSkirmishGroupPlan enemyPlan;
		BuildRoadblockAssaultExactPlans(friendlyPlan, enemyPlan);

		record = new JLH_AmbientSkirmishRecord();
		record.Init(JLH_DCF_AmbientSkirmishManager.NextSkirmishId(), m_sNodeKey, JLH_DCF_NodeDebug.EntityLabel(GetOwner()), definition, friendlyGroups, enemyGroups, roadblockPosition, attackerPosition, roadblockPosition, friendlyPlan, enemyPlan);
		record.Runtime.TrackEntity(roadblockEntity, "roadblock_assault_composition");
		AssignRoadblockAssaultBehavior(record, definition);
		record.Runtime.RefreshExclusions("roadblock_assault_spawn_refresh");

		LogEvent("roadblock_assault_started", "id=" + record.Id.ToString() + " center=" + roadblockPosition.ToString() + " usUnits=" + friendlyAlive.ToString() + " ussrUnits=" + enemyAlive.ToString(), true);
		return true;
	}

	bool ExecuteRoadblockBreakthroughDefinition(notnull JLH_AmbientSkirmishEventDefinition definition, IEntity player, out JLH_AmbientSkirmishRecord record, out string reason)
	{
		record = null;
		reason = "";

		JLH_AmbientSkirmishGroupPlan friendlyPlan;
		if (!SelectRoadblockBreakthroughPlan(definition, false, friendlyPlan, reason))
			return false;

		JLH_AmbientSkirmishGroupPlan enemyPlan;
		if (!SelectRoadblockBreakthroughPlan(definition, true, enemyPlan, reason))
			return false;

		vector roadblockPosition;
		vector roadDirection;
		float roadWidth;
		float snapDistance;
		if (!SelectRoadblockAssaultRoad(player, roadblockPosition, roadDirection, roadWidth, snapDistance, reason))
			return false;

		bool mainRoad = IsRoadblockBreakthroughMainRoad(roadWidth);
		ResourceName checkpointPrefab = ResolveRoadblockBreakthroughComposition(roadWidth);
		LogEvent("roadblock_breakthrough_road_selected", "pos=" + roadblockPosition.ToString() + " direction=" + roadDirection.ToString() + " width=" + roadWidth.ToString() + " roadType=" + RoadblockBreakthroughRoadTypeLabel(mainRoad) + " snapDistance=" + snapDistance.ToString(), true);

		vector attackerPosition;
		vector unusedDefenderPrimaryPosition;
		vector unusedDefenderSecondaryPosition;
		if (!BuildRoadblockAssaultPositions(player, roadblockPosition, roadDirection, attackerPosition, unusedDefenderPrimaryPosition, unusedDefenderSecondaryPosition, reason))
			return false;

		IEntity roadblockEntity = SpawnRoadblockComposition(checkpointPrefab, roadblockPosition, roadDirection, "roadblock_breakthrough_composition");
		if (!roadblockEntity)
		{
			reason = "roadblock_breakthrough_composition_spawn_failed";
			return false;
		}

		LogEvent("roadblock_breakthrough_checkpoint_spawned", "prefab=" + checkpointPrefab + " roadType=" + RoadblockBreakthroughRoadTypeLabel(mainRoad) + " pos=" + roadblockPosition.ToString(), true);

		vector defenderPrimaryPosition;
		vector defenderSecondaryPosition;
		BuildRoadblockBreakthroughDefenderPositions(roadblockPosition, roadDirection, defenderPrimaryPosition, defenderSecondaryPosition);

		array<SCR_AIGroup> enemyGroups = {};
		if (!SpawnRoadblockBreakthroughGroupPlan(enemyPlan, defenderPrimaryPosition, defenderSecondaryPosition, "enemy", enemyGroups, reason))
		{
			JLH_AddonSpawnUtility.DeleteRuntimeEntity(roadblockEntity, "roadblock_breakthrough_orphan_cleanup_composition");
			return false;
		}

		array<SCR_AIGroup> friendlyGroups = {};
		if (!SpawnRoadblockBreakthroughFriendlyGroupPlan(friendlyPlan, attackerPosition, roadblockPosition, friendlyGroups, reason))
		{
			CleanupSpawnedGroups(enemyGroups, "roadblock_breakthrough_enemy_orphan_cleanup");
			JLH_AddonSpawnUtility.DeleteRuntimeEntity(roadblockEntity, "roadblock_breakthrough_orphan_cleanup_composition");
			return false;
		}

		record = new JLH_AmbientSkirmishRecord();
		record.Init(JLH_DCF_AmbientSkirmishManager.NextSkirmishId(), m_sNodeKey, JLH_DCF_NodeDebug.EntityLabel(GetOwner()), definition, friendlyGroups, enemyGroups, roadblockPosition, attackerPosition, roadblockPosition, friendlyPlan, enemyPlan);
		record.Runtime.TrackEntity(roadblockEntity, "roadblock_breakthrough_composition");
		AssignRoadblockBreakthroughBehavior(record, definition);
		record.Runtime.RefreshExclusions("roadblock_breakthrough_spawn_refresh");

		int friendlyAlive = CountSkirmishGroupUnits(friendlyGroups);
		int enemyAlive = CountSkirmishGroupUnits(enemyGroups);
		LogEvent("roadblock_breakthrough_started", "id=" + record.Id.ToString() + " center=" + roadblockPosition.ToString() + " usPos=" + attackerPosition.ToString() + " usUnits=" + friendlyAlive.ToString() + " ussrUnits=" + enemyAlive.ToString(), true);
		return true;
	}

	bool ExecuteVehicleSkirmishDefinition(notnull JLH_AmbientSkirmishEventDefinition definition, IEntity player, out JLH_AmbientSkirmishRecord record, out string reason)
	{
		record = null;
		reason = "";

		ApplySupplyTruckRaidLoadedTruckPrefab(definition);
		ApplyVehicleSkirmishReliableVehiclePrefabs(definition);

		bool spawnFriendlyVehicle = ShouldSpawnFriendlyVehicle(definition);
		bool spawnEnemyVehicle = ShouldSpawnEnemyVehicle(definition);
		int friendlyVehicleCount;
		int enemyVehicleCount;
		int friendlySupportGroups;
		int enemySupportGroups;
		ResolveVehicleSkirmishMix(definition, spawnFriendlyVehicle, spawnEnemyVehicle, friendlyVehicleCount, enemyVehicleCount, friendlySupportGroups, enemySupportGroups);

		JLH_AmbientSkirmishGroupPlan friendlyPlan;
		if (!SelectVehicleFriendlyPlan(definition, friendlyVehicleCount, friendlySupportGroups, friendlyPlan, reason))
			return false;

		JLH_AmbientSkirmishGroupPlan enemyPlan;
		if (!SelectVehicleEnemyPlan(definition, enemyVehicleCount, enemySupportGroups, enemyPlan, reason))
			return false;

		vector center;
		if (!SelectContactCenter(player, definition, center, reason))
			return false;

		vector friendlyPosition;
		vector enemyPosition;
		float groupSeparation = Math.Clamp(m_fGroupSeparation * Math.Max(0.1, definition.SeparationMultiplier), 35.0, 700.0);
		int largestVehicleCount = friendlyVehicleCount;
		if (enemyVehicleCount > largestVehicleCount)
			largestVehicleCount = enemyVehicleCount;

		int largestSupportGroupCount = friendlySupportGroups;
		if (enemySupportGroups > largestSupportGroupCount)
			largestSupportGroupCount = enemySupportGroups;

		if (ShouldUseMountedVehicleCrewPlan(definition, largestVehicleCount, largestSupportGroupCount))
			groupSeparation = Math.Clamp(groupSeparation * 0.55, 70.0, 110.0);

		if (!BuildScenarioGroupPositions(definition, player, center, groupSeparation, friendlyPosition, enemyPosition, reason))
			return false;

		vector friendlyVehicleFacePosition = enemyPosition;
		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.SUPPLY_TRUCK_RAID)
			SnapSupplyTruckRaidSpawnToRoad(center, friendlyPosition, enemyPosition, friendlyVehicleFacePosition);

		array<IEntity> friendlyVehicles = {};
		array<IEntity> enemyVehicles = {};

		if (spawnFriendlyVehicle)
		{
			if (!SpawnSkirmishVehicles(definition, true, friendlyPosition, friendlyVehicleFacePosition, "friendly", friendlyVehicleCount, friendlyVehicles, reason))
				return false;
		}

		if (spawnEnemyVehicle)
		{
			if (!SpawnSkirmishVehicles(definition, false, enemyPosition, friendlyPosition, "enemy", enemyVehicleCount, enemyVehicles, reason))
			{
				CleanupSpawnedVehicles(friendlyVehicles, "ambient_skirmish_vehicle_orphan_cleanup");
				return false;
			}
		}

		array<IEntity> noFriendlyExactMarkers = {};
		array<IEntity> noEnemyExactMarkers = {};
		array<SCR_AIGroup> friendlyGroups = {};
		if (!SpawnGroupPlan(friendlyPlan, friendlyPosition, "friendly", noFriendlyExactMarkers, friendlyGroups, reason))
		{
			CleanupSpawnedVehicles(friendlyVehicles, "ambient_skirmish_friendly_vehicle_orphan_cleanup");
			CleanupSpawnedVehicles(enemyVehicles, "ambient_skirmish_enemy_vehicle_orphan_cleanup");
			return false;
		}

		array<SCR_AIGroup> enemyGroups = {};
		if (!SpawnGroupPlan(enemyPlan, enemyPosition, "enemy", noEnemyExactMarkers, enemyGroups, reason))
		{
			CleanupSpawnedGroups(friendlyGroups, "ambient_skirmish_friendly_orphan_cleanup");
			CleanupSpawnedVehicles(friendlyVehicles, "ambient_skirmish_friendly_vehicle_orphan_cleanup");
			CleanupSpawnedVehicles(enemyVehicles, "ambient_skirmish_enemy_vehicle_orphan_cleanup");
			return false;
		}

		record = new JLH_AmbientSkirmishRecord();
		record.Init(JLH_DCF_AmbientSkirmishManager.NextSkirmishId(), m_sNodeKey, JLH_DCF_NodeDebug.EntityLabel(GetOwner()), definition, friendlyGroups, enemyGroups, center, friendlyPosition, enemyPosition, friendlyPlan, enemyPlan);
		record.AttachVehicles(friendlyVehicles, enemyVehicles);
		BuildScenarioScene(record, definition);
		record.Runtime.RefreshExclusions("ambient_skirmish_vehicle_spawn_refresh");
		ScheduleVehicleSkirmishMount(record, definition, 0);
		string spawnLog = "id=" + record.Id.ToString();
		spawnLog = spawnLog + " event=" + definition.SkirmishEventName;
		spawnLog = spawnLog + " friendlyVehicles=" + friendlyVehicles.Count().ToString();
		spawnLog = spawnLog + " enemyVehicles=" + enemyVehicles.Count().ToString();
		spawnLog = spawnLog + " friendlyVehiclePrefab=" + definition.FriendlyVehiclePrefab;
		spawnLog = spawnLog + " enemyVehiclePrefab=" + definition.EnemyVehiclePrefab;
		spawnLog = spawnLog + " mount=queued";
		LogEvent("ambient_skirmish_vehicle_spawn_success", spawnLog, true);
		return true;
	}

	protected void ApplySupplyTruckRaidLoadedTruckPrefab(notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		if (definition.SkirmishEventType != JLH_AmbientSkirmishEventType.SUPPLY_TRUCK_RAID)
			return;

		if (SUPPLY_TRUCK_RAID_LOADED_TRUCK == "" || !JLH_AddonSpawnUtility.CanLoadPrefab(SUPPLY_TRUCK_RAID_LOADED_TRUCK))
			return;

		if (definition.FriendlyVehiclePrefab == SUPPLY_TRUCK_RAID_LOADED_TRUCK)
			return;

		ResourceName previousPrefab = definition.FriendlyVehiclePrefab;
		definition.FriendlyVehiclePrefab = SUPPLY_TRUCK_RAID_LOADED_TRUCK;
		definition.FriendlyVehicleSource = "supply_truck_raid_loaded_default";
		LogEvent("supply_truck_loaded_prefab", "previous=" + previousPrefab + " prefab=" + definition.FriendlyVehiclePrefab, true);
	}

	protected void ApplyVehicleSkirmishReliableVehiclePrefabs(notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		if (definition.SkirmishEventType != JLH_AmbientSkirmishEventType.VEHICLE_SKIRMISH)
			return;

		if (VEHICLE_SKIRMISH_DEFAULT_US_ARMED == "" || !JLH_AddonSpawnUtility.CanLoadPrefab(VEHICLE_SKIRMISH_DEFAULT_US_ARMED))
			return;

		if (definition.FriendlyVehiclePrefab == VEHICLE_SKIRMISH_DEFAULT_US_ARMED)
			return;

		ResourceName previousPrefab = definition.FriendlyVehiclePrefab;
		definition.FriendlyVehiclePrefab = VEHICLE_SKIRMISH_DEFAULT_US_ARMED;
		definition.FriendlyVehicleSource = "vehicle_skirmish_default_us_armed_gunner";
		LogEvent("vehicle_skirmish_friendly_vehicle_prefab", "previous=" + previousPrefab + " prefab=" + definition.FriendlyVehiclePrefab, true);
	}

	protected bool SnapSupplyTruckRaidSpawnToRoad(inout vector center, inout vector friendlyPosition, vector enemyPosition, inout vector friendlyVehicleFacePosition)
	{
		RoadNetworkManager roadManager = GetRoadNetworkManager();
		if (!roadManager)
		{
			LogEvent("supply_truck_road_snap_failed", "reason=road_network_missing pos=" + friendlyPosition.ToString(), true);
			return false;
		}

		vector roadPosition;
		vector roadDirection;
		float roadWidth;
		float snapDistance;
		string roadReason;
		string source = "friendly";
		if (!TryProjectToNearbyRoad(roadManager, friendlyPosition, SUPPLY_TRUCK_RAID_ROAD_SEARCH_RADIUS, roadPosition, roadDirection, roadWidth, snapDistance, roadReason))
		{
			source = "center";
			if (!TryProjectToNearbyRoad(roadManager, center, SUPPLY_TRUCK_RAID_ROAD_SEARCH_RADIUS, roadPosition, roadDirection, roadWidth, snapDistance, roadReason))
			{
				LogEvent("supply_truck_road_snap_failed", "reason=" + roadReason + " friendly=" + friendlyPosition.ToString() + " center=" + center.ToString(), true);
				return false;
			}
		}

		vector facingProbeForward = roadPosition + (roadDirection * 18.0);
		vector facingProbeRear = roadPosition - (roadDirection * 18.0);
		if (HorizontalDistanceSq(facingProbeForward, enemyPosition) > HorizontalDistanceSq(facingProbeRear, enemyPosition))
			roadDirection = roadDirection * -1.0;

		vector previousFriendlyPosition = friendlyPosition;
		friendlyPosition = JLH_AddonWorldQuery.GroundPosition(roadPosition);
		center = friendlyPosition;
		friendlyVehicleFacePosition = JLH_AddonWorldQuery.GroundPosition(friendlyPosition + (roadDirection * 30.0));

		LogEvent("supply_truck_road_snap", "source=" + source + " old=" + previousFriendlyPosition.ToString() + " road=" + friendlyPosition.ToString() + " dir=" + roadDirection.ToString() + " width=" + roadWidth.ToString() + " snapDistance=" + snapDistance.ToString(), true);
		return true;
	}

	bool ExecuteConvoyAmbushDefinition(notnull JLH_AmbientSkirmishEventDefinition definition, IEntity player, out JLH_AmbientSkirmishRecord record, out string reason)
	{
		record = null;
		reason = "";

		bool usAttacksUssr = definition.SkirmishEventType == JLH_AmbientSkirmishEventType.CONVOY_AMBUSH_US_ATTACKS_USSR;
		string convoyFactionKey = m_sFriendlyFactionKey;
		string ambushFactionKey = JLH_DCF_FactionCompatibilityRegistry.ResolveEnemyFactionKey(m_sFriendlyFactionKey, m_sEnemyFactionKey);
		if (usAttacksUssr)
		{
			convoyFactionKey = JLH_DCF_FactionCompatibilityRegistry.ResolveEnemyFactionKey(m_sFriendlyFactionKey, m_sEnemyFactionKey);
			ambushFactionKey = m_sFriendlyFactionKey;
		}

		if (convoyFactionKey == "" || ambushFactionKey == "")
		{
			reason = "convoy_ambush_faction_missing";
			return false;
		}

		vector markerPos;
		if (!SelectContactCenter(player, definition, markerPos, reason))
			return false;

		vector roadPos;
		vector roadDir;
		float roadWidth;
		float roadSnapDistance;
		string roadSource;
		if (!SelectConvoyAmbushRoad(player, markerPos, roadPos, roadDir, roadWidth, roadSnapDistance, roadSource, reason))
		{
			LogConvoyAmbush("road_snap_failed marker=" + markerPos.ToString() + " reason=" + reason);
			return false;
		}
		LogConvoyAmbush("road_snap source=" + roadSource + " marker=" + markerPos.ToString() + " road=" + roadPos.ToString() + " dir=" + roadDir.ToString() + " width=" + roadWidth.ToString() + " snapDistance=" + roadSnapDistance.ToString());

		ResourceName frontVehiclePrefab;
		ResourceName troopTruckPrefab;
		ResourceName rearVehiclePrefab;
		if (!ResolveConvoyAmbushVehiclePrefabs(definition, convoyFactionKey, usAttacksUssr, frontVehiclePrefab, troopTruckPrefab, rearVehiclePrefab, reason))
			return false;

		vector roadRight = Vector(roadDir[2], 0.0, -roadDir[0]);
		vector frontVehiclePos = JLH_AddonWorldQuery.GroundPosition(roadPos + (roadDir * 18.0));
		vector truckPos = JLH_AddonWorldQuery.GroundPosition(roadPos);
		vector rearVehiclePos = JLH_AddonWorldQuery.GroundPosition(roadPos - (roadDir * 18.0));

		array<IEntity> convoyVehicles = {};
		IEntity frontVehicle = SpawnConvoyAmbushVehicle(frontVehiclePrefab, frontVehiclePos, roadDir, "front", convoyVehicles, reason);
		if (!frontVehicle)
			return false;

		IEntity troopTruck = SpawnConvoyAmbushVehicle(troopTruckPrefab, truckPos, roadDir, "truck", convoyVehicles, reason);
		if (!troopTruck)
		{
			CleanupSpawnedVehicles(convoyVehicles, "convoy_ambush_vehicle_partial_cleanup");
			return false;
		}

		IEntity rearVehicle = SpawnConvoyAmbushVehicle(rearVehiclePrefab, rearVehiclePos, roadDir, "rear", convoyVehicles, reason);
		if (!rearVehicle)
		{
			CleanupSpawnedVehicles(convoyVehicles, "convoy_ambush_vehicle_partial_cleanup");
			return false;
		}

		JLH_AmbientSkirmishGroupPlan convoyPlan;
		ResourceName configuredConvoyTroopGroup = m_sConvoyAmbushUSTroopGroupPrefab;
		if (usAttacksUssr)
			configuredConvoyTroopGroup = "";

		if (!BuildConvoyAmbushUSTroopPlan(convoyFactionKey, configuredConvoyTroopGroup, convoyPlan, reason))
		{
			CleanupSpawnedVehicles(convoyVehicles, "convoy_ambush_vehicle_orphan_cleanup");
			return false;
		}

		array<SCR_AIGroup> convoyGroups = {};
		vector usTroopSpawn = ResolveConvoyAmbushGroupSpawnPosition(truckPos + (roadRight * 8.0), truckPos, "us_troops");
		array<vector> usTroopPositions = {};
		usTroopPositions.Insert(usTroopSpawn);
		if (!SpawnConvoyAmbushPlanAtPositions(convoyPlan, usTroopPositions, "convoy_troops", convoyGroups, reason))
		{
			CleanupSpawnedVehicles(convoyVehicles, "convoy_ambush_vehicle_orphan_cleanup");
			return false;
		}

		int convoyTruckGroupCount = convoyGroups.Count();
		SCR_AIGroup convoyTroopGroup = null;
		if (!convoyGroups.IsEmpty())
			convoyTroopGroup = convoyGroups[0];

		LogConvoyAmbush("spawn_convoy_troops faction=" + convoyFactionKey + " count=" + CountConvoyAmbushGroupsUnits(convoyGroups).ToString() + " pos=" + usTroopSpawn.ToString());

		JLH_AmbientSkirmishGroupPlan frontCasualtyPlan;
		SCR_AIGroup frontCasualtyGroup;
		if (!SpawnConvoyAmbushVehicleCasualtyGroup(frontVehiclePos, convoyFactionKey, "front", frontCasualtyPlan, frontCasualtyGroup, reason))
		{
			CleanupSpawnedGroups(convoyGroups, "convoy_ambush_convoy_troop_orphan_cleanup");
			CleanupSpawnedVehicles(convoyVehicles, "convoy_ambush_vehicle_orphan_cleanup");
			return false;
		}

		convoyPlan.AppendPlan(frontCasualtyPlan);
		convoyGroups.Insert(frontCasualtyGroup);

		JLH_AmbientSkirmishGroupPlan rearCasualtyPlan;
		SCR_AIGroup rearCasualtyGroup;
		if (!SpawnConvoyAmbushVehicleCasualtyGroup(rearVehiclePos, convoyFactionKey, "rear", rearCasualtyPlan, rearCasualtyGroup, reason))
		{
			CleanupSpawnedGroups(convoyGroups, "convoy_ambush_convoy_troop_orphan_cleanup");
			CleanupSpawnedVehicles(convoyVehicles, "convoy_ambush_vehicle_orphan_cleanup");
			return false;
		}

		convoyPlan.AppendPlan(rearCasualtyPlan);
		convoyGroups.Insert(rearCasualtyGroup);

		JLH_AmbientSkirmishGroupPlan ambushPlan;
		bool ambushPlanBuilt = false;
		if (usAttacksUssr)
			ambushPlanBuilt = BuildConvoyAmbushUSAmbushPlan(ambushFactionKey, ambushPlan, reason);
		else
			ambushPlanBuilt = BuildConvoyAmbushUSSRPlan(ambushFactionKey, ambushPlan, reason);

		if (!ambushPlanBuilt)
		{
			CleanupSpawnedGroups(convoyGroups, "convoy_ambush_convoy_troop_orphan_cleanup");
			CleanupSpawnedVehicles(convoyVehicles, "convoy_ambush_vehicle_orphan_cleanup");
			return false;
		}

		array<vector> ambushPositions = {};
		BuildConvoyAmbushEnemyPositions(roadPos, roadDir, roadRight, ambushPlan.GroupCount(), ambushPositions);

		array<SCR_AIGroup> ambushGroups = {};
		string ambushSideLabel = "ussr_ambush";
		if (usAttacksUssr)
			ambushSideLabel = "us_ambush";

		if (!SpawnConvoyAmbushPlanAtPositions(ambushPlan, ambushPositions, ambushSideLabel, ambushGroups, reason))
		{
			CleanupSpawnedGroups(convoyGroups, "convoy_ambush_convoy_troop_orphan_cleanup");
			CleanupSpawnedVehicles(convoyVehicles, "convoy_ambush_vehicle_orphan_cleanup");
			return false;
		}

		record = new JLH_AmbientSkirmishRecord();
		array<ResourceName> convoyVehiclePrefabs = {};
		convoyVehiclePrefabs.Insert(frontVehiclePrefab);
		convoyVehiclePrefabs.Insert(troopTruckPrefab);
		convoyVehiclePrefabs.Insert(rearVehiclePrefab);
		array<IEntity> noVehicles = {};
		array<ResourceName> noVehiclePrefabs = {};

		if (usAttacksUssr)
		{
			record.Init(JLH_DCF_AmbientSkirmishManager.NextSkirmishId(), m_sNodeKey, JLH_DCF_NodeDebug.EntityLabel(GetOwner()), definition, ambushGroups, convoyGroups, truckPos, roadPos, truckPos, ambushPlan, convoyPlan);
			record.AttachVehicles(noVehicles, convoyVehicles);
			record.AttachVehiclePrefabs(noVehiclePrefabs, convoyVehiclePrefabs);
		}
		else
		{
			record.Init(JLH_DCF_AmbientSkirmishManager.NextSkirmishId(), m_sNodeKey, JLH_DCF_NodeDebug.EntityLabel(GetOwner()), definition, convoyGroups, ambushGroups, truckPos, truckPos, roadPos, convoyPlan, ambushPlan);
			record.AttachVehicles(convoyVehicles, noVehicles);
			record.AttachVehiclePrefabs(convoyVehiclePrefabs, noVehiclePrefabs);
		}

		record.ConvoyAmbushTruck = troopTruck;
		record.ConvoyAmbushUSGroup = convoyTroopGroup;
		record.ConvoyAmbushTruckGroups.Clear();
		for (int convoyTruckGroupIndex = 0; convoyTruckGroupIndex < convoyTruckGroupCount && convoyTruckGroupIndex < convoyGroups.Count(); convoyTruckGroupIndex++)
		{
			SCR_AIGroup convoyTruckGroup = convoyGroups[convoyTruckGroupIndex];
			if (convoyTruckGroup)
				record.ConvoyAmbushTruckGroups.Insert(convoyTruckGroup);
		}

		record.ConvoyAmbushFrontCasualtyGroup = frontCasualtyGroup;
		record.ConvoyAmbushRearCasualtyGroup = rearCasualtyGroup;

		PrepareConvoyAmbushVehicleCasualties(record, frontCasualtyGroup, frontVehicle, "front");
		PrepareConvoyAmbushVehicleCasualties(record, rearCasualtyGroup, rearVehicle, "rear");
		for (int prepareTruckGroupIndex = 0; prepareTruckGroupIndex < record.ConvoyAmbushTruckGroups.Count(); prepareTruckGroupIndex++)
			PrepareConvoyAmbushUSTroops(record, record.ConvoyAmbushTruckGroups[prepareTruckGroupIndex], troopTruck, truckPos);

		AssignConvoyAmbushBehavior(record, definition, truckPos);
		ScheduleConvoyAmbushStagedDestruction(record, frontVehicle, rearVehicle);
		record.Runtime.RefreshExclusions("convoy_ambush_spawn_refresh");

		LogConvoyAmbush("truck_preserved");
		return true;
	}

	protected bool SelectConvoyAmbushRoad(IEntity player, vector contactCenter, out vector roadPos, out vector roadDir, out float roadWidth, out float snapDistance, out string source, out string reason)
	{
		roadPos = "0 0 0";
		roadDir = "0 0 1";
		roadWidth = 0.0;
		snapDistance = 0.0;
		source = "";
		reason = "";

		RoadNetworkManager roadManager = GetRoadNetworkManager();
		if (!roadManager)
		{
			reason = "convoy_ambush_road_network_missing";
			return false;
		}

		float directSnapRadius = Math.Max(0.0, m_fConvoyAmbushRoadSearchRadius);
		float candidateRoadWidth;
		float candidateSnapDistance;
		float candidateScore;
		string candidateReason;
		if (TryResolveConvoyAmbushRoadCandidate(roadManager, contactCenter, player, directSnapRadius, contactCenter, roadPos, roadDir, roadWidth, snapDistance, candidateScore, candidateReason))
		{
			source = "contact_center";
			return true;
		}

		if (candidateReason != "")
			reason = candidateReason;

		IEntity owner = GetOwner();
		if (!owner)
		{
			if (reason == "")
				reason = "area_missing";
			return false;
		}

		// Markerless random convoy areas must search the area, not only the random contact center.
		vector anchor = owner.GetOrigin();
		string areaSource = "area";
		bool hasChildSkirmishMarkers = m_bUseChildMarkers && HasChildSkirmishMarkers();
		if (m_ActiveScenarioMarker && m_ActiveScenarioMarker.GetOwner())
		{
			anchor = m_ActiveScenarioMarker.GetOwner().GetOrigin();
			areaSource = "scenario_marker_area";
		}
		else if (hasChildSkirmishMarkers)
		{
			anchor = contactCenter;
			areaSource = "child_marker_area";
		}

		bool found = false;
		float bestScore = -99999999.0;
		string bestSource = "";
		float areaSearchRadius = Math.Max(CONVOY_AMBUSH_AREA_ROAD_SEARCH_MIN_RADIUS, GetEffectiveSpawnRadius());
		if (!m_ActiveScenarioMarker && !hasChildSkirmishMarkers)
			areaSearchRadius = Math.Max(areaSearchRadius, GetEffectiveActivationRadius());
		areaSearchRadius = Math.Max(areaSearchRadius, directSnapRadius);
		float sampleSnapRadius = Math.Max(directSnapRadius, Math.Max(CONVOY_AMBUSH_AREA_ROAD_SAMPLE_MIN_SNAP, Math.Min(CONVOY_AMBUSH_AREA_ROAD_SAMPLE_MAX_SNAP, areaSearchRadius * 0.35)));

		vector candidateRoadPos;
		vector candidateRoadDir;
		if (TryResolveConvoyAmbushRoadCandidate(roadManager, anchor, player, areaSearchRadius, anchor, candidateRoadPos, candidateRoadDir, candidateRoadWidth, candidateSnapDistance, candidateScore, candidateReason))
		{
			found = true;
			bestScore = candidateScore;
			roadPos = candidateRoadPos;
			roadDir = candidateRoadDir;
			roadWidth = candidateRoadWidth;
			snapDistance = candidateSnapDistance;
			bestSource = areaSource + "_nearest";
		}
		else if (candidateReason != "")
		{
			reason = candidateReason;
		}

		int attempts = SPAWN_CANDIDATE_ATTEMPTS * 2;
		for (int i = 0; i < attempts; i++)
		{
			vector candidate = JLH_AddonWorldQuery.GroundPosition(anchor + JLH_AddonWorldQuery.RandomOffset(0.0, areaSearchRadius));
			if (!TryResolveConvoyAmbushRoadCandidate(roadManager, candidate, player, sampleSnapRadius, anchor, candidateRoadPos, candidateRoadDir, candidateRoadWidth, candidateSnapDistance, candidateScore, candidateReason))
			{
				if (candidateReason != "")
					reason = candidateReason;
				continue;
			}

			if (!found || candidateScore > bestScore)
			{
				found = true;
				bestScore = candidateScore;
				roadPos = candidateRoadPos;
				roadDir = candidateRoadDir;
				roadWidth = candidateRoadWidth;
				snapDistance = candidateSnapDistance;
				bestSource = areaSource + "_sample";
			}
		}

		if (!found)
		{
			if (reason == "")
				reason = "convoy_ambush_no_valid_road";
			return false;
		}

		source = bestSource;
		return true;
	}

	protected bool TryResolveConvoyAmbushRoadCandidate(RoadNetworkManager roadManager, vector candidate, IEntity player, float maxSnapDistance, vector anchor, out vector roadPosition, out vector roadDirection, out float roadWidth, out float snapDistance, out float score, out string reason)
	{
		roadPosition = "0 0 0";
		roadDirection = "0 0 1";
		roadWidth = 0.0;
		snapDistance = 0.0;
		score = 0.0;
		reason = "";

		if (!TryProjectToNearbyRoad(roadManager, candidate, maxSnapDistance, roadPosition, roadDirection, roadWidth, snapDistance, reason))
		{
			if (reason != "")
				reason = "convoy_ambush_" + reason;
			return false;
		}

		if (JLH_AddonWorldQuery.IsWaterOrNearWater(roadPosition, CONVOY_AMBUSH_ROAD_WATER_CLEARANCE_RADIUS))
		{
			reason = "convoy_ambush_road_water";
			return false;
		}

		float slope = JLH_AddonWorldQuery.GetSlopeDegrees(roadPosition);
		if (slope > CONVOY_AMBUSH_MAX_ROAD_SLOPE_DEGREES)
		{
			reason = "convoy_ambush_road_too_steep";
			return false;
		}

		if (!IsCenterCandidateValid(roadPosition, reason))
		{
			reason = "convoy_ambush_center_" + reason;
			return false;
		}

		roadDirection = NormalizeConvoyAmbushDirection(roadDirection);
		score = (roadWidth * 10.0) - (snapDistance * 0.8) - (slope * 2.0) - (vector.Distance(anchor, roadPosition) * 0.04);
		if (player)
		{
			float playerDistance = vector.Distance(player.GetOrigin(), roadPosition);
			score = score - (playerDistance * 0.06);
			if (JLH_AddonWorldQuery.HasLineOfSight(player, roadPosition))
				score = score - 12.0;
		}

		return true;
	}

	protected vector ResolveConvoyAmbushFallbackDirection(IEntity player, vector roadPos)
	{
		if (player)
			return NormalizeConvoyAmbushDirection(DirectionFromTo(player.GetOrigin(), roadPos));

		return NormalizeConvoyAmbushDirection(JLH_AddonWorldQuery.DirectionFromAngle(Math.RandomFloat(0.0, 360.0)));
	}

	protected vector NormalizeConvoyAmbushDirection(vector direction)
	{
		direction[1] = 0.0;
		float length = Math.Sqrt((direction[0] * direction[0]) + (direction[2] * direction[2]));
		if (length <= 0.001)
			return "0 0 1";

		return Vector(direction[0] / length, 0.0, direction[2] / length);
	}

	protected bool ResolveConvoyAmbushVehiclePrefabs(notnull JLH_AmbientSkirmishEventDefinition definition, string convoyFactionKey, bool usAttacksUssr, out ResourceName frontVehiclePrefab, out ResourceName troopTruckPrefab, out ResourceName rearVehiclePrefab, out string reason)
	{
		frontVehiclePrefab = "";
		troopTruckPrefab = "";
		rearVehiclePrefab = "";
		reason = "";

		ResourceName frontConfigured = m_sConvoyAmbushFrontVehiclePrefab;
		ResourceName truckConfigured = m_sConvoyAmbushTroopTruckPrefab;
		ResourceName rearConfigured = m_sConvoyAmbushRearVehiclePrefab;
		ResourceName defaultTruck = CONVOY_AMBUSH_DEFAULT_TROOP_TRUCK;
		if (usAttacksUssr)
		{
			defaultTruck = CONVOY_AMBUSH_DEFAULT_USSR_TROOP_TRUCK;
			if (truckConfigured == CONVOY_AMBUSH_DEFAULT_TROOP_TRUCK)
				truckConfigured = "";
		}

		if (!ResolveConvoyAmbushVehiclePrefab(definition, convoyFactionKey, "front", frontConfigured, "", true, false, frontVehiclePrefab, reason))
			return false;

		if (!ResolveConvoyAmbushVehiclePrefab(definition, convoyFactionKey, "truck", truckConfigured, defaultTruck, false, true, troopTruckPrefab, reason))
			return false;

		if (!ResolveConvoyAmbushVehiclePrefab(definition, convoyFactionKey, "rear", rearConfigured, "", true, false, rearVehiclePrefab, reason))
			return false;

		return true;
	}

	protected bool ResolveConvoyAmbushVehiclePrefab(notnull JLH_AmbientSkirmishEventDefinition definition, string convoyFactionKey, string role, ResourceName configuredPrefab, ResourceName defaultPrefab, bool armedLight, bool troopTruck, out ResourceName vehiclePrefab, out string reason)
	{
		vehiclePrefab = "";
		reason = "";

		if (configuredPrefab != "" && configuredPrefab != ResourceName.Empty)
		{
			if (JLH_AddonSpawnUtility.CanLoadPrefab(configuredPrefab))
			{
				vehiclePrefab = configuredPrefab;
				LogConvoyAmbush("vehicle_prefab role=" + role + " source=configured prefab=" + vehiclePrefab);
				return true;
			}

			LogConvoyAmbush("vehicle_prefab_override_invalid role=" + role + " prefab=" + configuredPrefab);
		}

		if (troopTruck && defaultPrefab != "" && JLH_AddonSpawnUtility.CanLoadPrefab(defaultPrefab))
		{
			vehiclePrefab = defaultPrefab;
			LogConvoyAmbush("vehicle_prefab role=truck faction=" + convoyFactionKey + " source=default_transport prefab=" + vehiclePrefab);
			return true;
		}

		array<string> requiredLabels = {};
		array<string> preferredLabels = {};
		array<string> excludedLabels = {};
		array<string> fallbackLabels = {};
		BuildConvoyAmbushVehicleLabels(armedLight, troopTruck, requiredLabels, preferredLabels, excludedLabels, fallbackLabels);

		JLH_CORE_VehicleLabelSelection selection;
		if (JLH_CORE_CompatibilityResolver.TrySelectVehiclePrefabByLabelFilters(convoyFactionKey, requiredLabels, preferredLabels, excludedLabels, fallbackLabels, selection, reason))
		{
			vehiclePrefab = selection.Prefab;
			LogConvoyAmbush("vehicle_prefab role=" + role + " faction=" + convoyFactionKey + " source=" + selection.Source + " preferredMatches=" + selection.PreferredMatches.ToString() + " prefab=" + vehiclePrefab);
			return true;
		}

		if (armedLight && IsUSSRLikeConvoyFaction(convoyFactionKey) && JLH_AddonSpawnUtility.CanLoadPrefab(SCENE_USSR_UAZ))
		{
			vehiclePrefab = SCENE_USSR_UAZ;
			reason = "";
			LogConvoyAmbush("vehicle_prefab role=" + role + " faction=" + convoyFactionKey + " source=fallback_ussr_uaz prefab=" + vehiclePrefab);
			return true;
		}

		reason = "convoy_ambush_vehicle_prefab_missing_" + role + "_" + reason;
		return false;
	}

	protected void BuildConvoyAmbushVehicleLabels(bool armedLight, bool troopTruck, notnull array<string> requiredLabels, notnull array<string> preferredLabels, notnull array<string> excludedLabels, notnull array<string> fallbackLabels)
	{
		requiredLabels.Clear();
		preferredLabels.Clear();
		excludedLabels.Clear();
		fallbackLabels.Clear();

		if (armedLight)
		{
			AddVehicleLabelName(requiredLabels, "TRAIT_ARMED");
			AddVehicleLabelName(preferredLabels, "VEHICLE_LIGHT");
			AddVehicleLabelName(preferredLabels, "VEHICLE_CAR");
			AddVehicleLabelName(fallbackLabels, "TRAIT_ARMED");
			AddVehicleLabelName(excludedLabels, "TRAIT_ARMORED");
			AddVehicleLabelName(excludedLabels, "VEHICLE_ARMORED");
			AddVehicleLabelName(excludedLabels, "VEHICLE_APC");
			AddVehicleLabelName(excludedLabels, "VEHICLE_TANK");
			AddVehicleLabelName(excludedLabels, "VEHICLE_HEAVY");
			AddVehicleLabelName(excludedLabels, "TRAIT_SUPPLY");
		}
		else if (troopTruck)
		{
			AddVehicleLabelName(requiredLabels, "TRAIT_TRANSPORT");
			AddVehicleLabelName(preferredLabels, "VEHICLE_TRUCK");
			AddVehicleLabelName(preferredLabels, "TRAIT_TRANSPORT");
			AddVehicleLabelName(fallbackLabels, "VEHICLE_TRUCK");
			AddVehicleLabelName(fallbackLabels, "TRAIT_TRANSPORT");
			AddVehicleLabelName(excludedLabels, "TRAIT_ARMED");
			AddVehicleLabelName(excludedLabels, "VEHICLE_TANK");
			AddVehicleLabelName(excludedLabels, "VEHICLE_APC");
		}

		AddGroundVehicleExclusionNames(excludedLabels);
	}

	protected IEntity SpawnConvoyAmbushVehicle(ResourceName prefab, vector position, vector roadDir, string role, notnull array<IEntity> outVehicles, out string reason)
	{
		vector facePosition = position + (roadDir * 12.0);
		IEntity vehicle = SpawnSkirmishVehicle(prefab, position, facePosition, "convoy_" + role, reason);
		if (!vehicle)
			return null;

		ApplyConvoyAmbushVehicleSetPieceTransform(vehicle, position, roadDir, role);
		if (role == "truck")
			EnsureConvoyAmbushTruckRunning(vehicle);

		outVehicles.Insert(vehicle);
		LogConvoyAmbush("spawn_vehicle role=" + role + " prefab=" + prefab + " pos=" + position.ToString() + " dir=" + roadDir.ToString());
		return vehicle;
	}

	protected void ApplyConvoyAmbushVehicleSetPieceTransform(IEntity vehicle, vector position, vector roadDir, string role)
	{
		if (!vehicle)
			return;

		vector forward = NormalizeConvoyAmbushDirection(roadDir);
		vector right = Vector(forward[2], 0.0, -forward[0]);

		if (role == "front")
		{
			vector skewedForward = NormalizeConvoyAmbushDirection(forward - (right * 0.58));
			vector skewedRight = Vector(skewedForward[2], 0.0, -skewedForward[0]);
			vector skewedUp = Vector(0.0, 1.0, 0.0);
			vector skewedPos = JLH_AddonWorldQuery.GroundPosition(position + (right * 1.7) + (forward * 0.8));
			skewedPos[1] = skewedPos[1] + 0.08;
			SetConvoyAmbushVehicleTransform(vehicle, skewedPos, skewedForward, skewedRight, skewedUp);
			LogConvoyAmbush("wreck_pose role=front mode=grounded_skew pos=" + skewedPos.ToString() + " dir=" + skewedForward.ToString());
			return;
		}

		if (role == "rear")
		{
			vector shiftedForward = NormalizeConvoyAmbushDirection(forward + (right * 0.36));
			vector shiftedRight = Vector(shiftedForward[2], 0.0, -shiftedForward[0]);
			vector worldUp = Vector(0.0, 1.0, 0.0);
			float rollAmount = 0.42;
			vector tiltedRight = NormalizeConvoyAmbushVector3D((shiftedRight * 0.92) + (worldUp * rollAmount), shiftedRight);
			vector tiltedUp = NormalizeConvoyAmbushVector3D((worldUp * 0.92) - (shiftedRight * rollAmount), worldUp);
			vector shiftedPos = JLH_AddonWorldQuery.GroundPosition(position - (right * 1.8) - (forward * 1.0));
			shiftedPos[1] = shiftedPos[1] + 0.25;
			SetConvoyAmbushVehicleTransform(vehicle, shiftedPos, shiftedForward, tiltedRight, tiltedUp);
			LogConvoyAmbush("wreck_pose role=rear mode=skewed_tilt pos=" + shiftedPos.ToString() + " dir=" + shiftedForward.ToString());
		}
	}

	protected void SetConvoyAmbushVehicleTransform(IEntity vehicle, vector position, vector forward, vector right, vector up)
	{
		if (!vehicle)
			return;

		vector transform[4];
		transform[0] = right;
		transform[1] = up;
		transform[2] = forward;
		transform[3] = position;
		vehicle.SetTransform(transform);
	}

	protected vector NormalizeConvoyAmbushVector3D(vector direction, vector fallback)
	{
		float length = Math.Sqrt((direction[0] * direction[0]) + (direction[1] * direction[1]) + (direction[2] * direction[2]));
		if (length <= 0.001)
			return fallback;

		return Vector(direction[0] / length, direction[1] / length, direction[2] / length);
	}

	protected void EnsureConvoyAmbushTruckRunning(IEntity truck)
	{
		if (!truck)
			return;

		BaseVehicleControllerComponent controller = BaseVehicleControllerComponent.Cast(truck.FindComponent(BaseVehicleControllerComponent));
		if (!controller)
		{
			LogConvoyAmbush("truck_runtime engine=false headlights=false reason=no_controller");
			return;
		}

		RefillConvoyAmbushTruckFuel(controller);
		controller.SetEngineDrowned(false);
		controller.SetEngineStartupChance(100.0);
		controller.ForceStartEngine();
		if (!controller.IsEngineOn())
			controller.StartEngine();

		bool engineOn = controller.IsEngineOn();

		bool headlightsOn = false;
		BaseLightManagerComponent lightManager = controller.GetLightManager();
		if (lightManager)
		{
			lightManager.SetLightsState(ELightType.AllLights, true);
			lightManager.SetLightsState(ELightType.Presence, true);
			lightManager.SetLightsState(ELightType.Head, true);
			lightManager.SetLightsState(ELightType.HiBeam, true);
			lightManager.SetLightsState(ELightType.Rear, true);
			lightManager.SetLightsState(ELightType.Dashboard, true);
			headlightsOn = lightManager.GetLightsState(ELightType.Head);
		}

		LogConvoyAmbush("truck_runtime engine=" + JLH_DCF_NodeDebug.BoolLabel(engineOn) + " headlights=" + JLH_DCF_NodeDebug.BoolLabel(headlightsOn));
	}

	protected bool RefillConvoyAmbushTruckFuel(BaseVehicleControllerComponent controller)
	{
		if (!controller)
			return false;

		FuelManagerComponent fuelManager = controller.GetFuelManager();
		if (!fuelManager)
			return false;

		array<BaseFuelNode> fuelNodes = {};
		int nodeCount = fuelManager.GetFuelNodesList(fuelNodes);
		if (nodeCount <= 0)
			return false;

		bool refilled = false;
		foreach (BaseFuelNode fuelNode : fuelNodes)
		{
			if (!fuelNode)
				continue;

			float nodeMax = fuelNode.GetMaxFuel();
			if (nodeMax <= 0.0)
				continue;

			if (fuelNode.GetFuel() >= nodeMax)
				continue;

			fuelNode.SetFuel(nodeMax);
			refilled = true;
		}

		return refilled;
	}

	protected bool BuildConvoyAmbushUSTroopPlan(string convoyFactionKey, ResourceName configuredGroupPrefab, out JLH_AmbientSkirmishGroupPlan plan, out string reason)
	{
		plan = null;
		reason = "";

		if (TryBuildConvoyAmbushConfiguredGroupPlan(configuredGroupPrefab, convoyFactionKey, JLH_DCF_AmbientGroupRole.CONVOY_ESCORT, "configured:convoy_troops", 15, false, plan))
			return true;

		if (JLH_AmbientSkirmishGroupComposer.TryBuildFriendlySinglePlan(convoyFactionKey, JLH_DCF_AmbientGroupRole.CONVOY_ESCORT, 15, 15, plan, reason))
			return true;

		if (JLH_AmbientSkirmishGroupComposer.TryBuildFriendlyPlan(convoyFactionKey, JLH_DCF_AmbientGroupRole.CONVOY_ESCORT, 15, 15, plan, reason))
			return true;

		if (JLH_AmbientSkirmishGroupComposer.TryBuildFriendlyPlan(convoyFactionKey, JLH_DCF_AmbientGroupRole.CONVOY_ESCORT, 14, 16, plan, reason))
			return true;

		if (reason == "")
			reason = "convoy_ambush_us_troop_plan_failed";

		return false;
	}

	protected bool BuildConvoyAmbushVehicleCasualtyPlan(string convoyFactionKey, out JLH_AmbientSkirmishGroupPlan plan, out string reason)
	{
		plan = null;
		reason = "";

		if (JLH_AmbientSkirmishGroupComposer.TryBuildFriendlySinglePlan(convoyFactionKey, JLH_DCF_AmbientGroupRole.FIRETEAM_4, 4, 4, plan, reason))
			return true;

		if (reason == "")
			reason = "convoy_ambush_vehicle_casualty_plan_failed";

		return false;
	}

	protected bool SpawnConvoyAmbushVehicleCasualtyGroup(vector vehiclePos, string convoyFactionKey, string role, out JLH_AmbientSkirmishGroupPlan casualtyPlan, out SCR_AIGroup casualtyGroup, out string reason)
	{
		casualtyPlan = null;
		casualtyGroup = null;
		reason = "";

		if (!BuildConvoyAmbushVehicleCasualtyPlan(convoyFactionKey, casualtyPlan, reason))
			return false;

		vector spawnPos = ResolveConvoyAmbushGroupSpawnPosition(vehiclePos, vehiclePos, role + "_vehicle_casualties");
		array<vector> positions = {};
		positions.Insert(spawnPos);
		array<SCR_AIGroup> groups = {};
		if (!SpawnConvoyAmbushPlanAtPositions(casualtyPlan, positions, "convoy_vehicle_casualties_" + role, groups, reason))
			return false;

		if (groups.IsEmpty())
		{
			reason = "convoy_ambush_vehicle_casualty_group_missing_" + role;
			return false;
		}

		casualtyGroup = groups[0];
		LogConvoyAmbush("spawn_convoy_vehicle_casualties role=" + role + " faction=" + convoyFactionKey + " count=" + JLH_AddonSpawnUtility.CountAliveGroupUnits(casualtyGroup).ToString() + " pos=" + spawnPos.ToString());
		return true;
	}

	protected bool BuildConvoyAmbushUSSRPlan(string enemyFactionKey, out JLH_AmbientSkirmishGroupPlan plan, out string reason)
	{
		plan = null;
		reason = "";

		if (enemyFactionKey == "")
		{
			reason = "convoy_ambush_enemy_faction_not_compatible";
			return false;
		}

		int minGroups = Math.Clamp(m_iConvoyAmbushMinEnemyGroups, 2, 6);
		int maxGroups = Math.Clamp(m_iConvoyAmbushMaxEnemyGroups, minGroups, 6);
		int groupCount = Math.RandomInt(minGroups, maxGroups + 1);

		plan = new JLH_AmbientSkirmishGroupPlan();
		plan.Init(enemyFactionKey, JLH_DCF_AmbientGroupRole.MG_TEAM, 0, "convoy_ambush_no_at:" + groupCount.ToString());

		for (int i = 0; i < groupCount; i++)
		{
			ResourceName configuredGroup = "";
			if (i == 0)
				configuredGroup = m_sConvoyAmbushUSSRFrontGroupPrefab;
			else if (i == 1)
				configuredGroup = m_sConvoyAmbushUSSRRearGroupPrefab;

			JLH_AmbientSkirmishGroupPlan configuredPlan;
			if (TryBuildConvoyAmbushConfiguredGroupPlan(configuredGroup, enemyFactionKey, JLH_DCF_AmbientGroupRole.MG_TEAM, "configured:ussr_ambush_" + i.ToString(), 4, true, configuredPlan))
			{
				plan.AppendPlan(configuredPlan);
				continue;
			}

			JLH_DCF_AmbientGroupRole role = JLH_DCF_AmbientGroupRole.MG_TEAM;
			if (m_bConvoyAmbushAllowAT)
				role = JLH_DCF_AmbientGroupRole.FOREST_AMBUSH;

			JLH_DCF_AmbientGroupSelection selection;
			if (!JLH_DCF_AmbientGroupRoleResolver.TrySelectFactionGroup(enemyFactionKey, role, 4, 6, selection, reason))
			{
				if (role == JLH_DCF_AmbientGroupRole.MG_TEAM && JLH_DCF_AmbientGroupRoleResolver.TrySelectFactionGroup(enemyFactionKey, JLH_DCF_AmbientGroupRole.FOREST_AMBUSH, 4, 6, selection, reason))
					role = JLH_DCF_AmbientGroupRole.FOREST_AMBUSH;
				else
				{
					plan = null;
					if (reason == "")
						reason = "convoy_ambush_ussr_group_selection_failed";

					return false;
				}
			}

			if (!m_bConvoyAmbushAllowAT && IsLikelyATGroupPrefab(selection.Prefab))
			{
				plan = null;
				reason = "convoy_ambush_at_group_rejected";
				return false;
			}

			plan.AddSelection(selection);
		}

		plan.TargetUnits = plan.TotalUnits;
		LogConvoyAmbush("ussr_plan groups=" + plan.GroupCount().ToString() + " units=" + plan.TotalUnits.ToString() + " allowAT=" + JLH_DCF_NodeDebug.BoolLabel(m_bConvoyAmbushAllowAT));
		return !plan.IsEmpty();
	}

	protected bool BuildConvoyAmbushUSAmbushPlan(string ambushFactionKey, out JLH_AmbientSkirmishGroupPlan plan, out string reason)
	{
		plan = null;
		reason = "";

		if (ambushFactionKey == "")
		{
			reason = "convoy_ambush_us_ambush_faction_missing";
			return false;
		}

		plan = new JLH_AmbientSkirmishGroupPlan();
		plan.Init(ambushFactionKey, JLH_DCF_AmbientGroupRole.FIRETEAM_4, 0, "convoy_ambush_us_2x4_no_at");

		for (int i = 0; i < 2; i++)
		{
			JLH_DCF_AmbientGroupSelection selection;
			if (!JLH_DCF_AmbientGroupRoleResolver.TrySelectFactionGroup(ambushFactionKey, JLH_DCF_AmbientGroupRole.FIRETEAM_4, 4, 4, selection, reason))
			{
				plan = null;
				if (reason == "")
					reason = "convoy_ambush_us_group_selection_failed";

				return false;
			}

			if (!m_bConvoyAmbushAllowAT && IsLikelyATGroupPrefab(selection.Prefab))
			{
				plan = null;
				reason = "convoy_ambush_us_at_group_rejected";
				return false;
			}

			plan.AddSelection(selection);
		}

		plan.TargetUnits = plan.TotalUnits;
		LogConvoyAmbush("us_plan groups=" + plan.GroupCount().ToString() + " units=" + plan.TotalUnits.ToString() + " allowAT=" + JLH_DCF_NodeDebug.BoolLabel(m_bConvoyAmbushAllowAT));
		return !plan.IsEmpty();
	}

	protected bool TryBuildConvoyAmbushConfiguredGroupPlan(ResourceName prefab, string factionKey, JLH_DCF_AmbientGroupRole role, string source, int estimatedUnits, bool rejectAT, out JLH_AmbientSkirmishGroupPlan plan)
	{
		plan = null;
		if (prefab == "" || prefab == ResourceName.Empty)
			return false;

		if (!JLH_AddonSpawnUtility.CanLoadPrefab(prefab))
		{
			LogConvoyAmbush("group_prefab_override_invalid source=" + source + " prefab=" + prefab);
			return false;
		}

		if (rejectAT && !m_bConvoyAmbushAllowAT && IsLikelyATGroupPrefab(prefab))
		{
			LogConvoyAmbush("group_prefab_override_rejected_at source=" + source + " prefab=" + prefab);
			return false;
		}

		plan = new JLH_AmbientSkirmishGroupPlan();
		plan.Init(factionKey, role, estimatedUnits, source);
		AppendConvoyAmbushPlanSelection(plan, prefab, factionKey, source, role, estimatedUnits);
		return true;
	}

	protected void AppendConvoyAmbushPlanSelection(notnull JLH_AmbientSkirmishGroupPlan plan, ResourceName prefab, string factionKey, string source, JLH_DCF_AmbientGroupRole role, int estimatedUnits)
	{
		ref JLH_DCF_EnemyGroupSelection selection = new JLH_DCF_EnemyGroupSelection();
		selection.Prefab = prefab;
		selection.EnemyFactionKey = factionKey;
		selection.Source = source;
		selection.Role = JLH_DCF_AmbientGroupRoleResolver.ToLegacyRole(role);
		selection.EstimatedUnits = estimatedUnits;
		plan.AddLegacySelection(selection);
	}

	protected bool SpawnConvoyAmbushPlanAtPositions(notnull JLH_AmbientSkirmishGroupPlan plan, notnull array<vector> positions, string sideLabel, notnull array<SCR_AIGroup> outGroups, out string reason)
	{
		outGroups.Clear();
		reason = "";

		if (!plan || plan.IsEmpty())
		{
			reason = sideLabel + "_plan_empty";
			return false;
		}

		for (int i = 0; i < plan.Groups.Count(); i++)
		{
			JLH_DCF_EnemyGroupSelection selection = plan.Groups[i];
			if (!selection || selection.Prefab == "")
			{
				CleanupSpawnedGroups(outGroups, "convoy_ambush_" + sideLabel + "_partial_cleanup_invalid_selection");
				reason = sideLabel + "_selection_invalid";
				return false;
			}

			vector position = "0 0 0";
			if (i < positions.Count())
				position = positions[i];
			else if (!positions.IsEmpty())
				position = positions[positions.Count() - 1];

			SCR_AIGroup group = JLH_AddonSpawnUtility.SpawnGroup(selection.Prefab, position, "convoy_ambush_" + sideLabel + "_group_spawn_" + i.ToString(), selection.EstimatedUnits);
			if (!group)
			{
				CleanupSpawnedGroups(outGroups, "convoy_ambush_" + sideLabel + "_partial_cleanup_spawn_failed");
				reason = sideLabel + "_spawn_failed";
				return false;
			}

			outGroups.Insert(group);
			if (sideLabel == "ussr_ambush" || sideLabel == "us_ambush")
				LogConvoyAmbush("spawn_ambush_group faction=" + selection.EnemyFactionKey + " side=" + ConvoyAmbushGroupSideLabel(i) + " count=" + JLH_AddonSpawnUtility.CountAliveGroupUnits(group).ToString() + " prefab=" + selection.Prefab + " pos=" + position.ToString());
		}

		return !outGroups.IsEmpty();
	}

	protected void BuildConvoyAmbushEnemyPositions(vector roadPos, vector roadDir, vector roadRight, int groupCount, notnull array<vector> positions)
	{
		positions.Clear();
		positions.Insert(ResolveConvoyAmbushGroupSpawnPosition(roadPos + (roadDir * 95.0) + (roadRight * 55.0), roadPos, "front"));
		positions.Insert(ResolveConvoyAmbushGroupSpawnPosition(roadPos - (roadDir * 95.0) - (roadRight * 55.0), roadPos, "rear"));

		if (groupCount > 2)
			positions.Insert(ResolveConvoyAmbushGroupSpawnPosition(roadPos - (roadRight * 70.0), roadPos, "flank"));

		for (int i = 3; i < groupCount; i++)
		{
			float side = 1.0;
			if ((i % 2) == 0)
				side = -1.0;

			positions.Insert(ResolveConvoyAmbushGroupSpawnPosition(roadPos + (roadRight * side * (66.0 + (i * 6.0))), roadPos, "extra_" + i.ToString()));
		}
	}

	protected vector ResolveConvoyAmbushGroupSpawnPosition(vector desired, vector center, string label)
	{
		string reason;
		vector grounded = JLH_AddonWorldQuery.GroundPosition(desired);
		if (IsGroupPositionValid(grounded, reason) && !JLH_AddonWorldQuery.IsWaterOrNearWater(grounded, 5.0))
			return grounded;

		for (int i = 0; i < 10; i++)
		{
			vector candidate = JLH_AddonWorldQuery.GroundPosition(desired + JLH_AddonWorldQuery.RandomOffset(8.0, 28.0));
			if (IsGroupPositionValid(candidate, reason) && !JLH_AddonWorldQuery.IsWaterOrNearWater(candidate, 5.0))
				return candidate;
		}

		LogConvoyAmbush("group_position_fallback label=" + label + " reason=" + reason + " desired=" + desired.ToString());
		return grounded;
	}

	protected string ConvoyAmbushGroupSideLabel(int groupIndex)
	{
		if (groupIndex == 0)
			return "front";
		if (groupIndex == 1)
			return "rear";
		if (groupIndex == 2)
			return "flank";

		return "extra_" + groupIndex.ToString();
	}

	protected int CountConvoyAmbushGroupsUnits(notnull array<SCR_AIGroup> groups)
	{
		int count = 0;
		foreach (SCR_AIGroup group : groups)
		{
			if (group)
				count += JLH_AddonSpawnUtility.CountAliveGroupUnits(group);
		}

		return count;
	}

	protected bool IsLikelyATGroupPrefab(ResourceName prefab)
	{
		string token = prefab;
		token.ToLower();
		return token.Contains("team_at") || token.Contains("_at") || token.Contains("/at") || token.Contains("lat") || token.Contains("rpg");
	}

	protected bool IsUSSRLikeConvoyFaction(string factionKey)
	{
		factionKey.ToUpper();
		return factionKey == "USSR" || factionKey == "OPFOR";
	}

	protected void LogConvoyAmbush(string details)
	{
		JLH_AddonDebug.Event(CONVOY_AMBUSH_LOG_PREFIX, details, true);
	}

	protected void PrepareConvoyAmbushUSTroops(notnull JLH_AmbientSkirmishRecord record, SCR_AIGroup group, IEntity truck, vector truckPos)
	{
		if (!group)
			return;

		vector survivorCenter = truckPos;
		if (truck)
			survivorCenter = JLH_AddonWorldQuery.GroundPosition(truck.GetOrigin());

		LayoutConvoyAmbushUSTroopsAroundTruck(group, survivorCenter, "spawned_survivors");
		AssignConvoyAmbushUSTroopDefend(record, group, survivorCenter, 0);
		LogConvoyAmbush("convoy_troops_on_foot count=" + JLH_AddonSpawnUtility.CountAliveGroupUnits(group).ToString() + " mode=spawned_near_truck");
	}

	protected void PrepareConvoyAmbushVehicleCasualties(notnull JLH_AmbientSkirmishRecord record, SCR_AIGroup group, IEntity vehicle, string role)
	{
		if (!group || !vehicle)
			return;

		vector vehiclePos = JLH_AddonWorldQuery.GroundPosition(vehicle.GetOrigin());
		int moved = JLH_AddonSpawnUtility.ScatterGroupUnitsAround(group, vehiclePos, 99, 2.5, 6.5, 3.0, 8.0, "convoy_ambush_vehicle_casualties_" + role);
		JLH_AddonSpawnUtility.AssignDefend(group, vehiclePos, record.Runtime, "convoy_ambush_vehicle_casualties_hold_" + role);
		LogConvoyAmbush("convoy_vehicle_casualties_near_vehicle role=" + role + " count=" + JLH_AddonSpawnUtility.CountAliveGroupUnits(group).ToString() + " moved=" + moved.ToString() + " target=4 pos=" + vehiclePos.ToString());
	}

	protected int MountConvoyAmbushUSTroopsIntoTruck(SCR_AIGroup group, IEntity truck)
	{
		return MountConvoyAmbushGroupIntoVehicle(group, truck, true);
	}

	protected int MountConvoyAmbushGroupIntoVehicle(SCR_AIGroup group, IEntity vehicle, bool cargoOnly)
	{
		if (!group || !vehicle)
			return 0;

		//group.DeactivateAllMembers();
		array<AIAgent> agents = {};
		group.GetAgents(agents);

		int mounted = 0;
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!unit || !JLH_AddonSpawnUtility.IsUnitAlive(unit))
				continue;

			if (IsSkirmishUnitInVehicle(unit, vehicle))
			{
				mounted++;
				continue;
			}

			SCR_CompartmentAccessComponent access = SCR_CompartmentAccessComponent.Cast(unit.FindComponent(SCR_CompartmentAccessComponent));
			if (!access)
				continue;

			unit.SetOrigin(vehicle.GetOrigin());
			bool moved = false;
			if (cargoOnly)
			{
				moved = access.MoveInVehicle(vehicle, ECompartmentType.CARGO);
			}
			else
			{
				if (access.MoveInVehicle(vehicle, ECompartmentType.PILOT))
					moved = true;
				else if (access.MoveInVehicle(vehicle, ECompartmentType.TURRET))
					moved = true;
				else if (access.MoveInVehicle(vehicle, ECompartmentType.CARGO))
					moved = true;
				else
					moved = access.MoveInVehicleAny(vehicle);
			}

			if (moved || IsSkirmishUnitInVehicle(unit, vehicle))
				mounted++;
		}

		return mounted;
	}

	protected void ScheduleConvoyAmbushUSTroopDismount(notnull JLH_AmbientSkirmishRecord record, SCR_AIGroup group, IEntity truck)
	{
		if (!GetGame() || !GetGame().GetCallqueue())
		{
			DismountConvoyAmbushUSTroops(record, group, truck);
			return;
		}

		record.ConvoyAmbushDismountScheduled = true;
		GetGame().GetCallqueue().CallLater(JLH_AmbientSkirmish_DismountConvoyAmbushUSTroops, CONVOY_AMBUSH_US_DISMOUNT_DELAY_MS, false, this, record, group, truck);
	}

	void DismountConvoyAmbushUSTroops(JLH_AmbientSkirmishRecord record, SCR_AIGroup group, IEntity truck)
	{
		if (!record || !group || !truck)
			return;

		if (!HasRuntimeRecord(record))
			return;

		int requested = RequestConvoyAmbushUSTroopExit(group, truck);
		vector truckPos = JLH_AddonWorldQuery.GroundPosition(truck.GetOrigin());
		EnsureConvoyAmbushTruckRunning(truck);
		LayoutConvoyAmbushUSTroopsAroundTruck(group, truckPos, "dismount");
		AssignConvoyAmbushUSTroopDefend(record, group, truckPos, 0);
		record.ConvoyAmbushDismountScheduled = false;
		record.Runtime.RefreshExclusions("convoy_ambush_us_troop_dismount_refresh");
		LogConvoyAmbush("us_troops_dismount requested=" + requested.ToString() + " defend=" + truckPos.ToString());
	}

	protected int RequestConvoyAmbushUSTroopExit(SCR_AIGroup group, IEntity truck)
	{
		if (!group || !truck)
			return 0;

		int requested = 0;
		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!unit || !JLH_AddonSpawnUtility.IsUnitAlive(unit))
				continue;

			if (TryEjectSkirmishUnitFromVehicle(unit, truck, "convoy_ambush_us_troop_dismount"))
				requested++;
		}

		return requested;
	}

	protected void LayoutConvoyAmbushUSTroopsAroundTruck(SCR_AIGroup group, vector truckPos, string reason)
	{
		if (!group)
			return;

		int moved = JLH_AddonSpawnUtility.ScatterGroupUnitsAround(group, truckPos, 99, 6.0, 13.0, 14.0, 24.0, "convoy_ambush_us_troops_" + reason);
		LogConvoyAmbush("us_troops_layout mode=" + reason + " moved=" + moved.ToString() + " center=" + truckPos.ToString());
	}

	protected void AssignConvoyAmbushUSTroopDefend(notnull JLH_AmbientSkirmishRecord record, SCR_AIGroup group, vector truckPos, int groupIndex)
	{
		if (!group)
			return;

		int sideGroupCount = record.FriendlyGroups.Count();
		if (IsConvoyAmbushGroupInArray(record.EnemyGroups, group))
			sideGroupCount = record.EnemyGroups.Count();
		if (sideGroupCount < 1)
			sideGroupCount = 1;

		vector defendPos = BuildComposedGroupPosition(truckPos, groupIndex, sideGroupCount);
		JLH_AddonSpawnUtility.AssignDefend(group, defendPos, record.Runtime, "convoy_ambush_us_troops_defend_" + groupIndex.ToString());
	}

	protected bool IsConvoyAmbushGroupInArray(notnull array<SCR_AIGroup> groups, SCR_AIGroup group)
	{
		if (!group)
			return false;

		for (int i = 0; i < groups.Count(); i++)
		{
			if (group == groups[i])
				return true;
		}

		return false;
	}

	protected bool IsConvoyAmbushTruckGroup(notnull JLH_AmbientSkirmishRecord record, SCR_AIGroup group)
	{
		if (!group)
			return false;

		if (group == record.ConvoyAmbushUSGroup)
			return true;

		for (int i = 0; i < record.ConvoyAmbushTruckGroups.Count(); i++)
		{
			if (group == record.ConvoyAmbushTruckGroups[i])
				return true;
		}

		return false;
	}

	protected void AssignConvoyAmbushBehavior(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishEventDefinition definition, vector truckPos)
	{
		LogScenarioCombatProfile(record, definition);

		bool usAttacksUssr = definition.SkirmishEventType == JLH_AmbientSkirmishEventType.CONVOY_AMBUSH_US_ATTACKS_USSR;
		array<SCR_AIGroup> convoyGroups = record.FriendlyGroups;
		array<SCR_AIGroup> ambushGroups = record.EnemyGroups;
		if (usAttacksUssr)
		{
			convoyGroups = record.EnemyGroups;
			ambushGroups = record.FriendlyGroups;
		}

		for (int i = 0; i < convoyGroups.Count(); i++)
		{
			SCR_AIGroup convoyGroup = convoyGroups[i];
			if (!convoyGroup)
				continue;

			if (record.ConvoyAmbushDismountScheduled && IsConvoyAmbushTruckGroup(record, convoyGroup))
				continue;
			if (convoyGroup == record.ConvoyAmbushFrontCasualtyGroup || convoyGroup == record.ConvoyAmbushRearCasualtyGroup)
				continue;

			AssignConvoyAmbushUSTroopDefend(record, convoyGroup, truckPos, i);
		}

		for (int j = 0; j < ambushGroups.Count(); j++)
		{
			SCR_AIGroup ambushGroup = ambushGroups[j];
			if (!ambushGroup)
				continue;

			int ambushGroupCount = ambushGroups.Count();
			if (ambushGroupCount < 1)
				ambushGroupCount = 1;

			vector assaultTarget = BuildComposedGroupPosition(truckPos, j, ambushGroupCount);
			JLH_AddonSpawnUtility.AssignMoveAndSearch(ambushGroup, assaultTarget, 45.0, 3, record.Runtime, "convoy_ambush_assault_" + j.ToString());
		}

		record.Runtime.RefreshExclusions("convoy_ambush_behavior_refresh");
		LogConvoyAmbush("assault_orders target=truck pos=" + truckPos.ToString() + " ambushGroups=" + ambushGroups.Count().ToString());
	}

	protected void ScheduleConvoyAmbushStagedDestruction(notnull JLH_AmbientSkirmishRecord record, IEntity frontVehicle, IEntity rearVehicle)
	{
		DamageConvoyAmbushVehicle(record, frontVehicle, "front");

		if (!rearVehicle || !GetGame() || !GetGame().GetCallqueue())
		{
			DamageConvoyAmbushVehicle(record, rearVehicle, "rear");
			return;
		}

		record.ConvoyAmbushRearDestroyScheduled = true;
		int delayMs = Math.RandomInt(CONVOY_AMBUSH_REAR_DESTROY_MIN_DELAY_MS, CONVOY_AMBUSH_REAR_DESTROY_MAX_DELAY_MS + 1);
		GetGame().GetCallqueue().CallLater(JLH_AmbientSkirmish_ConvoyAmbushDestroyRearDelayed, delayMs, false, this, record, rearVehicle);
	}

	void ConvoyAmbushDestroyRearDelayed(JLH_AmbientSkirmishRecord record, IEntity rearVehicle)
	{
		if (!record || !rearVehicle)
			return;

		if (!HasRuntimeRecord(record))
			return;

		record.ConvoyAmbushRearDestroyScheduled = false;
		DamageConvoyAmbushVehicle(record, rearVehicle, "rear");
		record.Runtime.RefreshExclusions("convoy_ambush_rear_destroy_refresh");
	}

	protected bool DamageConvoyAmbushVehicle(notnull JLH_AmbientSkirmishRecord record, IEntity vehicle, string role)
	{
		if (!vehicle)
			return false;

		bool damaged = JLH_AddonSpawnUtility.DestroyRuntimeEntity(vehicle, "convoy_ambush_staged_destroy_" + role);
		DamageConvoyAmbushVehicleCasualties(record, role);
		LogConvoyAmbush("staged_destroy role=" + role + " result=" + JLH_DCF_NodeDebug.BoolLabel(damaged));
		return damaged;
	}

	protected void DamageConvoyAmbushVehicleCasualties(notnull JLH_AmbientSkirmishRecord record, string role)
	{
		SCR_AIGroup casualtyGroup = null;
		if (role == "front")
			casualtyGroup = record.ConvoyAmbushFrontCasualtyGroup;
		else if (role == "rear")
			casualtyGroup = record.ConvoyAmbushRearCasualtyGroup;

		if (!casualtyGroup)
			return;

		array<AIAgent> agents = {};
		casualtyGroup.GetAgents(agents);
		int killed = 0;
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!unit || !JLH_AddonSpawnUtility.IsUnitAlive(unit))
				continue;

			if (JLH_AddonSpawnUtility.DestroyRuntimeEntity(unit, "convoy_ambush_vehicle_casualty_" + role))
				killed++;
		}

		LogConvoyAmbush("convoy_vehicle_casualties_destroyed role=" + role + " count=" + killed.ToString());
	}

















	void ScheduleVehicleSkirmishMount(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishEventDefinition definition, int attempt)
	{
		if (!GetGame() || !GetGame().GetCallqueue())
		{
			FinalizeVehicleSkirmishMount(record, definition, attempt);
			return;
		}

		LogEvent("ambient_skirmish_vehicle_mount_queued", "id=" + record.Id.ToString() + " event=" + definition.SkirmishEventName + " attempt=" + attempt.ToString() + " delayMs=" + VEHICLE_MOUNT_RETRY_DELAY_MS.ToString(), true);
		GetGame().GetCallqueue().CallLater(JLH_AmbientSkirmish_FinalizeVehicleSkirmishMount, VEHICLE_MOUNT_RETRY_DELAY_MS, false, this, record, definition, attempt);
	}

	void FinalizeVehicleSkirmishMount(JLH_AmbientSkirmishRecord record, JLH_AmbientSkirmishEventDefinition definition, int attempt)
	{
		if (!record || !definition)
			return;

		if (!HasRuntimeRecord(record))
			return;

		string reason;
		if (ShouldSpawnFriendlyVehicle(definition) && !record.FriendlyVehicleMounted)
		{
			if (record.FriendlyVehicles.IsEmpty())
			{
				CleanupRuntimeRecord(record, "ambient_skirmish_friendly_vehicle_mount_missing_vehicle", true);
				return;
			}

			if (!MountSkirmishVehicleSide(record, record.FriendlyGroups, record.FriendlyVehicles, record.FriendlyVehiclePrefabs, definition.FriendlyVehiclePrefab, definition, "friendly", reason))
			{
				if (ShouldRetryVehicleMount(reason, attempt))
				{
					ScheduleVehicleSkirmishMount(record, definition, attempt + 1);
					return;
				}

				LogEvent("ambient_skirmish_vehicle_mount_failed", "id=" + record.Id.ToString() + " side=friendly reason=" + reason + " attempt=" + attempt.ToString(), true);
				AbortVehicleSkirmish(record, "friendly", -1, reason, null);
				return;
			}

			record.FriendlyVehicleMounted = true;
		}

		if (ShouldSpawnEnemyVehicle(definition) && !record.EnemyVehicleMounted)
		{
			if (record.EnemyVehicles.IsEmpty())
			{
				CleanupRuntimeRecord(record, "ambient_skirmish_enemy_vehicle_mount_missing_vehicle", true);
				return;
			}

			if (!MountSkirmishVehicleSide(record, record.EnemyGroups, record.EnemyVehicles, record.EnemyVehiclePrefabs, definition.EnemyVehiclePrefab, definition, "enemy", reason))
			{
				if (ShouldRetryVehicleMount(reason, attempt))
				{
					ScheduleVehicleSkirmishMount(record, definition, attempt + 1);
					return;
				}

				LogEvent("ambient_skirmish_vehicle_mount_failed", "id=" + record.Id.ToString() + " side=enemy reason=" + reason + " attempt=" + attempt.ToString(), true);
				AbortVehicleSkirmish(record, "enemy", -1, reason, null);
				return;
			}

			record.EnemyVehicleMounted = true;
		}

		record.Runtime.RefreshExclusions("ambient_skirmish_vehicle_mount_refresh");
		if (!record.VehicleBehaviorAssigned && ShouldAssignVehicleBehaviorAfterMountValidation(definition))
		{
			AssignVehicleSkirmishBehavior(record, definition);
			record.VehicleBehaviorAssigned = true;
		}

		if (ShouldSkipLiveVehicleSkirmishMountValidation(definition))
		{
			LogEvent("vehicle_skirmish_mount_validation_skipped", "id=" + record.Id.ToString() + " event=" + definition.SkirmishEventName + " attempt=" + attempt.ToString() + " friendlyCommandsIssued=" + JLH_DCF_NodeDebug.BoolLabel(record.FriendlyVehicleMounted) + " enemyCommandsIssued=" + JLH_DCF_NodeDebug.BoolLabel(record.EnemyVehicleMounted) + " behaviorAssigned=" + JLH_DCF_NodeDebug.BoolLabel(record.VehicleBehaviorAssigned) + " reason=live_vehicle_duel_allows_vanilla_ai_dismounts", true);
			return;
		}

		LogEvent("ambient_skirmish_vehicle_mount_state_pending", "id=" + record.Id.ToString() + " event=" + definition.SkirmishEventName + " attempt=" + attempt.ToString() + " friendlyCommandsIssued=" + JLH_DCF_NodeDebug.BoolLabel(record.FriendlyVehicleMounted) + " enemyCommandsIssued=" + JLH_DCF_NodeDebug.BoolLabel(record.EnemyVehicleMounted) + " validationDelayMs=" + VEHICLE_MOUNT_VALIDATION_DELAY_MS.ToString(), true);
		ScheduleVehicleSkirmishPostMountValidation(record, definition, 0);
	}

	protected bool ShouldSkipLiveVehicleSkirmishMountValidation(notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		return definition.SkirmishEventType == JLH_AmbientSkirmishEventType.VEHICLE_SKIRMISH;
	}

	protected void ScheduleVehicleSkirmishPostMountValidation(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishEventDefinition definition, int attempt)
	{
		if (!GetGame() || !GetGame().GetCallqueue())
			return;

		GetGame().GetCallqueue().CallLater(JLH_AmbientSkirmish_ValidateVehicleSkirmishMount, VEHICLE_MOUNT_VALIDATION_DELAY_MS, false, this, record, definition, attempt);
	}

	protected void ScheduleVehicleSkirmishPostRouteValidation(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishEventDefinition definition, int attempt)
	{
		if (!GetGame() || !GetGame().GetCallqueue())
			return;

		if (record.VehiclePostRouteValidationScheduled)
			return;

		record.VehiclePostRouteValidationScheduled = true;
		GetGame().GetCallqueue().CallLater(JLH_AmbientSkirmish_ValidateVehicleSkirmishMount, VEHICLE_POST_ROUTE_VALIDATION_DELAY_MS, false, this, record, definition, attempt);
	}

	void ValidateVehicleSkirmishMount(JLH_AmbientSkirmishRecord record, JLH_AmbientSkirmishEventDefinition definition, int attempt)
	{
		if (!record || !definition)
			return;

		if (!HasRuntimeRecord(record))
			return;

		bool valid = true;
		bool recovered = false;
		bool deferred = false;
		string validationFailureReason = "";
		if (record.FriendlyVehicleMounted)
		{
			string friendlyReason;
			int friendlyFailedIndex;
			if (!ValidateVehicleSkirmishMountSide(record, record.FriendlyGroups, record.FriendlyVehicles, record.FriendlyVehiclePrefabs, definition.FriendlyVehiclePrefab, definition, "friendly", true, friendlyReason, friendlyFailedIndex))
			{
				if (!IgnoreVehicleSkirmishRevalidationFailure(record, definition, true, friendlyFailedIndex, "friendly", friendlyReason))
				{
					if (validationFailureReason == "")
						validationFailureReason = friendlyReason;
					if (IsDeferredVehicleValidationReason(friendlyReason))
					{
						valid = false;
						deferred = true;
					}
					else
					{
						valid = false;
						if (!HandleVehicleMountValidationFailure(record, record.FriendlyGroups, record.FriendlyVehicles, record.FriendlyVehiclePrefabs, definition.FriendlyVehiclePrefab, definition, "friendly", true, friendlyFailedIndex, friendlyReason, recovered, deferred))
							return;
					}
				}
			}
		}

		if (record.EnemyVehicleMounted)
		{
			string enemyReason;
			int enemyFailedIndex;
			if (!ValidateVehicleSkirmishMountSide(record, record.EnemyGroups, record.EnemyVehicles, record.EnemyVehiclePrefabs, definition.EnemyVehiclePrefab, definition, "enemy", false, enemyReason, enemyFailedIndex))
			{
				if (!IgnoreVehicleSkirmishRevalidationFailure(record, definition, false, enemyFailedIndex, "enemy", enemyReason))
				{
					if (validationFailureReason == "")
						validationFailureReason = enemyReason;
					if (IsDeferredVehicleValidationReason(enemyReason))
					{
						valid = false;
						deferred = true;
					}
					else
					{
						valid = false;
						if (!HandleVehicleMountValidationFailure(record, record.EnemyGroups, record.EnemyVehicles, record.EnemyVehiclePrefabs, definition.EnemyVehiclePrefab, definition, "enemy", false, enemyFailedIndex, enemyReason, recovered, deferred))
							return;
					}
				}
			}
		}

		if (valid)
		{
			LogEvent("ambient_skirmish_vehicle_mount_validated", "id=" + record.Id.ToString() + " event=" + definition.SkirmishEventName + " attempt=" + attempt.ToString(), true);
			if (!record.VehicleBehaviorAssigned && ShouldAssignVehicleBehaviorAfterMountValidation(definition))
			{
				LogEvent("ambient_skirmish_vehicle_mount_finalized", "id=" + record.Id.ToString() + " event=" + definition.SkirmishEventName + " attempt=" + attempt.ToString() + " friendlyMounted=" + JLH_DCF_NodeDebug.BoolLabel(record.FriendlyVehicleMounted) + " enemyMounted=" + JLH_DCF_NodeDebug.BoolLabel(record.EnemyVehicleMounted), true);
				AssignVehicleSkirmishBehavior(record, definition);
				record.VehicleBehaviorAssigned = true;
				ScheduleVehicleSkirmishPostRouteValidation(record, definition, 0);
				return;
			}

			if (record.VehiclePostRouteValidationScheduled && !record.VehiclePostRouteValidationComplete)
			{
				record.VehiclePostRouteValidationComplete = true;
				LogEvent("ambient_skirmish_vehicle_post_route_mount_validated", "id=" + record.Id.ToString() + " event=" + definition.SkirmishEventName + " attempt=" + attempt.ToString(), true);
			}

			return;
		}

		record.Runtime.RefreshExclusions("ambient_skirmish_vehicle_mount_validation_refresh");
		if (deferred)
		{
			ScheduleVehicleSkirmishPostMountValidation(record, definition, attempt);
			return;
		}

		if (recovered && attempt + 1 < VEHICLE_MOUNT_VALIDATION_MAX_ATTEMPTS)
		{
			ScheduleVehicleSkirmishPostMountValidation(record, definition, attempt + 1);
			return;
		}

		if (validationFailureReason == "")
			validationFailureReason = "vehicle_mount_validation_failed";

		AbortVehicleSkirmish(record, "unknown", -1, validationFailureReason, null);
	}

	protected bool IgnoreVehicleSkirmishRevalidationFailure(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishEventDefinition definition, bool friendlySide, int vehicleIndex, string sideLabel, string validationReason)
	{
		if (definition.SkirmishEventType != JLH_AmbientSkirmishEventType.VEHICLE_SKIRMISH)
			return false;

		if (!validationReason.Contains("_vehicle_validation_") && !validationReason.Contains("_vehicle_mount_"))
			return false;

		JLH_AmbientSkirmishVehicleLifecycle lifecycle = null;
		if (vehicleIndex >= 0)
			lifecycle = record.GetVehicleLifecycle(friendlySide, vehicleIndex);

		bool liveVehicleSkirmish = record.VehicleBehaviorAssigned;
		if (lifecycle && (lifecycle.MountValidated || lifecycle.RouteAssignedAfterStableMount))
			liveVehicleSkirmish = true;

		if (!liveVehicleSkirmish)
			return false;

		bool routeAssigned = false;
		bool mountValidated = false;
		string state = "NO_LIFECYCLE";
		if (lifecycle)
		{
			routeAssigned = lifecycle.RouteAssignedAfterStableMount;
			mountValidated = lifecycle.MountValidated;
			if (lifecycle.RouteAssignedAfterStableMount)
				lifecycle.State = JLH_AmbientSkirmishVehicleMountState.ROUTE_ASSIGNED;
			else
				lifecycle.State = JLH_AmbientSkirmishVehicleMountState.ROUTE_ASSIGN_ALLOWED;

			lifecycle.FailedReason = "";
			lifecycle.ValidationFailureCount = 0;
			state = VehicleMountStateLabel(lifecycle.State);
		}

		string details = "id=" + record.Id.ToString();
		details = details + " side=" + sideLabel;
		details = details + " vehicleIndex=" + vehicleIndex.ToString();
		details = details + " reason=" + validationReason;
		details = details + " mountValidated=" + JLH_DCF_NodeDebug.BoolLabel(mountValidated);
		details = details + " routeAssigned=" + JLH_DCF_NodeDebug.BoolLabel(routeAssigned);
		details = details + " behaviorAssigned=" + JLH_DCF_NodeDebug.BoolLabel(record.VehicleBehaviorAssigned);
		details = details + " state=" + state;
		LogEvent("vehicle_skirmish_revalidation_ignored", details, true);
		return true;
	}

	protected bool HandleVehicleMountValidationFailure(notnull JLH_AmbientSkirmishRecord record, notnull array<SCR_AIGroup> groups, notnull array<IEntity> vehicles, array<ResourceName> vehiclePrefabs, ResourceName fallbackVehiclePrefab, notnull JLH_AmbientSkirmishEventDefinition definition, string sideLabel, bool friendlySide, int vehicleIndex, string validationReason, inout bool recovered, inout bool deferred)
	{
		JLH_AmbientSkirmishVehicleLifecycle lifecycle = record.GetVehicleLifecycle(friendlySide, vehicleIndex);
		if (!lifecycle)
		{
			AbortVehicleSkirmish(record, sideLabel, vehicleIndex, validationReason, lifecycle);
			return false;
		}

		lifecycle.ValidationFailureCount++;
		lifecycle.FailedReason = validationReason;

		if (lifecycle.ValidationFailureCount >= VEHICLE_VALIDATION_MAX_FAILURES)
		{
			AbortVehicleSkirmish(record, sideLabel, vehicleIndex, "max_mount_recovery_failures", lifecycle);
			return false;
		}

		if (lifecycle.MountAttemptCount >= VEHICLE_MOUNT_MAX_ATTEMPTS)
		{
			AbortVehicleSkirmish(record, sideLabel, vehicleIndex, "max_mount_attempts", lifecycle);
			return false;
		}

		if (IsRecoverableVehicleValidationReason(validationReason) && lifecycle.ValidationFailureCount == 1)
		{
			if (lifecycle.TemporaryHoldAssigned || lifecycle.RouteAssignedAfterStableMount)
				lifecycle.State = JLH_AmbientSkirmishVehicleMountState.VEHICLE_OCCUPANCY_STABILIZING;
			else
				lifecycle.State = JLH_AmbientSkirmishVehicleMountState.WAITING_FOR_OCCUPANCY_CONFIRM;
			LogVehicleValidationDeferred(record, sideLabel, vehicleIndex, lifecycle, validationReason, "first_failure_grace");
			deferred = true;
			return true;
		}

		int now = System.GetTickCount();
		if (lifecycle.LastRecoveryTime > 0 && now - lifecycle.LastRecoveryTime < VEHICLE_MOUNT_RECOVERY_DELAY_MS)
		{
			LogVehicleValidationDeferred(record, sideLabel, vehicleIndex, lifecycle, validationReason, "recovery_cooldown");
			deferred = true;
			return true;
		}

		string recoveryReason;
		if (RecoverVehicleSkirmishMountVehicle(record, groups, vehicles, vehiclePrefabs, fallbackVehiclePrefab, definition, sideLabel, friendlySide, vehicleIndex, validationReason, recoveryReason))
		{
			recovered = true;
			return true;
		}

		if (IsDeferredVehicleValidationReason(recoveryReason))
		{
			deferred = true;
			return true;
		}

		if (recoveryReason == "")
			recoveryReason = validationReason;

		AbortVehicleSkirmish(record, sideLabel, vehicleIndex, recoveryReason, lifecycle);
		return false;
	}

	protected bool RecoverVehicleSkirmishMountVehicle(notnull JLH_AmbientSkirmishRecord record, notnull array<SCR_AIGroup> groups, notnull array<IEntity> vehicles, array<ResourceName> vehiclePrefabs, ResourceName fallbackVehiclePrefab, notnull JLH_AmbientSkirmishEventDefinition definition, string sideLabel, bool friendlySide, int vehicleIndex, string validationReason, out string reason)
	{
		reason = "";
		int vehicleCount = groups.Count();
		if (vehicles.Count() < vehicleCount)
			vehicleCount = vehicles.Count();
		if (vehicleIndex < 0 || vehicleIndex >= vehicleCount)
		{
			reason = sideLabel + "_vehicle_recovery_missing_vehicle_index_" + vehicleIndex.ToString();
			return false;
		}

		if (!BeginVehicleMountAttempt(record, friendlySide, vehicleIndex, sideLabel, "recovery", reason))
			return false;

		JLH_AmbientSkirmishVehicleLifecycle lifecycle = record.GetVehicleLifecycle(friendlySide, vehicleIndex);
		if (lifecycle)
			lifecycle.LastRecoveryTime = System.GetTickCount();

		SCR_AIGroup group = groups[vehicleIndex];
		IEntity vehicle = vehicles[vehicleIndex];
		ResourceName vehiclePrefab = ResolveRuntimeVehiclePrefab(vehiclePrefabs, vehicleIndex, fallbackVehiclePrefab);
		LogEvent("ambient_skirmish_vehicle_mount_recover", "side=" + sideLabel + " vehicleIndex=" + vehicleIndex.ToString() + " reason=" + validationReason + " attempts=" + GetVehicleMountAttemptCount(lifecycle).ToString() + " validationFailures=" + GetVehicleValidationFailureCount(lifecycle).ToString(), true);
		if (MountSkirmishVehicleCrewGroup(record, group, vehicle, vehiclePrefab, definition, sideLabel, vehicleIndex, reason))
		{
			AssignVehicleStabilizationHold(record, group, vehicle, sideLabel, friendlySide, vehicleIndex, "recovery");
			return true;
		}

		if (lifecycle)
		{
			lifecycle.State = JLH_AmbientSkirmishVehicleMountState.FAILED_RECOVERY;
			lifecycle.FailedReason = reason;
		}

		LogEvent("ambient_skirmish_vehicle_mount_recover_failed", "side=" + sideLabel + " vehicleIndex=" + vehicleIndex.ToString() + " reason=" + reason, true);
		return false;
	}

	protected bool ValidateVehicleSkirmishMountSide(notnull JLH_AmbientSkirmishRecord record, notnull array<SCR_AIGroup> groups, notnull array<IEntity> vehicles, array<ResourceName> vehiclePrefabs, ResourceName fallbackVehiclePrefab, notnull JLH_AmbientSkirmishEventDefinition definition, string sideLabel, bool friendlySide, out string reason, out int failedVehicleIndex)
	{
		reason = "";
		failedVehicleIndex = -1;
		int vehicleCount = groups.Count();
		if (vehicles.Count() < vehicleCount)
			vehicleCount = vehicles.Count();
		if (vehicleCount <= 0)
		{
			reason = sideLabel + "_vehicle_validation_missing_group_or_vehicle";
			return false;
		}

		for (int i = 0; i < vehicleCount; i++)
		{
			ResourceName vehiclePrefab = ResolveRuntimeVehiclePrefab(vehiclePrefabs, i, fallbackVehiclePrefab);
			if (!BeginVehicleMountValidation(record, friendlySide, i, sideLabel, vehiclePrefab, reason))
			{
				failedVehicleIndex = i;
				return false;
			}

			JLH_AmbientSkirmishVehicleLifecycle lifecycle = record.GetVehicleLifecycle(friendlySide, i);
			if (!ValidateVehicleSkirmishCrewGroup(lifecycle, groups[i], vehicles[i], vehiclePrefab, definition, sideLabel, i, reason))
			{
				failedVehicleIndex = i;
				return false;
			}

			MarkVehicleMountStable(record, friendlySide, i, sideLabel, vehiclePrefab);
		}

		return true;
	}

	protected bool ValidateVehicleSkirmishCrewGroup(JLH_AmbientSkirmishVehicleLifecycle lifecycle, SCR_AIGroup group, IEntity vehicle, ResourceName vehiclePrefab, notnull JLH_AmbientSkirmishEventDefinition definition, string sideLabel, int vehicleIndex, out string reason)
	{
		reason = "";
		if (!group || !vehicle)
		{
			reason = sideLabel + "_vehicle_validation_missing_group_or_vehicle_index_" + vehicleIndex.ToString();
			return false;
		}

		IEntity mountTarget = vehicle;
		bool sharedCombatMount = ShouldUseSharedCombatVehicleMount(definition);
		bool btrStrictTurret = IsBtrVehiclePrefab(vehiclePrefab);
		if (!sharedCombatMount && !btrStrictTurret)
		{
			mountTarget = ResolveVehicleMountTarget(vehicle);
			if (!mountTarget)
				mountTarget = vehicle;
		}

		LogSkirmishVehicleRuntimeTargetAudit(vehiclePrefab, sideLabel, vehicleIndex, vehicle, mountTarget, "validation");

		bool driverMounted = false;
		bool gunnerMounted = false;
		int mountedCrew = 0;
		int driverCompartmentCount = 0;
		int turretCompartmentCount = 0;
		string driverCompartment = "NONE";
		string gunnerCompartment = "NONE";
		IEntity validationDriverEntity = null;
		IEntity validationGunnerEntity = null;
		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!unit || !JLH_AddonSpawnUtility.IsUnitAlive(unit))
				continue;

			if (!IsSkirmishUnitInVehicle(unit, mountTarget))
				continue;

			mountedCrew++;
			if (!driverMounted && IsSkirmishUnitInCompartmentType(unit, mountTarget, ECompartmentType.PILOT))
			{
				driverMounted = true;
				driverCompartment = GetSkirmishUnitCompartmentLabel(unit, mountTarget);
				driverCompartmentCount++;
				validationDriverEntity = unit;
			}

			bool unitGunner = false;
			if (btrStrictTurret)
				unitGunner = IsBtrVehiclePatrolGunnerRole(unit, mountTarget);
			else if (RequiresStrictVehicleSkirmishGunner(definition))
				unitGunner = IsCombatSkirmishUnitInGunnerRole(unit, mountTarget);
			else if (sharedCombatMount)
				unitGunner = IsCombatSkirmishUnitInGunnerRole(unit, mountTarget);
			else
				unitGunner = IsSkirmishUnitInGunnerRole(unit, mountTarget);

			if (RequiresStrictVehicleSkirmishGunner(definition) && unit == validationDriverEntity)
				unitGunner = false;

			if (btrStrictTurret)
				LogBtrPostMountSeatValidation(vehiclePrefab, sideLabel, vehicleIndex, unit, mountTarget, unitGunner, "validation");

			if (!gunnerMounted && unitGunner)
			{
				gunnerMounted = true;
				validationGunnerEntity = unit;
				gunnerCompartment = GetSkirmishUnitCompartmentLabel(unit, mountTarget);
				turretCompartmentCount++;
			}
		}

		int requiredMountedCrew = GetRequiredVehicleMountedCrewForValidation(definition);
		bool attackCapable = driverMounted && (!RequiresVehicleGunner(definition) || gunnerMounted) && mountedCrew >= requiredMountedCrew;
		LogSkirmishVehicleCompartmentValidation(vehiclePrefab, sideLabel, vehicleIndex, driverMounted, gunnerMounted, driverCompartmentCount, turretCompartmentCount, mountedCrew, requiredMountedCrew, attackCapable, driverCompartment, gunnerCompartment);
		if (btrStrictTurret && attackCapable)
			LogBtrTurretMountSuccess(vehiclePrefab, sideLabel, vehicleIndex, "validation_confirmed", gunnerCompartment, validationGunnerEntity, driverMounted, gunnerMounted, mountedCrew, attackCapable);

		if (ShouldDeferVehicleOccupancyValidation(lifecycle, vehiclePrefab, sideLabel, vehicleIndex, driverMounted, gunnerMounted, mountedCrew, requiredMountedCrew, driverCompartment, gunnerCompartment))
		{
			reason = sideLabel + "_vehicle_validation_occupancy_pending_index_" + vehicleIndex.ToString();
			return false;
		}

		if (!driverMounted)
		{
			reason = sideLabel + "_vehicle_validation_no_driver_index_" + vehicleIndex.ToString();
			return false;
		}

		if (RequiresVehicleGunner(definition) && !gunnerMounted)
		{
			reason = sideLabel + "_vehicle_validation_no_gunner_index_" + vehicleIndex.ToString();
			if (btrStrictTurret)
				LogBtrInvalidNoGunner(vehiclePrefab, sideLabel, vehicleIndex, mountedCrew, driverMounted, driverCompartment);

			return false;
		}

		if (mountedCrew < requiredMountedCrew)
		{
			reason = sideLabel + "_vehicle_validation_not_filled_index_" + vehicleIndex.ToString();
			return false;
		}

		return true;
	}

	protected bool BeginVehicleMountAttempt(notnull JLH_AmbientSkirmishRecord record, bool friendlySide, int vehicleIndex, string sideLabel, string phase, out string reason)
	{
		reason = "";
		JLH_AmbientSkirmishVehicleLifecycle lifecycle = record.GetVehicleLifecycle(friendlySide, vehicleIndex);
		if (!lifecycle)
		{
			reason = sideLabel + "_vehicle_mount_missing_lifecycle_index_" + vehicleIndex.ToString();
			return false;
		}

		if (lifecycle.RecoveryLocked || lifecycle.State == JLH_AmbientSkirmishVehicleMountState.ABORTED)
		{
			reason = sideLabel + "_vehicle_recovery_locked_index_" + vehicleIndex.ToString();
			return false;
		}

		if (lifecycle.MountAttemptCount >= VEHICLE_MOUNT_MAX_ATTEMPTS)
		{
			reason = sideLabel + "_vehicle_max_mount_attempts_index_" + vehicleIndex.ToString();
			return false;
		}

		int now = System.GetTickCount();
		if (lifecycle.LastMountAttemptTime > 0 && now - lifecycle.LastMountAttemptTime < VEHICLE_MOUNT_RECOVERY_DELAY_MS)
		{
			reason = sideLabel + "_vehicle_mount_cooldown_index_" + vehicleIndex.ToString();
			LogVehicleValidationDeferred(record, sideLabel, vehicleIndex, lifecycle, reason, phase);
			return false;
		}

		lifecycle.MountAttemptCount++;
		lifecycle.LastMountAttemptTime = now;
		lifecycle.State = JLH_AmbientSkirmishVehicleMountState.MOUNTING;
		lifecycle.TemporaryHoldAssigned = false;
		lifecycle.MountValidated = false;
		lifecycle.RoutePendingLogged = false;
		lifecycle.IdleStateLogged = false;
		lifecycle.DismountPreventedLogged = false;

		string details = "id=" + record.Id.ToString();
		details = details + " phase=" + phase;
		details = details + " side=" + sideLabel;
		details = details + " vehicleIndex=" + vehicleIndex.ToString();
		details = details + " attempts=" + lifecycle.MountAttemptCount.ToString();
		details = details + " validationFailures=" + lifecycle.ValidationFailureCount.ToString();
		details = details + " state=" + VehicleMountStateLabel(lifecycle.State);
		LogEvent("ambient_skirmish_vehicle_mount_command_issued", details, true);
		return true;
	}

	protected bool BeginVehicleMountValidation(notnull JLH_AmbientSkirmishRecord record, bool friendlySide, int vehicleIndex, string sideLabel, ResourceName vehiclePrefab, out string reason)
	{
		reason = "";
		JLH_AmbientSkirmishVehicleLifecycle lifecycle = record.GetVehicleLifecycle(friendlySide, vehicleIndex);
		if (!lifecycle)
		{
			reason = sideLabel + "_vehicle_validation_missing_lifecycle_index_" + vehicleIndex.ToString();
			return false;
		}

		if (lifecycle.RecoveryLocked || lifecycle.State == JLH_AmbientSkirmishVehicleMountState.ABORTED)
		{
			reason = sideLabel + "_vehicle_validation_recovery_locked_index_" + vehicleIndex.ToString();
			return false;
		}

		int now = System.GetTickCount();
		if (lifecycle.LastMountAttemptTime > 0 && now - lifecycle.LastMountAttemptTime < VEHICLE_MOUNT_VALIDATION_DELAY_MS)
		{
			if (lifecycle.TemporaryHoldAssigned || lifecycle.RouteAssignedAfterStableMount)
				lifecycle.State = JLH_AmbientSkirmishVehicleMountState.VEHICLE_OCCUPANCY_STABILIZING;
			else
				lifecycle.State = JLH_AmbientSkirmishVehicleMountState.WAITING_FOR_OCCUPANCY_CONFIRM;
			reason = sideLabel + "_vehicle_validation_mount_pending_index_" + vehicleIndex.ToString();
			LogVehicleValidationDeferred(record, sideLabel, vehicleIndex, lifecycle, reason, "mount_stabilizing");
			return false;
		}

		if (lifecycle.LastValidationTime > 0 && now - lifecycle.LastValidationTime < VEHICLE_MOUNT_VALIDATION_COOLDOWN_MS)
		{
			reason = sideLabel + "_vehicle_validation_cooldown_index_" + vehicleIndex.ToString();
			LogVehicleValidationDeferred(record, sideLabel, vehicleIndex, lifecycle, reason, "validation_cooldown");
			return false;
		}

		lifecycle.LastValidationTime = now;
		lifecycle.State = JLH_AmbientSkirmishVehicleMountState.VALIDATING_MOUNT;
		return true;
	}

	protected void MarkVehicleMountStable(notnull JLH_AmbientSkirmishRecord record, bool friendlySide, int vehicleIndex, string sideLabel, ResourceName vehiclePrefab)
	{
		JLH_AmbientSkirmishVehicleLifecycle lifecycle = record.GetVehicleLifecycle(friendlySide, vehicleIndex);
		if (!lifecycle)
			return;

		bool logStableTransition = !lifecycle.MountValidated || (lifecycle.State != JLH_AmbientSkirmishVehicleMountState.ROUTE_ASSIGN_ALLOWED && lifecycle.State != JLH_AmbientSkirmishVehicleMountState.ROUTE_ASSIGNED);
		lifecycle.State = JLH_AmbientSkirmishVehicleMountState.OCCUPANCY_STABLE;
		ClearVehicleOccupancyPending(lifecycle);
		if (logStableTransition)
			LogOccupancyStable(record, vehiclePrefab, sideLabel, vehicleIndex, lifecycle);

		lifecycle.State = JLH_AmbientSkirmishVehicleMountState.VALIDATED;
		lifecycle.MountValidated = true;
		if (logStableTransition)
			LogValidationConfirmed(record, vehiclePrefab, sideLabel, vehicleIndex, lifecycle);

		if (lifecycle.RouteAssignedAfterStableMount)
			lifecycle.State = JLH_AmbientSkirmishVehicleMountState.ROUTE_ASSIGNED;
		else
			lifecycle.State = JLH_AmbientSkirmishVehicleMountState.ROUTE_ASSIGN_ALLOWED;

		lifecycle.FailedReason = "";
		lifecycle.ValidationFailureCount = 0;

		string details = "id=" + record.Id.ToString();
		details = details + " side=" + sideLabel;
		details = details + " vehicleIndex=" + vehicleIndex.ToString();
		details = details + " prefab=" + vehiclePrefab;
		details = details + " state=" + VehicleMountStateLabel(lifecycle.State);
		details = details + " attempts=" + lifecycle.MountAttemptCount.ToString();
		details = details + " validationFailures=" + lifecycle.ValidationFailureCount.ToString();
		LogEvent("ambient_skirmish_vehicle_mount_state_confirmed", details, true);
	}

	protected void SetVehicleMountState(notnull JLH_AmbientSkirmishRecord record, bool friendlySide, int vehicleIndex, JLH_AmbientSkirmishVehicleMountState state, string failedReason)
	{
		JLH_AmbientSkirmishVehicleLifecycle lifecycle = record.GetVehicleLifecycle(friendlySide, vehicleIndex);
		if (!lifecycle)
			return;

		lifecycle.State = state;
		if (failedReason != "")
			lifecycle.FailedReason = failedReason;
	}

	protected void LogVehicleValidationDeferred(notnull JLH_AmbientSkirmishRecord record, string sideLabel, int vehicleIndex, JLH_AmbientSkirmishVehicleLifecycle lifecycle, string reason, string phase)
	{
		string details = "id=" + record.Id.ToString();
		details = details + " phase=" + phase;
		details = details + " side=" + sideLabel;
		details = details + " vehicleIndex=" + vehicleIndex.ToString();
		details = details + " reason=" + reason;
		if (lifecycle)
		{
			details = details + " state=" + VehicleMountStateLabel(lifecycle.State);
			details = details + " attempts=" + lifecycle.MountAttemptCount.ToString();
			details = details + " validationFailures=" + lifecycle.ValidationFailureCount.ToString();
		}

		LogEvent("validation_deferred_mount_pending", details, true);
	}

	protected bool IsDeferredVehicleValidationReason(string reason)
	{
		if (reason.Contains("_vehicle_validation_mount_pending"))
			return true;
		if (reason.Contains("_vehicle_validation_occupancy_pending"))
			return true;
		if (reason.Contains("_vehicle_validation_cooldown"))
			return true;
		if (reason.Contains("_vehicle_mount_cooldown"))
			return true;

		return false;
	}

	protected bool IsRecoverableVehicleValidationReason(string reason)
	{
		if (reason.Contains("_vehicle_validation_no_driver"))
			return true;
		if (reason.Contains("_vehicle_validation_no_gunner"))
			return true;
		if (reason.Contains("_vehicle_validation_not_filled"))
			return true;

		return false;
	}

	protected void RecordVehicleOccupancyCommandState(JLH_AmbientSkirmishRecord record, bool friendlySide, int vehicleIndex, ResourceName vehiclePrefab, string sideLabel, bool driverMounted, bool gunnerMounted, bool commanderMounted, int mountedCount, string driverCompartment, string gunnerCompartment, string commanderCompartment)
	{
		if (!record)
			return;

		JLH_AmbientSkirmishVehicleLifecycle lifecycle = record.GetVehicleLifecycle(friendlySide, vehicleIndex);
		if (!lifecycle)
			return;

		lifecycle.PendingDriverSeat = driverMounted && IsPendingCompartmentLabel(driverCompartment);
		lifecycle.PendingGunnerSeat = gunnerMounted && IsPendingCompartmentLabel(gunnerCompartment);
		lifecycle.PendingCommanderSeat = commanderMounted && IsPendingCompartmentLabel(commanderCompartment);
		lifecycle.PendingMountedCount = mountedCount;
		lifecycle.PendingDriverCompartment = driverCompartment;
		lifecycle.PendingGunnerCompartment = gunnerCompartment;
		lifecycle.PendingCommanderCompartment = commanderCompartment;

		if (HasPendingVehicleOccupancy(lifecycle))
		{
			lifecycle.LastOccupancyPendingTime = System.GetTickCount();
			lifecycle.State = JLH_AmbientSkirmishVehicleMountState.WAITING_FOR_OCCUPANCY_CONFIRM;
			LogOccupancyPending(record, vehiclePrefab, sideLabel, vehicleIndex, lifecycle, "mount_command");
			return;
		}

		lifecycle.LastOccupancyPendingTime = 0;
		lifecycle.State = JLH_AmbientSkirmishVehicleMountState.WAITING_FOR_OCCUPANCY_CONFIRM;
	}

	protected bool HasPendingVehicleOccupancy(JLH_AmbientSkirmishVehicleLifecycle lifecycle)
	{
		if (!lifecycle)
			return false;

		if (lifecycle.PendingDriverSeat)
			return true;
		if (lifecycle.PendingGunnerSeat)
			return true;
		if (lifecycle.PendingCommanderSeat)
			return true;

		return false;
	}

	protected bool ShouldDeferVehicleOccupancyValidation(JLH_AmbientSkirmishVehicleLifecycle lifecycle, ResourceName vehiclePrefab, string sideLabel, int vehicleIndex, bool driverMounted, bool gunnerMounted, int mountedCrew, int requiredMountedCrew, string driverCompartment, string gunnerCompartment)
	{
		if (!HasPendingVehicleOccupancy(lifecycle))
			return false;

		bool gunnerMissing = requiredMountedCrew > 1 && !gunnerMounted;
		bool requiredSeatsMissing = !driverMounted || gunnerMissing || mountedCrew < requiredMountedCrew;
		if (!requiredSeatsMissing)
			return false;

		int pendingStarted = lifecycle.LastOccupancyPendingTime;
		if (pendingStarted <= 0)
			pendingStarted = lifecycle.LastMountAttemptTime;

		int pendingAgeMs = System.GetTickCount() - pendingStarted;
		if (pendingAgeMs < VEHICLE_OCCUPANCY_CONFIRM_TIMEOUT_MS)
		{
			if (lifecycle.TemporaryHoldAssigned || lifecycle.RouteAssignedAfterStableMount)
				lifecycle.State = JLH_AmbientSkirmishVehicleMountState.VEHICLE_OCCUPANCY_STABILIZING;
			else
				lifecycle.State = JLH_AmbientSkirmishVehicleMountState.WAITING_FOR_OCCUPANCY_CONFIRM;
			LogValidationWaitingPendingSeats(vehiclePrefab, sideLabel, vehicleIndex, lifecycle, driverMounted, gunnerMounted, mountedCrew, driverCompartment, gunnerCompartment, pendingAgeMs);
			return true;
		}

		LogValidationTimeoutFailed(vehiclePrefab, sideLabel, vehicleIndex, lifecycle, driverMounted, gunnerMounted, mountedCrew, driverCompartment, gunnerCompartment, pendingAgeMs);
		ClearVehicleOccupancyPending(lifecycle);
		return false;
	}

	protected void ClearVehicleOccupancyPending(JLH_AmbientSkirmishVehicleLifecycle lifecycle)
	{
		if (!lifecycle)
			return;

		lifecycle.LastOccupancyPendingTime = 0;
		lifecycle.PendingMountedCount = 0;
		lifecycle.PendingDriverSeat = false;
		lifecycle.PendingGunnerSeat = false;
		lifecycle.PendingCommanderSeat = false;
		lifecycle.PendingDriverCompartment = "";
		lifecycle.PendingGunnerCompartment = "";
		lifecycle.PendingCommanderCompartment = "";
	}

	protected void LogOccupancyPending(notnull JLH_AmbientSkirmishRecord record, ResourceName vehiclePrefab, string sideLabel, int vehicleIndex, JLH_AmbientSkirmishVehicleLifecycle lifecycle, string phase)
	{
		string details = "id=" + record.Id.ToString();
		details = details + " phase=" + phase;
		details = details + " side=" + sideLabel;
		details = details + " vehicleIndex=" + vehicleIndex.ToString();
		details = details + " prefab=" + vehiclePrefab;
		details = details + " mounted=" + lifecycle.PendingMountedCount.ToString();
		details = details + " driverCompartment=" + lifecycle.PendingDriverCompartment;
		details = details + " gunnerCompartment=" + lifecycle.PendingGunnerCompartment;
		details = details + " commanderCompartment=" + lifecycle.PendingCommanderCompartment;
		details = details + " state=" + VehicleMountStateLabel(lifecycle.State);
		LogEvent("occupancy_pending", details, true);
	}

	protected void LogValidationWaitingPendingSeats(ResourceName vehiclePrefab, string sideLabel, int vehicleIndex, JLH_AmbientSkirmishVehicleLifecycle lifecycle, bool driverMounted, bool gunnerMounted, int mountedCrew, string driverCompartment, string gunnerCompartment, int pendingAgeMs)
	{
		string details = "side=" + sideLabel;
		details = details + " vehicleIndex=" + vehicleIndex.ToString();
		details = details + " prefab=" + vehiclePrefab;
		details = details + " mountedCrew=" + mountedCrew.ToString();
		details = details + " pendingMounted=" + lifecycle.PendingMountedCount.ToString();
		details = details + " driver=" + JLH_DCF_NodeDebug.BoolLabel(driverMounted);
		details = details + " gunner=" + JLH_DCF_NodeDebug.BoolLabel(gunnerMounted);
		details = details + " driverCompartment=" + driverCompartment;
		details = details + " gunnerCompartment=" + gunnerCompartment;
		details = details + " pendingDriverCompartment=" + lifecycle.PendingDriverCompartment;
		details = details + " pendingGunnerCompartment=" + lifecycle.PendingGunnerCompartment;
		details = details + " pendingAgeMs=" + pendingAgeMs.ToString();
		details = details + " timeoutMs=" + VEHICLE_OCCUPANCY_CONFIRM_TIMEOUT_MS.ToString();
		details = details + " state=" + VehicleMountStateLabel(lifecycle.State);
		LogEvent("validation_waiting_pending_seats", details, true);
	}

	protected void LogValidationTimeoutFailed(ResourceName vehiclePrefab, string sideLabel, int vehicleIndex, JLH_AmbientSkirmishVehicleLifecycle lifecycle, bool driverMounted, bool gunnerMounted, int mountedCrew, string driverCompartment, string gunnerCompartment, int pendingAgeMs)
	{
		string details = "side=" + sideLabel;
		details = details + " vehicleIndex=" + vehicleIndex.ToString();
		details = details + " prefab=" + vehiclePrefab;
		details = details + " mountedCrew=" + mountedCrew.ToString();
		details = details + " pendingMounted=" + lifecycle.PendingMountedCount.ToString();
		details = details + " driver=" + JLH_DCF_NodeDebug.BoolLabel(driverMounted);
		details = details + " gunner=" + JLH_DCF_NodeDebug.BoolLabel(gunnerMounted);
		details = details + " driverCompartment=" + driverCompartment;
		details = details + " gunnerCompartment=" + gunnerCompartment;
		details = details + " pendingDriverCompartment=" + lifecycle.PendingDriverCompartment;
		details = details + " pendingGunnerCompartment=" + lifecycle.PendingGunnerCompartment;
		details = details + " pendingAgeMs=" + pendingAgeMs.ToString();
		details = details + " timeoutMs=" + VEHICLE_OCCUPANCY_CONFIRM_TIMEOUT_MS.ToString();
		LogEvent("validation_timeout_failed", details, true);
	}

	protected void LogOccupancyStable(notnull JLH_AmbientSkirmishRecord record, ResourceName vehiclePrefab, string sideLabel, int vehicleIndex, JLH_AmbientSkirmishVehicleLifecycle lifecycle)
	{
		string details = "id=" + record.Id.ToString();
		details = details + " side=" + sideLabel;
		details = details + " vehicleIndex=" + vehicleIndex.ToString();
		details = details + " prefab=" + vehiclePrefab;
		details = details + " attempts=" + lifecycle.MountAttemptCount.ToString();
		details = details + " validationFailures=" + lifecycle.ValidationFailureCount.ToString();
		details = details + " state=" + VehicleMountStateLabel(lifecycle.State);
		LogEvent("occupancy_stable", details, true);
	}

	protected void LogValidationConfirmed(notnull JLH_AmbientSkirmishRecord record, ResourceName vehiclePrefab, string sideLabel, int vehicleIndex, JLH_AmbientSkirmishVehicleLifecycle lifecycle)
	{
		string details = "id=" + record.Id.ToString();
		details = details + " side=" + sideLabel;
		details = details + " vehicleIndex=" + vehicleIndex.ToString();
		details = details + " prefab=" + vehiclePrefab;
		details = details + " attempts=" + lifecycle.MountAttemptCount.ToString();
		details = details + " validationFailures=" + lifecycle.ValidationFailureCount.ToString();
		details = details + " state=" + VehicleMountStateLabel(lifecycle.State);
		LogEvent("validation_confirmed", details, true);
	}

	protected int GetVehicleMountAttemptCount(JLH_AmbientSkirmishVehicleLifecycle lifecycle)
	{
		if (!lifecycle)
			return 0;

		return lifecycle.MountAttemptCount;
	}

	protected int GetVehicleValidationFailureCount(JLH_AmbientSkirmishVehicleLifecycle lifecycle)
	{
		if (!lifecycle)
			return 0;

		return lifecycle.ValidationFailureCount;
	}

	protected void AbortVehicleSkirmish(notnull JLH_AmbientSkirmishRecord record, string sideLabel, int vehicleIndex, string reason, JLH_AmbientSkirmishVehicleLifecycle lifecycle)
	{
		if (reason == "")
			reason = "max_mount_recovery_failures";

		if (lifecycle)
		{
			lifecycle.State = JLH_AmbientSkirmishVehicleMountState.ABORTED;
			lifecycle.RecoveryLocked = true;
			lifecycle.FailedReason = reason;
		}

		string details = "id=" + record.Id.ToString();
		details = details + " reason=" + reason;
		details = details + " side=" + sideLabel;
		details = details + " vehicleIndex=" + vehicleIndex.ToString();
		if (lifecycle)
		{
			details = details + " attempts=" + lifecycle.MountAttemptCount.ToString();
			details = details + " validationFailures=" + lifecycle.ValidationFailureCount.ToString();
			details = details + " state=" + VehicleMountStateLabel(lifecycle.State);
			details = details + " failedReason=" + lifecycle.FailedReason;
		}

		LogEvent("ambient_skirmish_vehicle_abort", details, true);
		MarkCooldown();
		CleanupRuntimeRecord(record, "ambient_skirmish_" + reason, true);
	}

	protected string VehicleMountStateLabel(JLH_AmbientSkirmishVehicleMountState state)
	{
		if (state == JLH_AmbientSkirmishVehicleMountState.WAITING_FOR_CREW)
			return "WAITING_FOR_CREW";
		if (state == JLH_AmbientSkirmishVehicleMountState.MOUNTING)
			return "MOUNTING";
		if (state == JLH_AmbientSkirmishVehicleMountState.VEHICLE_OCCUPANCY_STABILIZING)
			return "VEHICLE_OCCUPANCY_STABILIZING";
		if (state == JLH_AmbientSkirmishVehicleMountState.WAITING_FOR_MOUNT_STABLE)
			return "WAITING_FOR_MOUNT_STABLE";
		if (state == JLH_AmbientSkirmishVehicleMountState.WAITING_FOR_OCCUPANCY_CONFIRM)
			return "WAITING_FOR_OCCUPANCY_CONFIRM";
		if (state == JLH_AmbientSkirmishVehicleMountState.VALIDATING_MOUNT)
			return "VALIDATING_MOUNT";
		if (state == JLH_AmbientSkirmishVehicleMountState.OCCUPANCY_STABLE)
			return "OCCUPANCY_STABLE";
		if (state == JLH_AmbientSkirmishVehicleMountState.VALIDATED)
			return "VALIDATED";
		if (state == JLH_AmbientSkirmishVehicleMountState.ROUTE_READY)
			return "ROUTE_READY";
		if (state == JLH_AmbientSkirmishVehicleMountState.ROUTE_ASSIGN_ALLOWED)
			return "ROUTE_ASSIGN_ALLOWED";
		if (state == JLH_AmbientSkirmishVehicleMountState.ROUTE_ASSIGNED)
			return "ROUTE_ASSIGNED";
		if (state == JLH_AmbientSkirmishVehicleMountState.FAILED_RECOVERY)
			return "FAILED_RECOVERY";
		if (state == JLH_AmbientSkirmishVehicleMountState.ABORTED)
			return "ABORTED";

		return "UNKNOWN";
	}

	protected bool ShouldRetryVehicleMount(string reason, int attempt)
	{
		if (!GetGame() || !GetGame().GetCallqueue())
			return false;

		if (reason.Contains("_vehicle_crew_not_ready"))
			return attempt < VEHICLE_CREW_READY_MAX_ATTEMPTS;
		if (reason.Contains("_vehicle_mount_cooldown"))
			return attempt < VEHICLE_MOUNT_MAX_ATTEMPTS;

		if (attempt >= VEHICLE_MOUNT_MAX_ATTEMPTS)
			return false;

		if (reason.Contains("_vehicle_no_driver"))
			return true;
		if (reason.Contains("_vehicle_no_gunner"))
			return attempt < 1;
		if (reason.Contains("_vehicle_no_commander"))
			return true;

		return false;
	}

	bool ShouldSpawnFriendlyVehicle(notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		if (!definition.RequiresVehicles)
			return false;

		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.CONVOY_AMBUSH_US_ATTACKS_USSR)
			return false;

		return true;
	}

	bool ShouldSpawnEnemyVehicle(notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		if (!definition.RequiresVehicles)
			return false;

		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.VEHICLE_SKIRMISH)
			return true;
		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.CONVOY_AMBUSH_US_ATTACKS_USSR)
			return true;

		return false;
	}

	protected bool ShouldAssignMountedVehicleHold(notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		return false;
	}

	protected bool ShouldAssignVehicleBehaviorAfterMountValidation(notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		return definition.RequiresVehicles;
	}

	protected void ResolveVehicleSkirmishMix(notnull JLH_AmbientSkirmishEventDefinition definition, bool spawnFriendlyVehicle, bool spawnEnemyVehicle, out int friendlyVehicleCount, out int enemyVehicleCount, out int friendlySupportGroups, out int enemySupportGroups)
	{
		friendlyVehicleCount = 0;
		enemyVehicleCount = 0;
		friendlySupportGroups = 0;
		enemySupportGroups = 0;

		if (spawnFriendlyVehicle)
			friendlyVehicleCount = 1;
		if (spawnEnemyVehicle)
			enemyVehicleCount = 1;

		if (!spawnFriendlyVehicle)
		{
			friendlyVehicleCount = 0;
			friendlySupportGroups = 0;
		}

		if (!spawnEnemyVehicle)
		{
			enemyVehicleCount = 0;
			enemySupportGroups = 0;
		}

		friendlyVehicleCount = Math.Clamp(friendlyVehicleCount, 0, 3);
		enemyVehicleCount = Math.Clamp(enemyVehicleCount, 0, 3);
		friendlySupportGroups = Math.Clamp(friendlySupportGroups, 0, 2);
		enemySupportGroups = Math.Clamp(enemySupportGroups, 0, 2);

		string mixLog = "event=" + definition.SkirmishEventName;
		mixLog = mixLog + " friendlyVehicles=" + friendlyVehicleCount.ToString();
		mixLog = mixLog + " enemyVehicles=" + enemyVehicleCount.ToString();
		mixLog = mixLog + " friendlySupportGroups=" + friendlySupportGroups.ToString();
		mixLog = mixLog + " enemySupportGroups=" + enemySupportGroups.ToString();
		LogEvent("ambient_skirmish_vehicle_mix_selected", mixLog, true);
	}

	bool ShouldUseMountedVehicleCrewPlan(notnull JLH_AmbientSkirmishEventDefinition definition, int vehicleCount, int supportGroups)
	{
		JLH_AmbientSkirmishEventType eventType = definition.SkirmishEventType;
		if (eventType != JLH_AmbientSkirmishEventType.VEHICLE_SKIRMISH)
			return false;

		return vehicleCount > 0 || supportGroups > 0;
	}

	IEntity SpawnSkirmishVehicle(ResourceName vehiclePrefab, vector position, vector facePosition, string sideLabel, out string reason)
	{
		reason = "";
		if (vehiclePrefab == "")
		{
			reason = sideLabel + "_vehicle_prefab_missing";
			return null;
		}

		IEntity vehicle = JLH_AddonSpawnUtility.SpawnEntity(vehiclePrefab, position, "ambient_skirmish_" + sideLabel + "_vehicle_spawn");
		if (!vehicle)
		{
			reason = sideLabel + "_vehicle_spawn_failed";
			return null;
		}

		OrientSkirmishVehicleToward(vehicle, position, facePosition);
		LogEvent("ambient_skirmish_vehicle_spawned", "side=" + sideLabel + " prefab=" + vehiclePrefab + " pos=" + position.ToString() + " facing=" + facePosition.ToString(), true);
		return vehicle;
	}

	protected bool SpawnSkirmishVehicles(notnull JLH_AmbientSkirmishEventDefinition definition, bool friendlySide, vector basePosition, vector facePosition, string sideLabel, int vehicleCount, notnull array<IEntity> outVehicles, out string reason)
	{
		outVehicles.Clear();
		reason = "";

		int safeVehicleCount = Math.Clamp(vehicleCount, 0, 3);
		for (int i = 0; i < safeVehicleCount; i++)
		{
			ResourceName vehiclePrefab = ResolveVehiclePrefabForSlot(definition, friendlySide, i, safeVehicleCount);
			vector spawnPosition = BuildVehicleSkirmishSpawnPosition(basePosition, i, safeVehicleCount);

			IEntity vehicle = SpawnSkirmishVehicle(vehiclePrefab, spawnPosition, facePosition, sideLabel, reason);
			if (!vehicle)
			{
				CleanupSpawnedVehicles(outVehicles, "ambient_skirmish_" + sideLabel + "_vehicle_partial_cleanup");
				return false;
			}

			outVehicles.Insert(vehicle);
		}

		return !outVehicles.IsEmpty();
	}

	protected void OrientSkirmishVehicleToward(IEntity vehicle, vector position, vector facePosition)
	{
		if (!vehicle)
			return;

		vector forward = DirectionFromTo(position, facePosition);
		vector right = Vector(forward[2], 0.0, -forward[0]);
		vector up = Vector(0.0, 1.0, 0.0);

		vector transform[4];
		transform[0] = right;
		transform[1] = up;
		transform[2] = forward;
		transform[3] = JLH_AddonWorldQuery.GroundPosition(position);
		vehicle.SetTransform(transform);
	}

	protected ResourceName ResolveVehiclePrefabForSlot(notnull JLH_AmbientSkirmishEventDefinition definition, bool friendlySide, int vehicleIndex, int vehicleCount)
	{
		ResourceName primaryPrefab;
		if (friendlySide)
			primaryPrefab = definition.FriendlyVehiclePrefab;
		else
			primaryPrefab = definition.EnemyVehiclePrefab;

		return primaryPrefab;
	}

	bool TryResolveVehiclePrefabForSide(notnull JLH_AmbientSkirmishEventDefinition definition, bool friendlySide, out ResourceName vehiclePrefab, out string source, out string reason)
	{
		vehiclePrefab = "";
		source = "";
		reason = "";

		string factionKey = m_sFriendlyFactionKey;
		if (!friendlySide)
			factionKey = JLH_DCF_FactionCompatibilityRegistry.ResolveEnemyFactionKey(m_sFriendlyFactionKey, m_sEnemyFactionKey);

		if (factionKey == "")
		{
			reason = "vehicle_side_faction_missing";
			return false;
		}

		array<string> requiredLabels = {};
		array<string> preferredLabels = {};
		array<string> excludedLabels = {};
		array<string> fallbackLabels = {};
		if (friendlySide)
			BuildSideVehicleLabelFilters(definition, m_aFriendlyRequiredVehicleLabels, m_aFriendlyPreferredVehicleLabels, m_aFriendlyExcludedVehicleLabels, m_aFriendlyFallbackVehicleLabels, requiredLabels, preferredLabels, excludedLabels, fallbackLabels);
		else
			BuildSideVehicleLabelFilters(definition, m_aEnemyRequiredVehicleLabels, m_aEnemyPreferredVehicleLabels, m_aEnemyExcludedVehicleLabels, m_aEnemyFallbackVehicleLabels, requiredLabels, preferredLabels, excludedLabels, fallbackLabels);

		int fallbackCandidateCount;
		string candidateReason;
		int candidateCount = JLH_CORE_CompatibilityResolver.CountVehiclePrefabCandidatesByLabelFilters(factionKey, requiredLabels, preferredLabels, excludedLabels, fallbackLabels, fallbackCandidateCount, candidateReason);
		string candidateLog = "event=" + definition.SkirmishEventName;
		if (friendlySide)
			candidateLog = candidateLog + " side=friendly";
		else
			candidateLog = candidateLog + " side=enemy";
		candidateLog = candidateLog + " faction=" + factionKey;
		candidateLog = candidateLog + " candidates=" + candidateCount.ToString();
		candidateLog = candidateLog + " fallbackCandidates=" + fallbackCandidateCount.ToString();
		candidateLog = candidateLog + " required=" + FormatVehicleLabelList(requiredLabels);
		candidateLog = candidateLog + " preferred=" + FormatVehicleLabelList(preferredLabels);
		candidateLog = candidateLog + " excluded=" + FormatVehicleLabelList(excludedLabels);
		candidateLog = candidateLog + " fallback=" + FormatVehicleLabelList(fallbackLabels);
		if (candidateReason != "")
			candidateLog = candidateLog + " reason=" + candidateReason;
		LogEvent("ambient_skirmish_vehicle_candidate_pool", candidateLog, true);

		JLH_CORE_VehicleLabelSelection selection;
		if (!JLH_CORE_CompatibilityResolver.TrySelectVehiclePrefabByLabelFilters(factionKey, requiredLabels, preferredLabels, excludedLabels, fallbackLabels, selection, reason))
		{
			if (!friendlySide && TryResolveEnemyProfileVehicleForDefinition(definition, factionKey, requiredLabels, preferredLabels, vehiclePrefab, source))
			{
				reason = "";
				return true;
			}

			return false;
		}

		vehiclePrefab = selection.Prefab;
		source = selection.Source;
		return vehiclePrefab != "";
	}

	protected bool TryResolveEnemyProfileVehicleForDefinition(notnull JLH_AmbientSkirmishEventDefinition definition, string factionKey, notnull array<string> requiredLabels, notnull array<string> preferredLabels, out ResourceName vehiclePrefab, out string source)
	{
		vehiclePrefab = "";
		source = "";
		if (!IsUSSRLikeVehicleFaction(factionKey))
			return false;

		string role = ResolveEnemyProfileVehicleRole(requiredLabels, preferredLabels);
		ResourceName profilePrefab;
		if (!JLH_DCF_DEVEnemyAssetResolver.TryResolveEnemyVehiclePrefabByRole(role, profilePrefab))
			return false;

		if (profilePrefab == "")
			return false;

		vehiclePrefab = profilePrefab;
		source = "enemy_profile:" + role;
		LogEvent("ambient_skirmish_enemy_vehicle_profile_fallback", "event=" + definition.SkirmishEventName + " role=" + role + " prefab=" + vehiclePrefab, true);
		return true;
	}

	protected bool IsUSSRLikeVehicleFaction(string factionKey)
	{
		factionKey.ToUpper();
		return factionKey == "USSR" || factionKey == "FIA" || factionKey == "OPFOR";
	}

	protected string ResolveEnemyProfileVehicleRole(notnull array<string> requiredLabels, notnull array<string> preferredLabels)
	{
		if (VehicleLabelListsContain(requiredLabels, preferredLabels, "TRAIT_ARMORED") || VehicleLabelListsContain(requiredLabels, preferredLabels, "VEHICLE_ARMORED"))
			return JLH_EnemyAssetRole.VEHICLE_PATROL_ARMORED;

		if (VehicleLabelListsContain(requiredLabels, preferredLabels, "TRAIT_ARMED"))
			return JLH_EnemyAssetRole.VEHICLE_PATROL_ARMED;

		return JLH_EnemyAssetRole.VEHICLE_PATROL_LIGHT;
	}

	protected bool VehicleLabelListsContain(notnull array<string> requiredLabels, notnull array<string> preferredLabels, string labelName)
	{
		if (labelName == "")
			return false;

		if (requiredLabels.Contains(labelName))
			return true;

		return preferredLabels.Contains(labelName);
	}

	vector BuildVehicleSkirmishSpawnPosition(vector basePosition, int vehicleIndex, int vehicleCount)
	{
		vector baseGround = JLH_AddonWorldQuery.GroundPosition(basePosition);
		if (vehicleCount <= 1)
			return baseGround;

		float ringDistance = Math.Clamp(14.0 + (vehicleCount * 4.0), 18.0, 32.0);
		float angleStep = 360.0 / Math.Max(1.0, vehicleCount);
		float angle = (angleStep * vehicleIndex) + Math.RandomFloat(-8.0, 8.0);
		for (int attempt = 0; attempt < 6; attempt++)
		{
			vector candidate = JLH_AddonWorldQuery.GroundPosition(baseGround + (JLH_AddonWorldQuery.DirectionFromAngle(angle + (attempt * 61.0)) * ringDistance));
			if (!JLH_AddonWorldQuery.IsWaterOrNearWater(candidate, 4.0))
				return candidate;
		}

		return baseGround;
	}


	void CleanupSpawnedVehicles(notnull array<IEntity> vehicles, string reason)
	{
		for (int i = vehicles.Count() - 1; i >= 0; i--)
		{
			if (vehicles[i])
				JLH_AddonSpawnUtility.DeleteRuntimeEntity(vehicles[i], reason);
		}

		vehicles.Clear();
	}

	protected bool IsVehicleCrewGroupPopulationReadyForMount(JLH_AmbientSkirmishRecord record, SCR_AIGroup group, notnull JLH_AmbientSkirmishEventDefinition definition, string sideLabel, int vehicleIndex, out int crewSeen, out int unitsSeen, out int readyCrew, out int requiredReadyCrew, out bool replicationNotReady)
	{
		crewSeen = 0;
		unitsSeen = 0;
		readyCrew = 0;
		requiredReadyCrew = 0;
		replicationNotReady = false;

		if (!group)
			return false;

		bool requireGunner = RequiresVehicleGunner(definition);
		int targetCrew = GetMountedVehicleCrewTarget(definition);
		bool allowCommander = targetCrew >= 3;
		requiredReadyCrew = GetRequiredVehicleCrewPopulationForMount(record, sideLabel, vehicleIndex, targetCrew, requireGunner, allowCommander);

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		crewSeen = agents.Count();
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!unit)
				continue;

			unitsSeen++;
			if (!JLH_AddonSpawnUtility.IsUnitAlive(unit))
				continue;

			if (!EntityHasRpl(unit))
			{
				replicationNotReady = true;
				continue;
			}

			readyCrew++;
		}

		if (crewSeen <= 0 || unitsSeen <= 0)
			return false;

		if (replicationNotReady && readyCrew <= 0)
			return false;

		return readyCrew >= requiredReadyCrew;
	}

	protected void LogVehicleCrewPopulationWaiting(notnull JLH_AmbientSkirmishRecord record, SCR_AIGroup group, string sideLabel, int vehicleIndex, int crewSeen, int unitsSeen, int readyCrew, int requiredReadyCrew, bool replicationNotReady, string reason)
	{
		string details = "id=" + record.Id.ToString();
		details = details + " side=" + sideLabel;
		details = details + " vehicleIndex=" + vehicleIndex.ToString();
		details = details + " crewGroup=" + EntityDebugLabel(group);
		details = details + " crewSeen=" + crewSeen.ToString();
		details = details + " unitsSeen=" + unitsSeen.ToString();
		details = details + " readyCrew=" + readyCrew.ToString();
		details = details + " requiredReadyCrew=" + requiredReadyCrew.ToString();
		details = details + " replicationNotReady=" + JLH_DCF_NodeDebug.BoolLabel(replicationNotReady);
		details = details + " reason=" + reason;
		LogEvent("ambient_skirmish_vehicle_crew_population_waiting", details, true);
	}

	protected void MarkVehicleCrewPopulationConfirmed(notnull JLH_AmbientSkirmishRecord record, string sideLabel, int vehicleIndex)
	{
		JLH_AmbientSkirmishVehicleLifecycle lifecycle = record.GetVehicleLifecycle(sideLabel == "friendly", vehicleIndex);
		if (lifecycle)
			lifecycle.FullCrewPopulationConfirmed = true;
	}

	protected bool MountSkirmishVehicleSide(notnull JLH_AmbientSkirmishRecord record, notnull array<SCR_AIGroup> groups, notnull array<IEntity> vehicles, array<ResourceName> vehiclePrefabs, ResourceName fallbackVehiclePrefab, notnull JLH_AmbientSkirmishEventDefinition definition, string sideLabel, out string reason)
	{
		reason = "";
		if (vehicles.IsEmpty())
		{
			reason = sideLabel + "_vehicle_mount_missing_vehicle";
			return false;
		}

		if (groups.Count() < vehicles.Count())
		{
			reason = sideLabel + "_vehicle_mount_missing_crew_group";
			return false;
		}

		if (!VerifyUniqueCrewGroupsForVehicles(record.Id, sideLabel, groups, vehicles.Count(), "pre_mount"))
		{
			reason = sideLabel + "_vehicle_mount_reused_crew_group";
			return false;
		}

		for (int i = 0; i < vehicles.Count(); i++)
		{
			IEntity vehicle = vehicles[i];
			SCR_AIGroup group = groups[i];
			if (!vehicle || !group)
			{
				reason = sideLabel + "_vehicle_mount_missing_group_or_vehicle_index_" + i.ToString();
				return false;
			}

			int crewSeen;
			int unitsSeen;
			int readyCrew;
			int requiredReadyCrew;
			bool replicationNotReady;
			if (!IsVehicleCrewGroupPopulationReadyForMount(record, group, definition, sideLabel, i, crewSeen, unitsSeen, readyCrew, requiredReadyCrew, replicationNotReady))
			{
				if (crewSeen <= 0 || unitsSeen <= 0)
					reason = sideLabel + "_vehicle_crew_not_ready";
				else if (replicationNotReady && readyCrew <= 0)
					reason = sideLabel + "_vehicle_crew_not_ready_replication_not_ready";
				else
					reason = sideLabel + "_vehicle_crew_not_ready_full_group_population";

				LogVehicleCrewPopulationWaiting(record, group, sideLabel, i, crewSeen, unitsSeen, readyCrew, requiredReadyCrew, replicationNotReady, reason);
				return false;
			}

		}

		for (int i = 0; i < vehicles.Count(); i++)
		{
			IEntity vehicle = vehicles[i];
			SCR_AIGroup group = groups[i];
			MarkVehicleCrewPopulationConfirmed(record, sideLabel, i);

			ResourceName vehiclePrefab = ResolveRuntimeVehiclePrefab(vehiclePrefabs, i, fallbackVehiclePrefab);
			if (!BeginVehicleMountAttempt(record, sideLabel == "friendly", i, sideLabel, "initial_mount", reason))
				return false;

			if (!MountSkirmishVehicleCrewGroup(record, group, vehicle, vehiclePrefab, definition, sideLabel, i, reason))
				return false;

			AssignVehicleStabilizationHold(record, group, vehicle, sideLabel, sideLabel == "friendly", i, "initial_mount");
			LogCrewGroupAssignedVehicle(record, sideLabel, group, vehicle, vehiclePrefab, i, vehicles.Count(), "vehicle_occupancy_stabilizing", CountMountedVehicleCrew(group, vehicle));
		}

		return true;
	}

	protected bool AssignVehicleStabilizationHold(notnull JLH_AmbientSkirmishRecord record, SCR_AIGroup group, IEntity vehicle, string sideLabel, bool friendlySide, int vehicleIndex, string phase)
	{
		JLH_AmbientSkirmishVehicleLifecycle lifecycle = record.GetVehicleLifecycle(friendlySide, vehicleIndex);
		if (!group || !vehicle || !lifecycle)
			return false;

		if (lifecycle.RouteAssignedAfterStableMount && GetGroupWaypointTypeLabel(group) != "NONE")
			return true;

		vector holdTarget = JLH_AddonWorldQuery.GroundPosition(vehicle.GetOrigin());
		bool assigned = JLH_AddonSpawnUtility.AssignMountedVehicleHold(group, holdTarget, record.Runtime, "ambient_skirmish_" + sideLabel + "_vehicle_stabilization_hold_" + vehicleIndex.ToString());
		ActivateSkirmishVehicleGroupAI(group);

		lifecycle.TemporaryHoldAssigned = assigned;
		if (!lifecycle.MountValidated)
			lifecycle.State = JLH_AmbientSkirmishVehicleMountState.VEHICLE_OCCUPANCY_STABILIZING;

		string holdLog = "id=" + record.Id.ToString();
		holdLog = holdLog + " phase=" + phase;
		holdLog = holdLog + " side=" + sideLabel;
		holdLog = holdLog + " vehicleIndex=" + vehicleIndex.ToString();
		holdLog = holdLog + " target=" + holdTarget.ToString();
		holdLog = holdLog + " assigned=" + JLH_DCF_NodeDebug.BoolLabel(assigned);
		holdLog = holdLog + " currentWaypointType=" + GetGroupWaypointTypeLabel(group);
		holdLog = holdLog + " groupCombatMode=" + GetGroupCombatModeLabel(group, IsGroupCombatActive(group));
		holdLog = holdLog + " state=" + VehicleMountStateLabel(lifecycle.State);
		LogEvent("vehicle_hold_task_assigned", holdLog, true);

		string stableLog = "id=" + record.Id.ToString();
		stableLog = stableLog + " phase=" + phase;
		stableLog = stableLog + " side=" + sideLabel;
		stableLog = stableLog + " vehicleIndex=" + vehicleIndex.ToString();
		stableLog = stableLog + " vehicle=" + EntityDebugLabel(vehicle);
		stableLog = stableLog + " crewGroup=" + EntityDebugLabel(group);
		stableLog = stableLog + " state=" + VehicleMountStateLabel(lifecycle.State);
		LogEvent("vehicle_occupancy_stabilizing", stableLog, true);

		if (!lifecycle.RoutePendingLogged)
		{
			lifecycle.RoutePendingLogged = true;
			LogEvent("vehicle_route_pending", "id=" + record.Id.ToString() + " phase=" + phase + " side=" + sideLabel + " vehicleIndex=" + vehicleIndex.ToString() + " state=" + VehicleMountStateLabel(lifecycle.State), true);
		}

		return assigned;
	}

	protected bool MountSkirmishVehicleCrew(notnull array<SCR_AIGroup> groups, IEntity vehicle, ResourceName vehiclePrefab, notnull JLH_AmbientSkirmishEventDefinition definition, string sideLabel, out string reason)
	{
		reason = "";
		if (groups.IsEmpty() || !groups[0] || !vehicle)
		{
			reason = sideLabel + "_vehicle_mount_missing_group_or_vehicle";
			return false;
		}

		return MountSkirmishVehicleCrewGroup(null, groups[0], vehicle, vehiclePrefab, definition, sideLabel, 0, reason);
	}

	protected ResourceName ResolveRuntimeVehiclePrefab(array<ResourceName> vehiclePrefabs, int vehicleIndex, ResourceName fallbackVehiclePrefab)
	{
		if (vehiclePrefabs && vehicleIndex >= 0 && vehicleIndex < vehiclePrefabs.Count() && vehiclePrefabs[vehicleIndex] != "")
			return vehiclePrefabs[vehicleIndex];

		return fallbackVehiclePrefab;
	}

	protected IEntity ResolveVehicleMountTarget(IEntity vehicle)
	{
		if (!vehicle)
			return null;

		IEntity usageOwner = null;
		SCR_AIVehicleUsageComponent vehicleUsage = SCR_AIVehicleUsageComponent.FindOnNearestParent(vehicle, usageOwner);
		if (vehicleUsage && usageOwner)
			return usageOwner;

		return vehicle;
	}

	protected void LogSkirmishVehicleRuntimeTargetAudit(ResourceName vehiclePrefab, string sideLabel, int vehicleIndex, IEntity vehicle, IEntity mountTarget, string phase)
	{
		if (!ShouldLogVehicleDeep())
			return;

		IEntity usageOwner = null;
		SCR_AIVehicleUsageComponent vehicleUsage = null;
		if (vehicle)
			vehicleUsage = SCR_AIVehicleUsageComponent.FindOnNearestParent(vehicle, usageOwner);

		string auditLog = "phase=" + phase;
		auditLog = auditLog + " side=" + sideLabel;
		auditLog = auditLog + " vehicleIndex=" + vehicleIndex.ToString();
		auditLog = auditLog + " prefab=" + vehiclePrefab;
		auditLog = auditLog + " runtimeVehicle=" + EntityDebugLabel(vehicle);
		auditLog = auditLog + " runtimeVehicleType=" + EntityTypeLabel(vehicle);
		auditLog = auditLog + " runtimeVehicleRoot=" + EntityRootDebugLabel(vehicle);
		auditLog = auditLog + " runtimeVehicleHasRpl=" + JLH_DCF_NodeDebug.BoolLabel(EntityHasRpl(vehicle));
		auditLog = auditLog + " vehicleUsageFound=" + JLH_DCF_NodeDebug.BoolLabel(vehicleUsage != null);
		auditLog = auditLog + " vehicleUsageOwner=" + EntityDebugLabel(usageOwner);
		auditLog = auditLog + " vehicleUsageOwnerType=" + EntityTypeLabel(usageOwner);
		auditLog = auditLog + " moveTarget=" + EntityDebugLabel(mountTarget);
		auditLog = auditLog + " moveTargetType=" + EntityTypeLabel(mountTarget);
		auditLog = auditLog + " moveTargetRoot=" + EntityRootDebugLabel(mountTarget);
		auditLog = auditLog + " moveTargetHasRpl=" + JLH_DCF_NodeDebug.BoolLabel(EntityHasRpl(mountTarget));
		auditLog = auditLog + " occupancyValidationTarget=" + EntityDebugLabel(mountTarget);
		auditLog = auditLog + " compartmentManagerFound=unknown";
		auditLog = auditLog + " compartmentManagerClass=safe_api_unavailable";
		auditLog = auditLog + " totalRuntimeCompartments=-1";
		LogEvent("ambient_skirmish_vehicle_runtime_target_audit", auditLog, false);
	}

	protected bool ShouldUseSharedCombatVehicleMount(notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		return false;
	}

	protected bool MountSharedCombatVehicleCrewGroup(JLH_AmbientSkirmishRecord record, SCR_AIGroup group, IEntity vehicle, ResourceName vehiclePrefab, notnull JLH_AmbientSkirmishEventDefinition definition, string sideLabel, int vehicleIndex, out string reason)
	{
		reason = "";
		if (!group || !vehicle)
		{
			reason = sideLabel + "_vehicle_mount_missing_group_or_vehicle";
			return false;
		}

		LogSkirmishVehicleRuntimeTargetAudit(vehiclePrefab, sideLabel, vehicleIndex, vehicle, vehicle, "combat_mount");
		//group.DeactivateAllMembers();

		bool requireGunner = RequiresVehicleGunner(definition);
		int targetCrew = GetMountedVehicleCrewTarget(definition);
		bool allowCommander = targetCrew >= 3;
		bool btrStrictTurret = IsBtrVehiclePrefab(vehiclePrefab);
		if (btrStrictTurret && requireGunner)
			LogBtrTurretRequired(vehiclePrefab, sideLabel, vehicleIndex, targetCrew);

		bool driverMounted = false;
		bool gunnerMounted = false;
		bool commanderMounted = false;
		bool replicationNotReady = false;
		IEntity driverEntity = null;
		IEntity gunnerEntity = null;
		IEntity commanderEntity = null;
		string driverCompartment = "NONE";
		string gunnerCompartment = "NONE";
		string commanderCompartment = "NONE";
		int unmoved = 0;

		JLH_AmbientSkirmishVehicleMountLog mountLog = new JLH_AmbientSkirmishVehicleMountLog();
		mountLog.SideLabel = sideLabel;
		mountLog.VehicleIndex = vehicleIndex;
		mountLog.VehiclePrefab = vehiclePrefab;
		mountLog.VehicleEntity = vehicle;
		mountLog.MountTargetEntity = vehicle;
		mountLog.CrewGroup = group;
		mountLog.ArmedRequired = requireGunner;
		mountLog.DriverCompartment = "NONE";
		mountLog.GunnerCompartment = "NONE";
		mountLog.CommanderCompartment = "NONE";

		array<AIAgent> agents = {};
		array<IEntity> mountedCrewUnits = {};
		group.GetAgents(agents);
		int crewSeen = agents.Count();
		int unitsSeen = 0;
		int readyCrew = 0;
		int requiredReadyCrew = GetRequiredVehicleCrewPopulationForMount(record, sideLabel, vehicleIndex, targetCrew, requireGunner, allowCommander);

		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!unit)
				continue;

			unitsSeen++;
			if (!JLH_AddonSpawnUtility.IsUnitAlive(unit))
				continue;

			if (!EntityHasRpl(unit))
			{
				replicationNotReady = true;
				continue;
			}

			readyCrew++;
		}

		if (crewSeen <= 0 || unitsSeen <= 0)
		{
			reason = sideLabel + "_vehicle_crew_not_ready";
			SetSkirmishVehicleCrewReadiness(mountLog, requiredReadyCrew, readyCrew);
			UpdateSkirmishVehicleMountLog(mountLog, crewSeen, unitsSeen, targetCrew, 0, false, false, false, driverCompartment, gunnerCompartment, commanderCompartment, unmoved, 0, reason);
			LogSkirmishVehicleMountAttempt(mountLog);
			return false;
		}

		if (replicationNotReady && readyCrew <= 0)
		{
			reason = sideLabel + "_vehicle_crew_not_ready_replication_not_ready";
			SetSkirmishVehicleCrewReadiness(mountLog, requiredReadyCrew, readyCrew);
			UpdateSkirmishVehicleMountLog(mountLog, crewSeen, unitsSeen, targetCrew, 0, false, false, false, driverCompartment, gunnerCompartment, commanderCompartment, unmoved, 0, reason);
			LogSkirmishVehicleMountAttempt(mountLog);
			return false;
		}

		if (readyCrew < requiredReadyCrew)
		{
			reason = sideLabel + "_vehicle_crew_not_ready_full_group_population";
			SetSkirmishVehicleCrewReadiness(mountLog, requiredReadyCrew, readyCrew);
			UpdateSkirmishVehicleMountLog(mountLog, crewSeen, unitsSeen, targetCrew, 0, false, false, false, driverCompartment, gunnerCompartment, commanderCompartment, unmoved, 0, reason);
			LogSkirmishVehicleMountAttempt(mountLog);
			return false;
		}

		unitsSeen = 0;
		readyCrew = 0;
		replicationNotReady = false;

		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!unit)
				continue;

			unitsSeen++;
			if (!JLH_AddonSpawnUtility.IsUnitAlive(unit))
				continue;

			if (!EntityHasRpl(unit))
			{
				replicationNotReady = true;
				continue;
			}

			readyCrew++;
			if (IsSkirmishUnitInVehicle(unit, vehicle))
			{
				if (!driverMounted && IsSkirmishUnitInCompartmentType(unit, vehicle, ECompartmentType.PILOT))
				{
					driverMounted = true;
					driverEntity = unit;
					mountLog.DriverEntity = unit;
					driverCompartment = GetSkirmishUnitCompartmentLabel(unit, vehicle);
					mountLog.DriverCompartmentCount = 1;
					mountedCrewUnits.Insert(unit);
					continue;
				}

				bool existingGunner = false;
				if (btrStrictTurret)
					existingGunner = IsBtrVehiclePatrolGunnerRole(unit, vehicle);
				else
					existingGunner = IsCombatSkirmishUnitInGunnerRole(unit, vehicle);

				if (!gunnerMounted && existingGunner)
				{
					gunnerMounted = true;
					gunnerEntity = unit;
					mountLog.GunnerEntity = unit;
					gunnerCompartment = GetSkirmishUnitCompartmentLabel(unit, vehicle);
					mountLog.TurretFound = true;
					mountLog.GunnerCompatible = true;
					mountLog.TurretCompartmentCount = 1;
					mountedCrewUnits.Insert(unit);
					if (btrStrictTurret)
						LogBtrTurretMountSuccess(vehiclePrefab, sideLabel, vehicleIndex, "existing_turret", gunnerCompartment, unit, false, false, 0, false);

					continue;
				}

				if (allowCommander && driverMounted && gunnerMounted && !commanderMounted && IsSkirmishUnitInCompartmentType(unit, vehicle, ECompartmentType.CARGO))
				{
					commanderMounted = true;
					commanderEntity = unit;
					mountLog.CommanderEntity = unit;
					commanderCompartment = GetSkirmishUnitCompartmentLabel(unit, vehicle);
					mountLog.CommanderCompartmentCount = 1;
					mountedCrewUnits.Insert(unit);
					continue;
				}

				unmoved++;
				continue;
			}

			SCR_CompartmentAccessComponent access = SCR_CompartmentAccessComponent.Cast(unit.FindComponent(SCR_CompartmentAccessComponent));
			if (!access)
			{
				unmoved++;
				continue;
			}

			unit.SetOrigin(vehicle.GetOrigin());
			bool moved = false;
			if (!driverMounted && access.MoveInVehicle(vehicle, ECompartmentType.PILOT))
			{
				driverMounted = true;
				driverEntity = unit;
				mountLog.DriverEntity = unit;
				driverCompartment = GetSkirmishUnitCompartmentLabel(unit, vehicle);
				if (!IsSkirmishUnitInVehicle(unit, vehicle))
					driverCompartment = EnsurePendingCompartmentLabel(driverCompartment, ECompartmentType.PILOT, false);
				mountLog.DriverCompartmentCount = 1;
				mountedCrewUnits.Insert(unit);
				moved = true;
			}
			else if (requireGunner && !gunnerMounted)
			{
				bool gunnerMoveAccepted = false;
				if (btrStrictTurret)
					gunnerMoveAccepted = TryMountBtrGunnerUsingVehiclePatrolPath(unit, vehicle, vehiclePrefab, sideLabel, vehicleIndex, "gunner_turret", gunnerCompartment);
				else if (access.MoveInVehicle(vehicle, ECompartmentType.TURRET))
				{
					gunnerMoveAccepted = true;
					gunnerCompartment = GetSkirmishUnitCompartmentLabel(unit, vehicle);
					if (!IsSkirmishUnitInVehicle(unit, vehicle))
						gunnerCompartment = EnsurePendingCompartmentLabel(gunnerCompartment, ECompartmentType.TURRET, false);
				}

				if (gunnerMoveAccepted)
				{
					LogBtrMountRoleResolution(vehiclePrefab, sideLabel, vehicleIndex, driverCompartment, "TURRET", gunnerCompartment, true, false);
					gunnerMounted = true;
					gunnerEntity = unit;
					mountLog.GunnerEntity = unit;
					mountLog.TurretFound = true;
					mountLog.GunnerCompatible = true;
					mountLog.TurretCompartmentCount = 1;
					mountedCrewUnits.Insert(unit);
					moved = true;
					if (btrStrictTurret)
						LogBtrTurretMountSuccess(vehiclePrefab, sideLabel, vehicleIndex, "turret_command", gunnerCompartment, unit, false, false, 0, false);
				}
				else
				{
					if (btrStrictTurret)
						LogBtrTurretMountFailed(vehiclePrefab, sideLabel, vehicleIndex, "vehicle_patrol_turret_move_failed", "TURRET", gunnerCompartment, unit);
				}
			}
			if (allowCommander && driverMounted && gunnerMounted && !commanderMounted)
			{
				if (!moved && access.MoveInVehicle(vehicle, ECompartmentType.CARGO))
				{
					commanderMounted = true;
					commanderEntity = unit;
					mountLog.CommanderEntity = unit;
					commanderCompartment = GetSkirmishUnitCompartmentLabel(unit, vehicle);
					if (!IsSkirmishUnitInVehicle(unit, vehicle))
						commanderCompartment = EnsurePendingCompartmentLabel(commanderCompartment, ECompartmentType.CARGO, false);
					mountLog.CommanderCompartmentCount = 1;
					mountedCrewUnits.Insert(unit);
					moved = true;
				}
			}

			if (!moved)
				unmoved++;
		}

		if (crewSeen <= 0 || unitsSeen <= 0)
		{
			reason = sideLabel + "_vehicle_crew_not_ready";
			SetSkirmishVehicleCrewReadiness(mountLog, requiredReadyCrew, readyCrew);
			UpdateSkirmishVehicleMountLog(mountLog, crewSeen, unitsSeen, targetCrew, mountedCrewUnits.Count(), driverMounted, gunnerMounted, commanderMounted, driverCompartment, gunnerCompartment, commanderCompartment, unmoved, 0, reason);
			LogSkirmishVehicleMountAttempt(mountLog);
			return false;
		}

		if (replicationNotReady && mountedCrewUnits.Count() <= 0)
		{
			reason = sideLabel + "_vehicle_crew_not_ready_replication_not_ready";
			SetSkirmishVehicleCrewReadiness(mountLog, requiredReadyCrew, readyCrew);
			UpdateSkirmishVehicleMountLog(mountLog, crewSeen, unitsSeen, targetCrew, mountedCrewUnits.Count(), driverMounted, gunnerMounted, commanderMounted, driverCompartment, gunnerCompartment, commanderCompartment, unmoved, 0, reason);
			LogSkirmishVehicleMountAttempt(mountLog);
			return false;
		}

		if (readyCrew < requiredReadyCrew)
		{
			reason = sideLabel + "_vehicle_crew_not_ready";
			SetSkirmishVehicleCrewReadiness(mountLog, requiredReadyCrew, readyCrew);
			UpdateSkirmishVehicleMountLog(mountLog, crewSeen, unitsSeen, targetCrew, mountedCrewUnits.Count(), driverMounted, gunnerMounted, commanderMounted, driverCompartment, gunnerCompartment, commanderCompartment, unmoved, 0, reason);
			LogSkirmishVehicleMountAttempt(mountLog);
			return false;
		}

		if (!driverMounted)
		{
			reason = sideLabel + "_vehicle_no_driver";
			UpdateSkirmishVehicleMountLog(mountLog, crewSeen, unitsSeen, targetCrew, mountedCrewUnits.Count(), driverMounted, gunnerMounted, commanderMounted, driverCompartment, gunnerCompartment, commanderCompartment, unmoved, 0, reason);
			LogSkirmishVehicleMountAttempt(mountLog);
			return false;
		}

		if (requireGunner && !gunnerMounted)
		{
			reason = sideLabel + "_vehicle_no_gunner";
			if (btrStrictTurret)
				LogBtrTurretMountFailed(vehiclePrefab, sideLabel, vehicleIndex, reason, "TURRET", gunnerCompartment, gunnerEntity);

			UpdateSkirmishVehicleMountLog(mountLog, crewSeen, unitsSeen, targetCrew, mountedCrewUnits.Count(), driverMounted, gunnerMounted, commanderMounted, driverCompartment, gunnerCompartment, commanderCompartment, unmoved, 0, reason);
			LogSkirmishVehicleMountAttempt(mountLog);
			return false;
		}

		int deletedUnused = PruneVehicleCrewFootUnitsNow(group, vehicle, sideLabel, "shared_combat_mount", mountedCrewUnits);
		UpdateSkirmishVehicleMountLog(mountLog, crewSeen, unitsSeen, targetCrew, mountedCrewUnits.Count(), driverMounted, gunnerMounted, commanderMounted, driverCompartment, gunnerCompartment, commanderCompartment, unmoved, deletedUnused, "");
		LogSkirmishVehicleMountAttempt(mountLog);

		if (record)
			record.SetVehicleCrew(sideLabel == "friendly", vehicleIndex, driverEntity, gunnerEntity);

		if (!RegisterSkirmishUsableVehicle(group, vehicle, sideLabel, reason))
			return false;

		RecordVehicleOccupancyCommandState(record, sideLabel == "friendly", vehicleIndex, vehiclePrefab, sideLabel, driverMounted, gunnerMounted, commanderMounted, mountedCrewUnits.Count(), driverCompartment, gunnerCompartment, commanderCompartment);
		return true;
	}

	protected bool MountSkirmishVehicleCrewGroup(JLH_AmbientSkirmishRecord record, SCR_AIGroup group, IEntity vehicle, ResourceName vehiclePrefab, notnull JLH_AmbientSkirmishEventDefinition definition, string sideLabel, int vehicleIndex, out string reason)
	{
		reason = "";
		if (!group || !vehicle)
		{
			reason = sideLabel + "_vehicle_mount_missing_group_or_vehicle";
			return false;
		}

		if (ShouldUseSharedCombatVehicleMount(definition))
			return MountSharedCombatVehicleCrewGroup(record, group, vehicle, vehiclePrefab, definition, sideLabel, vehicleIndex, reason);

		IEntity mountTarget = ResolveVehicleMountTarget(vehicle);
		if (!mountTarget)
			mountTarget = vehicle;

		LogSkirmishVehicleRuntimeTargetAudit(vehiclePrefab, sideLabel, vehicleIndex, vehicle, mountTarget, "mount");

		//group.DeactivateAllMembers();

		bool driverMounted = false;
		bool gunnerMounted = false;
		bool commanderMounted = false;
		bool replicationNotReady = false;
		int unmoved = 0;
		bool requireGunner = RequiresVehicleGunner(definition);
		int targetCrew = GetMountedVehicleCrewTarget(definition);
		bool allowCommander = targetCrew > 2;
		bool btrStrictTurret = IsBtrVehiclePrefab(vehiclePrefab);
		if (btrStrictTurret)
			mountTarget = vehicle;

		if (btrStrictTurret && requireGunner)
			LogBtrTurretRequired(vehiclePrefab, sideLabel, vehicleIndex, targetCrew);

		string driverCompartment = "NONE";
		string gunnerCompartment = "NONE";
		string commanderCompartment = "NONE";
		string failureReason = "";
		JLH_AmbientSkirmishVehicleMountLog mountLog = new JLH_AmbientSkirmishVehicleMountLog();
		mountLog.SideLabel = sideLabel;
		mountLog.VehicleIndex = vehicleIndex;
		mountLog.VehiclePrefab = vehiclePrefab;
		mountLog.VehicleEntity = vehicle;
		mountLog.MountTargetEntity = mountTarget;
		mountLog.CrewGroup = group;
		mountLog.ArmedRequired = requireGunner;
		mountLog.DriverCompartment = "NONE";
		mountLog.GunnerCompartment = "NONE";
		mountLog.CommanderCompartment = "NONE";

		array<AIAgent> agents = {};
		array<IEntity> aliveUnits = {};
		array<IEntity> mountedCrewUnits = {};
		array<IEntity> pendingCommanderUnits = {};
		array<string> pendingCommanderCompartments = {};
		group.GetAgents(agents);
		int crewSeen = agents.Count();
		int unitsSeen = 0;
		int readyCrew = 0;
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!unit)
				continue;

			unitsSeen++;
			if (!JLH_AddonSpawnUtility.IsUnitAlive(unit))
				continue;

			if (!EntityHasRpl(unit))
			{
				replicationNotReady = true;
				continue;
			}

			aliveUnits.Insert(unit);
			readyCrew++;
		}

		int requiredReadyCrew = GetRequiredVehicleCrewPopulationForMount(record, sideLabel, vehicleIndex, targetCrew, requireGunner, allowCommander);
		if (crewSeen <= 0 || unitsSeen <= 0)
		{
			reason = sideLabel + "_vehicle_crew_not_ready";
			SetSkirmishVehicleCrewReadiness(mountLog, requiredReadyCrew, readyCrew);
			UpdateSkirmishVehicleMountLog(mountLog, crewSeen, unitsSeen, targetCrew, 0, false, false, false, driverCompartment, gunnerCompartment, commanderCompartment, unmoved, 0, reason);
			LogSkirmishVehicleMountAttempt(mountLog);
			return false;
		}

		if (replicationNotReady && readyCrew <= 0)
		{
			reason = sideLabel + "_vehicle_crew_not_ready_replication_not_ready";
			SetSkirmishVehicleCrewReadiness(mountLog, requiredReadyCrew, readyCrew);
			UpdateSkirmishVehicleMountLog(mountLog, crewSeen, unitsSeen, targetCrew, 0, false, false, false, driverCompartment, gunnerCompartment, commanderCompartment, unmoved, 0, reason);
			LogSkirmishVehicleMountAttempt(mountLog);
			return false;
		}

		if (readyCrew < requiredReadyCrew)
		{
			reason = sideLabel + "_vehicle_crew_not_ready_full_group_population";
			SetSkirmishVehicleCrewReadiness(mountLog, requiredReadyCrew, readyCrew);
			UpdateSkirmishVehicleMountLog(mountLog, crewSeen, unitsSeen, targetCrew, 0, false, false, false, driverCompartment, gunnerCompartment, commanderCompartment, unmoved, 0, reason);
			LogSkirmishVehicleMountAttempt(mountLog);
			return false;
		}

		foreach (IEntity existingDriverUnit : aliveUnits)
		{
			if (!existingDriverUnit || mountedCrewUnits.Contains(existingDriverUnit))
				continue;

			if (IsSkirmishUnitInCompartmentType(existingDriverUnit, mountTarget, ECompartmentType.PILOT))
			{
				driverMounted = true;
				driverCompartment = GetSkirmishUnitCompartmentLabel(existingDriverUnit, mountTarget);
				mountLog.DriverCompartmentCount = 1;
				mountedCrewUnits.Insert(existingDriverUnit);
				break;
			}
		}

		foreach (IEntity existingGunnerUnit : aliveUnits)
		{
			if (!existingGunnerUnit || mountedCrewUnits.Contains(existingGunnerUnit))
				continue;

			bool existingGunner = false;
			if (btrStrictTurret)
				existingGunner = IsBtrVehiclePatrolGunnerRole(existingGunnerUnit, mountTarget);
			else if (RequiresStrictVehicleSkirmishGunner(definition))
				existingGunner = IsCombatSkirmishUnitInGunnerRole(existingGunnerUnit, mountTarget);
			else
				existingGunner = IsSkirmishUnitInGunnerRole(existingGunnerUnit, mountTarget);

			if (existingGunner)
			{
				gunnerMounted = true;
				gunnerCompartment = GetSkirmishUnitCompartmentLabel(existingGunnerUnit, mountTarget);
				mountLog.TurretFound = true;
				mountLog.GunnerCompatible = true;
				mountLog.TurretCompartmentCount = 1;
				mountedCrewUnits.Insert(existingGunnerUnit);
				if (btrStrictTurret)
					LogBtrTurretMountSuccess(vehiclePrefab, sideLabel, vehicleIndex, "existing_turret", gunnerCompartment, existingGunnerUnit, false, false, 0, false);

				break;
			}
		}

		if (!driverMounted)
		{
			int driverCompartmentIndex = 0;
			foreach (IEntity driverUnit : aliveUnits)
			{
				if (!driverUnit || mountedCrewUnits.Contains(driverUnit))
					continue;

				bool driverMoveAccepted = TryMountSkirmishUnitIntoCombatCompartment(driverUnit, mountTarget, vehiclePrefab, sideLabel, vehicleIndex, "driver", ECompartmentType.PILOT, false, driverCompartmentIndex, driverCompartment);
				if (driverMoveAccepted)
				{
					driverMounted = true;
					if (!IsSkirmishUnitInCompartmentType(driverUnit, mountTarget, ECompartmentType.PILOT))
						driverCompartment = EnsurePendingCompartmentLabel(driverCompartment, ECompartmentType.PILOT, false);

					mountLog.DriverCompartmentCount = 1;
					mountedCrewUnits.Insert(driverUnit);
					break;
				}

				if (IsSkirmishUnitInVehicle(driverUnit, mountTarget))
					TryEjectSkirmishUnitFromVehicle(driverUnit, mountTarget, "driver_wrong_compartment");

				driverCompartmentIndex++;
			}
		}

		if (!gunnerMounted)
		{
			int turretCompartmentIndex = 0;
			foreach (IEntity turretUnit : aliveUnits)
			{
				if (!turretUnit || mountedCrewUnits.Contains(turretUnit))
					continue;

				bool turretMoveAccepted = false;
				if (btrStrictTurret)
					turretMoveAccepted = TryMountBtrGunnerUsingVehiclePatrolPath(turretUnit, mountTarget, vehiclePrefab, sideLabel, vehicleIndex, "gunner_turret", gunnerCompartment);
				else
					turretMoveAccepted = TryMountSkirmishUnitIntoCombatCompartment(turretUnit, mountTarget, vehiclePrefab, sideLabel, vehicleIndex, "gunner_turret", ECompartmentType.TURRET, false, turretCompartmentIndex, gunnerCompartment);
				if (turretMoveAccepted)
				{
					bool strictTurret = true;
					if (btrStrictTurret)
						strictTurret = !IsSkirmishUnitInVehicle(turretUnit, mountTarget) || IsBtrVehiclePatrolGunnerRole(turretUnit, mountTarget);
					else if (RequiresStrictVehicleSkirmishGunner(definition))
						strictTurret = !IsSkirmishUnitInVehicle(turretUnit, mountTarget) || IsCombatSkirmishUnitInGunnerRole(turretUnit, mountTarget);

					if (!strictTurret)
					{
						TryEjectSkirmishUnitFromVehicle(turretUnit, mountTarget, "btr_turret_request_landed_in_non_turret");
						LogBtrTurretMountFailed(vehiclePrefab, sideLabel, vehicleIndex, "turret_request_landed_in_non_turret", "TURRET", gunnerCompartment, turretUnit);
						turretCompartmentIndex++;
						continue;
					}

					gunnerMounted = true;
					mountLog.TurretFound = true;
					mountLog.GunnerCompatible = true;
					if (RequiresStrictVehicleSkirmishGunner(definition))
					{
						if (!IsCombatSkirmishUnitInGunnerRole(turretUnit, mountTarget))
							gunnerCompartment = EnsurePendingCompartmentLabel(gunnerCompartment, ECompartmentType.TURRET, false);
					}
					else if (!IsSkirmishUnitInGunnerRole(turretUnit, mountTarget))
						gunnerCompartment = EnsurePendingCompartmentLabel(gunnerCompartment, ECompartmentType.TURRET, false);

					mountLog.TurretCompartmentCount = 1;
					mountedCrewUnits.Insert(turretUnit);
					if (btrStrictTurret)
						LogBtrTurretMountSuccess(vehiclePrefab, sideLabel, vehicleIndex, "turret_command", gunnerCompartment, turretUnit, false, false, 0, false);

					break;
				}

				if (IsSkirmishUnitInVehicle(turretUnit, mountTarget))
					TryEjectSkirmishUnitFromVehicle(turretUnit, mountTarget, "gunner_turret_wrong_compartment");

				turretCompartmentIndex++;
			}
		}

		if (!gunnerMounted && !btrStrictTurret && ShouldAllowNonTurretVehicleGunnerFallback(definition))
		{
			int cargoGunnerIndex = 0;
			foreach (IEntity cargoGunnerUnit : aliveUnits)
			{
				if (!cargoGunnerUnit || mountedCrewUnits.Contains(cargoGunnerUnit))
					continue;

				string cargoCompartment = "NONE";
				if (!TryMountSkirmishUnitIntoCombatCompartment(cargoGunnerUnit, mountTarget, vehiclePrefab, sideLabel, vehicleIndex, "gunner_cargo_fire_control", ECompartmentType.CARGO, false, cargoGunnerIndex, cargoCompartment))
				{
					cargoGunnerIndex++;
					continue;
				}

				if (IsSkirmishUnitInGunnerRole(cargoGunnerUnit, mountTarget))
				{
					gunnerMounted = true;
					gunnerCompartment = cargoCompartment;
					mountLog.TurretFound = true;
					mountLog.GunnerCompatible = true;
					mountLog.TurretCompartmentCount = 1;
					mountedCrewUnits.Insert(cargoGunnerUnit);
					break;
				}

				if (!IsSkirmishUnitInVehicle(cargoGunnerUnit, mountTarget) && IsPendingCompartmentLabel(cargoCompartment))
				{
					gunnerMounted = true;
					gunnerCompartment = EnsurePendingCompartmentLabel(cargoCompartment, ECompartmentType.CARGO, false);
					mountLog.TurretFound = true;
					mountLog.GunnerCompatible = true;
					mountLog.TurretCompartmentCount = 1;
					mountedCrewUnits.Insert(cargoGunnerUnit);
					break;
				}

				if (IsSkirmishUnitInVehicle(cargoGunnerUnit, mountTarget))
				{
					if (allowCommander && !IsSkirmishUnitInCompartmentType(cargoGunnerUnit, mountTarget, ECompartmentType.PILOT))
					{
						pendingCommanderUnits.Insert(cargoGunnerUnit);
						pendingCommanderCompartments.Insert(cargoCompartment);
					}
					else
					{
						TryEjectSkirmishUnitFromVehicle(cargoGunnerUnit, mountTarget, "gunner_cargo_no_fire_control");
					}
				}

				cargoGunnerIndex++;
			}
		}

		if (!gunnerMounted && !btrStrictTurret && ShouldAllowNonTurretVehicleGunnerFallback(definition))
		{
			int anyGunnerIndex = 0;
			foreach (IEntity anyGunnerUnit : aliveUnits)
			{
				if (!anyGunnerUnit || mountedCrewUnits.Contains(anyGunnerUnit) || pendingCommanderUnits.Contains(anyGunnerUnit))
					continue;

				string anyCompartment = "NONE";
				if (!TryMountSkirmishUnitIntoCombatCompartment(anyGunnerUnit, mountTarget, vehiclePrefab, sideLabel, vehicleIndex, "gunner_any_fire_control", ECompartmentType.TURRET, true, anyGunnerIndex, anyCompartment))
				{
					anyGunnerIndex++;
					continue;
				}

				if (IsSkirmishUnitInGunnerRole(anyGunnerUnit, mountTarget))
				{
					gunnerMounted = true;
					gunnerCompartment = anyCompartment;
					mountLog.TurretFound = true;
					mountLog.GunnerCompatible = true;
					mountLog.TurretCompartmentCount = 1;
					mountedCrewUnits.Insert(anyGunnerUnit);
					break;
				}

				if (!IsSkirmishUnitInVehicle(anyGunnerUnit, mountTarget) && IsPendingCompartmentLabel(anyCompartment))
				{
					gunnerMounted = true;
					gunnerCompartment = EnsurePendingCompartmentLabel(anyCompartment, ECompartmentType.TURRET, true);
					mountLog.TurretFound = true;
					mountLog.GunnerCompatible = true;
					mountLog.TurretCompartmentCount = 1;
					mountedCrewUnits.Insert(anyGunnerUnit);
					break;
				}

				if (IsSkirmishUnitInVehicle(anyGunnerUnit, mountTarget))
				{
					if (allowCommander && !IsSkirmishUnitInCompartmentType(anyGunnerUnit, mountTarget, ECompartmentType.PILOT))
					{
						pendingCommanderUnits.Insert(anyGunnerUnit);
						pendingCommanderCompartments.Insert(anyCompartment);
					}
					else
					{
						TryEjectSkirmishUnitFromVehicle(anyGunnerUnit, mountTarget, "gunner_any_no_fire_control");
					}
				}

				anyGunnerIndex++;
			}
		}

		if (allowCommander && driverMounted && gunnerMounted)
		{
			for (int pendingIndex = 0; pendingIndex < pendingCommanderUnits.Count(); pendingIndex++)
			{
				IEntity pendingCommander = pendingCommanderUnits[pendingIndex];
				if (!pendingCommander || mountedCrewUnits.Contains(pendingCommander))
					continue;

				if (IsSkirmishUnitInVehicle(pendingCommander, mountTarget) && !IsSkirmishUnitInCompartmentType(pendingCommander, mountTarget, ECompartmentType.PILOT))
				{
					commanderMounted = true;
					commanderCompartment = GetSkirmishUnitCompartmentLabel(pendingCommander, mountTarget);
					if (pendingIndex < pendingCommanderCompartments.Count() && pendingCommanderCompartments[pendingIndex] != "")
						commanderCompartment = pendingCommanderCompartments[pendingIndex];

					mountLog.CommanderCompartmentCount = 1;
					mountedCrewUnits.Insert(pendingCommander);
					break;
				}
			}

			int commanderCompartmentIndex = 0;
			foreach (IEntity commanderUnit : aliveUnits)
			{
				if (commanderMounted)
					break;

				if (!commanderUnit || mountedCrewUnits.Contains(commanderUnit))
					continue;

				bool commanderMoveAccepted = TryMountSkirmishUnitIntoCombatCompartment(commanderUnit, mountTarget, vehiclePrefab, sideLabel, vehicleIndex, "commander", ECompartmentType.CARGO, false, commanderCompartmentIndex, commanderCompartment);
				if (commanderMoveAccepted)
				{
					commanderMounted = true;
					if (!IsSkirmishUnitInVehicle(commanderUnit, mountTarget) || IsSkirmishUnitInCompartmentType(commanderUnit, mountTarget, ECompartmentType.PILOT))
						commanderCompartment = EnsurePendingCompartmentLabel(commanderCompartment, ECompartmentType.CARGO, false);

					mountLog.CommanderCompartmentCount = 1;
					mountedCrewUnits.Insert(commanderUnit);
					break;
				}

				if (IsSkirmishUnitInVehicle(commanderUnit, mountTarget))
					TryEjectSkirmishUnitFromVehicle(commanderUnit, mountTarget, "commander_wrong_compartment");

				commanderCompartmentIndex++;
			}
		}

		foreach (IEntity pendingCleanupUnit : pendingCommanderUnits)
		{
			if (!pendingCleanupUnit || mountedCrewUnits.Contains(pendingCleanupUnit))
				continue;

			if (IsSkirmishUnitInVehicle(pendingCleanupUnit, mountTarget))
				TryEjectSkirmishUnitFromVehicle(pendingCleanupUnit, mountTarget, "unused_pending_commander");
		}

		foreach (IEntity unassignedUnit : aliveUnits)
		{
			if (unassignedUnit && !mountedCrewUnits.Contains(unassignedUnit))
				unmoved++;
		}

		if (crewSeen <= 0 || unitsSeen <= 0)
		{
			reason = sideLabel + "_vehicle_crew_not_ready";
			SetSkirmishVehicleCrewReadiness(mountLog, requiredReadyCrew, readyCrew);
			UpdateSkirmishVehicleMountLog(mountLog, crewSeen, unitsSeen, targetCrew, mountedCrewUnits.Count(), driverMounted, gunnerMounted, commanderMounted, driverCompartment, gunnerCompartment, commanderCompartment, unmoved, 0, reason);
			LogSkirmishVehicleMountAttempt(mountLog);
			return false;
		}

		if (readyCrew < requiredReadyCrew)
		{
			reason = sideLabel + "_vehicle_crew_not_ready";
			SetSkirmishVehicleCrewReadiness(mountLog, requiredReadyCrew, readyCrew);
			UpdateSkirmishVehicleMountLog(mountLog, crewSeen, unitsSeen, targetCrew, mountedCrewUnits.Count(), driverMounted, gunnerMounted, commanderMounted, driverCompartment, gunnerCompartment, commanderCompartment, unmoved, 0, reason);
			LogSkirmishVehicleMountAttempt(mountLog);
			return false;
		}

		if (!driverMounted)
		{
			reason = sideLabel + "_vehicle_no_driver";
			UpdateSkirmishVehicleMountLog(mountLog, crewSeen, unitsSeen, targetCrew, mountedCrewUnits.Count(), driverMounted, gunnerMounted, commanderMounted, driverCompartment, gunnerCompartment, commanderCompartment, unmoved, 0, reason);
			LogSkirmishVehicleMountAttempt(mountLog);
			return false;
		}

		if (requireGunner && !gunnerMounted)
		{
			reason = sideLabel + "_vehicle_no_gunner";
			if (btrStrictTurret)
				LogBtrTurretMountFailed(vehiclePrefab, sideLabel, vehicleIndex, reason, "TURRET", gunnerCompartment, null);

			UpdateSkirmishVehicleMountLog(mountLog, crewSeen, unitsSeen, targetCrew, mountedCrewUnits.Count(), driverMounted, gunnerMounted, commanderMounted, driverCompartment, gunnerCompartment, commanderCompartment, unmoved, 0, reason);
			LogSkirmishVehicleMountAttempt(mountLog);
			return false;
		}

		int deletedUnused = PruneVehicleCrewFootUnitsNow(group, vehicle, sideLabel, "mount", mountedCrewUnits);
		UpdateSkirmishVehicleMountLog(mountLog, crewSeen, unitsSeen, targetCrew, mountedCrewUnits.Count(), driverMounted, gunnerMounted, commanderMounted, driverCompartment, gunnerCompartment, commanderCompartment, unmoved, deletedUnused, failureReason);
		LogSkirmishVehicleMountAttempt(mountLog);

		if (!RegisterSkirmishUsableVehicle(group, vehicle, sideLabel, reason))
			return false;

		RecordVehicleOccupancyCommandState(record, sideLabel == "friendly", vehicleIndex, vehiclePrefab, sideLabel, driverMounted, gunnerMounted, commanderMounted, mountedCrewUnits.Count(), driverCompartment, gunnerCompartment, commanderCompartment);
		JLH_AddonSpawnUtility.ActivateGroup(group);
		return true;
	}

	protected bool TryMountBtrGunnerUsingVehiclePatrolPath(IEntity unit, IEntity vehicle, ResourceName vehiclePrefab, string sideLabel, int vehicleIndex, string seatRoleType, out string compartmentLabel)
	{
		compartmentLabel = "NONE";
		bool occupantAfterMount = false;
		bool vehiclePatrolGunnerRole = false;
		bool moveResult = JLH_DCF_VehiclePatrolSeatResolver.TryMoveGunner(unit, vehicle, compartmentLabel, occupantAfterMount, vehiclePatrolGunnerRole);
		string reason = "";
		if (!moveResult && (!unit || !vehicle))
			reason = "missing_unit_or_vehicle";
		else if (!moveResult && compartmentLabel == "NO_ACCESS")
			reason = "missing_compartment_access";
		else if (!moveResult)
			reason = "move_failed";

		LogBtrVehiclePatrolSeatPath(vehiclePrefab, sideLabel, vehicleIndex, seatRoleType, vehicle, unit, moveResult, occupantAfterMount, compartmentLabel, vehiclePatrolGunnerRole, reason);
		LogSkirmishVehicleCompartmentDebug(vehiclePrefab, sideLabel, vehicleIndex, seatRoleType, ECompartmentType.TURRET, false, 0, moveResult, occupantAfterMount, compartmentLabel, SkirmishUnitHasFireControl(unit, vehicle), reason, unit, vehicle);

		return moveResult;
	}

	protected void LogBtrVehiclePatrolSeatPath(ResourceName vehiclePrefab, string sideLabel, int vehicleIndex, string phase, IEntity vehicle, IEntity unit, bool moveResult, bool occupantAfterMount, string compartmentLabel, bool vehiclePatrolGunnerRole, string reason)
	{
		if (!IsBtrVehiclePrefab(vehiclePrefab))
			return;

		string details = "phase=" + phase;
		details = details + " side=" + sideLabel;
		details = details + " vehicleIndex=" + vehicleIndex.ToString();
		details = details + " prefab=" + vehiclePrefab;
		details = details + " unit=" + EntityDebugLabel(unit);
		details = details + " moveInVehicleTarget=" + EntityDebugLabel(vehicle);
		details = details + " requestedCompartment=TURRET";
		details = details + " source=JLH_VehiclePatrolNodeComponent.MoveInVehicle_TURRET";
		details = details + " moveInVehicleResult=" + JLH_DCF_NodeDebug.BoolLabel(moveResult);
		details = details + " occupantAfterMount=" + JLH_DCF_NodeDebug.BoolLabel(occupantAfterMount);
		details = details + " actualCompartment=" + compartmentLabel;
		details = details + " vehiclePatrolGunnerRole=" + JLH_DCF_NodeDebug.BoolLabel(vehiclePatrolGunnerRole);
		if (reason != "")
			details = details + " reason=" + reason;

		LogEvent("btr_vehicle_patrol_seat_path", details, true);
	}

	protected bool TryMountSkirmishUnitIntoCombatCompartment(IEntity unit, IEntity vehicle, ResourceName vehiclePrefab, string sideLabel, int vehicleIndex, string seatRoleType, ECompartmentType requestedType, bool useAnyCompartment, int compartmentIndex, out string compartmentLabel)
	{
		compartmentLabel = "NONE";
		bool moveResult = false;
		bool occupantAfterMount = false;
		string failureReason = "";
		if (!unit || !vehicle)
		{
			failureReason = "missing_unit_or_vehicle";
			LogSkirmishVehicleCompartmentDebug(vehiclePrefab, sideLabel, vehicleIndex, seatRoleType, requestedType, useAnyCompartment, compartmentIndex, false, false, "NONE", false, failureReason, unit, vehicle);
			return false;
		}

		SCR_CompartmentAccessComponent access = SCR_CompartmentAccessComponent.Cast(unit.FindComponent(SCR_CompartmentAccessComponent));
		if (!access)
		{
			failureReason = "missing_compartment_access";
			LogSkirmishVehicleCompartmentDebug(vehiclePrefab, sideLabel, vehicleIndex, seatRoleType, requestedType, useAnyCompartment, compartmentIndex, false, false, "NO_ACCESS", false, failureReason, unit, vehicle);
			return false;
		}

		unit.SetOrigin(vehicle.GetOrigin());
		if (useAnyCompartment)
			moveResult = access.MoveInVehicleAny(vehicle);
		else
			moveResult = access.MoveInVehicle(vehicle, requestedType);

		occupantAfterMount = IsSkirmishUnitInVehicle(unit, vehicle);
		compartmentLabel = GetSkirmishUnitCompartmentLabel(unit, vehicle);
		bool hasFireControl = SkirmishUnitHasFireControl(unit, vehicle);
		if (!moveResult)
			failureReason = "move_failed";
		else if (!occupantAfterMount)
		{
			failureReason = "not_occupant_after_mount";
			compartmentLabel = EnsurePendingCompartmentLabel(compartmentLabel, requestedType, useAnyCompartment);
		}

		LogSkirmishVehicleCompartmentDebug(vehiclePrefab, sideLabel, vehicleIndex, seatRoleType, requestedType, useAnyCompartment, compartmentIndex, moveResult, occupantAfterMount, compartmentLabel, hasFireControl, failureReason, unit, vehicle);
		return moveResult;
	}

	protected void LogSkirmishVehicleCompartmentDebug(ResourceName vehiclePrefab, string sideLabel, int vehicleIndex, string seatRoleType, ECompartmentType requestedType, bool useAnyCompartment, int compartmentIndex, bool moveResult, bool occupantAfterMount, string compartmentLabel, bool hasFireControl, string failureReason, IEntity unit, IEntity vehicle)
	{
		if (!ShouldLogVehicleDeep())
			return;

		string debugLog = "side=" + sideLabel;
		debugLog = debugLog + " vehicleIndex=" + vehicleIndex.ToString();
		debugLog = debugLog + " prefab=" + vehiclePrefab;
		debugLog = debugLog + " unit=" + EntityDebugLabel(unit);
		debugLog = debugLog + " moveTarget=" + EntityDebugLabel(vehicle);
		debugLog = debugLog + " moveTargetType=" + EntityTypeLabel(vehicle);
		debugLog = debugLog + " moveTargetRoot=" + EntityRootDebugLabel(vehicle);
		debugLog = debugLog + " moveTargetHasRpl=" + JLH_DCF_NodeDebug.BoolLabel(EntityHasRpl(vehicle));
		debugLog = debugLog + " seatRoleType=" + seatRoleType;
		debugLog = debugLog + " requestedCompartment=" + CompartmentTypeLabel(requestedType);
		debugLog = debugLog + " useAny=" + JLH_DCF_NodeDebug.BoolLabel(useAnyCompartment);
		debugLog = debugLog + " compartmentIndex=" + compartmentIndex.ToString();
		debugLog = debugLog + " moveInVehicleResult=" + JLH_DCF_NodeDebug.BoolLabel(moveResult);
		debugLog = debugLog + " occupantAfterMount=" + JLH_DCF_NodeDebug.BoolLabel(occupantAfterMount);
		debugLog = debugLog + " occupancyValidationTarget=" + EntityDebugLabel(vehicle);
		SCR_CompartmentAccessComponent access = null;
		if (unit)
			access = SCR_CompartmentAccessComponent.Cast(unit.FindComponent(SCR_CompartmentAccessComponent));

		IEntity occupancyVehicle = null;
		if (access)
			occupancyVehicle = access.GetVehicle();

		debugLog = debugLog + " occupancyVehicle=" + EntityDebugLabel(occupancyVehicle);
		debugLog = debugLog + " occupancyVehicleRoot=" + EntityRootDebugLabel(occupancyVehicle);
		debugLog = debugLog + " actualCompartment=" + compartmentLabel;
		debugLog = debugLog + " fireControl=" + JLH_DCF_NodeDebug.BoolLabel(hasFireControl);
		debugLog = debugLog + " compartmentManagerFound=unknown";
		debugLog = debugLog + " compartmentManagerClass=safe_api_unavailable";
		debugLog = debugLog + " totalRuntimeCompartments=-1";
		if (failureReason == "not_occupant_after_mount")
			debugLog = debugLog + " state=pending_occupancy";
		else if (failureReason != "")
			debugLog = debugLog + " failure=" + failureReason;

		LogEvent("ambient_skirmish_vehicle_compartment_debug", debugLog, false);
	}

	protected void LogSkirmishVehicleCompartmentValidation(ResourceName vehiclePrefab, string sideLabel, int vehicleIndex, bool driverMounted, bool gunnerMounted, int driverCompartmentCount, int turretCompartmentCount, int mountedCrew, int requiredMountedCrew, bool attackCapable, string driverCompartment, string gunnerCompartment)
	{
		if (!ShouldLogVehicleVerbose())
			return;

		string validationLog = "side=" + sideLabel;
		validationLog = validationLog + " vehicleIndex=" + vehicleIndex.ToString();
		validationLog = validationLog + " prefab=" + vehiclePrefab;
		validationLog = validationLog + " driverOccupantPresent=" + JLH_DCF_NodeDebug.BoolLabel(driverMounted);
		validationLog = validationLog + " turretOccupantPresent=" + JLH_DCF_NodeDebug.BoolLabel(gunnerMounted);
		validationLog = validationLog + " driverCompartmentCount=" + driverCompartmentCount.ToString();
		validationLog = validationLog + " turretCompartmentCount=" + turretCompartmentCount.ToString();
		validationLog = validationLog + " mountedCrew=" + mountedCrew.ToString();
		validationLog = validationLog + " requiredMountedCrew=" + requiredMountedCrew.ToString();
		validationLog = validationLog + " driverCompartment=" + driverCompartment;
		validationLog = validationLog + " gunnerCompartment=" + gunnerCompartment;
		validationLog = validationLog + " gunnerCompatible=" + JLH_DCF_NodeDebug.BoolLabel(gunnerMounted);
		validationLog = validationLog + " attackCapable=" + JLH_DCF_NodeDebug.BoolLabel(attackCapable);
		LogEvent("ambient_skirmish_vehicle_compartment_validation", validationLog, false);
	}

	protected void UpdateSkirmishVehicleMountLog(JLH_AmbientSkirmishVehicleMountLog mountLog, int crewSeen, int unitsSeen, int targetCrew, int mountedCount, bool driverMounted, bool gunnerMounted, bool commanderMounted, string driverCompartment, string gunnerCompartment, string commanderCompartment, int unmoved, int deletedUnused, string failureReason)
	{
		if (!mountLog)
			return;

		mountLog.CrewSeen = crewSeen;
		mountLog.UnitsSeen = unitsSeen;
		mountLog.TargetCrew = targetCrew;
		mountLog.MountedCount = mountedCount;
		mountLog.DriverMounted = driverMounted;
		mountLog.GunnerMounted = gunnerMounted;
		mountLog.CommanderMounted = commanderMounted;
		mountLog.DriverCompartment = driverCompartment;
		mountLog.GunnerCompartment = gunnerCompartment;
		mountLog.CommanderCompartment = commanderCompartment;
		mountLog.Unmoved = unmoved;
		mountLog.DeletedUnused = deletedUnused;
		mountLog.FailureReason = failureReason;
	}

	protected void SetSkirmishVehicleCrewReadiness(JLH_AmbientSkirmishVehicleMountLog mountLog, int expectedCrew, int readyCrew)
	{
		if (!mountLog)
			return;

		mountLog.ExpectedCrew = expectedCrew;
		mountLog.ReadyCrew = readyCrew;
	}

	protected void LogSkirmishVehicleMountAttempt(JLH_AmbientSkirmishVehicleMountLog mountData)
	{
		if (!mountData)
			return;

		string mountLog = "side=";
		mountLog = mountLog + mountData.SideLabel;
		mountLog = mountLog + " vehicleIndex=" + mountData.VehicleIndex.ToString();
		mountLog = mountLog + " prefab=" + mountData.VehiclePrefab;
		mountLog = mountLog + " vehicle=" + EntityDebugLabel(mountData.VehicleEntity);
		mountLog = mountLog + " crewGroup=" + EntityDebugLabel(mountData.CrewGroup);
		mountLog = mountLog + " mountTarget=" + EntityDebugLabel(mountData.MountTargetEntity);
		mountLog = mountLog + " agents=" + mountData.CrewSeen.ToString();
		mountLog = mountLog + " units=" + mountData.UnitsSeen.ToString();
		mountLog = mountLog + " targetCrew=" + mountData.TargetCrew.ToString();
		if (mountData.ExpectedCrew > 0)
		{
			mountLog = mountLog + " expectedCrew=" + mountData.ExpectedCrew.ToString();
			mountLog = mountLog + " readyCrew=" + mountData.ReadyCrew.ToString();
		}
		mountLog = mountLog + " mounted=" + mountData.MountedCount.ToString();
		mountLog = mountLog + " driver=" + JLH_DCF_NodeDebug.BoolLabel(mountData.DriverMounted);
		mountLog = mountLog + " driverEntity=" + EntityDebugLabel(mountData.DriverEntity);
		mountLog = mountLog + " driverCompartment=" + mountData.DriverCompartment;
		mountLog = mountLog + " driverCompartments=" + mountData.DriverCompartmentCount.ToString();
		mountLog = mountLog + " gunner=" + JLH_DCF_NodeDebug.BoolLabel(mountData.GunnerMounted);
		mountLog = mountLog + " gunnerEntity=" + EntityDebugLabel(mountData.GunnerEntity);
		mountLog = mountLog + " gunnerCompartment=" + mountData.GunnerCompartment;
		mountLog = mountLog + " turretFound=" + JLH_DCF_NodeDebug.BoolLabel(mountData.TurretFound);
		mountLog = mountLog + " turretCompartments=" + mountData.TurretCompartmentCount.ToString();
		mountLog = mountLog + " gunnerCompatible=" + JLH_DCF_NodeDebug.BoolLabel(mountData.GunnerCompatible);
		mountLog = mountLog + " commander=" + JLH_DCF_NodeDebug.BoolLabel(mountData.CommanderMounted);
		mountLog = mountLog + " commanderEntity=" + EntityDebugLabel(mountData.CommanderEntity);
		mountLog = mountLog + " commanderCompartment=" + mountData.CommanderCompartment;
		mountLog = mountLog + " commanderCompartments=" + mountData.CommanderCompartmentCount.ToString();
		mountLog = mountLog + " driverEqualsGunner=" + JLH_DCF_NodeDebug.BoolLabel(AreSameEntity(mountData.DriverEntity, mountData.GunnerEntity));
		mountLog = mountLog + " driverEqualsCommander=" + JLH_DCF_NodeDebug.BoolLabel(AreSameEntity(mountData.DriverEntity, mountData.CommanderEntity));
		mountLog = mountLog + " gunnerEqualsCommander=" + JLH_DCF_NodeDebug.BoolLabel(AreSameEntity(mountData.GunnerEntity, mountData.CommanderEntity));
		mountLog = mountLog + " unmoved=" + mountData.Unmoved.ToString();
		mountLog = mountLog + " deletedUnused=" + mountData.DeletedUnused.ToString();
		if (mountData.FailureReason != "")
			mountLog = mountLog + " failure=" + mountData.FailureReason;

		LogEvent("ambient_skirmish_vehicle_mount", mountLog, true);
		if (mountData.FailureReason.Contains("_vehicle_crew_not_ready"))
		{
			string readyLog = "side=" + mountData.SideLabel;
			readyLog = readyLog + " vehicleIndex=" + mountData.VehicleIndex.ToString();
			readyLog = readyLog + " prefab=" + mountData.VehiclePrefab;
			readyLog = readyLog + " expectedCrew=" + mountData.ExpectedCrew.ToString();
			readyLog = readyLog + " readyCrew=" + mountData.ReadyCrew.ToString();
			readyLog = readyLog + " agents=" + mountData.CrewSeen.ToString();
			readyLog = readyLog + " units=" + mountData.UnitsSeen.ToString();
			readyLog = readyLog + " reason=" + mountData.FailureReason;
			LogEvent("ambient_skirmish_vehicle_crew_not_ready", readyLog, true);
		}
	}

	protected int PruneVehicleCrewFootUnitsNow(SCR_AIGroup group, IEntity vehicle, string sideLabel, string phase, array<IEntity> preservedUnits)
	{
		if (!group || !vehicle || !JLH_AddonSpawnUtility.IsUnitAlive(vehicle))
			return 0;

		int deleted = 0;
		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!unit || !JLH_AddonSpawnUtility.IsUnitAlive(unit))
				continue;

			if (preservedUnits && preservedUnits.Contains(unit))
				continue;

			if (JLH_DCF_PlayerPresence.IsPlayerControlledEntity(unit))
				continue;

			JLH_DCF_RuntimeCleanupManager.Unregister(unit);
			JLH_AddonSpawnUtility.DeleteRuntimeEntity(unit, "ambient_skirmish_" + sideLabel + "_vehicle_" + phase + "_foot_crew");
			deleted++;
		}

		return deleted;
	}

	protected string EntityDebugLabel(IEntity entity)
	{
		if (!entity)
			return "NONE";

		string label = JLH_DCF_NodeDebug.EntityLabel(entity);
		if (label != "")
			return label;

		return entity.Type().ToString();
	}

	protected string EntityTypeLabel(IEntity entity)
	{
		if (!entity)
			return "NONE";

		return entity.Type().ToString();
	}

	protected bool AreSameEntity(IEntity first, IEntity second)
	{
		if (!first || !second)
			return false;

		return first == second;
	}

	protected string EntityRootDebugLabel(IEntity entity)
	{
		if (!entity)
			return "NONE";

		IEntity root = entity.GetRootParent();
		if (!root)
			return "NONE";

		return EntityDebugLabel(root);
	}

	protected bool EntityHasRpl(IEntity entity)
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

	protected bool IsSkirmishUnitInVehicle(IEntity unit, IEntity vehicle)
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

	protected bool IsSkirmishUnitInCompartmentType(IEntity unit, IEntity vehicle, ECompartmentType compartmentType)
	{
		if (!IsSkirmishUnitInVehicle(unit, vehicle))
			return false;

		SCR_CompartmentAccessComponent access = SCR_CompartmentAccessComponent.Cast(unit.FindComponent(SCR_CompartmentAccessComponent));
		if (!access)
			return false;

		BaseCompartmentSlot compartment = access.GetCompartment();
		if (!compartment)
			return false;

		return SCR_CompartmentAccessComponent.GetCompartmentType(compartment) == compartmentType;
	}

	protected bool TryEjectSkirmishUnitFromVehicle(IEntity unit, IEntity vehicle, string reason)
	{
		if (!IsSkirmishUnitInVehicle(unit, vehicle))
			return false;

		SCR_CompartmentAccessComponent access = SCR_CompartmentAccessComponent.Cast(unit.FindComponent(SCR_CompartmentAccessComponent));
		if (!access)
			return false;

		if (!access.CanGetOutVehicle())
			return false;

		bool ejected = access.GetOutVehicle(EGetOutType.TELEPORT, 0, ECloseDoorAfterActions.LEAVE_OPEN, true);
		LogEvent("ambient_skirmish_vehicle_compartment_eject", "reason=" + reason + " unit=" + EntityDebugLabel(unit) + " vehicle=" + EntityDebugLabel(vehicle) + " result=" + JLH_DCF_NodeDebug.BoolLabel(ejected), true);
		return ejected;
	}

	protected bool SkirmishUnitHasFireControl(IEntity unit, IEntity vehicle)
	{
		if (!IsSkirmishUnitInVehicle(unit, vehicle))
			return false;

		SCR_CompartmentAccessComponent access = SCR_CompartmentAccessComponent.Cast(unit.FindComponent(SCR_CompartmentAccessComponent));
		if (!access)
			return false;

		return access.GetControlledFireModeManager() != null;
	}

	protected string CompartmentTypeLabel(ECompartmentType compartmentType)
	{
		if (compartmentType == ECompartmentType.PILOT)
			return "PILOT";
		if (compartmentType == ECompartmentType.TURRET)
			return "TURRET";
		if (compartmentType == ECompartmentType.CARGO)
			return "CARGO";

		return "UNKNOWN";
	}

	protected bool IsPendingCompartmentLabel(string compartmentLabel)
	{
		if (compartmentLabel == "")
			return false;

		return compartmentLabel.Contains("PENDING");
	}

	protected string EnsurePendingCompartmentLabel(string compartmentLabel, ECompartmentType requestedType, bool useAnyCompartment)
	{
		if (compartmentLabel != "" && compartmentLabel != "NONE" && compartmentLabel != "UNKNOWN")
		{
			if (IsPendingCompartmentLabel(compartmentLabel))
				return compartmentLabel;

			return compartmentLabel + "_PENDING";
		}

		if (useAnyCompartment)
			return "ANY_PENDING";

		return CompartmentTypeLabel(requestedType) + "_PENDING";
	}

	protected bool IsPendingTurretCompartmentLabel(string compartmentLabel)
	{
		if (!IsPendingCompartmentLabel(compartmentLabel))
			return false;

		return compartmentLabel.Contains("TURRET");
	}

	protected string GetSkirmishUnitCompartmentLabel(IEntity unit, IEntity vehicle)
	{
		if (!IsSkirmishUnitInVehicle(unit, vehicle))
			return "NONE";

		SCR_CompartmentAccessComponent access = SCR_CompartmentAccessComponent.Cast(unit.FindComponent(SCR_CompartmentAccessComponent));
		if (!access)
			return "NO_ACCESS";

		bool hasFireControl = access.GetControlledFireModeManager() != null;
		BaseCompartmentSlot compartment = access.GetCompartment();
		if (!compartment)
		{
			if (hasFireControl)
				return "UNKNOWN_FIRE_CONTROL";

			return "UNKNOWN";
		}

		ECompartmentType compartmentType = SCR_CompartmentAccessComponent.GetCompartmentType(compartment);
		string label = "UNKNOWN";
		if (compartmentType == ECompartmentType.PILOT)
			label = "PILOT";
		else if (compartmentType == ECompartmentType.TURRET)
			label = "TURRET";
		else if (compartmentType == ECompartmentType.CARGO)
			label = "CARGO";

		if (hasFireControl)
			label = label + "_FIRE_CONTROL";

		return label;
	}

	protected bool IsSkirmishUnitInGunnerRole(IEntity unit, IEntity vehicle)
	{
		if (!IsSkirmishUnitInVehicle(unit, vehicle))
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
		}

		if (access.GetControlledFireModeManager() != null)
			return true;

		if (compartment)
		{
			ECompartmentType finalCompartmentType = SCR_CompartmentAccessComponent.GetCompartmentType(compartment);
			if (finalCompartmentType == ECompartmentType.PILOT || finalCompartmentType == ECompartmentType.CARGO)
				return false;
		}

		return false;
	}

	protected bool IsCombatSkirmishUnitInGunnerRole(IEntity unit, IEntity vehicle)
	{
		if (!IsSkirmishUnitInVehicle(unit, vehicle))
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

		return false;
	}

	protected bool RequiresVehicleGunner(notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.VEHICLE_SKIRMISH)
			return true;

		return false;
	}

	protected bool RequiresStrictVehicleSkirmishGunner(notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		return definition.SkirmishEventType == JLH_AmbientSkirmishEventType.VEHICLE_SKIRMISH;
	}

	protected bool ShouldAllowNonTurretVehicleGunnerFallback(notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		return !RequiresStrictVehicleSkirmishGunner(definition);
	}

	protected bool IsBtrVehiclePrefab(ResourceName vehiclePrefab)
	{
		if (vehiclePrefab == "")
			return false;

		string token = vehiclePrefab;
		token.ToLower();
		if (token.Contains("btr70"))
			return true;
		if (token.Contains("btr_70"))
			return true;
		if (token.Contains("btr-70"))
			return true;

		return false;
	}

	protected bool IsBtrVehiclePatrolGunnerRole(IEntity unit, IEntity vehicle)
	{
		return JLH_DCF_VehiclePatrolSeatResolver.IsGunnerRole(unit, vehicle);
	}

	protected void LogBtrTurretRequired(ResourceName vehiclePrefab, string sideLabel, int vehicleIndex, int targetCrew)
	{
		if (!IsBtrVehiclePrefab(vehiclePrefab))
			return;

		string details = "side=" + sideLabel;
		details = details + " vehicleIndex=" + vehicleIndex.ToString();
		details = details + " prefab=" + vehiclePrefab;
		details = details + " targetCrew=" + targetCrew.ToString();
		details = details + " order=PILOT,TURRET,CARGO";
		details = details + " turretMandatory=true";
		LogEvent("ambient_skirmish_btr_turret_required", details, true);
	}

	protected void LogBtrTurretMountSuccess(ResourceName vehiclePrefab, string sideLabel, int vehicleIndex, string phase, string compartment, IEntity unit, bool driverMounted, bool gunnerMounted, int mountedCrew, bool attackCapable)
	{
		if (!IsBtrVehiclePrefab(vehiclePrefab))
			return;

		if (phase != "validation_confirmed")
			return;

		string details = "phase=" + phase;
		details = details + " side=" + sideLabel;
		details = details + " vehicleIndex=" + vehicleIndex.ToString();
		details = details + " prefab=" + vehiclePrefab;
		details = details + " unit=" + EntityDebugLabel(unit);
		details = details + " compartment=" + compartment;
		int driverCount = 0;
		if (driverMounted)
			driverCount = 1;
		int gunnerCount = 0;
		if (gunnerMounted)
			gunnerCount = 1;
		details = details + " driver=" + driverCount.ToString();
		details = details + " gunner=" + gunnerCount.ToString();
		details = details + " mountedCrew=" + mountedCrew.ToString();
		details = details + " attackCapable=" + JLH_DCF_NodeDebug.BoolLabel(attackCapable);
		LogEvent("ambient_skirmish_btr_turret_mount_success", details, true);
	}

	protected void LogBtrTurretMountFailed(ResourceName vehiclePrefab, string sideLabel, int vehicleIndex, string reason, string attemptedCompartment, string actualCompartment, IEntity unit)
	{
		if (!IsBtrVehiclePrefab(vehiclePrefab))
			return;

		string details = "side=" + sideLabel;
		details = details + " vehicleIndex=" + vehicleIndex.ToString();
		details = details + " prefab=" + vehiclePrefab;
		details = details + " unit=" + EntityDebugLabel(unit);
		details = details + " attemptedCompartment=" + attemptedCompartment;
		details = details + " actualCompartment=" + actualCompartment;
		details = details + " reason=" + reason;
		LogEvent("ambient_skirmish_btr_turret_mount_failed", details, true);
	}

	protected void LogBtrInvalidNoGunner(ResourceName vehiclePrefab, string sideLabel, int vehicleIndex, int mountedCrew, bool driverMounted, string driverCompartment)
	{
		if (!IsBtrVehiclePrefab(vehiclePrefab))
			return;

		string details = "side=" + sideLabel;
		details = details + " vehicleIndex=" + vehicleIndex.ToString();
		details = details + " prefab=" + vehiclePrefab;
		details = details + " mountedCrew=" + mountedCrew.ToString();
		details = details + " driverMounted=" + JLH_DCF_NodeDebug.BoolLabel(driverMounted);
		details = details + " driverCompartment=" + driverCompartment;
		details = details + " reason=no_turret_gunner";
		LogEvent("ambient_skirmish_btr_invalid_no_gunner", details, true);
	}

	protected void LogBtrMountRoleResolution(ResourceName vehiclePrefab, string sideLabel, int vehicleIndex, string driverCompartment, string secondSeatAttempted, string secondSeatActual, bool turretFound, bool cargoRejectedAsGunner)
	{
		if (!ShouldLogVehicleDeep())
			return;

		if (!IsBtrVehiclePrefab(vehiclePrefab))
			return;

		string roleLog = "side=" + sideLabel;
		roleLog = roleLog + " vehicleIndex=" + vehicleIndex.ToString();
		roleLog = roleLog + " prefab=" + vehiclePrefab;
		roleLog = roleLog + " driverCompartment=" + driverCompartment;
		roleLog = roleLog + " secondSeatAttempted=" + secondSeatAttempted;
		roleLog = roleLog + " secondSeatActual=" + secondSeatActual;
		roleLog = roleLog + " turretFound=" + JLH_DCF_NodeDebug.BoolLabel(turretFound);
		roleLog = roleLog + " cargoRejectedAsGunner=" + JLH_DCF_NodeDebug.BoolLabel(cargoRejectedAsGunner);
		LogEvent("btr_mount_role_resolution", roleLog, false);
	}

	protected void LogBtrPostMountSeatValidation(ResourceName vehiclePrefab, string sideLabel, int vehicleIndex, IEntity unit, IEntity vehicle, bool acceptedAsTurret, string phase)
	{
		if (!ShouldLogVehicleDeep())
			return;

		if (!IsBtrVehiclePrefab(vehiclePrefab))
			return;

		string actualCompartment = GetSkirmishUnitCompartmentLabel(unit, vehicle);
		bool fireControl = SkirmishUnitHasFireControl(unit, vehicle);
		string validationLog = "phase=" + phase;
		validationLog = validationLog + " side=" + sideLabel;
		validationLog = validationLog + " vehicleIndex=" + vehicleIndex.ToString();
		validationLog = validationLog + " prefab=" + vehiclePrefab;
		validationLog = validationLog + " unit=" + EntityDebugLabel(unit);
		validationLog = validationLog + " actualCompartment=" + actualCompartment;
		validationLog = validationLog + " fireControl=" + JLH_DCF_NodeDebug.BoolLabel(fireControl);
		validationLog = validationLog + " acceptedAsTurret=" + JLH_DCF_NodeDebug.BoolLabel(acceptedAsTurret);
		LogEvent("btr_post_mount_seat_validation", validationLog, false);
	}

	protected int GetMountedVehicleCrewTarget(notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.VEHICLE_SKIRMISH)
			return VEHICLE_SKIRMISH_MAX_MOUNTED_CREW;

		return VEHICLE_SKIRMISH_MAX_MOUNTED_CREW;
	}

	protected int GetMountedVehicleCargoTarget(notnull JLH_AmbientSkirmishEventDefinition definition, int targetCrew)
	{
		int cargoTarget = targetCrew - 2;
		if (cargoTarget <= 0)
			return 0;
		if (cargoTarget > VEHICLE_SKIRMISH_MAX_CARGO_CREW)
			return VEHICLE_SKIRMISH_MAX_CARGO_CREW;

		return cargoTarget;
	}

	protected int GetRequiredVehicleReadyCrew(int targetCrew, bool requireGunner, bool allowCommander)
	{
		int requiredReadyCrew = 1;
		if (requireGunner)
			requiredReadyCrew = 2;
		if (allowCommander && targetCrew > requiredReadyCrew)
			requiredReadyCrew = targetCrew;

		return Math.Clamp(requiredReadyCrew, 1, Math.Max(1, targetCrew));
	}

	protected int GetRequiredVehicleCrewPopulationForMount(JLH_AmbientSkirmishRecord record, string sideLabel, int vehicleIndex, int targetCrew, bool requireGunner, bool allowCommander)
	{
		int requiredReadyCrew = GetRequiredVehicleReadyCrew(targetCrew, requireGunner, allowCommander);
		if (!record)
			return requiredReadyCrew;

		string source = "";
		if (sideLabel == "friendly")
			source = record.GetFriendlyGroupSource(vehicleIndex);
		else
			source = record.GetEnemyGroupSource(vehicleIndex);

		if (source.Contains("dedicated_vehicle_crew"))
		{
			JLH_AmbientSkirmishVehicleLifecycle lifecycle = record.GetVehicleLifecycle(sideLabel == "friendly", vehicleIndex);
			if (lifecycle && lifecycle.FullCrewPopulationConfirmed)
				return requiredReadyCrew;

			return Math.Max(requiredReadyCrew, FULL_VEHICLE_CREW_GROUP_EXPECTED_UNITS);
		}

		return requiredReadyCrew;
	}

	protected int GetRequiredVehicleMountedCrewForValidation(notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.VEHICLE_SKIRMISH)
			return VEHICLE_SKIRMISH_MAX_MOUNTED_CREW;

		if (RequiresVehicleGunner(definition))
			return 2;

		return 1;
	}

	protected bool RegisterSkirmishUsableVehicle(SCR_AIGroup group, IEntity vehicle, string sideLabel, out string reason)
	{
		reason = "";
		if (!group || !vehicle)
		{
			reason = sideLabel + "_vehicle_usage_missing_group_or_vehicle";
			return false;
		}

		SCR_AIGroupUtilityComponent utility = group.GetGroupUtilityComponent();
		if (!utility)
		{
			reason = sideLabel + "_vehicle_usage_missing_group_utility";
			return false;
		}

		IEntity usageOwner = null;
		SCR_AIVehicleUsageComponent vehicleUsage = SCR_AIVehicleUsageComponent.FindOnNearestParent(vehicle, usageOwner);
		if (!vehicleUsage || !vehicleUsage.CanBePiloted())
		{
			reason = sideLabel + "_vehicle_not_pilotable";
			return false;
		}

		utility.SetMaxAutonomousDistance(4500.0);
		utility.AddUsableVehicle(vehicleUsage);
		LogEvent("ambient_skirmish_vehicle_usage_registered", "side=" + sideLabel + " vehicle=" + EntityDebugLabel(vehicle) + " usageOwner=" + EntityDebugLabel(usageOwner) + " usageOwnerType=" + EntityTypeLabel(usageOwner), true);
		return true;
	}

	protected void ActivateSkirmishVehicleGroupAI(SCR_AIGroup group)
	{
		if (!group)
			return;

		group.ActivateAI();
		//group.ActivateAllMembers();

		AIControlComponent control = group.GetControlComponent();
		if (control)
			control.ActivateAI();
	}

	protected bool TryBuildDedicatedMountedVehiclePlan(string factionKey, JLH_DCF_AmbientGroupRole supportRole, int vehicleCount, int supportGroups, int mountedCrewCount, bool enemySide, out JLH_AmbientSkirmishGroupPlan plan, out string reason)
	{
		plan = null;
		reason = "";

		int mountedGroupCount = Math.Clamp(vehicleCount, 0, 3);
		int supportGroupCount = Math.Clamp(supportGroups, 0, 2);
		if (mountedGroupCount <= 0)
		{
			reason = "dedicated_vehicle_crew_unavailable_no_vehicle";
			return false;
		}

		ResourceName crewPrefab = ResolveDedicatedVehicleCrewGroupPrefab(factionKey);
		if (crewPrefab == "")
		{
			reason = "dedicated_vehicle_crew_unavailable";
			return false;
		}

		plan = new JLH_AmbientSkirmishGroupPlan();
		plan.Init(factionKey, JLH_DCF_AmbientGroupRole.VEHICLE_CREW, 0, "dedicated_vehicle_crew:" + mountedGroupCount.ToString());
		for (int i = 0; i < mountedGroupCount; i++)
		{
			AddDedicatedMountedVehicleCrewSelection(plan, crewPrefab, factionKey);
		}

		if (supportGroupCount > 0)
		{
			JLH_AmbientSkirmishGroupPlan supportPlan;
			string supportReason;
			bool supportSelected;
			if (enemySide)
				supportSelected = JLH_AmbientSkirmishGroupComposer.TryBuildEnemyMultiSinglePlan(m_sFriendlyFactionKey, factionKey, supportRole, supportGroupCount, supportGroupCount, 4, 8, supportPlan, supportReason);
			else
				supportSelected = JLH_AmbientSkirmishGroupComposer.TryBuildFriendlyMultiSinglePlan(factionKey, supportRole, supportGroupCount, supportGroupCount, 4, 8, supportPlan, supportReason);

			if (!supportSelected)
			{
				plan = null;
				reason = "dedicated_vehicle_support_group_failed";
				if (supportReason != "")
					reason = reason + "_" + supportReason;

				return false;
			}

			plan.AppendPlan(supportPlan);
		}

		plan.TargetUnits = plan.TotalUnits;
		return !plan.IsEmpty();
	}

	protected void AddDedicatedMountedVehicleCrewSelection(notnull JLH_AmbientSkirmishGroupPlan plan, ResourceName crewPrefab, string factionKey)
	{
		JLH_DCF_AmbientGroupSelection selection = new JLH_DCF_AmbientGroupSelection();
		selection.Init(crewPrefab, factionKey, "dedicated_vehicle_crew", JLH_DCF_AmbientGroupRole.VEHICLE_CREW, FULL_VEHICLE_CREW_GROUP_EXPECTED_UNITS, false, false);
		plan.AddSelection(selection);
	}

	protected ResourceName ResolveDedicatedVehicleCrewGroupPrefab(string factionKey)
	{
		ResourceName prefab = ResolveKnownDedicatedVehicleCrewGroupPrefab(factionKey);
		if (prefab != "")
		{
			LogEvent("dedicated_vehicle_crew_prefab_audit", "faction=" + factionKey + " prefab=" + prefab + " stableCrew=true source=framework_full_vehicle_group expectedUnits=" + FULL_VEHICLE_CREW_GROUP_EXPECTED_UNITS.ToString(), true);
			return prefab;
		}

		return ResourceName.Empty;
	}

	protected ResourceName ResolveKnownDedicatedVehicleCrewGroupPrefab(string factionKey)
	{
		string normalizedFaction = factionKey;
		normalizedFaction.ToUpper();

		if (normalizedFaction == "USSR" || normalizedFaction == "OPFOR")
			return DEFAULT_USSR_VEHICLE_CREW_GROUP_PREFAB;

		if (normalizedFaction == "US" || normalizedFaction == "BLUFOR")
			return DEFAULT_US_VEHICLE_CREW_GROUP_PREFAB;

		return ResourceName.Empty;
	}

	protected void LogVehicleGroupPlanSelected(string sideLabel, notnull JLH_AmbientSkirmishGroupPlan plan, int vehicleCount, int supportGroups)
	{
		string planLog = "faction=" + plan.PlanFactionKey;
		planLog = planLog + " vehicles=" + vehicleCount.ToString();
		planLog = planLog + " supportGroups=" + supportGroups.ToString();
		planLog = planLog + " groups=" + plan.GroupCount().ToString();
		planLog = planLog + " units=" + plan.TotalUnits.ToString();
		planLog = planLog + " source=" + plan.Source;
		LogEvent("ambient_skirmish_" + sideLabel + "_vehicle_groups_selected", planLog, true);
	}

	bool SelectVehicleFriendlyPlan(notnull JLH_AmbientSkirmishEventDefinition definition, int vehicleCount, int supportGroups, out JLH_AmbientSkirmishGroupPlan plan, out string reason)
	{
		plan = null;
		reason = "";

		if (ShouldUseMountedVehicleCrewPlan(definition, vehicleCount, supportGroups))
		{
			int mountedCrewCount = GetMountedVehicleCrewTarget(definition);
			string dedicatedReason;
			if (TryBuildDedicatedMountedVehiclePlan(m_sFriendlyFactionKey, definition.FriendlyRole, vehicleCount, supportGroups, mountedCrewCount, false, plan, dedicatedReason))
			{
				LogVehicleGroupPlanSelected("friendly", plan, vehicleCount, supportGroups);
				return true;
			}

			if (dedicatedReason != "" && !dedicatedReason.Contains("dedicated_vehicle_crew_unavailable"))
			{
				reason = dedicatedReason;
				return false;
			}

			LogEvent("ambient_skirmish_vehicle_dedicated_crew_fallback", "side=friendly faction=" + m_sFriendlyFactionKey + " reason=" + dedicatedReason, true);
			int groupCount = Math.Clamp(vehicleCount + supportGroups, 1, 8);
			int maxVehicleGroupUnits = mountedCrewCount + 1;
			if (JLH_AmbientSkirmishGroupComposer.TryBuildFriendlyMultiSinglePlan(m_sFriendlyFactionKey, definition.FriendlyRole, groupCount, groupCount, mountedCrewCount, maxVehicleGroupUnits, plan, reason))
			{
				LogVehicleGroupPlanSelected("friendly", plan, vehicleCount, supportGroups);
				return true;
			}

			if (reason == "")
				reason = "friendly_vehicle_group_composition_failed";

			return false;
		}

		return SelectFriendlyPlan(definition, plan, reason);
	}

	bool SelectVehicleEnemyPlan(notnull JLH_AmbientSkirmishEventDefinition definition, int vehicleCount, int supportGroups, out JLH_AmbientSkirmishGroupPlan plan, out string reason)
	{
		plan = null;
		reason = "";

		if (ShouldUseMountedVehicleCrewPlan(definition, vehicleCount, supportGroups))
		{
			string enemyFactionKey = JLH_DCF_FactionCompatibilityRegistry.ResolveEnemyFactionKey(m_sFriendlyFactionKey, m_sEnemyFactionKey);
			int mountedCrewCount = GetMountedVehicleCrewTarget(definition);
			string dedicatedReason;
			if (TryBuildDedicatedMountedVehiclePlan(enemyFactionKey, definition.EnemyRole, vehicleCount, supportGroups, mountedCrewCount, true, plan, dedicatedReason))
			{
				LogVehicleGroupPlanSelected("enemy", plan, vehicleCount, supportGroups);
				return true;
			}

			if (dedicatedReason != "" && !dedicatedReason.Contains("dedicated_vehicle_crew_unavailable"))
			{
				reason = dedicatedReason;
				return false;
			}

			LogEvent("ambient_skirmish_vehicle_dedicated_crew_fallback", "side=enemy faction=" + enemyFactionKey + " reason=" + dedicatedReason, true);
			int groupCount = Math.Clamp(vehicleCount + supportGroups, 1, 8);
			int maxVehicleGroupUnits = mountedCrewCount + 1;
			if (JLH_AmbientSkirmishGroupComposer.TryBuildEnemyMultiSinglePlan(m_sFriendlyFactionKey, m_sEnemyFactionKey, definition.EnemyRole, groupCount, groupCount, mountedCrewCount, maxVehicleGroupUnits, plan, reason))
			{
				LogVehicleGroupPlanSelected("enemy", plan, vehicleCount, supportGroups);
				return true;
			}

			if (reason == "")
				reason = "enemy_vehicle_group_composition_failed";

			return false;
		}

		return SelectEnemyPlan(definition, plan, reason);
	}

	protected bool SelectFriendlyPlan(notnull JLH_AmbientSkirmishEventDefinition definition, out JLH_AmbientSkirmishGroupPlan plan, out string reason)
	{
		plan = null;
		reason = "";

		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.INFANTRY_CLASH)
			return SelectInfantryClashPlan(definition, false, plan, reason);

		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.RECON_TEAM_CONTACT)
			return SelectSplitReconPlan(definition, false, plan, reason);

		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.SMALL_PATROL_CONTACT)
			return SelectSmallPatrolPlan(definition, false, plan, reason);

		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.CAMP_ASSAULT)
			return SelectCampAssaultPlan(definition, false, plan, reason);

		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.ROADBLOCK_BREAKTHROUGH)
			return SelectRoadblockBreakthroughPlan(definition, false, plan, reason);

		if (ShouldUseCrashSurvivorLayout(definition))
		{
			if (JLH_AmbientSkirmishGroupComposer.TryBuildFriendlySinglePlan(m_sFriendlyFactionKey, definition.FriendlyRole, definition.FriendlyMinUnits, definition.FriendlyMaxUnits, plan, reason))
			{
				LogEvent("ambient_skirmish_friendly_single_group_selected", "faction=" + plan.PlanFactionKey + " groups=" + plan.GroupCount().ToString() + " units=" + plan.TotalUnits.ToString() + " source=" + plan.Source, true);
				return true;
			}

			if (reason == "")
				reason = "friendly_single_group_failed";

			return false;
		}

		if (JLH_AmbientSkirmishGroupComposer.TryBuildFriendlyPlan(m_sFriendlyFactionKey, definition.FriendlyRole, definition.FriendlyMinUnits, definition.FriendlyMaxUnits, plan, reason))
		{
			LogEvent("ambient_skirmish_friendly_composition_selected", "faction=" + plan.PlanFactionKey + " groups=" + plan.GroupCount().ToString() + " units=" + plan.TotalUnits.ToString() + " source=" + plan.Source, false);
			return true;
		}

		if (reason == "")
			reason = "friendly_group_composition_failed";

		return false;
	}

	protected bool SelectEnemyPlan(notnull JLH_AmbientSkirmishEventDefinition definition, out JLH_AmbientSkirmishGroupPlan plan, out string reason)
	{
		plan = null;
		reason = "";

		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.INFANTRY_CLASH)
			return SelectInfantryClashPlan(definition, true, plan, reason);

		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.RECON_TEAM_CONTACT)
			return SelectSplitReconPlan(definition, true, plan, reason);

		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.SMALL_PATROL_CONTACT)
			return SelectSmallPatrolPlan(definition, true, plan, reason);

		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.CAMP_ASSAULT)
			return SelectCampAssaultPlan(definition, true, plan, reason);

		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.ROADBLOCK_BREAKTHROUGH)
			return SelectRoadblockBreakthroughPlan(definition, true, plan, reason);

		if (ShouldUseCrashSurvivorLayout(definition))
		{
			JLH_DCF_AmbientGroupRole crashEnemyRole = definition.EnemyRole;
			if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.DOWNED_CREW_RESCUE)
				crashEnemyRole = JLH_DCF_AmbientGroupRole.FOREST_AMBUSH;

			int hunterGroups = 2;
			int hunterMin = 4;
			int hunterMax = 4;
			if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.DOWNED_US_HELI_RESCUE)
			{
				hunterGroups = 3;
				hunterMin = 4;
				hunterMax = 6;
			}

			if (JLH_AmbientSkirmishGroupComposer.TryBuildEnemyMultiSinglePlan(m_sFriendlyFactionKey, m_sEnemyFactionKey, crashEnemyRole, hunterGroups, hunterGroups, hunterMin, hunterMax, plan, reason))
			{
				LogEvent("ambient_skirmish_enemy_multi_group_selected", "event=" + definition.SkirmishEventName + " rescue_profile=" + definition.RescueProfile + " faction=" + plan.PlanFactionKey + " groups=" + plan.GroupCount().ToString() + " units=" + plan.TotalUnits.ToString() + " source=" + plan.Source, true);
				return true;
			}

			if (reason == "")
				reason = "enemy_multi_group_failed";

			return false;
		}

		if (JLH_AmbientSkirmishGroupComposer.TryBuildEnemyPlan(m_sFriendlyFactionKey, m_sEnemyFactionKey, definition.EnemyRole, definition.EnemyMinUnits, definition.EnemyMaxUnits, plan, reason))
		{
			LogEvent("ambient_skirmish_enemy_composition_selected", "faction=" + plan.PlanFactionKey + " groups=" + plan.GroupCount().ToString() + " units=" + plan.TotalUnits.ToString() + " source=" + plan.Source, false);
			return true;
		}

		if (reason == "")
			reason = "enemy_group_composition_failed";

		return false;
	}

	protected bool SelectInfantryClashPlan(notnull JLH_AmbientSkirmishEventDefinition definition, bool enemySide, out JLH_AmbientSkirmishGroupPlan plan, out string reason)
	{
		plan = null;
		reason = "";

		bool selected;
		if (enemySide)
			selected = JLH_AmbientSkirmishGroupComposer.TryBuildEnemyInfantryClashPlan(m_sFriendlyFactionKey, m_sEnemyFactionKey, definition.EnemyMinUnits, definition.EnemyMaxUnits, plan, reason);
		else
			selected = JLH_AmbientSkirmishGroupComposer.TryBuildFriendlyInfantryClashPlan(m_sFriendlyFactionKey, definition.FriendlyMinUnits, definition.FriendlyMaxUnits, plan, reason);

		if (selected)
		{
			LogScenarioPlanSelected(definition, enemySide, plan, "infantry_clash_mixed_groups");
			return true;
		}

		if (reason == "")
			reason = "infantry_clash_plan_failed";

		return false;
	}

	protected bool SelectSmallPatrolPlan(notnull JLH_AmbientSkirmishEventDefinition definition, bool enemySide, out JLH_AmbientSkirmishGroupPlan plan, out string reason)
	{
		plan = null;
		reason = "";

		bool selected;
		if (enemySide)
			selected = JLH_AmbientSkirmishGroupComposer.TryBuildEnemySinglePlan(m_sFriendlyFactionKey, m_sEnemyFactionKey, definition.EnemyRole, definition.EnemyMinUnits, definition.EnemyMaxUnits, plan, reason);
		else
			selected = JLH_AmbientSkirmishGroupComposer.TryBuildFriendlySinglePlan(m_sFriendlyFactionKey, definition.FriendlyRole, definition.FriendlyMinUnits, definition.FriendlyMaxUnits, plan, reason);

		if (selected)
		{
			LogScenarioPlanSelected(definition, enemySide, plan, "small_patrol_single");
			return true;
		}

		if (reason == "")
			reason = "small_patrol_plan_failed";

		return false;
	}

	protected bool SelectSplitReconPlan(notnull JLH_AmbientSkirmishEventDefinition definition, bool enemySide, out JLH_AmbientSkirmishGroupPlan plan, out string reason)
	{
		plan = null;
		reason = "";

		bool selected;
		if (enemySide)
			selected = JLH_AmbientSkirmishGroupComposer.TryBuildEnemyMultiSinglePlan(m_sFriendlyFactionKey, m_sEnemyFactionKey, definition.EnemyRole, 1, 2, 2, 4, plan, reason);
		else
			selected = JLH_AmbientSkirmishGroupComposer.TryBuildFriendlyMultiSinglePlan(m_sFriendlyFactionKey, definition.FriendlyRole, 1, 2, 2, 4, plan, reason);

		if (selected)
		{
			LogScenarioPlanSelected(definition, enemySide, plan, "split_recon_elements");
			return true;
		}

		if (reason == "")
			reason = "split_recon_plan_failed";

		return false;
	}

	protected bool SelectCampAssaultPlan(notnull JLH_AmbientSkirmishEventDefinition definition, bool enemySide, out JLH_AmbientSkirmishGroupPlan plan, out string reason)
	{
		plan = null;
		reason = "";

		JLH_AmbientSkirmishGroupPlan friendlyPlan;
		JLH_AmbientSkirmishGroupPlan enemyPlan;
		if (!BuildCampAssaultExactPlans(friendlyPlan, enemyPlan, reason))
			return false;

		if (enemySide)
			plan = enemyPlan;
		else
			plan = friendlyPlan;

		LogScenarioPlanSelected(definition, enemySide, plan, "camp_fortified_assault");
		return true;
	}

	protected bool SelectRoadblockBreakthroughPlan(notnull JLH_AmbientSkirmishEventDefinition definition, bool enemySide, out JLH_AmbientSkirmishGroupPlan plan, out string reason)
	{
		plan = null;
		reason = "";

		JLH_AmbientSkirmishGroupPlan friendlyPlan;
		JLH_AmbientSkirmishGroupPlan enemyPlan;
		if (!BuildRoadblockBreakthroughExactPlans(friendlyPlan, enemyPlan, reason))
			return false;

		if (enemySide)
			plan = enemyPlan;
		else
			plan = friendlyPlan;

		LogScenarioPlanSelected(definition, enemySide, plan, "roadblock_breakthrough_exact_checkpoint");
		return true;
	}

	protected void AppendOptionalSupportPlan(notnull JLH_AmbientSkirmishEventDefinition definition, bool enemySide, JLH_DCF_AmbientGroupRole supportRole, string supportLabel, notnull JLH_AmbientSkirmishGroupPlan plan)
	{
		JLH_AmbientSkirmishGroupPlan supportPlan;
		string supportReason;
		bool supportSelected;
		if (enemySide)
			supportSelected = JLH_AmbientSkirmishGroupComposer.TryBuildEnemySinglePlan(m_sFriendlyFactionKey, m_sEnemyFactionKey, supportRole, 3, 5, supportPlan, supportReason);
		else
			supportSelected = JLH_AmbientSkirmishGroupComposer.TryBuildFriendlySinglePlan(m_sFriendlyFactionKey, supportRole, 3, 5, supportPlan, supportReason);

		if (supportSelected)
		{
			plan.AppendPlan(supportPlan);
			return;
		}

		if (supportReason != "")
			LogEvent("ambient_skirmish_support_composition_skipped", "event=" + definition.SkirmishEventName + " side=" + SideLabel(enemySide) + " support=" + supportLabel + " reason=" + supportReason, false);
	}

	protected void LogScenarioPlanSelected(notnull JLH_AmbientSkirmishEventDefinition definition, bool enemySide, notnull JLH_AmbientSkirmishGroupPlan plan, string compositionType)
	{
		string details = "event=" + definition.SkirmishEventName;
		details = details + " side=" + SideLabel(enemySide);
		details = details + " composition_type=" + compositionType;
		details = details + " groups=" + plan.GroupCount().ToString();
		details = details + " units=" + plan.TotalUnits.ToString();
		details = details + " source=" + plan.Source;
		LogEvent("ambient_skirmish_composition_type", details, true);
	}

	protected string SideLabel(bool enemySide)
	{
		if (enemySide)
			return "enemy";

		return "friendly";
	}

	bool SpawnGroupPlan(notnull JLH_AmbientSkirmishGroupPlan plan, vector basePosition, string sideLabel, notnull array<IEntity> exactMarkers, notnull array<SCR_AIGroup> outGroups, out string reason)
	{
		return SpawnGroupPlanWithOffset(plan, basePosition, sideLabel, exactMarkers, outGroups, reason, 0.0);
	}

	bool SpawnGroupPlanWithOffset(notnull JLH_AmbientSkirmishGroupPlan plan, vector basePosition, string sideLabel, notnull array<IEntity> exactMarkers, notnull array<SCR_AIGroup> outGroups, out string reason, float groupOffsetSpacing)
	{
		outGroups.Clear();
		reason = "";

		if (!plan || plan.IsEmpty())
		{
			reason = sideLabel + "_plan_empty";
			return false;
		}

		int markerCursor = 0;
		if (!exactMarkers.IsEmpty() && exactMarkers.Count() < plan.TotalUnits)
			LogEvent("ambient_skirmish_child_marker_shortage", "side=" + sideLabel + " requestedUnits=" + plan.TotalUnits.ToString() + " exactMarkers=" + exactMarkers.Count().ToString() + " fallbackUnits=" + (plan.TotalUnits - exactMarkers.Count()).ToString(), true);

		for (int i = 0; i < plan.Groups.Count(); i++)
		{
			JLH_DCF_EnemyGroupSelection selection = plan.Groups[i];
			if (!selection || selection.Prefab == "")
			{
				CleanupSpawnedGroups(outGroups, "ambient_skirmish_" + sideLabel + "_partial_cleanup_invalid_selection");
				reason = sideLabel + "_selection_invalid";
				return false;
			}

			vector groupPosition = BuildComposedGroupPosition(basePosition, i, plan.Groups.Count());
			if (groupOffsetSpacing > 0.0)
				groupPosition = BuildComposedGroupPositionWithSpacing(basePosition, i, plan.Groups.Count(), groupOffsetSpacing);
			SCR_AIGroup group;
			if (markerCursor < exactMarkers.Count() && exactMarkers[markerCursor])
				group = JLH_AddonSpawnUtility.SpawnGroupAtExactTransform(selection.Prefab, exactMarkers[markerCursor], "ambient_skirmish_" + sideLabel + "_group_spawn_exact_marker_" + i.ToString(), selection.EstimatedUnits);
			else
				group = JLH_AddonSpawnUtility.SpawnGroup(selection.Prefab, groupPosition, "ambient_skirmish_" + sideLabel + "_group_spawn_" + i.ToString(), selection.EstimatedUnits);

			if (!group)
			{
				CleanupSpawnedGroups(outGroups, "ambient_skirmish_" + sideLabel + "_partial_cleanup_spawn_failed");
				reason = sideLabel + "_spawn_failed";
				return false;
			}

			if (IsDedicatedVehicleCrewSelection(selection))
			{
				bool reusedBeforeInsert = outGroups.Contains(group);
				LogCrewGroupCreated(group, selection, sideLabel, i, plan.Groups.Count(), reusedBeforeInsert);
				if (reusedBeforeInsert)
					LogCrewGroupReusedDetected(0, sideLabel, group, null, outGroups.Find(group), i, i, plan.Groups.Count(), "group_spawn", "spawned_group_instance_already_seen");
			}

			if (!exactMarkers.IsEmpty())
			{
				int unitsSeen;
				int exactUnitsPlaced;
				int markersUsed;
				JLH_AddonSpawnUtility.ApplyExactUnitMarkerTransforms(group, exactMarkers, markerCursor, groupPosition, unitsSeen, exactUnitsPlaced, markersUsed, "ambient_skirmish_" + sideLabel + "_exact_marker_units_" + i.ToString());
				markerCursor += markersUsed;
				if (exactUnitsPlaced < unitsSeen)
					LogEvent("ambient_skirmish_child_marker_runtime_shortage", "side=" + sideLabel + " groupIndex=" + i.ToString() + " units=" + unitsSeen.ToString() + " exactUnits=" + exactUnitsPlaced.ToString() + " exactMarkers=" + exactMarkers.Count().ToString() + " fallbackUnits=" + (unitsSeen - exactUnitsPlaced).ToString(), true);
			}

			outGroups.Insert(group);
		}

		return !outGroups.IsEmpty();
	}

	protected SCR_AIGroup SpawnRoadblockAssaultGroup(ResourceName prefab, vector position, string label, notnull array<SCR_AIGroup> outGroups, out string reason, int requestedMembers = -1)
	{
		reason = "";
		if (prefab == "" || !JLH_AddonSpawnUtility.CanLoadPrefab(prefab))
		{
			reason = "roadblock_assault_group_prefab_missing_" + label;
			return null;
		}

		SCR_AIGroup group = JLH_AddonSpawnUtility.SpawnGroup(prefab, position, "roadblock_assault_" + label, requestedMembers);
		if (!group)
		{
			reason = "roadblock_assault_group_spawn_failed_" + label;
			return null;
		}

		outGroups.Insert(group);
		return group;
	}

	protected bool SpawnRoadblockBreakthroughGroupPlan(notnull JLH_AmbientSkirmishGroupPlan plan, vector primaryPosition, vector secondaryPosition, string sideLabel, notnull array<SCR_AIGroup> outGroups, out string reason)
	{
		outGroups.Clear();
		reason = "";

		if (!plan || plan.IsEmpty())
		{
			reason = "roadblock_breakthrough_" + sideLabel + "_plan_empty";
			return false;
		}

		vector center = JLH_AddonWorldQuery.GroundPosition((primaryPosition + secondaryPosition) * 0.5);
		for (int i = 0; i < plan.Groups.Count(); i++)
		{
			JLH_DCF_EnemyGroupSelection selection = plan.Groups[i];
			if (!selection || selection.Prefab == "")
			{
				CleanupSpawnedGroups(outGroups, "roadblock_breakthrough_" + sideLabel + "_partial_cleanup_invalid_selection");
				reason = "roadblock_breakthrough_" + sideLabel + "_selection_invalid";
				return false;
			}

			vector spawnPosition = primaryPosition;
			if (i == 1)
				spawnPosition = secondaryPosition;
			else if (i > 1)
				spawnPosition = BuildComposedGroupPosition(center, i, plan.Groups.Count());

			spawnPosition = JLH_AddonWorldQuery.GroundPosition(spawnPosition);
			SCR_AIGroup group = JLH_AddonSpawnUtility.SpawnGroup(selection.Prefab, spawnPosition, "roadblock_breakthrough_" + sideLabel + "_group_" + i.ToString(), selection.EstimatedUnits);
			if (!group)
			{
				CleanupSpawnedGroups(outGroups, "roadblock_breakthrough_" + sideLabel + "_partial_cleanup_spawn_failed");
				reason = "roadblock_breakthrough_" + sideLabel + "_spawn_failed";
				return false;
			}

			outGroups.Insert(group);
			LogEvent("roadblock_breakthrough_group_spawned", "side=" + sideLabel + " group=" + i.ToString() + " prefab=" + selection.Prefab + " pos=" + spawnPosition.ToString() + " estimatedUnits=" + selection.EstimatedUnits.ToString(), true);
		}

		return !outGroups.IsEmpty();
	}

	protected void BuildRoadblockBreakthroughDefenderPositions(vector roadblockPosition, vector roadDirection, out vector primaryPosition, out vector secondaryPosition)
	{
		vector direction = roadDirection;
		if (vector.Distance(direction, "0 0 0") <= 0.001)
			direction = "0 0 1";

		vector lateral = Vector(-direction[2], 0.0, direction[0]);
		primaryPosition = JLH_AddonWorldQuery.GroundPosition(roadblockPosition - (direction * 4.0) + (lateral * 4.0));
		secondaryPosition = JLH_AddonWorldQuery.GroundPosition(roadblockPosition + (direction * 4.0) - (lateral * 4.0));
	}

	protected bool SpawnRoadblockBreakthroughFriendlyGroupPlan(notnull JLH_AmbientSkirmishGroupPlan plan, vector attackerPosition, vector roadblockPosition, notnull array<SCR_AIGroup> outGroups, out string reason)
	{
		outGroups.Clear();
		reason = "";

		if (!plan || plan.IsEmpty())
		{
			reason = "roadblock_breakthrough_friendly_plan_empty";
			return false;
		}

		vector approachDirection = DirectionFromTo(attackerPosition, roadblockPosition);
		if (vector.Distance(approachDirection, "0 0 0") <= 0.001)
			approachDirection = "0 0 1";

		vector lateral = Vector(-approachDirection[2], 0.0, approachDirection[0]);
		for (int i = 0; i < plan.Groups.Count(); i++)
		{
			JLH_DCF_EnemyGroupSelection selection = plan.Groups[i];
			if (!selection || selection.Prefab == "")
			{
				CleanupSpawnedGroups(outGroups, "roadblock_breakthrough_friendly_partial_cleanup_invalid_selection");
				reason = "roadblock_breakthrough_friendly_selection_invalid";
				return false;
			}

			float lineOffset = BuildInfantryClashGroupLineOffset(i, plan.Groups.Count(), 18.0);
			float depthOffset = 0.0;
			if (i > 0)
				depthOffset = -10.0;

			vector spawnPosition = JLH_AddonWorldQuery.GroundPosition(attackerPosition + (lateral * lineOffset) + (approachDirection * depthOffset));
			SCR_AIGroup group = JLH_AddonSpawnUtility.SpawnGroup(selection.Prefab, spawnPosition, "roadblock_breakthrough_us_push_group_" + i.ToString(), selection.EstimatedUnits);
			if (!group)
			{
				CleanupSpawnedGroups(outGroups, "roadblock_breakthrough_friendly_partial_cleanup_spawn_failed");
				reason = "roadblock_breakthrough_friendly_spawn_failed";
				return false;
			}

			outGroups.Insert(group);
			LogEvent("roadblock_breakthrough_group_spawned", "side=friendly group=" + i.ToString() + " prefab=" + selection.Prefab + " pos=" + spawnPosition.ToString() + " estimatedUnits=" + selection.EstimatedUnits.ToString(), true);
		}

		return !outGroups.IsEmpty();
	}

	protected int CountSkirmishGroupUnits(notnull array<SCR_AIGroup> groups)
	{
		int count = 0;
		foreach (SCR_AIGroup group : groups)
		{
			if (group)
				count += JLH_AddonSpawnUtility.CountAliveGroupUnits(group);
		}

		return count;
	}

	protected bool BuildRoadblockBreakthroughExactPlans(out JLH_AmbientSkirmishGroupPlan friendlyPlan, out JLH_AmbientSkirmishGroupPlan enemyPlan, out string reason)
	{
		reason = "";
		friendlyPlan = null;
		enemyPlan = null;

		if (!JLH_AddonSpawnUtility.CanLoadPrefab(ROADBLOCK_ASSAULT_GROUP_US))
		{
			reason = "roadblock_breakthrough_us_fireteam_prefab_missing";
			return false;
		}

		if (!JLH_AddonSpawnUtility.CanLoadPrefab(ROADBLOCK_ASSAULT_GROUP_USSR_AT))
		{
			reason = "roadblock_breakthrough_ussr_at_prefab_missing";
			return false;
		}

		if (!JLH_AddonSpawnUtility.CanLoadPrefab(CAMP_ASSAULT_GROUP_USSR_FIRETEAM_4))
		{
			reason = "roadblock_breakthrough_ussr_fireteam_prefab_missing";
			return false;
		}

		friendlyPlan = new JLH_AmbientSkirmishGroupPlan();
		friendlyPlan.Init("US", JLH_DCF_AmbientGroupRole.FIRETEAM_4, 8, "roadblock_breakthrough_exact_checkpoint");
		AppendRoadblockAssaultPlanSelection(friendlyPlan, ROADBLOCK_ASSAULT_GROUP_US, "US", "roadblock_breakthrough_exact:us_fireteam_0", JLH_DCF_AmbientGroupRole.FIRETEAM_4, 4);
		AppendRoadblockAssaultPlanSelection(friendlyPlan, ROADBLOCK_ASSAULT_GROUP_US, "US", "roadblock_breakthrough_exact:us_fireteam_1", JLH_DCF_AmbientGroupRole.FIRETEAM_4, 4);

		enemyPlan = new JLH_AmbientSkirmishGroupPlan();
		enemyPlan.Init("USSR", JLH_DCF_AmbientGroupRole.ROADBLOCK_GUARD, 6, "roadblock_breakthrough_exact_checkpoint");
		AppendRoadblockAssaultPlanSelection(enemyPlan, ROADBLOCK_ASSAULT_GROUP_USSR_AT, "USSR", "roadblock_breakthrough_exact:ussr_at_team", JLH_DCF_AmbientGroupRole.AT_TEAM, 2);
		AppendRoadblockAssaultPlanSelection(enemyPlan, CAMP_ASSAULT_GROUP_USSR_FIRETEAM_4, "USSR", "roadblock_breakthrough_exact:ussr_fireteam", JLH_DCF_AmbientGroupRole.FIRETEAM_4, 4);

		return true;
	}

	protected void BuildRoadblockAssaultExactPlans(out JLH_AmbientSkirmishGroupPlan friendlyPlan, out JLH_AmbientSkirmishGroupPlan enemyPlan)
	{
		friendlyPlan = new JLH_AmbientSkirmishGroupPlan();
		friendlyPlan.Init("US", JLH_DCF_AmbientGroupRole.FIRETEAM_4, 4, "roadblock_assault_exact_vanilla");
		AppendRoadblockAssaultPlanSelection(friendlyPlan, ROADBLOCK_ASSAULT_GROUP_US, "US", "vanilla_exact:us_fireteam", JLH_DCF_AmbientGroupRole.FIRETEAM_4, 4);

		enemyPlan = new JLH_AmbientSkirmishGroupPlan();
		enemyPlan.Init("USSR", JLH_DCF_AmbientGroupRole.ROADBLOCK_GUARD, 8, "roadblock_assault_exact_vanilla");
		AppendRoadblockAssaultPlanSelection(enemyPlan, ROADBLOCK_ASSAULT_GROUP_USSR_AT, "USSR", "vanilla_exact:ussr_at_team", JLH_DCF_AmbientGroupRole.AT_TEAM, 2);
		AppendRoadblockAssaultPlanSelection(enemyPlan, ROADBLOCK_ASSAULT_GROUP_USSR_FIRE, "USSR", "vanilla_exact:ussr_firegroup", JLH_DCF_AmbientGroupRole.FIRETEAM_4, 6);
	}

	protected void AppendRoadblockAssaultPlanSelection(notnull JLH_AmbientSkirmishGroupPlan plan, ResourceName prefab, string factionKey, string source, JLH_DCF_AmbientGroupRole role, int estimatedUnits)
	{
		ref JLH_DCF_EnemyGroupSelection selection = new JLH_DCF_EnemyGroupSelection();
		selection.Prefab = prefab;
		selection.EnemyFactionKey = factionKey;
		selection.Source = source;
		selection.Role = role;
		selection.EstimatedUnits = estimatedUnits;
		plan.AddLegacySelection(selection);
	}

	protected bool SelectRoadblockAssaultRoad(IEntity player, out vector roadblockPosition, out vector roadDirection, out float roadWidth, out float snapDistance, out string reason)
	{
		roadblockPosition = "0 0 0";
		roadDirection = "0 0 1";
		roadWidth = 0.0;
		snapDistance = 0.0;
		reason = "";

		RoadNetworkManager roadManager = GetRoadNetworkManager();
		if (!roadManager)
		{
			reason = "roadblock_assault_road_network_missing";
			return false;
		}

		IEntity owner = GetOwner();
		if (!owner)
		{
			reason = "area_missing";
			return false;
		}

		vector anchor = owner.GetOrigin();
		if (m_ActiveScenarioMarker && m_ActiveScenarioMarker.GetOwner())
			anchor = m_ActiveScenarioMarker.GetOwner().GetOrigin();

		bool found = false;
		float bestScore = -99999999.0;
		float searchRadius = Math.Max(GetEffectiveSpawnRadius(), 220.0);
		float maxSnapDistance = Math.Max(70.0, searchRadius * 0.3);
		int attempts = SPAWN_CANDIDATE_ATTEMPTS * 2;
		for (int i = 0; i < attempts; i++)
		{
			vector candidate = anchor;
			if (i > 0)
				candidate = anchor + JLH_AddonWorldQuery.RandomOffset(0.0, searchRadius);

			vector candidateRoadPosition;
			vector candidateRoadDirection;
			float candidateRoadWidth;
			float candidateSnapDistance;
			float candidateScore;
			string candidateReason;
			if (!TryResolveRoadblockAssaultRoadCandidate(roadManager, candidate, player, maxSnapDistance, candidateRoadPosition, candidateRoadDirection, candidateRoadWidth, candidateSnapDistance, candidateScore, candidateReason))
			{
				if (candidateReason != "")
					reason = candidateReason;
				continue;
			}

			if (!found || candidateScore > bestScore)
			{
				found = true;
				bestScore = candidateScore;
				roadblockPosition = candidateRoadPosition;
				roadDirection = candidateRoadDirection;
				roadWidth = candidateRoadWidth;
				snapDistance = candidateSnapDistance;
			}
		}

		if (!found && reason == "")
			reason = "roadblock_assault_no_valid_road";

		return found;
	}

	protected bool TryResolveRoadblockAssaultRoadCandidate(RoadNetworkManager roadManager, vector candidate, IEntity player, float maxSnapDistance, out vector roadblockPosition, out vector roadDirection, out float roadWidth, out float snapDistance, out float score, out string reason)
	{
		roadblockPosition = "0 0 0";
		roadDirection = "0 0 1";
		roadWidth = 0.0;
		snapDistance = 0.0;
		score = 0.0;
		reason = "";

		if (!roadManager)
		{
			reason = "roadblock_assault_road_network_missing";
			return false;
		}

		if (!TryProjectToNearbyRoad(roadManager, candidate, maxSnapDistance, roadblockPosition, roadDirection, roadWidth, snapDistance, reason))
			return false;

		if (roadWidth < ROADBLOCK_ASSAULT_MIN_ROAD_WIDTH)
		{
			reason = "roadblock_assault_road_too_narrow";
			return false;
		}

		if (JLH_AddonWorldQuery.IsWaterOrNearWater(roadblockPosition, 10.0))
		{
			reason = "roadblock_assault_road_water";
			return false;
		}

		float slope = JLH_AddonWorldQuery.GetSlopeDegrees(roadblockPosition);
		if (slope > 10.0)
		{
			reason = "roadblock_assault_road_too_steep";
			return false;
		}

		if (!IsCenterCandidateValid(roadblockPosition, reason))
		{
			reason = "roadblock_assault_center_" + reason;
			return false;
		}

		if (!HasRoadblockAssaultRoadContinuity(roadManager, roadblockPosition, roadDirection, roadWidth, reason))
			return false;

		if (!HasRoadblockAssaultCombatSpace(roadblockPosition, roadDirection, reason))
			return false;

		score = (roadWidth * 14.0) - (snapDistance * 1.7) - (slope * 4.0);
		if (player)
		{
			float playerDistance = vector.Distance(player.GetOrigin(), roadblockPosition);
			score = score - (Math.AbsFloat(playerDistance - 380.0) * 0.03);
			if (JLH_AddonWorldQuery.HasLineOfSight(player, roadblockPosition))
				score = score - 20.0;
		}

		return true;
	}

	protected RoadNetworkManager GetRoadNetworkManager()
	{
		ChimeraGame game = ChimeraGame.Cast(GetGame());
		if (!game)
			return null;

		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(game.GetAIWorld());
		if (!aiWorld)
			return null;

		return aiWorld.GetRoadNetworkManager();
	}

	protected bool TryProjectToNearbyRoad(RoadNetworkManager roadManager, vector candidate, float maxSnapDistance, out vector roadPosition, out vector roadDirection, out float roadWidth, out float snapDistance, out string reason)
	{
		roadPosition = "0 0 0";
		roadDirection = "0 0 1";
		roadWidth = 0.0;
		snapDistance = 0.0;
		reason = "";

		if (!roadManager)
		{
			reason = "roadblock_assault_road_network_missing";
			return false;
		}

		BaseRoad road;
		int roadResult = roadManager.GetClosestRoad(candidate, road, snapDistance, false);
		if (roadResult <= 0 || !road)
		{
			reason = "roadblock_assault_no_road_near_candidate";
			return false;
		}

		if (snapDistance > maxSnapDistance)
		{
			reason = "roadblock_assault_road_snap_too_far";
			return false;
		}

		roadWidth = road.GetWidth();
		if (roadWidth <= 0.1)
			roadWidth = 3.0;

		if (!FindClosestPointOnRoad(road, candidate, roadPosition, roadDirection, reason))
			return false;

		roadPosition = JLH_AddonWorldQuery.GroundPosition(roadPosition);
		return true;
	}

	protected bool FindClosestPointOnRoad(BaseRoad road, vector candidate, out vector roadPosition, out vector roadDirection, out string reason)
	{
		roadPosition = "0 0 0";
		roadDirection = "0 0 1";
		reason = "";

		if (!road)
		{
			reason = "road_missing";
			return false;
		}

		array<vector> points = {};
		if (road.GetPoints(points) < 2)
		{
			reason = "road_points_missing";
			return false;
		}

		bool found = false;
		float bestDistanceSq = 99999999.0;
		for (int i = 0; i < points.Count() - 1; i++)
		{
			vector start = points[i];
			vector finish = points[i + 1];
			vector segment = finish - start;
			segment[1] = 0.0;
			float segmentLengthSq = (segment[0] * segment[0]) + (segment[2] * segment[2]);
			if (segmentLengthSq <= 0.01)
				continue;

			float t = (((candidate[0] - start[0]) * segment[0]) + ((candidate[2] - start[2]) * segment[2])) / segmentLengthSq;
			t = Math.Clamp(t, 0.0, 1.0);

			vector projected = start + ((finish - start) * t);
			float distanceSq = HorizontalDistanceSq(candidate, projected);
			if (!found || distanceSq < bestDistanceSq)
			{
				found = true;
				bestDistanceSq = distanceSq;
				roadPosition = projected;
				roadDirection = DirectionFromTo(start, finish);
			}
		}

		if (!found)
		{
			reason = "road_segment_missing";
			return false;
		}

		return true;
	}

	protected float HorizontalDistanceSq(vector a, vector b)
	{
		float dx = a[0] - b[0];
		float dz = a[2] - b[2];
		return (dx * dx) + (dz * dz);
	}

	protected bool HasRoadblockAssaultRoadContinuity(RoadNetworkManager roadManager, vector center, vector roadDirection, float roadWidth, out string reason)
	{
		reason = "";
		if (!roadManager)
		{
			reason = "roadblock_assault_road_network_missing";
			return false;
		}

		vector lateral = Vector(-roadDirection[2], 0.0, roadDirection[0]);
		float alongSnap = Math.Max(8.0, roadWidth + 4.0);
		float sideSnap = Math.Max(5.0, (roadWidth * 0.75) + 1.5);
		float sideSample = Math.Clamp(roadWidth * 0.45, 2.0, 5.0);

		if (!IsRoadNear(roadManager, center + (roadDirection * 18.0), alongSnap))
		{
			reason = "roadblock_assault_road_forward_gap";
			return false;
		}

		if (!IsRoadNear(roadManager, center - (roadDirection * 18.0), alongSnap))
		{
			reason = "roadblock_assault_road_rear_gap";
			return false;
		}

		if (!IsRoadNear(roadManager, center + (lateral * sideSample), sideSnap) || !IsRoadNear(roadManager, center - (lateral * sideSample), sideSnap))
		{
			reason = "roadblock_assault_road_edge_gap";
			return false;
		}

		return true;
	}

	protected bool IsRoadNear(RoadNetworkManager roadManager, vector position, float maxDistance)
	{
		if (!roadManager)
			return false;

		BaseRoad road;
		float distance;
		int result = roadManager.GetClosestRoad(position, road, distance, false);
		return result > 0 && road && distance <= maxDistance;
	}

	protected bool HasRoadblockAssaultCombatSpace(vector center, vector roadDirection, out string reason)
	{
		reason = "";
		vector lateral = Vector(-roadDirection[2], 0.0, roadDirection[0]);
		if (!IsRoadblockAssaultGroundUsable(center + (lateral * 18.0), 22.0, reason))
		{
			reason = "roadblock_assault_left_space_" + reason;
			return false;
		}

		if (!IsRoadblockAssaultGroundUsable(center - (lateral * 18.0), 22.0, reason))
		{
			reason = "roadblock_assault_right_space_" + reason;
			return false;
		}

		return true;
	}

	protected bool BuildRoadblockAssaultPositions(IEntity player, vector roadblockPosition, vector roadDirection, out vector attackerPosition, out vector defenderATPosition, out vector defenderFirePosition, out string reason)
	{
		attackerPosition = "0 0 0";
		defenderATPosition = "0 0 0";
		defenderFirePosition = "0 0 0";
		reason = "";

		RoadNetworkManager roadManager = GetRoadNetworkManager();
		if (!roadManager)
		{
			reason = "roadblock_assault_road_network_missing";
			return false;
		}

		bool foundAttacker = false;
		float bestScore = -99999999.0;
		vector bestAttackDirection = roadDirection;
		for (int sideIndex = 0; sideIndex < 2; sideIndex++)
		{
			vector attackDirection = roadDirection;
			if (sideIndex == 1)
				attackDirection = roadDirection * -1.0;

			for (int distanceIndex = 0; distanceIndex < 5; distanceIndex++)
			{
				float distance = 170.0 + (distanceIndex * 28.0) + Math.RandomFloat(-12.0, 12.0);
				distance = Math.Clamp(distance, ROADBLOCK_ASSAULT_MIN_ATTACKER_DISTANCE, ROADBLOCK_ASSAULT_MAX_ATTACKER_DISTANCE);
				vector roadCandidate;
				vector approachDirection;
				float approachWidth;
				float snapDistance;
				string roadReason;
				if (!TryProjectToNearbyRoad(roadManager, roadblockPosition - (attackDirection * distance), 65.0, roadCandidate, approachDirection, approachWidth, snapDistance, roadReason))
				{
					if (roadReason != "")
						reason = roadReason;
					continue;
				}

				vector approachLateral = Vector(-approachDirection[2], 0.0, approachDirection[0]);
				float lateralOffset = 10.0;
				if ((distanceIndex % 2) == 1)
					lateralOffset = -12.0;
				else if (sideIndex == 1)
					lateralOffset = 12.0;

				vector candidate = JLH_AddonWorldQuery.GroundPosition(roadCandidate + (approachLateral * lateralOffset));
				if (!IsRoadblockAssaultGroupPositionValid(candidate, 24.0, reason))
					continue;

				float score = 1000.0 - Math.AbsFloat(distance - 220.0) - (snapDistance * 2.0);
				if (player)
				{
					float playerDistance = vector.Distance(player.GetOrigin(), candidate);
					score = score - Math.AbsFloat(playerDistance - 280.0) * 0.05;
				}

				if (HasClearTerrainLine(candidate, roadblockPosition))
					score = score - 45.0;

				if (!foundAttacker || score > bestScore)
				{
					foundAttacker = true;
					bestScore = score;
					attackerPosition = candidate;
					bestAttackDirection = attackDirection;
				}
			}
		}

		if (!foundAttacker)
		{
			if (reason == "")
				reason = "roadblock_assault_no_attacker_position";
			return false;
		}

		vector roadLateral = Vector(-roadDirection[2], 0.0, roadDirection[0]);
		vector atDesired = roadblockPosition + (bestAttackDirection * 11.0) - (roadLateral * 13.0);
		vector fireDesired = roadblockPosition + (bestAttackDirection * 8.0) + (roadLateral * 16.0);
		if (!ResolveRoadblockAssaultDefenderPosition(atDesired, roadblockPosition, defenderATPosition, reason))
			return false;

		if (!ResolveRoadblockAssaultDefenderPosition(fireDesired, roadblockPosition, defenderFirePosition, reason))
			return false;

		if (vector.Distance(defenderATPosition, defenderFirePosition) < 12.0)
		{
			vector shiftedFire = JLH_AddonWorldQuery.GroundPosition(fireDesired + (roadLateral * 12.0));
			if (!IsRoadblockAssaultGroupPositionValid(shiftedFire, 24.0, reason))
			{
				reason = "roadblock_assault_defenders_stacked";
				return false;
			}

			defenderFirePosition = shiftedFire;
		}

		return true;
	}

	protected bool ResolveRoadblockAssaultDefenderPosition(vector desired, vector center, out vector position, out string reason)
	{
		position = "0 0 0";
		reason = "";

		for (int i = 0; i < 8; i++)
		{
			vector candidate = desired;
			if (i > 0)
				candidate = desired + JLH_AddonWorldQuery.RandomOffset(2.0, Math.Min(13.0, 4.0 + (i * 2.0)));

			candidate = JLH_AddonWorldQuery.GroundPosition(candidate);
			if (vector.Distance(candidate, center) > 42.0)
				continue;

			if (!IsRoadblockAssaultGroupPositionValid(candidate, 24.0, reason))
				continue;

			position = candidate;
			reason = "";
			return true;
		}

		if (reason == "")
			reason = "roadblock_assault_no_defender_position";

		return false;
	}

	protected bool IsRoadblockAssaultGroupPositionValid(vector candidate, float maxSlope, out string reason)
	{
		if (!IsGroupPositionValid(candidate, reason))
			return false;

		return IsRoadblockAssaultGroundUsable(candidate, maxSlope, reason);
	}

	protected bool IsRoadblockAssaultGroundUsable(vector candidate, float maxSlope, out string reason)
	{
		reason = "";
		if (JLH_AddonWorldQuery.IsWaterOrNearWater(candidate, 5.0))
		{
			reason = "water";
			return false;
		}

		if (maxSlope > 0.0 && JLH_AddonWorldQuery.GetSlopeDegrees(candidate) > maxSlope)
		{
			reason = "steep_terrain";
			return false;
		}

		return true;
	}

	protected bool HasClearTerrainLine(vector from, vector to)
	{
		if (!GetGame() || !GetGame().GetWorld())
			return false;

		TraceParam trace = new TraceParam();
		trace.Start = from + Vector(0, 1.5, 0);
		trace.End = to + Vector(0, 1.2, 0);
		trace.Flags = TraceFlags.DEFAULT | TraceFlags.ANY_CONTACT;
		trace.LayerMask = EPhysicsLayerDefs.Projectile;

		float fraction = GetGame().GetWorld().TraceMove(trace, null);
		return fraction >= 0.98;
	}

	protected bool IsRoadblockBreakthroughMainRoad(float roadWidth)
	{
		return roadWidth >= ROADBLOCK_BREAKTHROUGH_MAIN_ROAD_MIN_WIDTH;
	}

	protected ResourceName ResolveRoadblockBreakthroughComposition(float roadWidth)
	{
		if (IsRoadblockBreakthroughMainRoad(roadWidth))
			return ROADBLOCK_BREAKTHROUGH_COMPOSITION_MAIN_USSR;

		return ROADBLOCK_BREAKTHROUGH_COMPOSITION_DIRT_USSR;
	}

	protected string RoadblockBreakthroughRoadTypeLabel(bool mainRoad)
	{
		if (mainRoad)
			return "main";

		return "dirt";
	}

	protected IEntity SpawnRoadblockAssaultComposition(vector position, vector roadDirection)
	{
		return SpawnRoadblockComposition(ROADBLOCK_ASSAULT_COMPOSITION_USSR, position, roadDirection, "roadblock_assault_composition");
	}

	protected IEntity SpawnRoadblockComposition(ResourceName prefab, vector position, vector roadDirection, string label)
	{
		if (prefab == "" || !JLH_AddonSpawnUtility.CanLoadPrefab(prefab))
			return null;

		Resource resource = Resource.Load(prefab);
		if (!resource || !resource.IsValid() || !GetGame() || !GetGame().GetWorld())
			return null;

		vector forward = roadDirection;
		if (vector.Distance(forward, "0 0 0") <= 0.001)
			forward = "0 0 1";

		vector right = Vector(forward[2], 0.0, -forward[0]);
		vector up = Vector(0.0, 1.0, 0.0);
		vector transform[4];
		transform[0] = right;
		transform[1] = up;
		transform[2] = forward;
		transform[3] = JLH_AddonWorldQuery.GroundPosition(position);

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[0] = transform[0];
		spawnParams.Transform[1] = transform[1];
		spawnParams.Transform[2] = transform[2];
		spawnParams.Transform[3] = transform[3];

		IEntity entity = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams);
		if (entity)
			JLH_DCF_PersistenceExclusion.StopTrackingTree(entity, label);

		return entity;
	}

	protected bool SpawnRadialEnemyGroupPlan(notnull JLH_AmbientSkirmishEventDefinition definition, notnull JLH_AmbientSkirmishGroupPlan plan, vector center, float groupSeparation, notnull array<SCR_AIGroup> outGroups, out string reason)
	{
		outGroups.Clear();
		reason = "";

		if (!plan || plan.IsEmpty())
		{
			reason = "enemy_plan_empty";
			return false;
		}

		float baseAngle = Math.RandomFloat(0.0, 360.0);
		float spawnDistance = Math.Clamp(groupSeparation * 1.35, 120.0, 260.0);
		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.DOWNED_US_HELI_RESCUE)
			spawnDistance = Math.Clamp((groupSeparation * 1.7) - 50.0, 130.0, 290.0);
		else if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.DOWNED_CREW_RESCUE)
			spawnDistance = Math.Clamp(groupSeparation * 1.15, 95.0, 220.0);
		for (int i = 0; i < plan.Groups.Count(); i++)
		{
			JLH_DCF_EnemyGroupSelection selection = plan.Groups[i];
			if (!selection || selection.Prefab == "")
			{
				CleanupSpawnedGroups(outGroups, "ambient_skirmish_enemy_radial_partial_cleanup_invalid_selection");
				reason = "enemy_selection_invalid";
				return false;
			}

			vector groupPosition;
			if (!BuildRadialApproachPosition(center, baseAngle, i, plan.Groups.Count(), spawnDistance, groupPosition, reason))
			{
				CleanupSpawnedGroups(outGroups, "ambient_skirmish_enemy_radial_partial_cleanup_no_position");
				return false;
			}

			SCR_AIGroup group = JLH_AddonSpawnUtility.SpawnGroup(selection.Prefab, groupPosition, "ambient_skirmish_enemy_radial_group_spawn_" + i.ToString(), selection.EstimatedUnits);
			if (!group)
			{
				CleanupSpawnedGroups(outGroups, "ambient_skirmish_enemy_radial_partial_cleanup_spawn_failed");
				reason = "enemy_spawn_failed";
				return false;
			}

			outGroups.Insert(group);
		}

		LogEvent("ambient_skirmish_ambush_profile", "event=" + definition.SkirmishEventName + " ambush_profile=" + definition.AmbushProfile + " center=" + center.ToString() + " groups=" + outGroups.Count().ToString() + " distance=" + spawnDistance.ToString(), true);
		return !outGroups.IsEmpty();
	}

	protected bool BuildRadialApproachPosition(vector center, float baseAngle, int groupIndex, int groupCount, float distance, out vector position, out string reason)
	{
		position = "0 0 0";
		reason = "";

		float angleStep = 360.0 / Math.Max(1.0, groupCount);
		float angle = baseAngle + (angleStep * groupIndex) + Math.RandomFloat(-10.0, 10.0);
		for (int attempt = 0; attempt < SPAWN_CANDIDATE_ATTEMPTS; attempt++)
		{
			float attemptAngle = angle + (attempt * 23.0);
			float attemptDistance = distance + (attempt * 8.0);
			vector candidate = JLH_AddonWorldQuery.GroundPosition(center + (JLH_AddonWorldQuery.DirectionFromAngle(attemptAngle) * attemptDistance));
			if (!IsGroupPositionValid(candidate, reason))
				continue;

			position = candidate;
			return true;
		}

		if (reason == "")
			reason = "no_valid_radial_group_position";

		return false;
	}

	protected bool ShouldUseCrashSurvivorLayout(notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		JLH_AmbientSkirmishEventType eventType = definition.SkirmishEventType;
		return eventType == JLH_AmbientSkirmishEventType.DOWNED_CREW_RESCUE || eventType == JLH_AmbientSkirmishEventType.DOWNED_US_HELI_RESCUE;
	}

	protected bool ShouldUseRadialEnemyApproach(notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		return ShouldUseCrashSurvivorLayout(definition);
	}

	protected bool LayoutCrashSurvivors(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishEventDefinition definition, int attempt)
	{
		if (record.FriendlyGroups.IsEmpty() || !record.FriendlyGroups[0])
			return false;

		float dangerMin = 5.0;
		float dangerMax = 9.0;
		float survivorMin = 16.0;
		float survivorMax = 30.0;
		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.DOWNED_US_HELI_RESCUE)
		{
			dangerMin = 3.5;
			dangerMax = 7.0;
			survivorMin = 8.0;
			survivorMax = 14.0;
		}
		else if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.DOWNED_CREW_RESCUE)
		{
			dangerMin = 4.0;
			dangerMax = 8.0;
			survivorMin = 12.0;
			survivorMax = 24.0;
		}

		int casualties;
		int moved = JLH_AddonSpawnUtility.ScatterGroupUnitsAroundWithCasualties(record.FriendlyGroups[0], record.Center, 99, 99, dangerMin, dangerMax, survivorMin, survivorMax, casualties, "ambient_skirmish_crash_casualty_layout");
		LogEvent("ambient_skirmish_crash_casualties_laid_out", "id=" + record.Id.ToString() + " event=" + definition.SkirmishEventName + " attempt=" + attempt.ToString() + " moved=" + moved.ToString() + " casualties=" + casualties.ToString() + " center=" + record.Center.ToString(), true);
		if (moved > 0)
		{
			ScheduleDelayedCrashSurvivors(record, definition);
			return true;
		}

		if (attempt < CRASH_SURVIVOR_LAYOUT_MAX_ATTEMPTS)
			ScheduleCrashSurvivorLayoutRetry(record, definition, attempt + 1);

		return false;
	}

	protected void ScheduleCrashSurvivorLayoutRetry(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishEventDefinition definition, int attempt)
	{
		if (!GetGame() || !GetGame().GetCallqueue())
			return;

		GetGame().GetCallqueue().CallLater(JLH_AmbientSkirmish_RetryCrashSurvivorLayout, CRASH_SURVIVOR_LAYOUT_RETRY_MS, false, this, record, definition, attempt);
	}

	void RetryCrashSurvivorLayout(JLH_AmbientSkirmishRecord record, JLH_AmbientSkirmishEventDefinition definition, int attempt)
	{
		if (!record || !definition)
			return;

		if (!ShouldUseCrashSurvivorLayout(definition))
			return;

		if (LayoutCrashSurvivors(record, definition, attempt))
			record.Runtime.RefreshExclusions("ambient_skirmish_crash_survivor_retry_refresh");
	}

	protected void ScheduleDelayedCrashSurvivors(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		if (!GetGame() || !GetGame().GetCallqueue())
			return;

		int delayMs = GetCrashSurvivorReinforcementDelayMs(definition);
		LogEvent("ambient_skirmish_crash_survivor_reinforcement_queued", "id=" + record.Id.ToString() + " event=" + definition.SkirmishEventName + " delayMs=" + delayMs.ToString(), true);
		GetGame().GetCallqueue().CallLater(JLH_AmbientSkirmish_SpawnDelayedCrashSurvivors, delayMs, false, this, record, definition);
	}

	protected int GetCrashSurvivorReinforcementDelayMs(notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.DOWNED_US_HELI_RESCUE)
			return Math.Max(CRASH_SURVIVOR_REINFORCEMENT_DELAY_MS, m_iDownedUSHeliDestroyDelayMs + DOWNED_US_HELI_SURVIVOR_POST_BLAST_BUFFER_MS);

		return CRASH_SURVIVOR_REINFORCEMENT_DELAY_MS;
	}

	void SpawnDelayedCrashSurvivors(JLH_AmbientSkirmishRecord record, JLH_AmbientSkirmishEventDefinition definition)
	{
		if (!record || !definition)
			return;

		if (!ShouldUseCrashSurvivorLayout(definition))
			return;

		if (!HasRuntimeRecord(record))
			return;

		string reason;
		JLH_AmbientSkirmishGroupPlan survivorPlan;
		if (!JLH_AmbientSkirmishGroupComposer.TryBuildFriendlySinglePlan(m_sFriendlyFactionKey, definition.FriendlyRole, definition.FriendlyMinUnits, definition.FriendlyMaxUnits, survivorPlan, reason))
		{
			LogEvent("ambient_skirmish_crash_survivor_reinforcement_failed", "id=" + record.Id.ToString() + " event=" + definition.SkirmishEventName + " reason=" + reason, true);
			return;
		}

		vector defendPosition;
		if (!BuildCrashSurvivorDefendPosition(definition, record.Center, defendPosition))
		{
			LogEvent("ambient_skirmish_crash_survivor_reinforcement_failed", "id=" + record.Id.ToString() + " event=" + definition.SkirmishEventName + " reason=no_survivor_defend_position", true);
			return;
		}

		array<IEntity> noExactMarkers = {};
		array<SCR_AIGroup> survivorGroups = {};
		if (!SpawnGroupPlan(survivorPlan, defendPosition, "friendly_crash_survivor", noExactMarkers, survivorGroups, reason))
		{
			LogEvent("ambient_skirmish_crash_survivor_reinforcement_failed", "id=" + record.Id.ToString() + " event=" + definition.SkirmishEventName + " reason=" + reason, true);
			return;
		}

		for (int i = 0; i < survivorGroups.Count(); i++)
		{
			SCR_AIGroup survivorGroup = survivorGroups[i];
			if (!survivorGroup)
				continue;

			record.AttachFriendlyGroup(survivorGroup, "ambient_skirmish_friendly_crash_survivor_delayed_" + i.ToString(), survivorPlan.TotalUnits, survivorPlan.Source);
			JLH_AddonSpawnUtility.AssignPositionalDefend(survivorGroup, defendPosition, record.Runtime, "ambient_skirmish_friendly_crash_survivor_defend_" + i.ToString());
		}

		record.Runtime.RefreshExclusions("ambient_skirmish_crash_survivor_reinforcement_refresh");
		LogEvent("ambient_skirmish_crash_survivor_reinforcement_spawned", "id=" + record.Id.ToString() + " event=" + definition.SkirmishEventName + " groups=" + survivorGroups.Count().ToString() + " units=" + survivorPlan.TotalUnits.ToString() + " defend=" + defendPosition.ToString() + " source=" + survivorPlan.Source, true);
	}

	protected bool BuildCrashSurvivorDefendPosition(notnull JLH_AmbientSkirmishEventDefinition definition, vector center, out vector position)
	{
		position = "0 0 0";

		float minDistance = 16.0;
		float maxDistance = 26.0;
		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.DOWNED_US_HELI_RESCUE)
		{
			minDistance = 8.0;
			maxDistance = 14.0;
		}

		vector baseGround = JLH_AddonWorldQuery.GroundPosition(center);
		float baseAngle = Math.RandomFloat(0.0, 360.0);
		for (int i = 0; i < SPAWN_CANDIDATE_ATTEMPTS; i++)
		{
			float angle = baseAngle + (i * 47.0);
			float distance = Math.RandomFloat(minDistance, maxDistance);
			vector candidate = JLH_AddonWorldQuery.GroundPosition(baseGround + (JLH_AddonWorldQuery.DirectionFromAngle(angle) * distance));
			if (JLH_AddonWorldQuery.IsWaterOrNearWater(candidate, 4.0))
				continue;

			position = candidate;
			return true;
		}

		return false;
	}

	protected vector BuildComposedGroupPosition(vector basePosition, int groupIndex, int groupCount)
	{
		vector baseGround = JLH_AddonWorldQuery.GroundPosition(basePosition);
		if (groupIndex <= 0 || groupCount <= 1)
			return baseGround;

		float ringDistance = Math.Clamp(16.0 + (groupIndex * 9.0), 16.0, 70.0);
		float angleStep = 360.0 / Math.Max(1.0, groupCount);
		float angle = (angleStep * groupIndex) + Math.RandomFloat(-18.0, 18.0);

		for (int attempt = 0; attempt < 6; attempt++)
		{
			vector candidate = JLH_AddonWorldQuery.GroundPosition(baseGround + (JLH_AddonWorldQuery.DirectionFromAngle(angle + (attempt * 63.0)) * ringDistance));
			if (!JLH_AddonWorldQuery.IsWater(candidate))
				return candidate;
		}

		return baseGround;
	}

	protected vector BuildComposedGroupPositionWithSpacing(vector basePosition, int groupIndex, int groupCount, float spacing)
	{
		vector baseGround = JLH_AddonWorldQuery.GroundPosition(basePosition);
		if (groupIndex <= 0 || groupCount <= 1 || spacing <= 0.0)
			return baseGround;

		float ringDistance = Math.Clamp(spacing, 20.0, 120.0);
		float angleStep = 360.0 / Math.Max(1.0, groupCount);
		float angle = (angleStep * groupIndex) + Math.RandomFloat(-10.0, 10.0);

		for (int attempt = 0; attempt < 8; attempt++)
		{
			vector candidate = JLH_AddonWorldQuery.GroundPosition(baseGround + (JLH_AddonWorldQuery.DirectionFromAngle(angle + (attempt * 53.0)) * ringDistance));
			if (!JLH_AddonWorldQuery.IsWaterOrNearWater(candidate, 4.0))
				return candidate;
		}

		return baseGround;
	}

	void CleanupSpawnedGroups(notnull array<SCR_AIGroup> groups, string reason)
	{
		for (int i = groups.Count() - 1; i >= 0; i--)
		{
			if (groups[i])
				JLH_AddonSpawnUtility.DeleteGroupAndMembers(groups[i], reason);
		}

		groups.Clear();
	}

	protected bool SelectEventDefinition(JLH_DCF_AmbientSkirmishIntensity intensity, out JLH_AmbientSkirmishEventDefinition definition, out string reason)
	{
		definition = null;
		reason = "";

		JLH_AmbientSkirmishEventType eventType = m_eEventType;
		if (m_bRandomizeEventType || eventType == JLH_AmbientSkirmishEventType.RANDOM)
			eventType = SelectRandomAllowedEventType();

		if (!JLH_AmbientSkirmishEventLibrary.BuildDefinition(eventType, intensity, m_iMinInfantryPerSide, m_iMaxInfantryPerSide, m_bAllowVehicles, m_bAllowSupplyTrucks, m_bAllowStaticWeapons, definition, reason))
			return false;

		if (!definition)
		{
			reason = "event_definition_missing";
			return false;
		}

		ApplyVehicleLabelConfiguration(definition);
		LogScenarioSelected(definition);
		return true;
	}

	void LogScenarioSelected(notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		string details = "event=" + definition.SkirmishEventName;
		details = details + " scenario_selected=true";
		details = details + " encounter_role=" + definition.EncounterRole;
		details = details + " composition_type=" + definition.CompositionType;
		details = details + " spacing_profile=" + definition.SpacingProfile;
		details = details + " combat_profile=" + definition.CombatProfile;
		details = details + " route_profile=" + definition.RouteProfile;
		details = details + " convoy_profile=" + definition.ConvoyProfile;
		details = details + " ambush_profile=" + definition.AmbushProfile;
		details = details + " rescue_profile=" + definition.RescueProfile;
		details = details + " friendlyRole=" + SkirmishRoleName(definition.FriendlyRole);
		details = details + " enemyRole=" + SkirmishRoleName(definition.EnemyRole);
		details = details + " friendlyUnits=" + definition.FriendlyMinUnits.ToString() + "-" + definition.FriendlyMaxUnits.ToString();
		details = details + " enemyUnits=" + definition.EnemyMinUnits.ToString() + "-" + definition.EnemyMaxUnits.ToString();
		details = details + " separationMultiplier=" + definition.SeparationMultiplier.ToString();
		LogEvent("ambient_skirmish_scenario_selected", details, true);
	}

	protected void LogScenarioSpacingProfile(notnull JLH_AmbientSkirmishEventDefinition definition, vector center, vector friendlyPosition, vector enemyPosition, float shapedSeparation, string layoutSource)
	{
		string details = "event=" + definition.SkirmishEventName;
		details = details + " spacing_profile=" + definition.SpacingProfile;
		details = details + " layoutSource=" + layoutSource;
		details = details + " shapedSeparation=" + shapedSeparation.ToString();
		details = details + " center=" + center.ToString();
		details = details + " friendly=" + friendlyPosition.ToString();
		details = details + " enemy=" + enemyPosition.ToString();
		LogEvent("ambient_skirmish_spacing_profile", details, true);
	}

	protected void LogScenarioCombatProfile(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		string details = "id=" + record.Id.ToString();
		details = details + " event=" + definition.SkirmishEventName;
		details = details + " encounter_role=" + definition.EncounterRole;
		details = details + " combat_profile=" + definition.CombatProfile;
		details = details + " route_profile=" + definition.RouteProfile;
		details = details + " convoy_profile=" + definition.ConvoyProfile;
		details = details + " ambush_profile=" + definition.AmbushProfile;
		details = details + " rescue_profile=" + definition.RescueProfile;
		LogEvent("ambient_skirmish_combat_profile", details, true);
	}

	protected string SkirmishRoleName(JLH_DCF_AmbientGroupRole role)
	{
		if (role == JLH_DCF_AmbientGroupRole.SNIPER_TEAM) return "SNIPER_TEAM";
		if (role == JLH_DCF_AmbientGroupRole.RECON_TEAM) return "RECON_TEAM";
		if (role == JLH_DCF_AmbientGroupRole.FIRETEAM_4) return "FIRETEAM_4";
		if (role == JLH_DCF_AmbientGroupRole.SQUAD_6) return "SQUAD_6";
		if (role == JLH_DCF_AmbientGroupRole.FULL_SQUAD) return "FULL_SQUAD";
		if (role == JLH_DCF_AmbientGroupRole.FOREST_AMBUSH) return "FOREST_AMBUSH";
		if (role == JLH_DCF_AmbientGroupRole.ROADBLOCK_GUARD) return "ROADBLOCK_GUARD";
		if (role == JLH_DCF_AmbientGroupRole.VEHICLE_CREW) return "VEHICLE_CREW";
		if (role == JLH_DCF_AmbientGroupRole.CONVOY_ESCORT) return "CONVOY_ESCORT";
		if (role == JLH_DCF_AmbientGroupRole.AT_TEAM) return "AT_TEAM";
		if (role == JLH_DCF_AmbientGroupRole.MG_TEAM) return "MG_TEAM";
		return "UNKNOWN";
	}

	protected void ApplyVehicleLabelConfiguration(notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		if (!definition.RequiresVehicles && definition.SkirmishEventType != JLH_AmbientSkirmishEventType.DOWNED_CREW_RESCUE)
			return;

		ApplyVehicleRoleLabelDefaults(definition);
		definition.ApplyEditableVehicleLabelOverrides(m_aRequiredVehicleLabels, m_aPreferredVehicleLabels, m_aExcludedVehicleLabels, m_aFallbackVehicleLabels);
		if (IsCombatVehicleRoleScenario(definition))
			EnsureCombatVehicleGroundLabels(definition);

		definition.AddDefaultGroundVehicleExclusions();
	}

	protected bool IsCombatVehicleRoleScenario(notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.VEHICLE_SKIRMISH)
			return true;

		return false;
	}

	protected void ClearVehicleLabelFilters(notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		definition.RequiredVehicleLabels.Clear();
		definition.PreferredVehicleLabels.Clear();
		definition.FallbackVehicleLabels.Clear();
		definition.ExcludedVehicleLabels.Clear();
	}

	protected void EnsureCombatVehicleGroundLabels(notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		definition.AddRequiredVehicleLabel("TRAIT_ARMED");
		definition.AddDefaultGroundVehicleExclusions();
	}

	protected void ApplyAnyArmedGroundVehicleLabels(notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		ClearVehicleLabelFilters(definition);
		definition.AddRequiredVehicleLabel("TRAIT_ARMED");
		definition.AddFallbackVehicleLabel("TRAIT_ARMED");
		definition.AddDefaultGroundVehicleExclusions();
	}

	protected void ApplyVehicleRoleLabelDefaults(notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		if (m_eVehicleRole == JLH_AmbientSkirmishVehicleRole.ANY_ARMED_GROUND)
		{
			if (IsCombatVehicleRoleScenario(definition))
				ApplyAnyArmedGroundVehicleLabels(definition);

			return;
		}

		ClearVehicleLabelFilters(definition);
		definition.AddDefaultGroundVehicleExclusions();

		if (m_eVehicleRole == JLH_AmbientSkirmishVehicleRole.ARMED_LIGHT)
		{
			definition.AddRequiredVehicleLabel("TRAIT_ARMED");
			definition.AddPreferredVehicleLabel("VEHICLE_LIGHT");
			definition.AddPreferredVehicleLabel("VEHICLE_CAR");
			definition.AddExcludedVehicleLabel("TRAIT_ARMORED");
			definition.AddExcludedVehicleLabel("VEHICLE_APC");
			definition.AddExcludedVehicleLabel("VEHICLE_TANK");
			definition.AddExcludedVehicleLabel("VEHICLE_ARMORED");
			definition.AddExcludedVehicleLabel("VEHICLE_HEAVY");
			definition.AddExcludedVehicleLabel("TRAIT_SUPPLY");
			definition.AddFallbackVehicleLabel("TRAIT_ARMED");
			return;
		}

		if (m_eVehicleRole == JLH_AmbientSkirmishVehicleRole.APC)
		{
			definition.AddRequiredVehicleLabel("TRAIT_ARMED");
			definition.AddPreferredVehicleLabel("TRAIT_ARMORED");
			definition.AddPreferredVehicleLabel("VEHICLE_ARMORED");
			definition.AddPreferredVehicleLabel("VEHICLE_APC");
			definition.AddPreferredVehicleLabel("TRAIT_TRANSPORT");
			definition.AddExcludedVehicleLabel("VEHICLE_HEAVY");
			definition.AddExcludedVehicleLabel("TRAIT_SUPPLY");
			definition.AddFallbackVehicleLabel("TRAIT_ARMED");
			definition.AddFallbackVehicleLabel("TRAIT_ARMORED");
			definition.AddFallbackVehicleLabel("VEHICLE_APC");
			return;
		}

		if (m_eVehicleRole == JLH_AmbientSkirmishVehicleRole.TANK)
		{
			definition.AddRequiredVehicleLabel("TRAIT_ARMED");
			definition.AddPreferredVehicleLabel("TRAIT_ARMORED");
			definition.AddPreferredVehicleLabel("VEHICLE_ARMORED");
			definition.AddPreferredVehicleLabel("VEHICLE_TANK");
			definition.AddPreferredVehicleLabel("VEHICLE_APC");
			definition.AddExcludedVehicleLabel("VEHICLE_HEAVY");
			definition.AddExcludedVehicleLabel("TRAIT_SUPPLY");
			definition.AddFallbackVehicleLabel("TRAIT_ARMED");
			definition.AddFallbackVehicleLabel("TRAIT_ARMORED");
		}
	}

	protected bool HasEditableVehicleLabels(array<EEditableEntityLabel> labels)
	{
		if (!labels || labels.IsEmpty())
			return false;

		foreach (EEditableEntityLabel label : labels)
		{
			string labelName = SCR_Enum.GetEnumName(EEditableEntityLabel, label);
			if (labelName != "" && labelName != "NONE")
				return true;
		}

		return false;
	}

	protected void AddVehicleLabelName(notnull array<string> labels, string labelName)
	{
		if (labelName == "" || labelName == "NONE")
			return;

		if (!labels.Contains(labelName))
			labels.Insert(labelName);
	}

	protected void CopyVehicleLabelNames(array<string> sourceLabels, notnull array<string> targetLabels)
	{
		targetLabels.Clear();
		if (!sourceLabels)
			return;

		foreach (string labelName : sourceLabels)
		{
			AddVehicleLabelName(targetLabels, labelName);
		}
	}

	protected void CopyEditableVehicleLabelsToNames(array<EEditableEntityLabel> sourceLabels, notnull array<string> targetLabels, bool append)
	{
		if (!append)
			targetLabels.Clear();

		if (!sourceLabels)
			return;

		foreach (EEditableEntityLabel label : sourceLabels)
		{
			string labelName = SCR_Enum.GetEnumName(EEditableEntityLabel, label);
			AddVehicleLabelName(targetLabels, labelName);
		}
	}

	protected void AddGroundVehicleExclusionNames(notnull array<string> labels)
	{
		AddVehicleLabelName(labels, "VEHICLE_HELICOPTER");
		AddVehicleLabelName(labels, "VEHICLE_AIRPLANE");
		AddVehicleLabelName(labels, "VEHICLE_AIRCRAFT");
		AddVehicleLabelName(labels, "VEHICLE_PLANE");
		AddVehicleLabelName(labels, "VEHICLE_BOAT");
		AddVehicleLabelName(labels, "VEHICLE_SHIP");
		AddVehicleLabelName(labels, "VEHICLE_WATERCRAFT");
		AddVehicleLabelName(labels, "VEHICLE_SUBMARINE");
	}

	protected void BuildSideVehicleLabelFilters(notnull JLH_AmbientSkirmishEventDefinition definition, array<EEditableEntityLabel> requiredOverride, array<EEditableEntityLabel> preferredOverride, array<EEditableEntityLabel> excludedAdditions, array<EEditableEntityLabel> fallbackOverride, notnull array<string> requiredLabels, notnull array<string> preferredLabels, notnull array<string> excludedLabels, notnull array<string> fallbackLabels)
	{
		CopyVehicleLabelNames(definition.RequiredVehicleLabels, requiredLabels);
		CopyVehicleLabelNames(definition.PreferredVehicleLabels, preferredLabels);
		CopyVehicleLabelNames(definition.ExcludedVehicleLabels, excludedLabels);
		CopyVehicleLabelNames(definition.FallbackVehicleLabels, fallbackLabels);

		if (HasEditableVehicleLabels(requiredOverride))
			CopyEditableVehicleLabelsToNames(requiredOverride, requiredLabels, false);

		if (HasEditableVehicleLabels(preferredOverride))
			CopyEditableVehicleLabelsToNames(preferredOverride, preferredLabels, false);

		if (HasEditableVehicleLabels(fallbackOverride))
			CopyEditableVehicleLabelsToNames(fallbackOverride, fallbackLabels, false);

		if (HasEditableVehicleLabels(excludedAdditions))
			CopyEditableVehicleLabelsToNames(excludedAdditions, excludedLabels, true);

		if (IsCombatVehicleRoleScenario(definition))
			AddVehicleLabelName(requiredLabels, "TRAIT_ARMED");

		AddGroundVehicleExclusionNames(excludedLabels);
	}

	bool ResolveVehiclePrefabsForDefinition(notnull JLH_AmbientSkirmishEventDefinition definition, out string reason)
	{
		reason = "";
		if (!definition.RequiresVehicles)
			return true;

		string enemyFactionKey = JLH_DCF_FactionCompatibilityRegistry.ResolveEnemyFactionKey(m_sFriendlyFactionKey, m_sEnemyFactionKey);
		if (enemyFactionKey == "")
		{
			reason = "vehicle_enemy_faction_missing";
			return false;
		}

		bool needsFriendlyVehicle = ShouldSpawnFriendlyVehicle(definition);
		bool needsEnemyVehicle = ShouldSpawnEnemyVehicle(definition);
		if (!needsFriendlyVehicle && !needsEnemyVehicle)
		{
			reason = "vehicle_side_not_selected";
			return false;
		}

		array<string> friendlyRequiredLabels = {};
		array<string> friendlyPreferredLabels = {};
		array<string> friendlyExcludedLabels = {};
		array<string> friendlyFallbackLabels = {};
		array<string> enemyRequiredLabels = {};
		array<string> enemyPreferredLabels = {};
		array<string> enemyExcludedLabels = {};
		array<string> enemyFallbackLabels = {};

		if (needsFriendlyVehicle)
		{
			BuildSideVehicleLabelFilters(definition, m_aFriendlyRequiredVehicleLabels, m_aFriendlyPreferredVehicleLabels, m_aFriendlyExcludedVehicleLabels, m_aFriendlyFallbackVehicleLabels, friendlyRequiredLabels, friendlyPreferredLabels, friendlyExcludedLabels, friendlyFallbackLabels);
			JLH_CORE_VehicleLabelSelection friendlyVehicle;
			if (!JLH_CORE_CompatibilityResolver.TrySelectVehiclePrefabByLabelFilters(m_sFriendlyFactionKey, friendlyRequiredLabels, friendlyPreferredLabels, friendlyExcludedLabels, friendlyFallbackLabels, friendlyVehicle, reason))
			{
				reason = "friendly_vehicle_" + reason;
				return false;
			}

			definition.FriendlyVehiclePrefab = friendlyVehicle.Prefab;
			definition.FriendlyVehicleSource = friendlyVehicle.Source;
		}

		if (needsEnemyVehicle)
		{
			BuildSideVehicleLabelFilters(definition, m_aEnemyRequiredVehicleLabels, m_aEnemyPreferredVehicleLabels, m_aEnemyExcludedVehicleLabels, m_aEnemyFallbackVehicleLabels, enemyRequiredLabels, enemyPreferredLabels, enemyExcludedLabels, enemyFallbackLabels);
			JLH_CORE_VehicleLabelSelection enemyVehicle;
			if (!JLH_CORE_CompatibilityResolver.TrySelectVehiclePrefabByLabelFilters(enemyFactionKey, enemyRequiredLabels, enemyPreferredLabels, enemyExcludedLabels, enemyFallbackLabels, enemyVehicle, reason))
			{
				reason = "enemy_vehicle_" + reason;
				return false;
			}

			definition.EnemyVehiclePrefab = enemyVehicle.Prefab;
			definition.EnemyVehicleSource = enemyVehicle.Source;
		}

		string filterLog = "event=" + definition.SkirmishEventName;
		filterLog = filterLog + " friendlyFaction=" + m_sFriendlyFactionKey;
		filterLog = filterLog + " enemyFaction=" + enemyFactionKey;
		filterLog = filterLog + " sharedRequired=" + FormatVehicleLabelList(definition.RequiredVehicleLabels);
		filterLog = filterLog + " sharedPreferred=" + FormatVehicleLabelList(definition.PreferredVehicleLabels);
		filterLog = filterLog + " friendlyRequired=" + FormatVehicleLabelList(friendlyRequiredLabels);
		filterLog = filterLog + " friendlyPreferred=" + FormatVehicleLabelList(friendlyPreferredLabels);
		filterLog = filterLog + " friendlyExcluded=" + FormatVehicleLabelList(friendlyExcludedLabels);
		filterLog = filterLog + " friendlyFallback=" + FormatVehicleLabelList(friendlyFallbackLabels);
		filterLog = filterLog + " enemyRequired=" + FormatVehicleLabelList(enemyRequiredLabels);
		filterLog = filterLog + " enemyPreferred=" + FormatVehicleLabelList(enemyPreferredLabels);
		filterLog = filterLog + " enemyExcluded=" + FormatVehicleLabelList(enemyExcludedLabels);
		filterLog = filterLog + " enemyFallback=" + FormatVehicleLabelList(enemyFallbackLabels);
		filterLog = filterLog + " friendlyPrefab=" + definition.FriendlyVehiclePrefab;
		filterLog = filterLog + " enemyPrefab=" + definition.EnemyVehiclePrefab;
		LogEvent("ambient_skirmish_vehicle_prefabs_selected", filterLog, false);
		return true;
	}

	protected JLH_AmbientSkirmishEventType SelectRandomAllowedEventType()
	{
		ref array<JLH_AmbientSkirmishEventType> candidates = {};
		if (m_bAllowInfantryClash)
			candidates.Insert(JLH_AmbientSkirmishEventType.INFANTRY_CLASH);
		if (m_bAllowSmallPatrolContact)
			candidates.Insert(JLH_AmbientSkirmishEventType.SMALL_PATROL_CONTACT);
		if (m_bAllowVehicles)
		{
			candidates.Insert(JLH_AmbientSkirmishEventType.VEHICLE_SKIRMISH);
			candidates.Insert(JLH_AmbientSkirmishEventType.CONVOY_AMBUSH_USSR_ATTACKS_US);
			candidates.Insert(JLH_AmbientSkirmishEventType.CONVOY_AMBUSH_US_ATTACKS_USSR);
		}
		if (m_bAllowVehicles && m_bAllowSupplyTrucks)
			candidates.Insert(JLH_AmbientSkirmishEventType.SUPPLY_TRUCK_RAID);
		if (m_bAllowCampAssault)
			candidates.Insert(JLH_AmbientSkirmishEventType.CAMP_ASSAULT);
		if (m_bAllowRoadblockBreakthrough)
			candidates.Insert(JLH_AmbientSkirmishEventType.ROADBLOCK_BREAKTHROUGH);
		if (m_bAllowDownedCrewRescue)
			candidates.Insert(JLH_AmbientSkirmishEventType.DOWNED_CREW_RESCUE);
		if (m_bAllowDownedHeliRescue)
			candidates.Insert(JLH_AmbientSkirmishEventType.DOWNED_US_HELI_RESCUE);
		if (m_bAllowReconTeamContact)
			candidates.Insert(JLH_AmbientSkirmishEventType.RECON_TEAM_CONTACT);

		if (candidates.IsEmpty())
			return JLH_AmbientSkirmishEventType.INFANTRY_CLASH;

		return candidates[Math.RandomInt(0, candidates.Count())];
	}

	protected void ApplyScenarioMarkerSettings(notnull JLH_DCF_AmbientSkirmishScenarioMarkerComponent marker)
	{
		m_eEventType = marker.GetEventType();
		m_bRandomizeEventType = false;
		m_eIntensity = marker.GetIntensity();
		m_sFriendlyFactionKey = marker.GetFriendlyFactionKey();
		m_sEnemyFactionKey = marker.GetEnemyFactionKey();
		m_iMinInfantryPerSide = marker.GetMinInfantryPerSide();
		m_iMaxInfantryPerSide = marker.GetMaxInfantryPerSide();
		m_bAllowInfantryClash = marker.AllowInfantryClash();
		m_bAllowSmallPatrolContact = marker.AllowSmallPatrolContact();
		m_bAllowCampAssault = marker.AllowCampAssault();
		m_bAllowRoadblockBreakthrough = marker.AllowRoadblockBreakthrough();
		m_bAllowDownedCrewRescue = marker.AllowDownedCrewRescue();
		m_bAllowDownedHeliRescue = marker.AllowDownedHeliRescue();
		m_bAllowReconTeamContact = marker.AllowReconTeamContact();
		m_bAllowVehicles = marker.AllowVehicles();
		m_bAllowSupplyTrucks = marker.AllowSupplyTrucks();
		m_bAllowStaticWeapons = marker.AllowStaticWeapons();
		m_bSpawnScenarioDressing = marker.SpawnScenarioDressing();
		m_sDownedUSHeliLivePrefab = marker.GetDownedUSHeliLivePrefab();
		m_iDownedUSHeliDestroyDelayMs = marker.GetDownedUSHeliDestroyDelayMs();
		m_iDownedCrewVehicleDestroyDelayMs = marker.GetDownedCrewVehicleDestroyDelayMs();
		m_fConvoyAmbushRoadSearchRadius = marker.GetConvoyAmbushRoadSearchRadius();
		m_sConvoyAmbushFrontVehiclePrefab = marker.GetConvoyAmbushFrontVehiclePrefab();
		m_sConvoyAmbushTroopTruckPrefab = marker.GetConvoyAmbushTroopTruckPrefab();
		m_sConvoyAmbushRearVehiclePrefab = marker.GetConvoyAmbushRearVehiclePrefab();
		m_sConvoyAmbushUSTroopGroupPrefab = marker.GetConvoyAmbushUSTroopGroupPrefab();
		m_sConvoyAmbushUSSRFrontGroupPrefab = marker.GetConvoyAmbushUSSRFrontGroupPrefab();
		m_sConvoyAmbushUSSRRearGroupPrefab = marker.GetConvoyAmbushUSSRRearGroupPrefab();
		m_iConvoyAmbushMinEnemyGroups = marker.GetConvoyAmbushMinEnemyGroups();
		m_iConvoyAmbushMaxEnemyGroups = marker.GetConvoyAmbushMaxEnemyGroups();
		m_bConvoyAmbushAllowAT = marker.GetConvoyAmbushAllowAT();
		m_eVehicleRole = marker.GetVehicleRole();
		m_fSpawnRadius = marker.GetSpawnRadius();
		m_fMinPlayerDistance = marker.GetMinPlayerDistance();
		m_fGroupSeparation = marker.GetGroupSeparation();

		m_aRequiredVehicleLabels = {};
		m_aPreferredVehicleLabels = {};
		m_aExcludedVehicleLabels = {};
		m_aFallbackVehicleLabels = {};
		marker.CopyVehicleLabels(m_aRequiredVehicleLabels, m_aPreferredVehicleLabels, m_aExcludedVehicleLabels, m_aFallbackVehicleLabels);

		m_aFriendlyRequiredVehicleLabels = {};
		m_aFriendlyPreferredVehicleLabels = {};
		m_aFriendlyExcludedVehicleLabels = {};
		m_aFriendlyFallbackVehicleLabels = {};
		marker.CopyFriendlyVehicleLabels(m_aFriendlyRequiredVehicleLabels, m_aFriendlyPreferredVehicleLabels, m_aFriendlyExcludedVehicleLabels, m_aFriendlyFallbackVehicleLabels);

		m_aEnemyRequiredVehicleLabels = {};
		m_aEnemyPreferredVehicleLabels = {};
		m_aEnemyExcludedVehicleLabels = {};
		m_aEnemyFallbackVehicleLabels = {};
		marker.CopyEnemyVehicleLabels(m_aEnemyRequiredVehicleLabels, m_aEnemyPreferredVehicleLabels, m_aEnemyExcludedVehicleLabels, m_aEnemyFallbackVehicleLabels);
	}

	protected bool SelectScenarioMarker(IEntity player, out JLH_DCF_AmbientSkirmishScenarioMarkerComponent selectedMarker, out string reason)
	{
		selectedMarker = null;
		reason = "";

		array<JLH_DCF_AmbientSkirmishScenarioMarkerComponent> markers = {};
		CollectScenarioMarkers(GetOwner(), markers);
		if (markers.IsEmpty())
			return false;

		bool found = false;
		float bestDistance = 99999999.0;
		foreach (JLH_DCF_AmbientSkirmishScenarioMarkerComponent marker : markers)
		{
			if (!marker || !marker.IsEnabled() || !marker.IsScenarioMarker() || !marker.GetOwner())
				continue;

			vector candidate = marker.GetOwner().GetOrigin();
			if (!IsAuthoredSkirmishMarkerCandidateValid(candidate, reason))
				continue;

			float distance = vector.Distance(player.GetOrigin(), candidate);
			if (!found || distance < bestDistance)
			{
				found = true;
				bestDistance = distance;
				selectedMarker = marker;
			}
		}

		if (!found && !markers.IsEmpty() && reason == "")
			reason = "no_valid_scenario_marker";

		return found;
	}

	bool SelectContactCenter(IEntity player, notnull JLH_AmbientSkirmishEventDefinition definition, out vector center, out string reason)
	{
		center = "0 0 0";
		reason = "";

		if (m_ActiveScenarioMarker && m_ActiveScenarioMarker.GetOwner())
		{
			center = JLH_AddonWorldQuery.GroundPosition(m_ActiveScenarioMarker.GetOwner().GetOrigin());
			if (!IsAuthoredSkirmishMarkerCandidateValid(center, reason))
				return false;

			LogEvent("ambient_skirmish_scenario_marker_center_selected", "pos=" + center.ToString(), false);
			return true;
		}

		if (m_bUseChildMarkers && SelectChildMarkerCenter(player, center, reason))
		{
			LogEvent("ambient_skirmish_marker_selected", "pos=" + center.ToString(), false);
			return true;
		}

		if (m_bUseChildMarkers && HasChildSkirmishMarkers())
		{
			if (reason == "")
				reason = "no_valid_child_marker";

			return false;
		}

		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.INFANTRY_CLASH)
			return SelectInfantryClashRadiusCenter(player, center, reason);

		return SelectRadiusCenter(player, center, reason);
	}

	protected bool SelectChildMarkerCenter(IEntity player, out vector center, out string reason)
	{
		center = "0 0 0";
		reason = "";

		array<JLH_DCF_AmbientSkirmishMarkerComponent> markers = {};
		CollectSkirmishMarkers(GetOwner(), markers);
		if (markers.IsEmpty())
			return false;

		bool found = false;
		float bestDistance = 99999999.0;
		foreach (JLH_DCF_AmbientSkirmishMarkerComponent marker : markers)
		{
			if (!marker || !marker.GetOwner())
				continue;

			vector candidate = marker.GetOwner().GetOrigin();
			if (!IsAuthoredSkirmishMarkerCandidateValid(candidate, reason))
				continue;

			float distance = vector.Distance(player.GetOrigin(), candidate);
			if (!found || distance < bestDistance)
			{
				found = true;
				bestDistance = distance;
				center = candidate;
			}
		}

		return found;
	}

	protected bool CollectValidExactSkirmishMarkers(IEntity player, notnull array<IEntity> exactMarkers, out string reason)
	{
		exactMarkers.Clear();
		reason = "";

		array<JLH_DCF_AmbientSkirmishMarkerComponent> markers = {};
		CollectSkirmishMarkers(GetOwner(), markers);
		if (markers.IsEmpty())
			return false;

		foreach (JLH_DCF_AmbientSkirmishMarkerComponent marker : markers)
		{
			if (!marker || !marker.GetOwner())
				continue;

			vector candidate = marker.GetOwner().GetOrigin();
			if (!IsAuthoredSkirmishMarkerCandidateValid(candidate, reason))
				continue;

			exactMarkers.Insert(marker.GetOwner());
		}

		if (exactMarkers.IsEmpty() && reason == "")
			reason = "no_valid_exact_skirmish_markers";

		return !exactMarkers.IsEmpty();
	}

	protected bool HasChildSkirmishMarkers()
	{
		array<JLH_DCF_AmbientSkirmishMarkerComponent> markers = {};
		CollectSkirmishMarkers(GetOwner(), markers);
		return !markers.IsEmpty();
	}

	protected bool IsAuthoredSkirmishMarkerCandidateValid(vector candidate, out string reason)
	{
		reason = "";
		if (JLH_AddonWorldQuery.IsWater(candidate))
		{
			reason = "marker_water";
			return false;
		}

		if (!IsAreaSafetyCandidateValid(candidate, reason))
		{
			reason = "marker_" + reason;
			return false;
		}

		return true;
	}

	protected void PartitionExactSkirmishMarkers(notnull array<IEntity> exactMarkers, vector friendlyPosition, vector enemyPosition, notnull array<IEntity> friendlyMarkers, notnull array<IEntity> enemyMarkers)
	{
		friendlyMarkers.Clear();
		enemyMarkers.Clear();

		foreach (IEntity markerEntity : exactMarkers)
		{
			if (!markerEntity)
				continue;

			vector markerPosition = markerEntity.GetOrigin();
			float friendlyDistance = vector.Distance(markerPosition, friendlyPosition);
			float enemyDistance = vector.Distance(markerPosition, enemyPosition);

			if (friendlyDistance <= enemyDistance)
				friendlyMarkers.Insert(markerEntity);
			else
				enemyMarkers.Insert(markerEntity);
		}

		LogEvent("ambient_skirmish_exact_markers_partitioned", "total=" + exactMarkers.Count().ToString() + " friendly=" + friendlyMarkers.Count().ToString() + " enemy=" + enemyMarkers.Count().ToString(), false);
	}

	protected bool SelectRadiusCenter(IEntity player, out vector center, out string reason)
	{
		center = "0 0 0";
		reason = "";

		IEntity owner = GetOwner();
		if (!owner)
		{
			reason = "area_missing";
			return false;
		}

		bool found = false;
		float bestDistance = 99999999.0;
		for (int i = 0; i < SPAWN_CANDIDATE_ATTEMPTS; i++)
		{
			vector candidate = JLH_AddonWorldQuery.GroundPosition(owner.GetOrigin() + JLH_AddonWorldQuery.RandomOffset(0.0, GetEffectiveSpawnRadius()));
			if (!IsCenterCandidateValid(candidate, reason))
				continue;

			float distance = vector.Distance(player.GetOrigin(), candidate);
			if (!found || distance < bestDistance)
			{
				found = true;
				bestDistance = distance;
				center = candidate;
			}
		}

		if (!found && reason == "")
			reason = "no_valid_contact_center";

		return found;
	}

	protected bool SelectInfantryClashRadiusCenter(IEntity player, out vector center, out string reason)
	{
		center = "0 0 0";
		reason = "";

		IEntity owner = GetOwner();
		if (!owner)
		{
			reason = "area_missing";
			return false;
		}

		bool found = false;
		int bestScore = -1;
		float bestDistance = 99999999.0;
		string bestContext = "";
		for (int i = 0; i < SPAWN_CANDIDATE_ATTEMPTS * 2; i++)
		{
			vector candidate = JLH_AddonWorldQuery.GroundPosition(owner.GetOrigin() + JLH_AddonWorldQuery.RandomOffset(0.0, GetEffectiveSpawnRadius()));
			if (!IsCenterCandidateValid(candidate, reason))
				continue;

			string context;
			int contextScore = ScoreInfantryClashRandomContext(candidate, context);
			float distance = vector.Distance(player.GetOrigin(), candidate);
			if (!found || contextScore > bestScore || (contextScore == bestScore && distance < bestDistance))
			{
				found = true;
				bestScore = contextScore;
				bestDistance = distance;
				bestContext = context;
				center = candidate;
			}
		}

		if (!found)
		{
			if (reason == "")
				reason = "no_valid_infantry_clash_contact_center";

			return false;
		}

		LogEvent("ambient_skirmish_infantry_clash_random_center_selected", "context=" + bestContext + " pos=" + center.ToString() + " score=" + bestScore.ToString(), false);
		reason = "";
		return true;
	}

	protected int ScoreInfantryClashRandomContext(vector candidate, out string context)
	{
		context = "open_ground";

		bool nearStructure = HasSkirmishStructureNearby(candidate, 95.0);
		bool naturalCover = false;
		if (!nearStructure)
			naturalCover = HasSkirmishNaturalCoverNearby(candidate, 80.0);

		float slope = JLH_AddonWorldQuery.GetSlopeDegrees(candidate);
		if (nearStructure)
		{
			context = "town_edge";
			if (slope <= 12.0)
				return 6;

			return 5;
		}

		if (naturalCover)
		{
			context = "forest_edge";
			if (slope <= 16.0)
				return 5;

			return 4;
		}

		if (slope <= 8.0)
		{
			context = "open_field";
			return 4;
		}

		if (slope <= 18.0)
			return 2;

		return 1;
	}

	protected static bool HasSkirmishStructureNearby(vector position, float radius)
	{
		if (!GetGame() || !GetGame().GetWorld() || radius <= 0.0)
			return false;

		s_vContextQueryPosition = position;
		s_fContextQueryRadius = radius;
		s_bContextQueryFound = false;
		GetGame().GetWorld().QueryEntitiesBySphere(position, radius, QuerySkirmishStructureContextEntity, null, EQueryEntitiesFlags.ALL);
		return s_bContextQueryFound;
	}

	protected static bool HasSkirmishNaturalCoverNearby(vector position, float radius)
	{
		if (!GetGame() || !GetGame().GetWorld() || radius <= 0.0)
			return false;

		s_vContextQueryPosition = position;
		s_fContextQueryRadius = radius;
		s_bContextQueryFound = false;
		GetGame().GetWorld().QueryEntitiesBySphere(position, radius, QuerySkirmishNaturalCoverContextEntity, null, EQueryEntitiesFlags.ALL);
		return s_bContextQueryFound;
	}

	protected static bool QuerySkirmishStructureContextEntity(IEntity entity)
	{
		if (!entity || s_bContextQueryFound)
			return false;

		if (vector.Distance(entity.GetOrigin(), s_vContextQueryPosition) > s_fContextQueryRadius)
			return true;

		string token = entity.GetName();
		token += " ";
		token += entity.Type().ToString();

		EntityPrefabData prefabData = entity.GetPrefabData();
		if (prefabData)
		{
			token += " ";
			token += prefabData.GetPrefabName();
		}

		token.ToLower();
		if (token.Contains("building") || token.Contains("house") || token.Contains("barrack") || token.Contains("shed") || token.Contains("hangar") || token.Contains("garage") || token.Contains("farm") || token.Contains("industrial") || token.Contains("compound") || token.Contains("chapel") || token.Contains("guard"))
		{
			s_bContextQueryFound = true;
			return false;
		}

		return true;
	}

	protected static bool QuerySkirmishNaturalCoverContextEntity(IEntity entity)
	{
		if (!entity || s_bContextQueryFound)
			return false;

		if (vector.Distance(entity.GetOrigin(), s_vContextQueryPosition) > s_fContextQueryRadius)
			return true;

		string token = entity.GetName();
		token += " ";
		token += entity.Type().ToString();

		EntityPrefabData prefabData = entity.GetPrefabData();
		if (prefabData)
		{
			token += " ";
			token += prefabData.GetPrefabName();
		}

		token.ToLower();
		if (token.Contains("tree") || token.Contains("forest") || token.Contains("bush") || token.Contains("wood") || token.Contains("pine") || token.Contains("spruce") || token.Contains("birch") || token.Contains("hedge") || token.Contains("vegetation") || token.Contains("rock"))
		{
			s_bContextQueryFound = true;
			return false;
		}

		return true;
	}

	bool BuildGroupPositions(IEntity player, vector center, float groupSeparation, out vector friendlyPosition, out vector enemyPosition, out string reason)
	{
		friendlyPosition = "0 0 0";
		enemyPosition = "0 0 0";
		reason = "";

		vector baseDirection = DirectionFromTo(player.GetOrigin(), center);
		for (int i = 0; i < SPAWN_CANDIDATE_ATTEMPTS; i++)
		{
			vector direction = baseDirection;
			if (i > 0)
				direction = JLH_AddonWorldQuery.DirectionFromAngle(Math.RandomFloat(0.0, 360.0));

			float halfSeparation = groupSeparation * 0.5;
			vector friendlyCandidate = JLH_AddonWorldQuery.GroundPosition(center - (direction * halfSeparation));
			vector enemyCandidate = JLH_AddonWorldQuery.GroundPosition(center + (direction * halfSeparation));

			if (!IsGroupPositionValid(friendlyCandidate, reason))
				continue;

			if (!IsGroupPositionValid(enemyCandidate, reason))
				continue;

			friendlyPosition = friendlyCandidate;
			enemyPosition = enemyCandidate;
			LogEvent("ambient_skirmish_spawn_positions_selected", "center=" + center.ToString() + " friendly=" + friendlyPosition.ToString() + " enemy=" + enemyPosition.ToString(), false);
			return true;
		}

		if (reason == "")
			reason = "no_valid_group_positions";

		return false;
	}

	bool BuildScenarioGroupPositions(notnull JLH_AmbientSkirmishEventDefinition definition, IEntity player, vector center, float groupSeparation, out vector friendlyPosition, out vector enemyPosition, out string reason)
	{
		friendlyPosition = "0 0 0";
		enemyPosition = "0 0 0";
		reason = "";

		float shapedSeparation = Math.Clamp(groupSeparation * Math.Max(0.35, definition.LayoutSeparationScale), 20.0, 760.0);
		if (!BuildGroupPositions(player, center, shapedSeparation, friendlyPosition, enemyPosition, reason))
			return false;

		vector direction = DirectionFromTo(player.GetOrigin(), center);
		if (vector.Distance(direction, "0 0 0") <= 0.001)
			direction = DirectionFromTo(friendlyPosition, enemyPosition);

		vector lateral = Vector(-direction[2], 0.0, direction[0]);
		float halfSeparation = shapedSeparation * 0.5;
		vector friendlyCandidate = friendlyPosition;
		vector enemyCandidate = enemyPosition;
		bool useCustomShape = true;

		JLH_AmbientSkirmishEventType eventType = definition.SkirmishEventType;
		if (eventType == JLH_AmbientSkirmishEventType.CAMP_ASSAULT)
		{
			friendlyCandidate = center + (lateral * definition.LayoutLateralOffset);
			enemyCandidate = center - (direction * Math.Max(90.0, halfSeparation + Math.AbsFloat(definition.EnemyDepthOffset))) - (lateral * (definition.LayoutLateralOffset * 0.35));
		}
		else if (eventType == JLH_AmbientSkirmishEventType.ROADBLOCK_BREAKTHROUGH)
		{
			friendlyCandidate = center - (direction * Math.Max(110.0, halfSeparation + Math.AbsFloat(definition.FriendlyDepthOffset))) - (lateral * (definition.LayoutLateralOffset * 0.25));
			enemyCandidate = center + (lateral * definition.LayoutLateralOffset);
		}
		else if (eventType == JLH_AmbientSkirmishEventType.RECON_TEAM_CONTACT)
		{
			friendlyCandidate = center - (direction * halfSeparation) - (lateral * Math.AbsFloat(definition.LayoutLateralOffset));
			enemyCandidate = center + (direction * halfSeparation) + (lateral * Math.AbsFloat(definition.LayoutLateralOffset));
		}
		else if (eventType == JLH_AmbientSkirmishEventType.CONVOY_AMBUSH_USSR_ATTACKS_US || eventType == JLH_AmbientSkirmishEventType.SUPPLY_TRUCK_RAID)
		{
			friendlyCandidate = center - (direction * Math.Max(95.0, halfSeparation + Math.AbsFloat(definition.FriendlyDepthOffset)));
			enemyCandidate = center + (lateral * Math.AbsFloat(definition.LayoutLateralOffset)) + (direction * 18.0);
		}
		else if (eventType == JLH_AmbientSkirmishEventType.CONVOY_AMBUSH_US_ATTACKS_USSR)
		{
			friendlyCandidate = center + (lateral * Math.AbsFloat(definition.LayoutLateralOffset)) + (direction * 18.0);
			enemyCandidate = center - (direction * Math.Max(95.0, halfSeparation + Math.AbsFloat(definition.EnemyDepthOffset)));
		}
		else if (eventType == JLH_AmbientSkirmishEventType.VEHICLE_SKIRMISH)
		{
			friendlyCandidate = center - (direction * halfSeparation) - (lateral * (definition.LayoutLateralOffset * 0.5));
			enemyCandidate = center + (direction * halfSeparation) + (lateral * (definition.LayoutLateralOffset * 0.5));
		}
		else
		{
			useCustomShape = false;
		}

		if (useCustomShape && TryUseScenarioGroupPositions(friendlyCandidate, enemyCandidate, friendlyPosition, enemyPosition, reason))
		{
			LogScenarioSpacingProfile(definition, center, friendlyPosition, enemyPosition, shapedSeparation, "custom");
			return true;
		}

		LogScenarioSpacingProfile(definition, center, friendlyPosition, enemyPosition, shapedSeparation, "default");
		reason = "";
		return true;
	}

	protected bool TryUseScenarioGroupPositions(vector friendlyCandidate, vector enemyCandidate, out vector friendlyPosition, out vector enemyPosition, out string reason)
	{
		friendlyPosition = JLH_AddonWorldQuery.GroundPosition(friendlyCandidate);
		enemyPosition = JLH_AddonWorldQuery.GroundPosition(enemyCandidate);

		if (!IsGroupPositionValid(friendlyPosition, reason))
			return false;

		if (!IsGroupPositionValid(enemyPosition, reason))
			return false;

		reason = "";
		return true;
	}

	protected bool IsCenterCandidateValid(vector candidate, out string reason)
	{
		reason = "";
		if (JLH_AddonWorldQuery.IsWater(candidate))
		{
			reason = "water";
			return false;
		}

		if (m_fMinPlayerDistance > 0.0 && JLH_AddonWorldQuery.HasLiveDeployedPlayerWithin(candidate, m_fMinPlayerDistance, ""))
		{
			reason = "too_close_to_player";
			return false;
		}

		if (!IsAreaSafetyCandidateValid(candidate, reason))
			return false;

		return true;
	}

	protected bool IsAreaSafetyCandidateValid(vector candidate, out string reason)
	{
		reason = "";
		string safetyFactionKey = GetSkirmishSafetyFactionKey();

		if (m_fUSHQHardSafetyRadius > 0.0 && JLH_AmbientThreatSpawnSelector.IsInsideUSHQSafeZone(candidate, m_fUSHQHardSafetyRadius, safetyFactionKey))
		{
			reason = "hq_safezone";
			return false;
		}

		if (m_fUSBaseCoreSafetyRadius > 0.0 && JLH_AddonWorldQuery.IsNearFactionBase(candidate, m_fUSBaseCoreSafetyRadius, ""))
		{
			reason = "base_core";
			return false;
		}

		float usOwnedBaseSoftSafetyRadius = GetUSOwnedBaseSoftSafetyRadius();
		if (!m_bAllowNearUSBases && usOwnedBaseSoftSafetyRadius > 0.0 && JLH_AddonWorldQuery.IsNearFactionBase(candidate, usOwnedBaseSoftSafetyRadius, safetyFactionKey))
		{
			reason = "us_owned_base_soft_zone";
			return false;
		}

		return true;
	}

	protected string GetSkirmishSafetyFactionKey()
	{
		if (m_sFriendlyFactionKey != "")
			return m_sFriendlyFactionKey;

		return "US";
	}

	protected float GetUSOwnedBaseSoftSafetyRadius()
	{
		JLH_AmbientThreatsConfigData config = JLH_AmbientThreatsConfig.Get();
		if (config && config.AmbientThreatsUSOwnedBaseSoftSafetyRadius > 0.0)
			return config.AmbientThreatsUSOwnedBaseSoftSafetyRadius;

		return 0.0;
	}

	protected bool IsGroupPositionValid(vector candidate, out string reason)
	{
		if (!IsCenterCandidateValid(candidate, reason))
			return false;

		return true;
	}

	void BuildScenarioScene(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		int spawned = 0;
		JLH_AmbientSkirmishEventType eventType = definition.SkirmishEventType;
		if (!ShouldSpawnScenarioDressing() && eventType != JLH_AmbientSkirmishEventType.DOWNED_CREW_RESCUE && eventType != JLH_AmbientSkirmishEventType.DOWNED_US_HELI_RESCUE)
			return;

		if (eventType == JLH_AmbientSkirmishEventType.DOWNED_CREW_RESCUE)
			spawned = SpawnDownedCrewScene(record, definition);
		else if (eventType == JLH_AmbientSkirmishEventType.DOWNED_US_HELI_RESCUE)
			spawned = SpawnDownedHeliScene(record);
		else if (eventType == JLH_AmbientSkirmishEventType.CAMP_ASSAULT)
			spawned = SpawnCampAssaultLegacyScene(record);
		else if (eventType == JLH_AmbientSkirmishEventType.ROADBLOCK_BREAKTHROUGH)
			spawned = SpawnRoadblockScene(record);
		else if (eventType == JLH_AmbientSkirmishEventType.CONVOY_AMBUSH_USSR_ATTACKS_US || eventType == JLH_AmbientSkirmishEventType.CONVOY_AMBUSH_US_ATTACKS_USSR || eventType == JLH_AmbientSkirmishEventType.SUPPLY_TRUCK_RAID)
			spawned = SpawnAmbushedConvoyScene(record);
		else if (eventType == JLH_AmbientSkirmishEventType.VEHICLE_SKIRMISH)
			spawned = SpawnVehicleBattleScene(record);

		if (spawned > 0)
			LogEvent("ambient_skirmish_scene_dressing_spawned", "id=" + record.Id.ToString() + " event=" + definition.SkirmishEventName + " props=" + spawned.ToString(), true);
	}

	protected bool ShouldSpawnScenarioDressing()
	{
		// Temporarily disabled while ambient skirmish gameplay is stabilized.
		return false;
	}

	protected int SpawnDownedCrewScene(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		int spawned = 0;
		string source;
		string reason;
		ResourceName crashVehicle = ResolveDownedCrewCrashVehiclePrefab(definition, source, reason);

		vector sceneDir = ResolveDownedCrewSceneDirection(null, record.Center);

		spawned += SpawnSceneLiveCrashVehicleWithPose(record, crashVehicle, record.Center, "downed_crew_live_vehicle", m_iDownedCrewVehicleDestroyDelayMs, sceneDir, true);
		return spawned;
	}

	protected int SpawnDownedHeliScene(notnull JLH_AmbientSkirmishRecord record)
	{
		int spawned = 0;
		spawned += SpawnSceneLiveCrashVehicle(record, ResolveDownedUSHeliLivePrefab(), record.Center, "downed_us_heli_live", m_iDownedUSHeliDestroyDelayMs);
		return spawned;
	}


	protected int SpawnCampAssaultLegacyScene(notnull JLH_AmbientSkirmishRecord record)
	{
		return 0;
	}

	protected int SpawnRoadblockScene(notnull JLH_AmbientSkirmishRecord record)
	{
		int spawned = 0;
		spawned += SpawnSceneProp(record, SCENE_ROADBLOCK_USSR, record.Center, "roadblock_anchor");
		spawned += SpawnSceneVehicle(record, SCENE_USSR_UAZ, SceneOffset(record.Center, 315.0, 18.0), "roadblock_disabled_uaz");
		return spawned;
	}

	protected int SpawnAmbushedConvoyScene(notnull JLH_AmbientSkirmishRecord record)
	{
		int spawned = 0;
		spawned += SpawnSceneVehicle(record, SCENE_USSR_URAL, SceneOffset(record.Center, 315.0, 18.0), "convoy_disabled_ural");
		spawned += SpawnSceneProp(record, SCENE_ROADBLOCK_USSR, SceneOffset(record.Center, 0.0, 8.0), "convoy_roadblock");
		spawned += SpawnSceneProp(record, SCENE_TIRE_UAZ, SceneOffset(record.Center, 190.0, 12.0), "convoy_tire_debris");
		spawned += SpawnSceneProp(record, SCENE_BARREL_CLUTTER, SceneOffset(record.Center, 280.0, 16.0), "convoy_debris_barrels");
		return spawned;
	}

	protected int SpawnVehicleBattleScene(notnull JLH_AmbientSkirmishRecord record)
	{
		return 0;
	}

	protected int SpawnSceneVehicle(notnull JLH_AmbientSkirmishRecord record, ResourceName prefab, vector position, string label)
	{
		if (prefab == "" || !JLH_AddonSpawnUtility.CanLoadPrefab(prefab))
			return 0;

		vector spawnPosition = JLH_AddonWorldQuery.GroundPosition(position);
		if (JLH_AddonWorldQuery.IsWaterOrNearWater(spawnPosition, 4.0))
			return 0;

		IEntity entity = JLH_AddonSpawnUtility.SpawnEntity(prefab, spawnPosition, "ambient_skirmish_scene_" + label);
		if (!entity)
			return 0;

		JLH_AddonSpawnUtility.DrainVehicleFuel(entity);
		record.Runtime.TrackVehicle(entity, "ambient_skirmish_scene_vehicle_" + label);
		return 1;
	}

	protected int SpawnSceneLiveCrashVehicle(notnull JLH_AmbientSkirmishRecord record, ResourceName prefab, vector position, string label, int destroyDelayMs)
	{
		return SpawnSceneLiveCrashVehicleWithPose(record, prefab, position, label, destroyDelayMs, "0 0 1", false);
	}

	protected int SpawnSceneLiveCrashVehicleWithPose(notnull JLH_AmbientSkirmishRecord record, ResourceName prefab, vector position, string label, int destroyDelayMs, vector roadDir, bool useWreckPose)
	{
		if (prefab == "" || prefab == ResourceName.Empty)
		{
			LogEvent("ambient_skirmish_live_crash_vehicle_missing", "id=" + record.Id.ToString() + " label=" + label, true);
			return 0;
		}

		if (!JLH_AddonSpawnUtility.CanLoadPrefab(prefab))
		{
			LogEvent("ambient_skirmish_live_crash_vehicle_load_failed", "id=" + record.Id.ToString() + " label=" + label + " prefab=" + prefab, true);
			return 0;
		}

		vector spawnPosition = JLH_AddonWorldQuery.GroundPosition(position);
		if (JLH_AddonWorldQuery.IsWaterOrNearWater(spawnPosition, 4.0))
			return 0;

		IEntity entity = JLH_AddonSpawnUtility.SpawnEntity(prefab, spawnPosition, "ambient_skirmish_scene_" + label);
		if (!entity)
			return 0;

		if (useWreckPose)
			ApplyDownedCrewVehicleWreckTransform(entity, spawnPosition, roadDir);

		record.Runtime.TrackVehicle(entity, "ambient_skirmish_scene_live_crash_vehicle_" + label);
		ScheduleSceneVehicleDestruction(entity, record.Id, label, destroyDelayMs);
		return 1;
	}

	protected void ApplyDownedCrewVehicleWreckTransform(IEntity vehicle, vector position, vector roadDir)
	{
		if (!vehicle)
			return;

		vector forward = NormalizeConvoyAmbushDirection(roadDir);
		vector right = Vector(forward[2], 0.0, -forward[0]);
		float side = 1.0;
		if (Math.RandomInt(0, 2) == 0)
			side = -1.0;

		vector skewedForward = NormalizeConvoyAmbushDirection(forward + (right * side * 0.46));
		vector skewedRight = Vector(skewedForward[2], 0.0, -skewedForward[0]);
		vector worldUp = Vector(0.0, 1.0, 0.0);
		float rollAmount = 0.24 * side;
		vector tiltedRight = NormalizeConvoyAmbushVector3D((skewedRight * 0.96) + (worldUp * rollAmount), skewedRight);
		vector tiltedUp = NormalizeConvoyAmbushVector3D((worldUp * 0.96) - (skewedRight * rollAmount), worldUp);
		vector wreckPos = JLH_AddonWorldQuery.GroundPosition(position + (right * side * 1.4) + (forward * Math.RandomFloat(-1.2, 1.2)));
		wreckPos[1] = wreckPos[1] + 0.18;

		SetConvoyAmbushVehicleTransform(vehicle, wreckPos, skewedForward, tiltedRight, tiltedUp);
		LogEvent("downed_crew_wreck_pose", "pos=" + wreckPos.ToString() + " dir=" + skewedForward.ToString(), true);
	}

	protected ResourceName ResolveDownedUSHeliLivePrefab()
	{
		if (m_sDownedUSHeliLivePrefab != "" && m_sDownedUSHeliLivePrefab != ResourceName.Empty)
			return m_sDownedUSHeliLivePrefab;

		return SCENE_US_HELI_LIVE;
	}

	protected ResourceName ResolveDownedCrewCrashVehiclePrefab(notnull JLH_AmbientSkirmishEventDefinition definition, out string source, out string reason)
	{
		source = "";
		reason = "";

		if (JLH_AddonSpawnUtility.CanLoadPrefab(SCENE_US_DOWNED_HUMVEE))
		{
			source = "default_humvee";
			LogEvent("ambient_skirmish_scene_crash_vehicle_selected", "event=" + definition.SkirmishEventName + " faction=" + m_sFriendlyFactionKey + " prefab=" + SCENE_US_DOWNED_HUMVEE + " source=" + source + " preferredMatches=exact", true);
			return SCENE_US_DOWNED_HUMVEE;
		}

		JLH_CORE_VehicleLabelSelection selection;
		if (JLH_CORE_CompatibilityResolver.TrySelectVehiclePrefabByLabelFilters(m_sFriendlyFactionKey, definition.RequiredVehicleLabels, definition.PreferredVehicleLabels, definition.ExcludedVehicleLabels, definition.FallbackVehicleLabels, selection, reason))
		{
			source = selection.Source;
			LogEvent("ambient_skirmish_scene_crash_vehicle_selected", "event=" + definition.SkirmishEventName + " faction=" + m_sFriendlyFactionKey + " prefab=" + selection.Prefab + " source=" + selection.Source + " preferredMatches=" + selection.PreferredMatches.ToString(), true);
			return selection.Prefab;
		}

		source = "fallback_default";
		LogEvent("ambient_skirmish_scene_crash_vehicle_fallback", "event=" + definition.SkirmishEventName + " reason=" + reason + " prefab=" + SCENE_US_DOWNED_TRUCK, true);
		return SCENE_US_DOWNED_TRUCK;
	}

	protected void ScheduleSceneVehicleDestruction(IEntity vehicle, int recordId, string label, int delayMs)
	{
		if (!vehicle)
			return;

		int safeDelayMs = Math.Clamp(delayMs, 0, 10000);
		if (!GetGame() || !GetGame().GetCallqueue() || safeDelayMs <= 0)
		{
			DestroySceneVehicleDelayed(vehicle, recordId, label);
			return;
		}

		GetGame().GetCallqueue().CallLater(JLH_AmbientSkirmish_DestroySceneVehicleDelayed, safeDelayMs, false, this, vehicle, recordId, label);
	}

	void DestroySceneVehicleDelayed(IEntity vehicle, int recordId, string label)
	{
		bool destroyed = JLH_AddonSpawnUtility.DestroyRuntimeEntity(vehicle, "ambient_skirmish_scene_live_crash_vehicle_destroy");
		LogEvent("ambient_skirmish_scene_live_crash_vehicle_destroyed", "id=" + recordId.ToString() + " label=" + label + " destroyed=" + JLH_DCF_NodeDebug.BoolLabel(destroyed), true);
	}

	protected int SpawnSceneProp(notnull JLH_AmbientSkirmishRecord record, ResourceName prefab, vector position, string label)
	{
		if (prefab == "" || !JLH_AddonSpawnUtility.CanLoadPrefab(prefab))
			return 0;

		vector spawnPosition = JLH_AddonWorldQuery.GroundPosition(position);
		if (JLH_AddonWorldQuery.IsWaterOrNearWater(spawnPosition, 3.0))
			return 0;

		IEntity entity = JLH_AddonSpawnUtility.SpawnEntity(prefab, spawnPosition, "ambient_skirmish_scene_" + label);
		if (!entity)
			return 0;

		record.Runtime.TrackEntity(entity, "ambient_skirmish_scene_prop_" + label);
		return 1;
	}

	protected vector SceneOffset(vector center, float angleDeg, float distance)
	{
		float jitter = Math.RandomFloat(-14.0, 14.0);
		float jitterDistance = Math.RandomFloat(-2.5, 2.5);
		return JLH_AddonWorldQuery.GroundPosition(center + (JLH_AddonWorldQuery.DirectionFromAngle(angleDeg + jitter) * Math.Max(0.0, distance + jitterDistance)));
	}

	protected bool ShouldUseSceneAnchorCombat(notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		JLH_AmbientSkirmishEventType eventType = definition.SkirmishEventType;
		return eventType == JLH_AmbientSkirmishEventType.DOWNED_CREW_RESCUE || eventType == JLH_AmbientSkirmishEventType.DOWNED_US_HELI_RESCUE || eventType == JLH_AmbientSkirmishEventType.CAMP_ASSAULT;
	}

	protected void AssignCampAssaultBehavior(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishEventDefinition definition, vector campFacing)
	{
		LogScenarioCombatProfile(record, definition);

		for (int i = 0; i < record.FriendlyGroups.Count(); i++)
		{
			SCR_AIGroup friendlyGroup = record.FriendlyGroups[i];
			if (!friendlyGroup)
				continue;

			vector defendPosition = BuildCampAssaultHoldPosition(record.Center, campFacing, i, record.FriendlyGroups.Count());
			bool assigned = JLH_AddonSpawnUtility.AssignDefend(friendlyGroup, defendPosition, record.Runtime, "camp_assault_us_hold_" + i.ToString());
			LogEvent("camp_assault_hold_orders", "id=" + record.Id.ToString() + " group=" + i.ToString() + " target=" + defendPosition.ToString() + " assigned=" + JLH_DCF_NodeDebug.BoolLabel(assigned), true);
		}

		for (int j = 0; j < record.EnemyGroups.Count(); j++)
		{
			SCR_AIGroup enemyGroup = record.EnemyGroups[j];
			if (!enemyGroup)
				continue;

			vector assaultTarget = BuildCampAssaultAssaultTarget(record.Center, campFacing, j, record.EnemyGroups.Count());
			JLH_AddonSpawnUtility.AssignMoveAndSearch(enemyGroup, assaultTarget, 58.0, Math.Max(2, definition.EnemySearchPoints), record.Runtime, "camp_assault_ussr_assault_" + j.ToString());
			LogEvent("camp_assault_assault_orders", "id=" + record.Id.ToString() + " group=" + j.ToString() + " target=" + assaultTarget.ToString(), true);
		}

		record.Runtime.RefreshExclusions("camp_assault_behavior_refresh");
	}

	protected vector BuildCampAssaultHoldPosition(vector center, vector facing, int groupIndex, int groupCount)
	{
		vector forward = NormalizeConvoyAmbushDirection(facing);
		vector right = Vector(forward[2], 0.0, -forward[0]);
		float slot = groupIndex - ((groupCount - 1) * 0.5);
		vector candidate = JLH_AddonWorldQuery.GroundPosition(center + (right * slot * 5.0));
		if (!JLH_AddonWorldQuery.IsWaterOrNearWater(candidate, 3.0))
			return candidate;

		return JLH_AddonWorldQuery.GroundPosition(center);
	}

	protected vector BuildCampAssaultAssaultTarget(vector center, vector facing, int groupIndex, int groupCount)
	{
		vector forward = NormalizeConvoyAmbushDirection(facing);
		vector right = Vector(forward[2], 0.0, -forward[0]);
		float slot = groupIndex - ((groupCount - 1) * 0.5);
		vector candidate = JLH_AddonWorldQuery.GroundPosition(center + (right * slot * 12.0) + (forward * Math.RandomFloat(-7.0, 7.0)));
		if (!JLH_AddonWorldQuery.IsWaterOrNearWater(candidate, 3.0))
			return candidate;

		return JLH_AddonWorldQuery.GroundPosition(center);
	}

	protected void AssignRoadblockAssaultBehavior(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		LogScenarioCombatProfile(record, definition);

		for (int i = 0; i < record.EnemyGroups.Count(); i++)
		{
			SCR_AIGroup enemyGroup = record.EnemyGroups[i];
			if (!enemyGroup)
				continue;

			JLH_AddonSpawnUtility.AssignDefend(enemyGroup, record.Center, record.Runtime, "roadblock_assault_ussr_defend_" + i.ToString());
		}

		for (int j = 0; j < record.FriendlyGroups.Count(); j++)
		{
			SCR_AIGroup friendlyGroup = record.FriendlyGroups[j];
			if (!friendlyGroup)
				continue;

			AssignRoadblockAssaultSearchAndDestroy(friendlyGroup, record.Center, record.Runtime, "roadblock_assault_us_search_destroy_" + j.ToString());
		}

		record.Runtime.RefreshExclusions("roadblock_assault_behavior_refresh");
	}

	protected bool AssignRoadblockAssaultSearchAndDestroy(SCR_AIGroup group, vector target, notnull JLH_AddonRuntimeEntityRegistry registry, string reason)
	{
		if (!group)
			return false;

		JLH_DCF_NodeWaypointResolver.ClearGroupWaypoints(group);
		AIWaypoint waypoint = JLH_AddonSpawnUtility.SpawnWaypoint(ROADBLOCK_ASSAULT_WAYPOINT_SEARCH_AND_DESTROY, target, reason + "_waypoint");
		if (!waypoint)
			return false;

		group.AddWaypoint(waypoint);
		registry.TrackWaypoint(waypoint, reason + "_waypoint");
		JLH_AddonSpawnUtility.ActivateGroup(group);
		return true;
	}

	protected void AssignCombatBehavior(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		LogScenarioCombatProfile(record, definition);

		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.INFANTRY_CLASH)
		{
			AssignInfantryClashMoveIntoContactBehavior(record, definition);
			record.Runtime.RefreshExclusions("ambient_skirmish_behavior_refresh");
			return;
		}

		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.ROADBLOCK_BREAKTHROUGH)
		{
			AssignRoadblockBreakthroughBehavior(record, definition);
			record.Runtime.RefreshExclusions("ambient_skirmish_behavior_refresh");
			return;
		}

		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.RECON_TEAM_CONTACT)
		{
			AssignReconFlankBehavior(record, definition);
			record.Runtime.RefreshExclusions("ambient_skirmish_behavior_refresh");
			return;
		}

		if (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.SMALL_PATROL_CONTACT)
		{
			AssignSmallPatrolContactBehavior(record, definition);
			record.Runtime.RefreshExclusions("ambient_skirmish_behavior_refresh");
			return;
		}

		bool useSceneAnchor = ShouldUseSceneAnchorCombat(definition);
		bool useCrashSurvivorLayout = ShouldUseCrashSurvivorLayout(definition);
		vector friendlyAnchor = record.FriendlyPosition;
		vector enemyTargetAnchor = record.FriendlyPosition;
		if (useSceneAnchor)
		{
			friendlyAnchor = record.Center;
			enemyTargetAnchor = record.Center;
		}

		if (!useCrashSurvivorLayout)
		{
			for (int i = 0; i < record.FriendlyGroups.Count(); i++)
			{
				SCR_AIGroup friendlyGroup = record.FriendlyGroups[i];
				if (!friendlyGroup)
					continue;

				vector friendlyDefendPosition = BuildComposedGroupPosition(friendlyAnchor, i, record.FriendlyGroups.Count());
				JLH_AddonSpawnUtility.AssignDefend(friendlyGroup, friendlyDefendPosition, record.Runtime, "ambient_skirmish_friendly_defend_" + i.ToString());
			}
		}

		for (int j = 0; j < record.EnemyGroups.Count(); j++)
		{
			SCR_AIGroup enemyGroup = record.EnemyGroups[j];
			if (!enemyGroup)
				continue;

			vector enemyTarget = BuildComposedGroupPosition(enemyTargetAnchor, j + 1, record.EnemyGroups.Count() + 1);
			JLH_AddonSpawnUtility.AssignMoveAndSearch(enemyGroup, enemyTarget, Math.Max(80.0, m_fGroupSeparation * Math.Max(0.1, definition.SeparationMultiplier)), definition.EnemySearchPoints, record.Runtime, "ambient_skirmish_enemy_assault_" + definition.SkirmishEventName + "_" + j.ToString());
		}

		record.Runtime.RefreshExclusions("ambient_skirmish_behavior_refresh");
	}

	protected void AssignInfantryClashMoveIntoContactBehavior(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		vector direction = DirectionFromTo(record.FriendlyPosition, record.EnemyPosition);
		if (vector.Distance(direction, "0 0 0") <= 0.001)
			direction = DirectionFromTo(record.FriendlyPosition, record.Center);
		if (vector.Distance(direction, "0 0 0") <= 0.001)
			direction = JLH_AddonWorldQuery.DirectionFromAngle(Math.RandomFloat(0.0, 360.0));

		vector lateral = Vector(-direction[2], 0.0, direction[0]);
		for (int i = 0; i < record.FriendlyGroups.Count(); i++)
		{
			SCR_AIGroup friendlyGroup = record.FriendlyGroups[i];
			if (!friendlyGroup)
				continue;

			vector friendlyTarget = BuildInfantryClashLaneTarget(record.Center, lateral, i, record.FriendlyGroups.Count());
			bool friendlyAssigned = JLH_AddonSpawnUtility.AssignSingleWaypoint(friendlyGroup, JLH_AddonSpawnUtility.WAYPOINT_MOVE, friendlyTarget, record.Runtime, "ambient_skirmish_infantry_clash_friendly_move_" + i.ToString());
			LogEvent("ambient_skirmish_infantry_clash_move_assigned", "id=" + record.Id.ToString() + " side=friendly group=" + i.ToString() + " target=" + friendlyTarget.ToString() + " result=" + JLH_DCF_NodeDebug.BoolLabel(friendlyAssigned), true);
		}

		for (int j = 0; j < record.EnemyGroups.Count(); j++)
		{
			SCR_AIGroup enemyGroup = record.EnemyGroups[j];
			if (!enemyGroup)
				continue;

			vector enemyTarget = BuildInfantryClashLaneTarget(record.Center, lateral, j, record.EnemyGroups.Count());
			bool enemyAssigned = JLH_AddonSpawnUtility.AssignSingleWaypoint(enemyGroup, JLH_AddonSpawnUtility.WAYPOINT_MOVE, enemyTarget, record.Runtime, "ambient_skirmish_infantry_clash_enemy_move_" + j.ToString());
			LogEvent("ambient_skirmish_infantry_clash_move_assigned", "id=" + record.Id.ToString() + " side=enemy group=" + j.ToString() + " target=" + enemyTarget.ToString() + " result=" + JLH_DCF_NodeDebug.BoolLabel(enemyAssigned), true);
		}
	}

	protected vector BuildInfantryClashLaneTarget(vector center, vector lateral, int groupIndex, int groupCount)
	{
		float offset = BuildInfantryClashGroupLineOffset(groupIndex, groupCount, 50.0);
		vector target = JLH_AddonWorldQuery.GroundPosition(center + (lateral * offset));
		if (!JLH_AddonWorldQuery.IsWaterOrNearWater(target, 4.0))
			return target;

		return JLH_AddonWorldQuery.GroundPosition(center);
	}

	protected float BuildInfantryClashGroupLineOffset(int groupIndex, int groupCount, float spacing)
	{
		if (groupCount <= 1 || spacing <= 0.0)
			return 0.0;

		float centerIndex = (groupCount - 1) * 0.5;
		return (groupIndex - centerIndex) * spacing;
	}

	protected void AssignSmallPatrolContactBehavior(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		for (int i = 0; i < record.FriendlyGroups.Count(); i++)
		{
			SCR_AIGroup friendlyGroup = record.FriendlyGroups[i];
			if (!friendlyGroup)
				continue;

			vector friendlyTarget = BuildComposedGroupPosition(record.Center, i, record.FriendlyGroups.Count());
			JLH_AddonSpawnUtility.AssignMoveAndSearch(friendlyGroup, friendlyTarget, 70.0, definition.FriendlySearchPoints, record.Runtime, "ambient_skirmish_friendly_patrol_contact_" + i.ToString());
		}

		for (int j = 0; j < record.EnemyGroups.Count(); j++)
		{
			SCR_AIGroup enemyGroup = record.EnemyGroups[j];
			if (!enemyGroup)
				continue;

			vector enemyTarget = BuildComposedGroupPosition(record.Center, j + 1, record.EnemyGroups.Count() + 1);
			JLH_AddonSpawnUtility.AssignMoveAndSearch(enemyGroup, enemyTarget, 70.0, definition.EnemySearchPoints, record.Runtime, "ambient_skirmish_enemy_patrol_contact_" + j.ToString());
		}
	}

	protected void AssignReconFlankBehavior(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		vector direction = DirectionFromTo(record.FriendlyPosition, record.EnemyPosition);
		vector lateral = Vector(-direction[2], 0.0, direction[0]);
		for (int i = 0; i < record.FriendlyGroups.Count(); i++)
		{
			SCR_AIGroup friendlyGroup = record.FriendlyGroups[i];
			if (!friendlyGroup)
				continue;

			vector friendlyTarget = JLH_AddonWorldQuery.GroundPosition(record.EnemyPosition - (direction * 22.0) - (lateral * 55.0));
			JLH_AddonSpawnUtility.AssignMoveAndSearch(friendlyGroup, friendlyTarget, 95.0, definition.FriendlySearchPoints, record.Runtime, "ambient_skirmish_friendly_recon_flank_" + i.ToString());
		}

		for (int j = 0; j < record.EnemyGroups.Count(); j++)
		{
			SCR_AIGroup enemyGroup = record.EnemyGroups[j];
			if (!enemyGroup)
				continue;

			vector enemyTarget = JLH_AddonWorldQuery.GroundPosition(record.FriendlyPosition + (direction * 22.0) + (lateral * 55.0));
			JLH_AddonSpawnUtility.AssignMoveAndSearch(enemyGroup, enemyTarget, 95.0, definition.EnemySearchPoints, record.Runtime, "ambient_skirmish_enemy_recon_flank_" + j.ToString());
		}
	}

	protected void AssignRoadblockBreakthroughBehavior(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		vector direction = DirectionFromTo(record.FriendlyPosition, record.EnemyPosition);
		if (vector.Distance(direction, "0 0 0") <= 0.001)
			direction = "0 0 1";

		vector breakthroughTarget = JLH_AddonWorldQuery.GroundPosition(record.EnemyPosition + (direction * 95.0));
		vector lateral = Vector(-direction[2], 0.0, direction[0]);

		for (int i = 0; i < record.FriendlyGroups.Count(); i++)
		{
			SCR_AIGroup friendlyGroup = record.FriendlyGroups[i];
			if (!friendlyGroup)
				continue;

			vector friendlyTarget = JLH_AddonWorldQuery.GroundPosition(breakthroughTarget + (lateral * BuildInfantryClashGroupLineOffset(i, record.FriendlyGroups.Count(), 22.0)));
			JLH_AddonSpawnUtility.AssignMoveAndSearch(friendlyGroup, friendlyTarget, 110.0, definition.FriendlySearchPoints, record.Runtime, "ambient_skirmish_friendly_breakthrough_push_" + i.ToString());
		}

		for (int j = 0; j < record.EnemyGroups.Count(); j++)
		{
			SCR_AIGroup enemyGroup = record.EnemyGroups[j];
			if (!enemyGroup)
				continue;

			vector defendPrimaryPosition;
			vector defendSecondaryPosition;
			BuildRoadblockBreakthroughDefenderPositions(record.EnemyPosition, direction, defendPrimaryPosition, defendSecondaryPosition);

			vector defendPosition = defendPrimaryPosition;
			if (j == 1)
				defendPosition = defendSecondaryPosition;
			else if (j > 1)
				defendPosition = JLH_AddonWorldQuery.GroundPosition(record.EnemyPosition + (lateral * BuildInfantryClashGroupLineOffset(j, record.EnemyGroups.Count(), 6.0)));

			JLH_AddonSpawnUtility.AssignDefend(enemyGroup, defendPosition, record.Runtime, "ambient_skirmish_enemy_roadblock_hold_" + j.ToString());
		}
	}


	protected void AssignVehicleSkirmishBehavior(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishEventDefinition definition)
	{
		LogScenarioCombatProfile(record, definition);

		bool friendlyHasVehicle = !record.FriendlyVehicles.IsEmpty();
		bool enemyHasVehicle = !record.EnemyVehicles.IsEmpty();
		bool friendlyMountedVehicleCrew = friendlyHasVehicle && record.FriendlyVehicleMounted;
		bool enemyMountedVehicleCrew = enemyHasVehicle && record.EnemyVehicleMounted;
		int friendlyVehicleCrewGroups = record.FriendlyVehicles.Count();
		int enemyVehicleCrewGroups = record.EnemyVehicles.Count();
		bool assignMountedVehicleHold = ShouldAssignMountedVehicleHold(definition);
		bool friendlyConvoyMoves = friendlyHasVehicle && (definition.SkirmishEventType == JLH_AmbientSkirmishEventType.CONVOY_AMBUSH_USSR_ATTACKS_US || definition.SkirmishEventType == JLH_AmbientSkirmishEventType.SUPPLY_TRUCK_RAID);

		for (int i = 0; i < record.FriendlyGroups.Count(); i++)
		{
			SCR_AIGroup friendlyGroup = record.FriendlyGroups[i];
			if (!friendlyGroup)
				continue;

			bool friendlyDedicatedVehicleCrew = IsDedicatedVehicleCrewGroup(record, definition, true, i, friendlyVehicleCrewGroups);
			if (friendlyMountedVehicleCrew && i < friendlyVehicleCrewGroups && ShouldAssignScenarioVehicleRoute(definition, true))
			{
				LogMountedVehicleHoldSkipped(record, "friendly", i, "scenario_vehicle_route");
				if (!VerifyRouteAssignmentCrewGroup(record, "friendly", record.FriendlyGroups, friendlyGroup, record.FriendlyVehicles[i], true, i, friendlyVehicleCrewGroups))
					continue;

				AssignScenarioVehicleRoute(record, friendlyGroup, record.FriendlyVehicles[i], definition, true, i, friendlyVehicleCrewGroups);
				continue;
			}

			if (ShouldSkipInfantryOrderForDedicatedVehicleCrew(record, definition, true, i, friendlyVehicleCrewGroups))
			{
				LogSkirmishInfantryOrderSkipped(record, "friendly", i, "dedicated_vehicle_crew");
				continue;
			}

			if (friendlyMountedVehicleCrew && assignMountedVehicleHold && i < friendlyVehicleCrewGroups)
			{
				if (!VerifyRouteAssignmentCrewGroup(record, "friendly", record.FriendlyGroups, friendlyGroup, record.FriendlyVehicles[i], true, i, friendlyVehicleCrewGroups))
					continue;

				vector friendlyHoldTarget = ResolveMountedVehicleHoldPosition(record, true, i, friendlyVehicleCrewGroups);
				bool friendlyHoldAssigned = JLH_AddonSpawnUtility.AssignMountedVehicleHold(friendlyGroup, friendlyHoldTarget, record.Runtime, "ambient_skirmish_friendly_mounted_vehicle_hold_" + i.ToString());
				LogMountedVehicleHold(record, "friendly", i, friendlyHoldTarget, friendlyHoldAssigned);
				continue;
			}

			if (friendlyMountedVehicleCrew && i < friendlyVehicleCrewGroups)
			{
				LogEvent("ambient_skirmish_vehicle_crew_waypoints_skipped", "id=" + record.Id.ToString() + " side=friendly group=" + i.ToString() + " reason=mounted_vehicle_crew", true);
				continue;
			}

			if (friendlyConvoyMoves)
			{
				vector friendlyAdvanceTarget = BuildComposedGroupPosition(record.EnemyPosition, i, record.FriendlyGroups.Count());
				JLH_AddonSpawnUtility.AssignMoveAndSearch(friendlyGroup, friendlyAdvanceTarget, Math.Max(90.0, m_fGroupSeparation), 2, record.Runtime, "ambient_skirmish_friendly_vehicle_advance_" + i.ToString());
				continue;
			}

			vector friendlyDefendPosition = BuildComposedGroupPosition(record.FriendlyPosition, i, record.FriendlyGroups.Count());
			JLH_AddonSpawnUtility.AssignDefend(friendlyGroup, friendlyDefendPosition, record.Runtime, "ambient_skirmish_friendly_vehicle_defend_" + i.ToString());
		}

		for (int j = 0; j < record.EnemyGroups.Count(); j++)
		{
			SCR_AIGroup enemyGroup = record.EnemyGroups[j];
			if (!enemyGroup)
				continue;

			bool enemyDedicatedVehicleCrew = IsDedicatedVehicleCrewGroup(record, definition, false, j, enemyVehicleCrewGroups);
			if (enemyMountedVehicleCrew && j < enemyVehicleCrewGroups && ShouldAssignScenarioVehicleRoute(definition, false))
			{
				LogMountedVehicleHoldSkipped(record, "enemy", j, "scenario_vehicle_route");
				if (!VerifyRouteAssignmentCrewGroup(record, "enemy", record.EnemyGroups, enemyGroup, record.EnemyVehicles[j], false, j, enemyVehicleCrewGroups))
					continue;

				AssignScenarioVehicleRoute(record, enemyGroup, record.EnemyVehicles[j], definition, false, j, enemyVehicleCrewGroups);
				continue;
			}

			if (ShouldSkipInfantryOrderForDedicatedVehicleCrew(record, definition, false, j, enemyVehicleCrewGroups))
			{
				LogSkirmishInfantryOrderSkipped(record, "enemy", j, "dedicated_vehicle_crew");
				continue;
			}

			if (enemyMountedVehicleCrew && assignMountedVehicleHold && j < enemyVehicleCrewGroups)
			{
				if (!VerifyRouteAssignmentCrewGroup(record, "enemy", record.EnemyGroups, enemyGroup, record.EnemyVehicles[j], false, j, enemyVehicleCrewGroups))
					continue;

				vector enemyHoldTarget = ResolveMountedVehicleHoldPosition(record, false, j, enemyVehicleCrewGroups);
				bool enemyHoldAssigned = JLH_AddonSpawnUtility.AssignMountedVehicleHold(enemyGroup, enemyHoldTarget, record.Runtime, "ambient_skirmish_enemy_mounted_vehicle_hold_" + j.ToString());
				LogMountedVehicleHold(record, "enemy", j, enemyHoldTarget, enemyHoldAssigned);
				continue;
			}

			if (enemyMountedVehicleCrew && j < enemyVehicleCrewGroups)
			{
				LogEvent("ambient_skirmish_vehicle_crew_waypoints_skipped", "id=" + record.Id.ToString() + " side=enemy group=" + j.ToString() + " reason=mounted_vehicle_crew", true);
				continue;
			}

			vector enemyTarget = BuildComposedGroupPosition(record.FriendlyPosition, j, record.EnemyGroups.Count());
			if (enemyHasVehicle)
				JLH_AddonSpawnUtility.AssignMoveAndSearch(enemyGroup, enemyTarget, Math.Max(100.0, m_fGroupSeparation * Math.Max(0.1, definition.SeparationMultiplier)), 2, record.Runtime, "ambient_skirmish_enemy_vehicle_assault_" + definition.SkirmishEventName + "_" + j.ToString());
			else
				JLH_AddonSpawnUtility.AssignMoveAndSearch(enemyGroup, enemyTarget, Math.Max(80.0, m_fGroupSeparation * Math.Max(0.1, definition.SeparationMultiplier)), 3, record.Runtime, "ambient_skirmish_enemy_ambush_assault_" + definition.SkirmishEventName + "_" + j.ToString());
		}

		record.Runtime.RefreshExclusions("ambient_skirmish_vehicle_behavior_refresh");
	}


	protected bool ShouldAssignScenarioVehicleRoute(notnull JLH_AmbientSkirmishEventDefinition definition, bool friendlySide)
	{
		JLH_AmbientSkirmishEventType eventType = definition.SkirmishEventType;
		if (eventType == JLH_AmbientSkirmishEventType.VEHICLE_SKIRMISH)
			return true;

		if (eventType == JLH_AmbientSkirmishEventType.CONVOY_AMBUSH_USSR_ATTACKS_US || eventType == JLH_AmbientSkirmishEventType.SUPPLY_TRUCK_RAID)
			return friendlySide;

		if (eventType == JLH_AmbientSkirmishEventType.CONVOY_AMBUSH_US_ATTACKS_USSR)
			return !friendlySide;

		return false;
	}

	protected bool CanAssignStableVehicleRoute(notnull JLH_AmbientSkirmishRecord record, bool friendlySide, int vehicleIndex, string sideLabel, string routeType)
	{
		JLH_AmbientSkirmishVehicleLifecycle lifecycle = record.GetVehicleLifecycle(friendlySide, vehicleIndex);
		if (!lifecycle)
			return false;

		if (lifecycle.RouteAssignedAfterStableMount)
		{
			LogEvent("route_assignment_blocked_mount_unstable", "id=" + record.Id.ToString() + " side=" + sideLabel + " vehicleIndex=" + vehicleIndex.ToString() + " routeType=" + routeType + " reason=route_already_assigned state=" + VehicleMountStateLabel(lifecycle.State), true);
			return false;
		}

		if (lifecycle.State != JLH_AmbientSkirmishVehicleMountState.ROUTE_ASSIGN_ALLOWED && lifecycle.State != JLH_AmbientSkirmishVehicleMountState.ROUTE_READY && lifecycle.State != JLH_AmbientSkirmishVehicleMountState.VEHICLE_OCCUPANCY_STABILIZING && lifecycle.State != JLH_AmbientSkirmishVehicleMountState.WAITING_FOR_OCCUPANCY_CONFIRM)
		{
			LogEvent("route_assignment_blocked_mount_unstable", "id=" + record.Id.ToString() + " side=" + sideLabel + " vehicleIndex=" + vehicleIndex.ToString() + " routeType=" + routeType + " reason=mount_not_stable state=" + VehicleMountStateLabel(lifecycle.State) + " attempts=" + lifecycle.MountAttemptCount.ToString() + " validationFailures=" + lifecycle.ValidationFailureCount.ToString(), true);
			return false;
		}

		return true;
	}

	protected void MarkStableVehicleRouteAssigned(notnull JLH_AmbientSkirmishRecord record, bool friendlySide, int vehicleIndex, string sideLabel, string routeType, bool assigned)
	{
		if (!assigned)
			return;

		JLH_AmbientSkirmishVehicleLifecycle lifecycle = record.GetVehicleLifecycle(friendlySide, vehicleIndex);
		if (!lifecycle)
			return;

		lifecycle.RouteAssignedAfterStableMount = true;
		lifecycle.State = JLH_AmbientSkirmishVehicleMountState.ROUTE_ASSIGNED;
		LogEvent("ambient_skirmish_vehicle_route_assigned", "id=" + record.Id.ToString() + " side=" + sideLabel + " vehicleIndex=" + vehicleIndex.ToString() + " routeType=" + routeType + " mountValidated=" + JLH_DCF_NodeDebug.BoolLabel(lifecycle.MountValidated) + " state=" + VehicleMountStateLabel(lifecycle.State), true);
	}

	protected bool AssignScenarioVehicleRoute(notnull JLH_AmbientSkirmishRecord record, SCR_AIGroup group, IEntity vehicle, notnull JLH_AmbientSkirmishEventDefinition definition, bool friendlySide, int vehicleIndex, int vehicleCount)
	{
		if (!group || !vehicle)
			return false;

		string sideLabel = "enemy";
		if (friendlySide)
			sideLabel = "friendly";

		if (!CanAssignStableVehicleRoute(record, friendlySide, vehicleIndex, sideLabel, "scenario_vehicle_route"))
			return false;

		vector start = JLH_AddonWorldQuery.GroundPosition(vehicle.GetOrigin());
		JLH_DCF_NodeWaypointResolver.ClearGroupWaypoints(group);
		int waypointCount = CreateScenarioVehicleRoute(group, record.Runtime, start, record, definition, friendlySide, vehicleIndex, vehicleCount);
		bool assigned = waypointCount > 0;
		ActivateSkirmishVehicleGroupAI(group);
		MarkStableVehicleRouteAssigned(record, friendlySide, vehicleIndex, sideLabel, "scenario_vehicle_route", assigned);

		string routeLog = "id=" + record.Id.ToString();
		routeLog = routeLog + " event=" + definition.SkirmishEventName;
		routeLog = routeLog + " side=" + sideLabel;
		routeLog = routeLog + " vehicleIndex=" + vehicleIndex.ToString();
		routeLog = routeLog + " vehicleCount=" + vehicleCount.ToString();
		routeLog = routeLog + " route_profile=" + definition.RouteProfile;
		routeLog = routeLog + " convoy_profile=" + definition.ConvoyProfile;
		routeLog = routeLog + " start=" + start.ToString();
		routeLog = routeLog + " waypoints=" + waypointCount.ToString();
		routeLog = routeLog + " result=" + JLH_DCF_NodeDebug.BoolLabel(assigned);
		LogEvent("ambient_skirmish_route_profile", routeLog, true);
		return assigned;
	}

	protected int CreateScenarioVehicleRoute(SCR_AIGroup group, notnull JLH_AddonRuntimeEntityRegistry registry, vector start, notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishEventDefinition definition, bool friendlySide, int vehicleIndex, int vehicleCount)
	{
		if (!group)
			return 0;

		vector friendlyAnchor = JLH_AddonWorldQuery.GroundPosition(record.FriendlyPosition);
		vector enemyAnchor = JLH_AddonWorldQuery.GroundPosition(record.EnemyPosition);
		vector center = JLH_AddonWorldQuery.GroundPosition(record.Center);
		vector direction = DirectionFromTo(friendlyAnchor, enemyAnchor);
		vector lateral = Vector(-direction[2], 0.0, direction[0]);
		float sideSign = 1.0;
		if (friendlySide)
			sideSign = -1.0;

		float centeredIndex = vehicleIndex * 1.0;
		float centeredCount = Math.Max(0.0, (vehicleCount - 1) * 1.0);
		float lateralOffset = (centeredIndex - (centeredCount * 0.5)) * 28.0;

		array<vector> routePoints = {};
		JLH_AmbientSkirmishEventType eventType = definition.SkirmishEventType;
		if (eventType == JLH_AmbientSkirmishEventType.VEHICLE_SKIRMISH)
		{
			float standoff = Math.Clamp(vector.Distance(friendlyAnchor, enemyAnchor) * 0.36, 55.0, 95.0);
			float laneHalfLength = 55.0;
			vector laneCenter = center + (direction * (sideSign * standoff)) + (lateral * lateralOffset);
			routePoints.Insert(ResolveDryRoutePoint(laneCenter - (lateral * laneHalfLength)));
			routePoints.Insert(ResolveDryRoutePoint(center + (direction * (sideSign * 35.0)) + (lateral * (lateralOffset * 0.4))));
			routePoints.Insert(ResolveDryRoutePoint(laneCenter + (lateral * laneHalfLength)));
		}
		else
		{
			vector convoyStart = friendlyAnchor;
			vector convoyEnd = enemyAnchor;
			if (!friendlySide)
			{
				convoyStart = enemyAnchor;
				convoyEnd = friendlyAnchor;
			}

			vector convoyDirection = DirectionFromTo(convoyStart, convoyEnd);
			vector convoyLateral = Vector(-convoyDirection[2], 0.0, convoyDirection[0]);
			routePoints.Insert(ResolveDryRoutePoint(convoyStart + (convoyDirection * 45.0) + (convoyLateral * lateralOffset)));
			routePoints.Insert(ResolveDryRoutePoint(center + (convoyLateral * (lateralOffset * 0.35))));
			routePoints.Insert(ResolveDryRoutePoint(center + (convoyDirection * 130.0) + (convoyLateral * lateralOffset)));
		}

		int waypointCount = 0;
		array<AIWaypoint> routeWaypoints = {};
		for (int i = 0; i < routePoints.Count(); i++)
			waypointCount += AddScenarioVehicleRouteWaypoint(group, registry, routeWaypoints, routePoints[i], definition.RouteProfile);

		if (eventType == JLH_AmbientSkirmishEventType.VEHICLE_SKIRMISH && !routeWaypoints.IsEmpty())
			waypointCount += AddScenarioVehicleRouteCycleWaypoint(group, registry, routeWaypoints, routeWaypoints[0].GetOrigin(), definition.RouteProfile);

		return waypointCount;
	}

	protected int AddScenarioVehicleRouteWaypoint(SCR_AIGroup group, notnull JLH_AddonRuntimeEntityRegistry registry, notnull array<AIWaypoint> routeWaypoints, vector position, string routeProfile)
	{
		if (!group)
			return 0;

		AIWaypoint waypoint = JLH_AddonSpawnUtility.SpawnWaypoint(JLH_AddonSpawnUtility.WAYPOINT_MOVE, position, "ambient_skirmish_vehicle_route_" + routeProfile);
		if (!waypoint)
			return 0;

		group.AddWaypoint(waypoint);
		registry.TrackWaypoint(waypoint, "ambient_skirmish_vehicle_route_" + routeProfile);
		routeWaypoints.Insert(waypoint);
		return 1;
	}

	protected int AddScenarioVehicleRouteCycleWaypoint(SCR_AIGroup group, notnull JLH_AddonRuntimeEntityRegistry registry, notnull array<AIWaypoint> routeWaypoints, vector cyclePosition, string routeProfile)
	{
		if (!group || routeWaypoints.IsEmpty())
			return 0;

		AIWaypoint cycle = JLH_AddonSpawnUtility.SpawnWaypoint(JLH_AddonSpawnUtility.WAYPOINT_CYCLE, cyclePosition, "ambient_skirmish_vehicle_route_cycle_" + routeProfile);
		if (!cycle)
			return 0;

		AIWaypointCycle cycleWaypoint = AIWaypointCycle.Cast(cycle);
		if (!cycleWaypoint)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(cycle);
			return 0;
		}

		cycleWaypoint.SetRerunCounter(-1);
		cycleWaypoint.SetWaypoints(routeWaypoints);
		group.AddWaypoint(cycleWaypoint);
		registry.TrackWaypoint(cycleWaypoint, "ambient_skirmish_vehicle_route_cycle_" + routeProfile);
		return 1;
	}

	protected bool ShouldSkipInfantryOrderForDedicatedVehicleCrew(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishEventDefinition definition, bool friendlySide, int groupIndex, int vehicleCrewGroups)
	{
		return IsDedicatedVehicleCrewGroup(record, definition, friendlySide, groupIndex, vehicleCrewGroups);
	}

	protected bool IsDedicatedVehicleCrewGroup(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishEventDefinition definition, bool friendlySide, int groupIndex, int vehicleCrewGroups)
	{
		if (groupIndex < 0 || groupIndex >= vehicleCrewGroups)
			return false;

		string source;
		if (friendlySide)
			source = record.GetFriendlyGroupSource(groupIndex);
		else
			source = record.GetEnemyGroupSource(groupIndex);

		if (source.Contains("dedicated_vehicle_crew"))
			return true;

		return false;
	}

	protected bool IsDedicatedVehicleCrewSelection(JLH_DCF_EnemyGroupSelection selection)
	{
		if (!selection)
			return false;

		return selection.Source.Contains("dedicated_vehicle_crew");
	}

	protected void LogCrewGroupCreated(SCR_AIGroup group, JLH_DCF_EnemyGroupSelection selection, string sideLabel, int groupIndex, int plannedGroups, bool reusedBeforeInsert)
	{
		if (!group || !selection)
			return;

		string createLog = "side=" + sideLabel;
		createLog = createLog + " groupIndex=" + groupIndex.ToString();
		createLog = createLog + " plannedGroups=" + plannedGroups.ToString();
		createLog = createLog + " crewGroup=" + EntityDebugLabel(group);
		createLog = createLog + " prefab=" + selection.Prefab;
		createLog = createLog + " source=" + selection.Source;
		createLog = createLog + " estimatedUnits=" + selection.EstimatedUnits.ToString();
		createLog = createLog + " reusedBeforeInsert=" + JLH_DCF_NodeDebug.BoolLabel(reusedBeforeInsert);
		LogEvent("crew_group_created", createLog, true);
	}

	protected void LogCrewGroupAssignedVehicle(notnull JLH_AmbientSkirmishRecord record, string sideLabel, SCR_AIGroup group, IEntity vehicle, ResourceName vehiclePrefab, int vehicleIndex, int vehicleCount, string phase, int mountedCrew)
	{
		string assignLog = "id=" + record.Id.ToString();
		assignLog = assignLog + " phase=" + phase;
		assignLog = assignLog + " side=" + sideLabel;
		assignLog = assignLog + " vehicleIndex=" + vehicleIndex.ToString();
		assignLog = assignLog + " vehicleCount=" + vehicleCount.ToString();
		assignLog = assignLog + " vehicle=" + EntityDebugLabel(vehicle);
		assignLog = assignLog + " vehiclePrefab=" + vehiclePrefab;
		assignLog = assignLog + " crewGroup=" + EntityDebugLabel(group);
		assignLog = assignLog + " mountedCrew=" + mountedCrew.ToString();
		LogEvent("crew_group_assigned_vehicle", assignLog, true);
	}

	protected bool VerifyUniqueCrewGroupsForVehicles(int recordId, string sideLabel, notnull array<SCR_AIGroup> groups, int vehicleCount, string phase)
	{
		bool unique = true;
		int safeVehicleCount = Math.Min(vehicleCount, groups.Count());
		for (int i = 0; i < safeVehicleCount; i++)
		{
			SCR_AIGroup group = groups[i];
			if (!group)
				continue;

			for (int j = i + 1; j < safeVehicleCount; j++)
			{
				if (groups[j] != group)
					continue;

				LogCrewGroupReusedDetected(recordId, sideLabel, group, null, i, j, i, vehicleCount, phase, "same_group_instance_assigned_to_multiple_vehicle_slots");
				unique = false;
			}
		}

		return unique;
	}

	protected bool VerifyRouteAssignmentCrewGroup(notnull JLH_AmbientSkirmishRecord record, string sideLabel, notnull array<SCR_AIGroup> groups, SCR_AIGroup group, IEntity vehicle, bool friendlySide, int vehicleIndex, int vehicleCrewGroups)
	{
		if (!group || !vehicle)
			return false;

		if (vehicleIndex < 0 || vehicleIndex >= groups.Count())
		{
			LogCrewGroupReusedDetected(record.Id, sideLabel, group, vehicle, -1, vehicleIndex, vehicleIndex, vehicleCrewGroups, "route_task_precheck", "vehicle_index_outside_group_array");
			return false;
		}

		if (groups[vehicleIndex] != group)
		{
			int assignedIndex = FindCrewGroupIndex(groups, group, vehicleCrewGroups);
			LogCrewGroupReusedDetected(record.Id, sideLabel, group, vehicle, assignedIndex, vehicleIndex, vehicleIndex, vehicleCrewGroups, "route_task_precheck", "route_group_mismatch");
			return false;
		}

		int firstIndex = -1;
		int duplicateIndex = -1;
		if (FindCrewGroupReuse(groups, group, vehicleCrewGroups, firstIndex, duplicateIndex))
		{
			LogCrewGroupReusedDetected(record.Id, sideLabel, group, vehicle, firstIndex, duplicateIndex, vehicleIndex, vehicleCrewGroups, "route_task_precheck", "route_group_reused_by_another_vehicle");
			return false;
		}

		ResourceName vehiclePrefab;
		if (friendlySide)
			vehiclePrefab = record.GetFriendlyVehiclePrefab(vehicleIndex, "");
		else
			vehiclePrefab = record.GetEnemyVehiclePrefab(vehicleIndex, "");

		LogCrewGroupAssignedVehicle(record, sideLabel, group, vehicle, vehiclePrefab, vehicleIndex, vehicleCrewGroups, "route_task_ownership_verified", CountMountedVehicleCrew(group, vehicle));
		return true;
	}

	protected int FindCrewGroupIndex(notnull array<SCR_AIGroup> groups, SCR_AIGroup group, int groupLimit)
	{
		if (!group)
			return -1;

		int safeLimit = Math.Min(groupLimit, groups.Count());
		for (int i = 0; i < safeLimit; i++)
		{
			if (groups[i] == group)
				return i;
		}

		return -1;
	}

	protected bool FindCrewGroupReuse(notnull array<SCR_AIGroup> groups, SCR_AIGroup group, int groupLimit, out int firstIndex, out int duplicateIndex)
	{
		firstIndex = -1;
		duplicateIndex = -1;
		if (!group)
			return false;

		int safeLimit = Math.Min(groupLimit, groups.Count());
		for (int i = 0; i < safeLimit; i++)
		{
			if (groups[i] != group)
				continue;

			if (firstIndex < 0)
			{
				firstIndex = i;
				continue;
			}

			duplicateIndex = i;
			return true;
		}

		return false;
	}

	protected void LogCrewGroupReusedDetected(int recordId, string sideLabel, SCR_AIGroup group, IEntity vehicle, int firstIndex, int duplicateIndex, int vehicleIndex, int vehicleCrewGroups, string phase, string reason)
	{
		string reuseLog = "id=" + recordId.ToString();
		reuseLog = reuseLog + " phase=" + phase;
		reuseLog = reuseLog + " side=" + sideLabel;
		reuseLog = reuseLog + " vehicleIndex=" + vehicleIndex.ToString();
		reuseLog = reuseLog + " vehicleCrewGroups=" + vehicleCrewGroups.ToString();
		reuseLog = reuseLog + " firstGroupIndex=" + firstIndex.ToString();
		reuseLog = reuseLog + " duplicateGroupIndex=" + duplicateIndex.ToString();
		reuseLog = reuseLog + " crewGroup=" + EntityDebugLabel(group);
		reuseLog = reuseLog + " vehicle=" + EntityDebugLabel(vehicle);
		reuseLog = reuseLog + " reason=" + reason;
		LogEvent("crew_group_reused_detected", reuseLog, true);
	}

	protected void LogVehicleCrewGroupOwnership(notnull JLH_AmbientSkirmishRecord record, string sideLabel, notnull array<SCR_AIGroup> groups, SCR_AIGroup group, IEntity vehicle, bool friendlySide, int vehicleIndex, int vehicleCrewGroups)
	{
		int firstIndex = -1;
		for (int i = 0; i < vehicleCrewGroups && i < groups.Count(); i++)
		{
			if (groups[i] != group)
				continue;

			firstIndex = i;
			break;
		}

		bool uniqueGroup = firstIndex == vehicleIndex;
		IEntity driver = record.GetVehicleDriver(friendlySide, vehicleIndex);
		IEntity gunner = record.GetVehicleGunner(friendlySide, vehicleIndex);
		IEntity commander = FindMountedCommanderEntity(group, vehicle, driver, gunner);
		string ownershipLog = "id=" + record.Id.ToString();
		ownershipLog = ownershipLog + " side=" + sideLabel;
		ownershipLog = ownershipLog + " vehicleIndex=" + vehicleIndex.ToString();
		ownershipLog = ownershipLog + " vehicle=" + EntityDebugLabel(vehicle);
		ownershipLog = ownershipLog + " crewGroup=" + EntityDebugLabel(group);
		ownershipLog = ownershipLog + " routeAssignedGroup=" + EntityDebugLabel(group);
		ownershipLog = ownershipLog + " routeAssignedGroupIndex=" + vehicleIndex.ToString();
		ownershipLog = ownershipLog + " firstGroupIndex=" + firstIndex.ToString();
		ownershipLog = ownershipLog + " uniqueGroup=" + JLH_DCF_NodeDebug.BoolLabel(uniqueGroup);
		ownershipLog = ownershipLog + " driverEntity=" + EntityDebugLabel(driver);
		ownershipLog = ownershipLog + " gunnerEntity=" + EntityDebugLabel(gunner);
		ownershipLog = ownershipLog + " commanderEntity=" + EntityDebugLabel(commander);
		ownershipLog = ownershipLog + " driverEqualsGunner=" + JLH_DCF_NodeDebug.BoolLabel(AreSameEntity(driver, gunner));
		ownershipLog = ownershipLog + " driverEqualsCommander=" + JLH_DCF_NodeDebug.BoolLabel(AreSameEntity(driver, commander));
		ownershipLog = ownershipLog + " gunnerEqualsCommander=" + JLH_DCF_NodeDebug.BoolLabel(AreSameEntity(gunner, commander));
		ownershipLog = ownershipLog + " mountedCrew=" + CountMountedVehicleCrew(group, vehicle).ToString();
		LogEvent("ambient_skirmish_vehicle_crew_group_ownership", ownershipLog, true);
	}

	protected void LogSkirmishInfantryOrderSkipped(notnull JLH_AmbientSkirmishRecord record, string sideLabel, int groupIndex, string groupSource)
	{
		string skipLog = "id=" + record.Id.ToString();
		skipLog = skipLog + " side=" + sideLabel;
		skipLog = skipLog + " group=" + groupIndex.ToString();
		skipLog = skipLog + " groupSource=" + groupSource;
		skipLog = skipLog + " reason=vehicle_only_behavior";
		LogEvent("ambient_skirmish_infantry_order_skipped", skipLog, true);
	}






	protected vector ResolveDryRoutePoint(vector point)
	{
		vector ground = JLH_AddonWorldQuery.GroundPosition(point);
		if (!JLH_AddonWorldQuery.IsWaterOrNearWater(ground, 6.0))
			return ground;

		for (int i = 0; i < 6; i++)
		{
			vector candidate = JLH_AddonWorldQuery.GroundPosition(ground + JLH_AddonWorldQuery.RandomOffset(8.0, 34.0));
			if (!JLH_AddonWorldQuery.IsWaterOrNearWater(candidate, 6.0))
				return candidate;
		}

		return ground;
	}

	protected IEntity FindMountedCommanderEntity(SCR_AIGroup group, IEntity vehicle, IEntity driver, IEntity gunner)
	{
		if (!group || !vehicle)
			return null;

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!unit || unit == driver || unit == gunner || !JLH_AddonSpawnUtility.IsUnitAlive(unit))
				continue;

			if (!IsSkirmishUnitInVehicle(unit, vehicle))
				continue;

			if (IsSkirmishUnitInCompartmentType(unit, vehicle, ECompartmentType.CARGO))
				return unit;
		}

		return null;
	}

	protected int CountMountedVehicleCrew(SCR_AIGroup group, IEntity vehicle)
	{
		if (!group || !vehicle)
			return 0;

		int mountedCrew = 0;
		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!unit || !JLH_AddonSpawnUtility.IsUnitAlive(unit))
				continue;

			if (IsSkirmishUnitInVehicle(unit, vehicle))
				mountedCrew++;
		}

		return mountedCrew;
	}

	protected void AuditMountedAITransfer(notnull JLH_AmbientSkirmishRecord record, string sideLabel, SCR_AIGroup assignedGroup, IEntity vehicle, bool friendlySide, int vehicleIndex, int vehicleCrewGroups, string phase)
	{
		if (friendlySide)
			AuditMountedAITransferForGroups(record, sideLabel, assignedGroup, vehicle, record.FriendlyGroups, true, vehicleIndex, vehicleCrewGroups, phase);
		else
			AuditMountedAITransferForGroups(record, sideLabel, assignedGroup, vehicle, record.EnemyGroups, false, vehicleIndex, vehicleCrewGroups, phase);
	}

	protected void AuditMountedAITransferForGroups(notnull JLH_AmbientSkirmishRecord record, string sideLabel, SCR_AIGroup assignedGroup, IEntity vehicle, notnull array<SCR_AIGroup> groups, bool friendlySide, int vehicleIndex, int vehicleCrewGroups, string phase)
	{
		if (!assignedGroup || !vehicle)
			return;

		int groupLimit = Math.Min(vehicleCrewGroups, groups.Count());
		array<IEntity> auditUnits = {};
		AddUniqueAuditUnit(record.GetVehicleDriver(friendlySide, vehicleIndex), auditUnits);
		AddUniqueAuditUnit(record.GetVehicleGunner(friendlySide, vehicleIndex), auditUnits);
		AddUniqueAuditUnit(record.GetVehiclePassenger(friendlySide, vehicleIndex), auditUnits);
		CollectMountedVehicleAuditUnits(assignedGroup, vehicle, auditUnits);
		for (int i = 0; i < groupLimit; i++)
			CollectMountedVehicleAuditUnits(groups[i], vehicle, auditUnits);

		foreach (IEntity unit : auditUnits)
		{
			if (!unit || !JLH_AddonSpawnUtility.IsUnitAlive(unit))
				continue;

			bool inAssignedGroup = IsGroupMember(assignedGroup, unit);
			int owningGroupIndex = FindGroupIndexContainingUnit(groups, unit, groupLimit);
			if (inAssignedGroup && (owningGroupIndex < 0 || owningGroupIndex == vehicleIndex))
				continue;

			string transferLog = "id=" + record.Id.ToString();
			transferLog = transferLog + " phase=" + phase;
			transferLog = transferLog + " side=" + sideLabel;
			transferLog = transferLog + " vehicleIndex=" + vehicleIndex.ToString();
			transferLog = transferLog + " vehicle=" + EntityDebugLabel(vehicle);
			transferLog = transferLog + " assignedGroup=" + EntityDebugLabel(assignedGroup);
			transferLog = transferLog + " assignedGroupIndex=" + vehicleIndex.ToString();
			transferLog = transferLog + " owningGroupIndex=" + owningGroupIndex.ToString();
			if (owningGroupIndex >= 0 && owningGroupIndex < groups.Count())
				transferLog = transferLog + " owningGroup=" + EntityDebugLabel(groups[owningGroupIndex]);
			else
				transferLog = transferLog + " owningGroup=NONE";
			transferLog = transferLog + " unit=" + EntityDebugLabel(unit);
			transferLog = transferLog + " inAssignedGroup=" + JLH_DCF_NodeDebug.BoolLabel(inAssignedGroup);
			transferLog = transferLog + " inVehicle=" + JLH_DCF_NodeDebug.BoolLabel(IsSkirmishUnitInVehicle(unit, vehicle));
			LogEvent("mounted_ai_transferred", transferLog, true);
		}
	}

	protected void LogVehicleDriverLostAfterRouteAssign(notnull JLH_AmbientSkirmishRecord record, string sideLabel, SCR_AIGroup group, IEntity vehicle, bool friendlySide, int vehicleIndex, int waypointCount, bool routeAssigned)
	{
		IEntity driver = record.GetVehicleDriver(friendlySide, vehicleIndex);
		bool driverAlive = driver && JLH_AddonSpawnUtility.IsUnitAlive(driver);
		bool driverInVehicle = driverAlive && IsSkirmishUnitInVehicle(driver, vehicle);
		bool driverInGroup = driverAlive && IsGroupMember(group, driver);
		if (driverAlive && driverInVehicle && driverInGroup)
			return;

		JLH_AmbientSkirmishVehicleLifecycle lifecycle = record.GetVehicleLifecycle(friendlySide, vehicleIndex);
		if (lifecycle && !lifecycle.MountValidated && (lifecycle.State == JLH_AmbientSkirmishVehicleMountState.VEHICLE_OCCUPANCY_STABILIZING || lifecycle.State == JLH_AmbientSkirmishVehicleMountState.WAITING_FOR_OCCUPANCY_CONFIRM || lifecycle.RouteAssignedAfterStableMount))
		{
			string pendingLog = "id=" + record.Id.ToString();
			pendingLog = pendingLog + " phase=post_route_assign_driver_pending";
			pendingLog = pendingLog + " side=" + sideLabel;
			pendingLog = pendingLog + " vehicleIndex=" + vehicleIndex.ToString();
			pendingLog = pendingLog + " vehicle=" + EntityDebugLabel(vehicle);
			pendingLog = pendingLog + " crewGroup=" + EntityDebugLabel(group);
			pendingLog = pendingLog + " driverEntity=" + EntityDebugLabel(driver);
			pendingLog = pendingLog + " driverAlive=" + JLH_DCF_NodeDebug.BoolLabel(driverAlive);
			pendingLog = pendingLog + " driverInVehicle=" + JLH_DCF_NodeDebug.BoolLabel(driverInVehicle);
			pendingLog = pendingLog + " driverInGroup=" + JLH_DCF_NodeDebug.BoolLabel(driverInGroup);
			pendingLog = pendingLog + " waypoints=" + waypointCount.ToString();
			pendingLog = pendingLog + " routeAssigned=" + JLH_DCF_NodeDebug.BoolLabel(routeAssigned);
			pendingLog = pendingLog + " state=" + VehicleMountStateLabel(lifecycle.State);
			LogEvent("vehicle_occupancy_stabilizing", pendingLog, true);
			return;
		}

		IEntity replacementDriver = FindMountedDriverEntity(group, vehicle);
		string driverLog = "id=" + record.Id.ToString();
		driverLog = driverLog + " side=" + sideLabel;
		driverLog = driverLog + " vehicleIndex=" + vehicleIndex.ToString();
		driverLog = driverLog + " vehicle=" + EntityDebugLabel(vehicle);
		driverLog = driverLog + " crewGroup=" + EntityDebugLabel(group);
		driverLog = driverLog + " driverEntity=" + EntityDebugLabel(driver);
		driverLog = driverLog + " driverAlive=" + JLH_DCF_NodeDebug.BoolLabel(driverAlive);
		driverLog = driverLog + " driverInVehicle=" + JLH_DCF_NodeDebug.BoolLabel(driverInVehicle);
		driverLog = driverLog + " driverInGroup=" + JLH_DCF_NodeDebug.BoolLabel(driverInGroup);
		driverLog = driverLog + " replacementDriver=" + EntityDebugLabel(replacementDriver);
		driverLog = driverLog + " waypoints=" + waypointCount.ToString();
		driverLog = driverLog + " routeAssigned=" + JLH_DCF_NodeDebug.BoolLabel(routeAssigned);
		LogEvent("vehicle_driver_lost_after_route_assign", driverLog, true);
	}

	protected void CollectMountedVehicleAuditUnits(SCR_AIGroup group, IEntity vehicle, notnull array<IEntity> auditUnits)
	{
		if (!group || !vehicle)
			return;

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!unit || !JLH_AddonSpawnUtility.IsUnitAlive(unit))
				continue;

			if (IsSkirmishUnitInVehicle(unit, vehicle))
				AddUniqueAuditUnit(unit, auditUnits);
		}
	}

	protected void AddUniqueAuditUnit(IEntity unit, notnull array<IEntity> auditUnits)
	{
		if (!unit || auditUnits.Contains(unit))
			return;

		auditUnits.Insert(unit);
	}

	protected bool IsGroupMember(SCR_AIGroup group, IEntity unit)
	{
		if (!group || !unit)
			return false;

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (agent && agent.GetControlledEntity() == unit)
				return true;
		}

		return false;
	}

	protected int FindGroupIndexContainingUnit(notnull array<SCR_AIGroup> groups, IEntity unit, int groupLimit)
	{
		if (!unit)
			return -1;

		int safeLimit = Math.Min(groupLimit, groups.Count());
		for (int i = 0; i < safeLimit; i++)
		{
			if (IsGroupMember(groups[i], unit))
				return i;
		}

		return -1;
	}

	protected IEntity FindMountedDriverEntity(SCR_AIGroup group, IEntity vehicle)
	{
		if (!group || !vehicle)
			return null;

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!unit || !JLH_AddonSpawnUtility.IsUnitAlive(unit))
				continue;

			if (IsSkirmishUnitInCompartmentType(unit, vehicle, ECompartmentType.PILOT))
				return unit;
		}

		return null;
	}

	protected vector ResolveMountedVehicleHoldPosition(notnull JLH_AmbientSkirmishRecord record, bool friendlySide, int vehicleIndex, int vehicleCount)
	{
		vector midpoint = (record.FriendlyPosition + record.EnemyPosition) * 0.5;
		vector direction = DirectionFromTo(record.FriendlyPosition, record.EnemyPosition);
		vector lateral = Vector(-direction[2], 0, direction[0]);
		float halfHoldSeparation = 32.5;
		vector holdPosition;

		if (friendlySide)
			holdPosition = midpoint - (direction * halfHoldSeparation);
		else
			holdPosition = midpoint + (direction * halfHoldSeparation);

		if (vehicleCount > 1)
		{
			float centeredIndex = vehicleIndex * 1.0;
			float centeredCount = (vehicleCount - 1) * 1.0;
			float lateralOffset = (centeredIndex - (centeredCount * 0.5)) * 18.0;
			holdPosition = holdPosition + (lateral * lateralOffset);
		}

		return JLH_AddonWorldQuery.GroundPosition(holdPosition);
	}

	protected void LogMountedVehicleHold(notnull JLH_AmbientSkirmishRecord record, string sideLabel, int groupIndex, vector target, bool assigned)
	{
		string holdLog = "id=" + record.Id.ToString();
		holdLog = holdLog + " side=" + sideLabel;
		holdLog = holdLog + " group=" + groupIndex.ToString();
		holdLog = holdLog + " result=" + JLH_DCF_NodeDebug.BoolLabel(assigned);
		holdLog = holdLog + " target=" + target.ToString();
		LogEvent("ambient_skirmish_vehicle_crew_hold_assigned", holdLog, true);
	}

	protected void LogMountedVehicleHoldSkipped(notnull JLH_AmbientSkirmishRecord record, string sideLabel, int groupIndex, string reason)
	{
		string holdLog = "id=" + record.Id.ToString();
		holdLog = holdLog + " side=" + sideLabel;
		holdLog = holdLog + " group=" + groupIndex.ToString();
		holdLog = holdLog + " reason=" + reason;
		LogEvent("ambient_skirmish_vehicle_crew_hold_skipped", holdLog, true);
	}

	protected void MonitorVehicleCrewDismountDiagnostics()
	{
		for (int i = m_aRuntimeRecords.Count() - 1; i >= 0; i--)
		{
			if (i < 0 || i >= m_aRuntimeRecords.Count())
				continue;

			JLH_AmbientSkirmishRecord record = m_aRuntimeRecords[i];
			if (!record)
				continue;

			if (!HasRuntimeRecord(record))
				continue;

			if (record.FriendlyVehicles.IsEmpty() && record.EnemyVehicles.IsEmpty())
				continue;

			MonitorVehicleCrewDismountSide(record, true);
			MonitorVehicleCrewDismountSide(record, false);
		}
	}

	protected void MonitorVehicleCrewDismountSide(notnull JLH_AmbientSkirmishRecord record, bool friendlySide)
	{
		string sideLabel = "enemy";
		if (friendlySide)
			sideLabel = "friendly";

		int vehicleCount;
		int crewCount;
		if (!IsVehicleDismountMonitorSideReady(record, friendlySide, sideLabel, vehicleCount, crewCount))
			return;

		for (int i = 0; i < vehicleCount; i++)
		{
			JLH_AmbientSkirmishVehicleMonitorRecord vehicleRecord;
			if (!SafeGetVehicleMonitorRecord(record, friendlySide, sideLabel, i, vehicleCount, crewCount, vehicleRecord))
				continue;

			MonitorVehicleCrewRoleDismount(record, vehicleRecord, vehicleRecord.Gunner, "gunner");

			IEntity passenger = vehicleRecord.Passenger;
			if (!passenger)
			{
				passenger = FindMountedCommanderEntity(vehicleRecord.CrewGroup, vehicleRecord.VehicleEntity, vehicleRecord.Driver, vehicleRecord.Gunner);
				if (passenger)
				{
					SetVehicleMonitorPassengerSafe(record, vehicleRecord, passenger);
					vehicleRecord.Passenger = passenger;
				}
			}

			MonitorVehicleCrewRoleDismount(record, vehicleRecord, passenger, "passenger");
		}
	}

	protected bool IsVehicleDismountMonitorSideReady(notnull JLH_AmbientSkirmishRecord record, bool friendlySide, string sideLabel, out int vehicleCount, out int crewCount)
	{
		vehicleCount = GetDismountMonitorVehicleCount(record, friendlySide);
		crewCount = GetDismountMonitorCrewCount(record, friendlySide);

		if (!HasRuntimeRecord(record))
		{
			LogDismountMonitorSkipped(record, sideLabel, -1, vehicleCount, crewCount);
			return false;
		}

		if (vehicleCount <= 0 || crewCount <= 0)
		{
			if (vehicleCount != crewCount)
				LogDismountMonitorSkipped(record, sideLabel, -1, vehicleCount, crewCount);

			return false;
		}

		if (!record.Runtime || record.Runtime.Vehicles.IsEmpty())
		{
			LogDismountMonitorSkipped(record, sideLabel, -1, vehicleCount, crewCount);
			return false;
		}

		if (vehicleCount != crewCount)
		{
			LogDismountMonitorSkipped(record, sideLabel, -1, vehicleCount, crewCount);
			return false;
		}

		if (friendlySide && !record.FriendlyVehicleMounted)
			return false;
		if (!friendlySide && !record.EnemyVehicleMounted)
			return false;

		if (!AreDismountMonitorSideArraysSized(record, friendlySide, vehicleCount))
		{
			LogDismountMonitorSkipped(record, sideLabel, -1, vehicleCount, crewCount);
			return false;
		}

		if (!HasDismountMonitorSideRouteAssigned(record, friendlySide, vehicleCount))
			return false;

		return true;
	}

	protected int GetDismountMonitorVehicleCount(notnull JLH_AmbientSkirmishRecord record, bool friendlySide)
	{
		if (friendlySide)
			return record.FriendlyVehicles.Count();

		return record.EnemyVehicles.Count();
	}

	protected int GetDismountMonitorCrewCount(notnull JLH_AmbientSkirmishRecord record, bool friendlySide)
	{
		if (friendlySide)
			return record.FriendlyGroups.Count();

		return record.EnemyGroups.Count();
	}

	protected bool AreDismountMonitorSideArraysSized(notnull JLH_AmbientSkirmishRecord record, bool friendlySide, int vehicleCount)
	{
		if (vehicleCount <= 0)
			return false;

		if (friendlySide)
		{
			if (record.FriendlyGroups.Count() < vehicleCount)
				return false;
			if (record.FriendlyVehiclePrefabs.Count() > 0 && record.FriendlyVehiclePrefabs.Count() < vehicleCount)
				return false;
			if (record.FriendlyVehicleDrivers.Count() < vehicleCount)
				return false;
			if (record.FriendlyVehicleGunners.Count() < vehicleCount)
				return false;
			if (record.FriendlyVehiclePassengers.Count() < vehicleCount)
				return false;
			if (record.FriendlyVehicleGunnerDismountLogged.Count() < vehicleCount)
				return false;
			if (record.FriendlyVehiclePassengerDismountLogged.Count() < vehicleCount)
				return false;
			if (record.FriendlyVehicleLifecycles.Count() < vehicleCount)
				return false;

			return true;
		}

		if (record.EnemyGroups.Count() < vehicleCount)
			return false;
		if (record.EnemyVehiclePrefabs.Count() > 0 && record.EnemyVehiclePrefabs.Count() < vehicleCount)
			return false;
		if (record.EnemyVehicleDrivers.Count() < vehicleCount)
			return false;
		if (record.EnemyVehicleGunners.Count() < vehicleCount)
			return false;
		if (record.EnemyVehiclePassengers.Count() < vehicleCount)
			return false;
		if (record.EnemyVehicleGunnerDismountLogged.Count() < vehicleCount)
			return false;
		if (record.EnemyVehiclePassengerDismountLogged.Count() < vehicleCount)
			return false;
		if (record.EnemyVehicleLifecycles.Count() < vehicleCount)
			return false;

		return true;
	}

	protected bool HasDismountMonitorSideRouteAssigned(notnull JLH_AmbientSkirmishRecord record, bool friendlySide, int vehicleCount)
	{
		if (vehicleCount <= 0)
			return false;

		bool routeAssigned = false;
		for (int i = 0; i < vehicleCount; i++)
		{
			JLH_AmbientSkirmishVehicleLifecycle lifecycle;
			if (friendlySide)
			{
				if (i < 0 || i >= record.FriendlyVehicleLifecycles.Count())
					return false;

				lifecycle = record.FriendlyVehicleLifecycles[i];
			}
			else
			{
				if (i < 0 || i >= record.EnemyVehicleLifecycles.Count())
					return false;

				lifecycle = record.EnemyVehicleLifecycles[i];
			}

			if (!lifecycle)
				return false;
			if (lifecycle.State == JLH_AmbientSkirmishVehicleMountState.ABORTED || lifecycle.State == JLH_AmbientSkirmishVehicleMountState.FAILED_RECOVERY || lifecycle.RecoveryLocked)
				return false;
			if (lifecycle.RouteAssignedAfterStableMount)
				routeAssigned = true;
		}

		return routeAssigned;
	}

	protected bool SafeGetVehicleMonitorRecord(notnull JLH_AmbientSkirmishRecord record, bool friendlySide, string sideLabel, int index, int vehicleCount, int crewCount, out JLH_AmbientSkirmishVehicleMonitorRecord vehicleRecord)
	{
		vehicleRecord = null;
		if (index < 0)
		{
			LogDismountMonitorSkipped(record, sideLabel, index, vehicleCount, crewCount);
			return false;
		}

		JLH_AmbientSkirmishVehicleMonitorRecord safeRecord = new JLH_AmbientSkirmishVehicleMonitorRecord();
		safeRecord.FriendlySide = friendlySide;
		safeRecord.SideLabel = sideLabel;
		safeRecord.VehicleIndex = index;
		safeRecord.VehicleCount = vehicleCount;
		safeRecord.CrewCount = crewCount;

		if (friendlySide)
		{
			if (index >= record.FriendlyVehicles.Count() || index >= record.FriendlyGroups.Count() || index >= record.FriendlyVehicleDrivers.Count() || index >= record.FriendlyVehicleGunners.Count() || index >= record.FriendlyVehiclePassengers.Count() || index >= record.FriendlyVehicleGunnerDismountLogged.Count() || index >= record.FriendlyVehiclePassengerDismountLogged.Count() || index >= record.FriendlyVehicleLifecycles.Count())
			{
				LogDismountMonitorSkipped(record, sideLabel, index, vehicleCount, crewCount);
				return false;
			}

			if (record.FriendlyVehiclePrefabs.Count() > 0 && index >= record.FriendlyVehiclePrefabs.Count())
			{
				LogDismountMonitorSkipped(record, sideLabel, index, vehicleCount, crewCount);
				return false;
			}

			safeRecord.VehicleEntity = record.FriendlyVehicles[index];
			safeRecord.CrewGroup = record.FriendlyGroups[index];
			if (index < record.FriendlyVehiclePrefabs.Count())
				safeRecord.VehiclePrefab = record.FriendlyVehiclePrefabs[index];
			safeRecord.Driver = record.FriendlyVehicleDrivers[index];
			safeRecord.Gunner = record.FriendlyVehicleGunners[index];
			safeRecord.Passenger = record.FriendlyVehiclePassengers[index];
			safeRecord.GunnerDismountLogged = record.FriendlyVehicleGunnerDismountLogged[index];
			safeRecord.PassengerDismountLogged = record.FriendlyVehiclePassengerDismountLogged[index];
			safeRecord.Lifecycle = record.FriendlyVehicleLifecycles[index];
		}
		else
		{
			if (index >= record.EnemyVehicles.Count() || index >= record.EnemyGroups.Count() || index >= record.EnemyVehicleDrivers.Count() || index >= record.EnemyVehicleGunners.Count() || index >= record.EnemyVehiclePassengers.Count() || index >= record.EnemyVehicleGunnerDismountLogged.Count() || index >= record.EnemyVehiclePassengerDismountLogged.Count() || index >= record.EnemyVehicleLifecycles.Count())
			{
				LogDismountMonitorSkipped(record, sideLabel, index, vehicleCount, crewCount);
				return false;
			}

			if (record.EnemyVehiclePrefabs.Count() > 0 && index >= record.EnemyVehiclePrefabs.Count())
			{
				LogDismountMonitorSkipped(record, sideLabel, index, vehicleCount, crewCount);
				return false;
			}

			safeRecord.VehicleEntity = record.EnemyVehicles[index];
			safeRecord.CrewGroup = record.EnemyGroups[index];
			if (index < record.EnemyVehiclePrefabs.Count())
				safeRecord.VehiclePrefab = record.EnemyVehiclePrefabs[index];
			safeRecord.Driver = record.EnemyVehicleDrivers[index];
			safeRecord.Gunner = record.EnemyVehicleGunners[index];
			safeRecord.Passenger = record.EnemyVehiclePassengers[index];
			safeRecord.GunnerDismountLogged = record.EnemyVehicleGunnerDismountLogged[index];
			safeRecord.PassengerDismountLogged = record.EnemyVehiclePassengerDismountLogged[index];
			safeRecord.Lifecycle = record.EnemyVehicleLifecycles[index];
		}

		if (!safeRecord.CrewGroup || !safeRecord.VehicleEntity || !safeRecord.Lifecycle)
		{
			LogDismountMonitorSkipped(record, sideLabel, index, vehicleCount, crewCount);
			return false;
		}

		if (!record.Runtime || !record.Runtime.Vehicles.Contains(safeRecord.VehicleEntity))
		{
			LogDismountMonitorSkipped(record, sideLabel, index, vehicleCount, crewCount);
			return false;
		}

		if (safeRecord.Lifecycle.State == JLH_AmbientSkirmishVehicleMountState.ABORTED || safeRecord.Lifecycle.State == JLH_AmbientSkirmishVehicleMountState.FAILED_RECOVERY || safeRecord.Lifecycle.RecoveryLocked)
		{
			LogDismountMonitorSkipped(record, sideLabel, index, vehicleCount, crewCount);
			return false;
		}

		if (!safeRecord.Lifecycle.RouteAssignedAfterStableMount)
			return false;

		vehicleRecord = safeRecord;
		return true;
	}

	protected void SetVehicleMonitorPassengerSafe(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishVehicleMonitorRecord vehicleRecord, IEntity passenger)
	{
		if (!passenger)
			return;

		int index = vehicleRecord.VehicleIndex;
		if (index < 0)
			return;

		if (vehicleRecord.FriendlySide)
		{
			if (index >= record.FriendlyVehiclePassengers.Count())
			{
				LogDismountMonitorSkipped(record, vehicleRecord.SideLabel, index, vehicleRecord.VehicleCount, vehicleRecord.CrewCount);
				return;
			}

			record.FriendlyVehiclePassengers[index] = passenger;
			return;
		}

		if (index >= record.EnemyVehiclePassengers.Count())
		{
			LogDismountMonitorSkipped(record, vehicleRecord.SideLabel, index, vehicleRecord.VehicleCount, vehicleRecord.CrewCount);
			return;
		}

		record.EnemyVehiclePassengers[index] = passenger;
	}

	protected bool IsVehicleMonitorDismountLogged(notnull JLH_AmbientSkirmishVehicleMonitorRecord vehicleRecord, string role)
	{
		if (role == "gunner")
			return vehicleRecord.GunnerDismountLogged;

		return vehicleRecord.PassengerDismountLogged;
	}

	protected void SetVehicleMonitorDismountLoggedSafe(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishVehicleMonitorRecord vehicleRecord, string role, bool logged)
	{
		int index = vehicleRecord.VehicleIndex;
		if (index < 0)
			return;

		if (vehicleRecord.FriendlySide)
		{
			if (role == "gunner")
			{
				if (index >= record.FriendlyVehicleGunnerDismountLogged.Count())
				{
					LogDismountMonitorSkipped(record, vehicleRecord.SideLabel, index, vehicleRecord.VehicleCount, vehicleRecord.CrewCount);
					return;
				}

				record.FriendlyVehicleGunnerDismountLogged[index] = logged;
				vehicleRecord.GunnerDismountLogged = logged;
				return;
			}

			if (index >= record.FriendlyVehiclePassengerDismountLogged.Count())
			{
				LogDismountMonitorSkipped(record, vehicleRecord.SideLabel, index, vehicleRecord.VehicleCount, vehicleRecord.CrewCount);
				return;
			}

			record.FriendlyVehiclePassengerDismountLogged[index] = logged;
			vehicleRecord.PassengerDismountLogged = logged;
			return;
		}

		if (role == "gunner")
		{
			if (index >= record.EnemyVehicleGunnerDismountLogged.Count())
			{
				LogDismountMonitorSkipped(record, vehicleRecord.SideLabel, index, vehicleRecord.VehicleCount, vehicleRecord.CrewCount);
				return;
			}

			record.EnemyVehicleGunnerDismountLogged[index] = logged;
			vehicleRecord.GunnerDismountLogged = logged;
			return;
		}

		if (index >= record.EnemyVehiclePassengerDismountLogged.Count())
		{
			LogDismountMonitorSkipped(record, vehicleRecord.SideLabel, index, vehicleRecord.VehicleCount, vehicleRecord.CrewCount);
			return;
		}

		record.EnemyVehiclePassengerDismountLogged[index] = logged;
		vehicleRecord.PassengerDismountLogged = logged;
	}

	protected void LogDismountMonitorSkipped(notnull JLH_AmbientSkirmishRecord record, string sideLabel, int index, int vehicleCount, int crewCount)
	{
		string key = "ambient_skirmish_dismount_monitor_skipped:" + m_sNodeKey + ":" + record.Id.ToString() + ":" + sideLabel + ":" + index.ToString();
		if (!ShouldLogKey(key))
			return;

		string details = "id=" + record.Id.ToString();
		details = details + " reason=array_mismatch_or_cleanup_pending";
		details = details + " side=" + sideLabel;
		details = details + " index=" + index.ToString();
		details = details + " vehicleCount=" + vehicleCount.ToString();
		details = details + " crewCount=" + crewCount.ToString();
		LogEvent("ambient_skirmish_dismount_monitor_skipped", details, true);
	}

	protected void MonitorVehicleCrewRoleDismount(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishVehicleMonitorRecord vehicleRecord, IEntity unit, string role)
	{
		SCR_AIGroup group = vehicleRecord.CrewGroup;
		IEntity vehicle = vehicleRecord.VehicleEntity;
		if (!group || !vehicle || !unit)
			return;

		if (!JLH_AddonSpawnUtility.IsUnitAlive(unit))
			return;

		if (IsSkirmishUnitInVehicle(unit, vehicle))
		{
			SetVehicleMonitorDismountLoggedSafe(record, vehicleRecord, role, false);
			return;
		}

		if (IsVehicleMonitorDismountLogged(vehicleRecord, role))
			return;

		if (HandleVehicleCrewDismountDuringStabilization(record, vehicleRecord, unit, role))
			return;

		SetVehicleMonitorDismountLoggedSafe(record, vehicleRecord, role, true);
		LogVehicleCrewDismountDetected(record, vehicleRecord.SideLabel, vehicleRecord.VehicleIndex, group, vehicle, unit, role);
	}

	protected bool HandleVehicleCrewDismountDuringStabilization(notnull JLH_AmbientSkirmishRecord record, notnull JLH_AmbientSkirmishVehicleMonitorRecord vehicleRecord, IEntity unit, string role)
	{
		JLH_AmbientSkirmishVehicleLifecycle lifecycle = vehicleRecord.Lifecycle;
		if (!lifecycle)
			return false;

		SCR_AIGroup group = vehicleRecord.CrewGroup;
		IEntity vehicle = vehicleRecord.VehicleEntity;
		if (lifecycle.MountValidated)
			return false;

		if (lifecycle.State == JLH_AmbientSkirmishVehicleMountState.ABORTED || lifecycle.State == JLH_AmbientSkirmishVehicleMountState.FAILED_RECOVERY)
			return false;

		bool unitCombat = IsUnitCombatActive(unit);
		bool groupCombat = IsGroupCombatActive(group);
		if (unitCombat || groupCombat)
			return false;

		bool vehicleCanMove = CanVehicleStillMove(vehicle);
		if (!vehicleCanMove)
			return false;

		bool stabilizationActive = lifecycle.State == JLH_AmbientSkirmishVehicleMountState.VEHICLE_OCCUPANCY_STABILIZING || lifecycle.State == JLH_AmbientSkirmishVehicleMountState.WAITING_FOR_OCCUPANCY_CONFIRM || lifecycle.State == JLH_AmbientSkirmishVehicleMountState.VALIDATING_MOUNT || lifecycle.RouteAssignedAfterStableMount;
		if (!stabilizationActive)
			return false;

		string waypointBefore = GetGroupWaypointTypeLabel(group);
		string combatModeBefore = GetGroupCombatModeLabel(group, groupCombat);
		if (waypointBefore == "NONE" && combatModeBefore == "NONE" && !lifecycle.IdleStateLogged)
		{
			lifecycle.IdleStateLogged = true;
			string idleLog = "id=" + record.Id.ToString();
			idleLog = idleLog + " side=" + vehicleRecord.SideLabel;
			idleLog = idleLog + " vehicleIndex=" + vehicleRecord.VehicleIndex.ToString();
			idleLog = idleLog + " vehicle=" + EntityDebugLabel(vehicle);
			idleLog = idleLog + " unit=" + EntityDebugLabel(unit);
			idleLog = idleLog + " role=" + role;
			idleLog = idleLog + " currentWaypointType=" + waypointBefore;
			idleLog = idleLog + " groupCombatMode=" + combatModeBefore;
			idleLog = idleLog + " state=" + VehicleMountStateLabel(lifecycle.State);
			LogEvent("vehicle_idle_state_detected", idleLog, true);
		}

		if (waypointBefore == "NONE")
			AssignVehicleStabilizationHold(record, group, vehicle, vehicleRecord.SideLabel, vehicleRecord.FriendlySide, vehicleRecord.VehicleIndex, "dismount_guard");

		if (!lifecycle.DismountPreventedLogged)
		{
			lifecycle.DismountPreventedLogged = true;
			string preventLog = "id=" + record.Id.ToString();
			preventLog = preventLog + " side=" + vehicleRecord.SideLabel;
			preventLog = preventLog + " vehicleIndex=" + vehicleRecord.VehicleIndex.ToString();
			preventLog = preventLog + " vehicle=" + EntityDebugLabel(vehicle);
			preventLog = preventLog + " unit=" + EntityDebugLabel(unit);
			preventLog = preventLog + " role=" + role;
			preventLog = preventLog + " vehicleCanMove=" + JLH_DCF_NodeDebug.BoolLabel(vehicleCanMove);
			preventLog = preventLog + " currentWaypointType=" + waypointBefore;
			preventLog = preventLog + " currentWaypointTypeAfter=" + GetGroupWaypointTypeLabel(group);
			preventLog = preventLog + " groupCombatMode=" + combatModeBefore;
			preventLog = preventLog + " groupCombatModeAfter=" + GetGroupCombatModeLabel(group, IsGroupCombatActive(group));
			preventLog = preventLog + " routeAssigned=" + JLH_DCF_NodeDebug.BoolLabel(lifecycle.RouteAssignedAfterStableMount);
			preventLog = preventLog + " state=" + VehicleMountStateLabel(lifecycle.State);
			LogEvent("vehicle_dismount_prevented", preventLog, true);
		}

		SetVehicleMonitorDismountLoggedSafe(record, vehicleRecord, role, false);
		return true;
	}

	protected void LogVehicleCrewDismountDetected(notnull JLH_AmbientSkirmishRecord record, string sideLabel, int vehicleIndex, SCR_AIGroup group, IEntity vehicle, IEntity unit, string role)
	{
		bool unitCombat = IsUnitCombatActive(unit);
		bool groupCombat = IsGroupCombatActive(group);
		bool inCombat = unitCombat || groupCombat;
		string dismountLog = "id=" + record.Id.ToString();
		dismountLog = dismountLog + " side=" + sideLabel;
		dismountLog = dismountLog + " vehicleIndex=" + vehicleIndex.ToString();
		dismountLog = dismountLog + " vehicle=" + EntityDebugLabel(vehicle);
		dismountLog = dismountLog + " unit=" + EntityDebugLabel(unit);
		dismountLog = dismountLog + " role=" + role;
		dismountLog = dismountLog + " inCombat=" + JLH_DCF_NodeDebug.BoolLabel(inCombat);
		dismountLog = dismountLog + " vehicleDamage=" + GetVehicleDamageLabel(vehicle);
		dismountLog = dismountLog + " vehicleCanMove=" + JLH_DCF_NodeDebug.BoolLabel(CanVehicleStillMove(vehicle));
		dismountLog = dismountLog + " currentWaypointType=" + GetGroupWaypointTypeLabel(group);
		dismountLog = dismountLog + " groupCombatMode=" + GetGroupCombatModeLabel(group, groupCombat);
		dismountLog = dismountLog + " threatIconActive=" + JLH_DCF_NodeDebug.BoolLabel(inCombat);
		LogEvent("vehicleCrewDismountDetected", dismountLog, true);
	}

	protected bool IsUnitCombatActive(IEntity unit)
	{
		if (!unit)
			return false;

		SCR_AICombatComponent combat = SCR_AICombatComponent.Cast(unit.FindComponent(SCR_AICombatComponent));
		if (!combat)
			return false;

		if (combat.GetCurrentTarget())
			return true;

		if (combat.GetRetreatTarget())
			return true;

		if (combat.GetEndangeringEnemy())
			return true;

		BaseTarget lastSeenEnemy = combat.GetLastSeenEnemy();
		if (lastSeenEnemy)
			return true;

		return false;
	}

	protected bool IsGroupCombatActive(SCR_AIGroup group)
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
			if (unit && IsUnitCombatActive(unit))
				return true;
		}

		return false;
	}

	protected string GetVehicleDamageLabel(IEntity vehicle)
	{
		if (!vehicle)
			return "NONE";

		DamageManagerComponent damageManager = DamageManagerComponent.Cast(vehicle.FindComponent(DamageManagerComponent));
		if (!damageManager)
			return "unknown";

		return damageManager.GetHealthScaled().ToString();
	}

	protected bool CanVehicleStillMove(IEntity vehicle)
	{
		if (!vehicle)
			return false;

		DamageManagerComponent damageManager = DamageManagerComponent.Cast(vehicle.FindComponent(DamageManagerComponent));
		if (damageManager)
		{
			if (damageManager.IsDestroyed())
				return false;

			if (damageManager.GetState() == EDamageState.DESTROYED)
				return false;

			if (damageManager.GetHealthScaled() <= 0.0)
				return false;
		}

		IEntity usageOwner = null;
		SCR_AIVehicleUsageComponent vehicleUsage = SCR_AIVehicleUsageComponent.FindOnNearestParent(vehicle, usageOwner);
		if (!vehicleUsage || !vehicleUsage.CanBePiloted())
			return false;

		return true;
	}

	protected string GetGroupWaypointTypeLabel(SCR_AIGroup group)
	{
		if (!group)
			return "NONE";

		array<AIWaypoint> waypoints = {};
		group.GetWaypoints(waypoints);
		if (waypoints.IsEmpty())
			return "NONE";

		bool hasCycle = false;
		bool hasMove = false;
		bool hasOther = false;
		foreach (AIWaypoint waypoint : waypoints)
		{
			if (!waypoint)
				continue;

			if (AIWaypointCycle.Cast(waypoint))
			{
				hasCycle = true;
				continue;
			}

			EntityPrefabData prefabData = waypoint.GetPrefabData();
			if (prefabData)
			{
				string prefabName = prefabData.GetPrefabName();
				if (prefabName.Contains("AIWaypoint_Move"))
				{
					hasMove = true;
					continue;
				}
			}

			hasOther = true;
		}

		if (hasMove && hasCycle)
			return "MOVE+CYCLE";
		if (hasCycle)
			return "CYCLE";
		if (hasMove)
			return "MOVE";
		if (hasOther)
			return "OTHER";

		return "UNKNOWN";
	}

	protected string GetGroupCombatModeLabel(SCR_AIGroup group, bool groupCombat)
	{
		if (groupCombat)
			return "COMBAT";

		string waypointType = GetGroupWaypointTypeLabel(group);
		if (waypointType.Contains("CYCLE"))
			return "VEHICLE_ROUTE_CYCLE";
		if (waypointType != "NONE")
			return "VEHICLE_ROUTE";

		return "NONE";
	}

	protected void CleanupRuntime()
	{
		bool hasLivePlayers = JLH_AddonWorldQuery.HasLiveDeployedPlayer("");
		for (int i = m_aRuntimeRecords.Count() - 1; i >= 0; i--)
		{
			JLH_AmbientSkirmishRecord record = m_aRuntimeRecords[i];
			if (!record)
			{
				m_aRuntimeRecords.Remove(i);
				continue;
			}

			string reason;
			if (record.BothSidesDead())
			{
				MarkSkirmishRecordCleared(record, "ambient_skirmish_cleanup_dead");
				string deadBlockReason;
				if (!CanCleanupSkirmishRecord(record, true, deadBlockReason))
				{
					LogEvent("ambient_skirmish_cleanup_dead_deferred", "id=" + record.Id.ToString() + " reason=" + deadBlockReason, false);
					continue;
				}

				reason = "ambient_skirmish_cleanup_dead";
			}
			else if (!hasLivePlayers)
			{
				string noPlayerBlockReason;
				if (!CanCleanupSkirmishRecord(record, false, noPlayerBlockReason))
					continue;

				reason = "ambient_skirmish_cleanup_no_live_players";
			}
			else if (JLH_AddonWorldQuery.GetNearestLivePlayerDistance(record.Center, "") > m_fCleanupDistance)
			{
				string farBlockReason;
				if (!CanCleanupSkirmishRecord(record, false, farBlockReason))
					continue;

				reason = "ambient_skirmish_cleanup_far";
			}
			else
				continue;

			CleanupRecordAt(i, reason, true);
		}
	}

	protected void MarkSkirmishRecordCleared(notnull JLH_AmbientSkirmishRecord record, string reason)
	{
		if (record.m_bCleared)
			return;

		record.m_bCleared = true;
		record.m_iClearedTick = System.GetTickCount();
		JLH_CleanupSafetyService.LogPackageEnteredClearedState(BuildSkirmishCleanupPackageLabel(record, reason), m_bDebug);
	}

	protected bool CanCleanupSkirmishRecord(notnull JLH_AmbientSkirmishRecord record, bool requireLinger, out string blockedReason)
	{
		JLH_CleanupSafetyQuery query = new JLH_CleanupSafetyQuery();
		query.Init(BuildSkirmishCleanupPackageLabel(record, ""), record.Center);
		query.RequireLinger = requireLinger;
		query.ClearedTick = record.m_iClearedTick;
		query.LingerSeconds = JLH_CleanupSafetyService.DEFAULT_PACKAGE_LINGER_SECONDS;
		query.CleanupDistance = m_fCleanupDistance;
		query.VisibilityDistance = Math.Max(m_fCleanupDistance, JLH_CleanupSafetyService.DEFAULT_VISIBILITY_DISTANCE_METERS);
		query.PlayerFactionFilter = "";
		query.DebugEnabled = m_bDebug;
		JLH_CleanupSafetyService.PopulateFromRuntimeRegistry(query, record.Runtime);

		return JLH_CleanupSafetyService.CanCleanupPackage(query, blockedReason);
	}

	protected string BuildSkirmishCleanupPackageLabel(notnull JLH_AmbientSkirmishRecord record, string reason)
	{
		string label = "AmbientSkirmish id=" + record.Id.ToString() + " event=" + record.SkirmishEventName;
		if (m_sNodeKey != "")
			label += " node=" + m_sNodeKey;
		if (reason != "")
			label += " reason=" + reason;

		return label;
	}

	void CleanupAllRuntime(string reason, bool log)
	{
		for (int i = m_aRuntimeRecords.Count() - 1; i >= 0; i--)
			CleanupRecordAt(i, reason, log);
	}

	protected bool HasRuntimeRecord(JLH_AmbientSkirmishRecord targetRecord)
	{
		if (!targetRecord)
			return false;

		foreach (JLH_AmbientSkirmishRecord record : m_aRuntimeRecords)
		{
			if (record == targetRecord)
				return true;
		}

		return false;
	}

	protected bool CleanupRuntimeRecord(JLH_AmbientSkirmishRecord targetRecord, string reason, bool log)
	{
		if (!targetRecord)
			return false;

		for (int i = m_aRuntimeRecords.Count() - 1; i >= 0; i--)
		{
			if (m_aRuntimeRecords[i] != targetRecord)
				continue;

			CleanupRecordAt(i, reason, log);
			return true;
		}

		return false;
	}

	protected void CleanupRecordAt(int index, string reason, bool log)
	{
		if (index < 0 || index >= m_aRuntimeRecords.Count())
			return;

		JLH_AmbientSkirmishRecord record = m_aRuntimeRecords[index];
		if (record)
		{
			if (record.SkirmishEventType == JLH_AmbientSkirmishEventType.CAMP_ASSAULT)
				LogEvent("camp_assault_cleanup", "id=" + record.Id.ToString() + " reason=" + reason, log);

			if (record.SkirmishEventType == JLH_AmbientSkirmishEventType.ROADBLOCK_ASSAULT)
				LogEvent("roadblock_assault_cleanup", "id=" + record.Id.ToString() + " reason=" + reason, log);

			LogEvent(reason, "id=" + record.Id.ToString(), log);
			JLH_SkirmishMapIntelManager.OnSkirmishEnded(record, reason);
			record.Runtime.Cleanup(LOG_PREFIX, reason, log);
			LogEvent("ambient_skirmish_cap_released", "id=" + record.Id.ToString() + " reason=" + reason, log);
		}

		m_aRuntimeRecords.Remove(index);
	}

	int CountActiveLocalRuntime()
	{
		int count = 0;
		for (int i = m_aRuntimeRecords.Count() - 1; i >= 0; i--)
		{
			JLH_AmbientSkirmishRecord record = m_aRuntimeRecords[i];
			if (!record)
			{
				m_aRuntimeRecords.Remove(i);
				continue;
			}

			if (record.IsActive())
				count++;
		}

		return count;
	}

	protected bool FindNearestLivePlayerWithinActivation(out IEntity nearestPlayer, out float nearestDistance)
	{
		nearestPlayer = null;
		nearestDistance = 99999999.0;

		ref array<IEntity> players = {};
		JLH_AddonWorldQuery.GatherLiveDeployedPlayers(players, "");
		foreach (IEntity player : players)
		{
			if (!player)
				continue;

			float distance = vector.Distance(GetOwner().GetOrigin(), player.GetOrigin());
			if (distance > GetEffectiveActivationRadius() || distance >= nearestDistance)
				continue;

			nearestDistance = distance;
			nearestPlayer = player;
		}

		return nearestPlayer != null;
	}

	protected void CollectSkirmishMarkers(IEntity parent, notnull array<JLH_DCF_AmbientSkirmishMarkerComponent> markers)
	{
		if (!parent)
			return;

		IEntity child = parent.GetChildren();
		while (child)
		{
			IEntity next = child.GetSibling();
			JLH_DCF_AmbientSkirmishMarkerComponent marker = JLH_DCF_AmbientSkirmishMarkerComponent.Cast(child.FindComponent(JLH_DCF_AmbientSkirmishMarkerComponent));
			if (marker && marker.IsEnabled())
				markers.Insert(marker);

			CollectSkirmishMarkers(child, markers);
			child = next;
		}
	}

	protected void CollectScenarioMarkers(IEntity parent, notnull array<JLH_DCF_AmbientSkirmishScenarioMarkerComponent> markers)
	{
		if (!parent)
			return;

		IEntity child = parent.GetChildren();
		while (child)
		{
			IEntity next = child.GetSibling();
			JLH_DCF_AmbientSkirmishScenarioMarkerComponent marker = FindScenarioMarkerComponent(child);
			if (marker && marker.IsEnabled())
				markers.Insert(marker);

			CollectScenarioMarkers(child, markers);
			child = next;
		}
	}

	protected JLH_DCF_AmbientSkirmishScenarioMarkerComponent FindScenarioMarkerComponent(IEntity entity)
	{
		if (!entity)
			return null;

		JLH_DCF_AmbientSkirmishScenarioMarkerComponent marker;

		marker = JLH_DCF_InfantryClashSkirmishMarkerComponent.Cast(entity.FindComponent(JLH_DCF_InfantryClashSkirmishMarkerComponent));
		if (marker) return marker;
		marker = JLH_DCF_SmallPatrolSkirmishMarkerComponent.Cast(entity.FindComponent(JLH_DCF_SmallPatrolSkirmishMarkerComponent));
		if (marker) return marker;
		marker = JLH_DCF_VehicleSkirmishMarkerComponent.Cast(entity.FindComponent(JLH_DCF_VehicleSkirmishMarkerComponent));
		if (marker) return marker;
		marker = JLH_DCF_ConvoyAmbushUSSRSkirmishMarkerComponent.Cast(entity.FindComponent(JLH_DCF_ConvoyAmbushUSSRSkirmishMarkerComponent));
		if (marker) return marker;
		marker = JLH_DCF_ConvoyAmbushUSSkirmishMarkerComponent.Cast(entity.FindComponent(JLH_DCF_ConvoyAmbushUSSkirmishMarkerComponent));
		if (marker) return marker;
		marker = JLH_DCF_SupplyTruckRaidSkirmishMarkerComponent.Cast(entity.FindComponent(JLH_DCF_SupplyTruckRaidSkirmishMarkerComponent));
		if (marker) return marker;
		marker = JLH_DCF_CampAssaultSkirmishMarkerComponent.Cast(entity.FindComponent(JLH_DCF_CampAssaultSkirmishMarkerComponent));
		if (marker) return marker;
		marker = JLH_DCF_CheckpointAssaultSkirmishMarkerComponent.Cast(entity.FindComponent(JLH_DCF_CheckpointAssaultSkirmishMarkerComponent));
		if (marker) return marker;
		marker = JLH_DCF_RoadblockBreakthroughSkirmishMarkerComponent.Cast(entity.FindComponent(JLH_DCF_RoadblockBreakthroughSkirmishMarkerComponent));
		if (marker) return marker;
		marker = JLH_DCF_RoadblockAssaultSkirmishMarkerComponent.Cast(entity.FindComponent(JLH_DCF_RoadblockAssaultSkirmishMarkerComponent));
		if (marker) return marker;
		marker = JLH_DCF_DownedCrewRescueSkirmishMarkerComponent.Cast(entity.FindComponent(JLH_DCF_DownedCrewRescueSkirmishMarkerComponent));
		if (marker) return marker;
		marker = JLH_DCF_ReconTeamSkirmishMarkerComponent.Cast(entity.FindComponent(JLH_DCF_ReconTeamSkirmishMarkerComponent));
		if (marker) return marker;
		marker = JLH_DCF_DownedUSHeliRescueSkirmishMarkerComponent.Cast(entity.FindComponent(JLH_DCF_DownedUSHeliRescueSkirmishMarkerComponent));
		if (marker) return marker;

		return null;
	}

	protected JLH_DCF_AmbientSkirmishIntensity ResolveIntensity()
	{
		if (m_eIntensity != JLH_DCF_AmbientSkirmishIntensity.RANDOM)
			return m_eIntensity;

		int roll = Math.RandomInt(0, 100);
		if (roll < 50)
			return JLH_DCF_AmbientSkirmishIntensity.LIGHT;
		if (roll < 85)
			return JLH_DCF_AmbientSkirmishIntensity.MEDIUM;
		return JLH_DCF_AmbientSkirmishIntensity.HEAVY;
	}

	protected void NormalizeEditorFields()
	{
		m_iMinInfantryPerSide = Math.Clamp(m_iMinInfantryPerSide, 2, 20);
		m_iMaxInfantryPerSide = Math.Clamp(m_iMaxInfantryPerSide, m_iMinInfantryPerSide, 20);
		m_fActivationRadius = Math.Clamp(m_fActivationRadius, 0.0, 5000.0);
		m_fSpawnRadius = Math.Clamp(m_fSpawnRadius, 0.0, 5000.0);
		m_fMinPlayerDistance = Math.Clamp(m_fMinPlayerDistance, 0.0, 3000.0);
		m_fGroupSeparation = Math.Clamp(m_fGroupSeparation, 20.0, 500.0);
		m_fUSHQHardSafetyRadius = Math.Clamp(m_fUSHQHardSafetyRadius, 0.0, 3000.0);
		m_fUSBaseCoreSafetyRadius = Math.Clamp(m_fUSBaseCoreSafetyRadius, 0.0, 3000.0);
		m_fCleanupDistance = Math.Clamp(m_fCleanupDistance, 250.0, 8000.0);
		m_iCooldownSeconds = Math.Clamp(m_iCooldownSeconds, 0, 7200);
		m_iMaxActiveSkirmishes = Math.Clamp(m_iMaxActiveSkirmishes, 0, 6);
		m_fSpawnChance = Math.Clamp(m_fSpawnChance, 0.0, 1.0);
		m_iDownedUSHeliDestroyDelayMs = Math.Clamp(m_iDownedUSHeliDestroyDelayMs, 0, 10000);
		m_iDownedCrewVehicleDestroyDelayMs = Math.Clamp(m_iDownedCrewVehicleDestroyDelayMs, 0, 10000);
		m_fConvoyAmbushRoadSearchRadius = Math.Clamp(m_fConvoyAmbushRoadSearchRadius, 0.0, 300.0);
		m_iConvoyAmbushMinEnemyGroups = Math.Clamp(m_iConvoyAmbushMinEnemyGroups, 1, 6);
		m_iConvoyAmbushMaxEnemyGroups = Math.Clamp(m_iConvoyAmbushMaxEnemyGroups, m_iConvoyAmbushMinEnemyGroups, 6);
		if (m_sDownedUSHeliLivePrefab == "" || m_sDownedUSHeliLivePrefab == ResourceName.Empty)
			m_sDownedUSHeliLivePrefab = SCENE_US_HELI_LIVE;
		if (m_sFriendlyFactionKey == "")
			m_sFriendlyFactionKey = "US";
		if (m_sEnemyFactionKey == "")
			m_sEnemyFactionKey = "USSR";
	}

	float GetEditorVisualizerRadius()
	{
		return Math.Clamp(m_fActivationRadius, 1.0, 5000.0);
	}

	string GetSkirmishNodeKey()
	{
		return m_sNodeKey;
	}

	string GetSkirmishNodeName()
	{
		return JLH_DCF_NodeDebug.EntityLabel(GetOwner());
	}

	float GetConfiguredGroupSeparation()
	{
		return m_fGroupSeparation;
	}

	protected float GetEffectiveActivationRadius()
	{
		return Math.Clamp(m_fActivationRadius, 0.0, 5000.0) * GetRuntimeAreaScale();
	}

	protected float GetEffectiveSpawnRadius()
	{
		return Math.Clamp(m_fSpawnRadius, 0.0, 5000.0) * GetRuntimeAreaScale();
	}

	protected float GetRuntimeAreaScale()
	{
		if (!m_bScaleDistancesByEntityScale)
			return 1.0;

		IEntity owner = GetOwner();
		if (!owner)
			return 1.0;

		float scale = owner.GetScale();
		if (scale <= 0.001)
			return 1.0;

		return Math.Clamp(scale, 0.01, 1000.0);
	}

	protected vector DirectionFromTo(vector from, vector to)
	{
		vector delta = to - from;
		delta[1] = 0.0;
		float length = Math.Sqrt((delta[0] * delta[0]) + (delta[2] * delta[2]));
		if (length <= 0.001)
			return JLH_AddonWorldQuery.DirectionFromAngle(Math.RandomFloat(0.0, 360.0));

		return Vector(delta[0] / length, 0, delta[2] / length);
	}

	protected string FormatVehicleLabelList(array<string> labels)
	{
		if (!labels || labels.IsEmpty())
			return "NONE";

		string result = "";
		foreach (string labelName : labels)
		{
			if (labelName == "" || labelName == "NONE")
				continue;

			if (result != "")
				result += ",";

			result += labelName;
		}

		if (result == "")
			return "NONE";

		return result;
	}

	protected void MarkCooldown()
	{
		m_iNextAllowedSpawnTick = System.GetTickCount() + (m_iCooldownSeconds * 1000);
	}

	protected void DeferSpawnForRuntimeBudget(int delayMs)
	{
		if (delayMs < 250)
			delayMs = 250;

		m_iNextAllowedSpawnTick = System.GetTickCount() + delayMs;
		LogEvent("ambient_skirmish_spawn_deferred_budget", "delayMs=" + delayMs.ToString(), false);
	}

	protected void LogNoLiveDeployedHumanBlocked()
	{
		int now = System.GetTickCount();
		if (!m_bBlockedNoLiveDeployedHuman)
		{
			m_bBlockedNoLiveDeployedHuman = true;
			m_iLastNoLiveDeployedHumanLogTick = now;
			LogEvent("ambient_skirmish_blocked_no_live_deployed_human", "", false);
			return;
		}

		if (!m_bDebug)
			return;

		if (now - m_iLastNoLiveDeployedHumanLogTick < NO_LIVE_DEPLOYED_HUMAN_DEBUG_REPEAT_MS)
			return;

		m_iLastNoLiveDeployedHumanLogTick = now;
		LogEvent("ambient_skirmish_blocked_no_live_deployed_human", "state=still_blocked", false);
	}

	protected void LogLiveDeployedHumanResumed(int livePlayerCount)
	{
		if (!m_bBlockedNoLiveDeployedHuman)
			return;

		m_bBlockedNoLiveDeployedHuman = false;
		m_iLastNoLiveDeployedHumanLogTick = 0;
		LogEvent("ambient_skirmish_resumed_live_deployed_human", "count=" + livePlayerCount.ToString(), false);
	}

	protected void LogEnabledStateChange()
	{
		if (!m_bEnabledStateInitialized)
		{
			m_bEnabledStateInitialized = true;
			m_bLastEnabledState = m_bEnabled;
			return;
		}

		if (m_bLastEnabledState == m_bEnabled)
			return;

		m_bLastEnabledState = m_bEnabled;
		if (m_bEnabled)
			LogEvent("ambient_skirmish_area_enabled", "", true);
		else
			LogEvent("ambient_skirmish_area_disabled", "", true);
	}

	protected bool ShouldLogVehicleVerbose()
	{
		if (!m_bDebug)
			return false;

		if (m_eDebugLevel == JLH_AmbientSkirmishDebugLevel.VERBOSE)
			return true;
		if (m_eDebugLevel == JLH_AmbientSkirmishDebugLevel.DEEP)
			return true;

		return false;
	}

	protected bool ShouldLogVehicleDeep()
	{
		if (!m_bDebug)
			return false;

		return m_eDebugLevel == JLH_AmbientSkirmishDebugLevel.DEEP;
	}

	void LogEvent(string eventName, string details, bool force)
	{
		if (!force && !m_bDebug)
			return;

		string key = eventName + ":" + m_sNodeKey;
		if (details != "")
			key += ":" + details;

		if (!force && !ShouldLogKey(key))
			return;

		string nodeLabel = "NONE";
		string position = "0 0 0";
		IEntity owner = GetOwner();
		if (owner)
		{
			nodeLabel = JLH_DCF_NodeDebug.EntityLabel(owner);
			position = owner.GetOrigin().ToString();
		}

		string line = eventName;
		line += " area=" + nodeLabel;
		line += " key=" + m_sNodeKey;
		line += " pos=" + position;
		if (details != "")
			line += " " + details;

		JLH_AddonDebug.Event(LOG_PREFIX, line, true);
	}

	protected bool ShouldLogKey(string key)
	{
		int now = System.GetTickCount();
		int index = m_aLogKeys.Find(key);
		if (index >= 0)
		{
			if (now - m_aLogTicks[index] < LOG_THROTTLE_MS)
				return false;

			m_aLogTicks[index] = now;
			return true;
		}

		m_aLogKeys.Insert(key);
		m_aLogTicks.Insert(now);
		return true;
	}

	protected string BuildNodeKey(IEntity owner)
	{
		if (!owner)
			return "NONE";

		string name = owner.GetName();
		if (name != "")
			return name;

		vector pos = owner.GetOrigin();
		float gridX = Math.Floor(pos[0]);
		float gridZ = Math.Floor(pos[2]);
		return JLH_DCF_NodeDebug.EntityLabel(owner) + ":" + gridX.ToString() + ":" + gridZ.ToString();
	}
}

class JLH_DCF_AmbientSkirmishManager
{
	protected static const string LOG_PREFIX = "[AMBIENT_SKIRMISH]";
	protected static const int SCAN_INTERVAL_MS = 43000;
	protected static const int AREA_TICK_STAGGER_MS = 3500;
	protected static const int MIN_AREA_TICK_STAGGER_MS = 250;
	protected static const int TICK_SWEEP_RESERVE_MS = 4000;
	protected static const int GLOBAL_ACTIVE_CAP = 4;

	protected static ref array<JLH_DCF_AmbientSkirmishAreaComponent> s_aAreas = {};
	protected static bool s_bStarted;
	protected static int s_iNextSkirmishId = 1;
	protected static int s_iTickGeneration;

	static void RegisterArea(JLH_DCF_AmbientSkirmishAreaComponent area)
	{
		PruneInvalidAreas();
		if (!area || s_aAreas.Contains(area))
			return;

		s_aAreas.Insert(area);
		JLH_AddonDebug.Event(LOG_PREFIX, "ambient_skirmish_area_registered area=" + JLH_DCF_NodeDebug.EntityLabel(area.GetOwner()) + " manager=true", false);

		if (s_bStarted)
			area.RegisterArea();
	}

	static void Start(SCR_GameModeCampaign gameMode)
	{
		if (s_bStarted)
			return;

		s_bStarted = true;
		s_iNextSkirmishId = 1;
		s_iTickGeneration = 0;

		foreach (JLH_DCF_AmbientSkirmishAreaComponent area : s_aAreas)
		{
			if (area)
				area.RegisterArea();
		}

		if (GetGame() && GetGame().GetCallqueue())
		{
			GetGame().GetCallqueue().Remove(JLH_DCF_AmbientSkirmishManager_Tick);
			GetGame().GetCallqueue().CallLater(JLH_DCF_AmbientSkirmishManager_Tick, SCAN_INTERVAL_MS, true);
		}

		JLH_AddonDebug.Event(LOG_PREFIX, "ambient_skirmish_manager_started areas=" + s_aAreas.Count().ToString() + " scan=23 globalCap=" + GLOBAL_ACTIVE_CAP.ToString() + " staggerMs=" + AREA_TICK_STAGGER_MS.ToString(), true);
	}

	static void ResetForNewGame()
	{
		if (GetGame() && GetGame().GetCallqueue())
			GetGame().GetCallqueue().Remove(JLH_DCF_AmbientSkirmishManager_Tick);

		foreach (JLH_DCF_AmbientSkirmishAreaComponent area : s_aAreas)
		{
			if (area)
				area.CleanupAllRuntime("ambient_skirmish_cleanup_reset", true);
		}

		s_bStarted = false;
		s_iNextSkirmishId = 1;
		s_iTickGeneration++;
		PruneInvalidAreas();
	}

	static int NextSkirmishId()
	{
		int id = s_iNextSkirmishId;
		s_iNextSkirmishId++;
		return id;
	}

	static bool HasGlobalCapRoom(out int activeCount, out int activeCap)
	{
		activeCap = GLOBAL_ACTIVE_CAP;
		activeCount = CountActiveGlobal();
		if (activeCap <= 0)
			return false;

		return activeCount < activeCap;
	}

	static int CountActiveGlobal()
	{
		PruneInvalidAreas();
		int count = 0;
		foreach (JLH_DCF_AmbientSkirmishAreaComponent area : s_aAreas)
		{
			if (area)
				count += area.CountActiveLocalRuntime();
		}

		return count;
	}

	static void Tick()
	{
		if (!s_bStarted)
			return;

		PruneInvalidAreas();

		s_iTickGeneration++;
		int generation = s_iTickGeneration;
		int staggerMs = ResolveAreaTickStaggerMs(s_aAreas.Count());

		for (int i = 0; i < s_aAreas.Count(); i++)
		{
			JLH_DCF_AmbientSkirmishAreaComponent area = s_aAreas[i];
			if (area)
				ScheduleAreaTick(area, generation, i * staggerMs);
		}
	}

	static void TickAreaDelayed(JLH_DCF_AmbientSkirmishAreaComponent area, int generation)
	{
		if (!s_bStarted || generation != s_iTickGeneration)
			return;

		if (!area || !area.GetOwner())
			return;

		area.Tick();
	}

	protected static void ScheduleAreaTick(notnull JLH_DCF_AmbientSkirmishAreaComponent area, int generation, int delayMs)
	{
		if (delayMs <= 0 || !GetGame() || !GetGame().GetCallqueue())
		{
			TickAreaDelayed(area, generation);
			return;
		}

		GetGame().GetCallqueue().CallLater(JLH_DCF_AmbientSkirmishManager_TickAreaDelayed, delayMs, false, area, generation);
	}

	protected static int ResolveAreaTickStaggerMs(int areaCount)
	{
		if (areaCount <= 1)
			return 0;

		int maxSweepMs = SCAN_INTERVAL_MS - TICK_SWEEP_RESERVE_MS;
		if (maxSweepMs < MIN_AREA_TICK_STAGGER_MS)
			maxSweepMs = MIN_AREA_TICK_STAGGER_MS;

		int dynamicStaggerMs = maxSweepMs / (areaCount - 1);
		if (dynamicStaggerMs < 1)
			dynamicStaggerMs = 1;

		if (dynamicStaggerMs > AREA_TICK_STAGGER_MS)
			return AREA_TICK_STAGGER_MS;

		return dynamicStaggerMs;
	}

	protected static void PruneInvalidAreas()
	{
		for (int i = s_aAreas.Count() - 1; i >= 0; i--)
		{
			if (!s_aAreas[i] || !s_aAreas[i].GetOwner())
				s_aAreas.Remove(i);
		}
	}
}

void JLH_DCF_AmbientSkirmishManager_Tick()
{
	JLH_DCF_AmbientSkirmishManager.Tick();
}

void JLH_DCF_AmbientSkirmishManager_TickAreaDelayed(JLH_DCF_AmbientSkirmishAreaComponent area, int generation)
{
	JLH_DCF_AmbientSkirmishManager.TickAreaDelayed(area, generation);
}

void JLH_AmbientSkirmish_DestroySceneVehicleDelayed(JLH_DCF_AmbientSkirmishAreaComponent area, IEntity vehicle, int recordId, string label)
{
	if (area)
		area.DestroySceneVehicleDelayed(vehicle, recordId, label);
}

void JLH_AmbientSkirmish_RetryCrashSurvivorLayout(JLH_DCF_AmbientSkirmishAreaComponent area, JLH_AmbientSkirmishRecord record, JLH_AmbientSkirmishEventDefinition definition, int attempt)
{
	if (area)
		area.RetryCrashSurvivorLayout(record, definition, attempt);
}

void JLH_AmbientSkirmish_SpawnDelayedCrashSurvivors(JLH_DCF_AmbientSkirmishAreaComponent area, JLH_AmbientSkirmishRecord record, JLH_AmbientSkirmishEventDefinition definition)
{
	if (area)
		area.SpawnDelayedCrashSurvivors(record, definition);
}

void JLH_AmbientSkirmish_FinalizeVehicleSkirmishMount(JLH_DCF_AmbientSkirmishAreaComponent area, JLH_AmbientSkirmishRecord record, JLH_AmbientSkirmishEventDefinition definition, int attempt)
{
	if (area)
		area.FinalizeVehicleSkirmishMount(record, definition, attempt);
}

void JLH_AmbientSkirmish_ValidateVehicleSkirmishMount(JLH_DCF_AmbientSkirmishAreaComponent area, JLH_AmbientSkirmishRecord record, JLH_AmbientSkirmishEventDefinition definition, int attempt)
{
	if (area)
		area.ValidateVehicleSkirmishMount(record, definition, attempt);
}

void JLH_AmbientSkirmish_DismountConvoyAmbushUSTroops(JLH_DCF_AmbientSkirmishAreaComponent area, JLH_AmbientSkirmishRecord record, SCR_AIGroup group, IEntity truck)
{
	if (area)
		area.DismountConvoyAmbushUSTroops(record, group, truck);
}

void JLH_AmbientSkirmish_ConvoyAmbushDestroyRearDelayed(JLH_DCF_AmbientSkirmishAreaComponent area, JLH_AmbientSkirmishRecord record, IEntity rearVehicle)
{
	if (area)
		area.ConvoyAmbushDestroyRearDelayed(record, rearVehicle);
}
