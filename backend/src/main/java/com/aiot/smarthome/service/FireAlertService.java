package com.aiot.smarthome.service;

import com.aiot.smarthome.dto.FireAlertResponse;
import com.aiot.smarthome.model.FireAlert;
import com.aiot.smarthome.repository.FireAlertRepository;
import com.aiot.smarthome.realtime.RealtimeHub;

import org.springframework.stereotype.Service;

import java.time.LocalDateTime;
import java.util.List;


@Service
public class FireAlertService {


    private final FireAlertRepository repository;
    private final RealtimeHub realtimeHub;


    public FireAlertService(
            FireAlertRepository repository,
            RealtimeHub realtimeHub
    ){
        this.repository = repository;
        this.realtimeHub = realtimeHub;
    }


    public FireAlert getLatest(){

        return repository.findLatest()
                .orElse(null);

    }


    public List<FireAlert> getAlerts(){

        return repository.findAll();

    }


    public FireAlert createAlert(
            String deviceId,
            String imagePath,
            Double confidence
    ){

        FireAlert alert = new FireAlert(
                null,
                deviceId,
                imagePath,
                confidence,
                LocalDateTime.now(),
                "FIRE"
        );


        FireAlert saved = repository.save(alert);


        FireAlertResponse response =
                new FireAlertResponse(
                        saved.getId(),
                        saved.getDeviceId(),
                        saved.getImagePath(),
                        saved.getConfidence(),
                        saved.getDetectedAt(),
                        saved.getStatus()
                );


        realtimeHub.broadcastFireAlert(response);


        return saved;
    }

}