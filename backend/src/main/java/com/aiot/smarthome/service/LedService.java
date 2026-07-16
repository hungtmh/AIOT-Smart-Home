package com.aiot.smarthome.service;

import com.aiot.smarthome.dto.LedStateResponse;
import com.aiot.smarthome.model.DeviceState;
import com.aiot.smarthome.mqtt.MqttCommandPublisher;
import com.aiot.smarthome.repository.LedRepository;
import org.springframework.stereotype.Service;

@Service
public class LedService {
  private final LedRepository repository;
  private final MqttCommandPublisher mqttCommandPublisher;

  public LedService(LedRepository repository, MqttCommandPublisher mqttCommandPublisher) {
    this.repository = repository;
    this.mqttCommandPublisher = mqttCommandPublisher;
  }

  public LedStateResponse getLedState() {
    return LedStateResponse.from(repository.findLedState(), false);
  }

  public LedStateResponse commandLed(boolean state) {
    DeviceState nextState = repository.setDesiredLedState(state);
    boolean mqttPublished = mqttCommandPublisher.publishLedState(state);
    repository.addControlLog(state, "web", mqttPublished ? "MQTT_PUBLISHED" : "MQTT_NOT_CONNECTED");
    return LedStateResponse.from(nextState, mqttPublished);
  }

  public LedStateResponse toggleLed() {
    DeviceState current = repository.findLedState();
    return commandLed(!current.desiredState());
  }

  public void handleReportedLedState(boolean state) {
    repository.setReportedLedState(state);
    repository.addControlLog(state, "esp32", "STATE_REPORTED");
  }
}
