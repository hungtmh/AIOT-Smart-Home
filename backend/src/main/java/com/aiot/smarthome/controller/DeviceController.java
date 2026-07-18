package com.aiot.smarthome.controller;

import com.aiot.smarthome.dto.DeviceCommandRequest;
import com.aiot.smarthome.dto.DeviceStateResponse;
import com.aiot.smarthome.service.DeviceService;
import jakarta.validation.Valid;
import java.util.List;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/devices")
public class DeviceController {
  private final DeviceService deviceService;

  public DeviceController(DeviceService deviceService) {
    this.deviceService = deviceService;
  }

  @GetMapping
  public List<DeviceStateResponse> getDeviceStates() {
    return deviceService.getDeviceStates();
  }

  @GetMapping("/{deviceId}")
  public DeviceStateResponse getDeviceState(@PathVariable String deviceId) {
    return deviceService.getDeviceState(deviceId);
  }

  @PostMapping("/{deviceId}/command")
  public ResponseEntity<DeviceStateResponse> commandDevice(
      @PathVariable String deviceId,
      @Valid @RequestBody DeviceCommandRequest request) {
    return ResponseEntity.ok(deviceService.commandDevice(deviceId, request.state()));
  }

  @PostMapping("/{deviceId}/toggle")
  public ResponseEntity<DeviceStateResponse> toggleDevice(@PathVariable String deviceId) {
    return ResponseEntity.ok(deviceService.toggleDevice(deviceId));
  }
}
