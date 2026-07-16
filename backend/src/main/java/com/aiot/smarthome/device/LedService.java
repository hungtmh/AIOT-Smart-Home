package com.aiot.smarthome.device;

import com.aiot.smarthome.mqtt.MqttCommandPublisher;
import org.springframework.stereotype.Service;

@Service
public class LedService {
  private final DeviceRepository repository;
  private final MqttCommandPublisher mqttCommandPublisher;

  public LedService(DeviceRepository repository, MqttCommandPublisher mqttCommandPublisher) {
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
