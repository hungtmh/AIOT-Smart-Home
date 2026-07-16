package com.aiot.smarthome.repository;

import com.aiot.smarthome.model.DeviceState;
import java.sql.Timestamp;
import java.time.OffsetDateTime;
import java.time.ZoneOffset;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.dao.DataAccessException;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.stereotype.Repository;

@Repository
public class LedRepository {
  private static final Logger logger = LoggerFactory.getLogger(LedRepository.class);
  private static final String LED_DEVICE_ID = "led";

  private final JdbcTemplate jdbcTemplate;
  private boolean desiredLedState;
  private boolean reportedLedState;
  private OffsetDateTime desiredUpdatedAt = OffsetDateTime.now(ZoneOffset.UTC);
  private OffsetDateTime reportedUpdatedAt = OffsetDateTime.now(ZoneOffset.UTC);
  private boolean databaseAvailable = true;
  private boolean databaseWarningLogged;

  public LedRepository(JdbcTemplate jdbcTemplate) {
    this.jdbcTemplate = jdbcTemplate;
  }

  public DeviceState findLedState() {
    if (!databaseAvailable) {
      return memoryState();
    }

    try {
      ensureLedDevice();

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
          LED_DEVICE_ID);

      if (state != null) {
        syncMemoryState(state);
        return state;
      }
    } catch (DataAccessException exception) {
      warnDatabaseFallback(exception);
    }

    return memoryState();
  }

  public DeviceState setDesiredLedState(boolean state) {
    desiredLedState = state;
    desiredUpdatedAt = OffsetDateTime.now(ZoneOffset.UTC);

    if (!databaseAvailable) {
      return memoryState();
    }

    try {
      ensureLedDevice();

      jdbcTemplate.update(
          """
          update device_states
          set desired_state = ?, updated_at = now()
          where device_id = ?
          """,
          state,
          LED_DEVICE_ID);

      return findLedState();
    } catch (DataAccessException exception) {
      warnDatabaseFallback(exception);
      return memoryState();
    }
  }

  public DeviceState setReportedLedState(boolean state) {
    reportedLedState = state;
    reportedUpdatedAt = OffsetDateTime.now(ZoneOffset.UTC);

    if (!databaseAvailable) {
      return memoryState();
    }

    try {
      ensureLedDevice();

      jdbcTemplate.update(
          """
          update device_states
          set reported_state = ?, reported_at = now()
          where device_id = ?
          """,
          state,
          LED_DEVICE_ID);

      return findLedState();
    } catch (DataAccessException exception) {
      warnDatabaseFallback(exception);
      return memoryState();
    }
  }

  public void addControlLog(boolean requestedState, String source, String result) {
    if (!databaseAvailable) {
      return;
    }

    try {
      ensureLedDevice();

      jdbcTemplate.update(
          """
          insert into control_logs (device_id, requested_state, source, result)
          values (?, ?, ?, ?)
          """,
          LED_DEVICE_ID,
          requestedState,
          source,
          result);
    } catch (DataAccessException exception) {
      warnDatabaseFallback(exception);
    }
  }

  private void ensureLedDevice() {
    jdbcTemplate.update(
        """
        insert into devices (id, name, type)
        values (?, ?, ?)
        on conflict (id) do nothing
        """,
        LED_DEVICE_ID,
        "LED Light",
        "relay");

    jdbcTemplate.update(
        """
        insert into device_states (device_id, desired_state, reported_state)
        values (?, false, false)
        on conflict (device_id) do nothing
        """,
        LED_DEVICE_ID);
  }

  private OffsetDateTime toOffsetDateTime(Timestamp timestamp) {
    if (timestamp == null) {
      return null;
    }

    return timestamp.toInstant().atOffset(ZoneOffset.UTC);
  }

  private DeviceState memoryState() {
    return new DeviceState(
        LED_DEVICE_ID,
        desiredLedState,
        reportedLedState,
        desiredUpdatedAt,
        reportedUpdatedAt);
  }

  private void syncMemoryState(DeviceState state) {
    desiredLedState = state.desiredState();
    reportedLedState = state.reportedState();
    desiredUpdatedAt = state.updatedAt();
    reportedUpdatedAt = state.reportedAt();
  }

  private void warnDatabaseFallback(DataAccessException exception) {
    databaseAvailable = false;

    if (!databaseWarningLogged) {
      logger.warn("Database is unavailable; using in-memory LED state until the DB connection is fixed: {}",
          exception.getMostSpecificCause().getMessage());
      databaseWarningLogged = true;
    }
  }
}
