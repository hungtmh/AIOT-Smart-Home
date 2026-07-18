package com.aiot.smarthome;

import com.aiot.smarthome.config.AiotProperties;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.LinkedHashMap;
import java.util.Map;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.context.properties.EnableConfigurationProperties;

@SpringBootApplication
@EnableConfigurationProperties(AiotProperties.class)
public class AiotSmartHomeApplication {
  public static void main(String[] args) {
    Map<String, String> localEnv = readEnvFile();
    applyLocalEnv(localEnv);
    System.setProperty("debug", localEnv.getOrDefault("DEBUG", "false"));
    printConfigSummary();

    SpringApplication application = new SpringApplication(AiotSmartHomeApplication.class);
    application.setDefaultProperties(loadLocalEnv(localEnv));
    application.run(args);
  }

  private static void applyLocalEnv(Map<String, String> env) {
    env.forEach(System::setProperty);

    setSpringProperty(env, "spring.datasource.url", "SUPABASE_DB_URL");
    setSpringProperty(env, "spring.datasource.username", "SUPABASE_DB_USERNAME");
    setSpringProperty(env, "spring.datasource.password", "SUPABASE_DB_PASSWORD");
    setSpringProperty(env, "aiot.auth.jwt-secret", "SUPABASE_JWT_SECRET");
    setSpringProperty(env, "aiot.auth.jwks-uri", "SUPABASE_JWKS_URI");
    setSpringProperty(env, "aiot.cors.allowed-origins", "CORS_ALLOWED_ORIGINS");
    setSpringProperty(env, "aiot.mqtt.broker-uri", "MQTT_BROKER_URI");
    setSpringProperty(env, "aiot.mqtt.username", "MQTT_USERNAME");
    setSpringProperty(env, "aiot.mqtt.password", "MQTT_PASSWORD");
    setSpringProperty(env, "aiot.mqtt.command-topic-pattern", "MQTT_COMMAND_TOPIC_PATTERN");
    setSpringProperty(env, "aiot.mqtt.state-topic-pattern", "MQTT_STATE_TOPIC_PATTERN");
    setSpringProperty(env, "aiot.mqtt.telemetry-topic", "MQTT_TELEMETRY_TOPIC");
  }

  private static Map<String, Object> loadLocalEnv(Map<String, String> env) {
    Map<String, Object> properties = new LinkedHashMap<>();

    putIfPresent(properties, "spring.datasource.url", env, "SUPABASE_DB_URL");
    putIfPresent(properties, "spring.datasource.username", env, "SUPABASE_DB_USERNAME");
    putIfPresent(properties, "spring.datasource.password", env, "SUPABASE_DB_PASSWORD");
    putIfPresent(properties, "aiot.auth.jwt-secret", env, "SUPABASE_JWT_SECRET");
    putIfPresent(properties, "aiot.auth.jwks-uri", env, "SUPABASE_JWKS_URI");
    putIfPresent(properties, "aiot.cors.allowed-origins", env, "CORS_ALLOWED_ORIGINS");
    putIfPresent(properties, "aiot.mqtt.broker-uri", env, "MQTT_BROKER_URI");
    putIfPresent(properties, "aiot.mqtt.username", env, "MQTT_USERNAME");
    putIfPresent(properties, "aiot.mqtt.password", env, "MQTT_PASSWORD");
    putIfPresent(properties, "aiot.mqtt.command-topic-pattern", env, "MQTT_COMMAND_TOPIC_PATTERN");
    putIfPresent(properties, "aiot.mqtt.state-topic-pattern", env, "MQTT_STATE_TOPIC_PATTERN");
    putIfPresent(properties, "aiot.mqtt.telemetry-topic", env, "MQTT_TELEMETRY_TOPIC");

    return properties;
  }

  private static void setSpringProperty(Map<String, String> env, String propertyName, String envName) {
    String value = env.get(envName);
    if (value != null && !value.isBlank()) {
      System.setProperty(propertyName, value);
    }
  }

  private static Map<String, String> readEnvFile() {
    Path workingDirectory = Path.of("").toAbsolutePath();
    Path[] candidates = {
      workingDirectory.resolve(".env"),
      workingDirectory.resolve("backend/.env"),
      workingDirectory.resolve("AIOT-Smart-Home/backend/.env")
    };

    for (Path candidate : candidates) {
      if (Files.exists(candidate)) {
        return parseEnvFile(candidate);
      }
    }

    return Map.of();
  }

  private static Map<String, String> parseEnvFile(Path path) {
    Map<String, String> env = new LinkedHashMap<>();

    try {
      for (String rawLine : Files.readAllLines(path)) {
        String line = rawLine.trim();
        if (line.isBlank() || line.startsWith("#") || !line.contains("=")) {
          continue;
        }

        String[] parts = line.split("=", 2);
        env.put(parts[0].trim(), parts[1].trim());
      }
    } catch (IOException exception) {
      throw new IllegalStateException("Could not read local .env file: " + path, exception);
    }

    return env;
  }

  private static void putIfPresent(
      Map<String, Object> properties,
      String propertyName,
      Map<String, String> env,
      String envName) {
    String value = env.get(envName);
    if (value == null || value.isBlank()) {
      value = System.getenv(envName);
    }

    if (value != null && !value.isBlank()) {
      properties.put(propertyName, value);
    }
  }

  private static void printConfigSummary() {
    String datasourceUrl = System.getProperty("spring.datasource.url", "");
    String mqttBrokerUri = System.getProperty("aiot.mqtt.broker-uri", "");

    if (!datasourceUrl.isBlank()) {
      System.out.println("[config] Supabase DB: " + sanitizeJdbcUrl(datasourceUrl));
    }

    if (!mqttBrokerUri.isBlank()) {
      System.out.println("[config] MQTT broker: " + mqttBrokerUri);
    }
  }

  private static String sanitizeJdbcUrl(String jdbcUrl) {
    int queryStart = jdbcUrl.indexOf('?');
    if (queryStart == -1) {
      return jdbcUrl;
    }

    return jdbcUrl.substring(0, queryStart);
  }
}
