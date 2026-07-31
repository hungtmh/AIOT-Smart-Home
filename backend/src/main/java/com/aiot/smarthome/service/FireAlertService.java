package com.aiot.smarthome.service;

import com.aiot.smarthome.dto.FireAlertResponse;
import com.aiot.smarthome.model.FireAlert;
import com.aiot.smarthome.repository.FireAlertRepository;
import com.aiot.smarthome.realtime.RealtimeHub;

import org.springframework.stereotype.Service;

import java.time.LocalDateTime;
import java.util.List;
import java.util.Optional;


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



    // Python gọi vào đây để lưu cảnh báo cháy
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



        // Convert Entity -> DTO trước khi gửi websocket
        FireAlertResponse response = new FireAlertResponse(
                saved.getId(),
                saved.getDeviceId(),
                saved.getImagePath(),
                saved.getConfidence(),
                saved.getDetectedAt(),
                saved.getStatus()
        );


        // đẩy realtime qua websocket
        realtimeHub.broadcastFireAlert(response);



        return saved;
    }




    // lấy toàn bộ lịch sử cháy
    public List<FireAlert> getAlerts(){

        return repository.findAll();

    }




    // lấy cảnh báo mới nhất
    public FireAlert getLatest(){

        Optional<FireAlert> latest =
                repository.findLatest();


        return latest.orElse(null);

    }


}