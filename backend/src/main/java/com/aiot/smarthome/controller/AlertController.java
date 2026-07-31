package com.aiot.smarthome.controller;

import com.aiot.smarthome.service.AlertService;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;

@RestController
@RequestMapping("/api/alerts")
public class AlertController {
  private final AlertService alertService;

  public AlertController(AlertService alertService) {
    this.alertService = alertService;
  }

  @PostMapping("/fire")
  public ResponseEntity<String> receiveFireAlert(@RequestParam("image") MultipartFile image) {
    try {
      byte[] imageBytes = image.getBytes();
      alertService.handleFireAlert(imageBytes);
      return ResponseEntity.ok("Alert received and broadcasted");
    } catch (IOException e) {
      return ResponseEntity.status(500).body("Failed to process image");
    }
  }
}
