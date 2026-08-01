package com.aiot.smarthome.model;

import java.time.LocalDateTime;

public class FireAlert {

    private Long id;

    private String deviceId;

    private String imagePath;

    private Double confidence;

    private LocalDateTime detectedAt;

    private String status;


    public FireAlert() {
    }
    public FireAlert(
            Long id,
            String deviceId,
            String imagePath,
            Double confidence,
            LocalDateTime detectedAt,
            String status
    ) {
        this.id = id;
        this.deviceId = deviceId;
        this.imagePath = imagePath;
        this.confidence = confidence;
        this.detectedAt = detectedAt;
        this.status = status;
    }


    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }


    public String getDeviceId() {
        return deviceId;
    }

    public void setDeviceId(String deviceId) {
        this.deviceId = deviceId;
    }


    public String getImagePath() {
        return imagePath;
    }

    public void setImagePath(String imagePath) {
        this.imagePath = imagePath;
    }


    public Double getConfidence() {
        return confidence;
    }

    public void setConfidence(Double confidence) {
        this.confidence = confidence;
    }


    public LocalDateTime getDetectedAt() {
        return detectedAt;
    }

    public void setDetectedAt(LocalDateTime detectedAt) {
        this.detectedAt = detectedAt;
    }


    public String getStatus() {
        return status;
    }

    public void setStatus(String status) {
        this.status = status;
    }
}