enum JLH_AmbientVehiclePatrolDebugLevel
{
	OFF = 0,
	BASIC = 1,
	VERBOSE = 2,
	DEEP = 3
}

class JLH_AmbientVehiclePatrolVehicleFilterSet
{
	ref array<string> RequiredLabels = {};
	ref array<string> PreferredLabels = {};
	ref array<string> ExcludedLabels = {};
	string Source;
}

class JLH_AmbientVehiclePatrolRoute
{
	ref array<vector> Points = {};
	vector SpawnPosition;
	vector Facing;
	string Source;

	void Init(vector spawnPosition, vector facing, string source)
	{
		SpawnPosition = spawnPosition;
		Facing = facing;
		Source = source;
	}
}

class JLH_AmbientVehiclePatrolRuntime
{
	int Id;
	IEntity VehicleEntity;
	SCR_AIGroup CrewGroup;
	IEntity DriverEntity;
	IEntity GunnerEntity;
	ResourceName VehiclePrefab;
	ResourceName CrewPrefab;
	ref JLH_AmbientVehiclePatrolRoute Route;
	ref JLH_AddonRuntimeEntityRegistry Registry;
	int MountAttempts;
	bool Started;
	bool m_bCleared;
	int m_iClearedTick;

	void Init(int id, IEntity vehicle, SCR_AIGroup crewGroup, ResourceName vehiclePrefab, ResourceName crewPrefab, JLH_AmbientVehiclePatrolRoute route)
	{
		Id = id;
		VehicleEntity = vehicle;
		CrewGroup = crewGroup;
		VehiclePrefab = vehiclePrefab;
		CrewPrefab = crewPrefab;
		Route = route;
		Registry = new JLH_AddonRuntimeEntityRegistry();
		Registry.SetRuntimeOwner("AMBIENT_VEHICLE_PATROL");
		m_bCleared = false;
		m_iClearedTick = 0;
	}
}

class JLH_AmbientVehiclePatrolRouteGenerator
{
	protected static const float MAX_ROAD_SNAP_DISTANCE = 450.0;
	protected static const float MIN_ROUTE_SPAN_METERS = 120.0;
	protected static const float MAX_ROUTE_SPAN_METERS = 650.0;
	protected static const float MIN_POINT_SPACING_METERS = 35.0;
	protected static const float MAX_ROUTE_SLOPE_DEGREES = 18.0;

	static bool TryGenerate(vector center, float patrolRadius, bool preferRoadRoute, out JLH_AmbientVehiclePatrolRoute route, out string reason)
	{
		route = null;
		reason = "";

		if (TryGenerateRoadRoute(center, patrolRadius, route, reason))
			return true;

		if (preferRoadRoute)
			return false;

		return TryGenerateFallbackGroundRoute(center, patrolRadius, route, reason);
	}

	protected static bool TryGenerateRoadRoute(vector center, float patrolRadius, out JLH_AmbientVehiclePatrolRoute route, out string reason)
	{
		route = null;
		reason = "";

		RoadNetworkManager roadManager = GetRoadNetworkManager();
		if (!roadManager)
		{
			reason = "road_network_missing";
			return false;
		}

		BaseRoad road;
		float snapDistance;
		int roadResult = roadManager.GetClosestRoad(center, road, snapDistance, false);
		if (roadResult <= 0 || !road)
		{
			reason = "no_road_near_area";
			return false;
		}

		float maxSnap = Math.Clamp(patrolRadius, 90.0, MAX_ROAD_SNAP_DISTANCE);
		if (snapDistance > maxSnap)
		{
			reason = "road_snap_too_far";
			return false;
		}

		array<vector> roadPoints = {};
		if (road.GetPoints(roadPoints) < 2)
		{
			reason = "road_points_missing";
			return false;
		}

		float centerDistanceAlong;
		vector projectedCenter;
		vector projectedFacing;
		if (!FindClosestPointOnRoad(roadPoints, center, projectedCenter, projectedFacing, centerDistanceAlong))
		{
			reason = "road_projection_failed";
			return false;
		}

		float roadLength = GetPolylineLength(roadPoints);
		if (roadLength < MIN_ROUTE_SPAN_METERS)
		{
			reason = "road_too_short";
			return false;
		}

		float span = Math.Clamp(patrolRadius, MIN_ROUTE_SPAN_METERS, MAX_ROUTE_SPAN_METERS);
		float startDistance = Math.Clamp(centerDistanceAlong - (span * 0.5), 0.0, roadLength);
		float endDistance = Math.Clamp(centerDistanceAlong + (span * 0.5), 0.0, roadLength);
		if (endDistance - startDistance < MIN_ROUTE_SPAN_METERS * 0.55)
		{
			startDistance = Math.Clamp(centerDistanceAlong, 0.0, roadLength);
			endDistance = Math.Clamp(startDistance + Math.Min(span, roadLength - startDistance), 0.0, roadLength);
			if (endDistance - startDistance < MIN_ROUTE_SPAN_METERS * 0.55)
			{
				endDistance = Math.Clamp(centerDistanceAlong, 0.0, roadLength);
				startDistance = Math.Clamp(endDistance - Math.Min(span, endDistance), 0.0, roadLength);
			}
		}

		if (endDistance - startDistance < MIN_ROUTE_SPAN_METERS * 0.4)
		{
			reason = "route_span_too_short";
			return false;
		}

		int pointCount = Math.Clamp((int) Math.Round((endDistance - startDistance) / 140.0) + 1, 2, 5);
		ref array<vector> generatedPoints = {};
		vector firstFacing = projectedFacing;
		for (int i = 0; i < pointCount; i++)
		{
			float alpha = 0.0;
			if (pointCount > 1)
				alpha = i / (pointCount - 1.0);

			float distanceAlong = startDistance + ((endDistance - startDistance) * alpha);
			vector routePoint;
			vector routeFacing;
			if (!BuildPointAtDistance(roadPoints, distanceAlong, routePoint, routeFacing))
				continue;

			routePoint = JLH_AddonWorldQuery.GroundPosition(routePoint);
			if (!IsRoutePointSafe(routePoint))
				continue;

			if (IsDuplicatePoint(routePoint, generatedPoints))
				continue;

			if (generatedPoints.IsEmpty())
				firstFacing = routeFacing;

			generatedPoints.Insert(routePoint);
		}

		if (generatedPoints.Count() < 2)
		{
			reason = "route_points_invalid";
			return false;
		}

		vector facing = NormalizeHorizontalDirection(firstFacing);
		if (vector.Distance(facing, "0 0 0") <= 0.001 && generatedPoints.Count() >= 2)
			facing = DirectionFromTo(generatedPoints[0], generatedPoints[1]);

		route = new JLH_AmbientVehiclePatrolRoute();
		route.Init(generatedPoints[0], facing, "road");
		foreach (vector point : generatedPoints)
			route.Points.Insert(point);

		return true;
	}

	protected static bool TryGenerateFallbackGroundRoute(vector center, float patrolRadius, out JLH_AmbientVehiclePatrolRoute route, out string reason)
	{
		route = null;
		reason = "";

		float radius = Math.Clamp(patrolRadius * 0.45, 55.0, 220.0);
		ref array<vector> generatedPoints = {};
		for (int i = 0; i < 4; i++)
		{
			float angle = i * 90.0;
			vector point = JLH_AddonWorldQuery.GroundPosition(center + (JLH_AddonWorldQuery.DirectionFromAngle(angle) * radius));
			if (!IsRoutePointSafe(point))
				continue;

			if (!IsDuplicatePoint(point, generatedPoints))
				generatedPoints.Insert(point);
		}

		if (generatedPoints.Count() < 2)
		{
			reason = "fallback_route_points_invalid";
			return false;
		}

		route = new JLH_AmbientVehiclePatrolRoute();
		route.Init(generatedPoints[0], DirectionFromTo(generatedPoints[0], generatedPoints[1]), "ground_fallback");
		foreach (vector point : generatedPoints)
			route.Points.Insert(point);

		return true;
	}

	protected static bool IsRoutePointSafe(vector point)
	{
		if (JLH_AddonWorldQuery.GetSlopeDegrees(point) > MAX_ROUTE_SLOPE_DEGREES)
			return false;

		return true;
	}

	protected static bool IsDuplicatePoint(vector point, notnull array<vector> existingPoints)
	{
		foreach (vector existing : existingPoints)
		{
			if (HorizontalDistance(point, existing) < MIN_POINT_SPACING_METERS)
				return true;
		}

		return false;
	}

	protected static RoadNetworkManager GetRoadNetworkManager()
	{
		ChimeraGame game = ChimeraGame.Cast(GetGame());
		if (!game)
			return null;

		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(game.GetAIWorld());
		if (!aiWorld)
			return null;

		return aiWorld.GetRoadNetworkManager();
	}

	protected static bool FindClosestPointOnRoad(notnull array<vector> points, vector candidate, out vector roadPosition, out vector roadDirection, out float distanceAlong)
	{
		roadPosition = "0 0 0";
		roadDirection = "0 0 1";
		distanceAlong = 0.0;

		bool found = false;
		float bestDistanceSq = 99999999.0;
		float cumulativeDistance = 0.0;
		for (int i = 0; i < points.Count() - 1; i++)
		{
			vector start = points[i];
			vector finish = points[i + 1];
			vector segment = finish - start;
			segment[1] = 0.0;
			float segmentLength = Math.Sqrt((segment[0] * segment[0]) + (segment[2] * segment[2]));
			if (segmentLength <= 0.01)
				continue;

			float t = (((candidate[0] - start[0]) * segment[0]) + ((candidate[2] - start[2]) * segment[2])) / (segmentLength * segmentLength);
			t = Math.Clamp(t, 0.0, 1.0);

			vector projected = start + ((finish - start) * t);
			float distanceSq = HorizontalDistanceSq(candidate, projected);
			if (!found || distanceSq < bestDistanceSq)
			{
				found = true;
				bestDistanceSq = distanceSq;
				roadPosition = projected;
				roadDirection = DirectionFromTo(start, finish);
				distanceAlong = cumulativeDistance + (segmentLength * t);
			}

			cumulativeDistance += segmentLength;
		}

		return found;
	}

	protected static float GetPolylineLength(notnull array<vector> points)
	{
		float total = 0.0;
		for (int i = 0; i < points.Count() - 1; i++)
			total += HorizontalDistance(points[i], points[i + 1]);

		return total;
	}

	protected static bool BuildPointAtDistance(notnull array<vector> points, float requestedDistance, out vector point, out vector direction)
	{
		point = "0 0 0";
		direction = "0 0 1";

		float remaining = Math.Max(requestedDistance, 0.0);
		for (int i = 0; i < points.Count() - 1; i++)
		{
			vector start = points[i];
			vector finish = points[i + 1];
			float segmentLength = HorizontalDistance(start, finish);
			if (segmentLength <= 0.01)
				continue;

			if (remaining <= segmentLength)
			{
				float t = remaining / segmentLength;
				point = start + ((finish - start) * t);
				direction = DirectionFromTo(start, finish);
				return true;
			}

			remaining -= segmentLength;
		}

		if (points.Count() >= 2)
		{
			point = points[points.Count() - 1];
			direction = DirectionFromTo(points[points.Count() - 2], points[points.Count() - 1]);
			return true;
		}

		return false;
	}

	protected static float HorizontalDistanceSq(vector a, vector b)
	{
		float dx = a[0] - b[0];
		float dz = a[2] - b[2];
		return (dx * dx) + (dz * dz);
	}

	protected static float HorizontalDistance(vector a, vector b)
	{
		return Math.Sqrt(HorizontalDistanceSq(a, b));
	}

	protected static vector DirectionFromTo(vector from, vector to)
	{
		vector delta = to - from;
		return NormalizeHorizontalDirection(delta);
	}

	protected static vector NormalizeHorizontalDirection(vector direction)
	{
		direction[1] = 0.0;
		float length = Math.Sqrt((direction[0] * direction[0]) + (direction[2] * direction[2]));
		if (length <= 0.001)
			return JLH_AddonWorldQuery.DirectionFromAngle(Math.RandomFloat(0.0, 360.0));

		return Vector(direction[0] / length, 0.0, direction[2] / length);
	}
}

[ComponentEditorProps(category: "JLH Dynamic Conflict Framework/Ambient Vehicle Patrols", description: "Optional vehicle patrol marker. As a child or placed marker inside an Ambient Vehicle Patrol area, it can override vehicle label filters for that spawn point.")]
class JLH_AmbientVehiclePatrolMarkerComponentClass : ScriptComponentClass
{
}

class JLH_AmbientVehiclePatrolMarkerComponent : ScriptComponent
{
	[Attribute("1", UIWidgets.CheckBox, "Marker can be selected by parent Ambient Vehicle Patrol areas.", category: "JLH DCF Ambient Vehicle Patrol Marker")]
	protected bool m_bEnabled = true;

	[Attribute("0", UIWidgets.ComboBox, "Override: every selected vehicle must have all of these labels.", "", ParamEnumArray.FromEnum(EEditableEntityLabel), category: "JLH DCF Ambient Vehicle Patrol Marker - Vehicle Labels")]
	protected ref array<EEditableEntityLabel> m_aRequiredVehicleLabels;

	[Attribute("0", UIWidgets.ComboBox, "Override: prefer vehicles with these labels after required/excluded filtering.", "", ParamEnumArray.FromEnum(EEditableEntityLabel), category: "JLH DCF Ambient Vehicle Patrol Marker - Vehicle Labels")]
	protected ref array<EEditableEntityLabel> m_aPreferredVehicleLabels;

	[Attribute("0", UIWidgets.ComboBox, "Override: reject vehicles with any of these labels.", "", ParamEnumArray.FromEnum(EEditableEntityLabel), category: "JLH DCF Ambient Vehicle Patrol Marker - Vehicle Labels")]
	protected ref array<EEditableEntityLabel> m_aExcludedVehicleLabels;

	bool IsEnabled()
	{
		return m_bEnabled;
	}

	bool HasVehicleFilterOverride()
	{
		return HasEditableVehicleLabels(m_aRequiredVehicleLabels) || HasEditableVehicleLabels(m_aPreferredVehicleLabels) || HasEditableVehicleLabels(m_aExcludedVehicleLabels);
	}

	void CopyVehicleLabelFilters(notnull array<string> requiredLabels, notnull array<string> preferredLabels, notnull array<string> excludedLabels)
	{
		requiredLabels.Clear();
		preferredLabels.Clear();
		excludedLabels.Clear();
		CopyEditableVehicleLabelsToNames(m_aRequiredVehicleLabels, requiredLabels);
		CopyEditableVehicleLabelsToNames(m_aPreferredVehicleLabels, preferredLabels);
		CopyEditableVehicleLabelsToNames(m_aExcludedVehicleLabels, excludedLabels);
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

	protected void CopyEditableVehicleLabelsToNames(array<EEditableEntityLabel> sourceLabels, notnull array<string> targetLabels)
	{
		if (!sourceLabels)
			return;

		foreach (EEditableEntityLabel label : sourceLabels)
		{
			string labelName = SCR_Enum.GetEnumName(EEditableEntityLabel, label);
			if (labelName == "" || labelName == "NONE" || targetLabels.Contains(labelName))
				continue;

			targetLabels.Insert(labelName);
		}
	}
}

[ComponentEditorProps(category: "JLH Dynamic Conflict Framework/Ambient Vehicle Patrols", description: "Area or marker driven automatic ambient vehicle patrol.")]
class JLH_AmbientVehiclePatrolAreaComponentClass : ScriptComponentClass
{
}

class JLH_AmbientVehiclePatrolAreaComponent : ScriptComponent
{
	protected static const string LOG_PREFIX = "[AMBIENT_VEHICLE_PATROL]";
	protected static const string SYSTEM_NAME = "AMBIENT_VEHICLE_PATROL";
	protected static const int PENDING_MOUNT_DELAY_MS = 1500;
	protected static const int MOUNT_RETRY_DELAY_MS = 1500;
	protected static const int MAX_MOUNT_ATTEMPTS = 4;
	protected static const int LOG_THROTTLE_MS = 30000;
	protected static const int MAX_MOUNTED_CREW_FOR_ARMED_PATROL = 3;
	protected static const ResourceName DEFAULT_USSR_VEHICLE_CREW_GROUP_PREFAB = "{D4CF0B2B840CF001}Prefabs/Groups/JLH_DCF/JLH_USSR_VehicleGroup.et";
	protected static const ResourceName DEFAULT_US_VEHICLE_CREW_GROUP_PREFAB = "{101C1DACCD535E45}Prefabs/Groups/JLH_DCF/JLH_US_VehicleGroup.et";
	protected static ref array<JLH_AmbientVehiclePatrolMarkerComponent> s_aAreaMarkerQueryResults = {};
	protected static IEntity s_AreaMarkerQueryOwner;
	protected static vector s_vAreaMarkerQueryCenter;
	protected static float s_fAreaMarkerQueryRadius;

	[Attribute("1", UIWidgets.CheckBox, "Enable this Ambient Vehicle Patrol area or marker.", category: "JLH DCF Ambient Vehicle Patrol")]
	protected bool m_bEnabled = true;

	[Attribute("USSR", UIWidgets.EditBox, "Vehicle and crew faction key used by the shared compatibility resolver.", category: "JLH DCF Ambient Vehicle Patrol")]
	protected string m_sFactionKey = "USSR";

	[Attribute("0", UIWidgets.ComboBox, "Every selected vehicle must have all of these labels. Leave empty to allow any faction vehicle not rejected by excluded labels.", "", ParamEnumArray.FromEnum(EEditableEntityLabel), category: "JLH DCF Ambient Vehicle Patrol - Vehicle Labels")]
	protected ref array<EEditableEntityLabel> m_aRequiredVehicleLabels;

	[Attribute("0", UIWidgets.ComboBox, "Prefer vehicles with these labels after required/excluded filtering.", "", ParamEnumArray.FromEnum(EEditableEntityLabel), category: "JLH DCF Ambient Vehicle Patrol - Vehicle Labels")]
	protected ref array<EEditableEntityLabel> m_aPreferredVehicleLabels;

	[Attribute("0", UIWidgets.ComboBox, "Reject vehicles with any of these labels.", "", ParamEnumArray.FromEnum(EEditableEntityLabel), category: "JLH DCF Ambient Vehicle Patrol - Vehicle Labels")]
	protected ref array<EEditableEntityLabel> m_aExcludedVehicleLabels;

	[Attribute("450", UIWidgets.EditBox, "Road patrol generation radius around this area or marker.", "80 1200 1", category: "JLH DCF Ambient Vehicle Patrol - Distances")]
	protected float m_fPatrolRadius = 450.0;

	[Attribute("1000", UIWidgets.EditBox, "Activate when a live deployed player is within this distance.", "0 5000 1", category: "JLH DCF Ambient Vehicle Patrol - Distances")]
	protected float m_fActivationRadius = 1000.0;

	[Attribute("1800", UIWidgets.EditBox, "Cleanup this patrol when all live deployed players are farther than this distance.", "250 8000 1", category: "JLH DCF Ambient Vehicle Patrol - Distances")]
	protected float m_fDespawnRadius = 1800.0;

	[Attribute("220", UIWidgets.EditBox, "Reject patrol spawns closer than this to any live deployed player.", "0 3000 1", category: "JLH DCF Ambient Vehicle Patrol - Distances")]
	protected float m_fMinPlayerDistance = 220.0;

	[Attribute("1", UIWidgets.CheckBox, "Reject patrol spawns visible to live deployed players.", category: "JLH DCF Ambient Vehicle Patrol - Safety")]
	protected bool m_bRejectLineOfSightSpawns = true;

	[Attribute("650", UIWidgets.EditBox, "Reject patrol spawns when a live deployed player has line of sight within this distance.", "0 3000 1", category: "JLH DCF Ambient Vehicle Patrol - Safety")]
	protected float m_fLineOfSightRejectDistance = 650.0;

	[Attribute("1", UIWidgets.CheckBox, "Allow this area to spawn near US-owned bases, while still respecting base core safety radius.", category: "JLH DCF Ambient Vehicle Patrol - Safety")]
	protected bool m_bAllowNearUSBases = true;

	[Attribute("US", UIWidgets.EditBox, "Faction key treated as the protected friendly base owner for base safety checks.", category: "JLH DCF Ambient Vehicle Patrol - Safety")]
	protected string m_sProtectedBaseFactionKey = "US";

	[Attribute("0", UIWidgets.EditBox, "Optional hard safety radius around US HQ/MOB bases. Set to 0 to allow first-cap and nearby wilderness patrols.", "0 3000 1", category: "JLH DCF Ambient Vehicle Patrol - Safety")]
	protected float m_fUSHQHardSafetyRadius = 0.0;

	[Attribute("150", UIWidgets.EditBox, "Reject patrol spawns this close to protected-faction campaign base cores, normally US.", "0 3000 1", category: "JLH DCF Ambient Vehicle Patrol - Safety")]
	protected float m_fUSBaseCoreSafetyRadius = 150.0;

	[Attribute("0", UIWidgets.EditBox, "When Allow Near US Bases is off, reject patrol spawns within this distance of protected-faction bases. Set to 0 to use Ambient Threats config soft radius when available.", "0 3000 1", category: "JLH DCF Ambient Vehicle Patrol - Safety")]
	protected float m_fUSOwnedBaseSoftSafetyRadius = 0.0;

	[Attribute("220", UIWidgets.EditBox, "Block live patrol cleanup when a live deployed player is within this distance of the active vehicle or crew.", "0 3000 1", category: "JLH DCF Ambient Vehicle Patrol - Safety")]
	protected float m_fLiveCleanupPlayerSafeDistance = 220.0;

	[Attribute("650", UIWidgets.EditBox, "Block live patrol cleanup when a live deployed player has line of sight to the active vehicle or crew within this distance.", "0 3000 1", category: "JLH DCF Ambient Vehicle Patrol - Safety")]
	protected float m_fLiveCleanupLineOfSightRejectDistance = 650.0;

	[Attribute("1", UIWidgets.EditBox, "Maximum active vehicles owned by this area or marker.", "0 4 1", category: "JLH DCF Ambient Vehicle Patrol - Lifecycle")]
	protected int m_iMaxActiveVehicles = 1;

	[Attribute("420", UIWidgets.EditBox, "Seconds before this area or marker may try another patrol.", "0 7200 1", category: "JLH DCF Ambient Vehicle Patrol - Lifecycle")]
	protected int m_iCooldownSeconds = 420;

	[Attribute("1.0", UIWidgets.EditBox, "Chance from 0.0 to 1.0 that an eligible activation creates a patrol.", "0 1 0.01", category: "JLH DCF Ambient Vehicle Patrol - Lifecycle")]
	protected float m_fSpawnChance = 1.0;

	[Attribute("1", UIWidgets.CheckBox, "Use the nearest valid road or spline route when generating patrol points.", category: "JLH DCF Ambient Vehicle Patrol - Route")]
	protected bool m_bPreferRoadRoute = true;

	[Attribute("1", UIWidgets.CheckBox, "Add a cycle waypoint so the generated route loops.", category: "JLH DCF Ambient Vehicle Patrol - Route")]
	protected bool m_bLoopRoute = true;

	[Attribute("0", UIWidgets.ComboBox, "Debug logging detail: off, basic lifecycle, verbose route/selection, or deep mount validation.", "", ParamEnumArray.FromEnum(JLH_AmbientVehiclePatrolDebugLevel), category: "JLH DCF Ambient Vehicle Patrol - Debug")]
	protected JLH_AmbientVehiclePatrolDebugLevel m_eDebugLevel = JLH_AmbientVehiclePatrolDebugLevel.OFF;

	[Attribute("1", UIWidgets.CheckBox, "Area prefabs only. Multiplies radii by entity scale so the visible Workbench zone and runtime area stay aligned.", category: "JLH DCF Ambient Vehicle Patrol - Area Visual")]
	protected bool m_bScaleDistancesByEntityScale = true;

	protected bool m_bRegistered;
	protected bool m_bLastEnabledState;
	protected bool m_bEnabledStateInitialized;
	protected int m_iNextAllowedSpawnTick;
	protected string m_sAreaKey;
	protected ref array<ref JLH_AmbientVehiclePatrolRuntime> m_aRuntimes = {};
	protected ref array<string> m_aLogKeys = {};
	protected ref array<int> m_aLogTicks = {};

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		JLH_AmbientVehiclePatrolManager.RegisterArea(this);
	}

	void RegisterArea()
	{
		if (m_bRegistered)
			return;

		if (ShouldSuppressStandaloneMarkerRuntime())
			return;

		IEntity owner = GetOwner();
		if (!owner)
			return;

		NormalizeEditorFields();
		m_sAreaKey = BuildAreaKey(owner);
		m_iNextAllowedSpawnTick = 0;
		m_bRegistered = true;
		m_bLastEnabledState = m_bEnabled;
		m_bEnabledStateInitialized = true;
	}

	void Tick()
	{
		if (ShouldSuppressStandaloneMarkerRuntime())
		{
			CleanupAllRuntime("marker_claimed_by_area");
			return;
		}

		RegisterArea();
		if (!m_bRegistered)
			return;

		NormalizeEditorFields();
		LogEnabledStateChange();

		if (!m_bEnabled)
		{
			CleanupAllRuntime("disabled");
			return;
		}

		ref array<IEntity> livePlayers = {};
		JLH_AddonWorldQuery.GatherLiveDeployedPlayers(livePlayers, "");
		if (livePlayers.IsEmpty())
		{
			CleanupAllRuntime("no_live_players");
			return;
		}

		CleanupInvalidOrDistantRuntime();

		IEntity nearestPlayer;
		float nearestDistance;
		if (!FindNearestLivePlayerWithinActivation(nearestPlayer, nearestDistance))
			return;

		TryActivate(nearestPlayer, nearestDistance);
	}

	protected void TryActivate(IEntity player, float playerDistance)
	{
		if (!player)
			return;

		if (System.GetTickCount() < m_iNextAllowedSpawnTick)
			return;

		if (m_iMaxActiveVehicles <= 0 || CountActiveLocalRuntime() >= m_iMaxActiveVehicles)
			return;

		LogEvent("ambient_vehicle_patrol_area_active", "distance=" + playerDistance.ToString(), false);

		if (m_fSpawnChance < 1.0 && Math.RandomFloat(0.0, 1.0) > m_fSpawnChance)
		{
			MarkCooldown();
			return;
		}

		int budgetDelayMs;
		if (!JLH_DCF_RuntimeWorkBudget.TryReserveHeavySpawn("AmbientVehiclePatrol", budgetDelayMs))
		{
			DeferSpawnForRuntimeBudget(budgetDelayMs);
			return;
		}

		JLH_AmbientVehiclePatrolRuntime runtime;
		string reason;
		if (!SpawnPatrol(runtime, reason))
		{
			LogEvent("ambient_vehicle_patrol_area_active", "result=failed reason=" + reason, true);
			MarkCooldown();
			return;
		}

		m_aRuntimes.Insert(runtime);
		MarkCooldown();
	}

	protected bool SpawnPatrol(out JLH_AmbientVehiclePatrolRuntime runtime, out string reason)
	{
		runtime = null;
		reason = "";

		IEntity owner = GetOwner();
		if (!owner)
		{
			reason = "area_missing_owner";
			return false;
		}

		JLH_AmbientVehiclePatrolMarkerComponent marker;
		vector anchorPosition = owner.GetOrigin();
		string anchorSource = "area";
		if (SelectSpawnMarker(marker))
		{
			anchorPosition = marker.GetOwner().GetOrigin();
			anchorSource = "marker";
		}
		else if (IsMarkerRuntimeComponent())
		{
			marker = JLH_AmbientVehiclePatrolMarkerComponent.Cast(owner.FindComponent(JLH_AmbientVehiclePatrolMarkerComponent));
			anchorSource = "marker";
		}

		JLH_AmbientVehiclePatrolVehicleFilterSet filters = BuildVehicleFilterSet(marker);
		string factionKey = ResolveFactionKey();
		JLH_CORE_VehicleLabelSelection vehicleSelection;
		if (!TrySelectVehiclePrefab(factionKey, filters, vehicleSelection, reason))
			return false;

		LogEvent("ambient_vehicle_patrol_vehicle_selected", "faction=" + factionKey + " prefab=" + vehicleSelection.Prefab + " resolverSource=" + vehicleSelection.Source + " filterSource=" + filters.Source + " preferredMatches=" + vehicleSelection.PreferredMatches.ToString(), false);

		JLH_AmbientVehiclePatrolRoute route;
		if (!JLH_AmbientVehiclePatrolRouteGenerator.TryGenerate(anchorPosition, GetEffectivePatrolRadius(), m_bPreferRoadRoute, route, reason))
			return false;

		if (HasActiveVehicleNear(route.SpawnPosition, 30.0))
		{
			reason = "duplicate_spawn_stack_prevented";
			return false;
		}

		if (!IsSpawnPositionSafe(route.SpawnPosition, reason))
			return false;

		LogEvent("ambient_vehicle_patrol_route_generated", "points=" + route.Points.Count().ToString() + " source=" + route.Source + " anchor=" + anchorSource + " spawn=" + route.SpawnPosition.ToString(), false);
		LogRouteDetails(route);

		IEntity vehicle = SpawnVehicle(vehicleSelection.Prefab, route.SpawnPosition, route.Facing);
		if (!vehicle)
		{
			reason = "vehicle_spawn_failed";
			return false;
		}

		ResourceName crewPrefab = ResolveCrewGroupPrefab(factionKey);
		if (crewPrefab == "")
		{
			JLH_AddonSpawnUtility.DeleteRuntimeEntity(vehicle, "ambient_vehicle_patrol_vehicle_without_crew");
			reason = "crew_prefab_missing";
			return false;
		}

		SCR_AIGroup crewGroup = SpawnCrewGroup(crewPrefab, route.SpawnPosition, route.Facing);
		if (!crewGroup)
		{
			JLH_AddonSpawnUtility.DeleteRuntimeEntity(vehicle, "ambient_vehicle_patrol_vehicle_without_crew");
			reason = "crew_spawn_failed";
			return false;
		}

		runtime = new JLH_AmbientVehiclePatrolRuntime();
		runtime.Init(JLH_AmbientVehiclePatrolManager.NextPatrolId(), vehicle, crewGroup, vehicleSelection.Prefab, crewPrefab, route);
		runtime.Registry.TrackVehicle(vehicle, "ambient_vehicle_patrol_vehicle_spawn");
		runtime.Registry.TrackGroup(crewGroup, "ambient_vehicle_patrol_crew_spawn");

		LogEvent("ambient_vehicle_patrol_spawned", "id=" + runtime.Id.ToString() + " vehicle=" + JLH_DCF_NodeDebug.EntityLabel(vehicle) + " crewGroup=" + JLH_DCF_NodeDebug.EntityLabel(crewGroup), true);
		if (!QueuePendingMount(runtime))
		{
			CleanupRuntime(runtime, "mount_queue_missing");
			reason = "mount_queue_missing";
			return false;
		}

		return true;
	}

	protected bool QueuePendingMount(JLH_AmbientVehiclePatrolRuntime runtime)
	{
		if (!runtime || !GetGame() || !GetGame().GetCallqueue())
		{
			return false;
		}

		GetGame().GetCallqueue().CallLater(JLH_AmbientVehiclePatrol_FinalizePendingMount, PENDING_MOUNT_DELAY_MS, false, this, runtime.Id);
		return true;
	}

	void FinalizePendingMount(int runtimeId)
	{
		JLH_AmbientVehiclePatrolRuntime runtime = FindRuntime(runtimeId);
		if (!runtime || runtime.Started)
			return;

		if (!runtime.VehicleEntity || !runtime.CrewGroup)
		{
			CleanupRuntimeById(runtimeId, "pending_mount_missing_runtime");
			return;
		}

		if (CountGroupUnits(runtime.CrewGroup) <= 0)
		{
			if (RetryPendingMount(runtime, "crew_not_ready"))
				return;

			CleanupRuntimeById(runtimeId, "crew_not_ready");
			return;
		}

		RegisterUsableVehicle(runtime.CrewGroup, runtime.VehicleEntity);
		runtime.MountAttempts++;

		JLH_DCF_VehicleMountStats stats = new JLH_DCF_VehicleMountStats();
		bool mounted = MountCrew(runtime.CrewGroup, runtime.VehicleEntity, runtime.VehiclePrefab, stats);
		if (!mounted)
		{
			string failureReason = stats.FailureReason;
			if (failureReason == "")
				failureReason = "mount_failed";

			if ((failureReason == "replication_not_ready" || failureReason == "crew_entity_invalid") && RetryPendingMount(runtime, failureReason))
				return;

			CleanupRuntimeById(runtimeId, failureReason);
			return;
		}

		if (!AssignRouteToGroup(runtime.CrewGroup, runtime.Route, runtime.Registry, runtime.Id))
		{
			CleanupRuntimeById(runtimeId, "waypoint_assignment_failed");
			return;
		}

		runtime.DriverEntity = stats.DriverEntity;
		runtime.GunnerEntity = stats.GunnerEntity;
		runtime.Started = true;
		JLH_DCF_PersistenceExclusion.StopTrackingTree(runtime.CrewGroup, "ambient_vehicle_patrol_mount_finalized_group");
		JLH_DCF_PersistenceExclusion.StopTrackingTree(runtime.VehicleEntity, "ambient_vehicle_patrol_mount_finalized_vehicle");
		JLH_DCF_VehicleSustainment.RegisterVehicle(SYSTEM_NAME, runtime.VehicleEntity, runtime.CrewGroup, ResolveFactionKey(), JLH_DCF_NodeDebug.EntityLabel(GetOwner()), true);

		LogEvent("ambient_vehicle_patrol_mount_complete", "id=" + runtime.Id.ToString() + " crewSeen=" + stats.CrewSeen.ToString() + " driver=" + BoolText(stats.DriverMounted) + " gunner=" + BoolText(stats.GunnerMounted) + " deletedUnused=" + stats.RemovedUnassigned.ToString(), true);
		LogEvent("ambient_vehicle_patrol_started", "id=" + runtime.Id.ToString() + " waypoints=" + runtime.Route.Points.Count().ToString() + " loop=" + BoolText(m_bLoopRoute), true);
	}

	protected bool RetryPendingMount(JLH_AmbientVehiclePatrolRuntime runtime, string reason)
	{
		if (!runtime || runtime.MountAttempts >= MAX_MOUNT_ATTEMPTS || !GetGame() || !GetGame().GetCallqueue())
			return false;

		runtime.MountAttempts++;
		GetGame().GetCallqueue().CallLater(JLH_AmbientVehiclePatrol_FinalizePendingMount, MOUNT_RETRY_DELAY_MS, false, this, runtime.Id);
		return true;
	}

	protected bool TrySelectVehiclePrefab(string factionKey, JLH_AmbientVehiclePatrolVehicleFilterSet filters, out JLH_CORE_VehicleLabelSelection selection, out string reason)
	{
		selection = null;
		reason = "";

		array<string> fallbackLabels = {};
		if (!filters)
			filters = BuildVehicleFilterSet(null);

		if (JLH_CORE_CompatibilityResolver.TrySelectVehiclePrefabByLabelFiltersAllowAllVehicleKinds(factionKey, filters.RequiredLabels, filters.PreferredLabels, filters.ExcludedLabels, fallbackLabels, selection, reason))
		{
			LogVerbose("ambient_vehicle_patrol_vehicle_selected", "faction=" + factionKey + " filterSource=" + filters.Source + " required=" + FormatLabelList(filters.RequiredLabels) + " preferred=" + FormatLabelList(filters.PreferredLabels) + " excluded=" + FormatLabelList(filters.ExcludedLabels));
			return true;
		}

		reason = "vehicle_selection_" + reason;
		return false;
	}

	protected JLH_AmbientVehiclePatrolVehicleFilterSet BuildVehicleFilterSet(JLH_AmbientVehiclePatrolMarkerComponent marker)
	{
		JLH_AmbientVehiclePatrolVehicleFilterSet filters = new JLH_AmbientVehiclePatrolVehicleFilterSet();
		filters.Source = "area";
		if (marker && marker.HasVehicleFilterOverride())
		{
			marker.CopyVehicleLabelFilters(filters.RequiredLabels, filters.PreferredLabels, filters.ExcludedLabels);
			filters.Source = "marker";
			return filters;
		}

		CopyEditableVehicleLabelsToNames(m_aRequiredVehicleLabels, filters.RequiredLabels);
		CopyEditableVehicleLabelsToNames(m_aPreferredVehicleLabels, filters.PreferredLabels);
		CopyEditableVehicleLabelsToNames(m_aExcludedVehicleLabels, filters.ExcludedLabels);
		return filters;
	}

	protected void CopyEditableVehicleLabelsToNames(array<EEditableEntityLabel> sourceLabels, notnull array<string> targetLabels)
	{
		if (!sourceLabels)
			return;

		foreach (EEditableEntityLabel label : sourceLabels)
		{
			string labelName = SCR_Enum.GetEnumName(EEditableEntityLabel, label);
			if (labelName == "" || labelName == "NONE" || targetLabels.Contains(labelName))
				continue;

			targetLabels.Insert(labelName);
		}
	}

	protected ResourceName ResolveCrewGroupPrefab(string factionKey)
	{
		if (IsUSSRLikeFaction(factionKey))
		{
			ResourceName resolverCrewPrefab;
			if (JLH_DCF_DEVEnemyAssetResolver.TryResolveEnemyCrewGroupPrefabByRole(JLH_EnemyAssetRole.VEHICLE_CREW, resolverCrewPrefab) && resolverCrewPrefab != "")
				return resolverCrewPrefab;
		}

		return ResolveKnownDedicatedVehicleCrewGroupPrefab(factionKey);
	}

	protected ResourceName ResolveKnownDedicatedVehicleCrewGroupPrefab(string factionKey)
	{
		string normalizedFaction = factionKey;
		normalizedFaction.ToUpper();

		if (normalizedFaction == "USSR" || normalizedFaction == "OPFOR" || normalizedFaction == "FIA")
			return DEFAULT_USSR_VEHICLE_CREW_GROUP_PREFAB;

		if (normalizedFaction == "US" || normalizedFaction == "BLUFOR")
			return DEFAULT_US_VEHICLE_CREW_GROUP_PREFAB;

		return DEFAULT_USSR_VEHICLE_CREW_GROUP_PREFAB;
	}

	protected IEntity SpawnVehicle(ResourceName vehiclePrefab, vector position, vector facing)
	{
		IEntity vehicle = JLH_AddonSpawnUtility.SpawnEntity(vehiclePrefab, position, "ambient_vehicle_patrol_vehicle_spawn");
		if (!vehicle)
			return null;

		OrientEntity(vehicle, position, facing);
		return vehicle;
	}

	protected SCR_AIGroup SpawnCrewGroup(ResourceName crewPrefab, vector vehiclePosition, vector facing)
	{
		vector right = Vector(facing[2], 0.0, -facing[0]);
		vector crewPosition = JLH_AddonWorldQuery.GroundPosition(vehiclePosition + (right * 3.0));
		SCR_AIGroup group = JLH_AddonSpawnUtility.SpawnGroupDormant(crewPrefab, crewPosition, "ambient_vehicle_patrol_crew_spawn");
		if (!group)
			return null;

		//group.DeactivateAllMembers();
		return group;
	}

	protected void OrientEntity(IEntity entity, vector position, vector facing)
	{
		if (!entity)
			return;

		vector forward = NormalizeHorizontalDirection(facing);
		vector right = Vector(forward[2], 0.0, -forward[0]);
		vector up = Vector(0.0, 1.0, 0.0);

		vector transform[4];
		transform[0] = right;
		transform[1] = up;
		transform[2] = forward;
		transform[3] = JLH_AddonWorldQuery.GroundPosition(position);
		entity.SetTransform(transform);
	}

	protected bool MountCrew(SCR_AIGroup group, IEntity vehicle, ResourceName vehiclePrefab, notnull JLH_DCF_VehicleMountStats stats)
	{
		stats.Reset();
		if (!group || !vehicle)
		{
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
		bool likelyArmed = true;
		int maxMountedCrew = MAX_MOUNTED_CREW_FOR_ARMED_PATROL;
		JLH_DCF_VehicleCrewSpawnModeHandler crewSpawnHandler = JLH_DCF_VehicleCrewSpawnModeRouter.Create(JLH_DCF_VehicleCrewSpawnMode.DRIVER_GUNNER_AND_PASSENGERS);
		int maxCargoSeats = crewSpawnHandler.GetMaxCargoSeats(likelyArmed, maxMountedCrew);

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
				else if (crewSpawnHandler.CanMountGunnerSeat(likelyArmed, stats, maxMountedCrew) && access.MoveInVehicle(vehicle, ECompartmentType.TURRET))
				{
					if (!stats.GunnerMounted)
					{
						stats.GunnerMounted = true;
						stats.GunnerEntity = unit;
					}

					stats.TurretSeatsFilled++;
					stats.SeatsFilled++;
					moved = true;
				}
				else if (crewSpawnHandler.CanMountCargoSeat(likelyArmed, stats, maxCargoSeats, maxMountedCrew) && access.MoveInVehicle(vehicle, ECompartmentType.CARGO))
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

			JLH_AddonSpawnUtility.DeleteRuntimeEntity(unit, "ambient_vehicle_patrol_unused_crew");
			stats.RemovedUnassigned++;
		}

		stats.RemovedUnassigned += PruneMountedCrewOverLimit(group, vehicle, maxMountedCrew);
		SanitizeMountStatsPassengerRoles(stats);

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

		if (ShouldLogVerbose())
			LogEvent("ambient_vehicle_patrol_mount_complete", "validation crewSeen=" + stats.CrewSeen.ToString() + " seats=" + stats.SeatsFilled.ToString() + " driver=" + BoolText(stats.DriverMounted) + " gunner=" + BoolText(stats.GunnerMounted) + " failure=" + stats.FailureReason, true);

		if (gunnerRequired && !stats.GunnerMounted)
			return false;

		return stats.DriverMounted && stats.SeatsFilled > 0;
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

			JLH_AddonSpawnUtility.DeleteRuntimeEntity(unit, "ambient_vehicle_patrol_pruned_extra_mounted_crew");
			removed++;
		}

		return removed;
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

	protected bool AssignRouteToGroup(SCR_AIGroup group, JLH_AmbientVehiclePatrolRoute route, JLH_AddonRuntimeEntityRegistry registry, int runtimeId)
	{
		if (!group || !route || !registry || route.Points.Count() < 2)
			return false;

		JLH_DCF_NodeWaypointResolver.ClearGroupWaypoints(group);
		array<AIWaypoint> routeWaypoints = {};
		for (int i = 0; i < route.Points.Count(); i++)
		{
			AIWaypoint waypoint = JLH_AddonSpawnUtility.SpawnWaypoint(JLH_AddonSpawnUtility.WAYPOINT_MOVE, route.Points[i], "ambient_vehicle_patrol_route_move");
			if (!waypoint)
				continue;

			routeWaypoints.Insert(waypoint);
			registry.TrackWaypoint(waypoint, "ambient_vehicle_patrol_route_move");
			group.AddWaypoint(waypoint);
		}

		if (routeWaypoints.Count() < 2)
			return false;

		if (m_bLoopRoute)
		{
			AIWaypoint cycle = JLH_AddonSpawnUtility.SpawnWaypoint(JLH_AddonSpawnUtility.WAYPOINT_CYCLE, routeWaypoints[0].GetOrigin(), "ambient_vehicle_patrol_route_cycle");
			if (cycle)
			{
				AIWaypointCycle cycleWaypoint = AIWaypointCycle.Cast(cycle);
				if (!cycleWaypoint)
				{
					SCR_EntityHelper.DeleteEntityAndChildren(cycle);
				}
				else
				{
					cycleWaypoint.SetRerunCounter(-1);
					cycleWaypoint.SetWaypoints(routeWaypoints);
					registry.TrackWaypoint(cycleWaypoint, "ambient_vehicle_patrol_route_cycle");
					group.AddWaypoint(cycleWaypoint);
				}
			}
		}

		JLH_AddonSpawnUtility.ActivateGroup(group);
		LogVerbose("ambient_vehicle_patrol_started", "id=" + runtimeId.ToString() + " routeAssignedPoints=" + routeWaypoints.Count().ToString() + " loop=" + BoolText(m_bLoopRoute));
		return true;
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

	protected void LogRouteDetails(JLH_AmbientVehiclePatrolRoute route)
	{
		if (!ShouldLogVerbose() || !route)
			return;

		string details = "source=" + route.Source;
		details += " pointCount=" + route.Points.Count().ToString();
		for (int i = 0; i < route.Points.Count() && i < 5; i++)
			details += " p" + i.ToString() + "=" + route.Points[i].ToString();

		LogEvent("ambient_vehicle_patrol_route_generated", details, false);
	}

	protected bool SelectSpawnMarker(out JLH_AmbientVehiclePatrolMarkerComponent selectedMarker)
	{
		selectedMarker = null;

		array<JLH_AmbientVehiclePatrolMarkerComponent> markers = {};
		CollectAreaMarkers(markers);
		if (markers.IsEmpty())
			return false;

		selectedMarker = markers[Math.RandomInt(0, markers.Count())];
		return selectedMarker != null && selectedMarker.GetOwner() != null;
	}

	protected void CollectAreaMarkers(notnull array<JLH_AmbientVehiclePatrolMarkerComponent> markers)
	{
		markers.Clear();

		IEntity owner = GetOwner();
		if (!owner || IsMarkerRuntimeComponent())
			return;

		float areaRadius = GetAreaMarkerSearchRadius();
		CollectChildMarkers(owner, markers, areaRadius);
		CollectPlacedMarkersInArea(owner, markers, areaRadius);
	}

	protected void CollectChildMarkers(IEntity parent, notnull array<JLH_AmbientVehiclePatrolMarkerComponent> markers, float areaRadius)
	{
		if (!parent)
			return;

		IEntity child = parent.GetChildren();
		while (child)
		{
			IEntity next = child.GetSibling();
			JLH_AmbientVehiclePatrolMarkerComponent marker = JLH_AmbientVehiclePatrolMarkerComponent.Cast(child.FindComponent(JLH_AmbientVehiclePatrolMarkerComponent));
			if (marker && marker.IsEnabled() && IsMarkerInsideArea(marker.GetOwner(), areaRadius))
				AddUniqueAreaMarker(markers, marker);

			CollectChildMarkers(child, markers, areaRadius);
			child = next;
		}
	}

	protected void CollectPlacedMarkersInArea(IEntity owner, notnull array<JLH_AmbientVehiclePatrolMarkerComponent> markers, float areaRadius)
	{
		if (!owner || areaRadius <= 0.0 || !GetGame() || !GetGame().GetWorld())
			return;

		s_aAreaMarkerQueryResults.Clear();
		s_AreaMarkerQueryOwner = owner;
		s_vAreaMarkerQueryCenter = owner.GetOrigin();
		s_fAreaMarkerQueryRadius = areaRadius;
		GetGame().GetWorld().QueryEntitiesBySphere(s_vAreaMarkerQueryCenter, s_fAreaMarkerQueryRadius, QueryAreaMarkerEntity, null, EQueryEntitiesFlags.ALL);

		foreach (JLH_AmbientVehiclePatrolMarkerComponent marker : s_aAreaMarkerQueryResults)
		{
			if (marker)
				AddUniqueAreaMarker(markers, marker);
		}

		s_aAreaMarkerQueryResults.Clear();
		s_AreaMarkerQueryOwner = null;
		s_vAreaMarkerQueryCenter = "0 0 0";
		s_fAreaMarkerQueryRadius = 0.0;
	}

	protected static bool QueryAreaMarkerEntity(IEntity entity)
	{
		if (!entity || entity == s_AreaMarkerQueryOwner)
			return true;

		JLH_AmbientVehiclePatrolMarkerComponent marker = JLH_AmbientVehiclePatrolMarkerComponent.Cast(entity.FindComponent(JLH_AmbientVehiclePatrolMarkerComponent));
		if (!marker || !marker.IsEnabled())
			return true;

		if (IsMarkerOwnedByAnotherArea(entity))
			return true;

		if (vector.Distance(entity.GetOrigin(), s_vAreaMarkerQueryCenter) > s_fAreaMarkerQueryRadius)
			return true;

		if (!s_aAreaMarkerQueryResults.Contains(marker))
			s_aAreaMarkerQueryResults.Insert(marker);

		return true;
	}

	protected static bool IsMarkerOwnedByAnotherArea(IEntity markerEntity)
	{
		IEntity parent = markerEntity.GetParent();
		while (parent)
		{
			if (parent == s_AreaMarkerQueryOwner)
				return false;

			if (parent.FindComponent(JLH_AmbientVehiclePatrolAreaComponent))
				return true;

			parent = parent.GetParent();
		}

		return false;
	}

	protected bool IsMarkerInsideArea(IEntity markerEntity, float areaRadius)
	{
		IEntity owner = GetOwner();
		if (!owner || !markerEntity || areaRadius <= 0.0)
			return false;

		return vector.Distance(owner.GetOrigin(), markerEntity.GetOrigin()) <= areaRadius;
	}

	protected void AddUniqueAreaMarker(notnull array<JLH_AmbientVehiclePatrolMarkerComponent> markers, JLH_AmbientVehiclePatrolMarkerComponent marker)
	{
		if (marker && !markers.Contains(marker))
			markers.Insert(marker);
	}

	protected bool IsSpawnPositionSafe(vector spawnPosition, out string reason)
	{
		reason = "";

		if (JLH_AddonWorldQuery.GetSlopeDegrees(spawnPosition) > 18.0)
		{
			reason = "spawn_steep_terrain";
			return false;
		}

		if (m_fMinPlayerDistance > 0.0 && JLH_AddonWorldQuery.HasLiveDeployedPlayerWithin(spawnPosition, m_fMinPlayerDistance, ""))
		{
			reason = "player_too_close";
			return false;
		}

		if (m_bRejectLineOfSightSpawns && m_fLineOfSightRejectDistance > 0.0 && JLH_AddonWorldQuery.HasLineOfSightFromAnyLivePlayer(spawnPosition, m_fLineOfSightRejectDistance, ""))
		{
			reason = "visible_to_player";
			return false;
		}

		string protectedBaseFactionKey = GetProtectedBaseFactionKey();
		if (m_fUSHQHardSafetyRadius > 0.0 && JLH_AmbientThreatSpawnSelector.IsInsideUSHQSafeZone(spawnPosition, m_fUSHQHardSafetyRadius, protectedBaseFactionKey))
		{
			reason = "hq_safezone";
			return false;
		}

		if (m_fUSBaseCoreSafetyRadius > 0.0 && JLH_AddonWorldQuery.IsNearFactionBase(spawnPosition, m_fUSBaseCoreSafetyRadius, protectedBaseFactionKey))
		{
			reason = "protected_base_core";
			return false;
		}

		float usOwnedBaseSoftSafetyRadius = GetUSOwnedBaseSoftSafetyRadius();
		if (!m_bAllowNearUSBases && usOwnedBaseSoftSafetyRadius > 0.0 && JLH_AddonWorldQuery.IsNearFactionBase(spawnPosition, usOwnedBaseSoftSafetyRadius, protectedBaseFactionKey))
		{
			reason = "us_owned_base_soft_zone";
			return false;
		}

		return true;
	}

	protected void CleanupInvalidOrDistantRuntime()
	{
		for (int i = m_aRuntimes.Count() - 1; i >= 0; i--)
		{
			JLH_AmbientVehiclePatrolRuntime runtime = m_aRuntimes[i];
			if (!runtime)
			{
				m_aRuntimes.Remove(i);
				continue;
			}

			if (!runtime.VehicleEntity || IsEntityDestroyed(runtime.VehicleEntity))
			{
				MarkRuntimeCleared(runtime, "vehicle_destroyed");
				string vehicleDestroyedBlockReason;
				if (!CanCleanupAmbientVehicleRuntime(runtime, true, vehicleDestroyedBlockReason))
					continue;

				CleanupRuntime(runtime, "vehicle_destroyed");
				m_aRuntimes.Remove(i);
				continue;
			}

			if (!runtime.CrewGroup || CountLivingGroupUnits(runtime.CrewGroup) <= 0)
			{
				MarkRuntimeCleared(runtime, "crew_destroyed");
				string crewDestroyedBlockReason;
				if (!CanCleanupAmbientVehicleRuntime(runtime, true, crewDestroyedBlockReason))
					continue;

				CleanupRuntime(runtime, "crew_destroyed");
				m_aRuntimes.Remove(i);
				continue;
			}

			vector cleanupPosition;
			string cleanupPositionSource;
			if (!TryResolveRuntimeCleanupPosition(runtime, cleanupPosition, cleanupPositionSource))
				continue;

			LogLiveCleanupPositionSource(runtime, cleanupPositionSource, cleanupPosition);

			float nearest = JLH_AddonWorldQuery.GetNearestLivePlayerDistance(cleanupPosition, "");
			if (nearest > GetEffectiveDespawnRadius())
			{
				string farBlockReason;
				if (!CanCleanupAmbientVehicleRuntime(runtime, false, farBlockReason))
					continue;

				CleanupRuntime(runtime, "players_out_of_range");
				m_aRuntimes.Remove(i);
			}
		}
	}

	protected void MarkRuntimeCleared(notnull JLH_AmbientVehiclePatrolRuntime runtime, string reason)
	{
		if (runtime.m_bCleared)
			return;

		runtime.m_bCleared = true;
		runtime.m_iClearedTick = System.GetTickCount();
		JLH_CleanupSafetyService.LogPackageEnteredClearedState(BuildCleanupPackageLabel(runtime, reason), ShouldLogBasic());
	}

	protected bool CanCleanupAmbientVehicleRuntime(notnull JLH_AmbientVehiclePatrolRuntime runtime, bool requireLinger, out string blockedReason)
	{
		vector cleanupPosition;
		string cleanupPositionSource;
		if (!TryResolveRuntimeCleanupPosition(runtime, cleanupPosition, cleanupPositionSource))
		{
			IEntity owner = GetOwner();
			if (owner)
				cleanupPosition = owner.GetOrigin();
			else
				cleanupPosition = "0 0 0";
		}

		JLH_CleanupSafetyQuery query = new JLH_CleanupSafetyQuery();
		query.Init(BuildCleanupPackageLabel(runtime, ""), cleanupPosition);
		query.RequireLinger = requireLinger;
		query.ClearedTick = runtime.m_iClearedTick;
		query.LingerSeconds = JLH_CleanupSafetyService.DEFAULT_PACKAGE_LINGER_SECONDS;
		query.CleanupDistance = Math.Max(m_fLiveCleanupPlayerSafeDistance, JLH_CleanupSafetyService.MIN_CLEANUP_DISTANCE_METERS);
		query.VisibilityDistance = Math.Max(m_fLiveCleanupLineOfSightRejectDistance, JLH_CleanupSafetyService.DEFAULT_VISIBILITY_DISTANCE_METERS);
		query.PlayerFactionFilter = "";
		query.DebugEnabled = ShouldLogBasic();
		JLH_CleanupSafetyService.PopulateFromRuntimeRegistry(query, runtime.Registry);
		if (runtime.CrewGroup && !query.Groups.Contains(runtime.CrewGroup))
			query.Groups.Insert(runtime.CrewGroup);
		if (runtime.VehicleEntity && !query.Vehicles.Contains(runtime.VehicleEntity))
			query.Vehicles.Insert(runtime.VehicleEntity);

		return JLH_CleanupSafetyService.CanCleanupPackage(query, blockedReason);
	}

	protected string BuildCleanupPackageLabel(notnull JLH_AmbientVehiclePatrolRuntime runtime, string reason)
	{
		string label = "AmbientVehiclePatrol id=" + runtime.Id.ToString();
		if (reason != "")
			label += " reason=" + reason;

		return label;
	}

	protected bool TryResolveRuntimeCleanupPosition(JLH_AmbientVehiclePatrolRuntime runtime, out vector cleanupPosition, out string source)
	{
		cleanupPosition = "0 0 0";
		source = "";

		if (!runtime)
			return false;

		if (runtime.VehicleEntity && !IsEntityDestroyed(runtime.VehicleEntity))
		{
			cleanupPosition = runtime.VehicleEntity.GetOrigin();
			source = "vehicle";
			return true;
		}

		if (TryGetGroupRuntimePosition(runtime.CrewGroup, cleanupPosition))
		{
			source = "group";
			return true;
		}

		IEntity owner = GetOwner();
		if (owner)
		{
			cleanupPosition = owner.GetOrigin();
			source = "area";
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
			if (!unit || !IsUnitAlive(unit) || IsEntityDestroyed(unit))
				continue;

			position = unit.GetOrigin();
			return true;
		}

		position = group.GetOrigin();
		return true;
	}

	protected bool IsLiveCleanupBlockedByPlayerSafety(JLH_AmbientVehiclePatrolRuntime runtime, vector cleanupPosition)
	{
		if (m_fLiveCleanupPlayerSafeDistance > 0.0 && JLH_AddonWorldQuery.HasLiveDeployedPlayerWithin(cleanupPosition, m_fLiveCleanupPlayerSafeDistance, ""))
		{
			float nearestDistance = JLH_AddonWorldQuery.GetNearestLivePlayerDistance(cleanupPosition, "");
			LogEvent("ambient_vehicle_patrol_cleanup_blocked", "id=" + runtime.Id.ToString() + " reason=player_near distance=" + nearestDistance.ToString() + " limit=" + m_fLiveCleanupPlayerSafeDistance.ToString() + " cleanupPos=" + cleanupPosition.ToString(), false);
			return true;
		}

		if (m_fLiveCleanupLineOfSightRejectDistance > 0.0 && JLH_AddonWorldQuery.HasLineOfSightFromAnyLivePlayer(cleanupPosition, m_fLiveCleanupLineOfSightRejectDistance, ""))
		{
			LogEvent("ambient_vehicle_patrol_cleanup_blocked", "id=" + runtime.Id.ToString() + " reason=player_los range=" + m_fLiveCleanupLineOfSightRejectDistance.ToString() + " cleanupPos=" + cleanupPosition.ToString(), false);
			return true;
		}

		return false;
	}

	protected void LogLiveCleanupPositionSource(JLH_AmbientVehiclePatrolRuntime runtime, string source, vector cleanupPosition)
	{
		if (!ShouldLogVerbose() || !runtime)
			return;

		LogEvent("ambient_vehicle_patrol_cleanup_position", "id=" + runtime.Id.ToString() + " source=" + source + " cleanupPos=" + cleanupPosition.ToString(), false);
	}

	void CleanupAllRuntime(string reason)
	{
		for (int i = m_aRuntimes.Count() - 1; i >= 0; i--)
		{
			CleanupRuntime(m_aRuntimes[i], reason);
			m_aRuntimes.Remove(i);
		}
	}

	protected void CleanupRuntimeById(int runtimeId, string reason)
	{
		for (int i = m_aRuntimes.Count() - 1; i >= 0; i--)
		{
			JLH_AmbientVehiclePatrolRuntime runtime = m_aRuntimes[i];
			if (!runtime || runtime.Id != runtimeId)
				continue;

			CleanupRuntime(runtime, reason);
			m_aRuntimes.Remove(i);
			return;
		}
	}

	protected void CleanupRuntime(JLH_AmbientVehiclePatrolRuntime runtime, string reason)
	{
		if (!runtime)
			return;

		if (runtime.VehicleEntity)
			JLH_DCF_VehicleSustainment.UnregisterVehicle(runtime.VehicleEntity, reason);

		if (runtime.CrewGroup)
			JLH_DCF_NodeWaypointResolver.ClearGroupWaypoints(runtime.CrewGroup);

		if (runtime.Registry)
			runtime.Registry.Cleanup(LOG_PREFIX, reason, false);

		LogEvent("ambient_vehicle_patrol_cleanup", "id=" + runtime.Id.ToString() + " reason=" + reason, true);
	}

	int CountActiveLocalRuntime()
	{
		int count = 0;
		foreach (JLH_AmbientVehiclePatrolRuntime runtime : m_aRuntimes)
		{
			if (runtime && runtime.VehicleEntity && !IsEntityDestroyed(runtime.VehicleEntity))
				count++;
		}

		return count;
	}

	protected bool HasActiveVehicleNear(vector position, float radius)
	{
		foreach (JLH_AmbientVehiclePatrolRuntime runtime : m_aRuntimes)
		{
			if (!runtime || !runtime.VehicleEntity || IsEntityDestroyed(runtime.VehicleEntity))
				continue;

			if (vector.Distance(position, runtime.VehicleEntity.GetOrigin()) <= radius)
				return true;
		}

		return false;
	}

	protected JLH_AmbientVehiclePatrolRuntime FindRuntime(int runtimeId)
	{
		foreach (JLH_AmbientVehiclePatrolRuntime runtime : m_aRuntimes)
		{
			if (runtime && runtime.Id == runtimeId)
				return runtime;
		}

		return null;
	}

	protected bool FindNearestLivePlayerWithinActivation(out IEntity nearestPlayer, out float nearestDistance)
	{
		nearestPlayer = null;
		nearestDistance = 99999999.0;

		IEntity owner = GetOwner();
		if (!owner)
			return false;

		ref array<IEntity> players = {};
		JLH_AddonWorldQuery.GatherLiveDeployedPlayers(players, "");
		foreach (IEntity player : players)
		{
			if (!player)
				continue;

			float distance = vector.Distance(owner.GetOrigin(), player.GetOrigin());
			if (distance > GetEffectiveActivationRadius() || distance >= nearestDistance)
				continue;

			nearestPlayer = player;
			nearestDistance = distance;
		}

		return nearestPlayer != null;
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
			if (agent && agent.GetControlledEntity())
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
			if (agent && IsUnitAlive(agent.GetControlledEntity()))
				count++;
		}

		return count;
	}

	protected bool IsUnitAlive(IEntity entity)
	{
		if (!entity)
			return false;

		ChimeraCharacter character = ChimeraCharacter.Cast(entity);
		if (character && character.GetCharacterController())
			return !character.GetCharacterController().IsDead();

		return !IsEntityDestroyed(entity);
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

	protected bool IsUnitInVehicle(IEntity unit, IEntity vehicle)
	{
		return JLH_DCF_VehiclePatrolSeatResolver.IsUnitInVehicle(unit, vehicle);
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
		return JLH_DCF_VehiclePatrolSeatResolver.IsGunnerRole(unit, vehicle);
	}

	protected string ResolveFactionKey()
	{
		string factionKey = m_sFactionKey;
		if (factionKey == "")
			factionKey = "USSR";

		factionKey.ToUpper();
		return factionKey;
	}

	protected bool IsUSSRLikeFaction(string factionKey)
	{
		factionKey.ToUpper();
		return factionKey == "USSR" || factionKey == "OPFOR" || factionKey == "FIA";
	}

	bool IsEnabled()
	{
		return m_bEnabled;
	}

	bool IsMarkerRuntimeComponent()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return false;

		return owner.FindComponent(JLH_AmbientVehiclePatrolMarkerComponent) != null;
	}

	bool ContainsWorldPosition(vector position)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return false;

		return vector.Distance(owner.GetOrigin(), position) <= GetAreaMarkerSearchRadius();
	}

	protected bool ShouldSuppressStandaloneMarkerRuntime()
	{
		IEntity owner = GetOwner();
		if (!owner || !owner.FindComponent(JLH_AmbientVehiclePatrolMarkerComponent))
			return false;

		if (HasParentAmbientVehiclePatrolArea(owner))
			return true;

		return JLH_AmbientVehiclePatrolManager.HasContainingAreaForMarker(owner, this);
	}

	protected bool HasParentAmbientVehiclePatrolArea(IEntity markerOwner)
	{
		if (!markerOwner)
			return false;

		IEntity parent = markerOwner.GetParent();
		while (parent)
		{
			if (parent.FindComponent(JLH_AmbientVehiclePatrolAreaComponent))
				return true;

			parent = parent.GetParent();
		}

		return false;
	}

	protected void NormalizeEditorFields()
	{
		if (m_sFactionKey == "")
			m_sFactionKey = "USSR";

		if (m_sProtectedBaseFactionKey == "")
			m_sProtectedBaseFactionKey = "US";

		m_sProtectedBaseFactionKey.ToUpper();

		m_fPatrolRadius = Math.Clamp(m_fPatrolRadius, 80.0, 1200.0);
		m_fActivationRadius = Math.Clamp(m_fActivationRadius, 0.0, 5000.0);
		m_fDespawnRadius = Math.Clamp(m_fDespawnRadius, 250.0, 8000.0);
		m_fMinPlayerDistance = Math.Clamp(m_fMinPlayerDistance, 0.0, 3000.0);
		m_fLineOfSightRejectDistance = Math.Clamp(m_fLineOfSightRejectDistance, 0.0, 3000.0);
		m_fUSHQHardSafetyRadius = Math.Clamp(m_fUSHQHardSafetyRadius, 0.0, 3000.0);
		m_fUSBaseCoreSafetyRadius = Math.Clamp(m_fUSBaseCoreSafetyRadius, 0.0, 3000.0);
		m_fUSOwnedBaseSoftSafetyRadius = Math.Clamp(m_fUSOwnedBaseSoftSafetyRadius, 0.0, 3000.0);
		m_fLiveCleanupPlayerSafeDistance = Math.Clamp(m_fLiveCleanupPlayerSafeDistance, 0.0, 3000.0);
		m_fLiveCleanupLineOfSightRejectDistance = Math.Clamp(m_fLiveCleanupLineOfSightRejectDistance, 0.0, 3000.0);
		m_iMaxActiveVehicles = Math.Clamp(m_iMaxActiveVehicles, 0, 4);
		m_iCooldownSeconds = Math.Clamp(m_iCooldownSeconds, 0, 7200);
		m_fSpawnChance = Math.Clamp(m_fSpawnChance, 0.0, 1.0);
	}

	float GetEditorVisualizerRadius()
	{
		return Math.Clamp(m_fActivationRadius, 1.0, 5000.0);
	}

	protected float GetEffectivePatrolRadius()
	{
		return Math.Clamp(m_fPatrolRadius, 80.0, 1200.0) * GetRuntimeAreaScale();
	}

	protected float GetEffectiveActivationRadius()
	{
		return Math.Clamp(m_fActivationRadius, 0.0, 5000.0) * GetRuntimeAreaScale();
	}

	protected float GetEffectiveDespawnRadius()
	{
		return Math.Clamp(m_fDespawnRadius, 250.0, 8000.0) * GetRuntimeAreaScale();
	}

	protected string GetProtectedBaseFactionKey()
	{
		if (m_sProtectedBaseFactionKey != "")
			return m_sProtectedBaseFactionKey;

		return "US";
	}

	protected float GetUSOwnedBaseSoftSafetyRadius()
	{
		if (m_fUSOwnedBaseSoftSafetyRadius > 0.0)
			return m_fUSOwnedBaseSoftSafetyRadius;

		JLH_AmbientThreatsConfigData config = JLH_AmbientThreatsConfig.Get();
		if (config && config.AmbientThreatsUSOwnedBaseSoftSafetyRadius > 0.0)
			return config.AmbientThreatsUSOwnedBaseSoftSafetyRadius;

		return 0.0;
	}

	protected float GetAreaMarkerSearchRadius()
	{
		float radius = GetEffectiveActivationRadius();
		if (radius <= 0.0)
			radius = GetEffectivePatrolRadius();

		return radius;
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

	protected void MarkCooldown()
	{
		m_iNextAllowedSpawnTick = System.GetTickCount() + (m_iCooldownSeconds * 1000);
	}

	protected void DeferSpawnForRuntimeBudget(int delayMs)
	{
		if (delayMs < 250)
			delayMs = 250;

		m_iNextAllowedSpawnTick = System.GetTickCount() + delayMs;
		LogEvent("ambient_vehicle_patrol_spawn_deferred_budget", "delayMs=" + delayMs.ToString(), false);
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
	}

	protected bool ShouldLogBasic()
	{
		return m_eDebugLevel == JLH_AmbientVehiclePatrolDebugLevel.BASIC || ShouldLogVerbose();
	}

	protected bool ShouldLogVerbose()
	{
		return m_eDebugLevel == JLH_AmbientVehiclePatrolDebugLevel.VERBOSE || ShouldLogDeep();
	}

	protected bool ShouldLogDeep()
	{
		return m_eDebugLevel == JLH_AmbientVehiclePatrolDebugLevel.DEEP;
	}

	protected void LogVerbose(string eventName, string details)
	{
		if (!ShouldLogVerbose())
			return;

		LogEvent(eventName, details, false);
	}

	void LogEvent(string eventName, string details, bool force)
	{
		if (!force && !ShouldLogBasic())
			return;

		string key = eventName + ":" + m_sAreaKey;
		if (details != "")
			key += ":" + details;

		if (!force && !ShouldLogKey(key))
			return;

		string areaLabel = "NONE";
		string position = "0 0 0";
		IEntity owner = GetOwner();
		if (owner)
		{
			areaLabel = JLH_DCF_NodeDebug.EntityLabel(owner);
			position = owner.GetOrigin().ToString();
		}

		string line = eventName;
		line += " area=" + areaLabel;
		line += " key=" + m_sAreaKey;
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

	protected string BuildAreaKey(IEntity owner)
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

	protected string FormatLabelList(array<string> labels)
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

	protected string BoolText(bool value)
	{
		if (value)
			return "true";

		return "false";
	}

	protected vector NormalizeHorizontalDirection(vector direction)
	{
		direction[1] = 0.0;
		float length = Math.Sqrt((direction[0] * direction[0]) + (direction[2] * direction[2]));
		if (length <= 0.001)
			return JLH_AddonWorldQuery.DirectionFromAngle(Math.RandomFloat(0.0, 360.0));

		return Vector(direction[0] / length, 0.0, direction[2] / length);
	}
}

class JLH_AmbientVehiclePatrolManager
{
	protected static const string LOG_PREFIX = "[AMBIENT_VEHICLE_PATROL]";
	protected static const int SCAN_INTERVAL_MS = 37000;
	protected static const int AREA_TICK_STAGGER_MS = 3000;
	protected static const int MIN_AREA_TICK_STAGGER_MS = 250;
	protected static const int TICK_SWEEP_RESERVE_MS = 3500;

	protected static ref array<JLH_AmbientVehiclePatrolAreaComponent> s_aAreas = {};
	protected static bool s_bStarted;
	protected static int s_iNextPatrolId = 1;
	protected static int s_iTickGeneration;

	static void RegisterArea(JLH_AmbientVehiclePatrolAreaComponent area)
	{
		PruneInvalidAreas();
		if (!area || s_aAreas.Contains(area))
			return;

		s_aAreas.Insert(area);
		if (s_bStarted)
			area.RegisterArea();
	}

	static void Start(SCR_GameModeCampaign gameMode)
	{
		if (s_bStarted)
			return;

		s_bStarted = true;
		s_iNextPatrolId = 1;
		s_iTickGeneration = 0;

		foreach (JLH_AmbientVehiclePatrolAreaComponent area : s_aAreas)
		{
			if (area)
				area.RegisterArea();
		}

		if (GetGame() && GetGame().GetCallqueue())
		{
			GetGame().GetCallqueue().Remove(JLH_AmbientVehiclePatrolManager_Tick);
			GetGame().GetCallqueue().CallLater(JLH_AmbientVehiclePatrolManager_Tick, SCAN_INTERVAL_MS, true);
		}

	}

	static void ResetForNewGame()
	{
		if (GetGame() && GetGame().GetCallqueue())
			GetGame().GetCallqueue().Remove(JLH_AmbientVehiclePatrolManager_Tick);

		foreach (JLH_AmbientVehiclePatrolAreaComponent area : s_aAreas)
		{
			if (area)
				area.CleanupAllRuntime("reset");
		}

		s_bStarted = false;
		s_iNextPatrolId = 1;
		s_iTickGeneration++;
		PruneInvalidAreas();
	}

	static int NextPatrolId()
	{
		int id = s_iNextPatrolId;
		s_iNextPatrolId++;
		return id;
	}

	static bool HasContainingAreaForMarker(IEntity markerEntity, JLH_AmbientVehiclePatrolAreaComponent markerRuntimeComponent)
	{
		if (!markerEntity)
			return false;

		PruneInvalidAreas();
		foreach (JLH_AmbientVehiclePatrolAreaComponent area : s_aAreas)
		{
			if (!area || area == markerRuntimeComponent || !area.IsEnabled() || area.IsMarkerRuntimeComponent())
				continue;

			if (area.ContainsWorldPosition(markerEntity.GetOrigin()))
				return true;
		}

		return false;
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
			JLH_AmbientVehiclePatrolAreaComponent area = s_aAreas[i];
			if (area)
				ScheduleAreaTick(area, generation, i * staggerMs);
		}
	}

	static void TickAreaDelayed(JLH_AmbientVehiclePatrolAreaComponent area, int generation)
	{
		if (!s_bStarted || generation != s_iTickGeneration)
			return;

		if (!area || !area.GetOwner())
			return;

		area.Tick();
	}

	protected static void ScheduleAreaTick(notnull JLH_AmbientVehiclePatrolAreaComponent area, int generation, int delayMs)
	{
		if (delayMs <= 0 || !GetGame() || !GetGame().GetCallqueue())
		{
			TickAreaDelayed(area, generation);
			return;
		}

		GetGame().GetCallqueue().CallLater(JLH_AmbientVehiclePatrolManager_TickAreaDelayed, delayMs, false, area, generation);
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

void JLH_AmbientVehiclePatrolManager_Tick()
{
	JLH_AmbientVehiclePatrolManager.Tick();
}

void JLH_AmbientVehiclePatrolManager_TickAreaDelayed(JLH_AmbientVehiclePatrolAreaComponent area, int generation)
{
	JLH_AmbientVehiclePatrolManager.TickAreaDelayed(area, generation);
}

void JLH_AmbientVehiclePatrol_FinalizePendingMount(JLH_AmbientVehiclePatrolAreaComponent area, int runtimeId)
{
	if (area)
		area.FinalizePendingMount(runtimeId);
}
