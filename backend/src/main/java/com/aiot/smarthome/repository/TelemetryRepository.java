package com.aiot.smarthome.repository;

import com.aiot.smarthome.model.SensorTelemetry;
import java.sql.Timestamp;
import java.time.OffsetDateTime;
import java.time.ZoneOffset;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.dao.DataAccessException;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.stereotype.Repository;

@Repository
public class TelemetryRepository {
  private static final Logger logger = LoggerFactory.getLogger(TelemetryRepository.class);

  private final JdbcTemplate jdbcTemplate;
  private SensorTelemetry memoryTelemetry = new SensorTelemetry(28.4, 64.0, 18, OffsetDateTime.now(ZoneOffset.UTC));
  private boolean databaseAvailable = true;
  private boolean databaseWarningLogged;

  public TelemetryRepository(JdbcTemplate jdbcTemplate) {
    this.jdbcTemplate = jdbcTemplate;
  }

  public SensorTelemetry findLatest() {
    if (!databaseAvailable) {
      return memoryTelemetry;
    }

    try {
      ensureTelemetryTable();

      SensorTelemetry telemetry = jdbcTemplate.query(
          """
          select temperature, humidity, smoke_ppm, measured_at
          from telemetry_readings
          order by measured_at desc
          limit 1
          """,
          rs -> {
            if (!rs.next()) {
              return memoryTelemetry;
            }

            return new SensorTelemetry(
                rs.getDouble("temperature"),
                rs.getDouble("humidity"),
                rs.getInt("smoke_ppm"),
                toOffsetDateTime(rs.getTimestamp("measured_at")));
          });

      memoryTelemetry = telemetry;
      return telemetry;
    } catch (DataAccessException exception) {
      warnDatabaseFallback(exception);
      return memoryTelemetry;
    }
  }

  public SensorTelemetry save(double temperature, double humidity, int smokePpm) {
    memoryTelemetry = new SensorTelemetry(temperature, humidity, smokePpm, OffsetDateTime.now(ZoneOffset.UTC));

    if (!databaseAvailable) {
      return memoryTelemetry;
    }

    try {
      ensureTelemetryTable();

      jdbcTemplate.update(
          """
          insert into telemetry_readings (temperature, humidity, smoke_ppm)
          values (?, ?, ?)
          """,
          temperature,
          humidity,
          smokePpm);

      return findLatest();
    } catch (DataAccessException exception) {
      warnDatabaseFallback(exception);
      return memoryTelemetry;
    }
  }

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

  private OffsetDateTime toOffsetDateTime(Timestamp timestamp) {
    if (timestamp == null) {
      return null;
    }

    return timestamp.toInstant().atOffset(ZoneOffset.UTC);
  }

  private void warnDatabaseFallback(DataAccessException exception) {
    databaseAvailable = false;

    if (!databaseWarningLogged) {
      logger.warn("Database is unavailable; using in-memory telemetry until the DB connection is fixed: {}",
          exception.getMostSpecificCause().getMessage());
      databaseWarningLogged = true;
    }
  }
}
