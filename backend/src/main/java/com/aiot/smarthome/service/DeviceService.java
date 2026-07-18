package com.aiot.smarthome.service;

import com.aiot.smarthome.dto.DeviceStateResponse;
import com.aiot.smarthome.model.DeviceDefinition;
import com.aiot.smarthome.model.DeviceState;
import com.aiot.smarthome.mqtt.MqttCommandPublisher;
import com.aiot.smarthome.repository.DeviceRepository;
import java.util.List;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.web.server.ResponseStatusException;

@Service
public class DeviceService {
  private final DeviceCatalog catalog;
  private final DeviceRepository repository;
  private final MqttCommandPublisher mqttCommandPublisher;

  public DeviceService(DeviceCatalog catalog, DeviceRepository repository, MqttCommandPublisher mqttCommandPublisher) {
    this.catalog = catalog;
    this.repository = repository;
    this.mqttCommandPublisher = mqttCommandPublisher;
  }

  public List<DeviceStateResponse> getDeviceStates() {
    return repository.findAllStates(catalog.all()).stream()
        .map(state -> toResponse(state, false))
        .toList();
  }

  public DeviceStateResponse getDeviceState(String deviceId) {
    DeviceDefinition device = requireDevice(deviceId);
    return DeviceStateResponse.from(repository.findState(device), device.name(), device.type(), false);
  }

  public DeviceStateResponse commandDevice(String deviceId, boolean state) {
    DeviceDefinition device = requireDevice(deviceId);
    DeviceState nextState = repository.setDesiredState(device, state);
    boolean mqttPublished = mqttCommandPublisher.publishDeviceState(device.id(), state);
    repository.addControlLog(device.id(), state, "web", mqttPublished ? "MQTT_PUBLISHED" : "MQTT_NOT_CONNECTED");
    return DeviceStateResponse.from(nextState, device.name(), device.type(), mqttPublished);
  }

  public DeviceStateResponse toggleDevice(String deviceId) {
    DeviceDefinition device = requireDevice(deviceId);
    DeviceState current = repository.findState(device);
    return commandDevice(device.id(), !current.desiredState());
  }

  public void handleReportedState(String deviceId, boolean state) {
    DeviceDefinition device = requireDevice(deviceId);
    repository.setReportedState(device, state);
    repository.addControlLog(device.id(), state, "esp32", "STATE_REPORTED");
  }

  private DeviceStateResponse toResponse(DeviceState state, boolean mqttPublished) {
    DeviceDefinition device = requireDevice(state.deviceId());
    return DeviceStateResponse.from(state, device.name(), device.type(), mqttPublished);
  }

  private DeviceDefinition requireDevice(String deviceId) {
    return catalog.find(deviceId)
        .orElseThrow(() -> new ResponseStatusException(HttpStatus.NOT_FOUND, "Unsupported device: " + deviceId));
  }
}
