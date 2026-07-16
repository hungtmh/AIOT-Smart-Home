package com.aiot.smarthome.config;

import javax.crypto.spec.SecretKeySpec;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Base64;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.http.HttpMethod;
import org.springframework.http.MediaType;
import org.springframework.security.config.Customizer;
import org.springframework.security.config.annotation.web.builders.HttpSecurity;
import org.springframework.security.config.annotation.web.configurers.AbstractHttpConfigurer;
import org.springframework.security.oauth2.jose.jws.MacAlgorithm;
import org.springframework.security.oauth2.jose.jws.SignatureAlgorithm;
import org.springframework.security.oauth2.jwt.JwtDecoder;
import org.springframework.security.oauth2.jwt.NimbusJwtDecoder;
import org.springframework.security.web.SecurityFilterChain;

@Configuration
public class SecurityConfig {
  private static final Logger logger = LoggerFactory.getLogger(SecurityConfig.class);
  private static final String FALLBACK_SECRET = "dev-only-fallback-secret-with-at-least-32-bytes";
  private static final Pattern ALG_PATTERN = Pattern.compile("\"alg\"\\s*:\\s*\"([^\"]+)\"");
  private static final Pattern KID_PATTERN = Pattern.compile("\"kid\"\\s*:\\s*\"([^\"]+)\"");

  private final AiotProperties properties;

  public SecurityConfig(AiotProperties properties) {
    this.properties = properties;
  }

  @Bean
  public SecurityFilterChain securityFilterChain(HttpSecurity http) throws Exception {
    boolean jwtEnabled = hasJwtConfig();

    http.csrf(AbstractHttpConfigurer::disable)
        .cors(Customizer.withDefaults())
        .authorizeHttpRequests(authorize -> {
          authorize.requestMatchers(HttpMethod.OPTIONS, "/**").permitAll();

          if (jwtEnabled) {
            authorize.requestMatchers("/api/**").authenticated();
          } else {
            logger.warn("SUPABASE_JWT_SECRET is not configured; /api/** is temporarily public.");
            authorize.requestMatchers("/api/**").permitAll();
          }

          authorize.anyRequest().permitAll();
        });

    if (jwtEnabled) {
      http.oauth2ResourceServer(oauth2 -> oauth2
          .jwt(Customizer.withDefaults())
          .authenticationEntryPoint((request, response, exception) -> {
            String authorization = request.getHeader("Authorization");
            String hasAuthorization = authorization == null ? "false" : "true";
            logger.warn("Rejected API request {} {}: {}. Authorization header present: {}. JWT {}",
                request.getMethod(),
                request.getRequestURI(),
                exception.getMessage(),
                hasAuthorization,
                jwtHeaderSummary(authorization));

            response.setStatus(401);
            response.setContentType(MediaType.APPLICATION_JSON_VALUE);
            response.getOutputStream().write(("""
                {"error":"unauthorized","message":"JWT is missing or invalid"}
                """).getBytes(StandardCharsets.UTF_8));
          }));
    }

    return http.build();
  }

  @Bean
  public JwtDecoder jwtDecoder() {
    List<JwtDecoder> decoders = new ArrayList<>();

    if (hasJwksUri()) {
      decoders.add(jwksDecoder());
    }

    if (hasJwtSecret()) {
      decoders.add(secretDecoder(properties.auth().jwtSecret().getBytes(StandardCharsets.UTF_8)));
      decodeBase64Secret(properties.auth().jwtSecret()).ifPresent(secret -> decoders.add(secretDecoder(secret)));
    }

    if (decoders.isEmpty()) {
      decoders.add(secretDecoder(FALLBACK_SECRET.getBytes(StandardCharsets.UTF_8)));
    }

    if (decoders.size() == 1) {
      return decoders.get(0);
    }

    return new FallbackJwtDecoder(decoders);
  }

  private JwtDecoder jwksDecoder() {
    return NimbusJwtDecoder.withJwkSetUri(properties.auth().jwksUri())
        .jwsAlgorithm(SignatureAlgorithm.ES256)
        .build();
  }

  private JwtDecoder secretDecoder(byte[] secret) {
    SecretKeySpec key = new SecretKeySpec(secret, "HmacSHA256");
    return NimbusJwtDecoder.withSecretKey(key)
        .macAlgorithm(MacAlgorithm.HS256)
        .build();
  }

  private java.util.Optional<byte[]> decodeBase64Secret(String secret) {
    try {
      return java.util.Optional.of(Base64.getDecoder().decode(secret));
    } catch (IllegalArgumentException exception) {
      return java.util.Optional.empty();
    }
  }

  private String jwtHeaderSummary(String authorization) {
    if (authorization == null || !authorization.startsWith("Bearer ")) {
      return "header=none";
    }

    try {
      String token = authorization.substring("Bearer ".length());
      String encodedHeader = token.split("\\.")[0];
      String headerJson = new String(Base64.getUrlDecoder().decode(encodedHeader), StandardCharsets.UTF_8);
      return "alg=" + findJsonValue(headerJson, ALG_PATTERN) + " kid=" + findJsonValue(headerJson, KID_PATTERN);
    } catch (Exception exception) {
      return "header=unreadable";
    }
  }

  private String findJsonValue(String json, Pattern pattern) {
    Matcher matcher = pattern.matcher(json);
    return matcher.find() ? matcher.group(1) : "none";
  }

  private boolean hasJwtConfig() {
    return hasJwksUri() || hasJwtSecret();
  }

  private boolean hasJwksUri() {
    return properties.auth() != null
        && properties.auth().jwksUri() != null
        && !properties.auth().jwksUri().isBlank();
  }

  private boolean hasJwtSecret() {
    return properties.auth() != null
        && properties.auth().jwtSecret() != null
        && !properties.auth().jwtSecret().isBlank();
  }
}
