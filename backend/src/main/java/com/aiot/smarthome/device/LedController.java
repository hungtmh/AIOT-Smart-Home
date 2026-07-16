package com.aiot.smarthome.device;

import jakarta.validation.Valid;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/devices/led")
public class LedController {
  private final LedService ledService;

  public LedController(LedService ledService) {
    this.ledService = ledService;
  }

  @GetMapping
  public LedStateResponse getLedState() {
    return ledService.getLedState();
  }

  @PostMapping("/command")
  public ResponseEntity<LedStateResponse> commandLed(@Valid @RequestBody LedCommandRequest request) {
    return ResponseEntity.ok(ledService.commandLed(request.state()));
  }

  @PostMapping("/toggle")
  public ResponseEntity<LedStateResponse> toggleLed() {
    return ResponseEntity.ok(ledService.toggleLed());
  }
}
