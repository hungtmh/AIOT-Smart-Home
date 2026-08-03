package com.aiot.smarthome.service;

import com.aiot.smarthome.config.AiotProperties;
import com.aiot.smarthome.dto.FireAlertResponse;
import com.aiot.smarthome.model.FireAlert;
import com.aiot.smarthome.realtime.RealtimeHub;
import com.aiot.smarthome.repository.FireAlertRepository;
import com.aiot.smarthome.repository.HistoryRepository;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.context.annotation.Lazy;
import org.springframework.stereotype.Service;

import java.time.LocalDateTime;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;

@Service
public class FireAlertService {
    private static final Logger logger = LoggerFactory.getLogger(FireAlertService.class);

    private final FireAlertRepository repository;
    private final RealtimeHub realtimeHub;
    private final DeviceService deviceService;
    private final HistoryRepository historyRepository;
    private final AiotProperties properties;

    private final ScheduledExecutorService scheduler = Executors.newSingleThreadScheduledExecutor();
    private ScheduledFuture<?> scheduledPumpOffTask = null;

    public FireAlertService(
            FireAlertRepository repository,
            RealtimeHub realtimeHub,
            @Lazy DeviceService deviceService,
            HistoryRepository historyRepository,
            AiotProperties properties
    ) {
        this.repository = repository;
        this.realtimeHub = realtimeHub;
        this.deviceService = deviceService;
        this.historyRepository = historyRepository;
        this.properties = properties;
    }

    public FireAlert getLatest() {
        return repository.findLatest().orElse(null);
    }

    public List<FireAlert> getAlerts() {
        return repository.findAll();
    }

    public FireAlert createAlert(
            String deviceId,
            String imagePath,
            Double confidence
    ) {
        FireAlert alert = new FireAlert(
                null,
                deviceId,
                imagePath,
                confidence,
                LocalDateTime.now(),
                "FIRE"
        );

        FireAlert saved = repository.save(alert);

        FireAlertResponse response = new FireAlertResponse(
                saved.getId(),
                saved.getDeviceId(),
                saved.getImagePath(),
                saved.getConfidence(),
                saved.getDetectedAt(),
                saved.getStatus()
        );

        realtimeHub.broadcastFireAlert(response);

        // Check if confidence meets or exceeds pump activation threshold
        double threshold = (properties.fireAlert() != null) ? properties.fireAlert().pumpThreshold() : 0.80;
        int autoOffSec = (properties.fireAlert() != null) ? properties.fireAlert().pumpAutoOffSeconds() : 60;

        if (confidence != null && confidence >= threshold) {
            logger.info("High-confidence fire alert detected ({} >= {}). Auto-activating water pump relay...",
                    confidence, threshold);
            try {
                // 1. Activate pump relay via DeviceService (publishes MQTT & updates DB & broadcasts WS)
                deviceService.commandDevice("pump", true);

                // 2. Save history event
                historyRepository.saveAlert("FIRE_AUTO_PUMP_ON",
                        String.format("%.1f%%", confidence * 100),
                        String.format("%.1f%%", threshold * 100),
                        "AUTO_PUMP_ACTIVATED",
                        "Active");

                // 3. Schedule auto turn-off after autoOffSec seconds
                cancelScheduledPumpOff();
                scheduledPumpOffTask = scheduler.schedule(() -> {
                    try {
                        logger.info("Auto-off timer expired ({}s). Turning off water pump...", autoOffSec);
                        deviceService.commandDevice("pump", false);
                        historyRepository.saveAlert("FIRE_AUTO_PUMP_OFF",
                                "Timer " + autoOffSec + "s",
                                "-",
                                "AUTO_PUMP_DEACTIVATED",
                                "Resolved");
                    } catch (Exception e) {
                        logger.error("Failed to auto turn off pump: {}", e.getMessage());
                    }
                }, autoOffSec, TimeUnit.SECONDS);

            } catch (Exception e) {
                logger.error("Failed to auto-command pump on fire alert: {}", e.getMessage());
            }
        }

        return saved;
    }

    public synchronized void cancelScheduledPumpOff() {
        if (scheduledPumpOffTask != null && !scheduledPumpOffTask.isDone()) {
            scheduledPumpOffTask.cancel(false);
            scheduledPumpOffTask = null;
        }
    }
}