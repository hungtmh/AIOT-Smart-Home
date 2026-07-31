package com.aiot.smarthome.repository;

import com.aiot.smarthome.dto.RecentActivityRow;
import java.sql.Timestamp;
import java.time.OffsetDateTime;
import java.time.ZoneId;
import java.time.ZoneOffset;
import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.List;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.dao.DataAccessException;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.stereotype.Repository;

@Repository
public class HistoryRepository {
  private static final Logger logger = LoggerFactory.getLogger(HistoryRepository.class);
  private static final DateTimeFormatter TIME_FORMAT =
      DateTimeFormatter.ofPattern("dd/MM/yyyy, HH:mm:ss");
  private static final ZoneId APP_ZONE = ZoneId.of(
      System.getenv().getOrDefault("APP_TIMEZONE", "Asia/Ho_Chi_Minh"));

  private final JdbcTemplate jdbcTemplate;
  private boolean databaseAvailable = true;
  private boolean databaseWarningLogged;

  public HistoryRepository(JdbcTemplate jdbcTemplate) {
    this.jdbcTemplate = jdbcTemplate;
  }

  // ── Sensors (telemetry_readings) ──────────────────────────────────────────

  public List<List<String>> findSensors(int page, int size) {
    if (!databaseAvailable) {
      return List.of();
    }

    try {
      ensureTelemetryTable();

      return jdbcTemplate.query(
          """
          select temperature, humidity, smoke_ppm, measured_at
          from telemetry_readings
          order by measured_at desc
          limit ? offset ?
          """,
          (rs, rowNum) -> {
            OffsetDateTime measuredAt = toOffsetDateTime(rs.getTimestamp("measured_at"));
            return List.of(
                formatTime(measuredAt),
                String.valueOf(rs.getDouble("temperature")),
                String.valueOf(rs.getDouble("humidity")),
                rs.getInt("smoke_ppm") + " ppm");
          },
          size,
          page * size);
    } catch (DataAccessException exception) {
      warnDatabaseFallback(exception);
      return List.of();
    }
  }

  public long countSensors() {
    return countTable("telemetry_readings");
  }

  // ── Controls (control_logs) ───────────────────────────────────────────────

  public List<List<String>> findControls(int page, int size) {
    if (!databaseAvailable) {
      return List.of();
    }

    try {
      ensureControlLogsTable();

      return jdbcTemplate.query(
          """
          select device_id, requested_state, source, result, created_at
          from control_logs
          order by created_at desc
          limit ? offset ?
          """,
          (rs, rowNum) -> {
            OffsetDateTime createdAt = toOffsetDateTime(rs.getTimestamp("created_at"));
            boolean state = rs.getBoolean("requested_state");
            return List.of(
                formatTime(createdAt),
                rs.getString("device_id"),
                state ? "ON" : "OFF",
                rs.getString("source"),
                rs.getString("result"));
          },
          size,
          page * size);
    } catch (DataAccessException exception) {
      warnDatabaseFallback(exception);
      return List.of();
    }
  }

  public long countControls() {
    return countTable("control_logs");
  }

  // ── Voice Commands ────────────────────────────────────────────────────────

  public List<List<String>> findVoiceCommands(int page, int size) {
    if (!databaseAvailable) {
      return List.of();
    }

    try {
      ensureVoiceTable();

      return jdbcTemplate.query(
          """
          select recognized_text, mapped_device, action, confidence, result, created_at
          from voice_command_history
          order by created_at desc
          limit ? offset ?
          """,
          (rs, rowNum) -> {
            OffsetDateTime createdAt = toOffsetDateTime(rs.getTimestamp("created_at"));
            return List.of(
                formatTime(createdAt),
                rs.getString("recognized_text"),
                rs.getString("mapped_device") != null ? rs.getString("mapped_device") : "",
                rs.getString("action") != null ? rs.getString("action") : "",
                rs.getObject("confidence") != null ? rs.getDouble("confidence") + "%" : "",
                rs.getString("result"));
          },
          size,
          page * size);
    } catch (DataAccessException exception) {
      warnDatabaseFallback(exception);
      return List.of();
    }
  }

  public long countVoice() {
    return countTable("voice_command_history");
  }

  public void saveVoiceCommand(String recognizedText, String mappedDevice, String action,
      Double confidence, String result) {
    if (!databaseAvailable) {
      return;
    }

    try {
      ensureVoiceTable();

      jdbcTemplate.update(
          """
          insert into voice_command_history (recognized_text, mapped_device, action, confidence, result)
          values (?, ?, ?, ?, ?)
          """,
          recognizedText,
          mappedDevice,
          action,
          confidence,
          result);
    } catch (DataAccessException exception) {
      warnDatabaseFallback(exception);
    }
  }

  // ── Alerts ────────────────────────────────────────────────────────────────

  public List<List<String>> findAlerts(int page, int size) {
    if (!databaseAvailable) {
      return List.of();
    }

    try {
      ensureAlertTable();

      return jdbcTemplate.query(
          """
          select alert_type, sensor_value, threshold, action_taken, status, created_at
          from alert_history
          order by created_at desc
          limit ? offset ?
          """,
          (rs, rowNum) -> {
            OffsetDateTime createdAt = toOffsetDateTime(rs.getTimestamp("created_at"));
            return List.of(
                formatTime(createdAt),
                rs.getString("alert_type"),
                rs.getString("sensor_value") != null ? rs.getString("sensor_value") : "",
                rs.getString("threshold") != null ? rs.getString("threshold") : "",
                rs.getString("action_taken") != null ? rs.getString("action_taken") : "",
                rs.getString("status"));
          },
          size,
          page * size);
    } catch (DataAccessException exception) {
      warnDatabaseFallback(exception);
      return List.of();
    }
  }

  public long countAlerts() {
    return countTable("alert_history");
  }

  public void saveAlert(String alertType, String sensorValue, String threshold,
      String actionTaken, String status) {
    if (!databaseAvailable) {
      return;
    }

    try {
      ensureAlertTable();

      jdbcTemplate.update(
          """
          insert into alert_history (alert_type, sensor_value, threshold, action_taken, status)
          values (?, ?, ?, ?, ?)
          """,
          alertType,
          sensorValue,
          threshold,
          actionTaken,
          status);
    } catch (DataAccessException exception) {
      warnDatabaseFallback(exception);
    }
  }

  // ── Recent Activity (union from all tables) ───────────────────────────────

  public List<RecentActivityRow> findRecentActivity(int limit) {
    if (!databaseAvailable) {
      return List.of();
    }

    try {
      ensureTelemetryTable();
      ensureControlLogsTable();
      ensureVoiceTable();
      ensureAlertTable();

      return jdbcTemplate.query(
          """
          (
            select measured_at as event_time,
                   'Environment data saved' as event,
                   'temperature ' || temperature || ' C, humidity ' || humidity || '%' as detail
            from telemetry_readings
            order by measured_at desc
            limit ?
          )
          union all
          (
            select created_at as event_time,
                   'Device control: ' || device_id as event,
                   source || ' → ' || (case when requested_state then 'ON' else 'OFF' end) || ' (' || result || ')' as detail
            from control_logs
            order by created_at desc
            limit ?
          )
          union all
          (
            select created_at as event_time,
                   'Voice command recognized' as event,
                   recognized_text as detail
            from voice_command_history
            order by created_at desc
            limit ?
          )
          union all
          (
            select created_at as event_time,
                   'Alert: ' || alert_type as event,
                   sensor_value || ' (threshold: ' || threshold || ')' as detail
            from alert_history
            order by created_at desc
            limit ?
          )
          order by event_time desc
          limit ?
          """,
          (rs, rowNum) -> {
            OffsetDateTime eventTime = toOffsetDateTime(rs.getTimestamp("event_time"));
            return new RecentActivityRow(
                formatTimeShort(eventTime),
                rs.getString("event"),
                rs.getString("detail"));
          },
          limit, limit, limit, limit, limit);
    } catch (DataAccessException exception) {
      warnDatabaseFallback(exception);
      return List.of();
    }
  }

  // ── Ensure tables ─────────────────────────────────────────────────────────

  private void ensureTelemetryTable() {
    jdbcTemplate.update(
        """
        create table if not exists telemetry_readings (
          id bigserial primary key,
          temperature numeric(5, 2) not null,
          humidity numeric(5, 2) not null,
          smoke_ppm integer not null,
          measured_at timestamptz not null default now()
        )
        """);
  }

  private void ensureControlLogsTable() {
    jdbcTemplate.update(
        """
        create table if not exists control_logs (
          id bigserial primary key,
          device_id varchar(50) not null,
          requested_state boolean not null,
          source varchar(50) not null,
          result varchar(50) not null,
          created_at timestamptz not null default now()
        )
        """);
  }

  private void ensureVoiceTable() {
    jdbcTemplate.update(
        """
        create table if not exists voice_command_history (
          id bigserial primary key,
          recognized_text text not null,
          mapped_device varchar(50),
          action varchar(50),
          confidence numeric(5, 2),
          result varchar(20) not null default 'Accepted',
          created_at timestamptz not null default now()
        )
        """);
  }

  private void ensureAlertTable() {
    jdbcTemplate.update(
        """
        create table if not exists alert_history (
          id bigserial primary key,
          alert_type varchar(50) not null,
          sensor_value varchar(50),
          threshold varchar(50),
          action_taken varchar(100),
          status varchar(20) not null default 'Pending',
          created_at timestamptz not null default now()
        )
        """);
  }

  // ── Helpers ───────────────────────────────────────────────────────────────

  private long countTable(String tableName) {
    if (!databaseAvailable) {
      return 0;
    }

    try {
      // Ensure the table exists before counting
      switch (tableName) {
        case "telemetry_readings" -> ensureTelemetryTable();
        case "control_logs" -> ensureControlLogsTable();
        case "voice_command_history" -> ensureVoiceTable();
        case "alert_history" -> ensureAlertTable();
      }

      Long count = jdbcTemplate.queryForObject(
          "select count(*) from " + tableName, Long.class);
      return count != null ? count : 0;
    } catch (DataAccessException exception) {
      warnDatabaseFallback(exception);
      return 0;
    }
  }

  private OffsetDateTime toOffsetDateTime(Timestamp timestamp) {
    if (timestamp == null) {
      return null;
    }

    return timestamp.toInstant().atOffset(ZoneOffset.UTC);
  }

  private String formatTime(OffsetDateTime dateTime) {
    if (dateTime == null) {
      return "";
    }

    return dateTime.atZoneSameInstant(APP_ZONE).format(TIME_FORMAT);
  }

  private String formatTimeShort(OffsetDateTime dateTime) {
    if (dateTime == null) {
      return "";
    }

    return dateTime.atZoneSameInstant(APP_ZONE).format(DateTimeFormatter.ofPattern("HH:mm:ss"));
  }

  private void warnDatabaseFallback(DataAccessException exception) {
    databaseAvailable = false;

    if (!databaseWarningLogged) {
      logger.warn("Database is unavailable; history queries will return empty results: {}",
          exception.getMostSpecificCause().getMessage());
      databaseWarningLogged = true;
    }
  }
}
