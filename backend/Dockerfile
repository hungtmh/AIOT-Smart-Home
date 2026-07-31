# Stage 1: Build stage using Maven 3.9 + Java 17 Alpine
FROM maven:3.9.9-eclipse-temurin-17-alpine AS builder
WORKDIR /app

# Copy pom.xml and pre-fetch dependencies for better Docker layer caching
COPY pom.xml ./
RUN mvn dependency:go-offline -B || true

# Copy source code and compile production package
COPY src ./src
RUN mvn clean package -DskipTests

# Stage 2: Minimal Java 17 Runtime stage
FROM eclipse-temurin:17-jre-alpine
WORKDIR /app

# Copy executable JAR file from builder stage
COPY --from=builder /app/target/*.jar app.jar

EXPOSE 8080

ENTRYPOINT ["java", "-jar", "app.jar"]
