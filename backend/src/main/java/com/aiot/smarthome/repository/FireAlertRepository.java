package com.aiot.smarthome.repository;

import com.aiot.smarthome.model.FireAlert;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.time.LocalDateTime;
import java.util.List;
import java.util.Optional;

import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.stereotype.Repository;

@Repository
public class FireAlertRepository {

    private final JdbcTemplate jdbcTemplate;

    public FireAlertRepository(JdbcTemplate jdbcTemplate) {
        this.jdbcTemplate = jdbcTemplate;
    }

    public FireAlert save(FireAlert alert) {

        String sql = """
            INSERT INTO fire_alert
            (
                device_id,
                image_path,
                confidence,
                detected_at,
                status
            )
            VALUES (?, ?, ?, ?, ?)
            RETURNING id
            """;

        Long id = jdbcTemplate.queryForObject(
                sql,
                Long.class,
                alert.getDeviceId(),
                alert.getImagePath(),
                alert.getConfidence(),
                alert.getDetectedAt(),
                alert.getStatus()
        );

        alert.setId(id);

        return alert;
    }

    public List<FireAlert> findAll() {

        String sql = """
            SELECT *
            FROM fire_alert
            ORDER BY detected_at DESC
            """;

        return jdbcTemplate.query(
                sql,
                this::mapRow
        );
    }

    public Optional<FireAlert> findLatest() {

        String sql = """
            SELECT *
            FROM fire_alert
            ORDER BY detected_at DESC
            LIMIT 1
            """;

        List<FireAlert> result =
                jdbcTemplate.query(
                        sql,
                        this::mapRow
                );

        return result.stream().findFirst();
    }

    private FireAlert mapRow(
            ResultSet rs,
            int row
    ) throws SQLException {

        return new FireAlert(
                rs.getLong("id"),
                rs.getString("device_id"),
                rs.getString("image_path"),
                rs.getDouble("confidence"),
                rs.getObject(
                        "detected_at",
                        LocalDateTime.class
                ),
                rs.getString("status")
        );
    }
}