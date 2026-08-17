class JLH_AddonSpawnUtility
{
	static const ResourceName WAYPOINT_MOVE = "{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et";
	static const ResourceName WAYPOINT_CYCLE = "{35BD6541CBB8AC08}Prefabs/AI/Waypoints/AIWaypoint_Cycle.et";
	static const ResourceName WAYPOINT_DEFEND = "{93291E72AC23930F}Prefabs/AI/Waypoints/AIWaypoint_Defend.et";
	static const ResourceName WAYPOINT_DEFEND_CP = "{2A81753527971941}Prefabs/AI/Waypoints/AIWaypoint_Defend_CP.et";
	static const ResourceName WAYPOINT_SEARCH = "{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et";
	static const ResourceName WAYPOINT_OBSERVATION = "{93291E72AC23930F}Prefabs/AI/Waypoints/AIWaypoint_Defend.et";
	static const ResourceName WAYPOINT_ARTILLERY_SUPPORT = "";

	static const ResourceName GROUP_USSR_SENTRY = "{CB58D90EA14430AD}Prefabs/Groups/OPFOR/Group_USSR_SentryTeam.et";
	static const ResourceName GROUP_USSR_LIGHT_FIRETEAM = "{657590C1EC9E27D3}Prefabs/Groups/OPFOR/Group_USSR_LightFireTeam.et";
	static const ResourceName GROUP_USSR_MACHINEGUN = "{A2F75E45C66B1C0A}Prefabs/Groups/OPFOR/Group_USSR_MachineGunTeam.et";
	static const ResourceName GROUP_USSR_FIREGROUP = "{30ED11AA4F0D41E5}Prefabs/Groups/OPFOR/Group_USSR_FireGroup.et";
	static const ResourceName GROUP_USSR_RIFLESQUAD = "{E552DABF3636C2AD}Prefabs/Groups/OPFOR/Group_USSR_RifleSquad.et";
	static const ResourceName GROUP_USSR_HUNTER_MEDIUM = "{B0F1A7E33C6B4212}Prefabs/Groups/JLH_DCF/USSR_GroupLibrary/JLH_USSR_HunterMedium_01.et";
	static const ResourceName GROUP_USSR_HUNTER_HEAVY = "{B0F1A7E33C6B4213}Prefabs/Groups/JLH_DCF/USSR_GroupLibrary/JLH_USSR_HunterHeavy_01.et";
	protected static ref map<string, bool> s_mPrefabLoadValidationCache = new map<string, bool>();

	static bool CanLoadPrefab(ResourceName prefab)
	{
		if (prefab == "")
			return false;

		string key = prefab;
		if (!s_mPrefabLoadValidationCache)
			s_mPrefabLoadValidationCache = new map<string, bool>();

		if (s_mPrefabLoadValidationCache.Contains(key))
			return s_mPrefabLoadValidationCache.Get(key);

		Resource resource = Resource.Load(prefab);
		bool canLoad = resource && resource.IsValid();
		s_mPrefabLoadValidationCache.Set(key, canLoad);
		return canLoad;
	}

	static void ResetPrefabValidationCache()
	{
		if (s_mPrefabLoadValidationCache)
			s_mPrefabLoadValidationCache.Clear();
	}

	static SCR_AIGroup SpawnGroup(ResourceName groupPrefab, vector position, string reason)
	{
		Resource resource = Resource.Load(groupPrefab);
		if (!resource || !resource.IsValid() || !GetGame() || !GetGame().GetWorld())
			return null;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[3] = JLH_AddonWorldQuery.GroundPosition(position);

		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams));
		if (!group)
			return null;

		return PrepareSpawnedGroup(group, reason, true);
	}

	static SCR_AIGroup SpawnGroupDormant(ResourceName groupPrefab, vector position, string reason)
	{
		Resource resource = Resource.Load(groupPrefab);
		if (!resource || !resource.IsValid() || !GetGame() || !GetGame().GetWorld())
			return null;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[3] = JLH_AddonWorldQuery.GroundPosition(position);

		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams));
		if (!group)
			return null;

		return PrepareSpawnedGroup(group, reason, false);
	}

	static SCR_AIGroup SpawnGroupAtExactTransform(ResourceName groupPrefab, IEntity transformSource, string reason)
	{
		if (!transformSource)
			return null;

		Resource resource = Resource.Load(groupPrefab);
		if (!resource || !resource.IsValid() || !GetGame() || !GetGame().GetWorld())
			return null;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		transformSource.GetTransform(spawnParams.Transform);

		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams));
		if (!group)
			return null;

		return PrepareSpawnedGroup(group, reason, true);
	}

	static SCR_AIGroup SpawnGroupAtExactTransformDormant(ResourceName groupPrefab, IEntity transformSource, string reason)
	{
		if (!transformSource)
			return null;

		Resource resource = Resource.Load(groupPrefab);
		if (!resource || !resource.IsValid() || !GetGame() || !GetGame().GetWorld())
			return null;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		transformSource.GetTransform(spawnParams.Transform);

		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams));
		if (!group)
			return null;

		return PrepareSpawnedGroup(group, reason, false);
	}

	protected static SCR_AIGroup PrepareSpawnedGroup(SCR_AIGroup group, string reason, bool activate)
	{
		if (!group)
			return null;

		if (!group.GetSpawnImmediately())
			group.SpawnUnits();

		if (activate)
			ActivateGroup(group);

		JLH_DCF_PersistenceExclusion.StopTrackingTree(group, reason);
		return group;
	}

	static void ApplyExactUnitMarkerTransforms(SCR_AIGroup group, notnull array<IEntity> markerEntities, int markerStartIndex, vector fallbackCenter, out int unitsSeen, out int exactUnitsPlaced, out int markersUsed, string reason)
	{
		unitsSeen = 0;
		exactUnitsPlaced = 0;
		markersUsed = 0;
		if (!group)
			return;

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		unitsSeen = agents.Count();
		int fallbackIndex = 0;

		for (int i = 0; i < agents.Count(); i++)
		{
			AIAgent agent = agents[i];
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!unit)
				continue;

			int markerIndex = markerStartIndex + markersUsed;
			if (markerIndex < markerEntities.Count() && markerEntities[markerIndex])
			{
				vector exactTransform[4];
				markerEntities[markerIndex].GetTransform(exactTransform);
				unit.SetTransform(exactTransform);
				JLH_DCF_PersistenceExclusion.StopTrackingTree(unit, reason + "_exact_unit_marker");
				exactUnitsPlaced++;
				markersUsed++;
				continue;
			}

			unit.SetOrigin(BuildSafeFallbackUnitPosition(fallbackCenter, fallbackIndex));
			JLH_DCF_PersistenceExclusion.StopTrackingTree(unit, reason + "_fallback_unit_position");
			fallbackIndex++;
		}
	}

	static int ScatterGroupUnitsAround(SCR_AIGroup group, vector center, int dangerUnits, float dangerMinDistance, float dangerMaxDistance, float survivorMinDistance, float survivorMaxDistance, string reason)
	{
		int casualties;
		return ScatterGroupUnitsAroundInternal(group, center, dangerUnits, 0, dangerMinDistance, dangerMaxDistance, survivorMinDistance, survivorMaxDistance, casualties, reason);
	}

	static int ScatterGroupUnitsAroundWithCasualties(SCR_AIGroup group, vector center, int dangerUnits, int casualtyUnits, float dangerMinDistance, float dangerMaxDistance, float survivorMinDistance, float survivorMaxDistance, out int casualties, string reason)
	{
		return ScatterGroupUnitsAroundInternal(group, center, dangerUnits, casualtyUnits, dangerMinDistance, dangerMaxDistance, survivorMinDistance, survivorMaxDistance, casualties, reason);
	}

	protected static int ScatterGroupUnitsAroundInternal(SCR_AIGroup group, vector center, int dangerUnits, int casualtyUnits, float dangerMinDistance, float dangerMaxDistance, float survivorMinDistance, float survivorMaxDistance, out int casualties, string reason)
	{
		casualties = 0;
		if (!group)
			return 0;

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		if (agents.IsEmpty())
			return 0;

		int moved = 0;
		vector baseGround = JLH_AddonWorldQuery.GroundPosition(center);
		float baseAngle = Math.RandomFloat(0.0, 360.0);
		int unitIndex = 0;
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!unit)
				continue;

			bool danger = unitIndex < dangerUnits;
			float minDistance = dangerMinDistance;
			float maxDistance = dangerMaxDistance;
			if (!danger)
			{
				minDistance = survivorMinDistance;
				maxDistance = survivorMaxDistance;
			}

			vector position = BuildScatterPosition(baseGround, baseAngle, unitIndex, agents.Count(), minDistance, maxDistance);
			unit.SetOrigin(position);
			JLH_DCF_PersistenceExclusion.StopTrackingTree(unit, reason + "_scatter_unit");
			if (danger && casualties < casualtyUnits && NeutralizeRuntimeUnit(unit, reason + "_crash_casualty"))
				casualties++;

			moved++;
			unitIndex++;
		}

		return moved;
	}

	static bool NeutralizeRuntimeUnit(IEntity unit, string reason)
	{
		if (!unit || JLH_DCF_PlayerPresence.IsPlayerControlledEntity(unit))
			return false;

		if (DestroyRuntimeEntity(unit, reason))
			return true;

		return !IsUnitAlive(unit);
	}

	static bool DestroyRuntimeEntity(IEntity entity, string reason)
	{
		if (!entity)
			return false;

		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.GetDamageManager(entity);
		if (damageManager)
		{
			bool destroyed = damageManager.SetHealthScaled(0.0);
			if (!destroyed)
			{
				DamageManagerComponent postDamageManager = DamageManagerComponent.Cast(entity.FindComponent(DamageManagerComponent));
				if (postDamageManager)
					destroyed = postDamageManager.IsDestroyed() || postDamageManager.GetState() == EDamageState.DESTROYED || postDamageManager.GetHealthScaled() <= 0.0;
			}

			if (destroyed)
				JLH_DCF_PersistenceExclusion.StopTrackingTree(entity, reason);

			return destroyed;
		}

		DamageManagerComponent baseDamageManager = DamageManagerComponent.Cast(entity.FindComponent(DamageManagerComponent));
		if (!baseDamageManager)
			return false;

		bool baseDestroyed = baseDamageManager.SetHealthScaled(0.0);
		if (!baseDestroyed)
			baseDestroyed = baseDamageManager.IsDestroyed() || baseDamageManager.GetState() == EDamageState.DESTROYED || baseDamageManager.GetHealthScaled() <= 0.0;

		if (baseDestroyed)
			JLH_DCF_PersistenceExclusion.StopTrackingTree(entity, reason);

		return baseDestroyed;
	}

	protected static vector BuildScatterPosition(vector center, float baseAngle, int index, int count, float minDistance, float maxDistance)
	{
		if (maxDistance < minDistance)
		{
			float originalMin = minDistance;
			minDistance = maxDistance;
			maxDistance = originalMin;
		}

		float angleStep = 360.0 / Math.Max(1.0, count);
		float angle = baseAngle + (index * angleStep) + Math.RandomFloat(-16.0, 16.0);
		float distance = Math.RandomFloat(minDistance, maxDistance);

		for (int attempt = 0; attempt < 8; attempt++)
		{
			vector candidate = JLH_AddonWorldQuery.GroundPosition(center + (JLH_AddonWorldQuery.DirectionFromAngle(angle + (attempt * 41.0)) * distance));
			if (!JLH_AddonWorldQuery.IsWaterOrNearWater(candidate, 4.0))
				return candidate;
		}

		return center;
	}

	protected static vector BuildSafeFallbackUnitPosition(vector fallbackCenter, int fallbackIndex)
	{
		vector baseGround = JLH_AddonWorldQuery.GroundPosition(fallbackCenter);
		float ringDistance = 3.0 + (fallbackIndex * 1.6);
		float angle = fallbackIndex * 137.5;

		for (int attempt = 0; attempt < 8; attempt++)
		{
			vector candidate = JLH_AddonWorldQuery.GroundPosition(baseGround + (JLH_AddonWorldQuery.DirectionFromAngle(angle + (attempt * 47.0)) * ringDistance));
			if (!JLH_AddonWorldQuery.IsWaterOrNearWater(candidate, 4.0))
				return candidate;
		}

		return baseGround;
	}

	static IEntity SpawnEntity(ResourceName prefab, vector position, string reason)
	{
		Resource resource = Resource.Load(prefab);
		if (!resource || !resource.IsValid() || !GetGame() || !GetGame().GetWorld())
			return null;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[3] = JLH_AddonWorldQuery.GroundPosition(position);

		IEntity entity = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams);
		if (entity)
			JLH_DCF_PersistenceExclusion.StopTrackingTree(entity, reason);

		return entity;
	}

	static bool DrainVehicleFuel(IEntity vehicle)
	{
		FuelManagerComponent fuelManager = GetVehicleFuelManager(vehicle);
		if (!fuelManager)
			return false;

		array<BaseFuelNode> fuelNodes = {};
		int nodeCount = fuelManager.GetFuelNodesList(fuelNodes);
		if (nodeCount <= 0)
			return false;

		bool drained = false;
		foreach (BaseFuelNode fuelNode : fuelNodes)
		{
			if (!fuelNode)
				continue;

			if (fuelNode.GetFuel() <= 0.0)
				continue;

			fuelNode.SetFuel(0.0);
			drained = true;
		}

		return drained;
	}

	protected static FuelManagerComponent GetVehicleFuelManager(IEntity vehicle)
	{
		if (!vehicle)
			return null;

		BaseVehicleControllerComponent controller = BaseVehicleControllerComponent.Cast(vehicle.FindComponent(BaseVehicleControllerComponent));
		if (!controller)
			return null;

		return controller.GetFuelManager();
	}

	static AIWaypoint SpawnWaypoint(ResourceName waypointPrefab, vector position, string reason)
	{
		IEntity waypointEntity = SpawnEntity(waypointPrefab, position, reason);
		AIWaypoint waypoint = AIWaypoint.Cast(waypointEntity);
		if (!waypoint)
		{
			if (waypointEntity)
				DeleteRuntimeEntity(waypointEntity, reason + "_invalid_waypoint");

			return null;
		}

		HideRuntimeWaypoint(waypointEntity);
		return waypoint;
	}

	static bool AssignSingleWaypoint(SCR_AIGroup group, ResourceName waypointPrefab, vector target, notnull JLH_AddonRuntimeEntityRegistry registry, string reason)
	{
		if (!group)
			return false;

		JLH_DCF_NodeWaypointResolver.ClearGroupWaypoints(group);
		AIWaypoint waypoint = SpawnWaypoint(waypointPrefab, target, reason);
		if (!waypoint)
			return false;

		group.AddWaypoint(waypoint);
		registry.TrackWaypoint(waypoint, reason);
		ActivateGroup(group);
		return true;
	}

	static bool AssignMoveAndSearch(SCR_AIGroup group, vector target, float searchRadius, int searchPoints, notnull JLH_AddonRuntimeEntityRegistry registry, string reason)
	{
		if (!group)
			return false;

		JLH_DCF_NodeWaypointResolver.ClearGroupWaypoints(group);
		array<AIWaypoint> routeWaypoints = {};
		bool assigned = false;
		vector routeCenter;
		if (!ResolveDryPatrolPoint(target, target, Math.Max(searchRadius * 0.5, 20.0), routeCenter))
			routeCenter = JLH_AddonWorldQuery.GroundPosition(target);

		AIWaypoint move = SpawnWaypoint(WAYPOINT_MOVE, routeCenter, reason + "_move");
		if (move)
		{
			group.AddWaypoint(move);
			routeWaypoints.Insert(move);
			registry.TrackWaypoint(move, reason + "_move");
			assigned = true;
		}

		for (int i = 0; i < searchPoints; i++)
		{
			float angle = Math.RandomFloat(0.0, 360.0) + (i * 121.0);
			float distance = Math.RandomFloat(searchRadius * 0.35, searchRadius);
			vector point = JLH_AddonWorldQuery.GroundPosition(routeCenter + (JLH_AddonWorldQuery.DirectionFromAngle(angle) * distance));
			if (JLH_AddonWorldQuery.IsWaterOrNearWater(point, 6.0))
				continue;

			AIWaypoint search = SpawnWaypoint(WAYPOINT_MOVE, point, reason + "_search");
			if (!search)
				continue;

			group.AddWaypoint(search);
			routeWaypoints.Insert(search);
			registry.TrackWaypoint(search, reason + "_search");
			assigned = true;
		}

		AIWaypoint cycle = SpawnWaypoint(WAYPOINT_CYCLE, routeCenter, reason + "_cycle");
		if (cycle)
		{
			ConfigureCycleWaypoint(cycle, routeWaypoints);
			group.AddWaypoint(cycle);
			registry.TrackWaypoint(cycle, reason + "_cycle");
			assigned = true;
		}

		ActivateGroup(group);
		return assigned;
	}

	static bool AssignGeneratedPatrolLoop(SCR_AIGroup group, vector center, vector facing, float radius, int pointCount, notnull JLH_AddonRuntimeEntityRegistry registry, string reason)
	{
		if (!group)
			return false;

		if (radius < 35.0)
			radius = 35.0;

		if (pointCount < 4)
			pointCount = 4;

		JLH_DCF_NodeWaypointResolver.ClearGroupWaypoints(group);
		array<AIWaypoint> routeWaypoints = {};
		vector patrolCenter = JLH_AddonWorldQuery.GroundPosition(center);
		vector forward = NormalizeHorizontalDirection(facing);
		vector right = Vector(-forward[2], 0, forward[0]);

		for (int i = 0; i < pointCount; i++)
		{
			vector offset = BuildPatrolOffset(i, forward, right, radius);
			vector point;
			if (!ResolveDryPatrolPoint(patrolCenter + offset, patrolCenter, radius, point))
				continue;

			AIWaypoint waypoint = SpawnWaypoint(WAYPOINT_MOVE, point, reason + "_patrol");
			if (!waypoint)
				continue;

			group.AddWaypoint(waypoint);
			routeWaypoints.Insert(waypoint);
			registry.TrackWaypoint(waypoint, reason + "_patrol");
		}

		if (routeWaypoints.IsEmpty())
		{
			ActivateGroup(group);
			return false;
		}

		AIWaypoint cycle = SpawnWaypoint(WAYPOINT_CYCLE, routeWaypoints[0].GetOrigin(), reason + "_cycle");
		if (cycle)
		{
			ConfigureCycleWaypoint(cycle, routeWaypoints);
			group.AddWaypoint(cycle);
			registry.TrackWaypoint(cycle, reason + "_cycle");
		}

		ActivateGroup(group);
		return true;
	}

	static bool AssignGeneratedCasualWanderLoop(SCR_AIGroup group, vector center, vector facing, float radius, notnull JLH_AddonRuntimeEntityRegistry registry, string reason)
	{
		if (!group)
			return false;

		radius = Math.Clamp(radius, 10.0, 30.0);
		JLH_DCF_NodeWaypointResolver.ClearGroupWaypoints(group);
		array<AIWaypoint> routeWaypoints = {};
		vector patrolCenter = JLH_AddonWorldQuery.GroundPosition(center);
		vector forward = NormalizeHorizontalDirection(facing);
		vector right = Vector(-forward[2], 0, forward[0]);

		for (int i = 0; i < 2; i++)
		{
			vector offset;
			if (i == 0)
				offset = (forward * Math.RandomFloat(radius * 0.35, radius)) + (right * Math.RandomFloat(-radius * 0.25, radius * 0.25));
			else
				offset = (forward * Math.RandomFloat(-radius, -radius * 0.35)) + (right * Math.RandomFloat(-radius * 0.25, radius * 0.25));

			vector point;
			if (!ResolveDryPatrolPoint(patrolCenter + offset, patrolCenter, radius, point))
				continue;

			AIWaypoint waypoint = SpawnWaypoint(WAYPOINT_MOVE, point, reason + "_walk");
			if (!waypoint)
				continue;

			group.AddWaypoint(waypoint);
			routeWaypoints.Insert(waypoint);
			registry.TrackWaypoint(waypoint, reason + "_walk");
		}

		if (routeWaypoints.IsEmpty())
		{
			ActivateGroup(group);
			return false;
		}

		AIWaypoint cycle = SpawnWaypoint(WAYPOINT_CYCLE, routeWaypoints[0].GetOrigin(), reason + "_cycle");
		if (cycle)
		{
			ConfigureCycleWaypoint(cycle, routeWaypoints);
			group.AddWaypoint(cycle);
			registry.TrackWaypoint(cycle, reason + "_cycle");
		}

		ActivateGroup(group);
		return true;
	}

	static bool AssignGeneratedCivilianRoadWanderLoop(SCR_AIGroup group, notnull array<vector> routePoints, notnull JLH_AddonRuntimeEntityRegistry registry, string reason)
	{
		if (!group || routePoints.Count() < 2)
			return false;

		JLH_DCF_NodeWaypointResolver.ClearGroupWaypoints(group);
		array<AIWaypoint> routeWaypoints = {};
		foreach (vector routePoint : routePoints)
		{
			vector point = JLH_AddonWorldQuery.GroundPosition(routePoint);
			if (JLH_AddonWorldQuery.IsWaterOrNearWater(point, 6.0))
				continue;

			AIWaypoint waypoint = SpawnWaypoint(WAYPOINT_MOVE, point, reason + "_road_walk");
			if (!waypoint)
				continue;

			group.AddWaypoint(waypoint);
			routeWaypoints.Insert(waypoint);
			registry.TrackWaypoint(waypoint, reason + "_road_walk");
		}

		if (routeWaypoints.IsEmpty())
		{
			ActivateGroup(group);
			return false;
		}

		AIWaypoint cycle = SpawnWaypoint(WAYPOINT_CYCLE, routeWaypoints[0].GetOrigin(), reason + "_cycle");
		if (cycle)
		{
			ConfigureCycleWaypoint(cycle, routeWaypoints);
			group.AddWaypoint(cycle);
			registry.TrackWaypoint(cycle, reason + "_cycle");
		}

		ActivateGroup(group);
		return true;
	}

	protected static void ConfigureCycleWaypoint(AIWaypoint cycle, notnull array<AIWaypoint> routeWaypoints)
	{
		AIWaypointCycle cycleWaypoint = AIWaypointCycle.Cast(cycle);
		if (!cycleWaypoint || routeWaypoints.IsEmpty())
			return;

		cycleWaypoint.SetRerunCounter(-1);
		cycleWaypoint.SetWaypoints(routeWaypoints);
	}

	protected static vector NormalizeHorizontalDirection(vector direction)
	{
		direction[1] = 0;
		float length = Math.Sqrt((direction[0] * direction[0]) + (direction[2] * direction[2]));
		if (length <= 0.001)
			return JLH_AddonWorldQuery.DirectionFromAngle(Math.RandomFloat(0.0, 360.0));

		return Vector(direction[0] / length, 0, direction[2] / length);
	}

	protected static vector BuildPatrolOffset(int index, vector forward, vector right, float radius)
	{
		int slot = index % 4;
		if (slot == 0)
			return forward * radius;

		if (slot == 1)
			return right * radius;

		if (slot == 2)
			return forward * -radius;

		return right * -radius;
	}

	protected static bool ResolveDryPatrolPoint(vector desired, vector center, float radius, out vector point)
	{
		point = JLH_AddonWorldQuery.GroundPosition(desired);
		if (!JLH_AddonWorldQuery.IsWaterOrNearWater(point, 8.0))
			return true;

		float searchRadius = Math.Clamp(radius * 0.35, 12.0, 80.0);
		for (int attempt = 0; attempt < 10; attempt++)
		{
			float angle = attempt * 36.0;
			vector candidate = JLH_AddonWorldQuery.GroundPosition(desired + (JLH_AddonWorldQuery.DirectionFromAngle(angle) * searchRadius));
			if (!JLH_AddonWorldQuery.IsWaterOrNearWater(candidate, 8.0))
			{
				point = candidate;
				return true;
			}
		}

		vector centerGround = JLH_AddonWorldQuery.GroundPosition(center);
		if (!JLH_AddonWorldQuery.IsWaterOrNearWater(centerGround, 8.0))
		{
			point = centerGround;
			return true;
		}

		return false;
	}

	static bool AssignMountedVehicleHold(SCR_AIGroup group, vector target, notnull JLH_AddonRuntimeEntityRegistry registry, string reason)
	{
		if (!group)
			return false;

		JLH_DCF_NodeWaypointResolver.ClearGroupWaypoints(group);

		vector holdPoint;
		if (!ResolveDryPatrolPoint(target, target, 40.0, holdPoint))
			holdPoint = JLH_AddonWorldQuery.GroundPosition(target);

		AIWaypoint move = SpawnWaypoint(WAYPOINT_MOVE, holdPoint, reason + "_move");
		if (!move)
			return false;

		group.AddWaypoint(move);
		registry.TrackWaypoint(move, reason + "_move");

		array<AIWaypoint> routeWaypoints = {};
		routeWaypoints.Insert(move);

		AIWaypoint cycle = SpawnWaypoint(WAYPOINT_CYCLE, holdPoint, reason + "_cycle");
		if (cycle)
		{
			ConfigureCycleWaypoint(cycle, routeWaypoints);
			group.AddWaypoint(cycle);
			registry.TrackWaypoint(cycle, reason + "_cycle");
		}

		ActivateGroup(group);
		return true;
	}

	static bool AssignDefend(SCR_AIGroup group, vector target, notnull JLH_AddonRuntimeEntityRegistry registry, string reason)
	{
		if (AssignSingleWaypoint(group, WAYPOINT_DEFEND_CP, target, registry, reason + "_defend_cp"))
			return true;

		return AssignSingleWaypoint(group, WAYPOINT_DEFEND, target, registry, reason + "_defend");
	}

	static bool AssignPositionalDefend(SCR_AIGroup group, vector target, notnull JLH_AddonRuntimeEntityRegistry registry, string reason)
	{
		return AssignSingleWaypoint(group, WAYPOINT_DEFEND, target, registry, reason + "_defend");
	}

	static bool AssignArtillerySupport(SCR_AIGroup group, vector target, int shotCount, notnull JLH_AddonRuntimeEntityRegistry registry, string reason)
	{
		if (!group)
			return false;

		AIWaypoint waypoint = SpawnWaypoint(WAYPOINT_ARTILLERY_SUPPORT, target, reason + "_artillery_waypoint");
		if (!waypoint)
			return false;

		SCR_AIWaypointArtillerySupport artilleryWaypoint = SCR_AIWaypointArtillerySupport.Cast(waypoint);
		if (artilleryWaypoint)
		{
			artilleryWaypoint.SetTargetShotCount(shotCount);
			artilleryWaypoint.SetActive(true);
		}

		group.AddWaypoint(waypoint);
		registry.TrackWaypoint(waypoint, reason + "_artillery_waypoint");
		ActivateGroup(group);
		return true;
	}

	static void ActivateGroup(SCR_AIGroup group)
	{
		if (!group)
			return;

		group.ActivateAI();
		//group.ActivateAllMembers();

		AIControlComponent control = group.GetControlComponent();
		if (control)
			control.ActivateAI();
	}

	static int CountAliveGroupUnits(SCR_AIGroup group)
	{
		if (!group)
			return 0;

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		int alive = 0;
		foreach (AIAgent agent : agents)
		{
			if (agent && IsUnitAlive(agent.GetControlledEntity()))
				alive++;
		}

		return alive;
	}

	static bool IsGroupAlive(SCR_AIGroup group)
	{
		return CountAliveGroupUnits(group) > 0;
	}

	static int PruneGroupToAliveCount(SCR_AIGroup group, int targetAliveCount, string reason)
	{
		if (!group)
			return 0;

		if (targetAliveCount < 0)
			targetAliveCount = 0;

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		int kept = 0;
		for (int i = 0; i < agents.Count(); i++)
		{
			AIAgent agent = agents[i];
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (!unit || !IsUnitAlive(unit))
				continue;

			if (kept < targetAliveCount)
			{
				kept++;
				continue;
			}

			if (!JLH_DCF_PlayerPresence.IsPlayerControlledEntity(unit))
			{
				JLH_DCF_RuntimeCleanupManager.Unregister(unit);
				DeleteRuntimeEntity(unit, reason + "_excess_unit");
			}
		}

		return kept;
	}

	static bool IsUnitAlive(IEntity unit)
	{
		if (!unit)
			return false;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(unit);
		if (!character)
			return true;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (controller && controller.IsDead())
			return false;

		DamageManagerComponent damageManager = DamageManagerComponent.Cast(unit.FindComponent(DamageManagerComponent));
		if (damageManager)
		{
			if (damageManager.IsDestroyed())
				return false;
			if (damageManager.GetState() == EDamageState.DESTROYED)
				return false;
			if (damageManager.GetHealthScaled() <= 0.0)
				return false;
		}

		return true;
	}

	static void DeleteGroupAndMembers(SCR_AIGroup group, string reason)
	{
		if (!group)
			return;

		array<AIWaypoint> waypoints = {};
		group.GetWaypoints(waypoints);
		foreach (AIWaypoint waypoint : waypoints)
		{
			if (!waypoint)
				continue;

			group.RemoveWaypoint(waypoint);
			DeleteRuntimeEntity(waypoint, reason + "_waypoint");
		}

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (unit && !JLH_DCF_PlayerPresence.IsPlayerControlledEntity(unit))
				DeleteRuntimeEntity(unit, reason + "_unit");
		}

		DeleteRuntimeEntity(group, reason);
	}

	static void DeleteRuntimeEntity(IEntity entity, string reason)
	{
		if (!entity)
			return;

		JLH_DCF_RuntimeCleanupManager.UnregisterTree(entity);
		JLH_AddonRuntimeEntityMetadata.ForgetTree(entity);
		JLH_DCF_PersistenceExclusion.StopTrackingTreeNow(entity, reason + "_predelete");
		SCR_EntityHelper.DeleteEntityAndChildren(entity);
	}

	static ResourceName SelectRandomPrefab(notnull array<ResourceName> prefabs)
	{
		if (prefabs.IsEmpty())
			return "";

		int start = Math.RandomInt(0, prefabs.Count());
		for (int i = 0; i < prefabs.Count(); i++)
		{
			ResourceName prefab = prefabs[(start + i) % prefabs.Count()];
			if (CanLoadPrefab(prefab))
				return prefab;
		}

		return "";
	}

	protected static void HideRuntimeWaypoint(IEntity waypointEntity)
	{
		if (!waypointEntity)
			return;

		SCR_EditableEntityComponent editable = SCR_EditableEntityComponent.GetEditableEntity(waypointEntity);
		if (!editable)
			editable = SCR_EditableEntityComponent.Cast(waypointEntity.FindComponent(SCR_EditableEntityComponent));

		if (editable)
			editable.SetVisible(false);
	}

	static void HideEditablePresentation(IEntity entity)
	{
		HideRuntimeWaypoint(entity);
	}

	static void HideGroupEditablePresentation(SCR_AIGroup group)
	{
		if (!group)
			return;

		HideEditablePresentation(group);

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity unit = agent.GetControlledEntity();
			if (unit)
				HideEditablePresentation(unit);
		}
	}
}
