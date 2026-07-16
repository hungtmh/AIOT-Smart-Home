package com.aiot.smarthome.config;

import org.springframework.boot.context.properties.ConfigurationProperties;

@ConfigurationProperties(prefix = "aiot")
public record AiotProperties(Auth auth, Cors cors, Mqtt mqtt) {
  public record Auth(String jwtSecret, String jwksUri) {
  }

  public record Cors(String allowedOrigins) {
  }

  public record Mqtt(
      String brokerUri,
      String clientId,
      String username,
      String password,
      String commandTopic,
      String stateTopic) {
  }
}
