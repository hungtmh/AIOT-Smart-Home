package com.aiot.smarthome.repository;

import com.aiot.smarthome.model.DeviceDefinition;
import com.aiot.smarthome.model.DeviceState;
import java.sql.Timestamp;
import java.time.OffsetDateTime;
import java.time.ZoneOffset;
import java.util.Collection;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.dao.DataAccessException;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.stereotype.Repository;

@Repository
public class DeviceRepository {
  private static final Logger logger = LoggerFactory.getLogger(DeviceRepository.class);

  private final JdbcTemplate jdbcTemplate;
  private final Map<String, DeviceState> memoryStates = new ConcurrentHashMap<>();
  private boolean databaseAvailable = true;
  private boolean databaseWarningLogged;

  public DeviceRepository(JdbcTemplate jdbcTemplate) {
    this.jdbcTemplate = jdbcTemplate;
  }

  public List<DeviceState> findAllStates(Collection<DeviceDefinition> devices) {
    return devices.stream().map(this::findState).toList();
  }

  public DeviceState findState(DeviceDefinition device) {
    if (!databaseAvailable) {
      return memoryState(device.id());
    }

    try {
      ensureDevice(device);

      DeviceState state = jdbcTemplate.queryForObject(
          """
          select device_id, desired_state, reported_state, updated_at, reported_at
          from device_states
          where device_id = ?
          """,
          (rs, rowNum) -> new DeviceState(
              rs.getString("device_id"),
              rs.getBoolean("desired_state"),
              rs.getBoolean("reported_state"),
              toOffsetDateTime(rs.getTimestamp("updated_at")),
              toOffsetDateTime(rs.getTimestamp("reported_at"))),
          device.id());

      if (state != null) {
        memoryStates.put(device.id(), state);
        return state;
      }
    } catch (DataAccessException exception) {
      warnDatabaseFallback(exception);
    }

    return memoryState(device.id());
  }

  public DeviceState setDesiredState(DeviceDefinition device, boolean state) {
    updateMemoryDesired(device.id(), state);

    if (!databaseAvailable) {
      return memoryState(device.id());
    }

    try {
      ensureDevice(device);

      jdbcTemplate.update(
          """
          update device_states
          set desired_state = ?, updated_at = now()
          where device_id = ?
          """,
          state,
          device.id());

      return findState(device);
    } catch (DataAccessException exception) {
      warnDatabaseFallback(exception);
      return memoryState(device.id());
    }
  }

  public DeviceState setReportedState(DeviceDefinition device, boolean state) {
    updateMemoryReported(device.id(), state);

    if (!databaseAvailable) {
      return memoryState(device.id());
    }

    try {
      ensureDevice(device);

      jdbcTemplate.update(
          """
          update device_states
          set reported_state = ?, reported_at = now()
          where device_id = ?
          """,
          state,
          device.id());

      return findState(device);
    } catch (DataAccessException exception) {
      warnDatabaseFallback(exception);
      return memoryState(device.id());
    }
  }

  public void addControlLog(String deviceId, boolean requestedState, String source, String result) {
    if (!databaseAvailable) {
      return;
    }

    try {
      jdbcTemplate.update(
          """
          insert into control_logs (device_id, requested_state, source, result)
          values (?, ?, ?, ?)
          """,
          deviceId,
          requestedState,
          source,
          result);
    } catch (DataAccessException exception) {
      warnDatabaseFallback(exception);
    }
  }

  private void ensureDevice(DeviceDefinition device) {
    jdbcTemplate.update(
        """
        insert into devices (id, name, type)
        values (?, ?, ?)
        on conflict (id) do update set name = excluded.name, type = excluded.type
        """,
        device.id(),
        device.name(),
        device.type());

    jdbcTemplate.update(
        """
        insert into device_states (device_id, desired_state, reported_state)
        values (?, false, false)
        on conflict (device_id) do nothing
        """,
        device.id());
  }

  private OffsetDateTime toOffsetDateTime(Timestamp timestamp) {
    if (timestamp == null) {
      return null;
    }

    return timestamp.toInstant().atOffset(ZoneOffset.UTC);
  }

  private DeviceState memoryState(String deviceId) {
    return memoryStates.computeIfAbsent(deviceId, id -> {
      OffsetDateTime now = OffsetDateTime.now(ZoneOffset.UTC);
      return new DeviceState(id, false, false, now, now);
    });
  }

  private void updateMemoryDesired(String deviceId, boolean state) {
    DeviceState current = memoryState(deviceId);
    memoryStates.put(deviceId, new DeviceState(
        deviceId,
        state,
        current.reportedState(),
        OffsetDateTime.now(ZoneOffset.UTC),
        current.reportedAt()));
  }

  private void updateMemoryReported(String deviceId, boolean state) {
    DeviceState current = memoryState(deviceId);
    memoryStates.put(deviceId, new DeviceState(
        deviceId,
        current.desiredState(),
        state,
        current.updatedAt(),
        OffsetDateTime.now(ZoneOffset.UTC)));
  }

  private void warnDatabaseFallback(DataAccessException exception) {
    databaseAvailable = false;

    if (!databaseWarningLogged) {
      logger.warn("Database is unavailable; using in-memory device states until the DB connection is fixed: {}",
          exception.getMostSpecificCause().getMessage());
      databaseWarningLogged = true;
    }
  }
}
